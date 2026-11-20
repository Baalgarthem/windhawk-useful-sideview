#pragma once

#include <windows.h>
#include <commctrl.h>
#include <map>
#include <mutex>
#include <vector>
#include <windhawk_api.h>
#include <windhawk_utils.h>
#include "v2/ui/ventana_base.hpp"
#include "v2/ui/tema_visual.hpp"
#include "v2/nucleo/shell_extractor.hpp"
#include "v2/nucleo/enrutador_vistas.hpp"
#include "v2/nucleo/fabrica_vistas.hpp"
#include "v2/nucleo/configuracion_v2.hpp"

namespace SideViewV2 {

inline UINT WM_SIDEVIEW_TOGGLE = 0;
inline UINT WM_SIDEVIEW_SETTINGS = 0;

inline std::mutex g_bloqueoEstados;
inline std::map<HWND, EstadoPanelV2*> g_estadosPorVentana;

inline bool EsComparadorArchivosActivoPorSeleccionV2(
    const std::vector<std::wstring>& seleccion) {
    return FabricaVistas::DebeCrearComparador(seleccion);
}

LRESULT CALLBACK ProcedimientoVentanaPanel(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ProcedimientoSubclasePadre(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                            UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK ProcedimientoSubclaseTop(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                          DWORD_PTR dwRefData);

inline void DestruirEstado(EstadoPanelV2* estado, bool quitarDelMapa = true) {
    if (!estado) return;
    bool esperado = false;
    if (!estado->destruyendo.compare_exchange_strong(esperado, true,
                                                     std::memory_order_acq_rel)) {
        return;
    }

    // Cortar primero la entrada del estado evita que un timer, una reentrada
    // de DestroyWindow o una notificación pendiente use recursos en proceso
    // de liberación.
    estado->abierto = false;
    if (estado->panel && IsWindow(estado->panel)) {
        KillTimer(estado->panel, 101);
    }

    if (quitarDelMapa) {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        for (auto it = g_estadosPorVentana.begin(); it != g_estadosPorVentana.end(); ) {
            if (it->second == estado) {
                it = g_estadosPorVentana.erase(it);
            } else {
                ++it;
            }
        }
    }

    LiberarBufferMemoria(estado);

    if (estado->vistaContenido) {
        estado->vistaContenido->Liberar();
        estado->vistaContenido.reset();
    }

    if (estado->panel && IsWindow(estado->panel)) {
        DestroyWindow(estado->panel);
        estado->panel = nullptr;
    }

    if (estado->parent && IsWindow(estado->parent)) {
        RemoveWindowSubclass(estado->parent, ProcedimientoSubclasePadre, 1);
    }

    if (estado->tab && IsWindow(estado->tab)) {
        RemoveWindowSubclass(estado->tab, ProcedimientoSubclasePadre, 2);
    }

    delete estado;
}

inline void DisponerPanel(EstadoPanelV2* estado) {
    if (!estado || !estado->abierto || !estado->panel || !estado->view || !estado->parent) {
        return;
    }
    if (!IsWindow(estado->panel) || !IsWindow(estado->view) || !IsWindow(estado->parent)) {
        return;
    }

    UINT dpi = ObtenerDpiVentana(estado->panel);
    RECT rectCliente{};
    GetClientRect(estado->parent, &rectCliente);

    // El renderer GPU es un hijo directo del frame y el mod de referencia
    // conserva SHELLDLL_DefView como superficie COM oculta. No se debe
    // redimensionar el overlay GPU: sus hooks de layout y DirectComposition
    // volverían a ocupar el área completa y podrían producir una carrera de
    // geometría. El panel se dibuja encima del sector derecho y el overlay
    // permanece intacto como superficie de Explorer.
    if (estado->superficie.gpuAcelerada) {
        if (!IsWindowVisible(estado->view)) {
            ShowWindow(estado->panel, SW_HIDE);
            return;
        }
        auto compatibilidad = FabricaVistas::Dependencias().compatibilidadGpu;
        if (!compatibilidad || !compatibilidad->ActualizarLimites(estado->superficie)) {
            return;
        }
        estado->view = estado->superficie.superficieVisual;
        estado->parent = estado->superficie.contenedor;
        estado->viewOriginal = estado->superficie.limites;
        estado->parentCliente = rectCliente;

        const int anchoVista = estado->viewOriginal.right - estado->viewOriginal.left;
        const int altoVista = estado->viewOriginal.bottom - estado->viewOriginal.top;
        if (anchoVista < 1 || altoVista < 1) return;

        if (estado->comparadorArchivosActivo) {
            SetWindowPos(estado->panel, HWND_TOP, estado->viewOriginal.left,
                         estado->viewOriginal.top, anchoVista, altoVista,
                         SWP_NOACTIVATE | SWP_SHOWWINDOW);
        } else {
            const int anchoPanelSolicitado = EscalarDpi(estado->anchoDIP, dpi);
            const int anchoPanelMaximo = std::max(1, anchoVista - EscalarDpi(200, dpi));
            const int anchoPanelMinimo = std::min(EscalarDpi(220, dpi), anchoPanelMaximo);
            const int anchoPanel = std::min(
                std::max(anchoPanelSolicitado, anchoPanelMinimo), anchoPanelMaximo);
            const RECT rectPanel{
                estado->viewOriginal.right - anchoPanel,
                estado->viewOriginal.top,
                estado->viewOriginal.right,
                estado->viewOriginal.bottom
            };
            SetWindowPos(estado->panel, HWND_TOP, rectPanel.left, rectPanel.top,
                         rectPanel.right - rectPanel.left,
                         rectPanel.bottom - rectPanel.top,
                         SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
        InvalidateRect(estado->panel, nullptr, FALSE);
        return;
    }

    int deltaAncho = rectCliente.right - estado->parentCliente.right;
    int deltaAlto = rectCliente.bottom - estado->parentCliente.bottom;
    if (deltaAncho != 0 || deltaAlto != 0) {
        estado->viewOriginal.right += deltaAncho;
        estado->viewOriginal.bottom += deltaAlto;
        estado->parentCliente = rectCliente;
    }

    estado->viewOriginal.left = std::max(0L, estado->viewOriginal.left);
    estado->viewOriginal.top = std::max(0L, estado->viewOriginal.top);
    estado->viewOriginal.right = std::min(rectCliente.right, estado->viewOriginal.right);
    estado->viewOriginal.bottom = std::min(rectCliente.bottom, estado->viewOriginal.bottom);

    int anchoVista = estado->viewOriginal.right - estado->viewOriginal.left;
    int altoVista = estado->viewOriginal.bottom - estado->viewOriginal.top;
    if (anchoVista < 1 || altoVista < 1) return;

    // El comparador es una experiencia temporal a pantalla completa dentro
    // del contenedor de Explorer. Al salir se vuelve al layout lateral normal.
    if (estado->comparadorArchivosActivo) {
        SetWindowPos(estado->view, nullptr, estado->viewOriginal.left,
                     estado->viewOriginal.top, 1, altoVista,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowPos(estado->panel, HWND_TOP, estado->viewOriginal.left,
                     estado->viewOriginal.top, anchoVista, altoVista,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
        InvalidateRect(estado->panel, nullptr, FALSE);
        return;
    }

    int anchoPanelSolicitado = EscalarDpi(estado->anchoDIP, dpi);
    int anchoPanelMaximo = std::max(1, anchoVista - EscalarDpi(200, dpi));
    int anchoPanelMinimo = std::min(EscalarDpi(220, dpi), anchoPanelMaximo);
    int anchoPanel = std::min(std::max(anchoPanelSolicitado, anchoPanelMinimo), anchoPanelMaximo);

    int nuevoAnchoVista = std::max(1, anchoVista - anchoPanel);

    RECT rectPanel = {
        estado->viewOriginal.left + anchoVista - anchoPanel,
        estado->viewOriginal.top,
        estado->viewOriginal.right,
        estado->viewOriginal.bottom
    };

    // Redimensionar la vista nativa de archivos para reservar espacio sin distorsionar DirectUI
    SetWindowPos(estado->view, nullptr, estado->viewOriginal.left, estado->viewOriginal.top,
                 nuevoAnchoVista, altoVista, SWP_NOZORDER | SWP_NOACTIVATE);

    // Posicionar nuestro panel cooperativo a la derecha
    SetWindowPos(estado->panel, HWND_TOP, rectPanel.left, rectPanel.top,
                 rectPanel.right - rectPanel.left, rectPanel.bottom - rectPanel.top,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);

    InvalidateRect(estado->panel, nullptr, FALSE);
}

inline void CerrarEstado(EstadoPanelV2* estado) {
    if (!estado || !estado->abierto) return;
    estado->abierto = false;

    if (estado->panel && IsWindow(estado->panel)) {
        KillTimer(estado->panel, 101);
        ShowWindow(estado->panel, SW_HIDE);
    }

    // El overlay GPU nunca se redimensionó: sólo se ocultó nuestro panel.
    // Restaurarlo aquí provocaría una carrera con el layout del mod GPU.
    if (estado->superficie.gpuAcelerada) return;

    // Restaurar el ancho completo de la vista de archivos
    if (estado->view && IsWindow(estado->view)) {
        int anchoTotal = estado->viewOriginal.right - estado->viewOriginal.left;
        int altoTotal = estado->viewOriginal.bottom - estado->viewOriginal.top;
        SetWindowPos(estado->view, nullptr, estado->viewOriginal.left, estado->viewOriginal.top,
                     anchoTotal, altoTotal, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

inline EstadoPanelV2* AsegurarEstado(HWND top, IShellView** outVista = nullptr) {
    if (outVista) *outVista = nullptr;
    if (!IsWindow(top)) return nullptr;
    AmbitoComV2 com;
    if (!com.Disponible()) return nullptr;

    HWND tab = nullptr;
    IShellView* vistaShell = ObtenerShellViewActivo(top, &tab);
    if (!vistaShell) return nullptr;

    HWND ventanaVista = nullptr;
    vistaShell->GetWindow(&ventanaVista);
    if (!ventanaVista) {
        vistaShell->Release();
        return nullptr;
    }

    auto compatibilidad = FabricaVistas::Dependencias().compatibilidadGpu;
    const SuperficieExploradorV2 superficie = compatibilidad
        ? compatibilidad->Resolver(top, tab, ventanaVista)
        : SuperficieExploradorV2{};
    if (!superficie.EsValida()) {
        vistaShell->Release();
        return nullptr;
    }

    HWND clave = tab ? tab : top;
    EstadoPanelV2* estado = nullptr;

    {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        auto it = g_estadosPorVentana.find(clave);
        if (it != g_estadosPorVentana.end() &&
            it->second && !it->second->destruyendo.load(std::memory_order_acquire)) {
            estado = it->second;
        }
    }

    if (estado && (estado->vistaShell != ventanaVista ||
                   estado->view != superficie.superficieVisual ||
                   estado->parent != superficie.contenedor)) {
        DestruirEstado(estado, true);
        estado = nullptr;
    }

    if (!estado) {
        estado = new EstadoPanelV2();
        estado->top = top;
        estado->tab = clave;
        estado->vistaShell = ventanaVista;
        estado->superficie = superficie;
        estado->view = superficie.superficieVisual;
        estado->parent = superficie.contenedor;
        estado->tema = ObtenerTemaActual();

        estado->viewOriginal = superficie.limites;
        GetClientRect(estado->parent, &estado->parentCliente);

        HINSTANCE hInst = GetModuleHandleW(nullptr);
        estado->panel = CreateWindowExW(
            0, CLASE_PANEL_V2, L"",
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 10, 10, estado->parent, nullptr, hInst, estado);

        if (!estado->panel) {
            delete estado;
            vistaShell->Release();
            return nullptr;
        }

        SetWindowSubclass(estado->parent, ProcedimientoSubclasePadre, 1,
                          reinterpret_cast<DWORD_PTR>(estado));
        if (tab && tab != estado->parent) {
            SetWindowSubclass(tab, ProcedimientoSubclasePadre, 2, reinterpret_cast<DWORD_PTR>(estado));
        }

        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        g_estadosPorVentana[clave] = estado;
    }

    if (outVista) {
        *outVista = vistaShell;
    } else {
        vistaShell->Release();
    }

    return estado;
}

inline void AlternarSideView(HWND top) {
    if (!top || !IsWindow(top)) return;

    HRESULT hrCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool desinicializarCom = (hrCom == S_OK);

    IShellView* vista = nullptr;
    EstadoPanelV2* estado = AsegurarEstado(top, &vista);
    if (!estado) {
        if (vista) vista->Release();
        if (desinicializarCom) CoUninitialize();
        return;
    }

    if (estado->abierto) {
        CerrarEstado(estado);
    } else {
        estado->tema = ObtenerTemaActual();
        estado->abierto = true;

        // Extraer selección de archivos actual de Explorer
        if (vista) {
            std::wstring carpeta;
            auto archivos = ObtenerRutasSeleccionadasDeVista(vista, &carpeta);
            estado->archivosSeleccionados = archivos;
            estado->carpetaContenedora = carpeta;

            // Inyectar dinámicamente la vista correspondiente mediante la Fábrica
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(archivos, carpeta);
            estado->comparadorArchivosActivo = EsComparadorArchivosActivoPorSeleccionV2(archivos);
            if (estado->vistaContenido) estado->vistaContenido->AlActivar(estado->panel);
        } else {
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion({}, L"");
        }

        DisponerPanel(estado);
        SetTimer(estado->panel, 101, 500, nullptr); // Polling moderado para no cargar Explorer
    }

    if (vista) vista->Release();
    if (desinicializarCom) CoUninitialize();
}

inline LRESULT CALLBACK ProcedimientoVentanaPanel(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    EstadoPanelV2* estado = reinterpret_cast<EstadoPanelV2*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (estado && estado->vistaContenido) {
        try {
        if (msg == WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS &&
            estado->comparadorArchivosActivo) {
            estado->vistaContenido->Liberar();
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                estado->archivosSeleccionados, estado->carpetaContenedora, true);
            estado->comparadorArchivosActivo = false;
            if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
            DisponerPanel(estado);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (msg == WM_SIDEVIEW_SETTINGS) {
            if (estado->comparadorArchivosActivo &&
                !EsComparadorArchivosActivoPorSeleccionV2(estado->archivosSeleccionados)) {
                estado->vistaContenido->Liberar();
                estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                    estado->archivosSeleccionados, estado->carpetaContenedora);
                estado->comparadorArchivosActivo = false;
                if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
                DisponerPanel(estado);
            } else if (!estado->comparadorArchivosActivo &&
                       EsComparadorArchivosActivoPorSeleccionV2(estado->archivosSeleccionados)) {
                estado->vistaContenido->Liberar();
                estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                    estado->archivosSeleccionados, estado->carpetaContenedora);
                estado->comparadorArchivosActivo = true;
                if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
                DisponerPanel(estado);
            }
            if (estado->vistaContenido) estado->vistaContenido->ActualizarConfiguracion();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (msg == WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS &&
            !estado->comparadorArchivosActivo &&
            EsComparadorArchivosActivoPorSeleccionV2(estado->archivosSeleccionados)) {
            estado->vistaContenido->Liberar();
            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(
                estado->archivosSeleccionados, estado->carpetaContenedora);
            estado->comparadorArchivosActivo = true;
            if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
            DisponerPanel(estado);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        if (estado->vistaContenido->ProcesarMensajeVentana(msg, wParam, lParam, hwnd)) {
            return 0;
        }
        if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) {
            if (estado->vistaContenido->ProcesarMensajeRaton(msg, wParam, lParam, hwnd)) {
                return 0;
            }
        } else if (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) {
            if (estado->vistaContenido->ProcesarMensajeTeclado(msg, wParam, lParam, hwnd)) {
                return 0;
            }
        }
        } catch (...) {
            Wh_Log(L"[UsefulSideView] Excepción contenida en el procedimiento del panel");
            return 0;
        }
    }

    switch (msg) {
        case WM_NCCREATE: {
            CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            return TRUE;
        }

        case WM_ERASEBKGND:
            return 1; // Manejado completamente por el doble buffer

        case WM_TIMER: {
            IShellView* vistaPendiente = nullptr;
            try {
            if (wParam == 101 && estado && estado->abierto) {
                if (estado->superficie.gpuAcelerada) {
                    // El mod GPU puede rehacer su posición después de un
                    // cambio de pestaña o tamaño. Reubicar sólo nuestro panel
                    // mantiene la cooperación sin tocar su superficie.
                    DisponerPanel(estado);
                }
                HWND frente = GetForegroundWindow();
                if (frente && frente != estado->top && GetAncestor(frente, GA_ROOT) != estado->top) {
                    return 0;
                }
                // Actualizar selección periódicamente si el panel está visible
                AmbitoComV2 com;
                if (!com.Disponible()) return 0;
                HWND tab = nullptr;
                vistaPendiente = ObtenerShellViewActivo(estado->top, &tab);
                if (vistaPendiente) {
                    std::wstring carpeta;
                    auto seleccion = ObtenerRutasSeleccionadasDeVista(vistaPendiente, &carpeta);
                    if (seleccion != estado->archivosSeleccionados || carpeta != estado->carpetaContenedora) {
                        TipoVistaContenido tipoPrevio = ClasificarSeleccion(
                            estado->archivosSeleccionados, estado->carpetaContenedora);
                        TipoVistaContenido tipoNuevo = ClasificarSeleccion(seleccion, carpeta);

                        estado->archivosSeleccionados = seleccion;
                        estado->carpetaContenedora = carpeta;

                        const bool mantenerComparador =
                            estado->comparadorArchivosActivo &&
                            EsComparadorArchivosActivoPorSeleccionV2(seleccion);

                        // Si el comparador sigue recibiendo exactamente dos
                        // videos, sólo renueva sus fuentes; no desmonta la UX.
                        if (mantenerComparador) {
                            estado->vistaContenido->AlCambiarSeleccion(seleccion, carpeta);
                        } else if (estado->comparadorArchivosActivo ||
                                   tipoPrevio != tipoNuevo ||
                                   EsComparadorArchivosActivoPorSeleccionV2(seleccion) !=
                                       estado->comparadorArchivosActivo ||
                                   !estado->vistaContenido) {
                            if (estado->vistaContenido) estado->vistaContenido->Liberar();
                            estado->vistaContenido = FabricaVistas::CrearVistaParaSeleccion(seleccion, carpeta);
                            estado->comparadorArchivosActivo = EsComparadorArchivosActivoPorSeleccionV2(seleccion);
                            if (estado->vistaContenido) estado->vistaContenido->AlActivar(hwnd);
                            DisponerPanel(estado);
                        } else {
                            estado->vistaContenido->AlCambiarSeleccion(seleccion, carpeta);
                        }

                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                    vistaPendiente->Release();
                    vistaPendiente = nullptr;
                }
            }
            } catch (...) {
                if (vistaPendiente) vistaPendiente->Release();
                Wh_Log(L"[UsefulSideView] Excepción contenida durante WM_TIMER");
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdcPantalla = BeginPaint(hwnd, &ps);
            RECT rectCliente;
            GetClientRect(hwnd, &rectCliente);
            int ancho = rectCliente.right - rectCliente.left;
            int alto = rectCliente.bottom - rectCliente.top;

            if (ancho > 0 && alto > 0 && estado) {
                if (AsegurarBufferMemoria(estado, hdcPantalla, ancho, alto)) {
                    if (estado->vistaContenido) {
                        try {
                            estado->vistaContenido->Renderizar(
                                estado->dcMemoria, rectCliente, estado->tema);
                        } catch (...) {
                            Wh_Log(L"[UsefulSideView] Excepción contenida durante WM_PAINT");
                            HBRUSH fondo = CreateSolidBrush(estado->tema.fondo);
                            FillRect(estado->dcMemoria, &rectCliente, fondo);
                            DeleteObject(fondo);
                            SetBkMode(estado->dcMemoria, TRANSPARENT);
                            SetTextColor(estado->dcMemoria, estado->tema.textoSecundario);
                            DrawTextW(estado->dcMemoria,
                                      L"Useful Side View no pudo dibujar esta vista.", -1,
                                      &rectCliente,
                                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                        }
                    }
                    BitBlt(hdcPantalla, 0, 0, ancho, alto, estado->dcMemoria, 0, 0, SRCCOPY);
                }
            }

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY: {
            if (estado) {
                KillTimer(hwnd, 101);
                LiberarBufferMemoria(estado);
            }
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

inline LRESULT CALLBACK ProcedimientoSubclasePadre(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                                  UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    EstadoPanelV2* estado = reinterpret_cast<EstadoPanelV2*>(dwRefData);

    if (msg == WM_SIZE) {
        LRESULT res = DefSubclassProc(hwnd, msg, wParam, lParam);
        if (estado && estado->abierto) {
            DisponerPanel(estado);
        }
        return res;
    }

    if (msg == WM_CLOSE && estado && hwnd == estado->tab && estado->abierto) {
        CerrarEstado(estado);
    }

    // En Explorer con pestañas el contenedor de la pestaña puede destruirse
    // antes que la ventana principal. En Explorer clásico, en cambio, sólo
    // recibimos el ciclo de vida del top-level. Cubrir ambos evita estados
    // colgantes que bloqueaban la segunda ventana de Explorer.
    if (msg == WM_DESTROY && estado && (hwnd == estado->tab || hwnd == estado->top)) {
        DestruirEstado(estado, true);
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    if (msg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, ProcedimientoSubclasePadre, uIdSubclass);
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

inline LRESULT CALLBACK ProcedimientoSubclaseTop(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                                DWORD_PTR) {
    if (msg == WM_SIDEVIEW_TOGGLE) {
        try {
            AlternarSideView(hwnd);
        } catch (...) {
            Wh_Log(L"[UsefulSideView] Excepción contenida al alternar el panel");
        }
        return 0;
    }

    if (msg == WM_SETTINGCHANGE || msg == WM_THEMECHANGED || msg == WM_SYSCOLORCHANGE) {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        for (auto& par : g_estadosPorVentana) {
            if (par.second && par.second->top == hwnd) {
                par.second->tema = ObtenerTemaActual();
                if (par.second->panel && IsWindow(par.second->panel)) {
                    InvalidateRect(par.second->panel, nullptr, FALSE);
                }
            }
        }
    }

    if (msg == WM_NCDESTROY) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hwnd, ProcedimientoSubclaseTop);
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

inline bool RegistrarClasePanelV2(HINSTANCE hInstance) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ProcedimientoVentanaPanel;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = CLASE_PANEL_V2;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }
    return true;
}

} // namespace SideViewV2
