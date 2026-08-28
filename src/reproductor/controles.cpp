void alternarReproduccion() {
    if (!reproductor) return;
    MFP_MEDIAPLAYER_STATE estado;
    if (SUCCEEDED(reproductor->GetState(&estado)) &&
        estado == MFP_MEDIAPLAYER_STATE_PLAYING) {
        reproductor->Pause();
    } else {
        reproductor->Play();
    }
    InvalidateRect(ventanaReproductor, nullptr, FALSE);
}

void alternarSonido() {
    if (!reproductor) return;
    BOOL silenciado = TRUE;
    reproductor->GetMute(&silenciado);
    reproductor->SetMute(!silenciado);
    InvalidateRect(ventanaReproductor, nullptr, FALSE);
}

void buscarDesdeCoordenada(int posicionX) {
    if (!reproductor || !ventanaReproductor) return;
    RECT areaCliente;
    GetClientRect(ventanaReproductor, &areaCliente);
    int inicio = ANCHO_BOTON_REPRODUCIR + 10;
    int fin = areaCliente.right - ANCHO_BOTON_SONIDO - 10;
    if (fin <= inicio) return;

    double proporcion = static_cast<double>(
                            std::max(inicio, std::min(posicionX, fin)) - inicio) /
                        static_cast<double>(fin - inicio);
    PROPVARIANT duracion;
    PropVariantInit(&duracion);
    if (SUCCEEDED(reproductor->GetDuration(MFP_POSITIONTYPE_100NS, &duracion)) &&
        duracion.vt == VT_I8) {
        PROPVARIANT posicion;
        PropVariantInit(&posicion);
        posicion.vt = VT_I8;
        posicion.hVal.QuadPart =
            static_cast<LONGLONG>(duracion.hVal.QuadPart * proporcion);
        reproductor->SetPosition(MFP_POSITIONTYPE_100NS, &posicion);
    }
    PropVariantClear(&duracion);
}
