#pragma once

constexpr wchar_t CLASE_VENTANA_REPRODUCTOR[] = L"WindhawkUsefulSideView";
constexpr wchar_t CLASE_VENTANA_VIDEO[] = L"WindhawkUsefulSideViewVideo";
constexpr wchar_t CLASE_VENTANA_CONTROLES[] = L"WindhawkUsefulSideViewControls";
constexpr wchar_t CLASE_VENTANA_DELEGADO[] = L"WindhawkUsefulSideViewDelegate";
constexpr UINT_PTR ID_TEMPORIZADOR_INTERFAZ = 1;
constexpr UINT_PTR ID_TEMPORIZADOR_ANIMACION_GIF = 2;
constexpr UINT INTERVALO_REFRESCO_MS = 200;
constexpr int ALTO_CONTROLES = 96;
constexpr int TAMANO_BOTON_CONTROL = 34;
constexpr int SEPARACION_BOTONES = 8;
constexpr int CANTIDAD_BOTONES_CENTRALES = 5;
constexpr int ANCHO_GRUPO_BOTONES =
    TAMANO_BOTON_CONTROL * CANTIDAD_BOTONES_CENTRALES +
    SEPARACION_BOTONES * (CANTIDAD_BOTONES_CENTRALES - 1);
constexpr int MARGEN_BARRA_PROGRESO = 16;
constexpr int RADIO_PERILLA_PROGRESO = 6;
constexpr double PASO_BARRA_PROGRESO_TECLADO = 0.05;
constexpr UINT MENSAJE_ACTUALIZAR_INTERFAZ = WM_APP + 1;
constexpr UINT MENSAJE_ACTUALIZAR_CACHE_TIEMPO = WM_APP + 2;
constexpr UINT MENSAJE_EJECUTAR_BUSQUEDA = WM_APP + 3;

constexpr wchar_t GLIFO_REPRODUCIR[] = L"\uE768";
constexpr wchar_t GLIFO_PAUSAR[] = L"\uE769";
constexpr wchar_t GLIFO_ANTERIOR[] = L"\uE892";
constexpr wchar_t GLIFO_SIGUIENTE[] = L"\uE893";
constexpr wchar_t GLIFO_VOLUMEN[] = L"\uE767";
constexpr wchar_t GLIFO_SILENCIO[] = L"\uE74F";
constexpr wchar_t GLIFO_REPETICION[] = L"\uE8EE";

constexpr wchar_t CLASE_VENTANA_MENU_VELOCIDAD[] =
    L"WindhawkUsefulSideViewSpeedMenu";

constexpr int ANCHO_BOTON_VELOCIDAD = 46;
constexpr int ALTO_BOTON_VELOCIDAD = 26;
constexpr int ANCHO_PANEL_VELOCIDAD = 64;
constexpr int ALTO_OPCION_VELOCIDAD = 24;
constexpr int TOTAL_OPCIONES_VELOCIDAD = 6;
constexpr int ALTO_PANEL_VELOCIDAD =
    ALTO_OPCION_VELOCIDAD * TOTAL_OPCIONES_VELOCIDAD + 8;
constexpr float VALORES_VELOCIDAD[TOTAL_OPCIONES_VELOCIDAD] = {
    0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 2.0f};
constexpr const wchar_t* ETIQUETAS_VELOCIDAD[TOTAL_OPCIONES_VELOCIDAD] = {
    L"0.5x", L"0.75x", L"1.0x", L"1.25x", L"1.5x", L"2.0x"};


constexpr int ALTO_CONTROLES_COMPRIMIDOS = 104;
constexpr int TOTAL_FORMATOS_DESTINO = 5;
constexpr const wchar_t* FORMATOS_DESTINO[TOTAL_FORMATOS_DESTINO] = {
    L"CBZ (.cbz)", L"ZIP (.zip)", L"CBR (.cbr)", L"7Z (.7z)", L"TAR (.tar)"};
constexpr const wchar_t* EXTENSIONES_DESTINO[TOTAL_FORMATOS_DESTINO] = {
    L".cbz", L".zip", L".cbr", L".7z", L".tar"};

constexpr COLORREF COLOR_FONDO_REPRODUCTOR = RGB(15, 15, 15);

constexpr int ALTO_METADATOS_VIDEO = 90;
constexpr UINT_PTR ID_TEMPORIZADOR_PARPADEO_NAV = 1005;

constexpr wchar_t GLIFO_COMPRIMIDO[] = L"\uE8B7";
constexpr wchar_t GLIFO_CHEVRON_ARRIBA[] = L"\uE70E";


