#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/ui/ventana_base.hpp"
#include "v2/nucleo/servicios_vistas.hpp"
#include "v2/nucleo/servicio_com.hpp"
#include "v2/nucleo/configuracion_v2.hpp"
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <algorithm>
#include <atomic>
#include <mutex>

namespace SideViewV2 {

inline std::atomic<int> g_limiteCarpetasPesadasConfig{3};

struct PromptNombreCarpetaV2 {
    HWND ventanaPadre{nullptr};
    HWND campoTexto{nullptr};
    std::wstring resultado;
    bool terminado{false};
};

inline LRESULT CALLBACK ProcedimientoPromptNombreCarpetaV2(HWND hwnd, UINT mensaje,
                                                           WPARAM wParam, LPARAM lParam) {
    auto* estado = reinterpret_cast<PromptNombreCarpetaV2*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (mensaje == WM_NCCREATE) {
        auto* crear = reinterpret_cast<CREATESTRUCTW*>(lParam);
        estado = reinterpret_cast<PromptNombreCarpetaV2*>(crear->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(estado));
    }
    if (!estado) return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    if (mensaje == WM_CREATE) {
        CreateWindowW(L"STATIC", L"Nombre de la carpeta unificada:", WS_CHILD | WS_VISIBLE,
                      14, 12, 280, 20, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
        estado->campoTexto = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Carpeta unificada",
                                             WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                             14, 36, 280, 24, hwnd, reinterpret_cast<HMENU>(101),
                                             GetModuleHandleW(nullptr), nullptr);
        CreateWindowW(L"BUTTON", L"Aceptar", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                      134, 68, 76, 26, hwnd, reinterpret_cast<HMENU>(IDOK),
                      GetModuleHandleW(nullptr), nullptr);
        CreateWindowW(L"BUTTON", L"Cancelar", WS_CHILD | WS_VISIBLE,
                      218, 68, 76, 26, hwnd, reinterpret_cast<HMENU>(IDCANCEL),
                      GetModuleHandleW(nullptr), nullptr);
        SendMessageW(estado->campoTexto, EM_SETSEL, 0, -1);
        SetFocus(estado->campoTexto);
        return 0;
    }
    if (mensaje == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)) {
        if (LOWORD(wParam) == IDOK) {
            wchar_t texto[MAX_PATH]{};
            GetWindowTextW(estado->campoTexto, texto, ARRAYSIZE(texto));
            estado->resultado = texto;
            if (estado->resultado.empty()) return 0;
        }
        estado->terminado = true;
        EnableWindow(estado->ventanaPadre, TRUE);
        DestroyWindow(hwnd);
        return 0;
    }
    if (mensaje == WM_CLOSE) {
        estado->terminado = true;
        EnableWindow(estado->ventanaPadre, TRUE);
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}

inline std::wstring SolicitarNombreCarpetaV2(HWND padre) {
    static std::once_flag registroClase;
    std::call_once(registroClase, [] {
        WNDCLASSW clase{};
        clase.lpfnWndProc = ProcedimientoPromptNombreCarpetaV2;
        clase.hInstance = GetModuleHandleW(nullptr);
        clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        clase.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        clase.lpszClassName = L"UsefulSideView_PromptNombreCarpeta_V2";
        RegisterClassW(&clase);
    });
    PromptNombreCarpetaV2 estado{padre};
    EnableWindow(padre, FALSE);
    HWND ventana = CreateWindowExW(WS_EX_DLGMODALFRAME, L"UsefulSideView_PromptNombreCarpeta_V2",
                                   L"Fusionar carpetas", WS_CAPTION | WS_SYSMENU,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 320, 132, padre, nullptr,
                                   GetModuleHandleW(nullptr), &estado);
    if (!ventana) { EnableWindow(padre, TRUE); return {}; }
    RECT rcPadre{}, rcVentana{};
    GetWindowRect(padre, &rcPadre); GetWindowRect(ventana, &rcVentana);
    SetWindowPos(ventana, nullptr,
                 rcPadre.left + ((rcPadre.right - rcPadre.left) - (rcVentana.right - rcVentana.left)) / 2,
                 rcPadre.top + ((rcPadre.bottom - rcPadre.top) - (rcVentana.bottom - rcVentana.top)) / 2,
                 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(ventana, SW_SHOW); UpdateWindow(ventana);
    MSG mensaje{};
    while (!estado.terminado && GetMessageW(&mensaje, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(ventana, &mensaje)) { TranslateMessage(&mensaje); DispatchMessageW(&mensaje); }
    }
    EnableWindow(padre, TRUE); SetForegroundWindow(padre);
    return estado.resultado;
}

class VistaCarpetas : public IPanelContenido {
public:
    explicit VistaCarpetas(std::shared_ptr<ServicioAbrirConV2> servicio,
                           std::shared_ptr<ServicioNavegacionV2> navegacion,
                           std::shared_ptr<ServicioFusionCarpetasV2> fusion,
                           std::shared_ptr<ServicioInspeccionCarpetasV2> inspeccion,
                           std::shared_ptr<ServicioComparadorVideosV2> comparadorVideos)
        : m_servicio(std::move(servicio)), m_navegacion(std::move(navegacion)),
          m_fusion(std::move(fusion)), m_inspeccion(std::move(inspeccion)),
          m_comparadorVideos(std::move(comparadorVideos)) {
        m_programasRecordados = m_servicio->Programas();
        m_limiteCarpetasPesadas = std::max(1, std::min(g_limiteCarpetasPesadasConfig.load(), 100));
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override {
        m_carpetasSeleccionadas = archivos;
        m_indiceCarpeta = 0;
        m_desplazamientoScrollY = 0;
        m_scrollPesadasY = 0;
        m_nivelExpansionMax = 1; // 1 = Raíz, 2 = Nivel 1, 3 = Nivel 2, 4 = Todo
        m_limiteCarpetasPesadas = std::max(1, std::min(g_limiteCarpetasPesadasConfig.load(), 100));

        CargarCarpetaActual();
    }

    void ActualizarConfiguracion() override {
        int limiteAnterior = m_limiteCarpetasPesadas;
        m_limiteCarpetasPesadas = std::max(1, std::min(g_limiteCarpetasPesadasConfig.load(), 100));
        if (limiteAnterior != m_limiteCarpetasPesadas) CargarCarpetaActual();
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) override {
        POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

        if (msg == WM_MOUSEWHEEL) {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int incremento = (delta > 0 ? -36 : 36);

            // Si el cursor está sobre la caja de carpetas pesadas
            if (PtInRect(&m_rcCajaPesadas, pt)) {
                int totalMostrar = std::min<int>(m_limiteCarpetasPesadas, static_cast<int>(m_topCarpetasPesadas.size()));
                int altoTotalContenido = totalMostrar * 40;
                int altoVisible = m_rcCajaPesadas.bottom - m_rcCajaPesadas.top - 8;
                int maxScrollPesadas = std::max(0, altoTotalContenido - altoVisible);

                int nuevoScroll = std::max(0, std::min(m_scrollPesadasY + incremento, maxScrollPesadas));
                if (nuevoScroll != m_scrollPesadasY) {
                    m_scrollPesadasY = nuevoScroll;
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                }
                return true;
            }

            // Scroll exacto y acotado sobre la estructura de archivos
            int elementosVisibles = 0;
            for (const auto& item : m_elementosArbol) {
                if (item.nivelJerarquia < m_nivelExpansionMax) {
                    elementosVisibles++;
                }
            }

            int altoTotalArbol = elementosVisibles * 22 + 12;
            int altoVisibleArbol = m_rcCajaArbol.bottom - m_rcCajaArbol.top - 12;
            int maxScrollArbol = std::max(0, altoTotalArbol - altoVisibleArbol);

            int nuevoScrollArbol = std::max(0, std::min(m_desplazamientoScrollY + incremento, maxScrollArbol));
            if (nuevoScrollArbol != m_desplazamientoScrollY) {
                m_desplazamientoScrollY = nuevoScrollArbol;
                InvalidateRect(hwndPanel, nullptr, FALSE);
            }
            return true;
        }

        if (msg == WM_LBUTTONDOWN) {
            if (m_fusionEnCurso &&
                (PtInRect(&m_rcBotonFusionCarpetas, pt) || PtInRect(&m_rcBotonEliminarVacias, pt))) {
                return true;
            }
            // Botón Anterior
            if (PtInRect(&m_rcBotonAnterior, pt)) {
                if (m_carpetasSeleccionadas.size() > 1) {
                    m_indiceCarpeta = m_navegacion->Anterior(m_indiceCarpeta, m_carpetasSeleccionadas.size());
                    CargarCarpetaActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Siguiente
            if (PtInRect(&m_rcBotonSiguiente, pt)) {
                if (m_carpetasSeleccionadas.size() > 1) {
                    m_indiceCarpeta = m_navegacion->Siguiente(m_indiceCarpeta, m_carpetasSeleccionadas.size());
                    CargarCarpetaActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Alternar Expansión de Niveles
            if (PtInRect(&m_rcBotonExpansion, pt)) {
                m_nivelExpansionMax = (m_nivelExpansionMax >= 4) ? 1 : (m_nivelExpansionMax + 1);
                m_desplazamientoScrollY = 0;
                InvalidateRect(hwndPanel, nullptr, FALSE);
                return true;
            }
            // Botón Selector de Cantidad de Carpetas Pesadas
            if (PtInRect(&m_rcBotonLimitePesadas, pt)) {
                MostrarMenuLimitePesadas(hwndPanel);
                return true;
            }
            // Selector Desplegable de Programas
            if (PtInRect(&m_rcSelectorPrograma, pt)) {
                MostrarMenuDesplegableProgramas(hwndPanel);
                return true;
            }
            // Botón Ejecutar Abrir con
            if (PtInRect(&m_rcBotonEjecutarAbrir, pt)) {
                EjecutarAbrirCon(hwndPanel);
                return true;
            }
            // Botón Crear Enlace Simbólico
            if (PtInRect(&m_rcBotonCrearEnlace, pt)) {
                CrearEnlaceSimbolico(hwndPanel);
                return true;
            }
            // Botón Copiar Contenido al Portapapeles
            if (PtInRect(&m_rcBotonCopiarContenido, pt)) {
                CopiarListaAlPortapapeles(hwndPanel);
                return true;
            }
            // El comparador sólo se expone en la vista general, sin carpetas
            // seleccionadas. El flujo real queda aislado detrás del servicio.
            if (m_carpetasSeleccionadas.empty() &&
                PtInRect(&m_rcBotonCompararVideos, pt) &&
                ComparadorVideosExperimentalDisponibleV2()) {
                if (m_comparadorVideos &&
                    !m_comparadorVideos->CrearFlujoSimulado().izquierda.nombre.empty()) {
                    PostMessageW(hwndPanel, WM_SIDEVIEW_OPEN_COMPARADOR_VIDEOS, 0, 0);
                }
                return true;
            }
            // Botón Fusionar Carpetas (si hay múltiples seleccionadas)
            if (m_carpetasSeleccionadas.size() > 1 && PtInRect(&m_rcBotonFusionCarpetas, pt)) {
                EjecutarFusionCarpetas(hwndPanel);
                return true;
            }
            // Botón Limpiar Carpetas Vacías
            if (!m_carpetasVaciasDetectadas.empty() && PtInRect(&m_rcBotonEliminarVacias, pt)) {
                EliminarCarpetasVacias(hwndPanel);
                return true;
            }
        }

        return false;
    }

    bool ProcesarMensajeVentana(UINT msg, WPARAM, LPARAM, HWND hwndPanel) override {
        if (msg != ServicioFusionCarpetasV2::WM_FUSION_COMPLETADA) return false;
        m_fusion->FinalizarTrabajo();
        ResultadoFusionCarpetasV2 resultado = m_fusion->UltimoResultado();
        m_fusionEnCurso = false;
        if (resultado.errores == 0 && !m_destinoFusion.empty()) {
            m_carpetasSeleccionadas = {m_destinoFusion};
            m_indiceCarpeta = 0;
            CargarCarpetaActual();
            std::wstring mensaje = L"La fusión terminó correctamente. Se transfirieron " +
                std::to_wstring(resultado.elementosTransferidos) + L" elementos.";
            MessageBoxW(hwndPanel, mensaje.c_str(), L"Fusión completada", MB_OK | MB_ICONINFORMATION);
        } else {
            std::wstring mensaje = L"La fusión terminó con " + std::to_wstring(resultado.errores) +
                L" error(es) y transfirió " + std::to_wstring(resultado.elementosTransferidos) +
                L" elemento(s). Las carpetas de origen se conservaron cuando no fue posible completar una transferencia.";
            MessageBoxW(hwndPanel, mensaje.c_str(), L"Fusión incompleta", MB_OK | MB_ICONWARNING);
        }
        InvalidateRect(hwndPanel, nullptr, FALSE);
        return true;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        int ancho = rect.right - rect.left;
        int alto = rect.bottom - rect.top;
        if (ancho <= 0 || alto <= 0) return;

        // 1. Fondo principal
        HBRUSH brochaFondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, brochaFondo);
        DeleteObject(brochaFondo);

        // 2. Cabecera Fluent
        int altoCabecera = 54;
        RECT rectCabecera{rect.left, rect.top, rect.right, rect.top + altoCabecera};
        HBRUSH brochaCabecera = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &rectCabecera, brochaCabecera);
        DeleteObject(brochaCabecera);

        // Bordes separadores
        HPEN plumaBorde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaBorde));
        MoveToEx(hdc, rect.left, rect.top + altoCabecera, nullptr);
        LineTo(hdc, rect.right, rect.top + altoCabecera);
        MoveToEx(hdc, rect.left, rect.top, nullptr);
        LineTo(hdc, rect.left, rect.bottom);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaBorde);

        SetBkMode(hdc, TRANSPARENT);

        HFONT fuenteTitulo = CreateFontW(-16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT fuenteSubtitulo = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteBold = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuentePequena = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteVieja = static_cast<HFONT>(SelectObject(hdc, fuenteTitulo));

        std::wstring nombreCarpetaActual = L"Sin carpeta seleccionada";
        std::wstring rutaCompletaActual;
        if (!m_carpetasSeleccionadas.empty() && m_indiceCarpeta < m_carpetasSeleccionadas.size()) {
            rutaCompletaActual = m_carpetasSeleccionadas[m_indiceCarpeta];
            const wchar_t* pNom = PathFindFileNameW(rutaCompletaActual.c_str());
            nombreCarpetaActual = pNom ? pNom : rutaCompletaActual;
        }

        // Título de la cabecera
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring strTitulo = L"📁 " + nombreCarpetaActual;
        if (m_carpetasSeleccionadas.size() > 1) {
            strTitulo += L" (" + std::to_wstring(m_indiceCarpeta + 1) + L"/" +
                         std::to_wstring(m_carpetasSeleccionadas.size()) + L")";
        }
        RECT rectTextoTitulo{rect.left + 16, rect.top + 8, rect.right - 90, rect.top + 28};
        DrawTextW(hdc, strTitulo.c_str(), -1, &rectTextoTitulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Subtítulo con peso total
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring strSub = L"Peso total: " + FormatearTamano(m_tamanoTotalBytes) +
                              L" • " + std::to_wstring(m_totalSubcarpetas) + L" carpetas, " +
                              std::to_wstring(m_totalArchivos) + L" archivos";
        RECT rectTextoSub{rect.left + 16, rect.top + 28, rect.right - 90, rect.top + 46};
        DrawTextW(hdc, strSub.c_str(), -1, &rectTextoSub,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Botones de navegación en la cabecera (◀ y ▶)
        int altoBotonesNav = 28;
        int yNav = rect.top + 13;
        m_rcBotonAnterior = {rect.right - 78, yNav, rect.right - 46, yNav + altoBotonesNav};
        m_rcBotonSiguiente = {rect.right - 42, yNav, rect.right - 10, yNav + altoBotonesNav};
        DibujarBoton(hdc, m_rcBotonAnterior, L"◀", tema, m_carpetasSeleccionadas.size() > 1);
        DibujarBoton(hdc, m_rcBotonSiguiente, L"▶", tema, m_carpetasSeleccionadas.size() > 1);

        int margen = 16;
        int yCursor = rect.top + altoCabecera + 14;
        m_rcBotonCompararVideos = {};
        m_rcBotonFusionCarpetas = {};
        m_rcBotonEliminarVacias = {};

        // 3. SECCIÓN ESTRUCTURA DE ARCHIVOS (Árbol Jerárquico)
        SelectObject(hdc, fuentePequena);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rcEtiquetaArbol{rect.left + margen, yCursor, rect.right - 130, yCursor + 18};
        DrawTextW(hdc, L"ESTRUCTURA DE ARCHIVOS (VISTA DE CARPETAS)", -1, &rcEtiquetaArbol,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        // Botón Alternar Expansión
        std::wstring txtExpansion = L"▶ Raíz";
        if (m_nivelExpansionMax == 2) txtExpansion = L"▶ Nivel 1";
        else if (m_nivelExpansionMax == 3) txtExpansion = L"▶ Nivel 2";
        else if (m_nivelExpansionMax >= 4) txtExpansion = L"🔽 Todo";

        m_rcBotonExpansion = {rect.right - margen - 95, yCursor - 2, rect.right - margen, yCursor + 20};
        DibujarBoton(hdc, m_rcBotonExpansion, txtExpansion.c_str(), tema, true);

        yCursor += 24;

        // Contenedor del Árbol (Calculado proporcionalmente)
        int altoCajaArbol = std::max(100, std::min<int>(180, alto - 450));
        m_rcCajaArbol = {rect.left + margen, yCursor, rect.right - margen, yCursor + altoCajaArbol};
        DibujarContenedor(hdc, m_rcCajaArbol, tema);

        // Clamp estricto del scroll para evitar scroll infinito en vacío
        int elementosVisibles = 0;
        for (const auto& item : m_elementosArbol) {
            if (item.nivelJerarquia < m_nivelExpansionMax) {
                elementosVisibles++;
            }
        }
        int altoTotalArbol = elementosVisibles * 22 + 12;
        int altoVisibleArbol = m_rcCajaArbol.bottom - m_rcCajaArbol.top - 12;
        int maxScrollArbol = std::max(0, altoTotalArbol - altoVisibleArbol);
        m_desplazamientoScrollY = std::max(0, std::min(m_desplazamientoScrollY, maxScrollArbol));

        // Recorte y renderizado de filas del árbol
        HRGN rgnClip = CreateRectRgn(m_rcCajaArbol.left + 2, m_rcCajaArbol.top + 2,
                                     m_rcCajaArbol.right - 2, m_rcCajaArbol.bottom - 2);
        SelectClipRgn(hdc, rgnClip);

        int yFila = m_rcCajaArbol.top + 6 - m_desplazamientoScrollY;
        int altoFila = 22;

        if (m_elementosArbol.empty()) {
            SelectObject(hdc, fuenteSubtitulo);
            SetTextColor(hdc, tema.textoSecundario);
            RECT rcVacio = m_rcCajaArbol;
            DrawTextW(hdc, L"(Carpeta vacía o sin subelementos)", -1, &rcVacio,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        } else {
            SelectObject(hdc, fuenteSubtitulo);
            for (const auto& item : m_elementosArbol) {
                if (item.nivelJerarquia < m_nivelExpansionMax) {
                    if (yFila + altoFila >= m_rcCajaArbol.top && yFila <= m_rcCajaArbol.bottom) {
                        int sangria = m_rcCajaArbol.left + 10 + (item.nivelJerarquia * 16);
                        std::wstring icono = item.esDirectorio ? L"📁 " : L"📄 ";
                        std::wstring linea = icono + item.nombre;

                        SetTextColor(hdc, item.esDirectorio ? tema.acento : tema.textoPrincipal);
                        RECT rcNom{sangria, yFila, m_rcCajaArbol.right - 85, yFila + altoFila};
                        DrawTextW(hdc, linea.c_str(), -1, &rcNom,
                                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                        SetTextColor(hdc, tema.textoSecundario);
                        std::wstring strTam = item.esDirectorio ? L"-" : FormatearTamano(item.tamanoBytes);
                        RECT rcTam{m_rcCajaArbol.right - 80, yFila, m_rcCajaArbol.right - 10, yFila + altoFila};
                        DrawTextW(hdc, strTam.c_str(), -1, &rcTam,
                                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                    }
                    yFila += altoFila;
                }
            }
        }

        SelectClipRgn(hdc, nullptr);
        DeleteObject(rgnClip);

        yCursor += altoCajaArbol + 12;

        // 4. SECCIÓN ABRIR CARPETA CON
        SelectObject(hdc, fuentePequena);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rcEtiquetaAbrir{rect.left + margen, yCursor, rect.right - margen, yCursor + 16};
        DrawTextW(hdc, L"ABRIR CARPETA CON", -1, &rcEtiquetaAbrir, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yCursor += 20;

        int anchoBotonAbrir = 80;
        int anchoSelector = (rect.right - rect.left) - (margen * 2) - anchoBotonAbrir - 10;
        m_rcSelectorPrograma = {rect.left + margen, yCursor, rect.left + margen + anchoSelector, yCursor + 30};
        m_rcBotonEjecutarAbrir = {m_rcSelectorPrograma.right + 10, yCursor, rect.right - margen, yCursor + 30};

        std::wstring seleccionado = m_servicio->ProgramaActual();
        std::wstring strProg = seleccionado.empty() ? L"(click para seleccionar programa)" : PathFindFileNameW(seleccionado.c_str());
        DibujarBoton(hdc, m_rcSelectorPrograma, (strProg + L"  ▾").c_str(), tema, true);

        // Botón Verde / Fluent Abrir
        HBRUSH brAbrir = CreateSolidBrush(RGB(46, 160, 67));
        FillRect(hdc, &m_rcBotonEjecutarAbrir, brAbrir);
        DeleteObject(brAbrir);
        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, RGB(255, 255, 255));
        RECT rcTxtAbrir = m_rcBotonEjecutarAbrir;
        DrawTextW(hdc, L"Abrir", -1, &rcTxtAbrir, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yCursor += 40;

        // 5. SECCIÓN ACCIONES RÁPIDAS
        SelectObject(hdc, fuentePequena);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rcEtiquetaAcciones{rect.left + margen, yCursor, rect.right - margen, yCursor + 16};
        DrawTextW(hdc, L"ACCIONES RÁPIDAS", -1, &rcEtiquetaAcciones, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yCursor += 20;

        int anchoMitad = ((rect.right - rect.left) - (margen * 2) - 8) / 2;
        m_rcBotonCrearEnlace = {rect.left + margen, yCursor, rect.left + margen + anchoMitad, yCursor + 30};
        m_rcBotonCopiarContenido = {m_rcBotonCrearEnlace.right + 8, yCursor, rect.right - margen, yCursor + 30};

        DibujarBoton(hdc, m_rcBotonCrearEnlace, L"🔗 Crear enlace", tema, true);
        DibujarBoton(hdc, m_rcBotonCopiarContenido, L"📋 Copiar contenido", tema, true);

        yCursor += 38;

        // Activador experimental: sólo existe en el estado general de
        // carpetas. Al desactivarlo desaparece también su zona de interacción.
        if (m_carpetasSeleccionadas.empty() &&
            ComparadorVideosExperimentalDisponibleV2()) {
            m_rcBotonCompararVideos = {rect.left + margen, yCursor,
                                       rect.right - margen, yCursor + 32};
            DibujarBoton(hdc, m_rcBotonCompararVideos,
                         L"Comparador de videos · experimental", tema, true);
            yCursor += 40;
        }

        // 6. OPERACIONES CON CARPETA (Fusión o Eliminación de vacías)
        if (m_carpetasSeleccionadas.size() > 1) {
            SelectObject(hdc, fuentePequena);
            SetTextColor(hdc, tema.textoSecundario);
            RECT rcEtiquetaOp{rect.left + margen, yCursor, rect.right - margen, yCursor + 16};
            DrawTextW(hdc, L"OPERACIONES CON MÚLTIPLES CARPETAS", -1, &rcEtiquetaOp, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            yCursor += 20;
            m_rcBotonFusionCarpetas = {rect.left + margen, yCursor, rect.right - margen, yCursor + 32};
            DibujarBoton(hdc, m_rcBotonFusionCarpetas, L"🔀 Fusionar carpetas seleccionadas...", tema, true);
            yCursor += 40;
        } else if (!m_carpetasVaciasDetectadas.empty()) {
            SelectObject(hdc, fuentePequena);
            SetTextColor(hdc, RGB(220, 100, 30));
            RECT rcEtiquetaVacias{rect.left + margen, yCursor, rect.right - margen, yCursor + 16};
            std::wstring strVacias = L"CARPETAS VACÍAS DETECTADAS (" + std::to_wstring(m_carpetasVaciasDetectadas.size()) + L")";
            DrawTextW(hdc, strVacias.c_str(), -1, &rcEtiquetaVacias, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            yCursor += 20;
            m_rcBotonEliminarVacias = {rect.left + margen, yCursor, rect.right - margen, yCursor + 30};
            DibujarBoton(hdc, m_rcBotonEliminarVacias, L"🗑️ Eliminar carpetas vacías", tema, true);
            yCursor += 38;
        }

        // 7. SUBCARPETAS MÁS PESADAS (Con límite configurable y scroll independiente)
        if (!m_topCarpetasPesadas.empty()) {
            SelectObject(hdc, fuentePequena);
            SetTextColor(hdc, tema.textoSecundario);
            RECT rcEtiquetaTop{rect.left + margen, yCursor, rect.right - 100, yCursor + 16};
            DrawTextW(hdc, L"SUBCARPETAS MÁS PESADAS", -1, &rcEtiquetaTop, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            // Botón selector de cantidad: "Mostrar (N) ▾"
            std::wstring strBotonLimite = L"Mostrar (" + std::to_wstring(m_limiteCarpetasPesadas) + L") ▾";
            m_rcBotonLimitePesadas = {rect.right - margen - 95, yCursor - 2, rect.right - margen, yCursor + 18};
            DibujarBoton(hdc, m_rcBotonLimitePesadas, strBotonLimite.c_str(), tema, true);

            yCursor += 22;

            int altoDisponiblePesadas = rect.bottom - margen - yCursor;
            int altoCajaPesadas = std::max(60, altoDisponiblePesadas);
            m_rcCajaPesadas = {rect.left + margen, yCursor, rect.right - margen, yCursor + altoCajaPesadas};

            int totalMostrar = std::min<int>(m_limiteCarpetasPesadas, static_cast<int>(m_topCarpetasPesadas.size()));
            int altoTotalContenidoPesadas = totalMostrar * 40;
            int altoVisiblePesadas = m_rcCajaPesadas.bottom - m_rcCajaPesadas.top - 8;
            int maxScrollPesadas = std::max(0, altoTotalContenidoPesadas - altoVisiblePesadas);
            m_scrollPesadasY = std::max(0, std::min(m_scrollPesadasY, maxScrollPesadas));

            HRGN rgnClipPesadas = CreateRectRgn(m_rcCajaPesadas.left, m_rcCajaPesadas.top,
                                                m_rcCajaPesadas.right, m_rcCajaPesadas.bottom);
            SelectClipRgn(hdc, rgnClipPesadas);

            int yPesada = m_rcCajaPesadas.top + 2 - m_scrollPesadasY;
            int altoTarjetaPesada = 34;

            for (int i = 0; i < totalMostrar; ++i) {
                if (yPesada + altoTarjetaPesada >= m_rcCajaPesadas.top && yPesada <= m_rcCajaPesadas.bottom) {
                    RECT rcPesada{m_rcCajaPesadas.left, yPesada, m_rcCajaPesadas.right, yPesada + altoTarjetaPesada};
                    DibujarContenedor(hdc, rcPesada, tema);

                    SelectObject(hdc, fuenteSubtitulo);
                    SetTextColor(hdc, tema.textoPrincipal);
                    RECT rcNomP{rcPesada.left + 10, rcPesada.top + 2, rcPesada.right - 90, rcPesada.top + 18};
                    DrawTextW(hdc, m_topCarpetasPesadas[i].nombre.c_str(), -1, &rcNomP,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                    SetTextColor(hdc, tema.acento);
                    RECT rcTamP{rcPesada.right - 85, rcPesada.top + 2, rcPesada.right - 10, rcPesada.top + 18};
                    std::wstring strPesadaTam = FormatearTamano(m_topCarpetasPesadas[i].tamanoBytes);
                    DrawTextW(hdc, strPesadaTam.c_str(), -1, &rcTamP,
                              DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                    // Barra de progreso de peso
                    RECT rcBarraFondo{rcPesada.left + 10, rcPesada.top + 22, rcPesada.right - 10, rcPesada.top + 26};
                    HBRUSH brBkgBar = CreateSolidBrush(tema.fondoCabecera);
                    FillRect(hdc, &rcBarraFondo, brBkgBar);
                    DeleteObject(brBkgBar);

                    int anchoLleno = static_cast<int>((rcBarraFondo.right - rcBarraFondo.left) * (m_topCarpetasPesadas[i].porcentaje / 100.0));
                    RECT rcBarraProg{rcBarraFondo.left, rcBarraFondo.top, rcBarraFondo.left + anchoLleno, rcBarraFondo.bottom};
                    HBRUSH brFillBar = CreateSolidBrush(tema.acento);
                    FillRect(hdc, &rcBarraProg, brFillBar);
                    DeleteObject(brFillBar);
                }
                yPesada += altoTarjetaPesada + 6;
            }

            SelectClipRgn(hdc, nullptr);
            DeleteObject(rgnClipPesadas);
        }

        SelectObject(hdc, fuenteVieja);
        DeleteObject(fuenteTitulo);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteBold);
        DeleteObject(fuentePequena);
    }

private:
    void CargarCarpetaActual() {
        m_elementosArbol.clear();
        m_tamanoTotalBytes = 0;
        m_totalSubcarpetas = 0;
        m_totalArchivos = 0;
        m_topCarpetasPesadas.clear();
        m_carpetasVaciasDetectadas.clear();

        if (m_carpetasSeleccionadas.empty() || m_indiceCarpeta >= m_carpetasSeleccionadas.size()) return;

        const std::wstring& ruta = m_carpetasSeleccionadas[m_indiceCarpeta];
        ResultadoInspeccionCarpetaV2 resultado = m_inspeccion->Ejecutar(ruta, static_cast<size_t>(m_limiteCarpetasPesadas));
        m_elementosArbol = std::move(resultado.elementos);
        m_topCarpetasPesadas = std::move(resultado.pesadas);
        m_carpetasVaciasDetectadas = std::move(resultado.vacias);
        m_tamanoTotalBytes = resultado.tamanoTotalBytes;
        m_totalSubcarpetas = resultado.totalSubcarpetas;
        m_totalArchivos = resultado.totalArchivos;
    }

    void DibujarBoton(HDC hdc, const RECT& rc, const wchar_t* texto, const TemaVisual& tema, bool habilitado) {
        HBRUSH br = CreateSolidBrush(habilitado ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &rc, br);
        DeleteObject(br);

        HPEN pen = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        SetTextColor(hdc, habilitado ? tema.textoPrincipal : tema.textoSecundario);
        RECT rcTexto = rc;
        DrawTextW(hdc, texto, -1, &rcTexto, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
    }

    void DibujarContenedor(HDC hdc, const RECT& rc, const TemaVisual& tema) {
        HBRUSH br = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rc, br);
        DeleteObject(br);

        HPEN pen = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    std::wstring FormatearTamano(uintmax_t bytes) {
        if (bytes < 1024) return std::to_wstring(bytes) + L" B";
        if (bytes < 1024 * 1024) {
            wchar_t b[32];
            swprintf_s(b, L"%.1f KB", static_cast<double>(bytes) / 1024.0);
            return b;
        }
        if (bytes < 1024ULL * 1024 * 1024) {
            wchar_t b[32];
            swprintf_s(b, L"%.1f MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
            return b;
        }
        wchar_t b[32];
        swprintf_s(b, L"%.2f GB", static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0));
        return b;
    }

    void MostrarMenuLimitePesadas(HWND hwndPadre) {
        HMENU hMenu = CreatePopupMenu();
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas == 3 ? MF_CHECKED : 0) | MF_STRING, 1, L"Mostrar 3 carpetas (Por defecto)");
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas == 5 ? MF_CHECKED : 0) | MF_STRING, 2, L"Mostrar 5 carpetas");
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas == 10 ? MF_CHECKED : 0) | MF_STRING, 3, L"Mostrar 10 carpetas");
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas == 20 ? MF_CHECKED : 0) | MF_STRING, 4, L"Mostrar 20 carpetas");
        AppendMenuW(hMenu, (m_limiteCarpetasPesadas >= 999 ? MF_CHECKED : 0) | MF_STRING, 5, L"Mostrar todas");

        POINT ptMenu{m_rcBotonLimitePesadas.left, m_rcBotonLimitePesadas.bottom};
        ClientToScreen(hwndPadre, &ptMenu);
        int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, ptMenu.x, ptMenu.y, 0, hwndPadre, nullptr);
        DestroyMenu(hMenu);

        if (cmd == 1) m_limiteCarpetasPesadas = 3;
        else if (cmd == 2) m_limiteCarpetasPesadas = 5;
        else if (cmd == 3) m_limiteCarpetasPesadas = 10;
        else if (cmd == 4) m_limiteCarpetasPesadas = 20;
        else if (cmd == 5) m_limiteCarpetasPesadas = 999;

        if (cmd > 0) {
            g_limiteCarpetasPesadasConfig.store(m_limiteCarpetasPesadas);
            m_scrollPesadasY = 0;
            InvalidateRect(hwndPadre, nullptr, FALSE);
        }
    }

    void MostrarMenuDesplegableProgramas(HWND hwndPadre) {
        m_programasRecordados = m_servicio->Programas();
        HMENU hMenu = CreatePopupMenu();
        for (size_t i = 0; i < m_programasRecordados.size(); ++i) {
            AppendMenuW(hMenu, MF_STRING, i + 1, m_programasRecordados[i].c_str());
        }
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, 100, L"Examinar otro ejecutable...");

        POINT ptMenu{m_rcSelectorPrograma.left, m_rcSelectorPrograma.bottom};
        ClientToScreen(hwndPadre, &ptMenu);
        int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, ptMenu.x, ptMenu.y, 0, hwndPadre, nullptr);
        DestroyMenu(hMenu);

        if (cmd == 100) {
            if (m_servicio->AbrirDialogo(hwndPadre)) {
                m_programasRecordados = m_servicio->Programas();
                InvalidateRect(hwndPadre, nullptr, FALSE);
            }
        } else if (cmd > 0 && cmd <= static_cast<int>(m_programasRecordados.size())) {
            m_servicio->Seleccionar(m_programasRecordados[cmd - 1]);
            InvalidateRect(hwndPadre, nullptr, FALSE);
        }
    }

    void EjecutarAbrirCon(HWND hwndPadre) {
        std::wstring programa = m_servicio->ProgramaActual();
        if (programa.empty()) {
            MostrarMenuDesplegableProgramas(hwndPadre);
            programa = m_servicio->ProgramaActual();
            if (programa.empty()) return;
        }
        m_servicio->Seleccionar(programa);
        m_servicio->Ejecutar(m_carpetasSeleccionadas);
    }

    void CopiarListaAlPortapapeles(HWND hwndPadre) {
        if (m_carpetasSeleccionadas.empty() || m_indiceCarpeta >= m_carpetasSeleccionadas.size()) return;
        const std::wstring& ruta = m_carpetasSeleccionadas[m_indiceCarpeta];

        std::wstring textoAcumulado = L"Contenido de: " + ruta + L"\r\n\r\n";
        for (const auto& item : m_elementosArbol) {
            std::wstring sangria(item.nivelJerarquia * 2, L' ');
            textoAcumulado += sangria + (item.esDirectorio ? L"[CARPETA] " : L"  ") + item.nombre;
            if (!item.esDirectorio) {
                textoAcumulado += L" (" + FormatearTamano(item.tamanoBytes) + L")";
            }
            textoAcumulado += L"\r\n";
        }

        if (OpenClipboard(hwndPadre)) {
            bool copiado = EmptyClipboard();
            size_t bytes = (textoAcumulado.size() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = copiado ? GlobalAlloc(GMEM_MOVEABLE, bytes) : nullptr;
            if (copiado && !hMem) copiado = false;
            if (hMem) {
                void* pMem = GlobalLock(hMem);
                if (pMem) {
                    memcpy(pMem, textoAcumulado.c_str(), bytes);
                    GlobalUnlock(hMem);
                    copiado = SetClipboardData(CF_UNICODETEXT, hMem) != nullptr;
                    if (!copiado) GlobalFree(hMem);
                } else {
                    GlobalFree(hMem);
                    copiado = false;
                }
            }
            CloseClipboard();
            MessageBoxW(hwndPadre, copiado ? L"¡Estructura de la carpeta copiada al portapapeles!" : L"No fue posible copiar la estructura al portapapeles.", L"Useful Side View", MB_OK | (copiado ? MB_ICONINFORMATION : MB_ICONWARNING));
        }
    }

    void CrearEnlaceSimbolico(HWND hwndPadre) {
        if (m_carpetasSeleccionadas.empty() || m_indiceCarpeta >= m_carpetasSeleccionadas.size()) return;
        AmbitoComV2 com;
        if (!com.Disponible()) return;
        const std::wstring& rutaOrigen = m_carpetasSeleccionadas[m_indiceCarpeta];

        wchar_t rutaDestino[MAX_PATH] = L"";
        OPENFILENAMEW ofn{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwndPadre;
        ofn.lpstrFilter = L"Accesos Directos (*.lnk)\0*.lnk\0Todos los archivos (*.*)\0*.*\0";
        ofn.lpstrFile = rutaDestino;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = L"Crear enlace o acceso directo hacia esta carpeta";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

        if (GetSaveFileNameW(&ofn)) {
            std::wstring destinoStr = rutaDestino;
            if (destinoStr.size() < 4 || _wcsicmp(destinoStr.substr(destinoStr.size() - 4).c_str(), L".lnk") != 0) {
                destinoStr += L".lnk";
            }

            IShellLinkW* psl = nullptr;
            if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void**>(&psl))) &&
                SUCCEEDED(psl->SetPath(rutaOrigen.c_str()))) {
                IPersistFile* ppf = nullptr;
                if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf))) && ppf) {
                    HRESULT guardado = ppf->Save(destinoStr.c_str(), TRUE);
                    ppf->Release();
                    if (SUCCEEDED(guardado)) {
                    MessageBoxW(hwndPadre, L"Enlace creado exitosamente.", L"Useful Side View", MB_OK | MB_ICONINFORMATION);
                    }
                }
                psl->Release();
            }
        }
    }

    void EjecutarFusionCarpetas(HWND hwndPadre) {
        if (m_carpetasSeleccionadas.size() < 2) return;
        if (m_fusionEnCurso) return;
        std::wstring confirmacion = L"Se fusionarán " + std::to_wstring(m_carpetasSeleccionadas.size()) +
            L" carpetas.\n\nLa operación combinará todo su contenido en una sola carpeta y eliminará las carpetas de origen cuando la transferencia termine correctamente.\n\n¿Deseas continuar?";
        if (MessageBoxW(hwndPadre, confirmacion.c_str(), L"Useful Side View - Fusión", MB_YESNO | MB_ICONQUESTION) != IDYES) return;

        wchar_t padreBuffer[MAX_PATH]{};
        wcsncpy_s(padreBuffer, m_carpetasSeleccionadas.front().c_str(), _TRUNCATE);
        PathRemoveFileSpecW(padreBuffer);
        std::wstring nombre = SolicitarNombreCarpetaV2(hwndPadre);
        if (nombre.empty()) return;
        if (nombre.find_first_of(L"\\/:*?\"<>|") != std::wstring::npos || nombre == L"." || nombre == L"..") {
            MessageBoxW(hwndPadre, L"El nombre contiene caracteres no válidos.", L"Fusión no válida", MB_OK | MB_ICONWARNING);
            return;
        }
        std::wstring destino = std::wstring(padreBuffer) + L"\\" + nombre;
        for (const auto& origen : m_carpetasSeleccionadas) {
            std::wstring prefijo = origen;
            if (!prefijo.empty() && prefijo.back() != L'\\') prefijo += L'\\';
            if (_wcsicmp(destino.c_str(), origen.c_str()) == 0 ||
                (destino.size() > prefijo.size() && _wcsnicmp(destino.c_str(), prefijo.c_str(), prefijo.size()) == 0)) {
                MessageBoxW(hwndPadre, L"La carpeta destino no puede ser una carpeta de origen ni estar dentro de ella.", L"Fusión no válida", MB_OK | MB_ICONWARNING);
                return;
            }
        }

        DWORD atributosDestino = GetFileAttributesW(destino.c_str());
        if (atributosDestino != INVALID_FILE_ATTRIBUTES && !(atributosDestino & FILE_ATTRIBUTE_DIRECTORY)) {
            MessageBoxW(hwndPadre, L"La ruta elegida ya corresponde a un archivo.", L"Fusión no válida", MB_OK | MB_ICONWARNING);
            return;
        }
        if (atributosDestino == INVALID_FILE_ATTRIBUTES && !CreateDirectoryW(destino.c_str(), nullptr)) {
            MessageBoxW(hwndPadre, L"No fue posible crear la carpeta unificada.", L"Fusión fallida", MB_OK | MB_ICONERROR);
            return;
        }

        m_fusionEnCurso = true;
        m_destinoFusion = destino;
        if (!m_fusion->EjecutarEnSegundoPlano(m_carpetasSeleccionadas, destino, hwndPadre)) {
            m_fusionEnCurso = false;
            MessageBoxW(hwndPadre, L"No fue posible iniciar la fusión en segundo plano.", L"Fusión fallida", MB_OK | MB_ICONERROR);
            return;
        }
        MessageBoxW(hwndPadre,
            L"La fusión se ejecutará en segundo plano para evitar congelar Explorer.\n\nPuedes seguir usando la ventana; el contenido se actualizará al finalizar.",
            L"Fusión iniciada", MB_OK | MB_ICONINFORMATION);
    }

    void EliminarCarpetasVacias(HWND hwndPadre) {
        int eliminadas = 0;
        for (const auto& rVacia : m_carpetasVaciasDetectadas) {
            if (RemoveDirectoryW(rVacia.c_str())) {
                eliminadas++;
            }
        }
        m_carpetasVaciasDetectadas.clear();
        CargarCarpetaActual();
        InvalidateRect(hwndPadre, nullptr, FALSE);
        std::wstring msg = L"Se eliminaron " + std::to_wstring(eliminadas) + L" carpetas vacías.";
        MessageBoxW(hwndPadre, msg.c_str(), L"Useful Side View - Limpieza", MB_OK | MB_ICONINFORMATION);
    }

    std::vector<std::wstring> m_carpetasSeleccionadas;
    size_t m_indiceCarpeta{0};
    int m_desplazamientoScrollY{0};
    int m_scrollPesadasY{0};
    int m_nivelExpansionMax{1};
    int m_limiteCarpetasPesadas{3};

    uintmax_t m_tamanoTotalBytes{0};
    size_t m_totalSubcarpetas{0};
    size_t m_totalArchivos{0};

    std::vector<ItemArbolCarpetaV2> m_elementosArbol;
    std::vector<CarpetaPesadaV2> m_topCarpetasPesadas;
    std::vector<std::wstring> m_carpetasVaciasDetectadas;

    std::vector<std::wstring> m_programasRecordados;
    std::shared_ptr<ServicioAbrirConV2> m_servicio;
    std::shared_ptr<ServicioNavegacionV2> m_navegacion;
    std::shared_ptr<ServicioFusionCarpetasV2> m_fusion;
    std::shared_ptr<ServicioInspeccionCarpetasV2> m_inspeccion;
    std::shared_ptr<ServicioComparadorVideosV2> m_comparadorVideos;
    bool m_fusionEnCurso{false};
    std::wstring m_destinoFusion;

    RECT m_rcBotonAnterior{};
    RECT m_rcBotonSiguiente{};
    RECT m_rcBotonExpansion{};
    RECT m_rcCajaArbol{};
    RECT m_rcSelectorPrograma{};
    RECT m_rcBotonEjecutarAbrir{};
    RECT m_rcBotonCrearEnlace{};
    RECT m_rcBotonCopiarContenido{};
    RECT m_rcBotonCompararVideos{};
    RECT m_rcBotonFusionCarpetas{};
    RECT m_rcBotonEliminarVacias{};
    RECT m_rcBotonLimitePesadas{};
    RECT m_rcCajaPesadas{};
};

} // namespace SideViewV2
