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
