/**** start inlining useful-sideview.cpp ****/
// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Adds a lightweight video player to the Windows 11 File Explorer preview pane
// @version         0.2.1
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
  $name: Reproducción automática
- sound: false
  $name: Audio habilitado por defecto
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
#include <new>
#include <string>

namespace {

// El orden documenta las dependencias y permite producir una sola unidad de
// traducción, requisito del formato distribuible de Windhawk.
//
// controles.cpp define buscarEnReproduccion y la caché de tiempo.
// ventanas.cpp define el delegado del hilo reproductor (HWND_MESSAGE).
// panel_previsualizacion.cpp crea las ventanas visibles.
// hilo_reproductor.cpp posee IMFPMediaPlayer y actualiza la caché.
/**** start inlining nucleo/constantes.hpp ****/

constexpr wchar_t CLASE_VENTANA_REPRODUCTOR[] = L"WindhawkUsefulSideView";
constexpr wchar_t CLASE_VENTANA_VIDEO[] = L"WindhawkUsefulSideViewVideo";
constexpr wchar_t CLASE_VENTANA_CONTROLES[] = L"WindhawkUsefulSideViewControls";
constexpr wchar_t CLASE_VENTANA_DELEGADO[] = L"WindhawkUsefulSideViewDelegate";
constexpr UINT_PTR ID_TEMPORIZADOR_INTERFAZ = 1;
constexpr UINT INTERVALO_REFRESCO_MS = 200;
constexpr int ALTO_CONTROLES = 96;
constexpr int TAMANO_BOTON_CONTROL = 36;
constexpr int SEPARACION_BOTONES = 12;
constexpr int ANCHO_GRUPO_BOTONES =
    TAMANO_BOTON_CONTROL * 3 + SEPARACION_BOTONES * 2;
constexpr int MARGEN_BARRA_PROGRESO = 16;
constexpr int RADIO_PERILLA_PROGRESO = 6;
constexpr double PASO_BARRA_PROGRESO_TECLADO = 0.05;
constexpr UINT MENSAJE_ACTUALIZAR_INTERFAZ = WM_APP + 1;
constexpr UINT MENSAJE_ACTUALIZAR_CACHE_TIEMPO = WM_APP + 2;
constexpr UINT MENSAJE_EJECUTAR_BUSQUEDA = WM_APP + 3;

constexpr wchar_t GLIFO_REPRODUCIR[] = L"\uE768";
constexpr wchar_t GLIFO_PAUSAR[] = L"\uE769";
constexpr wchar_t GLIFO_VOLUMEN[] = L"\uE767";
constexpr wchar_t GLIFO_SILENCIO[] = L"\uE74F";
constexpr wchar_t GLIFO_REPETICION[] = L"\uE8EE";
/**** ended inlining nucleo/constantes.hpp ****/
/**** start inlining nucleo/configuracion.hpp ****/

struct Configuracion {
    // Windhawk notifica cambios desde otro hilo. Los atómicos evitan bloquear
    // el camino crítico del reproductor y eliminan carreras de datos.
    std::atomic_bool reproduccionAutomatica{false};
    std::atomic_bool sonidoHabilitado{false};
};
/**** ended inlining nucleo/configuracion.hpp ****/
/**** start inlining nucleo/estado.hpp ****/

Configuracion configuracion;
std::wstring rutaVideoActiva;
std::atomic_bool detenerHilo{false};
std::atomic_bool configuracionPendiente{true};
HANDLE hiloReproductor = nullptr;
HANDLE eventoDetener = nullptr;
HWND ventanaReproductor = nullptr;
HWND ventanaVideo = nullptr;
HWND ventanaControles = nullptr;
HWND ventanaDelegadoReproductor = nullptr;
HWND ventanaExploradorAnfitriona = nullptr;
IMFPMediaPlayer* reproductor = nullptr;
HFONT fuenteIconos = nullptr;
HFONT fuenteTexto = nullptr;

class ReceptorEventosMediaFoundation;
ReceptorEventosMediaFoundation* receptorEventos = nullptr;

// La barra pertenece al mismo hilo que su ventana, por lo que estas variables
// no necesitan atómicos ni sincronización adicional.
bool arrastrandoBarraProgreso = false;
double proporcionProgreso = 0.0;
std::atomic_bool cambioPosicionPendiente{false};
std::atomic_bool reproducirTrasCambioPosicion{false};
std::atomic_bool posicionamientoDisponible{false};
std::atomic_bool repeticionHabilitada{false};
std::atomic<double> proporcionPosicionSolicitada{0.0};
std::atomic<long long> posicionReproduccionCache{0};
std::atomic<long long> duracionReproduccionCache{0};
std::atomic_bool tiempoReproduccionDisponible{false};
/**** ended inlining nucleo/estado.hpp ****/
/**** start inlining nucleo/puntero_com.hpp ****/

// Administra una referencia COM y garantiza Release al salir del alcance.
// La clase expresa propiedad sin incorporar otro framework al proceso Explorer.
template <typename TipoInterfaz>
class PunteroCom {
  public:
    PunteroCom() = default;
    PunteroCom(const PunteroCom&) = delete;
    PunteroCom& operator=(const PunteroCom&) = delete;

    ~PunteroCom() { liberar(); }

    TipoInterfaz* obtener() const { return interfaz_; }
    TipoInterfaz* operator->() const { return interfaz_; }
    explicit operator bool() const { return interfaz_ != nullptr; }

    TipoInterfaz** recibir() {
        liberar();
        return &interfaz_;
    }

    void liberar() {
        if (!interfaz_) return;
        interfaz_->Release();
        interfaz_ = nullptr;
    }

  private:
    TipoInterfaz* interfaz_ = nullptr;
};
/**** ended inlining nucleo/puntero_com.hpp ****/
/**** start inlining archivos/seleccion_explorador.cpp ****/
bool esRutaDeVideo(const wchar_t* rutaArchivo) {
    if (!rutaArchivo || !*rutaArchivo || PathIsDirectoryW(rutaArchivo)) {
        return false;
    }

    const wchar_t* extensionArchivo = PathFindExtensionW(rutaArchivo);
    static constexpr const wchar_t* EXTENSIONES_VIDEO[] = {
        L".mp4", L".m4v", L".mov", L".wmv", L".avi", L".mkv",
        L".webm", L".mpg", L".mpeg", L".mts", L".m2ts", L".3gp"};
    for (const wchar_t* extensionVideo : EXTENSIONES_VIDEO) {
        if (_wcsicmp(extensionArchivo, extensionVideo) == 0) return true;
    }
    return false;
}

std::wstring obtenerRutaVideoDeSeleccion(IWebBrowserApp* explorador) {
    PunteroCom<IServiceProvider> proveedorServicios;
    if (FAILED(explorador->QueryInterface(
            IID_PPV_ARGS(proveedorServicios.recibir())))) {
        return {};
    }

    PunteroCom<IShellBrowser> navegadorCarpetas;
    if (FAILED(proveedorServicios->QueryService(
            SID_STopLevelBrowser,
            IID_PPV_ARGS(navegadorCarpetas.recibir())))) {
        return {};
    }

    PunteroCom<IShellView> vistaActiva;
    if (FAILED(navegadorCarpetas->QueryActiveShellView(
            vistaActiva.recibir()))) {
        return {};
    }

    PunteroCom<IFolderView2> vistaCarpeta;
    if (FAILED(vistaActiva->QueryInterface(
            IID_PPV_ARGS(vistaCarpeta.recibir())))) {
        return {};
    }

    PunteroCom<IShellItemArray> elementosSeleccionados;
    if (FAILED(vistaCarpeta->GetSelection(
            TRUE, elementosSeleccionados.recibir()))) {
        return {};
    }

    DWORD cantidadElementosSeleccionados = 0;
    if (FAILED(elementosSeleccionados->GetCount(
            &cantidadElementosSeleccionados)) ||
        cantidadElementosSeleccionados != 1) {
        return {};
    }

    PunteroCom<IShellItem> elementoSeleccionado;
    if (FAILED(elementosSeleccionados->GetItemAt(
            0, elementoSeleccionado.recibir()))) {
        return {};
    }

    PWSTR rutaElementoSeleccionado = nullptr;
    if (FAILED(elementoSeleccionado->GetDisplayName(
            SIGDN_FILESYSPATH, &rutaElementoSeleccionado))) {
        return {};
    }

    std::wstring rutaVideo;
    if (esRutaDeVideo(rutaElementoSeleccionado)) {
        rutaVideo = rutaElementoSeleccionado;
    }
    CoTaskMemFree(rutaElementoSeleccionado);
    return rutaVideo;
}

std::wstring obtenerVideoSeleccionado(IShellWindows* ventanasShell) {
    if (!ventanasShell) return {};

    long cantidadVentanasShell = 0;
    if (FAILED(ventanasShell->get_Count(&cantidadVentanasShell))) return {};

    // IShellWindows solo expone Item mediante una posición numérica. El índice
    // queda confinado a este recorrido y nombra explícitamente qué representa.
    for (long posicionVentanaShell = 0;
         posicionVentanaShell < cantidadVentanasShell;
         ++posicionVentanaShell) {
        VARIANT identificadorVentanaShell;
        VariantInit(&identificadorVentanaShell);
        identificadorVentanaShell.vt = VT_I4;
        identificadorVentanaShell.lVal = posicionVentanaShell;

        PunteroCom<IDispatch> despachoVentana;
        if (FAILED(ventanasShell->Item(
                identificadorVentanaShell, despachoVentana.recibir()))) {
            continue;
        }

        PunteroCom<IWebBrowserApp> explorador;
        if (FAILED(despachoVentana->QueryInterface(
                IID_PPV_ARGS(explorador.recibir())))) {
            continue;
        }

        SHANDLE_PTR identificadorVentana = 0;
        if (FAILED(explorador->get_HWND(&identificadorVentana))) continue;

        HWND ventanaExplorador =
            reinterpret_cast<HWND>(identificadorVentana);
        DWORD identificadorProceso = 0;
        GetWindowThreadProcessId(ventanaExplorador, &identificadorProceso);

        // Cada instancia inyectada atiende únicamente sus propias ventanas,
        // aunque ShellWindows enumere ventanas de otros procesos Explorer.
        if (identificadorProceso != GetCurrentProcessId() ||
            !IsWindowVisible(ventanaExplorador)) {
            continue;
        }

        std::wstring rutaVideo =
            obtenerRutaVideoDeSeleccion(explorador.obtener());
        if (!rutaVideo.empty()) {
            ventanaExploradorAnfitriona = ventanaExplorador;
            return rutaVideo;
        }
    }
    return {};
}
/**** ended inlining archivos/seleccion_explorador.cpp ****/
/**** start inlining interfaz/diseno_barra_progreso.hpp ****/

struct DisenoBarraProgreso {
    RECT barraControles{};
    RECT areaInteractiva{};
    RECT botonReproduccion{};
    RECT botonSonido{};
    RECT botonRepeticion{};
    int limiteIzquierdo = 0;
    int limiteDerecho = 0;
    int centroVertical = 0;
};

inline int calcularInicioGrupoBotones(int anchoCliente) {
    return std::max(0, (anchoCliente - ANCHO_GRUPO_BOTONES) / 2);
}

inline double limitarProporcionProgreso(double proporcion) {
    return std::max(0.0, std::min(proporcion, 1.0));
}

inline double calcularProporcionProgreso(
    const DisenoBarraProgreso& disenoBarraProgreso,
    int posicionHorizontal) {
    if (disenoBarraProgreso.limiteDerecho <=
        disenoBarraProgreso.limiteIzquierdo) {
        return 0.0;
    }
    int posicionLimitada = std::max(
        disenoBarraProgreso.limiteIzquierdo,
        std::min(posicionHorizontal, disenoBarraProgreso.limiteDerecho));
    return static_cast<double>(posicionLimitada -
                               disenoBarraProgreso.limiteIzquierdo) /
           static_cast<double>(disenoBarraProgreso.limiteDerecho -
                               disenoBarraProgreso.limiteIzquierdo);
}

inline int calcularPosicionProgreso(
    const DisenoBarraProgreso& disenoBarraProgreso, double proporcion) {
    return disenoBarraProgreso.limiteIzquierdo + static_cast<int>(
        (disenoBarraProgreso.limiteDerecho -
         disenoBarraProgreso.limiteIzquierdo) *
        limitarProporcionProgreso(proporcion));
}

inline bool contienePunto(const RECT& rectangulo, int posicionHorizontal,
                          int posicionVertical) {
    POINT punto{posicionHorizontal, posicionVertical};
    return PtInRect(&rectangulo, punto) != FALSE;
}

inline bool esClicEnBarraProgreso(
    const DisenoBarraProgreso& disenoBarraProgreso, int posicionHorizontal,
    int posicionVertical) {
    if (!contienePunto(disenoBarraProgreso.barraControles, posicionHorizontal,
                       posicionVertical)) {
        return false;
    }
    if (contienePunto(disenoBarraProgreso.botonReproduccion, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonSonido, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonRepeticion, posicionHorizontal,
                      posicionVertical)) {
        return false;
    }
    return posicionHorizontal >= disenoBarraProgreso.limiteIzquierdo &&
           posicionHorizontal <= disenoBarraProgreso.limiteDerecho &&
           posicionVertical < disenoBarraProgreso.botonReproduccion.top;
}

inline DisenoBarraProgreso calcularDisenoBarraProgreso(HWND ventana) {
    RECT areaCliente;
    GetClientRect(ventana, &areaCliente);
    DisenoBarraProgreso disenoBarraProgreso;
    disenoBarraProgreso.barraControles = areaCliente;
    disenoBarraProgreso.limiteIzquierdo = MARGEN_BARRA_PROGRESO;
    disenoBarraProgreso.limiteDerecho = static_cast<int>(std::max<LONG>(
        disenoBarraProgreso.limiteIzquierdo,
        areaCliente.right - MARGEN_BARRA_PROGRESO));
    disenoBarraProgreso.centroVertical = 34;
    int inicioGrupoBotones = calcularInicioGrupoBotones(areaCliente.right);
    int bordeSuperiorBotones = 54;
    disenoBarraProgreso.areaInteractiva = {
        disenoBarraProgreso.limiteIzquierdo, 5,
        disenoBarraProgreso.limiteDerecho, bordeSuperiorBotones - 4};
    disenoBarraProgreso.botonReproduccion = {
        inicioGrupoBotones, bordeSuperiorBotones,
        inicioGrupoBotones + TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonSonido = {
        disenoBarraProgreso.botonReproduccion.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonReproduccion.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonRepeticion = {
        disenoBarraProgreso.botonSonido.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonSonido.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    return disenoBarraProgreso;
}
/**** ended inlining interfaz/diseno_barra_progreso.hpp ****/
/**** start inlining reproductor/calculos_tiempo.hpp ****/

inline LONGLONG calcularPosicionTemporal(LONGLONG duracionTotal,
                                         double proporcionObjetivo) {
    if (duracionTotal <= 0) return 0;
    return static_cast<LONGLONG>(
        static_cast<double>(duracionTotal) *
        limitarProporcionProgreso(proporcionObjetivo));
}
/**** ended inlining reproductor/calculos_tiempo.hpp ****/
/**** start inlining reproductor/conversion_tiempo_media_foundation.hpp ****/

#include <propidl.h>
#include <limits>

inline bool extraerTiempoNoNegativoMediaFoundation(
    const PROPVARIANT& propiedadTiempo, LONGLONG* tiempoExtraido) {
    if (!tiempoExtraido) return false;

    ULONGLONG tiempoSinSigno = 0;
    if (propiedadTiempo.vt == VT_UI8) {
        tiempoSinSigno = propiedadTiempo.uhVal.QuadPart;
    } else if (propiedadTiempo.vt == VT_I8 &&
               propiedadTiempo.hVal.QuadPart >= 0) {
        tiempoSinSigno =
            static_cast<ULONGLONG>(propiedadTiempo.hVal.QuadPart);
    } else {
        return false;
    }

    constexpr ULONGLONG MAYOR_TIEMPO_REPRESENTABLE =
        static_cast<ULONGLONG>(std::numeric_limits<LONGLONG>::max());
    if (tiempoSinSigno > MAYOR_TIEMPO_REPRESENTABLE) return false;

    *tiempoExtraido = static_cast<LONGLONG>(tiempoSinSigno);
    return true;
}

inline bool extraerDuracionMediaFoundation(
    const PROPVARIANT& propiedadDuracion, LONGLONG* duracionExtraida) {
    return extraerTiempoNoNegativoMediaFoundation(propiedadDuracion,
                                                   duracionExtraida);
}

inline bool extraerPosicionMediaFoundation(
    const PROPVARIANT& propiedadPosicion, LONGLONG* posicionExtraida) {
    return extraerTiempoNoNegativoMediaFoundation(propiedadPosicion,
                                                   posicionExtraida);
}
/**** ended inlining reproductor/conversion_tiempo_media_foundation.hpp ****/
/**** start inlining reproductor/controles.cpp ****/
void reiniciarCacheTiempoReproduccion() {
    posicionReproduccionCache.store(0);
    duracionReproduccionCache.store(0);
    tiempoReproduccionDisponible.store(false);
    cambioPosicionPendiente.store(false);
    posicionamientoDisponible.store(false);
}

void invalidarInterfazControles() {
    if (ventanaControles) {
        InvalidateRect(ventanaControles, nullptr, FALSE);
    }
}

// Solo válido en el hilo reproductor: ahí vive IMFPMediaPlayer y COM responde
// correctamente a GetPosition y GetDuration.
bool leerTiempoDesdeReproductor(LONGLONG* posicionActual,
                                LONGLONG* duracionTotal) {
    if (!reproductor || !posicionActual || !duracionTotal) return false;

    PROPVARIANT propiedadPosicion;
    PROPVARIANT propiedadDuracion;
    PropVariantInit(&propiedadPosicion);
    PropVariantInit(&propiedadDuracion);
    HRESULT resultadoPosicion = reproductor->GetPosition(
        MFP_POSITIONTYPE_100NS, &propiedadPosicion);
    HRESULT resultadoDuracion = reproductor->GetDuration(
        MFP_POSITIONTYPE_100NS, &propiedadDuracion);
    LONGLONG posicionExtraida = 0;
    LONGLONG duracionExtraida = 0;
    bool tiempoDisponible =
        SUCCEEDED(resultadoPosicion) && SUCCEEDED(resultadoDuracion) &&
        extraerPosicionMediaFoundation(propiedadPosicion,
                                        &posicionExtraida) &&
        extraerDuracionMediaFoundation(propiedadDuracion,
                                        &duracionExtraida) &&
        duracionExtraida > 0;
    if (tiempoDisponible) {
        *posicionActual = posicionExtraida;
        *duracionTotal = duracionExtraida;
    }
    PropVariantClear(&propiedadPosicion);
    PropVariantClear(&propiedadDuracion);
    return tiempoDisponible;
}

void actualizarCacheTiempoReproduccion() {
    if (!reproductor) {
        reiniciarCacheTiempoReproduccion();
        return;
    }

    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    if (!leerTiempoDesdeReproductor(&posicionActual, &duracionTotal)) {
        return;
    }

    bool teniaTiempo = tiempoReproduccionDisponible.load();
    long long posicionAnterior = posicionReproduccionCache.load();
    posicionReproduccionCache.store(posicionActual);
    duracionReproduccionCache.store(duracionTotal);
    tiempoReproduccionDisponible.store(true);

    long long deltaPosicion = posicionActual - posicionAnterior;
    if (deltaPosicion < 0) {
        deltaPosicion = -deltaPosicion;
    }
    constexpr long long umbralActualizacion = 500000LL;
    if (ventanaControles &&
        (!teniaTiempo || deltaPosicion >= umbralActualizacion)) {
        PostMessageW(ventanaControles, MENSAJE_ACTUALIZAR_INTERFAZ, 0, 0);
    }
}

bool obtenerTiempoReproduccionCache(LONGLONG* posicionActual,
                                    LONGLONG* duracionTotal) {
    if (!tiempoReproduccionDisponible.load() || !posicionActual ||
        !duracionTotal) {
        return false;
    }
    *posicionActual = posicionReproduccionCache.load();
    *duracionTotal = duracionReproduccionCache.load();
    return *duracionTotal > 0;
}

bool aplicarPosicionAlReproductor(LONGLONG posicionObjetivo,
                                  bool reproducirAlCompletar) {
    if (!reproductor) return false;

    PROPVARIANT propiedadPosicion;
    PropVariantInit(&propiedadPosicion);
    propiedadPosicion.vt = VT_I8;
    propiedadPosicion.hVal.QuadPart = posicionObjetivo;

    reproducirTrasCambioPosicion.store(reproducirAlCompletar);
    HRESULT resultadoPosicion = reproductor->SetPosition(
        MFP_POSITIONTYPE_100NS, &propiedadPosicion);
    PropVariantClear(&propiedadPosicion);

    if (FAILED(resultadoPosicion)) {
        reproducirTrasCambioPosicion.store(false);
        Wh_Log(L"No fue posible cambiar la posición (HRESULT 0x%08X)",
               static_cast<unsigned int>(resultadoPosicion));
        return false;
    }

    invalidarInterfazControles();
    return true;
}

// Respaldo en el hilo reproductor cuando la caché todavía no tiene duración.
bool buscarEnReproductorDesdeHilo(double proporcionObjetivo,
                                  bool reproducirAlCompletar = false) {
    if (!reproductor) return false;

    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    if (!leerTiempoDesdeReproductor(&posicionActual, &duracionTotal)) {
        return false;
    }

    double proporcionLimitada =
        limitarProporcionProgreso(proporcionObjetivo);
    LONGLONG posicionObjetivo = calcularPosicionTemporal(
        duracionTotal, proporcionLimitada);
    if (!aplicarPosicionAlReproductor(posicionObjetivo, reproducirAlCompletar)) {
        return false;
    }

    duracionReproduccionCache.store(duracionTotal);
    tiempoReproduccionDisponible.store(true);
    return true;
}

// La caché evita consultar la duración durante cada interacción. Si todavía
// no está preparada, el delegado ejecuta una única lectura en el hilo dueño.
bool buscarEnReproduccion(double proporcionObjetivo,
                          bool reproducirAlCompletar = false) {
    if (!reproductor || !posicionamientoDisponible.load()) return false;

    double proporcionLimitada =
        limitarProporcionProgreso(proporcionObjetivo);
    proporcionPosicionSolicitada.store(proporcionLimitada);
    cambioPosicionPendiente.store(true);
    LONGLONG duracionTotal = duracionReproduccionCache.load();
    if (duracionTotal <= 0) {
        reproducirTrasCambioPosicion.store(reproducirAlCompletar);
        if (ventanaDelegadoReproductor &&
            IsWindow(ventanaDelegadoReproductor)) {
            bool busquedaAplicada =
                SendMessageW(ventanaDelegadoReproductor,
                             MENSAJE_EJECUTAR_BUSQUEDA, 0, 0) != 0;
            if (!busquedaAplicada) cambioPosicionPendiente.store(false);
            return busquedaAplicada;
        }
        cambioPosicionPendiente.store(false);
        return false;
    }

    LONGLONG posicionObjetivo =
        calcularPosicionTemporal(duracionTotal, proporcionLimitada);
    bool busquedaAplicada =
        aplicarPosicionAlReproductor(posicionObjetivo,
                                     reproducirAlCompletar);
    if (!busquedaAplicada) cambioPosicionPendiente.store(false);
    return busquedaAplicada;
}

void alternarReproduccion() {
    if (!reproductor) return;
    MFP_MEDIAPLAYER_STATE estadoReproduccion;
    if (SUCCEEDED(reproductor->GetState(&estadoReproduccion)) &&
        estadoReproduccion == MFP_MEDIAPLAYER_STATE_PLAYING) {
        reproductor->Pause();
    } else {
        reproductor->Play();
    }
    invalidarInterfazControles();
}

void alternarSonido() {
    if (!reproductor) return;
    BOOL sonidoSilenciado = TRUE;
    reproductor->GetMute(&sonidoSilenciado);
    reproductor->SetMute(!sonidoSilenciado);
    invalidarInterfazControles();
}

void alternarRepeticion() {
    repeticionHabilitada.store(!repeticionHabilitada.load());
    invalidarInterfazControles();
}

void iniciarArrastreBarraProgreso(HWND ventana, int posicionHorizontal) {
    if (!reproductor || !posicionamientoDisponible.load()) return;

    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);
    proporcionProgreso = calcularProporcionProgreso(
        disenoBarraProgreso, posicionHorizontal);
    arrastrandoBarraProgreso = true;

    SetFocus(ventana);
    SetCapture(ventana);
    invalidarInterfazControles();
}

void actualizarArrastreBarraProgreso(HWND ventana, int posicionHorizontal) {
    if (!arrastrandoBarraProgreso) return;

    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);
    proporcionProgreso = calcularProporcionProgreso(
        disenoBarraProgreso, posicionHorizontal);

    invalidarInterfazControles();
}

void finalizarArrastreBarraProgreso(HWND ventana, bool aplicarCambio) {
    if (!arrastrandoBarraProgreso) return;
    if (aplicarCambio) {
        buscarEnReproduccion(proporcionProgreso);
    }
    arrastrandoBarraProgreso = false;
    if (GetCapture() == ventana) ReleaseCapture();
    invalidarInterfazControles();
}

void desplazarBarraProgresoConTeclado(double incrementoProporcion) {
    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    if (!obtenerTiempoReproduccionCache(&posicionActual, &duracionTotal)) {
        return;
    }
    buscarEnReproduccion(
        static_cast<double>(posicionActual) /
            static_cast<double>(duracionTotal) +
        incrementoProporcion);
}
/**** ended inlining reproductor/controles.cpp ****/
/**** start inlining reproductor/eventos_media_foundation.cpp ****/
class ReceptorEventosMediaFoundation final : public IMFPMediaPlayerCallback {
    std::atomic<ULONG> referencias_{1};

  public:
    STDMETHODIMP QueryInterface(REFIID identificadorInterfaz,
                                void** direccionInterfaz) override {
        if (!direccionInterfaz) return E_POINTER;
        if (identificadorInterfaz == IID_IUnknown ||
            identificadorInterfaz == __uuidof(IMFPMediaPlayerCallback)) {
            *direccionInterfaz = static_cast<IMFPMediaPlayerCallback*>(this);
            AddRef();
            return S_OK;
        }
        *direccionInterfaz = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return ++referencias_; }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG referenciasRestantes = --referencias_;
        if (!referenciasRestantes) delete this;
        return referenciasRestantes;
    }

    void STDMETHODCALLTYPE OnMediaPlayerEvent(
        MFP_EVENT_HEADER* eventoReproductor) override {
        if (!eventoReproductor) return;

        if (eventoReproductor->eEventType ==
                MFP_EVENT_TYPE_MEDIAITEM_CREATED &&
            SUCCEEDED(eventoReproductor->hrEvent) && reproductor) {
            auto eventoElementoCreado =
                MFP_GET_MEDIAITEM_CREATED_EVENT(eventoReproductor);
            reproductor->SetMediaItem(eventoElementoCreado->pMediaItem);
        }

        if (eventoReproductor->eEventType == MFP_EVENT_TYPE_MEDIAITEM_SET &&
            SUCCEEDED(eventoReproductor->hrEvent) && reproductor) {
            auto eventoElementoEstablecido =
                MFP_GET_MEDIAITEM_SET_EVENT(eventoReproductor);
            MFP_MEDIAITEM_CHARACTERISTICS caracteristicasElemento = 0;
            bool permitePosicionamiento =
                eventoElementoEstablecido &&
                eventoElementoEstablecido->pMediaItem &&
                SUCCEEDED(eventoElementoEstablecido->pMediaItem
                              ->GetCharacteristics(
                                  &caracteristicasElemento)) &&
                (caracteristicasElemento & MFP_MEDIAITEM_CAN_SEEK) != 0 &&
                (caracteristicasElemento & MFP_MEDIAITEM_HAS_SLOW_SEEK) == 0;
            posicionamientoDisponible.store(permitePosicionamiento);
            reproductor->SetMute(!configuracion.sonidoHabilitado.load());
            if (configuracion.reproduccionAutomatica.load()) {
                reproductor->Play();
            }
            if (ventanaDelegadoReproductor &&
                IsWindow(ventanaDelegadoReproductor)) {
                SendMessageW(ventanaDelegadoReproductor,
                             MENSAJE_ACTUALIZAR_CACHE_TIEMPO, 0, 0);
            }
        }

        if (eventoReproductor->eEventType == MFP_EVENT_TYPE_POSITION_SET) {
            bool debeReproducir =
                reproducirTrasCambioPosicion.exchange(false);
            if (SUCCEEDED(eventoReproductor->hrEvent) && debeReproducir &&
                reproductor) {
                reproductor->Play();
            }
            if (ventanaDelegadoReproductor &&
                IsWindow(ventanaDelegadoReproductor)) {
                SendMessageW(ventanaDelegadoReproductor,
                             MENSAJE_ACTUALIZAR_CACHE_TIEMPO, 0, 0);
            }
            cambioPosicionPendiente.store(false);
        }

        if (eventoReproductor->eEventType ==
                MFP_EVENT_TYPE_PLAYBACK_ENDED &&
            SUCCEEDED(eventoReproductor->hrEvent) &&
            repeticionHabilitada.load() && reproductor) {
            buscarEnReproduccion(0.0, true);
        }

        if (FAILED(eventoReproductor->hrEvent)) {
            Wh_Log(L"Evento de Media Foundation fallido: tipo %u, HRESULT "
                   L"0x%08X",
                   static_cast<unsigned int>(eventoReproductor->eEventType),
                   static_cast<unsigned int>(eventoReproductor->hrEvent));
        }

        if (ventanaControles) {
            PostMessageW(ventanaControles, MENSAJE_ACTUALIZAR_INTERFAZ, 0, 0);
        }
    }
};
/**** ended inlining reproductor/eventos_media_foundation.cpp ****/
/**** start inlining interfaz/renderizado.cpp ****/
std::wstring formatearTiempo(LONGLONG unidadesCienNanosegundos) {
    long long segundosTotales = unidadesCienNanosegundos / 10000000LL;
    long long horas = segundosTotales / 3600;
    int minutos = static_cast<int>((segundosTotales % 3600) / 60);
    int segundos = static_cast<int>(segundosTotales % 60);
    auto formatearDosDigitos = [](int numero) {
        std::wstring numeroFormateado = std::to_wstring(numero);
        return numeroFormateado.length() == 1
                   ? L"0" + numeroFormateado
                   : numeroFormateado;
    };
    if (horas > 0) {
        return std::to_wstring(horas) + L":" +
               formatearDosDigitos(minutos) + L":" +
               formatearDosDigitos(segundos);
    }
    return formatearDosDigitos(minutos) + L":" +
           formatearDosDigitos(segundos);
}

bool crearRecursosInterfaz() {
    fuenteIconos = CreateFontW(
        -20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe Fluent Icons");
    fuenteTexto = CreateFontW(
        -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    if (fuenteIconos && fuenteTexto) return true;

    if (fuenteIconos) DeleteObject(fuenteIconos);
    if (fuenteTexto) DeleteObject(fuenteTexto);
    fuenteIconos = nullptr;
    fuenteTexto = nullptr;
    return false;
}

void liberarRecursosInterfaz() {
    if (fuenteIconos) DeleteObject(fuenteIconos);
    if (fuenteTexto) DeleteObject(fuenteTexto);
    fuenteIconos = nullptr;
    fuenteTexto = nullptr;
}

void dibujarLinea(HDC contextoDibujo, int limiteIzquierdo,
                  int limiteDerecho, int posicionVertical, int grosor,
                  COLORREF color) {
    HPEN lapiz = CreatePen(PS_SOLID, grosor, color);
    HPEN lapizPreviamenteSeleccionado =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapiz));
    MoveToEx(contextoDibujo, limiteIzquierdo, posicionVertical, nullptr);
    LineTo(contextoDibujo, limiteDerecho, posicionVertical);
    SelectObject(contextoDibujo, lapizPreviamenteSeleccionado);
    DeleteObject(lapiz);
}

void dibujarBotonControl(HDC contextoDibujo, const RECT& areaBoton,
                         const wchar_t* glifo, bool resaltado,
                         bool botonPrincipal) {
    COLORREF colorFondo = botonPrincipal
                              ? RGB(0, 120, 212)
                              : resaltado ? RGB(35, 91, 138)
                                           : RGB(43, 43, 43);
    COLORREF colorBorde = resaltado || botonPrincipal
                              ? RGB(94, 181, 255)
                              : RGB(72, 72, 72);
    HBRUSH pincelFondo = CreateSolidBrush(colorFondo);
    HPEN lapizBorde = CreatePen(PS_SOLID, 1, colorBorde);
    HBRUSH pincelAnterior =
        static_cast<HBRUSH>(SelectObject(contextoDibujo, pincelFondo));
    HPEN lapizAnterior =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    RoundRect(contextoDibujo, areaBoton.left, areaBoton.top,
              areaBoton.right, areaBoton.bottom, 14, 14);
    SelectObject(contextoDibujo, lapizAnterior);
    SelectObject(contextoDibujo, pincelAnterior);
    DeleteObject(lapizBorde);
    DeleteObject(pincelFondo);

    HFONT fuenteAnterior =
        static_cast<HFONT>(SelectObject(contextoDibujo, fuenteIconos));
    SetTextColor(contextoDibujo, RGB(250, 250, 250));
    RECT areaIcono = areaBoton;
    DrawTextW(contextoDibujo, glifo, -1, &areaIcono,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(contextoDibujo, fuenteAnterior);
}

void dibujarInterfaz(HWND ventana) {
    PAINTSTRUCT pintura;
    HDC contextoDibujo = BeginPaint(ventana, &pintura);
    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);

    HBRUSH pincelFondo = CreateSolidBrush(RGB(25, 25, 25));
    FillRect(contextoDibujo, &disenoBarraProgreso.barraControles,
             pincelFondo);
    DeleteObject(pincelFondo);
    SetBkMode(contextoDibujo, TRANSPARENT);
    SetTextColor(contextoDibujo, RGB(245, 245, 245));

    MFP_MEDIAPLAYER_STATE estadoReproduccion = MFP_MEDIAPLAYER_STATE_EMPTY;
    BOOL sonidoSilenciado = TRUE;
    if (reproductor) {
        reproductor->GetState(&estadoReproduccion);
        reproductor->GetMute(&sonidoSilenciado);
    }
    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    bool tiempoDisponible =
        obtenerTiempoReproduccionCache(&posicionActual, &duracionTotal);
    double progresoNormalizado =
        tiempoDisponible
            ? static_cast<double>(posicionActual) /
                  static_cast<double>(duracionTotal)
            : 0.0;
    if (arrastrandoBarraProgreso) {
        progresoNormalizado = proporcionProgreso;
        if (tiempoDisponible) {
            posicionActual = calcularPosicionTemporal(
                duracionTotal, proporcionProgreso);
        }
    } else if (cambioPosicionPendiente.load()) {
        progresoNormalizado = proporcionPosicionSolicitada.load();
        if (tiempoDisponible) {
            posicionActual = calcularPosicionTemporal(
                duracionTotal, progresoNormalizado);
        }
    }

    if (tiempoDisponible) {
        LONGLONG tiempoRestante =
            std::max(0LL, duracionTotal - posicionActual);
        std::wstring textoTranscurrido = formatearTiempo(posicionActual);
        std::wstring textoRestante =
            L"-" + formatearTiempo(tiempoRestante);
        RECT areaTranscurrido{disenoBarraProgreso.limiteIzquierdo, 5,
                              disenoBarraProgreso.limiteDerecho, 22};
        RECT areaRestante = areaTranscurrido;
        HFONT fuenteAnterior =
            static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTexto));
        SetTextColor(contextoDibujo, RGB(210, 210, 210));
        DrawTextW(contextoDibujo, textoTranscurrido.c_str(), -1,
                  &areaTranscurrido,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        DrawTextW(contextoDibujo, textoRestante.c_str(), -1, &areaRestante,
                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuenteAnterior);
    }

    dibujarLinea(contextoDibujo, disenoBarraProgreso.limiteIzquierdo,
                 disenoBarraProgreso.limiteDerecho,
                 disenoBarraProgreso.centroVertical, 3,
                 tiempoDisponible && posicionamientoDisponible.load()
                     ? RGB(92, 92, 92)
                     : RGB(58, 58, 58));
    int posicionPerilla =
        calcularPosicionProgreso(disenoBarraProgreso, progresoNormalizado);
    dibujarLinea(contextoDibujo, disenoBarraProgreso.limiteIzquierdo,
                 posicionPerilla, disenoBarraProgreso.centroVertical, 4,
                 RGB(0, 120, 215));

    HBRUSH pincelPerilla = CreateSolidBrush(RGB(245, 245, 245));
    HBRUSH pincelPreviamenteSeleccionado =
        static_cast<HBRUSH>(SelectObject(contextoDibujo, pincelPerilla));
    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(0, 120, 215));
    HPEN lapizPreviamenteSeleccionado =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    Ellipse(contextoDibujo, posicionPerilla - RADIO_PERILLA_PROGRESO,
            disenoBarraProgreso.centroVertical - RADIO_PERILLA_PROGRESO,
            posicionPerilla + RADIO_PERILLA_PROGRESO + 1,
            disenoBarraProgreso.centroVertical + RADIO_PERILLA_PROGRESO + 1);
    SelectObject(contextoDibujo, lapizPreviamenteSeleccionado);
    SelectObject(contextoDibujo, pincelPreviamenteSeleccionado);
    DeleteObject(lapizBorde);
    DeleteObject(pincelPerilla);

    bool reproduciendo =
        estadoReproduccion == MFP_MEDIAPLAYER_STATE_PLAYING;
    dibujarBotonControl(contextoDibujo,
                         disenoBarraProgreso.botonReproduccion,
                         reproduciendo ? GLIFO_PAUSAR : GLIFO_REPRODUCIR,
                         reproduciendo, true);
    dibujarBotonControl(contextoDibujo, disenoBarraProgreso.botonSonido,
                         sonidoSilenciado ? GLIFO_SILENCIO : GLIFO_VOLUMEN,
                         !sonidoSilenciado, false);
    dibujarBotonControl(contextoDibujo,
                         disenoBarraProgreso.botonRepeticion,
                         GLIFO_REPETICION, repeticionHabilitada.load(),
                         false);

    if (GetFocus() == ventana) {
        RECT areaFoco = disenoBarraProgreso.areaInteractiva;
        InflateRect(&areaFoco, -2, -1);
        DrawFocusRect(contextoDibujo, &areaFoco);
    }
    EndPaint(ventana, &pintura);
}
/**** ended inlining interfaz/renderizado.cpp ****/
/**** start inlining interfaz/ventanas.cpp ****/
void ajustarLayoutVentanasReproductor(HWND ventanaContenedora) {
    RECT areaCliente;
    GetClientRect(ventanaContenedora, &areaCliente);
    int altoAreaVideo =
        std::max<LONG>(0, areaCliente.bottom - ALTO_CONTROLES);

    if (ventanaVideo) {
        SetWindowPos(ventanaVideo, HWND_BOTTOM, 0, 0, areaCliente.right,
                     altoAreaVideo, SWP_NOACTIVATE);
    }
    if (ventanaControles) {
        SetWindowPos(ventanaControles, HWND_TOP, 0, altoAreaVideo,
                     areaCliente.right, ALTO_CONTROLES,
                     SWP_SHOWWINDOW | SWP_NOACTIVATE);
    }
    if (reproductor) {
        reproductor->UpdateVideo();
    }
}

LRESULT CALLBACK procesarVentanaDelegadoReproductor(HWND ventana, UINT mensaje,
                                                    WPARAM parametroMensaje,
                                                    LPARAM datosMensaje) {
    if (mensaje == MENSAJE_EJECUTAR_BUSQUEDA) {
        return buscarEnReproductorDesdeHilo(
                   proporcionPosicionSolicitada.load(),
                   reproducirTrasCambioPosicion.load())
                   ? 1
                   : 0;
    }
    if (mensaje == MENSAJE_ACTUALIZAR_CACHE_TIEMPO) {
        actualizarCacheTiempoReproduccion();
        return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaControles(HWND ventana, UINT mensaje,
                                          WPARAM parametroMensaje,
                                          LPARAM datosMensaje) {
    switch (mensaje) {
        case WM_LBUTTONDOWN: {
            int posicionHorizontal = GET_X_LPARAM(datosMensaje);
            int posicionVertical = GET_Y_LPARAM(datosMensaje);
            DisenoBarraProgreso disenoBarraProgreso =
                calcularDisenoBarraProgreso(ventana);
            if (esClicEnBarraProgreso(disenoBarraProgreso, posicionHorizontal,
                                      posicionVertical)) {
                iniciarArrastreBarraProgreso(ventana, posicionHorizontal);
            } else if (contienePunto(
                           disenoBarraProgreso.botonReproduccion,
                           posicionHorizontal, posicionVertical)) {
                alternarReproduccion();
            } else if (contienePunto(disenoBarraProgreso.botonSonido,
                                     posicionHorizontal,
                                     posicionVertical)) {
                alternarSonido();
            } else if (contienePunto(
                           disenoBarraProgreso.botonRepeticion,
                           posicionHorizontal, posicionVertical)) {
                alternarRepeticion();
            }
            return 0;
        }
        case WM_MOUSEMOVE:
            if (GetCapture() == ventana && arrastrandoBarraProgreso) {
                actualizarArrastreBarraProgreso(
                    ventana, GET_X_LPARAM(datosMensaje));
            }
            return 0;
        case WM_LBUTTONUP:
            if (arrastrandoBarraProgreso) {
                finalizarArrastreBarraProgreso(ventana, true);
            }
            return 0;
        case WM_CAPTURECHANGED:
        case WM_CANCELMODE:
            if (arrastrandoBarraProgreso) {
                finalizarArrastreBarraProgreso(ventana, false);
            }
            return 0;
        case WM_KEYDOWN:
            if (parametroMensaje == VK_LEFT || parametroMensaje == VK_DOWN) {
                desplazarBarraProgresoConTeclado(
                    -PASO_BARRA_PROGRESO_TECLADO);
                return 0;
            }
            if (parametroMensaje == VK_RIGHT || parametroMensaje == VK_UP) {
                desplazarBarraProgresoConTeclado(
                    PASO_BARRA_PROGRESO_TECLADO);
                return 0;
            }
            if (parametroMensaje == VK_HOME) {
                buscarEnReproduccion(0.0);
                return 0;
            }
            if (parametroMensaje == VK_END) {
                buscarEnReproduccion(1.0);
                return 0;
            }
            if (parametroMensaje == VK_SPACE) {
                alternarReproduccion();
                return 0;
            }
            if (parametroMensaje == L'M') {
                alternarSonido();
                return 0;
            }
            if (parametroMensaje == L'L') {
                alternarRepeticion();
                return 0;
            }
            break;
        case WM_GETDLGCODE:
            return DLGC_WANTARROWS | DLGC_WANTCHARS;
        case WM_TIMER:
            ajustarLayoutVentanasReproductor(ventanaReproductor);
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case MENSAJE_ACTUALIZAR_INTERFAZ:
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            dibujarInterfaz(ventana);
            return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaReproductor(HWND ventana, UINT mensaje,
                                             WPARAM parametroMensaje,
                                             LPARAM datosMensaje) {
    if (mensaje == WM_SIZE) {
        ajustarLayoutVentanasReproductor(ventana);
        return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaVideo(HWND ventana, UINT mensaje,
                                      WPARAM parametroMensaje,
                                      LPARAM datosMensaje) {
    if (mensaje == WM_LBUTTONDOWN) {
        alternarReproduccion();
        return 0;
    }
    if (mensaje == WM_ERASEBKGND) {
        RECT areaCliente;
        GetClientRect(ventana, &areaCliente);
        FillRect(reinterpret_cast<HDC>(parametroMensaje), &areaCliente,
                 static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        return 1;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

bool registrarClasesDeVentana() {
    WNDCLASSEXW descripcionClase{sizeof(descripcionClase)};
    descripcionClase.hInstance = GetModuleHandleW(nullptr);
    descripcionClase.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    descripcionClase.lpfnWndProc = procesarVentanaReproductor;
    descripcionClase.lpszClassName = CLASE_VENTANA_REPRODUCTOR;
    ATOM atomoClaseReproductor = RegisterClassExW(&descripcionClase);
    if (!atomoClaseReproductor &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    descripcionClase.lpfnWndProc = procesarVentanaVideo;
    descripcionClase.hbrBackground =
        static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    descripcionClase.lpszClassName = CLASE_VENTANA_VIDEO;
    ATOM atomoClaseVideo = RegisterClassExW(&descripcionClase);
    if (!atomoClaseVideo && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        if (atomoClaseReproductor) {
            UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                             descripcionClase.hInstance);
        }
        return false;
    }

    descripcionClase.lpfnWndProc = procesarVentanaControles;
    descripcionClase.hbrBackground = nullptr;
    descripcionClase.lpszClassName = CLASE_VENTANA_CONTROLES;
    ATOM atomoClaseControles = RegisterClassExW(&descripcionClase);
    if (!atomoClaseControles &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        UnregisterClassW(CLASE_VENTANA_VIDEO, descripcionClase.hInstance);
        if (atomoClaseReproductor) {
            UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                             descripcionClase.hInstance);
        }
        return false;
    }

    descripcionClase.lpfnWndProc = procesarVentanaDelegadoReproductor;
    descripcionClase.lpszClassName = CLASE_VENTANA_DELEGADO;
    if (RegisterClassExW(&descripcionClase) ||
        GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
        return true;
    }

    UnregisterClassW(CLASE_VENTANA_CONTROLES, descripcionClase.hInstance);
    UnregisterClassW(CLASE_VENTANA_VIDEO, descripcionClase.hInstance);
    if (atomoClaseReproductor) {
        UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                         descripcionClase.hInstance);
    }
    return false;
}

void eliminarClasesDeVentana() {
    HINSTANCE instancia = GetModuleHandleW(nullptr);
    UnregisterClassW(CLASE_VENTANA_REPRODUCTOR, instancia);
    UnregisterClassW(CLASE_VENTANA_VIDEO, instancia);
    UnregisterClassW(CLASE_VENTANA_CONTROLES, instancia);
    UnregisterClassW(CLASE_VENTANA_DELEGADO, instancia);
}
/**** ended inlining interfaz/ventanas.cpp ****/
/**** start inlining integracion/panel_previsualizacion.cpp ****/
BOOL CALLBACK encontrarVistaDeArchivos(HWND ventanaCandidata,
                                       LPARAM direccionResultado) {
    wchar_t nombreClase[64];
    if (GetClassNameW(ventanaCandidata, nombreClase, ARRAYSIZE(nombreClase)) &&
        lstrcmpW(nombreClase, L"SHELLDLL_DefView") == 0) {
        *reinterpret_cast<HWND*>(direccionResultado) = ventanaCandidata;
        return FALSE;
    }
    return TRUE;
}

bool obtenerAreaPanel(RECT* areaPanel) {
    if (!areaPanel || !ventanaExploradorAnfitriona ||
        !IsWindow(ventanaExploradorAnfitriona)) {
        return false;
    }
    HWND vistaArchivos = nullptr;
    EnumChildWindows(ventanaExploradorAnfitriona, encontrarVistaDeArchivos,
                     reinterpret_cast<LPARAM>(&vistaArchivos));
    if (!vistaArchivos) return false;

    RECT areaVentanaAnfitriona;
    RECT areaVistaPantalla;
    GetClientRect(ventanaExploradorAnfitriona, &areaVentanaAnfitriona);
    GetWindowRect(vistaArchivos, &areaVistaPantalla);
    POINT esquinaVista{areaVistaPantalla.right, areaVistaPantalla.top};
    ScreenToClient(ventanaExploradorAnfitriona, &esquinaVista);

    // El panel abierto contrae SHELLDLL_DefView y deja una franja amplia a su
    // derecha. Sin panel, esa franja no supera el umbral de seguridad.
    constexpr int ANCHO_MINIMO_PANEL = 180;
    int anchoPanelDetectado = areaVentanaAnfitriona.right - esquinaVista.x;
    if (anchoPanelDetectado < ANCHO_MINIMO_PANEL) return false;
    *areaPanel = {esquinaVista.x, esquinaVista.y,
                  areaVentanaAnfitriona.right,
                  areaVentanaAnfitriona.bottom};
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
    ajustarLayoutVentanasReproductor(ventanaReproductor);
    return true;
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
    if (ventanaReproductor) DestroyWindow(ventanaReproductor);

    ventanaReproductor = nullptr;
    ventanaVideo = nullptr;
    ventanaControles = nullptr;
    arrastrandoBarraProgreso = false;
    proporcionProgreso = 0.0;
    reproducirTrasCambioPosicion.store(false);
    reiniciarCacheTiempoReproduccion();
}

bool asegurarReproductor(const std::wstring& rutaVideo) {
    RECT areaPanel;
    if (!obtenerAreaPanel(&areaPanel)) return false;

    // El decodificador solo se crea cuando coinciden video seleccionado y
    // panel visible; las demás ventanas de Explorer no pagan este costo.
    if (!ventanaReproductor) {
        ventanaReproductor = CreateWindowExW(
            0, CLASE_VENTANA_REPRODUCTOR, L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS |
                WS_CLIPCHILDREN,
            0, 0, 0, 0, ventanaExploradorAnfitriona, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (!ventanaReproductor) return false;

        ventanaVideo = CreateWindowExW(
            0, CLASE_VENTANA_VIDEO, L"", WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, ventanaReproductor, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (!ventanaVideo) {
            liberarRecursosDelReproductor();
            return false;
        }

        ventanaControles = CreateWindowExW(
            0, CLASE_VENTANA_CONTROLES, L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, ventanaReproductor,
            nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!ventanaControles) {
            liberarRecursosDelReproductor();
            return false;
        }

        receptorEventos = new (std::nothrow) ReceptorEventosMediaFoundation();
        if (!receptorEventos) {
            liberarRecursosDelReproductor();
            return false;
        }
        if (FAILED(MFPCreateMediaPlayer(nullptr, FALSE, 0, receptorEventos,
                                        ventanaVideo, &reproductor))) {
            liberarRecursosDelReproductor();
            return false;
        }
        if (!SetTimer(ventanaControles, ID_TEMPORIZADOR_INTERFAZ,
                      INTERVALO_REFRESCO_MS, nullptr)) {
            liberarRecursosDelReproductor();
            return false;
        }
    }

    if (!reproductor || !ajustarReproductorAlPanel()) return false;
    reproductor->Stop();
    reproductor->SetMute(!configuracion.sonidoHabilitado.load());
    reiniciarCacheTiempoReproduccion();
    return SUCCEEDED(reproductor->CreateMediaItemFromURL(
        rutaVideo.c_str(), FALSE, 0, nullptr));
}
/**** ended inlining integracion/panel_previsualizacion.cpp ****/
/**** start inlining ejecucion/hilo_reproductor.cpp ****/
void procesarMensajesDelHilo() {
    MSG mensajePendiente;
    while (PeekMessageW(&mensajePendiente, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&mensajePendiente);
        DispatchMessageW(&mensajePendiente);
    }
}

bool asegurarDelegadoReproductor() {
    if (ventanaDelegadoReproductor) return true;
    ventanaDelegadoReproductor = CreateWindowExW(
        0, CLASE_VENTANA_DELEGADO, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
        GetModuleHandleW(nullptr), nullptr);
    if (!ventanaDelegadoReproductor) {
        Wh_Log(L"No fue posible crear el delegado del reproductor (error %u)",
               static_cast<unsigned int>(GetLastError()));
        return false;
    }
    return true;
}

DWORD WINAPI ejecutarHiloReproductor(void*) {
    HRESULT resultadoInicializacionCom =
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(resultadoInicializacionCom)) {
        Wh_Log(L"No fue posible inicializar COM (HRESULT 0x%08X)",
               static_cast<unsigned int>(resultadoInicializacionCom));
        return 1;
    }
    if (FAILED(MFStartup(MF_VERSION))) {
        Wh_Log(L"No fue posible inicializar Media Foundation");
        CoUninitialize();
        return 1;
    }
    if (!asegurarDelegadoReproductor()) {
        MFShutdown();
        CoUninitialize();
        return 1;
    }

    PunteroCom<IShellWindows> ventanasShell;
    HRESULT resultadoCreacionShell = CoCreateInstance(
        CLSID_ShellWindows, nullptr, CLSCTX_ALL,
        IID_PPV_ARGS(ventanasShell.recibir()));
    if (FAILED(resultadoCreacionShell)) {
        Wh_Log(L"No fue posible obtener IShellWindows (HRESULT 0x%08X)",
               static_cast<unsigned int>(resultadoCreacionShell));
        MFShutdown();
        CoUninitialize();
        return 1;
    }

    while (!detenerHilo.load()) {
        // La configuración se aplica una sola vez por cambio. Así, el botón de
        // sonido conserva la elección interactiva del usuario.
        if (reproductor && configuracionPendiente.exchange(false)) {
            reproductor->SetMute(!configuracion.sonidoHabilitado.load());
        }

        std::wstring rutaSeleccionada =
            obtenerVideoSeleccionado(ventanasShell.obtener());
        if (!rutaSeleccionada.empty() &&
            rutaSeleccionada != rutaVideoActiva &&
            asegurarReproductor(rutaSeleccionada)) {
            rutaVideoActiva = rutaSeleccionada;
        }
        if (rutaSeleccionada.empty()) {
            rutaVideoActiva.clear();
            if (ventanaReproductor) {
                ShowWindow(ventanaReproductor, SW_HIDE);
            }
        } else if (ventanaReproductor) {
            ajustarReproductorAlPanel();
        }

        DWORD resultadoEspera = MsgWaitForMultipleObjects(
            1, &eventoDetener, FALSE, 100, QS_ALLINPUT);
        if (resultadoEspera == WAIT_OBJECT_0) break;
        if (resultadoEspera == WAIT_FAILED) {
            Wh_Log(L"Falló la espera del hilo reproductor (error %u)",
                   static_cast<unsigned int>(GetLastError()));
            break;
        }
        procesarMensajesDelHilo();
        if (reproductor && ventanaControles &&
            IsWindowVisible(ventanaControles)) {
            actualizarCacheTiempoReproduccion();
        }
    }

    liberarRecursosDelReproductor();
    if (ventanaDelegadoReproductor) {
        DestroyWindow(ventanaDelegadoReproductor);
        ventanaDelegadoReproductor = nullptr;
    }
    // Las referencias COM deben liberarse antes de cerrar el apartamento.
    ventanasShell.liberar();
    MFShutdown();
    CoUninitialize();
    return 0;
}
/**** ended inlining ejecucion/hilo_reproductor.cpp ****/

}  // namespace

/**** start inlining windhawk/ciclo_vida.cpp ****/
namespace {

void cargarConfiguracion() {
    configuracion.reproduccionAutomatica.store(
        Wh_GetIntSetting(L"autoplay") != 0);
    configuracion.sonidoHabilitado.store(Wh_GetIntSetting(L"sound") != 0);
    configuracionPendiente.store(true);
}

}  // namespace

BOOL Wh_ModInit() {
    cargarConfiguracion();
    if (!crearRecursosInterfaz()) {
        Wh_Log(L"No fue posible crear las fuentes de la interfaz");
        return FALSE;
    }
    if (!registrarClasesDeVentana()) {
        Wh_Log(L"No fue posible registrar las clases de ventana");
        liberarRecursosInterfaz();
        return FALSE;
    }
    detenerHilo.store(false);
    eventoDetener = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!eventoDetener) {
        Wh_Log(L"No fue posible crear el evento de detención (error %u)",
               static_cast<unsigned int>(GetLastError()));
        eliminarClasesDeVentana();
        liberarRecursosInterfaz();
        return FALSE;
    }
    hiloReproductor =
        CreateThread(nullptr, 0, ejecutarHiloReproductor, nullptr, 0, nullptr);
    if (!hiloReproductor) {
        Wh_Log(L"No fue posible crear el hilo reproductor (error %u)",
               static_cast<unsigned int>(GetLastError()));
        CloseHandle(eventoDetener);
        eventoDetener = nullptr;
        eliminarClasesDeVentana();
        liberarRecursosInterfaz();
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
    if (eventoDetener) SetEvent(eventoDetener);
    if (hiloReproductor) {
        // No se descarga código mientras el hilo todavía pueda ejecutarlo.
        WaitForSingleObject(hiloReproductor, INFINITE);
        CloseHandle(hiloReproductor);
        hiloReproductor = nullptr;
    }
    if (eventoDetener) {
        CloseHandle(eventoDetener);
        eventoDetener = nullptr;
    }
    eliminarClasesDeVentana();
    liberarRecursosInterfaz();
}
/**** ended inlining windhawk/ciclo_vida.cpp ****/
/**** ended inlining useful-sideview.cpp ****/
