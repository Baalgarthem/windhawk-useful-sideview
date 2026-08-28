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
