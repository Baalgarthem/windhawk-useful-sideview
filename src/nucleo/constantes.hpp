#pragma once

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
