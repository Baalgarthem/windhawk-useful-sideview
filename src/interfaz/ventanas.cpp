LRESULT CALLBACK procesarVentanaReproductor(HWND ventana, UINT mensaje,
                                             WPARAM wParam, LPARAM lParam) {
    switch (mensaje) {
        case WM_LBUTTONDOWN: {
            RECT areaCliente;
            GetClientRect(ventana, &areaCliente);
            int posicionX = GET_X_LPARAM(lParam);
            int posicionY = GET_Y_LPARAM(lParam);
            if (posicionY >= areaCliente.bottom - ALTO_CONTROLES) {
                if (posicionX < ANCHO_BOTON_REPRODUCIR) {
                    alternarReproduccion();
                } else if (posicionX >=
                           areaCliente.right - ANCHO_BOTON_SONIDO) {
                    alternarSonido();
                } else {
                    buscarDesdeCoordenada(posicionX);
                    SetCapture(ventana);
                }
            }
            return 0;
        }
        case WM_MOUSEMOVE:
            if (GetCapture() == ventana) {
                buscarDesdeCoordenada(GET_X_LPARAM(lParam));
            }
            return 0;
        case WM_LBUTTONUP:
            if (GetCapture() == ventana) ReleaseCapture();
            return 0;
        case WM_SIZE:
            if (ventanaVideo) {
                RECT areaCliente;
                GetClientRect(ventana, &areaCliente);
                SetWindowPos(ventanaVideo, nullptr, 0, 0, areaCliente.right,
                             std::max<LONG>(0, areaCliente.bottom - ALTO_CONTROLES),
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
            if (reproductor) reproductor->UpdateVideo();
            return 0;
        case WM_TIMER:
        case MENSAJE_ACTUALIZAR_INTERFAZ:
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            dibujarInterfaz(ventana);
            return 0;
    }
    return DefWindowProcW(ventana, mensaje, wParam, lParam);
}

LRESULT CALLBACK procesarVentanaVideo(HWND ventana, UINT mensaje,
                                      WPARAM wParam, LPARAM lParam) {
    if (mensaje == WM_LBUTTONDOWN) {
        alternarReproduccion();
        return 0;
    }
    if (mensaje == WM_ERASEBKGND) {
        RECT areaCliente;
        GetClientRect(ventana, &areaCliente);
        FillRect(reinterpret_cast<HDC>(wParam), &areaCliente,
                 static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        return 1;
    }
    return DefWindowProcW(ventana, mensaje, wParam, lParam);
}

bool registrarClasesDeVentana() {
    WNDCLASSEXW clase{sizeof(clase)};
    clase.lpfnWndProc = procesarVentanaReproductor;
    clase.hInstance = GetModuleHandleW(nullptr);
    clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    clase.lpszClassName = CLASE_VENTANA_REPRODUCTOR;
    if (!RegisterClassExW(&clase) &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    clase.lpfnWndProc = procesarVentanaVideo;
    clase.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    clase.lpszClassName = CLASE_VENTANA_VIDEO;
    return RegisterClassExW(&clase) ||
           GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

void eliminarClasesDeVentana() {
    UnregisterClassW(CLASE_VENTANA_REPRODUCTOR, GetModuleHandleW(nullptr));
    UnregisterClassW(CLASE_VENTANA_VIDEO, GetModuleHandleW(nullptr));
}
