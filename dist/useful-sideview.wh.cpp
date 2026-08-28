/**** start inlining useful-sideview.cpp ****/
// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Adds a lightweight video player to the Windows 11 File Explorer preview pane
// @version         0.1.0
// @author          Reyr
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lmfplay -lmfplat -lmfuuid -lshlwapi -lgdi32 -luuid
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Useful Side View

Turns the Windows 11 File Explorer preview pane into a responsive video player.
Sound and autoplay are disabled by default. Supported formats depend on the
Media Foundation codecs installed in Windows.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- autoplay: false
  $name: Autoplay videos
- sound: false
  $name: Enable sound
*/
// ==/WindhawkModSettings==

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mfapi.h>
#include <mfplay.h>
#include <exdisp.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <shlwapi.h>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>

namespace {

// El orden documenta las dependencias y permite producir una sola unidad de
// traducción, requisito del formato distribuible de Windhawk.
/**** start inlining nucleo/constantes.hpp ****/

constexpr wchar_t CLASE_VENTANA_REPRODUCTOR[] = L"WindhawkUsefulSideView";
constexpr wchar_t CLASE_VENTANA_VIDEO[] = L"WindhawkUsefulSideViewVideo";
constexpr UINT_PTR ID_TEMPORIZADOR_INTERFAZ = 1;
constexpr UINT INTERVALO_REFRESCO_MS = 200;
constexpr int ALTO_CONTROLES = 42;
constexpr int ANCHO_BOTON_REPRODUCIR = 52;
constexpr int ANCHO_BOTON_SONIDO = 52;
constexpr UINT MENSAJE_ACTUALIZAR_INTERFAZ = WM_APP + 1;
/**** ended inlining nucleo/constantes.hpp ****/
/**** start inlining nucleo/configuracion.hpp ****/

struct Configuracion {
    // Windhawk notifica cambios desde otro hilo. Los atómicos evitan bloquear
    // el camino crítico del reproductor y eliminan carreras de datos.
    std::atomic_bool reproduccionAutomatica{false};
    std::atomic_bool sonidoHabilitado{false};
};

void cargarConfiguracion();
/**** ended inlining nucleo/configuracion.hpp ****/
/**** start inlining nucleo/estado.hpp ****/

Configuracion configuracion;
std::wstring rutaVideoActiva;
std::atomic_bool detenerHilo{false};
HANDLE hiloReproductor = nullptr;
HWND ventanaReproductor = nullptr;
HWND ventanaVideo = nullptr;
HWND ventanaHost = nullptr;
IMFPMediaPlayer* reproductor = nullptr;

class ReceptorEventosMediaFoundation;
ReceptorEventosMediaFoundation* receptorEventos = nullptr;
/**** ended inlining nucleo/estado.hpp ****/
/**** start inlining archivos/seleccion_explorador.cpp ****/
bool esRutaDeVideo(const wchar_t* ruta) {
    if (!ruta || !*ruta || PathIsDirectoryW(ruta)) return false;
    const wchar_t* extension = PathFindExtensionW(ruta);
    static constexpr const wchar_t* EXTENSIONES_VIDEO[] = {
        L".mp4", L".m4v", L".mov", L".wmv", L".avi", L".mkv",
        L".webm", L".mpg", L".mpeg", L".mts", L".m2ts", L".3gp"};
    for (const wchar_t* extensionVideo : EXTENSIONES_VIDEO) {
        if (_wcsicmp(extension, extensionVideo) == 0) return true;
    }
    return false;
}

std::wstring obtenerVideoSeleccionado() {
    IShellWindows* ventanasShell = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&ventanasShell)))) {
        return {};
    }

    std::wstring rutaSeleccionada;
    long cantidad = 0;
    ventanasShell->get_Count(&cantidad);
    for (long indice = 0; indice < cantidad && rutaSeleccionada.empty(); ++indice) {
        VARIANT posicion;
        VariantInit(&posicion);
        posicion.vt = VT_I4;
        posicion.lVal = indice;
        IDispatch* despacho = nullptr;
        if (FAILED(ventanasShell->Item(posicion, &despacho)) || !despacho) continue;

        IWebBrowserApp* navegador = nullptr;
        if (SUCCEEDED(despacho->QueryInterface(IID_PPV_ARGS(&navegador)))) {
            SHANDLE_PTR identificadorVentana = 0;
            navegador->get_HWND(&identificadorVentana);
            HWND candidata = reinterpret_cast<HWND>(identificadorVentana);
            DWORD idProceso = 0;
            GetWindowThreadProcessId(candidata, &idProceso);

            // Cada instancia inyectada atiende únicamente sus propias ventanas
            // de Explorer, incluso cuando ShellWindows enumera todas globalmente.
            if (idProceso == GetCurrentProcessId() && IsWindowVisible(candidata)) {
                IServiceProvider* proveedor = nullptr;
                if (SUCCEEDED(navegador->QueryInterface(IID_PPV_ARGS(&proveedor)))) {
                    IShellBrowser* navegadorShell = nullptr;
                    if (SUCCEEDED(proveedor->QueryService(
                            SID_STopLevelBrowser, IID_PPV_ARGS(&navegadorShell)))) {
                        IShellView* vistaShell = nullptr;
                        if (SUCCEEDED(navegadorShell->QueryActiveShellView(&vistaShell))) {
                            IFolderView2* vistaCarpeta = nullptr;
                            if (SUCCEEDED(vistaShell->QueryInterface(
                                    IID_PPV_ARGS(&vistaCarpeta)))) {
                                IShellItemArray* seleccion = nullptr;
                                if (SUCCEEDED(vistaCarpeta->GetSelection(
                                        TRUE, &seleccion)) && seleccion) {
                                    DWORD elementos = 0;
                                    seleccion->GetCount(&elementos);
                                    if (elementos == 1) {
                                        IShellItem* elemento = nullptr;
                                        if (SUCCEEDED(seleccion->GetItemAt(0, &elemento))) {
                                            PWSTR ruta = nullptr;
                                            if (SUCCEEDED(elemento->GetDisplayName(
                                                    SIGDN_FILESYSPATH, &ruta)) &&
                                                esRutaDeVideo(ruta)) {
                                                rutaSeleccionada = ruta;
                                                ventanaHost = candidata;
                                            }
                                            CoTaskMemFree(ruta);
                                            elemento->Release();
                                        }
                                    }
                                    seleccion->Release();
                                }
                                vistaCarpeta->Release();
                            }
                            vistaShell->Release();
                        }
                        navegadorShell->Release();
                    }
                    proveedor->Release();
                }
            }
            navegador->Release();
        }
        despacho->Release();
    }
    ventanasShell->Release();
    return rutaSeleccionada;
}
/**** ended inlining archivos/seleccion_explorador.cpp ****/
/**** start inlining reproductor/eventos_media_foundation.cpp ****/
class ReceptorEventosMediaFoundation final : public IMFPMediaPlayerCallback {
    std::atomic<ULONG> referencias_{1};

  public:
    STDMETHODIMP QueryInterface(REFIID interfaz, void** resultado) override {
        if (!resultado) return E_POINTER;
        if (interfaz == IID_IUnknown ||
            interfaz == __uuidof(IMFPMediaPlayerCallback)) {
            *resultado = static_cast<IMFPMediaPlayerCallback*>(this);
            AddRef();
            return S_OK;
        }
        *resultado = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return ++referencias_; }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG restantes = --referencias_;
        if (!restantes) delete this;
        return restantes;
    }

    void STDMETHODCALLTYPE OnMediaPlayerEvent(MFP_EVENT_HEADER* evento) override {
        if (!evento) return;

        // Media Foundation crea el elemento de forma asíncrona. Solo este
        // evento garantiza que ya puede asignarse al reproductor.
        if (evento->eEventType == MFP_EVENT_TYPE_MEDIAITEM_CREATED &&
            SUCCEEDED(evento->hrEvent) && reproductor) {
            auto eventoCreacion =
                reinterpret_cast<MFP_MEDIAITEM_CREATED_EVENT*>(evento);
            reproductor->SetMediaItem(eventoCreacion->pMediaItem);
        }
        if (evento->eEventType == MFP_EVENT_TYPE_MEDIAITEM_SET &&
            SUCCEEDED(evento->hrEvent) && reproductor) {
            reproductor->SetMute(!configuracion.sonidoHabilitado.load());
            if (configuracion.reproduccionAutomatica.load()) reproductor->Play();
        }
        if (ventanaReproductor) {
            PostMessageW(ventanaReproductor, MENSAJE_ACTUALIZAR_INTERFAZ, 0, 0);
        }
    }
};
/**** ended inlining reproductor/eventos_media_foundation.cpp ****/
/**** start inlining reproductor/controles.cpp ****/
void alternarReproduccion() {
    if (!reproductor) return;
    MFP_MEDIAPLAYER_STATE estado;
    if (SUCCEEDED(reproductor->GetState(&estado)) &&
        estado == MFP_MEDIAPLAYER_STATE_PLAYING) {
        reproductor->Pause();
    } else {
        reproductor->Play();
    }
    InvalidateRect(ventanaReproductor, nullptr, FALSE);
}

void alternarSonido() {
    if (!reproductor) return;
    BOOL silenciado = TRUE;
    reproductor->GetMute(&silenciado);
    reproductor->SetMute(!silenciado);
    InvalidateRect(ventanaReproductor, nullptr, FALSE);
}

void buscarDesdeCoordenada(int posicionX) {
    if (!reproductor || !ventanaReproductor) return;
    RECT areaCliente;
    GetClientRect(ventanaReproductor, &areaCliente);
    int inicio = ANCHO_BOTON_REPRODUCIR + 10;
    int fin = areaCliente.right - ANCHO_BOTON_SONIDO - 10;
    if (fin <= inicio) return;

    double proporcion = static_cast<double>(
                            std::max(inicio, std::min(posicionX, fin)) - inicio) /
                        static_cast<double>(fin - inicio);
    PROPVARIANT duracion;
    PropVariantInit(&duracion);
    if (SUCCEEDED(reproductor->GetDuration(MFP_POSITIONTYPE_100NS, &duracion)) &&
        duracion.vt == VT_I8) {
        PROPVARIANT posicion;
        PropVariantInit(&posicion);
        posicion.vt = VT_I8;
        posicion.hVal.QuadPart =
            static_cast<LONGLONG>(duracion.hVal.QuadPart * proporcion);
        reproductor->SetPosition(MFP_POSITIONTYPE_100NS, &posicion);
    }
    PropVariantClear(&duracion);
}
/**** ended inlining reproductor/controles.cpp ****/
/**** start inlining interfaz/renderizado.cpp ****/
void dibujarInterfaz(HWND ventana) {
    PAINTSTRUCT pintura;
    HDC contexto = BeginPaint(ventana, &pintura);
    RECT areaCliente;
    GetClientRect(ventana, &areaCliente);
    RECT barra{0, std::max<LONG>(0, areaCliente.bottom - ALTO_CONTROLES),
               areaCliente.right, areaCliente.bottom};

    HBRUSH fondo = CreateSolidBrush(RGB(25, 25, 25));
    FillRect(contexto, &barra, fondo);
    DeleteObject(fondo);
    SetBkMode(contexto, TRANSPARENT);
    SetTextColor(contexto, RGB(245, 245, 245));

    MFP_MEDIAPLAYER_STATE estado = MFP_MEDIAPLAYER_STATE_EMPTY;
    BOOL silenciado = TRUE;
    if (reproductor) {
        reproductor->GetState(&estado);
        reproductor->GetMute(&silenciado);
    }

    RECT botonReproducir{0, barra.top, ANCHO_BOTON_REPRODUCIR, barra.bottom};
    DrawTextW(contexto,
              estado == MFP_MEDIAPLAYER_STATE_PLAYING ? L"Pause" : L"Play", -1,
              &botonReproducir, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    RECT botonSonido{areaCliente.right - ANCHO_BOTON_SONIDO, barra.top,
                     areaCliente.right, barra.bottom};
    DrawTextW(contexto, silenciado ? L"Muted" : L"Sound", -1, &botonSonido,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    int inicio = ANCHO_BOTON_REPRODUCIR + 10;
    int fin = areaCliente.right - ANCHO_BOTON_SONIDO - 10;
    int centroY = barra.top + ALTO_CONTROLES / 2;
    HPEN lineaBase = CreatePen(PS_SOLID, 3, RGB(100, 100, 100));
    HPEN lapizAnterior = static_cast<HPEN>(SelectObject(contexto, lineaBase));
    MoveToEx(contexto, inicio, centroY, nullptr);
    LineTo(contexto, fin, centroY);
    SelectObject(contexto, lapizAnterior);
    DeleteObject(lineaBase);

    if (reproductor) {
        PROPVARIANT posicion;
        PROPVARIANT duracion;
        PropVariantInit(&posicion);
        PropVariantInit(&duracion);
        if (SUCCEEDED(reproductor->GetPosition(MFP_POSITIONTYPE_100NS, &posicion)) &&
            SUCCEEDED(reproductor->GetDuration(MFP_POSITIONTYPE_100NS, &duracion)) &&
            posicion.vt == VT_I8 && duracion.vt == VT_I8 &&
            duracion.hVal.QuadPart > 0) {
            int progreso = inicio + static_cast<int>(
                (fin - inicio) * static_cast<double>(posicion.hVal.QuadPart) /
                duracion.hVal.QuadPart);
            HPEN lineaProgreso = CreatePen(PS_SOLID, 4, RGB(0, 120, 215));
            lapizAnterior =
                static_cast<HPEN>(SelectObject(contexto, lineaProgreso));
            MoveToEx(contexto, inicio, centroY, nullptr);
            LineTo(contexto, progreso, centroY);
            SelectObject(contexto, lapizAnterior);
            DeleteObject(lineaProgreso);
        }
        PropVariantClear(&posicion);
        PropVariantClear(&duracion);
    }
    EndPaint(ventana, &pintura);
}
/**** ended inlining interfaz/renderizado.cpp ****/
/**** start inlining interfaz/ventanas.cpp ****/
LRESULT CALLBACK procesarVentanaReproductor(HWND ventana, UINT mensaje,
                                             WPARAM wParam, LPARAM lParam) {
    switch (mensaje) {
        case WM_LBUTTONDOWN: {
            RECT areaCliente;
            GetClientRect(ventana, &areaCliente);
            int posicionX = GET_X_LPARAM(lParam);
            int posicionY = GET_Y_LPARAM(lParam);
            if (posicionY >= areaCliente.bottom - ALTO_CONTROLES) {
                if (posicionX < ANCHO_BOTON_REPRODUCIR) {
                    alternarReproduccion();
                } else if (posicionX >=
                           areaCliente.right - ANCHO_BOTON_SONIDO) {
                    alternarSonido();
                } else {
                    buscarDesdeCoordenada(posicionX);
                    SetCapture(ventana);
                }
            }
            return 0;
        }
        case WM_MOUSEMOVE:
            if (GetCapture() == ventana) {
                buscarDesdeCoordenada(GET_X_LPARAM(lParam));
            }
            return 0;
        case WM_LBUTTONUP:
            if (GetCapture() == ventana) ReleaseCapture();
            return 0;
        case WM_SIZE:
            if (ventanaVideo) {
                RECT areaCliente;
                GetClientRect(ventana, &areaCliente);
                SetWindowPos(ventanaVideo, nullptr, 0, 0, areaCliente.right,
                             std::max<LONG>(0, areaCliente.bottom - ALTO_CONTROLES),
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
            if (reproductor) reproductor->UpdateVideo();
            return 0;
        case WM_TIMER:
        case MENSAJE_ACTUALIZAR_INTERFAZ:
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            dibujarInterfaz(ventana);
            return 0;
    }
    return DefWindowProcW(ventana, mensaje, wParam, lParam);
}

LRESULT CALLBACK procesarVentanaVideo(HWND ventana, UINT mensaje,
                                      WPARAM wParam, LPARAM lParam) {
    if (mensaje == WM_LBUTTONDOWN) {
        alternarReproduccion();
        return 0;
    }
    if (mensaje == WM_ERASEBKGND) {
        RECT areaCliente;
        GetClientRect(ventana, &areaCliente);
        FillRect(reinterpret_cast<HDC>(wParam), &areaCliente,
                 static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        return 1;
    }
    return DefWindowProcW(ventana, mensaje, wParam, lParam);
}

bool registrarClasesDeVentana() {
    WNDCLASSEXW clase{sizeof(clase)};
    clase.lpfnWndProc = procesarVentanaReproductor;
    clase.hInstance = GetModuleHandleW(nullptr);
    clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    clase.lpszClassName = CLASE_VENTANA_REPRODUCTOR;
    if (!RegisterClassExW(&clase) &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    clase.lpfnWndProc = procesarVentanaVideo;
    clase.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    clase.lpszClassName = CLASE_VENTANA_VIDEO;
    return RegisterClassExW(&clase) ||
           GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

void eliminarClasesDeVentana() {
    UnregisterClassW(CLASE_VENTANA_REPRODUCTOR, GetModuleHandleW(nullptr));
    UnregisterClassW(CLASE_VENTANA_VIDEO, GetModuleHandleW(nullptr));
}
/**** ended inlining interfaz/ventanas.cpp ****/
/**** start inlining host/panel_previsualizacion.cpp ****/
BOOL CALLBACK encontrarVistaDeArchivos(HWND ventana, LPARAM resultado) {
    wchar_t clase[64];
    if (GetClassNameW(ventana, clase, ARRAYSIZE(clase)) &&
        lstrcmpW(clase, L"SHELLDLL_DefView") == 0) {
        *reinterpret_cast<HWND*>(resultado) = ventana;
        return FALSE;
    }
    return TRUE;
}

bool obtenerAreaPanel(RECT* areaPanel) {
    if (!areaPanel || !ventanaHost || !IsWindow(ventanaHost)) return false;
    HWND vistaArchivos = nullptr;
    EnumChildWindows(ventanaHost, encontrarVistaDeArchivos,
                     reinterpret_cast<LPARAM>(&vistaArchivos));
    if (!vistaArchivos) return false;

    RECT areaHost;
    RECT areaVistaPantalla;
    GetClientRect(ventanaHost, &areaHost);
    GetWindowRect(vistaArchivos, &areaVistaPantalla);
    POINT esquinaVista{areaVistaPantalla.right, areaVistaPantalla.top};
    ScreenToClient(ventanaHost, &esquinaVista);

    // El panel abierto contrae SHELLDLL_DefView y deja una franja amplia a su
    // derecha. Sin panel, esa franja no supera el umbral de seguridad.
    constexpr int ANCHO_MINIMO_PANEL = 180;
    int ancho = areaHost.right - esquinaVista.x;
    if (ancho < ANCHO_MINIMO_PANEL) return false;
    *areaPanel = {esquinaVista.x, esquinaVista.y, areaHost.right,
                  areaHost.bottom};
    return true;
}

bool ajustarReproductorAlPanel() {
    if (!ventanaReproductor) return false;
    RECT areaPanel;
    if (!obtenerAreaPanel(&areaPanel)) {
        ShowWindow(ventanaReproductor, SW_HIDE);
        return false;
    }
    SetWindowPos(ventanaReproductor, HWND_TOP, areaPanel.left, areaPanel.top,
                 areaPanel.right - areaPanel.left,
                 areaPanel.bottom - areaPanel.top,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);
    return true;
}

bool asegurarReproductor(const std::wstring& ruta) {
    RECT areaPanel;
    if (!obtenerAreaPanel(&areaPanel)) return false;

    // El decodificador solo se crea cuando coinciden video seleccionado y
    // panel visible; las demás ventanas de Explorer no pagan este costo.
    if (!ventanaReproductor) {
        ventanaReproductor = CreateWindowExW(
            0, CLASE_VENTANA_REPRODUCTOR, L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 0,
            0, ventanaHost, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!ventanaReproductor) return false;
        ventanaVideo = CreateWindowExW(
            0, CLASE_VENTANA_VIDEO, L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
            ventanaReproductor, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!ventanaVideo) return false;
        SetTimer(ventanaReproductor, ID_TEMPORIZADOR_INTERFAZ,
                 INTERVALO_REFRESCO_MS, nullptr);
        receptorEventos = new ReceptorEventosMediaFoundation();
        if (FAILED(MFPCreateMediaPlayer(nullptr, FALSE, 0, receptorEventos,
                                        ventanaVideo, &reproductor))) {
            return false;
        }
    }

    ajustarReproductorAlPanel();
    reproductor->Stop();
    reproductor->SetMute(!configuracion.sonidoHabilitado.load());
    return SUCCEEDED(
        reproductor->CreateMediaItemFromURL(ruta.c_str(), FALSE, 0, nullptr));
}
/**** ended inlining host/panel_previsualizacion.cpp ****/
/**** start inlining runtime/hilo_reproductor.cpp ****/
void procesarMensajesDelHilo() {
    MSG mensaje;
    while (PeekMessageW(&mensaje, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&mensaje);
        DispatchMessageW(&mensaje);
    }
}

void liberarRecursosDelReproductor() {
    if (reproductor) {
        reproductor->Shutdown();
        reproductor->Release();
        reproductor = nullptr;
    }
    if (receptorEventos) {
        receptorEventos->Release();
        receptorEventos = nullptr;
    }
    if (ventanaReproductor) {
        DestroyWindow(ventanaReproductor);
        ventanaReproductor = nullptr;
        ventanaVideo = nullptr;
    }
}

DWORD WINAPI ejecutarHiloReproductor(void*) {
    HRESULT resultadoCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(resultadoCom)) return 1;
    if (FAILED(MFStartup(MF_VERSION))) {
        CoUninitialize();
        return 1;
    }

    while (!detenerHilo.load()) {
        // Solo este hilo interactúa con IMFPMediaPlayer. Centralizar su acceso
        // simplifica la sincronización con COM y con Media Foundation.
        if (reproductor) {
            reproductor->SetMute(!configuracion.sonidoHabilitado.load());
        }

        std::wstring rutaSeleccionada = obtenerVideoSeleccionado();
        if (!rutaSeleccionada.empty() && rutaSeleccionada != rutaVideoActiva &&
            asegurarReproductor(rutaSeleccionada)) {
            rutaVideoActiva = rutaSeleccionada;
        }
        if (rutaSeleccionada.empty()) {
            rutaVideoActiva.clear();
            if (ventanaReproductor) ShowWindow(ventanaReproductor, SW_HIDE);
        } else if (ventanaReproductor) {
            ajustarReproductorAlPanel();
        }

        MsgWaitForMultipleObjects(0, nullptr, FALSE, 100, QS_ALLINPUT);
        procesarMensajesDelHilo();
    }

    liberarRecursosDelReproductor();
    MFShutdown();
    CoUninitialize();
    return 0;
}
/**** ended inlining runtime/hilo_reproductor.cpp ****/

}  // namespace

/**** start inlining windhawk/ciclo_vida.cpp ****/
namespace {

void cargarConfiguracion() {
    configuracion.reproduccionAutomatica.store(
        Wh_GetIntSetting(L"autoplay") != 0);
    configuracion.sonidoHabilitado.store(Wh_GetIntSetting(L"sound") != 0);
}

}  // namespace

BOOL Wh_ModInit() {
    cargarConfiguracion();
    if (!registrarClasesDeVentana()) return FALSE;
    detenerHilo.store(false);
    hiloReproductor =
        CreateThread(nullptr, 0, ejecutarHiloReproductor, nullptr, 0, nullptr);
    if (!hiloReproductor) {
        eliminarClasesDeVentana();
        return FALSE;
    }
    return TRUE;
}

void Wh_ModSettingsChanged() {
    // No se toca IMFPMediaPlayer aquí: el hilo propietario observará los
    // valores atómicos durante su siguiente iteración.
    cargarConfiguracion();
}

void Wh_ModUninit() {
    detenerHilo.store(true);
    if (hiloReproductor) {
        WaitForSingleObject(hiloReproductor, 3000);
        CloseHandle(hiloReproductor);
        hiloReproductor = nullptr;
    }
    eliminarClasesDeVentana();
}
/**** ended inlining windhawk/ciclo_vida.cpp ****/
/**** ended inlining useful-sideview.cpp ****/
