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
