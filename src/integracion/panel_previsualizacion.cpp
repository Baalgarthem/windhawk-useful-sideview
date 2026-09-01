struct DatosBusquedaVistaArchivos {
    HWND mejorVistaArchivos{nullptr};
    int mayorAreaPixeles{0};
};

BOOL CALLBACK encontrarVistaDeArchivos(HWND ventanaCandidata,
                                       LPARAM direccionResultado) {
    auto* datosBusqueda = reinterpret_cast<DatosBusquedaVistaArchivos*>(direccionResultado);
    wchar_t nombreClase[64];
    if (GetClassNameW(ventanaCandidata, nombreClase, ARRAYSIZE(nombreClase)) &&
        lstrcmpW(nombreClase, L"SHELLDLL_DefView") == 0 &&
        IsWindowVisible(ventanaCandidata)) {
        RECT rectanguloCandidata;
        GetWindowRect(ventanaCandidata, &rectanguloCandidata);
        int anchoCandidata = rectanguloCandidata.right - rectanguloCandidata.left;
        int altoCandidata = rectanguloCandidata.bottom - rectanguloCandidata.top;
        int areaCandidata = anchoCandidata * altoCandidata;
        if (anchoCandidata > 50 && altoCandidata > 50 && areaCandidata > datosBusqueda->mayorAreaPixeles) {
            datosBusqueda->mayorAreaPixeles = areaCandidata;
            datosBusqueda->mejorVistaArchivos = ventanaCandidata;
        }
    }
    return TRUE;
}

bool obtenerAreaPanel(RECT* areaPanel);
bool ajustarReproductorAlPanel(const RECT* areaPanelCalculada);

LRESULT CALLBACK procesarSubclaseVistaArchivos(HWND hwnd, UINT uMsg,
                                              WPARAM wParam, LPARAM lParam,
                                              UINT_PTR uIdSubclass,
                                              DWORD_PTR dwRefData) {
    if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) {
        if (wParam == 'P' || wParam == 'p') {
            bool ctrlPresionado = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            bool altPresionado = (GetKeyState(VK_MENU) & 0x8000) != 0;
            if (ctrlPresionado || altPresionado) {
                HWND ventanaAnfitriona = ventanaExploradorAnfitriona ? ventanaExploradorAnfitriona : GetAncestor(hwnd, GA_ROOT);
                if (ventanaAnfitriona && IsWindow(ventanaAnfitriona)) {
                    // 41489 (0xA211) es el comando nativo de Explorer para alternar el panel de previsualización
                    PostMessageW(ventanaAnfitriona, WM_COMMAND, 41489, 0);
                    return 0;
                }
            }
        }
    }
    LRESULT resultado = DefSubclassProc(hwnd, uMsg, wParam, lParam);
    if (uMsg == WM_WINDOWPOSCHANGED || uMsg == WM_SIZE || uMsg == WM_SHOWWINDOW) {
        if (ventanaReproductor && IsWindow(ventanaReproductor)) {
            RECT areaPanel{};
            if (obtenerAreaPanel(&areaPanel)) {
                ajustarReproductorAlPanel(&areaPanel);
            }
        }
    } else if (uMsg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, procesarSubclaseVistaArchivos, uIdSubclass);
    }
    return resultado;
}

inline void asegurarSubclaseVistaArchivos(HWND vistaArchivos) {
    if (vistaArchivos && IsWindow(vistaArchivos)) {
        SetWindowSubclass(vistaArchivos, procesarSubclaseVistaArchivos,
                          ID_SUBCLASE_VISTA_ARCHIVOS, 0);
    }
}

bool obtenerAreaPanel(RECT* areaPanel) {
    if (!areaPanel) return false;

    if (!ventanaExploradorAnfitriona || !IsWindow(ventanaExploradorAnfitriona) ||
        !IsWindowVisible(ventanaExploradorAnfitriona)) {
        DWORD procesoActual = GetCurrentProcessId();
        HWND ventanaFoco = GetForegroundWindow();
        if (ventanaFoco) {
            DWORD identificadorProcesoFoco = 0;
            GetWindowThreadProcessId(ventanaFoco, &identificadorProcesoFoco);
            if (identificadorProcesoFoco == procesoActual) {
                HWND ventanaNormalizada = normalizarVentanaExploradorAnfitriona(ventanaFoco);
                if (ventanaNormalizada && IsWindowVisible(ventanaNormalizada)) {
                    ventanaExploradorAnfitriona = ventanaNormalizada;
                }
            }
        }
        if (!ventanaExploradorAnfitriona || !IsWindow(ventanaExploradorAnfitriona)) {
            HWND ventanaCandidataExplorador = FindWindowExW(nullptr, nullptr, L"CabinetWClass", nullptr);
            while (ventanaCandidataExplorador) {
                DWORD identificadorProcesoCandidata = 0;
                GetWindowThreadProcessId(ventanaCandidataExplorador, &identificadorProcesoCandidata);
                if (identificadorProcesoCandidata == procesoActual && IsWindowVisible(ventanaCandidataExplorador)) {
                    ventanaExploradorAnfitriona = normalizarVentanaExploradorAnfitriona(ventanaCandidataExplorador);
                    break;
                }
                ventanaCandidataExplorador = FindWindowExW(nullptr, ventanaCandidataExplorador, L"CabinetWClass", nullptr);
            }
        }
    }

    if (!ventanaExploradorAnfitriona || !IsWindow(ventanaExploradorAnfitriona) ||
        !IsWindowVisible(ventanaExploradorAnfitriona)) {
        return false;
    }

    HWND vistaArchivos = ventanaVistaArchivosActiva;
    if (!vistaArchivos || !IsWindow(vistaArchivos) || !IsWindowVisible(vistaArchivos)) {
        DatosBusquedaVistaArchivos datosBusqueda;
        EnumChildWindows(ventanaExploradorAnfitriona, encontrarVistaDeArchivos,
                         reinterpret_cast<LPARAM>(&datosBusqueda));
        vistaArchivos = datosBusqueda.mejorVistaArchivos;
    }
    if (!vistaArchivos) return false;

    asegurarSubclaseVistaArchivos(vistaArchivos);

    RECT areaContenedorPantalla;
    RECT areaVistaPantalla;
    GetWindowRect(ventanaExploradorAnfitriona, &areaContenedorPantalla);
    GetWindowRect(vistaArchivos, &areaVistaPantalla);

    constexpr int ANCHO_MINIMO_PANEL = 100;
    constexpr int MARGEN_SEPARADOR_EXPLORER = 4;
    int anchoPanelDetectado = areaContenedorPantalla.right - (areaVistaPantalla.right + MARGEN_SEPARADOR_EXPLORER);
    if (anchoPanelDetectado < ANCHO_MINIMO_PANEL) return false;

    RECT rectPanelPantalla = {
        areaVistaPantalla.right + MARGEN_SEPARADOR_EXPLORER,
        areaVistaPantalla.top,
        areaContenedorPantalla.right,
        areaVistaPantalla.bottom
    };

    MapWindowPoints(HWND_DESKTOP, ventanaExploradorAnfitriona,
                    reinterpret_cast<POINT*>(&rectPanelPantalla), 2);

    *areaPanel = rectPanelPantalla;
    return true;
}

inline bool debeBloquearTamanoParaTipo(TipoContenidoArchivo tipoContenido) {
    switch (tipoContenido) {
        case TipoContenidoArchivo::Video:
            return configuracion.bloquearTamanoVideo.load();
        case TipoContenidoArchivo::Imagen:
            return configuracion.bloquearTamanoImagen.load();
        case TipoContenidoArchivo::Comprimido:
            return configuracion.bloquearTamanoComprimidos.load();
        case TipoContenidoArchivo::Texto:
            return configuracion.bloquearTamanoTexto.load();
        case TipoContenidoArchivo::Documento:
            return configuracion.bloquearTamanoDocumentos.load();
        case TipoContenidoArchivo::Codigo:
            return configuracion.bloquearTamanoCodigo.load();
        case TipoContenidoArchivo::VistaCarpetas:
            return configuracion.bloquearTamanoVistaCarpetas.load();
        case TipoContenidoArchivo::VistaVacia:
            return configuracion.bloquearTamanoVistaVacia.load();
        case TipoContenidoArchivo::Desconocido:
            return configuracion.bloquearTamanoDesconocido.load();
        default:
            return false;
    }
}

bool ajustarReproductorAlPanel(const RECT* areaPanelCalculada = nullptr) {
    if (!ventanaReproductor) return false;
    RECT areaPanel;
    if (areaPanelCalculada) {
        areaPanel = *areaPanelCalculada;
    } else if (!obtenerAreaPanel(&areaPanel)) {
        ShowWindow(ventanaReproductor, SW_HIDE);
        return false;
    }

    if (GetParent(ventanaReproductor) != ventanaExploradorAnfitriona) {
        SetParent(ventanaReproductor, ventanaExploradorAnfitriona);
    }

    RECT areaActual;
    GetWindowRect(ventanaReproductor, &areaActual);
    MapWindowPoints(HWND_DESKTOP, ventanaExploradorAnfitriona,
                    reinterpret_cast<POINT*>(&areaActual), 2);

    int nuevoAncho = areaPanel.right - areaPanel.left;
    int nuevoAlto = areaPanel.bottom - areaPanel.top;
    int anchoActual = areaActual.right - areaActual.left;
    int altoActual = areaActual.bottom - areaActual.top;

    bool bloqueoActivo = debeBloquearTamanoParaTipo(tipoContenidoActivo.load());
    if (bloqueoActivo && anchoActual > 0 && altoActual > 0) {
        SetWindowPos(ventanaReproductor, HWND_TOP, areaPanel.left, areaPanel.top,
                     anchoActual, altoActual,
                     SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS);
        ajustarLayoutVentanasReproductor(ventanaReproductor);
        return true;
    }

    SetWindowPos(ventanaReproductor, HWND_TOP, areaPanel.left, areaPanel.top,
                 nuevoAncho, nuevoAlto,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS);

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
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    tipoContenidoActivo.store(TipoContenidoArchivo::Ninguno);

    if (ventanaMenuVelocidad) {
        DestroyWindow(ventanaMenuVelocidad);
        ventanaMenuVelocidad = nullptr;
    }
    menuVelocidadDesplegado = false;
    animandoCierreMenuVelocidad = false;
    opacidadMenuVelocidad = 255;
    opcionVelocidadResaltada = -1;

    if (ventanaVistaArchivosActiva && IsWindow(ventanaVistaArchivosActiva)) {
        RemoveWindowSubclass(ventanaVistaArchivosActiva, procesarSubclaseVistaArchivos,
                             ID_SUBCLASE_VISTA_ARCHIVOS);
    }
    ventanaVistaArchivosActiva = nullptr;

    if (ventanaReproductor) DestroyWindow(ventanaReproductor);

    ventanaReproductor = nullptr;
    ventanaVideo = nullptr;
    ventanaControles = nullptr;
    arrastrandoBarraProgreso = false;
    proporcionProgreso = 0.0;
    reproducirTrasCambioPosicion.store(false);
    reiniciarCacheTiempoReproduccion();
}

bool asegurarVentanasContenedoras() {
    RECT areaPanel;
    if (!obtenerAreaPanel(&areaPanel)) return false;

    if (ventanaReproductor && IsWindow(ventanaReproductor)) {
        if (GetParent(ventanaReproductor) != ventanaExploradorAnfitriona) {
            SetParent(ventanaReproductor, ventanaExploradorAnfitriona);
        }
        return true;
    }

    if (!ventanaReproductor) {
        ventanaReproductor = CreateWindowExW(
            0, CLASE_VENTANA_REPRODUCTOR, L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS |
                WS_CLIPCHILDREN,
            areaPanel.left, areaPanel.top,
            areaPanel.right - areaPanel.left,
            areaPanel.bottom - areaPanel.top,
            ventanaExploradorAnfitriona, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (!ventanaReproductor) return false;

        ventanaVideo = CreateWindowExW(
            0, CLASE_VENTANA_VIDEO, L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 0, 0, ventanaReproductor, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (!ventanaVideo) {
            liberarRecursosDelReproductor();
            return false;
        }

        ventanaControles = CreateWindowExW(
            0, CLASE_VENTANA_CONTROLES, L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 0, 0, ventanaReproductor,
            nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!ventanaControles) {
            liberarRecursosDelReproductor();
            return false;
        }

        ventanaMenuVelocidad = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            CLASE_VENTANA_MENU_VELOCIDAD, L"",
            WS_POPUP | WS_CLIPSIBLINGS,
            0, 0, ANCHO_PANEL_VELOCIDAD, ALTO_PANEL_VELOCIDAD,
            ventanaReproductor, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (ventanaMenuVelocidad) {
            SetLayeredWindowAttributes(ventanaMenuVelocidad, 0, 255, LWA_ALPHA);
        }

        if (!SetTimer(ventanaControles, ID_TEMPORIZADOR_INTERFAZ,
                      INTERVALO_REFRESCO_MS, nullptr)) {
            liberarRecursosDelReproductor();
            return false;
        }
    }
    return true;
}

inline void detenerYLiberarReproductorMfSiExiste() {
    bool reproductorEstabaActivo = (reproductor != nullptr);
    if (reproductor) {
        reproductor->Shutdown();
        reproductor->Release();
        reproductor = nullptr;
    }
    if (receptorEventos) {
        receptorEventos->Release();
        receptorEventos = nullptr;
    }
    reiniciarCacheTiempoReproduccion();
    if (reproductorEstabaActivo && ventanaReproductor && IsWindow(ventanaReproductor)) {
        if (ventanaVideo && IsWindow(ventanaVideo)) {
            DestroyWindow(ventanaVideo);
            ventanaVideo = nullptr;
        }
        RECT areaCliente;
        GetClientRect(ventanaReproductor, &areaCliente);
        ventanaVideo = CreateWindowExW(
            0, CLASE_VENTANA_VIDEO, L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, areaCliente.right, areaCliente.bottom,
            ventanaReproductor, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (ventanaVideo) {
            InvalidateRect(ventanaVideo, nullptr, FALSE);
        }
    }
}

inline void liberarRecursosDeTodasCategorias() {
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    liberarRecursosDesconocido();
    tipoContenidoActivo.store(TipoContenidoArchivo::Ninguno);
}

bool asegurarReproductor(const std::vector<std::wstring>& listaVideos) {
    if (listaVideos.empty()) return false;
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    tipoContenidoActivo.store(TipoContenidoArchivo::Video);

    if (!asegurarVentanasContenedoras()) return false;

    if (!reproductor) {
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
    }

    if (!reproductor || !ajustarReproductorAlPanel()) return false;
    listaVideosSeleccionados = listaVideos;
    std::sort(listaVideosSeleccionados.begin(), listaVideosSeleccionados.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    indiceVideoSeleccionado = 0;
    reproductor->Stop();
    reproductor->SetMute(!configuracion.sonidoHabilitado.load());
    reiniciarCacheTiempoReproduccion();
    HRESULT resultadoCarga = reproductor->CreateMediaItemFromURL(
        listaVideosSeleccionados.front().c_str(), FALSE, 0, nullptr);
    invalidarInterfaz();
    return SUCCEEDED(resultadoCarga);
}

bool asegurarVisorImagen(const std::vector<std::wstring>& listaImagenes) {
    if (listaImagenes.empty()) return false;
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    tipoContenidoActivo.store(TipoContenidoArchivo::Imagen);

    if (!asegurarVentanasContenedoras()) return false;
    estadoImagen.imagenesSeleccionadas = listaImagenes;
    std::sort(estadoImagen.imagenesSeleccionadas.begin(), estadoImagen.imagenesSeleccionadas.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    estadoImagen.indiceImagenSeleccionada = 0;
    if (!cargarImagenPorRuta(estadoImagen.imagenesSeleccionadas.front())) return false;
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorTexto(const std::vector<std::wstring>& listaTextos) {
    if (listaTextos.empty()) return false;
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosImagen();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    tipoContenidoActivo.store(TipoContenidoArchivo::Texto);

    if (!asegurarVentanasContenedoras()) return false;
    estadoTexto.textosSeleccionados = listaTextos;
    std::sort(estadoTexto.textosSeleccionados.begin(), estadoTexto.textosSeleccionados.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    estadoTexto.indiceTextoSeleccionado = 0;
    if (!cargarTextoPorRuta(estadoTexto.textosSeleccionados.front())) return false;
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorPendiente(const std::vector<std::wstring>& listaArchivos,
                            TipoContenidoArchivo tipo) {
    if (listaArchivos.empty()) return false;
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    tipoContenidoActivo.store(tipo);

    if (!asegurarVentanasContenedoras()) return false;

    EstadoPendienteImplementacion& estado =
        (tipo == TipoContenidoArchivo::Documento) ? estadoDocumentos : estadoCodigo;
    estado.archivosSeleccionados = listaArchivos;
    std::sort(estado.archivosSeleccionados.begin(), estado.archivosSeleccionados.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    estado.indiceActual = 0;
    estado.rutaActual = estado.archivosSeleccionados.front();
    estado.nombreArchivo = PathFindFileNameW(estado.rutaActual.c_str());
    estado.extension = PathFindExtensionW(estado.rutaActual.c_str());

    if (!ajustarReproductorAlPanel()) return false;
    invalidarInterfaz();
    return true;
}

bool asegurarVisorComprimidos(
    const std::vector<std::wstring>& listaArchivos) {
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosVistaCarpetas();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    tipoContenidoActivo.store(TipoContenidoArchivo::Comprimido);

    if (!asegurarVentanasContenedoras()) return false;
    std::vector<std::wstring> listaOrdenada = listaArchivos;
    std::sort(listaOrdenada.begin(), listaOrdenada.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    if (!cargarArchivosComprimidos(listaOrdenada)) return false;
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorVistaCarpetas(
    const std::vector<std::wstring>& listaCarpetas) {
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    tipoContenidoActivo.store(TipoContenidoArchivo::VistaCarpetas);

    if (!asegurarVentanasContenedoras()) return false;
    estadoVistaCarpetas.carpetasSeleccionadas = listaCarpetas;
    std::sort(estadoVistaCarpetas.carpetasSeleccionadas.begin(), estadoVistaCarpetas.carpetasSeleccionadas.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    if (!cargarCarpetaPorIndice(0)) return false;
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorVistaVacia(const std::wstring& rutaCarpeta) {
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    tipoContenidoActivo.store(TipoContenidoArchivo::VistaVacia);

    if (!asegurarVentanasContenedoras()) return false;
    calcularResumenVistaVacia(rutaCarpeta);
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorDesconocido(const std::vector<std::wstring>& listaArchivos) {
    if (listaArchivos.empty()) return false;
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    liberarRecursosDesconocido();
    tipoContenidoActivo.store(TipoContenidoArchivo::Desconocido);

    if (!asegurarVentanasContenedoras()) return false;

    estadoDesconocido.archivosSeleccionados = listaArchivos;
    std::sort(estadoDesconocido.archivosSeleccionados.begin(), estadoDesconocido.archivosSeleccionados.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    estadoDesconocido.indiceActual = 0;
    estadoDesconocido.rutaActual = estadoDesconocido.archivosSeleccionados.front();
    estadoDesconocido.nombreArchivo = PathFindFileNameW(estadoDesconocido.rutaActual.c_str());
    estadoDesconocido.extension = PathFindExtensionW(estadoDesconocido.rutaActual.c_str());

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

    if (!ajustarReproductorAlPanel()) return false;
    invalidarInterfaz();
    return true;
}

bool asegurarVisualizadorParaArchivos(
    const std::vector<std::wstring>& listaArchivos) {
    if (listaArchivos.empty()) return false;

    TipoContenidoArchivo tipoSeleccion = clasificarSeleccionArchivos(listaArchivos);

    if (tipoSeleccion == TipoContenidoArchivo::VistaCarpetas) {
        std::vector<std::wstring> listaCarpetas;
        for (const auto& ruta : listaArchivos) {
            if (esRutaCarpeta(ruta.c_str())) listaCarpetas.push_back(ruta);
        }
        return asegurarVisorVistaCarpetas(!listaCarpetas.empty() ? listaCarpetas : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::VistaVacia) {
        return asegurarVisorVistaVacia(listaArchivos.front());
    }
    if (tipoSeleccion == TipoContenidoArchivo::Comprimido) {
        std::vector<std::wstring> listaComprimidos;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionComprimido(ext)) listaComprimidos.push_back(ruta);
        }
        return asegurarVisorComprimidos(!listaComprimidos.empty() ? listaComprimidos : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Video) {
        std::vector<std::wstring> listaVideos;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionVideo(ext)) listaVideos.push_back(ruta);
        }
        return asegurarReproductor(!listaVideos.empty() ? listaVideos : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Imagen) {
        std::vector<std::wstring> listaImagenes;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionImagen(ext)) listaImagenes.push_back(ruta);
        }
        return asegurarVisorImagen(!listaImagenes.empty() ? listaImagenes : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Texto) {
        std::vector<std::wstring> listaTextos;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionTexto(ext)) listaTextos.push_back(ruta);
        }
        return asegurarVisorTexto(!listaTextos.empty() ? listaTextos : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Documento) {
        std::vector<std::wstring> listaDocumentos;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionDocumento(ext)) listaDocumentos.push_back(ruta);
        }
        return asegurarVisorPendiente(!listaDocumentos.empty() ? listaDocumentos : listaArchivos,
                                      TipoContenidoArchivo::Documento);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Codigo) {
        std::vector<std::wstring> listaCodigo;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionCodigo(ext)) listaCodigo.push_back(ruta);
        }
        return asegurarVisorPendiente(!listaCodigo.empty() ? listaCodigo : listaArchivos,
                                      TipoContenidoArchivo::Codigo);
    }
    return asegurarVisorDesconocido(listaArchivos);
}

