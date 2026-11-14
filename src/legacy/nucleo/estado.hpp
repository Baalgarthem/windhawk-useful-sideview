#pragma once

#include <commctrl.h>

constexpr UINT_PTR ID_SUBCLASE_VISTA_ARCHIVOS = 1001;
constexpr UINT_PTR ID_SUBCLASE_VENTANA_EXPLORADOR = 1002;

Configuracion configuracion;
std::wstring rutaVideoActiva;
std::vector<std::wstring> listaVideosSeleccionados;
size_t indiceVideoSeleccionado = 0;
std::wstring rutaArchivoActivo;
SRWLOCK bloqueoRutaArchivoActivo = SRWLOCK_INIT;  // Causa 4: proteger acceso multihilo
std::atomic<TipoContenidoArchivo> tipoContenidoActivo{TipoContenidoArchivo::Ninguno};
ULONG_PTR tokenGdiplus = 0;

std::atomic_bool detenerHilo{false};
std::atomic_bool configuracionPendiente{true};
HANDLE hiloReproductor = nullptr;
HANDLE eventoDetener = nullptr;
HWND ventanaReproductor = nullptr;
HWND ventanaVideo = nullptr;
HWND ventanaControles = nullptr;
HWND ventanaDelegadoReproductor = nullptr;
HWND ventanaExploradorAnfitriona = nullptr;
HWND ventanaVistaArchivosActiva = nullptr;
IMFPMediaPlayer* reproductor = nullptr;
HFONT fuenteIconos = nullptr;
HFONT fuenteTexto = nullptr;
HFONT fuenteTextoPequeno = nullptr;

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

std::atomic<float> velocidadReproduccionActual{1.0f};
std::atomic_bool navegacionManualEnCurso{false};
HWND ventanaMenuVelocidad = nullptr;
bool menuVelocidadDesplegado = false;
int opcionVelocidadResaltada = -1;
BYTE opacidadMenuVelocidad = 255;
bool animandoCierreMenuVelocidad = false;

struct EstadoVideoMetadatos {
    std::wstring rutaVideo;
    std::wstring nombreArchivo;
    std::wstring extensionFormato;
    std::wstring textoTamano;
    std::wstring textoDuracion;
    std::wstring textoFechaModificacion;
    int totalEnRango{0};
    int indiceEnRango{0};
    int totalEnCarpeta{0};
    int indiceEnCarpeta{0};
    bool esRangoSeleccionado{false};
    ULONGLONG tiempoInicioParpadeoNav{0};
};

inline EstadoVideoMetadatos estadoVideoMetadatos;

inline void liberarRecursosVideo() {
    estadoVideoMetadatos.rutaVideo.clear();
    estadoVideoMetadatos.nombreArchivo.clear();
    estadoVideoMetadatos.extensionFormato.clear();
    estadoVideoMetadatos.textoTamano.clear();
    estadoVideoMetadatos.textoDuracion.clear();
    estadoVideoMetadatos.textoFechaModificacion.clear();
    estadoVideoMetadatos.totalEnRango = 0;
    estadoVideoMetadatos.indiceEnRango = 0;
    estadoVideoMetadatos.totalEnCarpeta = 0;
    estadoVideoMetadatos.indiceEnCarpeta = 0;
    estadoVideoMetadatos.esRangoSeleccionado = false;
    estadoVideoMetadatos.tiempoInicioParpadeoNav = 0;
}

struct EstadoPendienteImplementacion {
    std::wstring rutaActual;
    std::wstring nombreArchivo;
    std::wstring extension;
    std::vector<std::wstring> archivosSeleccionados;
    size_t indiceActual{0};
    RECT botonAnterior{};
    RECT botonSiguiente{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};
};

inline EstadoPendienteImplementacion estadoDocumentos;
inline EstadoPendienteImplementacion estadoCodigo;

inline void liberarRecursosDocumentos() {
    estadoDocumentos.rutaActual.clear();
    estadoDocumentos.nombreArchivo.clear();
    estadoDocumentos.extension.clear();
    estadoDocumentos.archivosSeleccionados.clear();
    estadoDocumentos.indiceActual = 0;
    estadoDocumentos.hoverBotonAnterior = false;
    estadoDocumentos.hoverBotonSiguiente = false;
}

inline void liberarRecursosCodigo() {
    estadoCodigo.rutaActual.clear();
    estadoCodigo.nombreArchivo.clear();
    estadoCodigo.extension.clear();
    estadoCodigo.archivosSeleccionados.clear();
    estadoCodigo.indiceActual = 0;
    estadoCodigo.hoverBotonAnterior = false;
    estadoCodigo.hoverBotonSiguiente = false;
}

struct EstadoArchivoDesconocido {
    std::wstring rutaActual;
    std::wstring nombreArchivo;
    std::wstring extension;
    std::wstring tamanoFormateado;
    std::vector<std::wstring> archivosSeleccionados;
    size_t indiceActual{0};
    RECT botonAnterior{};
    RECT botonSiguiente{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};
};

inline EstadoArchivoDesconocido estadoDesconocido;

inline void liberarRecursosDesconocido() {
    estadoDesconocido.rutaActual.clear();
    estadoDesconocido.nombreArchivo.clear();
    estadoDesconocido.extension.clear();
    estadoDesconocido.tamanoFormateado.clear();
    estadoDesconocido.archivosSeleccionados.clear();
    estadoDesconocido.indiceActual = 0;
    estadoDesconocido.hoverBotonAnterior = false;
    estadoDesconocido.hoverBotonSiguiente = false;
}

