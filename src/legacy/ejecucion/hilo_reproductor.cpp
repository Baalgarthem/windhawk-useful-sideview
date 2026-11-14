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
    bool mfIniciado = SUCCEEDED(MFStartup(MF_VERSION));
    if (!mfIniciado) {
        Wh_Log(L"Media Foundation no está disponible; se omitirá la reproducción de video");
    }

    if (!asegurarDelegadoReproductor()) {
        if (mfIniciado) MFShutdown();
        CoUninitialize();
        return 1;
    }

    PunteroCom<IShellWindows> ventanasShell;

    while (!detenerHilo.load()) {
        if (!ventanasShell.obtener()) {
            CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_ALL,
                             IID_PPV_ARGS(ventanasShell.recibir()));
        }

        // La configuración se aplica una sola vez por cambio.
        if (configuracionPendiente.exchange(false)) {
            if (reproductor) {
                reproductor->SetMute(!configuracion.sonidoHabilitado.load());
            }
            if (ventanaReproductor && IsWindowVisible(ventanaReproductor)) {
                ajustarReproductorAlPanel();
                invalidarInterfaz();
            }
        }

        if (ventanasShell.obtener()) {
            long totalVentanas = 0;
            if (FAILED(ventanasShell->get_Count(&totalVentanas))) {
                // Desconexión RPC o instancia inválida: liberar para reconectar en el siguiente ciclo
                ventanasShell.liberar();
            }
        }

        std::wstring rutaCarpetaContenedora;
        std::vector<std::wstring> rutasSeleccionadas =
            obtenerArchivosSeleccionados(ventanasShell.obtener(),
                                         &rutaCarpetaContenedora);

        RECT areaPanel{};
        bool panelPrevisualizacionAbierto = obtenerAreaPanel(&areaPanel);
        if (!panelPrevisualizacionAbierto) {
            AcquireSRWLockShared(&bloqueoRutaArchivoActivo);
            bool rutaActivaNoVacia = !rutaArchivoActivo.empty();
            ReleaseSRWLockShared(&bloqueoRutaArchivoActivo);
            if (rutaActivaNoVacia ||
                (ventanaReproductor && IsWindowVisible(ventanaReproductor))) {
                liberarRecursosDeTodasCategorias();
                AcquireSRWLockExclusive(&bloqueoRutaArchivoActivo);
                rutaArchivoActivo.clear();
                ReleaseSRWLockExclusive(&bloqueoRutaArchivoActivo);
                rutaVideoActiva.clear();
                if (ventanaReproductor) {
                    ShowWindow(ventanaReproductor, SW_HIDE);
                }
            }
        } else {
            if (!rutasSeleccionadas.empty()) {
                std::wstring claveSeleccion;
                for (const auto& rutaElemento : rutasSeleccionadas) {
                    claveSeleccion += rutaElemento + L"|";
                }
                bool necesitaActualizar = false;
                {
                    AcquireSRWLockShared(&bloqueoRutaArchivoActivo);
                    necesitaActualizar = (claveSeleccion != rutaArchivoActivo);
                    ReleaseSRWLockShared(&bloqueoRutaArchivoActivo);
                }
                if (necesitaActualizar) {
                    if (asegurarVisualizadorParaArchivos(rutasSeleccionadas)) {
                        AcquireSRWLockExclusive(&bloqueoRutaArchivoActivo);
                        rutaArchivoActivo = claveSeleccion;
                        ReleaseSRWLockExclusive(&bloqueoRutaArchivoActivo);
                        rutaVideoActiva = rutasSeleccionadas.front();
                    }
                } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
                    if (estadoVistaCarpetas.indiceCarpetaActual < estadoVistaCarpetas.carpetasSeleccionadas.size()) {
                        const std::wstring& rutaCarpetaInspeccionada =
                            estadoVistaCarpetas.carpetasSeleccionadas[estadoVistaCarpetas.indiceCarpetaActual];
                        WIN32_FILE_ATTRIBUTE_DATA datosAtributosCarpeta;
                        if (GetFileAttributesExW(rutaCarpetaInspeccionada.c_str(), GetFileExInfoStandard, &datosAtributosCarpeta) != FALSE) {
                            if (memcmp(&datosAtributosCarpeta.ftLastWriteTime,
                                       &estadoVistaCarpetas.ftUltimaModificacionArbol,
                                       sizeof(FILETIME)) != 0) {
                                cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
                                invalidarInterfaz();
                            }
                        }
                    }
                }
            } else if (!rutaCarpetaContenedora.empty()) {
                std::wstring claveVistaVacia =
                    L"VISTA_VACIA|" + rutaCarpetaContenedora;
                bool necesitaVacia = false;
                {
                    AcquireSRWLockShared(&bloqueoRutaArchivoActivo);
                    necesitaVacia = (claveVistaVacia != rutaArchivoActivo);
                    ReleaseSRWLockShared(&bloqueoRutaArchivoActivo);
                }
                if (necesitaVacia) {
                    if (asegurarVisorVistaVacia(rutaCarpetaContenedora)) {
                        AcquireSRWLockExclusive(&bloqueoRutaArchivoActivo);
                        rutaArchivoActivo = claveVistaVacia;
                        ReleaseSRWLockExclusive(&bloqueoRutaArchivoActivo);
                        rutaVideoActiva.clear();
                    }
                } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
                    WIN32_FILE_ATTRIBUTE_DATA datosAttrCarpeta;
                    if (GetFileAttributesExW(rutaCarpetaContenedora.c_str(), GetFileExInfoStandard, &datosAttrCarpeta) != FALSE) {
                        if (memcmp(&datosAttrCarpeta.ftLastWriteTime,
                                   &estadoVistaCarpetas.resumenVistaVacia.ftUltimaModificacion,
                                   sizeof(FILETIME)) != 0) {
                            calcularResumenVistaVacia(rutaCarpetaContenedora);
                            invalidarInterfaz();
                        }
                    }
                }
            } else {
                AcquireSRWLockExclusive(&bloqueoRutaArchivoActivo);
                rutaArchivoActivo.clear();
                ReleaseSRWLockExclusive(&bloqueoRutaArchivoActivo);
                rutaVideoActiva.clear();
                if (ventanaReproductor) {
                    ShowWindow(ventanaReproductor, SW_HIDE);
                }
            }
            if (ventanaReproductor && IsWindowVisible(ventanaReproductor)) {
                ajustarReproductorAlPanel(&areaPanel);
            }
        }

        DWORD intervaloEspera =
            ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) ? 16 : 100;
        DWORD resultadoEspera = MsgWaitForMultipleObjects(
            1, &eventoDetener, FALSE, intervaloEspera, QS_ALLINPUT);
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
    if (mfIniciado) {
        MFShutdown();
    }
    CoUninitialize();
    return 0;
}
