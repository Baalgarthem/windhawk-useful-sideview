#pragma once

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
