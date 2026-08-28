#pragma once

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
