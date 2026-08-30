void ajustarLayoutVentanasReproductor(HWND ventanaContenedora) {
    RECT areaCliente;
    GetClientRect(ventanaContenedora, &areaCliente);
    int altoAreaVideo =
        std::max<LONG>(0, areaCliente.bottom - ALTO_CONTROLES);

    if (ventanaVideo) {
        SetWindowPos(ventanaVideo, HWND_BOTTOM, 0, 0, areaCliente.right,
                     altoAreaVideo, SWP_NOACTIVATE);
    }
    if (ventanaControles) {
        SetWindowPos(ventanaControles, HWND_TOP, 0, altoAreaVideo,
                     areaCliente.right, ALTO_CONTROLES,
                     SWP_SHOWWINDOW | SWP_NOACTIVATE);
    }
    if (reproductor) {
        reproductor->UpdateVideo();
    }
}

LRESULT CALLBACK procesarVentanaDelegadoReproductor(HWND ventana, UINT mensaje,
                                                    WPARAM parametroMensaje,
                                                    LPARAM datosMensaje) {
    if (mensaje == MENSAJE_EJECUTAR_BUSQUEDA) {
        return buscarEnReproductorDesdeHilo(
                   proporcionPosicionSolicitada.load(),
                   reproducirTrasCambioPosicion.load())
                   ? 1
                   : 0;
    }
    if (mensaje == MENSAJE_ACTUALIZAR_CACHE_TIEMPO) {
        actualizarCacheTiempoReproduccion();
        return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaControles(HWND ventana, UINT mensaje,
                                          WPARAM parametroMensaje,
                                          LPARAM datosMensaje) {
    switch (mensaje) {
        case WM_LBUTTONDOWN: {
            int posicionHorizontal = GET_X_LPARAM(datosMensaje);
            int posicionVertical = GET_Y_LPARAM(datosMensaje);
            DisenoBarraProgreso disenoBarraProgreso =
                calcularDisenoBarraProgreso(ventana);
            if (esClicEnBarraProgreso(disenoBarraProgreso, posicionHorizontal,
                                      posicionVertical)) {
                iniciarArrastreBarraProgreso(ventana, posicionHorizontal);
            } else if (contienePunto(
                           disenoBarraProgreso.botonReproduccion,
                           posicionHorizontal, posicionVertical)) {
                alternarReproduccion();
            } else if (contienePunto(disenoBarraProgreso.botonSonido,
                                     posicionHorizontal,
                                     posicionVertical)) {
                alternarSonido();
            } else if (contienePunto(
                           disenoBarraProgreso.botonRepeticion,
                           posicionHorizontal, posicionVertical)) {
                alternarRepeticion();
            }
            return 0;
        }
        case WM_MOUSEMOVE:
            if (GetCapture() == ventana && arrastrandoBarraProgreso) {
                actualizarArrastreBarraProgreso(
                    ventana, GET_X_LPARAM(datosMensaje));
            }
            return 0;
        case WM_LBUTTONUP:
            if (arrastrandoBarraProgreso) {
                finalizarArrastreBarraProgreso(ventana, true);
            }
            return 0;
        case WM_CAPTURECHANGED:
        case WM_CANCELMODE:
            if (arrastrandoBarraProgreso) {
                finalizarArrastreBarraProgreso(ventana, false);
            }
            return 0;
        case WM_KEYDOWN:
            if (parametroMensaje == VK_LEFT || parametroMensaje == VK_DOWN) {
                desplazarBarraProgresoConTeclado(
                    -PASO_BARRA_PROGRESO_TECLADO);
                return 0;
            }
            if (parametroMensaje == VK_RIGHT || parametroMensaje == VK_UP) {
                desplazarBarraProgresoConTeclado(
                    PASO_BARRA_PROGRESO_TECLADO);
                return 0;
            }
            if (parametroMensaje == VK_HOME) {
                buscarEnReproduccion(0.0);
                return 0;
            }
            if (parametroMensaje == VK_END) {
                buscarEnReproduccion(1.0);
                return 0;
            }
            if (parametroMensaje == VK_SPACE) {
                alternarReproduccion();
                return 0;
            }
            if (parametroMensaje == L'M') {
                alternarSonido();
                return 0;
            }
            if (parametroMensaje == L'L') {
                alternarRepeticion();
                return 0;
            }
            break;
        case WM_GETDLGCODE:
            return DLGC_WANTARROWS | DLGC_WANTCHARS;
        case WM_TIMER:
            ajustarLayoutVentanasReproductor(ventanaReproductor);
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case MENSAJE_ACTUALIZAR_INTERFAZ:
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            dibujarInterfaz(ventana);
            return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaReproductor(HWND ventana, UINT mensaje,
                                             WPARAM parametroMensaje,
                                             LPARAM datosMensaje) {
    if (mensaje == WM_SIZE) {
        ajustarLayoutVentanasReproductor(ventana);
        return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaVideo(HWND ventana, UINT mensaje,
                                      WPARAM parametroMensaje,
                                      LPARAM datosMensaje) {
    if (mensaje == WM_LBUTTONDOWN) {
        alternarReproduccion();
        return 0;
    }
    if (mensaje == WM_ERASEBKGND) {
        RECT areaCliente;
        GetClientRect(ventana, &areaCliente);
        FillRect(reinterpret_cast<HDC>(parametroMensaje), &areaCliente,
                 static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        return 1;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

bool registrarClasesDeVentana() {
    WNDCLASSEXW descripcionClase{sizeof(descripcionClase)};
    descripcionClase.hInstance = GetModuleHandleW(nullptr);
    descripcionClase.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    descripcionClase.lpfnWndProc = procesarVentanaReproductor;
    descripcionClase.lpszClassName = CLASE_VENTANA_REPRODUCTOR;
    ATOM atomoClaseReproductor = RegisterClassExW(&descripcionClase);
    if (!atomoClaseReproductor &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    descripcionClase.lpfnWndProc = procesarVentanaVideo;
    descripcionClase.hbrBackground =
        static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    descripcionClase.lpszClassName = CLASE_VENTANA_VIDEO;
    ATOM atomoClaseVideo = RegisterClassExW(&descripcionClase);
    if (!atomoClaseVideo && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        if (atomoClaseReproductor) {
            UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                             descripcionClase.hInstance);
        }
        return false;
    }

    descripcionClase.lpfnWndProc = procesarVentanaControles;
    descripcionClase.hbrBackground = nullptr;
    descripcionClase.lpszClassName = CLASE_VENTANA_CONTROLES;
    ATOM atomoClaseControles = RegisterClassExW(&descripcionClase);
    if (!atomoClaseControles &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        UnregisterClassW(CLASE_VENTANA_VIDEO, descripcionClase.hInstance);
        if (atomoClaseReproductor) {
            UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                             descripcionClase.hInstance);
        }
        return false;
    }

    descripcionClase.lpfnWndProc = procesarVentanaDelegadoReproductor;
    descripcionClase.lpszClassName = CLASE_VENTANA_DELEGADO;
    if (RegisterClassExW(&descripcionClase) ||
        GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
        return true;
    }

    UnregisterClassW(CLASE_VENTANA_CONTROLES, descripcionClase.hInstance);
    UnregisterClassW(CLASE_VENTANA_VIDEO, descripcionClase.hInstance);
    if (atomoClaseReproductor) {
        UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                         descripcionClase.hInstance);
    }
    return false;
}

void eliminarClasesDeVentana() {
    HINSTANCE instancia = GetModuleHandleW(nullptr);
    UnregisterClassW(CLASE_VENTANA_REPRODUCTOR, instancia);
    UnregisterClassW(CLASE_VENTANA_VIDEO, instancia);
    UnregisterClassW(CLASE_VENTANA_CONTROLES, instancia);
    UnregisterClassW(CLASE_VENTANA_DELEGADO, instancia);
}
