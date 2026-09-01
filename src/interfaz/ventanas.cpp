void ocultarMenuVelocidadInmediato() {
    if (ventanaMenuVelocidad && IsWindow(ventanaMenuVelocidad)) {
        ShowWindow(ventanaMenuVelocidad, SW_HIDE);
    }
    menuVelocidadDesplegado = false;
    animandoCierreMenuVelocidad = false;
    opacidadMenuVelocidad = 255;
    opcionVelocidadResaltada = -1;
    if (ventanaControles && IsWindow(ventanaControles)) {
        InvalidateRect(ventanaControles, nullptr, FALSE);
    }
}

void mostrarMenuVelocidadFlotante(HWND ventanaOrigen) {
    if (!ventanaMenuVelocidad || !IsWindow(ventanaMenuVelocidad)) return;

    DisenoBarraProgreso diseno = calcularDisenoBarraProgreso(ventanaOrigen);
    POINT puntoOrigen{diseno.botonVelocidad.right - ANCHO_PANEL_VELOCIDAD,
                      diseno.botonVelocidad.top - ALTO_PANEL_VELOCIDAD - 6};
    ClientToScreen(ventanaOrigen, &puntoOrigen);

    menuVelocidadDesplegado = true;
    opacidadMenuVelocidad = 255;
    SetLayeredWindowAttributes(ventanaMenuVelocidad, 0, 255, LWA_ALPHA);

    SetWindowPos(ventanaMenuVelocidad, HWND_TOPMOST, puntoOrigen.x,
                 puntoOrigen.y, ANCHO_PANEL_VELOCIDAD, ALTO_PANEL_VELOCIDAD,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);
    InvalidateRect(ventanaMenuVelocidad, nullptr, TRUE);
    InvalidateRect(ventanaOrigen, nullptr, FALSE);
}

void ajustarLayoutVentanasReproductor(HWND ventanaContenedora) {
    if (tipoContenidoActivo.load() != TipoContenidoArchivo::Video ||
        !IsWindowVisible(ventanaContenedora)) {
        ocultarMenuVelocidadInmediato();
    } else if (menuVelocidadDesplegado && ventanaMenuVelocidad && IsWindow(ventanaMenuVelocidad)) {
        if (ventanaControles && IsWindow(ventanaControles)) {
            DisenoBarraProgreso diseno = calcularDisenoBarraProgreso(ventanaControles);
            POINT puntoOrigen{diseno.botonVelocidad.right - ANCHO_PANEL_VELOCIDAD,
                              diseno.botonVelocidad.top - ALTO_PANEL_VELOCIDAD - 6};
            ClientToScreen(ventanaControles, &puntoOrigen);
            SetWindowPos(ventanaMenuVelocidad, HWND_TOPMOST, puntoOrigen.x, puntoOrigen.y,
                         ANCHO_PANEL_VELOCIDAD, ALTO_PANEL_VELOCIDAD,
                         SWP_NOACTIVATE | SWP_NOSIZE);
        }
    }
    RECT areaCliente;
    GetClientRect(ventanaContenedora, &areaCliente);
    int altoControlesEfectivo = ALTO_CONTROLES;
    bool controlesArriba = false;

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
        altoControlesEfectivo = ALTO_CONTROLES_COMPRIMIDOS;
    } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
        altoControlesEfectivo = 34;
        controlesArriba = true;
    } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen ||
               tipoContenidoActivo.load() == TipoContenidoArchivo::Documento ||
               tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo ||
               tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
        altoControlesEfectivo = 34;
    } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas ||
               tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
        altoControlesEfectivo = 0;
    }
    int altoAreaVideo =
        std::max<LONG>(0, areaCliente.bottom - altoControlesEfectivo);

    int yControles = 0;
    int yVideo = 0;
    if (controlesArriba && altoControlesEfectivo > 0) {
        yControles = 0;
        yVideo = altoControlesEfectivo;
    } else {
        yVideo = 0;
        yControles = altoAreaVideo;
    }

    HDWP hdwp = BeginDeferWindowPos(2);
    if (hdwp && ventanaVideo && IsWindow(ventanaVideo)) {
        hdwp = DeferWindowPos(hdwp, ventanaVideo, nullptr, 0, yVideo, areaCliente.right,
                              altoAreaVideo,
                              SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_SHOWWINDOW);
    }
    if (hdwp && ventanaControles && IsWindow(ventanaControles)) {
        if (altoControlesEfectivo > 0) {
            hdwp = DeferWindowPos(hdwp, ventanaControles, HWND_TOP, 0, yControles,
                                  areaCliente.right, altoControlesEfectivo,
                                  SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS);
        }
    }
    if (hdwp) {
        EndDeferWindowPos(hdwp);
    }

    if (ventanaControles && IsWindow(ventanaControles) && altoControlesEfectivo <= 0) {
        ShowWindow(ventanaControles, SW_HIDE);
    }
    if (ventanaVideo && IsWindow(ventanaVideo)) {
        InvalidateRect(ventanaVideo, nullptr, FALSE);
    }
    if (ventanaControles && IsWindow(ventanaControles) && altoControlesEfectivo > 0) {
        InvalidateRect(ventanaControles, nullptr, FALSE);
    }
    if (reproductor) {
        reproductor->UpdateVideo();
    }
}

inline bool mostrarDocumentoAnterior() {
    if (estadoDocumentos.archivosSeleccionados.size() > 1) {
        estadoDocumentos.indiceActual =
            (estadoDocumentos.indiceActual == 0)
                ? (estadoDocumentos.archivosSeleccionados.size() - 1)
                : (estadoDocumentos.indiceActual - 1);
        estadoDocumentos.rutaActual =
            estadoDocumentos.archivosSeleccionados[estadoDocumentos.indiceActual];
        estadoDocumentos.nombreArchivo =
            PathFindFileNameW(estadoDocumentos.rutaActual.c_str());
        estadoDocumentos.extension =
            PathFindExtensionW(estadoDocumentos.rutaActual.c_str());
        return true;
    }
    if (estadoDocumentos.rutaActual.empty()) return false;
    std::wstring rutaAnterior = obtenerDocumentoAnteriorEnCarpeta(estadoDocumentos.rutaActual);
    if (rutaAnterior.empty() || rutaAnterior == estadoDocumentos.rutaActual) return false;
    estadoDocumentos.rutaActual = rutaAnterior;
    estadoDocumentos.nombreArchivo = PathFindFileNameW(estadoDocumentos.rutaActual.c_str());
    estadoDocumentos.extension = PathFindExtensionW(estadoDocumentos.rutaActual.c_str());
    estadoDocumentos.archivosSeleccionados = { rutaAnterior };
    estadoDocumentos.indiceActual = 0;
    return true;
}

inline bool mostrarDocumentoSiguiente() {
    if (estadoDocumentos.archivosSeleccionados.size() > 1) {
        estadoDocumentos.indiceActual =
            (estadoDocumentos.indiceActual + 1) %
            estadoDocumentos.archivosSeleccionados.size();
        estadoDocumentos.rutaActual =
            estadoDocumentos.archivosSeleccionados[estadoDocumentos.indiceActual];
        estadoDocumentos.nombreArchivo =
            PathFindFileNameW(estadoDocumentos.rutaActual.c_str());
        estadoDocumentos.extension =
            PathFindExtensionW(estadoDocumentos.rutaActual.c_str());
        return true;
    }
    if (estadoDocumentos.rutaActual.empty()) return false;
    std::wstring rutaSiguiente = obtenerDocumentoSiguienteEnCarpeta(estadoDocumentos.rutaActual);
    if (rutaSiguiente.empty() || rutaSiguiente == estadoDocumentos.rutaActual) return false;
    estadoDocumentos.rutaActual = rutaSiguiente;
    estadoDocumentos.nombreArchivo = PathFindFileNameW(estadoDocumentos.rutaActual.c_str());
    estadoDocumentos.extension = PathFindExtensionW(estadoDocumentos.rutaActual.c_str());
    estadoDocumentos.archivosSeleccionados = { rutaSiguiente };
    estadoDocumentos.indiceActual = 0;
    return true;
}

inline bool mostrarCodigoAnterior() {
    if (estadoCodigo.archivosSeleccionados.size() > 1) {
        estadoCodigo.indiceActual =
            (estadoCodigo.indiceActual == 0)
                ? (estadoCodigo.archivosSeleccionados.size() - 1)
                : (estadoCodigo.indiceActual - 1);
        estadoCodigo.rutaActual =
            estadoCodigo.archivosSeleccionados[estadoCodigo.indiceActual];
        estadoCodigo.nombreArchivo =
            PathFindFileNameW(estadoCodigo.rutaActual.c_str());
        estadoCodigo.extension =
            PathFindExtensionW(estadoCodigo.rutaActual.c_str());
        return true;
    }
    if (estadoCodigo.rutaActual.empty()) return false;
    std::wstring rutaAnterior = obtenerCodigoAnteriorEnCarpeta(estadoCodigo.rutaActual);
    if (rutaAnterior.empty() || rutaAnterior == estadoCodigo.rutaActual) return false;
    estadoCodigo.rutaActual = rutaAnterior;
    estadoCodigo.nombreArchivo = PathFindFileNameW(estadoCodigo.rutaActual.c_str());
    estadoCodigo.extension = PathFindExtensionW(estadoCodigo.rutaActual.c_str());
    estadoCodigo.archivosSeleccionados = { rutaAnterior };
    estadoCodigo.indiceActual = 0;
    return true;
}

inline bool mostrarCodigoSiguiente() {
    if (estadoCodigo.archivosSeleccionados.size() > 1) {
        estadoCodigo.indiceActual =
            (estadoCodigo.indiceActual + 1) %
            estadoCodigo.archivosSeleccionados.size();
        estadoCodigo.rutaActual =
            estadoCodigo.archivosSeleccionados[estadoCodigo.indiceActual];
        estadoCodigo.nombreArchivo =
            PathFindFileNameW(estadoCodigo.rutaActual.c_str());
        estadoCodigo.extension =
            PathFindExtensionW(estadoCodigo.rutaActual.c_str());
        return true;
    }
    if (estadoCodigo.rutaActual.empty()) return false;
    std::wstring rutaSiguiente = obtenerCodigoSiguienteEnCarpeta(estadoCodigo.rutaActual);
    if (rutaSiguiente.empty() || rutaSiguiente == estadoCodigo.rutaActual) return false;
    estadoCodigo.rutaActual = rutaSiguiente;
    estadoCodigo.nombreArchivo = PathFindFileNameW(estadoCodigo.rutaActual.c_str());
    estadoCodigo.extension = PathFindExtensionW(estadoCodigo.rutaActual.c_str());
    estadoCodigo.archivosSeleccionados = { rutaSiguiente };
    estadoCodigo.indiceActual = 0;
    return true;
}

inline bool mostrarDesconocidoAnterior() {
    if (estadoDesconocido.archivosSeleccionados.size() > 1) {
        estadoDesconocido.indiceActual =
            (estadoDesconocido.indiceActual == 0)
                ? (estadoDesconocido.archivosSeleccionados.size() - 1)
                : (estadoDesconocido.indiceActual - 1);
        estadoDesconocido.rutaActual =
            estadoDesconocido.archivosSeleccionados[estadoDesconocido.indiceActual];
        estadoDesconocido.nombreArchivo =
            PathFindFileNameW(estadoDesconocido.rutaActual.c_str());
        estadoDesconocido.extension =
            PathFindExtensionW(estadoDesconocido.rutaActual.c_str());
        WIN32_FILE_ATTRIBUTE_DATA datosArchivo{};
        if (GetFileAttributesExW(estadoDesconocido.rutaActual.c_str(), GetFileExInfoStandard, &datosArchivo)) {
            ULARGE_INTEGER tamano;
            tamano.LowPart = datosArchivo.nFileSizeLow;
            tamano.HighPart = datosArchivo.nFileSizeHigh;
            wchar_t bufTamano[64]{};
            StrFormatByteSizeW(static_cast<LONGLONG>(tamano.QuadPart), bufTamano, ARRAYSIZE(bufTamano));
            estadoDesconocido.tamanoFormateado = bufTamano;
        } else {
            estadoDesconocido.tamanoFormateado = L"";
        }
        return true;
    }
    return false;
}

inline bool mostrarDesconocidoSiguiente() {
    if (estadoDesconocido.archivosSeleccionados.size() > 1) {
        estadoDesconocido.indiceActual =
            (estadoDesconocido.indiceActual + 1) %
            estadoDesconocido.archivosSeleccionados.size();
        estadoDesconocido.rutaActual =
            estadoDesconocido.archivosSeleccionados[estadoDesconocido.indiceActual];
        estadoDesconocido.nombreArchivo =
            PathFindFileNameW(estadoDesconocido.rutaActual.c_str());
        estadoDesconocido.extension =
            PathFindExtensionW(estadoDesconocido.rutaActual.c_str());
        WIN32_FILE_ATTRIBUTE_DATA datosArchivo{};
        if (GetFileAttributesExW(estadoDesconocido.rutaActual.c_str(), GetFileExInfoStandard, &datosArchivo)) {
            ULARGE_INTEGER tamano;
            tamano.LowPart = datosArchivo.nFileSizeLow;
            tamano.HighPart = datosArchivo.nFileSizeHigh;
            wchar_t bufTamano[64]{};
            StrFormatByteSizeW(static_cast<LONGLONG>(tamano.QuadPart), bufTamano, ARRAYSIZE(bufTamano));
            estadoDesconocido.tamanoFormateado = bufTamano;
        } else {
            estadoDesconocido.tamanoFormateado = L"";
        }
        return true;
    }
    return false;
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

inline void invalidarInterfaz() {
    if (ventanaControles) InvalidateRect(ventanaControles, nullptr, FALSE);
    if (ventanaVideo) InvalidateRect(ventanaVideo, nullptr, FALSE);
}

LRESULT CALLBACK procesarVentanaControles(HWND ventana, UINT mensaje,
                                          WPARAM parametroMensaje,
                                          LPARAM datosMensaje) {
    switch (mensaje) {
        case WM_LBUTTONDOWN: {
            int posicionHorizontal = GET_X_LPARAM(datosMensaje);
            int posicionVertical = GET_Y_LPARAM(datosMensaje);

            if (modoCursorPegadoBarraProgreso) {
                cancelarModoCursorPegadoBarraProgreso(ventana, true);
                return 0;
            }

            if (tipoContenidoActivo.load() ==
                TipoContenidoArchivo::Comprimido) {
                DisenoComprimidos disenoComprimidos =
                    calcularDisenoComprimidos(ventana);

                if (estadoComprimidos.menuProgramasAbierto) {
                    if (contienePunto(
                            disenoComprimidos.panelOpcionesPrograma,
                            posicionHorizontal, posicionVertical)) {
                        const std::vector<std::wstring>& listaProgramas =
                            obtenerListaProgramasActivaConst();
                        for (size_t posicion = 0;
                             posicion <
                             disenoComprimidos.opcionesPrograma.size();
                             ++posicion) {
                            if (contienePunto(
                                    disenoComprimidos
                                        .opcionesPrograma[posicion],
                                    posicionHorizontal,
                                    posicionVertical)) {
                                if (posicion < listaProgramas.size()) {
                                    estadoComprimidos
                                        .indiceProgramaSeleccionado =
                                        static_cast<int>(posicion);
                                    estadoComprimidos.programaTemporal.clear();
                                } else {
                                    estadoComprimidos
                                        .menuProgramasAbierto = false;
                                    estadoComprimidos
                                        .opcionProgramaHover = -1;
                                    invalidarInterfaz();
                                    seleccionarYAgregarPrograma(ventana);
                                    invalidarInterfaz();
                                    return 0;
                                }
                                estadoComprimidos.menuProgramasAbierto =
                                    false;
                                estadoComprimidos.opcionProgramaHover = -1;
                                invalidarInterfaz();
                                return 0;
                            }
                        }
                    }
                    estadoComprimidos.menuProgramasAbierto = false;
                    estadoComprimidos.opcionProgramaHover = -1;
                    invalidarInterfaz();
                    return 0;
                }

                if (estadoComprimidos.menuDesplegableAbierto) {
                    if (contienePunto(
                            disenoComprimidos.panelOpcionesFormato,
                            posicionHorizontal, posicionVertical)) {
                        for (int posicion = 0;
                             posicion < TOTAL_FORMATOS_DESTINO; ++posicion) {
                            if (contienePunto(
                                    disenoComprimidos
                                        .opcionesFormato[posicion],
                                    posicionHorizontal,
                                    posicionVertical)) {
                                estadoComprimidos
                                    .indiceFormatoDestino = posicion;
                                estadoComprimidos
                                    .menuDesplegableAbierto = false;
                                estadoComprimidos.opcionFormatoHover = -1;
                                invalidarInterfaz();
                                return 0;
                            }
                        }
                    }
                    estadoComprimidos.menuDesplegableAbierto = false;
                    estadoComprimidos.opcionFormatoHover = -1;
                    invalidarInterfaz();
                    return 0;
                }

                if (contienePunto(
                        disenoComprimidos.botonDesplegablePrograma,
                        posicionHorizontal, posicionVertical)) {
                    const std::vector<std::wstring>& listaProgramas =
                        obtenerListaProgramasActivaConst();
                    if (listaProgramas.empty() &&
                        estadoComprimidos.programaTemporal.empty()) {
                        seleccionarYAgregarPrograma(ventana);
                        invalidarInterfaz();
                        return 0;
                    }
                    estadoComprimidos.menuProgramasAbierto =
                        !estadoComprimidos.menuProgramasAbierto;
                    estadoComprimidos.menuDesplegableAbierto = false;
                    estadoComprimidos.opcionProgramaHover = -1;
                    invalidarInterfaz();
                    return 0;
                }

                if (contienePunto(
                        disenoComprimidos.botonEjecutarAbrir,
                        posicionHorizontal, posicionVertical)) {
                    ejecutarAbrirArchivosConPrograma(ventana);
                    invalidarInterfaz();
                    return 0;
                }

                if (contienePunto(
                        disenoComprimidos.botonDesplegableFormato,
                        posicionHorizontal, posicionVertical)) {
                    estadoComprimidos.menuDesplegableAbierto =
                        !estadoComprimidos.menuDesplegableAbierto;
                    estadoComprimidos.menuProgramasAbierto = false;
                    estadoComprimidos.opcionFormatoHover = -1;
                    invalidarInterfaz();
                    return 0;
                }

                if (contienePunto(
                        disenoComprimidos.botonEjecutarConversion,
                        posicionHorizontal, posicionVertical)) {
                    ejecutarConversionArchivos();
                    invalidarInterfaz();
                    return 0;
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoTexto.botonAnterior, puntoRaton)) {
                    if (mostrarTextoAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoTexto.botonSiguiente, puntoRaton)) {
                    if (mostrarTextoSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoTexto.botonEditar, puntoRaton)) {
                    if (!estadoTexto.rutaTextoActual.empty()) {
                        HINSTANCE res = ShellExecuteW(nullptr, L"edit", estadoTexto.rutaTextoActual.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                        if (reinterpret_cast<INT_PTR>(res) <= 32) {
                            ShellExecuteW(nullptr, L"open", estadoTexto.rutaTextoActual.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                        }
                    }
                    return 0;
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoDocumentos.botonAnterior, puntoRaton)) {
                    if (mostrarDocumentoAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoDocumentos.botonSiguiente, puntoRaton)) {
                    if (mostrarDocumentoSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoCodigo.botonAnterior, puntoRaton)) {
                    if (mostrarCodigoAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoCodigo.botonSiguiente, puntoRaton)) {
                    if (mostrarCodigoSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoDesconocido.botonAnterior, puntoRaton)) {
                    if (mostrarDesconocidoAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoDesconocido.botonSiguiente, puntoRaton)) {
                    if (mostrarDesconocidoSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                return 0;
            }

            DisenoBarraProgreso disenoBarraProgreso =
                calcularDisenoBarraProgreso(ventana);

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoImagen.botonAnterior, puntoRaton)) {
                    if (mostrarImagenAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoImagen.botonSiguiente, puntoRaton)) {
                    if (mostrarImagenSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
                if (contienePunto(disenoBarraProgreso.botonVelocidad,
                                  posicionHorizontal, posicionVertical)) {
                    if (menuVelocidadDesplegado) {
                        ocultarMenuVelocidadInmediato();
                    } else {
                        mostrarMenuVelocidadFlotante(ventana);
                    }
                    return 0;
                }
                if (menuVelocidadDesplegado) {
                    ocultarMenuVelocidadInmediato();
                }
            }

            if (contienePunto(disenoBarraProgreso.botonAnterior,
                              posicionHorizontal, posicionVertical)) {
                reproducirVideoAnterior();
                return 0;
            }
            if (contienePunto(disenoBarraProgreso.botonReproduccion,
                              posicionHorizontal, posicionVertical)) {
                alternarReproduccion();
                return 0;
            }
            if (contienePunto(disenoBarraProgreso.botonSiguiente,
                              posicionHorizontal, posicionVertical)) {
                reproducirVideoSiguiente();
                return 0;
            }
            if (contienePunto(disenoBarraProgreso.botonSonido,
                              posicionHorizontal, posicionVertical)) {
                alternarSonido();
                return 0;
            }
            if (contienePunto(disenoBarraProgreso.botonRepeticion,
                              posicionHorizontal, posicionVertical)) {
                alternarRepeticion();
                return 0;
            }

            if (esClicEnBarraProgreso(disenoBarraProgreso, posicionHorizontal,
                                      posicionVertical)) {
                iniciarArrastreBarraProgreso(ventana, posicionHorizontal);
                return 0;
            }
            return 0;
        }
        case WM_LBUTTONDBLCLK: {
            int posicionHorizontal = GET_X_LPARAM(datosMensaje);
            int posicionVertical = GET_Y_LPARAM(datosMensaje);
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
                DisenoBarraProgreso disenoBarraProgreso =
                    calcularDisenoBarraProgreso(ventana);
                if (esClicEnBarraProgreso(disenoBarraProgreso,
                                          posicionHorizontal,
                                          posicionVertical)) {
                    iniciarModoCursorPegadoBarraProgreso(
                        ventana, posicionHorizontal);
                    return 0;
                }
            }
            // Tratar el doble clic sobre los botones con la misma respuesta que WM_LBUTTONDOWN
            return procesarVentanaControles(ventana, WM_LBUTTONDOWN, parametroMensaje, datosMensaje);
        }
        case WM_MOUSEMOVE: {
            int posicionHorizontal = GET_X_LPARAM(datosMensaje);
            int posicionVertical = GET_Y_LPARAM(datosMensaje);

            if (tipoContenidoActivo.load() ==
                TipoContenidoArchivo::Comprimido) {
                DisenoComprimidos disenoComprimidos =
                    calcularDisenoComprimidos(ventana);

                if (estadoComprimidos.menuProgramasAbierto) {
                    int nuevoHoverProg = -1;
                    for (size_t posicionPrograma = 0;
                         posicionPrograma <
                         disenoComprimidos.opcionesPrograma.size();
                         ++posicionPrograma) {
                        if (contienePunto(
                                disenoComprimidos
                                    .opcionesPrograma[posicionPrograma],
                                posicionHorizontal, posicionVertical)) {
                            nuevoHoverProg =
                                static_cast<int>(posicionPrograma);
                            break;
                        }
                    }
                    if (nuevoHoverProg !=
                        estadoComprimidos.opcionProgramaHover) {
                        estadoComprimidos.opcionProgramaHover = nuevoHoverProg;
                        InvalidateRect(ventana, nullptr, FALSE);
                    }
                }

                if (estadoComprimidos.menuDesplegableAbierto) {
                    int nuevoHover = -1;
                    for (int posicion = 0; posicion < TOTAL_FORMATOS_DESTINO;
                         ++posicion) {
                        if (contienePunto(
                                disenoComprimidos.opcionesFormato[posicion],
                                posicionHorizontal, posicionVertical)) {
                            nuevoHover = posicion;
                            break;
                        }
                    }
                    if (nuevoHover !=
                        estadoComprimidos.opcionFormatoHover) {
                        estadoComprimidos.opcionFormatoHover = nuevoHover;
                        InvalidateRect(ventana, nullptr, FALSE);
                    }
                }
                return 0;
            }

            if (modoCursorPegadoBarraProgreso ||
                (GetCapture() == ventana && arrastrandoBarraProgreso)) {
                actualizarArrastreBarraProgreso(ventana, posicionHorizontal);
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT),
                                                  TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);

                POINT puntoRaton{posicionHorizontal, posicionVertical};
                bool hoverAnt =
                    PtInRect(&estadoTexto.botonAnterior, puntoRaton);
                bool hoverSig =
                    PtInRect(&estadoTexto.botonSiguiente, puntoRaton);
                if (hoverAnt != estadoTexto.hoverBotonAnterior ||
                    hoverSig != estadoTexto.hoverBotonSiguiente) {
                    estadoTexto.hoverBotonAnterior = hoverAnt;
                    estadoTexto.hoverBotonSiguiente = hoverSig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT pt{posicionHorizontal, posicionVertical};
                bool ant = PtInRect(&estadoDocumentos.botonAnterior, pt) != FALSE;
                bool sig = PtInRect(&estadoDocumentos.botonSiguiente, pt) != FALSE;
                if (ant != estadoDocumentos.hoverBotonAnterior || sig != estadoDocumentos.hoverBotonSiguiente) {
                    estadoDocumentos.hoverBotonAnterior = ant;
                    estadoDocumentos.hoverBotonSiguiente = sig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT pt{posicionHorizontal, posicionVertical};
                bool ant = PtInRect(&estadoCodigo.botonAnterior, pt) != FALSE;
                bool sig = PtInRect(&estadoCodigo.botonSiguiente, pt) != FALSE;
                if (ant != estadoCodigo.hoverBotonAnterior || sig != estadoCodigo.hoverBotonSiguiente) {
                    estadoCodigo.hoverBotonAnterior = ant;
                    estadoCodigo.hoverBotonSiguiente = sig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                bool ant = PtInRect(&estadoImagen.botonAnterior, puntoRaton) != FALSE;
                bool sig = PtInRect(&estadoImagen.botonSiguiente, puntoRaton) != FALSE;
                if (ant != estadoImagen.hoverBotonAnterior || sig != estadoImagen.hoverBotonSiguiente) {
                    estadoImagen.hoverBotonAnterior = ant;
                    estadoImagen.hoverBotonSiguiente = sig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                bool ant = PtInRect(&estadoDesconocido.botonAnterior, puntoRaton) != FALSE;
                bool sig = PtInRect(&estadoDesconocido.botonSiguiente, puntoRaton) != FALSE;
                if (ant != estadoDesconocido.hoverBotonAnterior || sig != estadoDesconocido.hoverBotonSiguiente) {
                    estadoDesconocido.hoverBotonAnterior = ant;
                    estadoDesconocido.hoverBotonSiguiente = sig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT)};
            eventoSeguimiento.dwFlags = TME_LEAVE;
            eventoSeguimiento.hwndTrack = ventana;
            TrackMouseEvent(&eventoSeguimiento);
            return 0;
        }
        case WM_MOUSELEAVE:
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                if (estadoTexto.hoverBotonAnterior ||
                    estadoTexto.hoverBotonSiguiente) {
                    estadoTexto.hoverBotonAnterior = false;
                    estadoTexto.hoverBotonSiguiente = false;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                if (estadoDocumentos.hoverBotonAnterior ||
                    estadoDocumentos.hoverBotonSiguiente) {
                    estadoDocumentos.hoverBotonAnterior = false;
                    estadoDocumentos.hoverBotonSiguiente = false;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                if (estadoCodigo.hoverBotonAnterior ||
                    estadoCodigo.hoverBotonSiguiente) {
                    estadoCodigo.hoverBotonAnterior = false;
                    estadoCodigo.hoverBotonSiguiente = false;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                if (estadoImagen.hoverBotonAnterior ||
                    estadoImagen.hoverBotonSiguiente) {
                    estadoImagen.hoverBotonAnterior = false;
                    estadoImagen.hoverBotonSiguiente = false;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
                if (estadoDesconocido.hoverBotonAnterior ||
                    estadoDesconocido.hoverBotonSiguiente) {
                    estadoDesconocido.hoverBotonAnterior = false;
                    estadoDesconocido.hoverBotonSiguiente = false;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            return 0;
        case WM_SETCURSOR: {
            POINT puntoCursor;
            GetCursorPos(&puntoCursor);
            RECT rectControles;
            GetWindowRect(ventana, &rectControles);
            if (puntoCursor.x >= rectControles.left && puntoCursor.x <= rectControles.left + 8) {
                SetCursor(LoadCursorW(nullptr, IDC_SIZEWE));
                return TRUE;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                DisenoBarraProgreso diseno = calcularDisenoBarraProgreso(ventana);
                if (esClicEnBarraProgreso(diseno, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonReproduccion, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonAnterior, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonSiguiente, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonSonido, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonRepeticion, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonVelocidad, puntoRaton.x, puntoRaton.y)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoTexto.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoTexto.botonSiguiente, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoDocumentos.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoDocumentos.botonSiguiente, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoCodigo.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoCodigo.botonSiguiente, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoImagen.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoImagen.botonSiguiente, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoDesconocido.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoDesconocido.botonSiguiente, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            break;
        }
        case WM_LBUTTONUP:
            if (modoCursorPegadoBarraProgreso) {
                return 0;
            }
            if (arrastrandoBarraProgreso) {
                finalizarArrastreBarraProgreso(ventana, true);
            }
            return 0;
        case WM_CAPTURECHANGED:
        case WM_CANCELMODE:
            if (modoCursorPegadoBarraProgreso) {
                cancelarModoCursorPegadoBarraProgreso(ventana, false);
                return 0;
            }
            if (arrastrandoBarraProgreso) {
                finalizarArrastreBarraProgreso(ventana, false);
            }
            return 0;
        case WM_MOUSEWHEEL: {
            if (tipoContenidoActivo.load() ==
                TipoContenidoArchivo::Comprimido) {
                short deltaRueda =
                    GET_WHEEL_DELTA_WPARAM(parametroMensaje);
                desplazarScrollLog(deltaRueda > 0 ? -1 : 1);
                InvalidateRect(ventana, nullptr, FALSE);
                return 0;
            }
            break;
        }
        case WM_KEYDOWN:
            if (parametroMensaje == VK_ESCAPE &&
                modoCursorPegadoBarraProgreso) {
                cancelarModoCursorPegadoBarraProgreso(ventana, false);
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                if (parametroMensaje == VK_LEFT || parametroMensaje == L'P') {
                    if (mostrarTextoAnterior()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_RIGHT || parametroMensaje == L'N') {
                    if (mostrarTextoSiguiente()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_UP) {
                    desplazarScrollTexto(-40);
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_DOWN ||
                    parametroMensaje == VK_SPACE) {
                    desplazarScrollTexto(40);
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_PRIOR) {
                    desplazarScrollTexto(-240);
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_NEXT) {
                    desplazarScrollTexto(240);
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_HOME) {
                    estadoTexto.desplazamientoScrollY = 0;
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_END) {
                    estadoTexto.desplazamientoScrollY =
                        std::max(0, estadoTexto.altoTotalContenido - 200);
                    invalidarInterfaz();
                    return 0;
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                if (parametroMensaje == VK_LEFT ||
                    parametroMensaje == VK_UP ||
                    parametroMensaje == L'P') {
                    if (mostrarDocumentoAnterior()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_RIGHT ||
                    parametroMensaje == VK_DOWN ||
                    parametroMensaje == VK_SPACE ||
                    parametroMensaje == L'N') {
                    if (mostrarDocumentoSiguiente()) invalidarInterfaz();
                    return 0;
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                if (parametroMensaje == VK_LEFT ||
                    parametroMensaje == VK_UP ||
                    parametroMensaje == L'P') {
                    if (mostrarCodigoAnterior()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_RIGHT ||
                    parametroMensaje == VK_DOWN ||
                    parametroMensaje == VK_SPACE ||
                    parametroMensaje == L'N') {
                    if (mostrarCodigoSiguiente()) invalidarInterfaz();
                    return 0;
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                if (parametroMensaje == VK_LEFT ||
                    parametroMensaje == VK_UP ||
                    parametroMensaje == L'P') {
                    if (mostrarImagenAnterior()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_RIGHT ||
                    parametroMensaje == VK_DOWN ||
                    parametroMensaje == VK_SPACE ||
                    parametroMensaje == L'N') {
                    if (mostrarImagenSiguiente()) invalidarInterfaz();
                    return 0;
                }
                return 0;
            }

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
            if (parametroMensaje == L'P') {
                reproducirVideoAnterior();
                return 0;
            }
            if (parametroMensaje == L'N') {
                reproducirVideoSiguiente();
                return 0;
            }
        case WM_NCHITTEST: {
            POINT puntoPantalla{GET_X_LPARAM(datosMensaje),
                                GET_Y_LPARAM(datosMensaje)};
            RECT rectVentana;
            GetWindowRect(ventana, &rectVentana);
            if (puntoPantalla.x >= rectVentana.left &&
                puntoPantalla.x <= rectVentana.left + 6) {
                return HTTRANSPARENT;
            }
            break;
        }
        case WM_GETDLGCODE:
            return DLGC_WANTARROWS | DLGC_WANTCHARS;
        case WM_TIMER:
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case WM_SIZE:
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
    if (mensaje == WM_ERASEBKGND) {
        return 1;
    }
    if (mensaje == WM_NCHITTEST) {
        POINT puntoPantalla{GET_X_LPARAM(datosMensaje),
                            GET_Y_LPARAM(datosMensaje)};
        RECT rectVentana;
        GetWindowRect(ventana, &rectVentana);
        if (puntoPantalla.x >= rectVentana.left &&
            puntoPantalla.x <= rectVentana.left + 6) {
            return HTTRANSPARENT;
        }
    }
    if (mensaje == WM_SIZE) {
        ajustarLayoutVentanasReproductor(ventana);
        return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaVideo(HWND ventana, UINT mensaje,
                                      WPARAM parametroMensaje,
                                      LPARAM datosMensaje) {
    if (mensaje == WM_ERASEBKGND) {
        return 1;
    }
    if (mensaje == WM_NCHITTEST) {
        POINT puntoPantalla{GET_X_LPARAM(datosMensaje),
                            GET_Y_LPARAM(datosMensaje)};
        RECT rectVentana;
        GetWindowRect(ventana, &rectVentana);
        if (puntoPantalla.x >= rectVentana.left &&
            puntoPantalla.x <= rectVentana.left + 6) {
            return HTTRANSPARENT;
        }
    }
    if (mensaje == WM_SIZE) {
        InvalidateRect(ventana, nullptr, FALSE);
        return 0;
    }
    if (mensaje == WM_MOUSEWHEEL) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            short deltaRueda = GET_WHEEL_DELTA_WPARAM(parametroMensaje);
            desplazarScrollTexto(deltaRueda > 0 ? -60 : 60);
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            short deltaRueda = GET_WHEEL_DELTA_WPARAM(parametroMensaje);
            int incremento = (deltaRueda > 0 ? -44 : 44);
            int maxScroll = std::max(0, estadoVistaCarpetas.altoTotalArbol - 100);
            estadoVistaCarpetas.desplazamientoScrollY = std::max(
                0, std::min(estadoVistaCarpetas.desplazamientoScrollY + incremento, maxScroll));
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
            if (!estadoComprimidos.esModoComic) {
                short deltaRueda = GET_WHEEL_DELTA_WPARAM(parametroMensaje);
                int incremento = (deltaRueda > 0 ? -44 : 44);
                desplazarScrollComprimido(incremento);
                InvalidateRect(ventana, nullptr, FALSE);
                return 0;
            }
        }
    }
    if (mensaje == WM_MOUSEMOVE) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
            TrackMouseEvent(&eventoSeguimiento);
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            bool hoverAnterior = PtInRect(&estadoVistaCarpetas.botonAnteriorCarpeta, pt) != FALSE;
            bool hoverSiguiente = PtInRect(&estadoVistaCarpetas.botonSiguienteCarpeta, pt) != FALSE;
            bool hoverExpansion = PtInRect(&estadoVistaCarpetas.botonAlternarExpansion, pt) != FALSE;
            bool hoverFusion = PtInRect(&estadoVistaCarpetas.botonFusionCarpetas, pt) != FALSE;
            if (hoverAnterior != estadoVistaCarpetas.hoverBotonAnterior ||
                hoverSiguiente != estadoVistaCarpetas.hoverBotonSiguiente ||
                hoverExpansion != estadoVistaCarpetas.hoverBotonAlternarExpansion ||
                hoverFusion != estadoVistaCarpetas.hoverBotonFusion) {
                estadoVistaCarpetas.hoverBotonAnterior = hoverAnterior;
                estadoVistaCarpetas.hoverBotonSiguiente = hoverSiguiente;
                estadoVistaCarpetas.hoverBotonAlternarExpansion = hoverExpansion;
                estadoVistaCarpetas.hoverBotonFusion = hoverFusion;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
            TrackMouseEvent(&eventoSeguimiento);
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            bool hover = PtInRect(&estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias, pt) != FALSE;
            if (hover != estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias) {
                estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias = hover;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
            if (!estadoComprimidos.esModoComic && estadoComprimidos.archivosSeleccionados.size() > 1) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
                bool ant = PtInRect(&estadoComprimidos.botonAnteriorComprimido, pt) != FALSE;
                bool sig = PtInRect(&estadoComprimidos.botonSiguienteComprimido, pt) != FALSE;
                if (ant != estadoComprimidos.hoverBotonAnterior || sig != estadoComprimidos.hoverBotonSiguiente) {
                    estadoComprimidos.hoverBotonAnterior = ant;
                    estadoComprimidos.hoverBotonSiguiente = sig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT),
                                              TME_LEAVE, ventana, 0};
            TrackMouseEvent(&eventoSeguimiento);

            if (estadoTexto.esFormatoMarkdown) {
                POINT puntoRaton{GET_X_LPARAM(datosMensaje),
                                 GET_Y_LPARAM(datosMensaje)};
                int barraHoverDetectada = -1;
                for (size_t posicionBarra = 0;
                     posicionBarra < estadoTexto.barrasIndice.size();
                     ++posicionBarra) {
                    if (PtInRect(
                            &estadoTexto.barrasIndice[posicionBarra].areaVisualBarra,
                            puntoRaton)) {
                        barraHoverDetectada = static_cast<int>(posicionBarra);
                        break;
                    }
                }
                if (barraHoverDetectada != estadoTexto.posicionBarraHover) {
                    estadoTexto.posicionBarraHover = barraHoverDetectada;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
            }
            return 0;
        }
    }
    if (mensaje == WM_MOUSELEAVE || mensaje == WM_KILLFOCUS) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            if (estadoVistaCarpetas.hoverBotonAnterior ||
                estadoVistaCarpetas.hoverBotonSiguiente ||
                estadoVistaCarpetas.hoverBotonAlternarExpansion ||
                estadoVistaCarpetas.hoverBotonFusion) {
                estadoVistaCarpetas.hoverBotonAnterior = false;
                estadoVistaCarpetas.hoverBotonSiguiente = false;
                estadoVistaCarpetas.hoverBotonAlternarExpansion = false;
                estadoVistaCarpetas.hoverBotonFusion = false;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
            if (estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias) {
                estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias = false;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
            if (estadoComprimidos.hoverBotonAnterior || estadoComprimidos.hoverBotonSiguiente) {
                estadoComprimidos.hoverBotonAnterior = false;
                estadoComprimidos.hoverBotonSiguiente = false;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            if (estadoTexto.posicionBarraHover != -1) {
                estadoTexto.posicionBarraHover = -1;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
    }
    if (mensaje == WM_SETCURSOR) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(ventana, &pt);
            if (PtInRect(&estadoVistaCarpetas.botonAnteriorCarpeta, pt) ||
                PtInRect(&estadoVistaCarpetas.botonSiguienteCarpeta, pt) ||
                PtInRect(&estadoVistaCarpetas.botonAlternarExpansion, pt) ||
                PtInRect(&estadoVistaCarpetas.botonSelectorPrograma, pt) ||
                PtInRect(&estadoVistaCarpetas.botonEjecutarAbrir, pt) ||
                PtInRect(&estadoVistaCarpetas.botonFusionCarpetas, pt)) {
                SetCursor(LoadCursorW(nullptr, IDC_HAND));
                return TRUE;
            }
            if (configuracion.interaccionArchivosVistaCarpetas.load() &&
                obtenerElementoArbolEnPunto(pt) != nullptr) {
                SetCursor(LoadCursorW(nullptr, IDC_HAND));
                return TRUE;
            }
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(ventana, &pt);
            if (PtInRect(&estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias, pt)) {
                SetCursor(LoadCursorW(nullptr, IDC_HAND));
                return TRUE;
            }
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
            if (!estadoComprimidos.esModoComic) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(ventana, &pt);
                if (PtInRect(&estadoComprimidos.botonAnteriorComprimido, pt) ||
                    PtInRect(&estadoComprimidos.botonSiguienteComprimido, pt) ||
                    PtInRect(&estadoComprimidos.areaBotonDesbloquear, pt) ||
                    obtenerElementoArbolComprimidoEnPunto(pt) != nullptr) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            POINT puntoRaton;
            GetCursorPos(&puntoRaton);
            ScreenToClient(ventana, &puntoRaton);

            for (size_t posicionClicable = 0;
                 posicionClicable < estadoTexto.elementosClicables.size();
                 ++posicionClicable) {
                if (PtInRect(
                        &estadoTexto.elementosClicables[posicionClicable]
                             .areaClic,
                        puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }

            if (estadoTexto.esFormatoMarkdown) {
                for (size_t posicionBarra = 0;
                     posicionBarra < estadoTexto.barrasIndice.size();
                     ++posicionBarra) {
                    if (PtInRect(&estadoTexto.barrasIndice[posicionBarra]
                                      .areaVisualBarra,
                                 puntoRaton)) {
                        SetCursor(LoadCursorW(nullptr, IDC_HAND));
                        return TRUE;
                    }
                }
            }
        }
    }
    if (mensaje == WM_LBUTTONDOWN) {
        if (menuVelocidadDesplegado) {
            ocultarMenuVelocidadInmediato();
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            if (PtInRect(&estadoVistaCarpetas.botonAnteriorCarpeta, pt)) {
                if (mostrarCarpetaAnterior()) invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonSiguienteCarpeta, pt)) {
                if (mostrarCarpetaSiguiente()) invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonAlternarExpansion, pt)) {
                estadoVistaCarpetas.nivelExpansionActual =
                    (estadoVistaCarpetas.nivelExpansionActual + 1) % 4;
                invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonSelectorPrograma, pt)) {
                seleccionarYAgregarProgramaCarpeta(ventana);
                invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonEjecutarAbrir, pt)) {
                ejecutarAbrirCarpetasConPrograma(ventana);
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonFusionCarpetas, pt)) {
                if (ejecutarFusionCarpetas(ventana)) {
                    invalidarInterfaz();
                }
                return 0;
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            if (PtInRect(&estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias, pt)) {
                if (eliminarCarpetasVacias()) {
                    invalidarInterfaz();
                }
                return 0;
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            POINT puntoRaton{GET_X_LPARAM(datosMensaje),
                             GET_Y_LPARAM(datosMensaje)};

            for (size_t posicionClicable = 0;
                 posicionClicable < estadoTexto.elementosClicables.size();
                 ++posicionClicable) {
                if (PtInRect(
                        &estadoTexto.elementosClicables[posicionClicable]
                             .areaClic,
                        puntoRaton)) {
                    ShellExecuteW(
                        nullptr, L"open",
                        estadoTexto.elementosClicables[posicionClicable]
                            .destinoUrl.c_str(),
                        nullptr, nullptr, SW_SHOWNORMAL);
                    return 0;
                }
            }

            if (estadoTexto.esFormatoMarkdown) {
                for (size_t posicionBarra = 0;
                     posicionBarra < estadoTexto.barrasIndice.size();
                     ++posicionBarra) {
                    if (PtInRect(&estadoTexto.barrasIndice[posicionBarra]
                                      .areaVisualBarra,
                                 puntoRaton)) {
                        estadoTexto.desplazamientoScrollY =
                            estadoTexto.barrasIndice[posicionBarra]
                                .posicionDestinoY;
                        InvalidateRect(ventana, nullptr, FALSE);
                        return 0;
                    }
                }
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            RECT rc;
            GetClientRect(ventana, &rc);
            if (pt.x < (rc.right - rc.left) / 2) {
                if (mostrarImagenAnterior()) invalidarInterfaz();
            } else {
                if (mostrarImagenSiguiente()) invalidarInterfaz();
            }
            return 0;
        }
        if (tipoContenidoActivo.load() ==
            TipoContenidoArchivo::Comprimido) {
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            if (!estadoComprimidos.esModoComic) {
                if (estadoComprimidos.archivosSeleccionados.size() > 1) {
                    if (PtInRect(&estadoComprimidos.botonAnteriorComprimido, pt)) {
                        if (mostrarComprimidoAnterior()) invalidarInterfaz();
                        return 0;
                    }
                    if (PtInRect(&estadoComprimidos.botonSiguienteComprimido, pt)) {
                        if (mostrarComprimidoSiguiente()) invalidarInterfaz();
                        return 0;
                    }
                }
                if (estadoComprimidos.protegidoConContrasena && !estadoComprimidos.contrasenaDesbloqueada) {
                    if (PtInRect(&estadoComprimidos.areaBotonDesbloquear, pt)) {
                        estadoComprimidos.contrasenaDesbloqueada = true;
                        invalidarInterfaz();
                        return 0;
                    }
                }
            }
            return 0;
        }
        alternarReproduccion();
        return 0;
    }
    if (mensaje == WM_LBUTTONDBLCLK) {
        if (menuVelocidadDesplegado) {
            ocultarMenuVelocidadInmediato();
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            POINT puntoRaton{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            if (PtInRect(&estadoVistaCarpetas.botonAnteriorCarpeta, puntoRaton)) {
                if (mostrarCarpetaAnterior()) invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonSiguienteCarpeta, puntoRaton)) {
                if (mostrarCarpetaSiguiente()) invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonSelectorPrograma, puntoRaton)) {
                seleccionarYAgregarProgramaCarpeta(ventana);
                invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonEjecutarAbrir, puntoRaton)) {
                ejecutarAbrirCarpetasConPrograma(ventana);
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonFusionCarpetas, puntoRaton)) {
                if (ejecutarFusionCarpetas(ventana)) {
                    invalidarInterfaz();
                }
                return 0;
            }
            if (configuracion.interaccionArchivosVistaCarpetas.load()) {
                const ElementoArbolVistaCarpetas* elementoArbol = obtenerElementoArbolEnPunto(puntoRaton);
                if (elementoArbol) {
                    ejecutarAbrirElementoArbol(*elementoArbol);
                    return 0;
                }
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
            POINT puntoRaton{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            if (PtInRect(&estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias, puntoRaton)) {
                if (eliminarCarpetasVacias()) {
                    invalidarInterfaz();
                }
                return 0;
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
            POINT puntoRaton{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            RECT rectanguloCliente;
            GetClientRect(ventana, &rectanguloCliente);
            if (puntoRaton.x < (rectanguloCliente.right - rectanguloCliente.left) / 2) {
                if (mostrarImagenAnterior()) invalidarInterfaz();
            } else {
                if (mostrarImagenSiguiente()) invalidarInterfaz();
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
            if (!estadoComprimidos.esModoComic && estadoComprimidos.archivosSeleccionados.size() > 1) {
                POINT puntoRaton{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
                if (PtInRect(&estadoComprimidos.botonAnteriorComprimido, puntoRaton)) {
                    if (mostrarComprimidoAnterior()) invalidarInterfaz();
                    return 0;
                }
                if (PtInRect(&estadoComprimidos.botonSiguienteComprimido, puntoRaton)) {
                    if (mostrarComprimidoSiguiente()) invalidarInterfaz();
                    return 0;
                }
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
            alternarReproduccion();
            return 0;
        }
        return 0;
    }
    if (mensaje == WM_RBUTTONUP) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            if (configuracion.interaccionArchivosVistaCarpetas.load()) {
                POINT puntoRaton{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
                const ElementoArbolVistaCarpetas* elementoArbol = obtenerElementoArbolEnPunto(puntoRaton);
                if (elementoArbol) {
                    POINT puntoPantalla = puntoRaton;
                    ClientToScreen(ventana, &puntoPantalla);
                    mostrarMenuContextualElementoArbol(ventana, *elementoArbol, puntoPantalla);
                    return 0;
                }
            }
            return 0;
        }
        return 0;
    }
    if (mensaje == WM_PAINT) {
        PAINTSTRUCT pintura;
        HDC contextoPantalla = BeginPaint(ventana, &pintura);
        RECT areaCliente;
        GetClientRect(ventana, &areaCliente);
        int anchoCliente = areaCliente.right - areaCliente.left;
        int altoCliente = areaCliente.bottom - areaCliente.top;

        if (anchoCliente > 0 && altoCliente > 0) {
            HDC contextoMemoria = CreateCompatibleDC(contextoPantalla);
            HBITMAP mapaBitsMemoria = CreateCompatibleBitmap(
                contextoPantalla, anchoCliente, altoCliente);
            HBITMAP mapaBitsAnterior = static_cast<HBITMAP>(
                SelectObject(contextoMemoria, mapaBitsMemoria));

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                dibujarLienzoTexto(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Documento ||
                       tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Codigo) {
                dibujarLienzoPendienteImplementacion(
                    contextoMemoria, areaCliente, tipoContenidoActivo.load());
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Imagen) {
                dibujarLienzoImagen(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Comprimido) {
                dibujarLienzoComprimidos(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::VistaCarpetas) {
                dibujarLienzoVistaCarpetas(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::VistaVacia) {
                dibujarLienzoVistaVacia(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Desconocido) {
                dibujarLienzoArchivoDesconocido(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Ninguno) {
                HBRUSH brochaFondo = CreateSolidBrush(COLOR_FONDO_REPRODUCTOR);
                FillRect(contextoMemoria, &areaCliente, brochaFondo);
                DeleteObject(brochaFondo);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            }

            SelectObject(contextoMemoria, mapaBitsAnterior);
            DeleteObject(mapaBitsMemoria);
            DeleteDC(contextoMemoria);
        }
        EndPaint(ventana, &pintura);
        return 0;
    }
    if (mensaje == WM_TIMER) {
        if (parametroMensaje == ID_TEMPORIZADOR_ANIMACION_GIF) {
            avanzarCuadroAnimacionGif(ventana);
            return 0;
        }
    }
    if (mensaje == WM_DESTROY) {
        KillTimer(ventana, ID_TEMPORIZADOR_ANIMACION_GIF);
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaMenuVelocidad(HWND ventana, UINT mensaje,
                                              WPARAM parametroMensaje,
                                              LPARAM datosMensaje) {
    switch (mensaje) {
        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT),
                                              TME_LEAVE, ventana, 0};
            TrackMouseEvent(&eventoSeguimiento);

            int posicionVertical = GET_Y_LPARAM(datosMensaje);
            int nuevaOpcion = (posicionVertical - 4) / ALTO_OPCION_VELOCIDAD;
            if (nuevaOpcion < 0 || nuevaOpcion >= TOTAL_OPCIONES_VELOCIDAD) {
                nuevaOpcion = -1;
            }
            if (nuevaOpcion != opcionVelocidadResaltada) {
                opcionVelocidadResaltada = nuevaOpcion;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        case WM_MOUSELEAVE: {
            opcionVelocidadResaltada = -1;
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int posicionVertical = GET_Y_LPARAM(datosMensaje);
            int opcionElegida = (posicionVertical - 4) / ALTO_OPCION_VELOCIDAD;
            if (opcionElegida >= 0 && opcionElegida < TOTAL_OPCIONES_VELOCIDAD) {
                cambiarVelocidadReproduccion(VALORES_VELOCIDAD[opcionElegida]);
                opcionVelocidadResaltada = opcionElegida;
                // Contraer el panel de opciones de velocidad inmediatamente
                ocultarMenuVelocidadInmediato();
            }
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            dibujarMenuVelocidadFlotante(ventana);
            return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

bool registrarClasesDeVentana() {
    WNDCLASSEXW descripcionClase{sizeof(descripcionClase)};
    descripcionClase.hInstance = GetModuleHandleW(nullptr);
    descripcionClase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    descripcionClase.style = CS_DBLCLKS;

    descripcionClase.lpfnWndProc = procesarVentanaReproductor;
    descripcionClase.lpszClassName = CLASE_VENTANA_REPRODUCTOR;
    ATOM atomoClaseReproductor = RegisterClassExW(&descripcionClase);
    if (!atomoClaseReproductor &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    descripcionClase.style = CS_DBLCLKS;
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

    descripcionClase.style = CS_DBLCLKS;
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

    descripcionClase.style = 0;
    descripcionClase.lpfnWndProc = procesarVentanaMenuVelocidad;
    descripcionClase.hbrBackground = nullptr;
    descripcionClase.lpszClassName = CLASE_VENTANA_MENU_VELOCIDAD;
    ATOM atomoClaseMenuVelocidad = RegisterClassExW(&descripcionClase);
    if (!atomoClaseMenuVelocidad &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        UnregisterClassW(CLASE_VENTANA_CONTROLES, descripcionClase.hInstance);
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

    UnregisterClassW(CLASE_VENTANA_MENU_VELOCIDAD, descripcionClase.hInstance);
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
    UnregisterClassW(CLASE_VENTANA_MENU_VELOCIDAD, instancia);
    UnregisterClassW(CLASE_VENTANA_DELEGADO, instancia);
}
