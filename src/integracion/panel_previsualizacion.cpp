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
