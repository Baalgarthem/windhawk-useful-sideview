std::wstring formatearTiempo(LONGLONG unidadesCienNanosegundos) {
    long long segundosTotales = unidadesCienNanosegundos / 10000000LL;
    long long horas = segundosTotales / 3600;
    int minutos = static_cast<int>((segundosTotales % 3600) / 60);
    int segundos = static_cast<int>(segundosTotales % 60);
    auto formatearDosDigitos = [](int numero) {
        std::wstring numeroFormateado = std::to_wstring(numero);
        return numeroFormateado.length() == 1
                   ? L"0" + numeroFormateado
                   : numeroFormateado;
    };
    if (horas > 0) {
        return std::to_wstring(horas) + L":" +
               formatearDosDigitos(minutos) + L":" +
               formatearDosDigitos(segundos);
    }
    return formatearDosDigitos(minutos) + L":" +
           formatearDosDigitos(segundos);
}

bool crearRecursosInterfaz() {
    fuenteIconos = CreateFontW(
        -20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe Fluent Icons");
    fuenteTexto = CreateFontW(
        -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    if (fuenteIconos && fuenteTexto) return true;

    if (fuenteIconos) DeleteObject(fuenteIconos);
    if (fuenteTexto) DeleteObject(fuenteTexto);
    fuenteIconos = nullptr;
    fuenteTexto = nullptr;
    return false;
}

void liberarRecursosInterfaz() {
    if (fuenteIconos) DeleteObject(fuenteIconos);
    if (fuenteTexto) DeleteObject(fuenteTexto);
    fuenteIconos = nullptr;
    fuenteTexto = nullptr;
}

void dibujarLinea(HDC contextoDibujo, int limiteIzquierdo,
                  int limiteDerecho, int posicionVertical, int grosor,
                  COLORREF color) {
    HPEN lapiz = CreatePen(PS_SOLID, grosor, color);
    HPEN lapizPreviamenteSeleccionado =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapiz));
    MoveToEx(contextoDibujo, limiteIzquierdo, posicionVertical, nullptr);
    LineTo(contextoDibujo, limiteDerecho, posicionVertical);
    SelectObject(contextoDibujo, lapizPreviamenteSeleccionado);
    DeleteObject(lapiz);
}

void dibujarBotonControl(HDC contextoDibujo, const RECT& areaBoton,
                         const wchar_t* glifo, bool resaltado,
                         bool botonPrincipal) {
    COLORREF colorFondo = botonPrincipal
                              ? RGB(0, 120, 212)
                              : resaltado ? RGB(35, 91, 138)
                                           : RGB(43, 43, 43);
    COLORREF colorBorde = resaltado || botonPrincipal
                              ? RGB(94, 181, 255)
                              : RGB(72, 72, 72);
    HBRUSH pincelFondo = CreateSolidBrush(colorFondo);
    HPEN lapizBorde = CreatePen(PS_SOLID, 1, colorBorde);
    HBRUSH pincelAnterior =
        static_cast<HBRUSH>(SelectObject(contextoDibujo, pincelFondo));
    HPEN lapizAnterior =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    RoundRect(contextoDibujo, areaBoton.left, areaBoton.top,
              areaBoton.right, areaBoton.bottom, 14, 14);
    SelectObject(contextoDibujo, lapizAnterior);
    SelectObject(contextoDibujo, pincelAnterior);
    DeleteObject(lapizBorde);
    DeleteObject(pincelFondo);

    HFONT fuenteAnterior =
        static_cast<HFONT>(SelectObject(contextoDibujo, fuenteIconos));
    SetTextColor(contextoDibujo, RGB(250, 250, 250));
    RECT areaIcono = areaBoton;
    DrawTextW(contextoDibujo, glifo, -1, &areaIcono,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(contextoDibujo, fuenteAnterior);
}

void dibujarInterfaz(HWND ventana) {
    PAINTSTRUCT pintura;
    HDC contextoDibujo = BeginPaint(ventana, &pintura);
    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);

    HBRUSH pincelFondo = CreateSolidBrush(RGB(25, 25, 25));
    FillRect(contextoDibujo, &disenoBarraProgreso.barraControles,
             pincelFondo);
    DeleteObject(pincelFondo);
    SetBkMode(contextoDibujo, TRANSPARENT);
    SetTextColor(contextoDibujo, RGB(245, 245, 245));

    MFP_MEDIAPLAYER_STATE estadoReproduccion = MFP_MEDIAPLAYER_STATE_EMPTY;
    BOOL sonidoSilenciado = TRUE;
    if (reproductor) {
        reproductor->GetState(&estadoReproduccion);
        reproductor->GetMute(&sonidoSilenciado);
    }
    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    bool tiempoDisponible =
        obtenerTiempoReproduccionCache(&posicionActual, &duracionTotal);
    double progresoNormalizado =
        tiempoDisponible
            ? static_cast<double>(posicionActual) /
                  static_cast<double>(duracionTotal)
            : 0.0;
    if (arrastrandoBarraProgreso) {
        progresoNormalizado = proporcionProgreso;
        if (tiempoDisponible) {
            posicionActual = calcularPosicionTemporal(
                duracionTotal, proporcionProgreso);
        }
    } else if (cambioPosicionPendiente.load()) {
        progresoNormalizado = proporcionPosicionSolicitada.load();
        if (tiempoDisponible) {
            posicionActual = calcularPosicionTemporal(
                duracionTotal, progresoNormalizado);
        }
    }

    if (tiempoDisponible) {
        LONGLONG tiempoRestante =
            std::max(0LL, duracionTotal - posicionActual);
        std::wstring textoTranscurrido = formatearTiempo(posicionActual);
        std::wstring textoRestante =
            L"-" + formatearTiempo(tiempoRestante);
        RECT areaTranscurrido{disenoBarraProgreso.limiteIzquierdo, 5,
                              disenoBarraProgreso.limiteDerecho, 22};
        RECT areaRestante = areaTranscurrido;
        HFONT fuenteAnterior =
            static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTexto));
        SetTextColor(contextoDibujo, RGB(210, 210, 210));
        DrawTextW(contextoDibujo, textoTranscurrido.c_str(), -1,
                  &areaTranscurrido,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        DrawTextW(contextoDibujo, textoRestante.c_str(), -1, &areaRestante,
                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuenteAnterior);
    }

    dibujarLinea(contextoDibujo, disenoBarraProgreso.limiteIzquierdo,
                 disenoBarraProgreso.limiteDerecho,
                 disenoBarraProgreso.centroVertical, 3,
                 tiempoDisponible && posicionamientoDisponible.load()
                     ? RGB(92, 92, 92)
                     : RGB(58, 58, 58));
    int posicionPerilla =
        calcularPosicionProgreso(disenoBarraProgreso, progresoNormalizado);
    dibujarLinea(contextoDibujo, disenoBarraProgreso.limiteIzquierdo,
                 posicionPerilla, disenoBarraProgreso.centroVertical, 4,
                 RGB(0, 120, 215));

    HBRUSH pincelPerilla = CreateSolidBrush(RGB(245, 245, 245));
    HBRUSH pincelPreviamenteSeleccionado =
        static_cast<HBRUSH>(SelectObject(contextoDibujo, pincelPerilla));
    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(0, 120, 215));
    HPEN lapizPreviamenteSeleccionado =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    Ellipse(contextoDibujo, posicionPerilla - RADIO_PERILLA_PROGRESO,
            disenoBarraProgreso.centroVertical - RADIO_PERILLA_PROGRESO,
            posicionPerilla + RADIO_PERILLA_PROGRESO + 1,
            disenoBarraProgreso.centroVertical + RADIO_PERILLA_PROGRESO + 1);
    SelectObject(contextoDibujo, lapizPreviamenteSeleccionado);
    SelectObject(contextoDibujo, pincelPreviamenteSeleccionado);
    DeleteObject(lapizBorde);
    DeleteObject(pincelPerilla);

    bool reproduciendo =
        estadoReproduccion == MFP_MEDIAPLAYER_STATE_PLAYING;
    dibujarBotonControl(contextoDibujo,
                         disenoBarraProgreso.botonReproduccion,
                         reproduciendo ? GLIFO_PAUSAR : GLIFO_REPRODUCIR,
                         reproduciendo, true);
    dibujarBotonControl(contextoDibujo, disenoBarraProgreso.botonSonido,
                         sonidoSilenciado ? GLIFO_SILENCIO : GLIFO_VOLUMEN,
                         !sonidoSilenciado, false);
    dibujarBotonControl(contextoDibujo,
                         disenoBarraProgreso.botonRepeticion,
                         GLIFO_REPETICION, repeticionHabilitada.load(),
                         false);

    if (GetFocus() == ventana) {
        RECT areaFoco = disenoBarraProgreso.areaInteractiva;
        InflateRect(&areaFoco, -2, -1);
        DrawFocusRect(contextoDibujo, &areaFoco);
    }
    EndPaint(ventana, &pintura);
}
