void dibujarInterfaz(HWND ventana) {
    PAINTSTRUCT pintura;
    HDC contexto = BeginPaint(ventana, &pintura);
    RECT areaCliente;
    GetClientRect(ventana, &areaCliente);
    RECT barra{0, std::max<LONG>(0, areaCliente.bottom - ALTO_CONTROLES),
               areaCliente.right, areaCliente.bottom};

    HBRUSH fondo = CreateSolidBrush(RGB(25, 25, 25));
    FillRect(contexto, &barra, fondo);
    DeleteObject(fondo);
    SetBkMode(contexto, TRANSPARENT);
    SetTextColor(contexto, RGB(245, 245, 245));

    MFP_MEDIAPLAYER_STATE estado = MFP_MEDIAPLAYER_STATE_EMPTY;
    BOOL silenciado = TRUE;
    if (reproductor) {
        reproductor->GetState(&estado);
        reproductor->GetMute(&silenciado);
    }

    RECT botonReproducir{0, barra.top, ANCHO_BOTON_REPRODUCIR, barra.bottom};
    DrawTextW(contexto,
              estado == MFP_MEDIAPLAYER_STATE_PLAYING ? L"Pause" : L"Play", -1,
              &botonReproducir, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    RECT botonSonido{areaCliente.right - ANCHO_BOTON_SONIDO, barra.top,
                     areaCliente.right, barra.bottom};
    DrawTextW(contexto, silenciado ? L"Muted" : L"Sound", -1, &botonSonido,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    int inicio = ANCHO_BOTON_REPRODUCIR + 10;
    int fin = areaCliente.right - ANCHO_BOTON_SONIDO - 10;
    int centroY = barra.top + ALTO_CONTROLES / 2;
    HPEN lineaBase = CreatePen(PS_SOLID, 3, RGB(100, 100, 100));
    HPEN lapizAnterior = static_cast<HPEN>(SelectObject(contexto, lineaBase));
    MoveToEx(contexto, inicio, centroY, nullptr);
    LineTo(contexto, fin, centroY);
    SelectObject(contexto, lapizAnterior);
    DeleteObject(lineaBase);

    if (reproductor) {
        PROPVARIANT posicion;
        PROPVARIANT duracion;
        PropVariantInit(&posicion);
        PropVariantInit(&duracion);
        if (SUCCEEDED(reproductor->GetPosition(MFP_POSITIONTYPE_100NS, &posicion)) &&
            SUCCEEDED(reproductor->GetDuration(MFP_POSITIONTYPE_100NS, &duracion)) &&
            posicion.vt == VT_I8 && duracion.vt == VT_I8 &&
            duracion.hVal.QuadPart > 0) {
            int progreso = inicio + static_cast<int>(
                (fin - inicio) * static_cast<double>(posicion.hVal.QuadPart) /
                duracion.hVal.QuadPart);
            HPEN lineaProgreso = CreatePen(PS_SOLID, 4, RGB(0, 120, 215));
            lapizAnterior =
                static_cast<HPEN>(SelectObject(contexto, lineaProgreso));
            MoveToEx(contexto, inicio, centroY, nullptr);
            LineTo(contexto, progreso, centroY);
            SelectObject(contexto, lapizAnterior);
            DeleteObject(lineaProgreso);
        }
        PropVariantClear(&posicion);
        PropVariantClear(&duracion);
    }
    EndPaint(ventana, &pintura);
}
