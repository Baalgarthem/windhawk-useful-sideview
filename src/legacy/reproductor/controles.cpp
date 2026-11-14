void reiniciarCacheTiempoReproduccion() {
    posicionReproduccionCache.store(0);
    duracionReproduccionCache.store(0);
    tiempoReproduccionDisponible.store(false);
    cambioPosicionPendiente.store(false);
    posicionamientoDisponible.store(false);
}

void invalidarInterfazControles() {
    if (ventanaControles) {
        InvalidateRect(ventanaControles, nullptr, FALSE);
    }
}

// Solo válido en el hilo reproductor: ahí vive IMFPMediaPlayer y COM responde
// correctamente a GetPosition y GetDuration.
bool leerTiempoDesdeReproductor(LONGLONG* posicionActual,
                                LONGLONG* duracionTotal) {
    if (!reproductor || !posicionActual || !duracionTotal) return false;

    PROPVARIANT propiedadPosicion;
    PROPVARIANT propiedadDuracion;
    PropVariantInit(&propiedadPosicion);
    PropVariantInit(&propiedadDuracion);
    HRESULT resultadoPosicion = reproductor->GetPosition(
        MFP_POSITIONTYPE_100NS, &propiedadPosicion);
    HRESULT resultadoDuracion = reproductor->GetDuration(
        MFP_POSITIONTYPE_100NS, &propiedadDuracion);
    LONGLONG posicionExtraida = 0;
    LONGLONG duracionExtraida = 0;
    bool tiempoDisponible =
        SUCCEEDED(resultadoPosicion) && SUCCEEDED(resultadoDuracion) &&
        extraerPosicionMediaFoundation(propiedadPosicion,
                                        &posicionExtraida) &&
        extraerDuracionMediaFoundation(propiedadDuracion,
                                        &duracionExtraida) &&
        duracionExtraida > 0;
    if (tiempoDisponible) {
        *posicionActual = posicionExtraida;
        *duracionTotal = duracionExtraida;
    }
    PropVariantClear(&propiedadPosicion);
    PropVariantClear(&propiedadDuracion);
    return tiempoDisponible;
}

void actualizarCacheTiempoReproduccion() {
    if (!reproductor) {
        reiniciarCacheTiempoReproduccion();
        return;
    }

    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    if (!leerTiempoDesdeReproductor(&posicionActual, &duracionTotal)) {
        return;
    }

    bool teniaTiempo = tiempoReproduccionDisponible.load();
    long long posicionAnterior = posicionReproduccionCache.load();
    posicionReproduccionCache.store(posicionActual);
    duracionReproduccionCache.store(duracionTotal);
    tiempoReproduccionDisponible.store(true);

    long long deltaPosicion = posicionActual - posicionAnterior;
    if (deltaPosicion < 0) {
        deltaPosicion = -deltaPosicion;
    }
    constexpr long long umbralActualizacion = 500000LL;
    if (ventanaControles &&
        (!teniaTiempo || deltaPosicion >= umbralActualizacion)) {
        PostMessageW(ventanaControles, MENSAJE_ACTUALIZAR_INTERFAZ, 0, 0);
    }
}

bool obtenerTiempoReproduccionCache(LONGLONG* posicionActual,
                                    LONGLONG* duracionTotal) {
    if (!tiempoReproduccionDisponible.load() || !posicionActual ||
        !duracionTotal) {
        return false;
    }
    *posicionActual = posicionReproduccionCache.load();
    *duracionTotal = duracionReproduccionCache.load();
    return *duracionTotal > 0;
}

bool aplicarPosicionAlReproductor(LONGLONG posicionObjetivo,
                                  bool reproducirAlCompletar) {
    if (!reproductor) return false;

    PROPVARIANT propiedadPosicion;
    PropVariantInit(&propiedadPosicion);
    propiedadPosicion.vt = VT_I8;
    propiedadPosicion.hVal.QuadPart = posicionObjetivo;

    reproducirTrasCambioPosicion.store(reproducirAlCompletar);
    HRESULT resultadoPosicion = reproductor->SetPosition(
        MFP_POSITIONTYPE_100NS, &propiedadPosicion);
    PropVariantClear(&propiedadPosicion);

    if (FAILED(resultadoPosicion)) {
        reproducirTrasCambioPosicion.store(false);
        Wh_Log(L"No fue posible cambiar la posición (HRESULT 0x%08X)",
               static_cast<unsigned int>(resultadoPosicion));
        return false;
    }

    invalidarInterfazControles();
    return true;
}

// Respaldo en el hilo reproductor cuando la caché todavía no tiene duración.
bool buscarEnReproductorDesdeHilo(double proporcionObjetivo,
                                  bool reproducirAlCompletar = false) {
    if (!reproductor) return false;

    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    if (!leerTiempoDesdeReproductor(&posicionActual, &duracionTotal)) {
        return false;
    }

    double proporcionLimitada =
        limitarProporcionProgreso(proporcionObjetivo);
    LONGLONG posicionObjetivo = calcularPosicionTemporal(
        duracionTotal, proporcionLimitada);
    if (!aplicarPosicionAlReproductor(posicionObjetivo, reproducirAlCompletar)) {
        return false;
    }

    duracionReproduccionCache.store(duracionTotal);
    tiempoReproduccionDisponible.store(true);
    return true;
}

// La caché evita consultar la duración durante cada interacción. Si todavía
// no está preparada, el delegado ejecuta una única lectura en el hilo dueño.
bool buscarEnReproduccion(double proporcionObjetivo,
                          bool reproducirAlCompletar = false) {
    if (!reproductor || !posicionamientoDisponible.load()) return false;

    double proporcionLimitada =
        limitarProporcionProgreso(proporcionObjetivo);
    proporcionPosicionSolicitada.store(proporcionLimitada);
    cambioPosicionPendiente.store(true);
    LONGLONG duracionTotal = duracionReproduccionCache.load();
    if (duracionTotal <= 0) {
        reproducirTrasCambioPosicion.store(reproducirAlCompletar);
        if (ventanaDelegadoReproductor &&
            IsWindow(ventanaDelegadoReproductor)) {
            bool busquedaAplicada =
                SendMessageW(ventanaDelegadoReproductor,
                             MENSAJE_EJECUTAR_BUSQUEDA, 0, 0) != 0;
            if (!busquedaAplicada) cambioPosicionPendiente.store(false);
            return busquedaAplicada;
        }
        cambioPosicionPendiente.store(false);
        return false;
    }

    LONGLONG posicionObjetivo =
        calcularPosicionTemporal(duracionTotal, proporcionLimitada);
    bool busquedaAplicada =
        aplicarPosicionAlReproductor(posicionObjetivo,
                                     reproducirAlCompletar);
    if (!busquedaAplicada) cambioPosicionPendiente.store(false);
    return busquedaAplicada;
}

void alternarReproduccion() {
    if (!reproductor) return;
    MFP_MEDIAPLAYER_STATE estadoReproduccion;
    if (SUCCEEDED(reproductor->GetState(&estadoReproduccion)) &&
        estadoReproduccion == MFP_MEDIAPLAYER_STATE_PLAYING) {
        reproductor->Pause();
    } else {
        reproductor->Play();
    }
    invalidarInterfazControles();
}

void alternarSonido() {
    if (!reproductor) return;
    BOOL sonidoSilenciado = TRUE;
    reproductor->GetMute(&sonidoSilenciado);
    reproductor->SetMute(!sonidoSilenciado);
    invalidarInterfazControles();
}

void alternarRepeticion() {
    repeticionHabilitada.store(!repeticionHabilitada.load());
    invalidarInterfazControles();
}

inline void actualizarMetadatosVideo(const std::wstring& rutaVideo) {
    if (rutaVideo.empty()) {
        liberarRecursosVideo();
        return;
    }

    estadoVideoMetadatos.rutaVideo = rutaVideo;
    estadoVideoMetadatos.nombreArchivo = PathFindFileNameW(rutaVideo.c_str());
    const wchar_t* extension = PathFindExtensionW(rutaVideo.c_str());
    estadoVideoMetadatos.extensionFormato = extension ? extension : L"";

    WIN32_FILE_ATTRIBUTE_DATA datosArchivo{};
    if (GetFileAttributesExW(rutaVideo.c_str(), GetFileExInfoStandard, &datosArchivo)) {
        ULARGE_INTEGER tamanoGrande{};
        tamanoGrande.LowPart = datosArchivo.nFileSizeLow;
        tamanoGrande.HighPart = datosArchivo.nFileSizeHigh;
        wchar_t buferTamano[64]{};
        StrFormatByteSizeW(static_cast<LONGLONG>(tamanoGrande.QuadPart), buferTamano, ARRAYSIZE(buferTamano));
        estadoVideoMetadatos.textoTamano = buferTamano;

        FILETIME ftLocal{};
        FileTimeToLocalFileTime(&datosArchivo.ftLastWriteTime, &ftLocal);
        SYSTEMTIME st{};
        FileTimeToSystemTime(&ftLocal, &st);
        wchar_t buferFecha[64]{};
        swprintf_s(buferFecha, L"%02u/%02u/%04u %02u:%02u",
                   st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
        estadoVideoMetadatos.textoFechaModificacion = buferFecha;
    } else {
        estadoVideoMetadatos.textoTamano = L"";
        estadoVideoMetadatos.textoFechaModificacion = L"";
    }

    std::vector<std::wstring> videosCarpeta = obtenerVideosEnMismaCarpeta(rutaVideo);
    estadoVideoMetadatos.totalEnCarpeta = static_cast<int>(videosCarpeta.size());
    auto iteradorEnCarpeta = std::find(videosCarpeta.begin(), videosCarpeta.end(), rutaVideo);
    if (iteradorEnCarpeta != videosCarpeta.end()) {
        estadoVideoMetadatos.indiceEnCarpeta = static_cast<int>(std::distance(videosCarpeta.begin(), iteradorEnCarpeta));
    } else {
        estadoVideoMetadatos.indiceEnCarpeta = 0;
    }

    if (listaVideosSeleccionados.size() > 1) {
        estadoVideoMetadatos.esRangoSeleccionado = true;
        estadoVideoMetadatos.totalEnRango = static_cast<int>(listaVideosSeleccionados.size());
        estadoVideoMetadatos.indiceEnRango = static_cast<int>(indiceVideoSeleccionado);
    } else {
        estadoVideoMetadatos.esRangoSeleccionado = false;
        estadoVideoMetadatos.totalEnRango = 1;
        estadoVideoMetadatos.indiceEnRango = 0;
    }
}

inline void activarParpadeoBotonesNavVideo() {
    estadoVideoMetadatos.tiempoInicioParpadeoNav = GetTickCount64();
    if (ventanaControles && IsWindow(ventanaControles)) {
        SetTimer(ventanaControles, ID_TEMPORIZADOR_PARPADEO_NAV, 420, nullptr);
        InvalidateRect(ventanaControles, nullptr, FALSE);
    }
}

bool reproducirVideoPorRuta(const std::wstring& rutaNuevoVideo) {
    if (!reproductor || rutaNuevoVideo.empty()) return false;
    navegacionManualEnCurso.store(true);
    reproductor->Stop();
    reiniciarCacheTiempoReproduccion();
    rutaVideoActiva = rutaNuevoVideo;
    actualizarMetadatosVideo(rutaNuevoVideo);
    HRESULT resultadoCarga = reproductor->CreateMediaItemFromURL(
        rutaNuevoVideo.c_str(), FALSE, 0, nullptr);
    if (FAILED(resultadoCarga)) {
        navegacionManualEnCurso.store(false);
        Wh_Log(L"No fue posible cargar el video: %s (HRESULT 0x%08X)",
               rutaNuevoVideo.c_str(),
               static_cast<unsigned int>(resultadoCarga));
        return false;
    }
    if (ventanaReproductor && IsWindow(ventanaReproductor)) {
        InvalidateRect(ventanaReproductor, nullptr, FALSE);
    }
    invalidarInterfazControles();
    return true;
}

void reproducirVideoAnterior() {
    if (listaVideosSeleccionados.size() > 1) {
        indiceVideoSeleccionado =
            (indiceVideoSeleccionado == 0)
                ? (listaVideosSeleccionados.size() - 1)
                : (indiceVideoSeleccionado - 1);
        reproducirVideoPorRuta(listaVideosSeleccionados[indiceVideoSeleccionado]);
        return;
    }
    if (rutaVideoActiva.empty()) return;
    std::vector<std::wstring> videosCarpeta = obtenerVideosEnMismaCarpeta(rutaVideoActiva);
    if (videosCarpeta.size() <= 1) {
        activarParpadeoBotonesNavVideo();
        return;
    }
    std::wstring rutaAnterior =
        obtenerVideoAnteriorEnCarpeta(rutaVideoActiva);
    if (!rutaAnterior.empty() && rutaAnterior != rutaVideoActiva) {
        reproducirVideoPorRuta(rutaAnterior);
    } else {
        activarParpadeoBotonesNavVideo();
    }
}

void reproducirVideoSiguiente() {
    if (listaVideosSeleccionados.size() > 1) {
        indiceVideoSeleccionado =
            (indiceVideoSeleccionado + 1) % listaVideosSeleccionados.size();
        reproducirVideoPorRuta(listaVideosSeleccionados[indiceVideoSeleccionado]);
        return;
    }
    if (rutaVideoActiva.empty()) return;
    std::vector<std::wstring> videosCarpeta = obtenerVideosEnMismaCarpeta(rutaVideoActiva);
    if (videosCarpeta.size() <= 1) {
        activarParpadeoBotonesNavVideo();
        return;
    }
    std::wstring rutaSiguiente =
        obtenerVideoSiguienteEnCarpeta(rutaVideoActiva);
    if (!rutaSiguiente.empty() && rutaSiguiente != rutaVideoActiva) {
        reproducirVideoPorRuta(rutaSiguiente);
    } else {
        activarParpadeoBotonesNavVideo();
    }
}

void cambiarVelocidadReproduccion(float nuevaVelocidad) {
    if (!reproductor) return;
    if (SUCCEEDED(reproductor->SetRate(nuevaVelocidad))) {
        velocidadReproduccionActual.store(nuevaVelocidad);
    }
    invalidarInterfazControles();
}


void iniciarArrastreBarraProgreso(HWND ventana, int posicionHorizontal) {
    if (!reproductor || !posicionamientoDisponible.load()) return;

    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);
    proporcionProgreso = calcularProporcionProgreso(
        disenoBarraProgreso, posicionHorizontal);
    arrastrandoBarraProgreso = true;

    SetFocus(ventana);
    SetCapture(ventana);
    invalidarInterfazControles();
}

void actualizarArrastreBarraProgreso(HWND ventana, int posicionHorizontal) {
    if (!arrastrandoBarraProgreso) return;
    if (GetCapture() != ventana) {
        arrastrandoBarraProgreso = false;
        invalidarInterfazControles();
        return;
    }

    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);
    proporcionProgreso = calcularProporcionProgreso(
        disenoBarraProgreso, posicionHorizontal);

    invalidarInterfazControles();
}

void finalizarArrastreBarraProgreso(HWND ventana, bool aplicarCambio) {
    if (!arrastrandoBarraProgreso) return;
    if (aplicarCambio) {
        buscarEnReproduccion(proporcionProgreso);
    }
    arrastrandoBarraProgreso = false;
    if (GetCapture() == ventana) ReleaseCapture();
    invalidarInterfazControles();
}

void desplazarBarraProgresoConTeclado(double incrementoProporcion) {
    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    if (!obtenerTiempoReproduccionCache(&posicionActual, &duracionTotal)) {
        return;
    }
    buscarEnReproduccion(
        static_cast<double>(posicionActual) /
            static_cast<double>(duracionTotal) +
        incrementoProporcion);
}
