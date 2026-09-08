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

std::wstring formatearEtiquetaVelocidad(float velocidad) {
    for (int posicionOpcion = 0; posicionOpcion < TOTAL_OPCIONES_VELOCIDAD;
         ++posicionOpcion) {
        if (std::abs(VALORES_VELOCIDAD[posicionOpcion] - velocidad) < 0.01f) {
            return ETIQUETAS_VELOCIDAD[posicionOpcion];
        }
    }
    wchar_t buferVelocidad[16];
    swprintf_s(buferVelocidad, L"%.2fx", static_cast<double>(velocidad));
    return buferVelocidad;
}

bool crearRecursosInterfaz() {
    fuenteIconos = CreateFontW(
        -18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe Fluent Icons");
    fuenteTexto = CreateFontW(
        -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    fuenteTextoPequeno = CreateFontW(
        -11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    if (fuenteIconos && fuenteTexto && fuenteTextoPequeno) return true;

    if (fuenteIconos) DeleteObject(fuenteIconos);
    if (fuenteTexto) DeleteObject(fuenteTexto);
    if (fuenteTextoPequeno) DeleteObject(fuenteTextoPequeno);
    fuenteIconos = nullptr;
    fuenteTexto = nullptr;
    fuenteTextoPequeno = nullptr;
    return false;
}

void liberarRecursosInterfaz() {
    if (fuenteIconos) DeleteObject(fuenteIconos);
    if (fuenteTexto) DeleteObject(fuenteTexto);
    if (fuenteTextoPequeno) DeleteObject(fuenteTextoPequeno);
    fuenteIconos = nullptr;
    fuenteTexto = nullptr;
    fuenteTextoPequeno = nullptr;
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
                         bool botonPrincipal, bool parpadeoAlerta = false) {
    COLORREF colorFondo = parpadeoAlerta
                              ? RGB(85, 45, 25)
                              : (botonPrincipal
                                     ? RGB(0, 120, 212)
                                     : (resaltado ? RGB(35, 91, 138)
                                                  : RGB(43, 43, 43)));
    COLORREF colorBorde = parpadeoAlerta
                              ? RGB(255, 170, 0)
                              : (resaltado || botonPrincipal
                                     ? RGB(94, 181, 255)
                                     : RGB(72, 72, 72));
    COLORREF colorTexto = parpadeoAlerta
                              ? RGB(255, 210, 100)
                              : RGB(250, 250, 250);
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
    SetTextColor(contextoDibujo, colorTexto);
    RECT areaIcono = areaBoton;
    DrawTextW(contextoDibujo, glifo, -1, &areaIcono,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(contextoDibujo, fuenteAnterior);
}

void dibujarBotonTexto(HDC contextoDibujo, const RECT& areaBoton,
                       const wchar_t* texto, bool resaltado) {
    COLORREF colorFondo = resaltado ? RGB(0, 120, 212) : RGB(43, 43, 43);
    COLORREF colorBorde = resaltado ? RGB(94, 181, 255) : RGB(72, 72, 72);
    HBRUSH pincelFondo = CreateSolidBrush(colorFondo);
    HPEN lapizBorde = CreatePen(PS_SOLID, 1, colorBorde);
    HBRUSH pincelAnterior =
        static_cast<HBRUSH>(SelectObject(contextoDibujo, pincelFondo));
    HPEN lapizAnterior =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    RoundRect(contextoDibujo, areaBoton.left, areaBoton.top,
              areaBoton.right, areaBoton.bottom, 10, 10);
    SelectObject(contextoDibujo, lapizAnterior);
    SelectObject(contextoDibujo, pincelAnterior);
    DeleteObject(lapizBorde);
    DeleteObject(pincelFondo);

    HFONT fuenteAnterior =
        static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
    SetTextColor(contextoDibujo, RGB(245, 245, 245));
    RECT areaTexto = areaBoton;
    DrawTextW(contextoDibujo, texto, -1, &areaTexto,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(contextoDibujo, fuenteAnterior);
}

void dibujarMenuVelocidadFlotante(HWND ventana) {
    PAINTSTRUCT pintura;
    HDC contextoPantalla = BeginPaint(ventana, &pintura);
    RECT areaCliente;
    GetClientRect(ventana, &areaCliente);
    int anchoCliente = areaCliente.right - areaCliente.left;
    int altoCliente = areaCliente.bottom - areaCliente.top;

    if (anchoCliente > 0 && altoCliente > 0) {
        HDC contextoMemoria = CreateCompatibleDC(contextoPantalla);
        HBITMAP mapaBitsMemoria =
            CreateCompatibleBitmap(contextoPantalla, anchoCliente, altoCliente);
        HBITMAP mapaBitsAnterior = static_cast<HBITMAP>(
            SelectObject(contextoMemoria, mapaBitsMemoria));

        HBRUSH pincelFondoPanel = CreateSolidBrush(RGB(28, 28, 28));
        HPEN lapizBordePanel = CreatePen(PS_SOLID, 1, RGB(70, 70, 70));
        HBRUSH pincelAnterior =
            static_cast<HBRUSH>(SelectObject(contextoMemoria, pincelFondoPanel));
        HPEN lapizAnterior =
            static_cast<HPEN>(SelectObject(contextoMemoria, lapizBordePanel));

        RoundRect(contextoMemoria, areaCliente.left, areaCliente.top,
                  areaCliente.right, areaCliente.bottom, 10, 10);

        SelectObject(contextoMemoria, lapizAnterior);
        SelectObject(contextoMemoria, pincelAnterior);
        DeleteObject(lapizBordePanel);
        DeleteObject(pincelFondoPanel);

        HFONT fuenteAnterior =
            static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTextoPequeno));
        SetBkMode(contextoMemoria, TRANSPARENT);
        float velocidadActual = velocidadReproduccionActual.load();

        for (int posicionOpcion = 0; posicionOpcion < TOTAL_OPCIONES_VELOCIDAD;
             ++posicionOpcion) {
            int topeOpcion = 4 + posicionOpcion * ALTO_OPCION_VELOCIDAD;
            RECT areaOpcion = {4, topeOpcion, anchoCliente - 4,
                               topeOpcion + ALTO_OPCION_VELOCIDAD};
            bool esVelocidadSeleccionada =
                std::abs(VALORES_VELOCIDAD[posicionOpcion] - velocidadActual) <
                0.01f;
            bool esOpcionHover =
                posicionOpcion == opcionVelocidadResaltada;

            if (esVelocidadSeleccionada || esOpcionHover) {
                COLORREF colorOpcion = esVelocidadSeleccionada
                                           ? RGB(0, 120, 212)
                                           : RGB(55, 55, 55);
                HBRUSH pincelOpcion = CreateSolidBrush(colorOpcion);
                HPEN lapizOpcion = CreatePen(PS_SOLID, 1, colorOpcion);
                HBRUSH pincelOpAnterior = static_cast<HBRUSH>(
                    SelectObject(contextoMemoria, pincelOpcion));
                HPEN lapizOpAnterior = static_cast<HPEN>(
                    SelectObject(contextoMemoria, lapizOpcion));
                RoundRect(contextoMemoria, areaOpcion.left, areaOpcion.top,
                          areaOpcion.right, areaOpcion.bottom, 6, 6);
                SelectObject(contextoMemoria, lapizOpAnterior);
                SelectObject(contextoMemoria, pincelOpAnterior);
                DeleteObject(lapizOpcion);
                DeleteObject(pincelOpcion);
            }

            SetTextColor(contextoMemoria, esVelocidadSeleccionada
                                             ? RGB(255, 255, 255)
                                             : (esOpcionHover ? RGB(245, 245, 245)
                                                              : RGB(210, 210, 210)));
            RECT areaTextoOpcion = areaOpcion;
            DrawTextW(contextoMemoria, ETIQUETAS_VELOCIDAD[posicionOpcion], -1,
                      &areaTextoOpcion,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }

        SelectObject(contextoMemoria, fuenteAnterior);

        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente, contextoMemoria,
               0, 0, SRCCOPY);

        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
    }
    EndPaint(ventana, &pintura);
}

void dibujarLienzoComprimidos(HDC contextoMemoria, const RECT& areaCliente) {
    int anchoCliente = areaCliente.right - areaCliente.left;
    int altoCliente = areaCliente.bottom - areaCliente.top;
    if (anchoCliente <= 0 || altoCliente <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(18, 18, 18));
    FillRect(contextoMemoria, &areaCliente, brochaFondo);
    DeleteObject(brochaFondo);

    Gdiplus::Graphics graficos(contextoMemoria);
    graficos.SetInterpolationMode(
        Gdiplus::InterpolationModeHighQualityBicubic);
    graficos.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    if (estadoComprimidos.esModoComic) {
        size_t totalElementos = estadoComprimidos.elementosSeleccionados.size();
        if (totalElementos == 1) {
            const ElementoComprimido& elemento =
                estadoComprimidos.elementosSeleccionados.front();
            if (elemento.portadaCargada) {
                int anchoImagen = static_cast<int>(elemento.portadaCargada->GetWidth());
                int altoImagen = static_cast<int>(elemento.portadaCargada->GetHeight());
                if (anchoImagen > 0 && altoImagen > 0) {
                    float escalaHorizontal =
                        static_cast<float>(anchoCliente - 24) /
                        static_cast<float>(anchoImagen);
                    float escalaVertical =
                        static_cast<float>(altoCliente - 24) /
                        static_cast<float>(altoImagen);
                    float escalaOptima =
                        std::min(escalaHorizontal, escalaVertical);

                    int anchoDestino =
                        static_cast<int>(static_cast<float>(anchoImagen) * escalaOptima);
                    int altoDestino =
                        static_cast<int>(static_cast<float>(altoImagen) * escalaOptima);
                    int posicionX = (anchoCliente - anchoDestino) / 2;
                    int posicionY = (altoCliente - altoDestino) / 2;

                    graficos.DrawImage(elemento.portadaCargada, posicionX,
                                       posicionY, anchoDestino, altoDestino);
                }
            } else {
                RECT areaIcono{0, altoCliente / 2 - 40, anchoCliente,
                               altoCliente / 2};
                RECT areaTexto{10, altoCliente / 2 + 5, anchoCliente - 10,
                               altoCliente / 2 + 30};
                RECT areaTamano{10, altoCliente / 2 + 30, anchoCliente - 10,
                                altoCliente / 2 + 50};

                HFONT fuenteIconoGrande = CreateFontW(
                    -42, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                    DEFAULT_PITCH | FF_DONTCARE, L"Segoe Fluent Icons");
                HFONT fuentePrevia = static_cast<HFONT>(
                    SelectObject(contextoMemoria, fuenteIconoGrande));
                SetBkMode(contextoMemoria, TRANSPARENT);
                SetTextColor(contextoMemoria, RGB(255, 140, 0));
                DrawTextW(contextoMemoria, GLIFO_COMPRIMIDO, -1, &areaIcono,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                SelectObject(contextoMemoria, fuenteTexto);
                SetTextColor(contextoMemoria, RGB(230, 230, 230));
                DrawTextW(contextoMemoria, elemento.nombreArchivo.c_str(), -1,
                          &areaTexto,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS |
                              DT_NOPREFIX);

                SetTextColor(contextoMemoria, RGB(160, 160, 160));
                DrawTextW(contextoMemoria, elemento.textoTamano.c_str(), -1,
                          &areaTamano,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                SelectObject(contextoMemoria, fuentePrevia);
                DeleteObject(fuenteIconoGrande);
            }
        } else if (totalElementos >= 2) {
            int limite = configuracion.limiteCuadriculaComprimidos.load();
            if (limite != 4 && limite != 6 && limite != 8 && limite != 10) {
                limite = 4;
            }
            int totalFilas = (limite + 1) / 2;

            int margen = 12;
            int separacion = 8;
            int anchoDisponible = anchoCliente - margen * 2 - separacion;
            int altoDisponible =
                altoCliente - margen * 2 - (totalFilas - 1) * separacion;
            int anchoCelda = std::max(40, anchoDisponible / 2);
            int altoCelda = std::max(40, altoDisponible / totalFilas);

            size_t cantidadADibujar = std::min<size_t>(
                static_cast<size_t>(limite), totalElementos);

            for (size_t posicion = 0; posicion < cantidadADibujar; ++posicion) {
                int fila = static_cast<int>(posicion / 2);
                int columna = static_cast<int>(posicion % 2);
                int izquierdaCelda = margen + columna * (anchoCelda + separacion);
                int topeCelda = margen + fila * (altoCelda + separacion);

                RECT areaCelda{izquierdaCelda, topeCelda,
                               izquierdaCelda + anchoCelda,
                               topeCelda + altoCelda};
                HBRUSH brochaCelda = CreateSolidBrush(RGB(28, 28, 28));
                FillRect(contextoMemoria, &areaCelda, brochaCelda);
                DeleteObject(brochaCelda);

                const ElementoComprimido& elemento =
                    estadoComprimidos.elementosSeleccionados[posicion];
                if (elemento.portadaCargada) {
                    int anchoImg = static_cast<int>(elemento.portadaCargada->GetWidth());
                    int altoImg = static_cast<int>(elemento.portadaCargada->GetHeight());
                    float escH = static_cast<float>(anchoCelda - 8) /
                                 static_cast<float>(anchoImg);
                    float escV = static_cast<float>(altoCelda - 8) /
                                 static_cast<float>(altoImg);
                    float esc = std::min(escH, escV);
                    int anchoD = static_cast<int>(static_cast<float>(anchoImg) * esc);
                    int altoD = static_cast<int>(static_cast<float>(altoImg) * esc);
                    int posX = izquierdaCelda + (anchoCelda - anchoD) / 2;
                    int posY = topeCelda + (altoCelda - altoD) / 2;
                    graficos.DrawImage(elemento.portadaCargada, posX, posY,
                                       anchoD, altoD);
                } else {
                    HFONT fuenteIconoPrevia = static_cast<HFONT>(
                        SelectObject(contextoMemoria, fuenteIconos));
                    SetBkMode(contextoMemoria, TRANSPARENT);
                    SetTextColor(contextoMemoria, RGB(255, 140, 0));
                    RECT areaIconoCelda{izquierdaCelda, topeCelda + 6,
                                        izquierdaCelda + anchoCelda,
                                        topeCelda + altoCelda / 2};
                    DrawTextW(contextoMemoria, GLIFO_COMPRIMIDO, -1,
                              &areaIconoCelda,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                    SelectObject(contextoMemoria, fuenteTextoPequeno);
                    SetTextColor(contextoMemoria, RGB(220, 220, 220));
                    RECT areaTextoCelda{izquierdaCelda + 4,
                                        topeCelda + altoCelda / 2,
                                        izquierdaCelda + anchoCelda - 4,
                                        topeCelda + altoCelda - 4};
                    DrawTextW(contextoMemoria, elemento.nombreArchivo.c_str(), -1,
                              &areaTextoCelda,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE |
                                  DT_END_ELLIPSIS | DT_NOPREFIX);
                    SelectObject(contextoMemoria, fuenteIconoPrevia);
                }

                HPEN lapizCelda = CreatePen(PS_SOLID, 1, RGB(48, 48, 48));
                HPEN lapizPrevio =
                    static_cast<HPEN>(SelectObject(contextoMemoria, lapizCelda));
                HBRUSH brochaNula =
                    static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
                HBRUSH brochaPrevia =
                    static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaNula));
                Rectangle(contextoMemoria, areaCelda.left, areaCelda.top,
                          areaCelda.right, areaCelda.bottom);
                SelectObject(contextoMemoria, brochaPrevia);
                SelectObject(contextoMemoria, lapizPrevio);
                DeleteObject(lapizCelda);
            }

            if (estadoComprimidos.totalSeleccionados > static_cast<size_t>(limite)) {
                std::wstring textoContador =
                    L"+" + std::to_wstring(
                               estadoComprimidos.totalSeleccionados -
                               static_cast<size_t>(limite));
                int anchoBadge = 46;
                int altoBadge = 26;
                int izquierdaBadge = anchoCliente - margen - anchoBadge;
                int topeBadge = margen;
                RECT areaBadge{izquierdaBadge, topeBadge,
                               izquierdaBadge + anchoBadge,
                               topeBadge + altoBadge};

                HBRUSH brochaBadge = CreateSolidBrush(RGB(0, 120, 215));
                HPEN lapizBadge = CreatePen(PS_SOLID, 1, RGB(0, 140, 240));
                HBRUSH brochaBadgePrevia = static_cast<HBRUSH>(
                    SelectObject(contextoMemoria, brochaBadge));
                HPEN lapizBadgePrevio =
                    static_cast<HPEN>(SelectObject(contextoMemoria, lapizBadge));
                RoundRect(contextoMemoria, areaBadge.left, areaBadge.top,
                          areaBadge.right, areaBadge.bottom, 12, 12);
                SelectObject(contextoMemoria, brochaBadgePrevia);
                SelectObject(contextoMemoria, lapizBadgePrevio);
                DeleteObject(brochaBadge);
                DeleteObject(lapizBadge);

                HFONT fuentePrevia = static_cast<HFONT>(
                    SelectObject(contextoMemoria, fuenteTextoPequeno));
                SetBkMode(contextoMemoria, TRANSPARENT);
                SetTextColor(contextoMemoria, RGB(255, 255, 255));
                DrawTextW(contextoMemoria, textoContador.c_str(), -1, &areaBadge,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                SelectObject(contextoMemoria, fuentePrevia);
            }
        }
        return;
    }

    // --- MODO COMPRIMIDOS COMUNES (TREEMAP) ---
    int margenSuperior = 10;
    int altoCabecera = 36;
    if (estadoComprimidos.archivosSeleccionados.size() > 1) {
        int anchoBoton = 72;
        int altoBoton = 24;
        estadoComprimidos.botonAnteriorComprimido = {
            12, margenSuperior + 4, 12 + anchoBoton, margenSuperior + 4 + altoBoton
        };
        estadoComprimidos.botonSiguienteComprimido = {
            anchoCliente - 12 - anchoBoton, margenSuperior + 4, anchoCliente - 12, margenSuperior + 4 + altoBoton
        };

        auto dibujarBotonNav = [&](const RECT& rect, const wchar_t* texto, bool hover) {
            COLORREF colorFondo = hover ? RGB(48, 54, 61) : RGB(33, 38, 45);
            COLORREF colorBorde = hover ? RGB(110, 118, 129) : RGB(48, 54, 61);
            HBRUSH brochaBtn = CreateSolidBrush(colorFondo);
            HPEN lapizBtn = CreatePen(PS_SOLID, 1, colorBorde);
            HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaBtn));
            HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoMemoria, lapizBtn));
            RoundRect(contextoMemoria, rect.left, rect.top, rect.right, rect.bottom, 6, 6);
            SelectObject(contextoMemoria, lapizPrev);
            SelectObject(contextoMemoria, brochaPrev);
            DeleteObject(lapizBtn);
            DeleteObject(brochaBtn);

            SetBkMode(contextoMemoria, TRANSPARENT);
            SetTextColor(contextoMemoria, hover ? RGB(255, 255, 255) : RGB(201, 209, 217));
            HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTextoPequeno));
            RECT rTexto = rect;
            DrawTextW(contextoMemoria, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(contextoMemoria, fuentePrev);
        };

        dibujarBotonNav(estadoComprimidos.botonAnteriorComprimido, L"◀ Ant", estadoComprimidos.hoverBotonAnterior);
        dibujarBotonNav(estadoComprimidos.botonSiguienteComprimido, L"Sig ▶", estadoComprimidos.hoverBotonSiguiente);

        std::wstring nombreArchivoActual = L"";
        if (estadoComprimidos.indiceArchivoActual < estadoComprimidos.archivosSeleccionados.size()) {
            nombreArchivoActual = PathFindFileNameW(
                estadoComprimidos.archivosSeleccionados[estadoComprimidos.indiceArchivoActual].c_str());
        }

        std::wstring textoCentro = nombreArchivoActual + L" (" +
                                   std::to_wstring(estadoComprimidos.indiceArchivoActual + 1) +
                                   L"/" + std::to_wstring(estadoComprimidos.archivosSeleccionados.size()) + L")";
        RECT areaCentro{
            estadoComprimidos.botonAnteriorComprimido.right + 6,
            margenSuperior,
            estadoComprimidos.botonSiguienteComprimido.left - 6,
            margenSuperior + altoCabecera
        };
        SetBkMode(contextoMemoria, TRANSPARENT);
        SetTextColor(contextoMemoria, RGB(240, 240, 240));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTexto));
        DrawTextW(contextoMemoria, textoCentro.c_str(), -1, &areaCentro,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(contextoMemoria, fuentePrev);
    } else {
        std::wstring nombreArchivoActual = L"";
        if (!estadoComprimidos.archivosSeleccionados.empty()) {
            nombreArchivoActual = PathFindFileNameW(
                estadoComprimidos.archivosSeleccionados.front().c_str());
        }
        RECT areaTitulo{14, margenSuperior, anchoCliente - 14, margenSuperior + altoCabecera};
        SetBkMode(contextoMemoria, TRANSPARENT);
        SetTextColor(contextoMemoria, RGB(240, 240, 240));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTexto));
        DrawTextW(contextoMemoria, (L"📦 " + nombreArchivoActual).c_str(), -1, &areaTitulo,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(contextoMemoria, fuentePrev);
    }

    // Área del TreeMap Box
    int margenArbol = 12;
    int topeArbol = margenSuperior + altoCabecera + 4;
    int altoArbolBox = altoCliente - topeArbol - 10;
    if (altoArbolBox < 40) return;

    estadoComprimidos.areaCajaArbol = {
        margenArbol, topeArbol, anchoCliente - margenArbol, topeArbol + altoArbolBox
    };

    HBRUSH brochaCaja = CreateSolidBrush(RGB(13, 17, 23));
    HPEN lapizCaja = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPrevCaja = static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaCaja));
    HPEN lapizPrevCaja = static_cast<HPEN>(SelectObject(contextoMemoria, lapizCaja));
    RoundRect(contextoMemoria, estadoComprimidos.areaCajaArbol.left,
              estadoComprimidos.areaCajaArbol.top, estadoComprimidos.areaCajaArbol.right,
              estadoComprimidos.areaCajaArbol.bottom, 8, 8);
    SelectObject(contextoMemoria, lapizPrevCaja);
    SelectObject(contextoMemoria, brochaPrevCaja);
    DeleteObject(lapizCaja);
    DeleteObject(brochaCaja);

    if (estadoComprimidos.protegidoConContrasena && !estadoComprimidos.contrasenaDesbloqueada) {
        SelectObject(contextoMemoria, fuenteTexto);
        SetTextColor(contextoMemoria, RGB(255, 180, 50));
        RECT areaIconoPass{estadoComprimidos.areaCajaArbol.left + 16,
                           estadoComprimidos.areaCajaArbol.top + 30,
                           estadoComprimidos.areaCajaArbol.right - 16,
                           estadoComprimidos.areaCajaArbol.top + 55};
        DrawTextW(contextoMemoria, L"🔒 Archivo protegido con contraseña", -1,
                  &areaIconoPass, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(contextoMemoria, fuenteTextoPequeno);
        SetTextColor(contextoMemoria, RGB(139, 148, 158));
        RECT areaDescPass{estadoComprimidos.areaCajaArbol.left + 20,
                          estadoComprimidos.areaCajaArbol.top + 62,
                          estadoComprimidos.areaCajaArbol.right - 20,
                          estadoComprimidos.areaCajaArbol.top + 105};
        DrawTextW(contextoMemoria,
                  L"Este archivo comprimido está cifrado. Para visualizar los archivos contenidos en el primer nivel, introduzca su contraseña.",
                  -1, &areaDescPass, DT_CENTER | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

        int anchoBotonDesbloquear = 200;
        int altoBotonDesbloquear = 32;
        int centroX = (estadoComprimidos.areaCajaArbol.left + estadoComprimidos.areaCajaArbol.right) / 2;
        int topBoton = estadoComprimidos.areaCajaArbol.top + 115;
        estadoComprimidos.areaBotonDesbloquear = {
            centroX - anchoBotonDesbloquear / 2, topBoton,
            centroX + anchoBotonDesbloquear / 2, topBoton + altoBotonDesbloquear
        };

        bool hoverDesbloquear = estadoComprimidos.hoverBotonDesbloquear;
        HBRUSH brochaDesb = CreateSolidBrush(hoverDesbloquear ? RGB(48, 54, 61) : RGB(33, 38, 45));
        HPEN lapizDesb = CreatePen(PS_SOLID, 1, hoverDesbloquear ? RGB(88, 166, 255) : RGB(48, 54, 61));
        HBRUSH brochaPrevDesb = static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaDesb));
        HPEN lapizPrevDesb = static_cast<HPEN>(SelectObject(contextoMemoria, lapizDesb));
        RoundRect(contextoMemoria, estadoComprimidos.areaBotonDesbloquear.left,
                  estadoComprimidos.areaBotonDesbloquear.top,
                  estadoComprimidos.areaBotonDesbloquear.right,
                  estadoComprimidos.areaBotonDesbloquear.bottom, 6, 6);
        SelectObject(contextoMemoria, lapizPrevDesb);
        SelectObject(contextoMemoria, brochaPrevDesb);
        DeleteObject(lapizDesb);
        DeleteObject(brochaDesb);

        SetTextColor(contextoMemoria, hoverDesbloquear ? RGB(255, 255, 255) : RGB(230, 237, 243));
        RECT rTextoBtn = estadoComprimidos.areaBotonDesbloquear;
        DrawTextW(contextoMemoria, L"🔑 Desbloquear contenido", -1, &rTextoBtn,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        return;
    }

    int altoCabeceraSeccion = 28;
    int altoFilaElemento = 22;

    int altoCalculadoTotal = 16;
    for (const auto& seccionActual : estadoComprimidos.seccionesContenido) {
        altoCalculadoTotal += altoCabeceraSeccion + static_cast<int>(seccionActual.elementos.size()) * altoFilaElemento + 8;
    }
    estadoComprimidos.altoTotalArbol = altoCalculadoTotal;

    int posicionVerticalInicio = estadoComprimidos.areaCajaArbol.top + 8 - estadoComprimidos.desplazamientoScrollY;

    HRGN regionRecorte = CreateRectRgn(
        estadoComprimidos.areaCajaArbol.left + 2,
        estadoComprimidos.areaCajaArbol.top + 2,
        estadoComprimidos.areaCajaArbol.right - 2,
        estadoComprimidos.areaCajaArbol.bottom - 2);
    SelectClipRgn(contextoMemoria, regionRecorte);

    HFONT fuenteTituloSeccion = CreateFontW(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteItemArbol = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    int posicionVerticalCursor = posicionVerticalInicio;

    for (auto& seccionActual : estadoComprimidos.seccionesContenido) {
        int topeSeccion = posicionVerticalCursor;
        int fondoSeccion = posicionVerticalCursor + altoCabeceraSeccion;
        posicionVerticalCursor += altoCabeceraSeccion;

        RECT areaSeccionHeader{
            estadoComprimidos.areaCajaArbol.left + 8,
            topeSeccion,
            estadoComprimidos.areaCajaArbol.right - 8,
            fondoSeccion
        };
        seccionActual.areaCabecera = areaSeccionHeader;

        if (fondoSeccion >= estadoComprimidos.areaCajaArbol.top &&
            topeSeccion <= estadoComprimidos.areaCajaArbol.bottom) {

            std::wstring textoBadge = seccionActual.iconoSeccion + L" " + seccionActual.tituloSeccion +
                                      L" (" + std::to_wstring(seccionActual.elementos.size()) + L")";

            SelectObject(contextoMemoria, fuenteTituloSeccion);
            SIZE tamanoTextoBadge{};
            GetTextExtentPoint32W(contextoMemoria, textoBadge.c_str(), static_cast<int>(textoBadge.size()), &tamanoTextoBadge);

            RECT areaPill = {
                areaSeccionHeader.left + 4,
                areaSeccionHeader.top + 3,
                areaSeccionHeader.left + 4 + tamanoTextoBadge.cx + 16,
                areaSeccionHeader.top + 3 + 22
            };

            HBRUSH brochaPill = CreateSolidBrush(RGB(30, 41, 59));
            HPEN lapizPill = CreatePen(PS_SOLID, 1, RGB(56, 139, 253));
            HBRUSH brochaPreviaPill = static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaPill));
            HPEN lapizPrevioPill = static_cast<HPEN>(SelectObject(contextoMemoria, lapizPill));
            RoundRect(contextoMemoria, areaPill.left, areaPill.top, areaPill.right, areaPill.bottom, 6, 6);
            SelectObject(contextoMemoria, lapizPrevioPill);
            SelectObject(contextoMemoria, brochaPreviaPill);
            DeleteObject(lapizPill);
            DeleteObject(brochaPill);

            SetBkMode(contextoMemoria, TRANSPARENT);
            SetTextColor(contextoMemoria, RGB(121, 192, 255));
            DrawTextW(contextoMemoria, textoBadge.c_str(), -1, &areaPill,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            int yLinea = areaSeccionHeader.top + 14;
            HPEN lapizDiv = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
            HPEN lapizPrevDiv = static_cast<HPEN>(SelectObject(contextoMemoria, lapizDiv));
            MoveToEx(contextoMemoria, areaPill.right + 10, yLinea, nullptr);
            LineTo(contextoMemoria, areaSeccionHeader.right - 4, yLinea);
            SelectObject(contextoMemoria, lapizPrevDiv);
            DeleteObject(lapizDiv);
        }

        SelectObject(contextoMemoria, fuenteItemArbol);
        for (auto& elementoActual : seccionActual.elementos) {
            int topeElem = posicionVerticalCursor;
            int fondoElem = posicionVerticalCursor + altoFilaElemento;
            posicionVerticalCursor += altoFilaElemento;

            RECT areaItem = {
                estadoComprimidos.areaCajaArbol.left + 16,
                topeElem,
                estadoComprimidos.areaCajaArbol.right - 80,
                fondoElem
            };
            elementoActual.areaElemento = areaItem;

            if (fondoElem >= estadoComprimidos.areaCajaArbol.top &&
                topeElem <= estadoComprimidos.areaCajaArbol.bottom) {

                std::wstring prefijo = elementoActual.esDirectorio ? L"📁 " : (seccionActual.iconoSeccion + L" ");
                std::wstring textoNombre = prefijo + elementoActual.rutaRelativa;

                SetBkMode(contextoMemoria, TRANSPARENT);
                SetTextColor(contextoMemoria, elementoActual.esDirectorio ? RGB(121, 192, 255) : RGB(225, 232, 240));
                DrawTextW(contextoMemoria, textoNombre.c_str(), -1, &areaItem,
                          DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                if (!elementoActual.esDirectorio && elementoActual.tamanoBytes > 0) {
                    std::wstring txtTamano = formatearTamanoArchivo(elementoActual.tamanoBytes);
                    RECT areaTam{
                        estadoComprimidos.areaCajaArbol.right - 78,
                        topeElem,
                        estadoComprimidos.areaCajaArbol.right - 12,
                        fondoElem
                    };
                    SetTextColor(contextoMemoria, RGB(139, 148, 158));
                    DrawTextW(contextoMemoria, txtTamano.c_str(), -1, &areaTam,
                              DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                }
            }
        }
        posicionVerticalCursor += 6;
    }

    SelectClipRgn(contextoMemoria, nullptr);
    DeleteObject(regionRecorte);
    DeleteObject(fuenteItemArbol);
    DeleteObject(fuenteTituloSeccion);
}

void dibujarControlesComprimidos(HDC contextoMemoria, HWND ventana,
                                 const RECT& areaCliente) {
    DisenoComprimidos diseno = calcularDisenoComprimidos(ventana);

    HFONT fuentePrevia =
        static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTexto));
    SetBkMode(contextoMemoria, TRANSPARENT);

    // --- CABECERA Y SEPARADOR DE OPERACIONES ---
    HPEN lapizSepOp = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSepOp = static_cast<HPEN>(SelectObject(contextoMemoria, lapizSepOp));
    MoveToEx(contextoMemoria, diseno.areaSeparadorOperaciones.left, diseno.areaSeparadorOperaciones.top, nullptr);
    LineTo(contextoMemoria, diseno.areaSeparadorOperaciones.right, diseno.areaSeparadorOperaciones.top);
    SelectObject(contextoMemoria, lapizPrevSepOp);
    DeleteObject(lapizSepOp);

    SelectObject(contextoMemoria, fuenteTextoPequeno);
    SetTextColor(contextoMemoria, RGB(139, 148, 158));
    DrawTextW(contextoMemoria, L"⚡ OPERACIONES CON ARCHIVOS COMPRIMIDOS", -1,
              &diseno.areaTituloOperaciones,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // --- 1. FILA ABRIR CON ---
    SelectObject(contextoMemoria, fuenteTexto);
    SetTextColor(contextoMemoria, RGB(220, 220, 220));
    DrawTextW(contextoMemoria, L"Abrir con:", -1,
              &diseno.areaEtiquetaAbrirCon,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    HBRUSH brochaProg = CreateSolidBrush(
        estadoComprimidos.menuProgramasAbierto ? RGB(45, 45, 45)
                                               : RGB(35, 35, 35));
    HPEN lapizProg = CreatePen(PS_SOLID, 1, RGB(70, 70, 70));
    HBRUSH brochaPreviaProg =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaProg));
    HPEN lapizPrevioProg =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizProg));
    RoundRect(contextoMemoria, diseno.botonDesplegablePrograma.left,
              diseno.botonDesplegablePrograma.top,
              diseno.botonDesplegablePrograma.right,
              diseno.botonDesplegablePrograma.bottom, 6, 6);
    SelectObject(contextoMemoria, brochaPreviaProg);
    SelectObject(contextoMemoria, lapizPrevioProg);
    DeleteObject(brochaProg);
    DeleteObject(lapizProg);

    RECT areaTextoProg = diseno.botonDesplegablePrograma;
    areaTextoProg.left += 8;
    areaTextoProg.right -= 22;
    SetTextColor(contextoMemoria, RGB(240, 240, 240));

    std::wstring textoProgramaActual = L"Añade un programa";
    std::wstring rutaResuelta = resolverProgramaParaComprimidos();
    if (!rutaResuelta.empty()) {
        textoProgramaActual = PathFindFileNameW(rutaResuelta.c_str());
    }

    DrawTextW(contextoMemoria, textoProgramaActual.c_str(), -1,
              &areaTextoProg,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS |
                  DT_NOPREFIX);

    RECT areaFlechaProg = diseno.botonDesplegablePrograma;
    areaFlechaProg.left = areaFlechaProg.right - 20;
    SelectObject(contextoMemoria, fuenteIconos);
    SetTextColor(contextoMemoria, RGB(180, 180, 180));
    DrawTextW(contextoMemoria, GLIFO_CHEVRON_ARRIBA, -1, &areaFlechaProg,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    HBRUSH brochaAbrir = CreateSolidBrush(RGB(0, 120, 215));
    HPEN lapizAbrir = CreatePen(PS_SOLID, 1, RGB(0, 140, 240));
    HBRUSH brochaPreviaAbrir =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaAbrir));
    HPEN lapizPrevioAbrir =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizAbrir));
    RoundRect(contextoMemoria, diseno.botonEjecutarAbrir.left,
              diseno.botonEjecutarAbrir.top,
              diseno.botonEjecutarAbrir.right,
              diseno.botonEjecutarAbrir.bottom, 6, 6);
    SelectObject(contextoMemoria, brochaPreviaAbrir);
    SelectObject(contextoMemoria, lapizPrevioAbrir);
    DeleteObject(brochaAbrir);
    DeleteObject(lapizAbrir);

    SelectObject(contextoMemoria, fuenteTextoPequeno);
    SetTextColor(contextoMemoria, RGB(255, 255, 255));
    DrawTextW(contextoMemoria, L"Abrir", -1,
              &diseno.botonEjecutarAbrir,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // --- 2. FILA CONVERTIR A ---
    SelectObject(contextoMemoria, fuenteTexto);
    SetTextColor(contextoMemoria, RGB(220, 220, 220));
    DrawTextW(contextoMemoria, L"Convertir a:", -1,
              &diseno.areaEtiquetaConversion,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    HBRUSH brochaCombo = CreateSolidBrush(
        estadoComprimidos.menuDesplegableAbierto ? RGB(45, 45, 45)
                                                 : RGB(35, 35, 35));
    HPEN lapizCombo = CreatePen(PS_SOLID, 1, RGB(70, 70, 70));
    HBRUSH brochaPreviaCombo =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaCombo));
    HPEN lapizPrevioCombo =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizCombo));
    RoundRect(contextoMemoria, diseno.botonDesplegableFormato.left,
              diseno.botonDesplegableFormato.top,
              diseno.botonDesplegableFormato.right,
              diseno.botonDesplegableFormato.bottom, 6, 6);
    SelectObject(contextoMemoria, brochaPreviaCombo);
    SelectObject(contextoMemoria, lapizPrevioCombo);
    DeleteObject(brochaCombo);
    DeleteObject(lapizCombo);

    RECT areaTextoCombo = diseno.botonDesplegableFormato;
    areaTextoCombo.left += 8;
    areaTextoCombo.right -= 22;
    SetTextColor(contextoMemoria, RGB(240, 240, 240));
    DrawTextW(contextoMemoria,
              FORMATOS_DESTINO[estadoComprimidos.indiceFormatoDestino], -1,
              &areaTextoCombo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS |
                  DT_NOPREFIX);

    RECT areaFlechaCombo = diseno.botonDesplegableFormato;
    areaFlechaCombo.left = areaFlechaCombo.right - 20;
    SelectObject(contextoMemoria, fuenteIconos);
    SetTextColor(contextoMemoria, RGB(180, 180, 180));
    DrawTextW(contextoMemoria, GLIFO_CHEVRON_ARRIBA, -1, &areaFlechaCombo,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    HBRUSH brochaBoton = CreateSolidBrush(
        estadoComprimidos.conversionEnCurso ? RGB(0, 90, 160)
                                            : RGB(0, 120, 215));
    HPEN lapizBoton = CreatePen(
        PS_SOLID, 1,
        estadoComprimidos.conversionEnCurso ? RGB(0, 110, 180)
                                            : RGB(0, 140, 240));
    HBRUSH brochaPreviaBoton =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaBoton));
    HPEN lapizPrevioBoton =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizBoton));
    RoundRect(contextoMemoria, diseno.botonEjecutarConversion.left,
              diseno.botonEjecutarConversion.top,
              diseno.botonEjecutarConversion.right,
              diseno.botonEjecutarConversion.bottom, 6, 6);
    SelectObject(contextoMemoria, brochaPreviaBoton);
    SelectObject(contextoMemoria, lapizPrevioBoton);
    DeleteObject(brochaBoton);
    DeleteObject(lapizBoton);

    SelectObject(contextoMemoria, fuenteTextoPequeno);
    SetTextColor(contextoMemoria, RGB(255, 255, 255));
    std::wstring textoBoton =
        estadoComprimidos.conversionEnCurso ? L"Convirtiendo..." : L"Convertir";
    DrawTextW(contextoMemoria, textoBoton.c_str(), -1,
              &diseno.botonEjecutarConversion,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // --- 3. PANELES FLOTANTES DESPLEGABLES ---

    SelectObject(contextoMemoria, fuentePrevia);
}

inline void dibujarBarraControlesPendiente(HDC contextoDibujo,
                                           const RECT& areaCliente,
                                           TipoContenidoArchivo tipo) {
    HBRUSH pincelFondo = CreateSolidBrush(RGB(18, 18, 18));
    FillRect(contextoDibujo, &areaCliente, pincelFondo);
    DeleteObject(pincelFondo);

    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(45, 45, 45));
    HPEN lapizPrevio = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    MoveToEx(contextoDibujo, areaCliente.left, areaCliente.top, nullptr);
    LineTo(contextoDibujo, areaCliente.right, areaCliente.top);
    SelectObject(contextoDibujo, lapizPrevio);
    DeleteObject(lapizBorde);

    EstadoPendienteImplementacion& estado =
        (tipo == TipoContenidoArchivo::Documento) ? estadoDocumentos : estadoCodigo;

    int anchoBoton = 84;
    int altoBoton = 24;
    int margenVertical = (areaCliente.bottom - areaCliente.top - altoBoton) / 2;

    estado.botonAnterior = {
        areaCliente.left + 12,
        areaCliente.top + margenVertical,
        areaCliente.left + 12 + anchoBoton,
        areaCliente.top + margenVertical + altoBoton
    };

    estado.botonSiguiente = {
        areaCliente.right - 12 - anchoBoton,
        areaCliente.top + margenVertical,
        areaCliente.right - 12,
        areaCliente.top + margenVertical + altoBoton
    };

    auto dibujarBotonNav = [&](const RECT& rect, const wchar_t* texto, bool hover) {
        COLORREF colorFondo = hover ? RGB(48, 54, 61) : RGB(33, 38, 45);
        COLORREF colorBorde = hover ? RGB(110, 118, 129) : RGB(48, 54, 61);
        HBRUSH brochaBtn = CreateSolidBrush(colorFondo);
        HPEN lapizBtn = CreatePen(PS_SOLID, 1, colorBorde);
        HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoDibujo, brochaBtn));
        HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBtn));
        RoundRect(contextoDibujo, rect.left, rect.top, rect.right, rect.bottom, 6, 6);
        SelectObject(contextoDibujo, lapizPrev);
        SelectObject(contextoDibujo, brochaPrev);
        DeleteObject(lapizBtn);
        DeleteObject(brochaBtn);

        SetBkMode(contextoDibujo, TRANSPARENT);
        SetTextColor(contextoDibujo, hover ? RGB(255, 255, 255) : RGB(201, 209, 217));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        RECT rTexto = rect;
        DrawTextW(contextoDibujo, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    };

    dibujarBotonNav(estado.botonAnterior, L"◀ Anterior", estado.hoverBotonAnterior);
    dibujarBotonNav(estado.botonSiguiente, L"Siguiente ▶", estado.hoverBotonSiguiente);

    std::wstring textoCentro;
    if (estado.archivosSeleccionados.size() > 1) {
        textoCentro = estado.nombreArchivo + L"  \u2022  " +
                      std::to_wstring(estado.indiceActual + 1) + L"/" +
                      std::to_wstring(estado.archivosSeleccionados.size());
    } else {
        textoCentro = estado.nombreArchivo;
    }
    if (!textoCentro.empty()) {
        RECT areaCentro = {
            estado.botonAnterior.right + 8,
            areaCliente.top,
            estado.botonSiguiente.left - 8,
            areaCliente.bottom
        };
        SetBkMode(contextoDibujo, TRANSPARENT);
        SetTextColor(contextoDibujo, RGB(139, 148, 158));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        DrawTextW(contextoDibujo, textoCentro.c_str(), -1, &areaCentro,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    }
}

inline void dibujarLienzoPendienteImplementacion(HDC contextoDestino,
                                                const RECT& areaDibujo,
                                                TipoContenidoArchivo tipo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTitulo = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubtitulo = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteTextoNormal = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteBadge = CreateFontW(-11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int yActual = areaDibujo.top + 24;

    SelectObject(contextoDestino, fuenteTitulo);
    SetTextColor(contextoDestino, RGB(255, 255, 255));
    RECT areaTitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 28};
    DrawTextW(contextoDestino, L"Useful Sideview", -1, &areaTitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 32;

    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(88, 166, 255));
    RECT areaSubtitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 22};
    std::wstring etiquetaCategoria =
        (tipo == TipoContenidoArchivo::Documento)
            ? L"\U0001F4C4 Documentos (PDF / Word)"
            : L"\U0001F4BB Códigos de Programación";
    DrawTextW(contextoDestino, etiquetaCategoria.c_str(), -1, &areaSubtitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 34;

    EstadoPendienteImplementacion& estado =
        (tipo == TipoContenidoArchivo::Documento) ? estadoDocumentos : estadoCodigo;

    int altoTarjeta = 146;
    RECT areaTarjeta{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + altoTarjeta};
    HBRUSH brochaTarjeta = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizTarjeta = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaTarjeta));
    HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoDestino, lapizTarjeta));
    RoundRect(contextoDestino, areaTarjeta.left, areaTarjeta.top,
              areaTarjeta.right, areaTarjeta.bottom, 8, 8);
    SelectObject(contextoDestino, lapizPrev);
    SelectObject(contextoDestino, brochaPrev);
    DeleteObject(lapizTarjeta);
    DeleteObject(brochaTarjeta);

    int yCard = areaTarjeta.top + 14;

    // Nombre de archivo
    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaNombreArchivo{areaTarjeta.left + 16, yCard, areaTarjeta.right - 16, yCard + 20};
    DrawTextW(contextoDestino, estado.nombreArchivo.c_str(), -1, &areaNombreArchivo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    yCard += 26;

    // Badge "Pendiente de implementación"
    RECT areaBadge{areaTarjeta.left + 16, yCard, areaTarjeta.left + 215, yCard + 22};
    HBRUSH brochaBadge = CreateSolidBrush(RGB(56, 35, 12));
    HPEN lapizBadge = CreatePen(PS_SOLID, 1, RGB(187, 128, 9));
    HBRUSH brochaPrevB = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaBadge));
    HPEN lapizPrevB = static_cast<HPEN>(SelectObject(contextoDestino, lapizBadge));
    RoundRect(contextoDestino, areaBadge.left, areaBadge.top, areaBadge.right, areaBadge.bottom, 4, 4);
    SelectObject(contextoDestino, lapizPrevB);
    SelectObject(contextoDestino, brochaPrevB);
    DeleteObject(lapizBadge);
    DeleteObject(brochaBadge);

    SelectObject(contextoDestino, fuenteBadge);
    SetTextColor(contextoDestino, RGB(227, 179, 65));
    DrawTextW(contextoDestino, L"\u23F3 Pendiente de implementación", -1, &areaBadge,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yCard += 28;

    // Mensaje descriptivo
    SelectObject(contextoDestino, fuenteTextoNormal);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    std::wstring mensaje =
        (tipo == TipoContenidoArchivo::Documento)
            ? L"El visor para documentos PDF y paquetería de Microsoft Word está en desarrollo y se integrará en una próxima versión."
            : L"El visor con resaltado de sintaxis para lenguajes de programación está en desarrollo y se integrará en una próxima versión.";

    RECT areaMensaje{areaTarjeta.left + 16, yCard, areaTarjeta.right - 16, areaTarjeta.bottom - 12};
    DrawTextW(contextoDestino, mensaje.c_str(), -1, &areaMensaje,
              DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

    DeleteObject(fuenteTitulo);
    DeleteObject(fuenteSubtitulo);
    DeleteObject(fuenteTextoNormal);
    DeleteObject(fuenteBadge);
}

inline void dibujarBarraControlesDesconocido(HDC contextoDibujo,
                                             const RECT& areaCliente) {
    HBRUSH pincelFondo = CreateSolidBrush(RGB(18, 18, 18));
    FillRect(contextoDibujo, &areaCliente, pincelFondo);
    DeleteObject(pincelFondo);

    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(45, 45, 45));
    HPEN lapizPrevio = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    MoveToEx(contextoDibujo, areaCliente.left, areaCliente.top, nullptr);
    LineTo(contextoDibujo, areaCliente.right, areaCliente.top);
    SelectObject(contextoDibujo, lapizPrevio);
    DeleteObject(lapizBorde);

    int anchoBoton = 84;
    int altoBoton = 24;
    int margenVertical = (areaCliente.bottom - areaCliente.top - altoBoton) / 2;

    estadoDesconocido.botonAnterior = {
        areaCliente.left + 12,
        areaCliente.top + margenVertical,
        areaCliente.left + 12 + anchoBoton,
        areaCliente.top + margenVertical + altoBoton
    };

    estadoDesconocido.botonSiguiente = {
        areaCliente.right - 12 - anchoBoton,
        areaCliente.top + margenVertical,
        areaCliente.right - 12,
        areaCliente.top + margenVertical + altoBoton
    };

    auto dibujarBotonNav = [&](const RECT& rect, const wchar_t* texto, bool hover) {
        COLORREF colorFondo = hover ? RGB(48, 54, 61) : RGB(33, 38, 45);
        COLORREF colorBorde = hover ? RGB(110, 118, 129) : RGB(48, 54, 61);
        HBRUSH brochaBtn = CreateSolidBrush(colorFondo);
        HPEN lapizBtn = CreatePen(PS_SOLID, 1, colorBorde);
        HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoDibujo, brochaBtn));
        HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBtn));
        RoundRect(contextoDibujo, rect.left, rect.top, rect.right, rect.bottom, 6, 6);
        SelectObject(contextoDibujo, lapizPrev);
        SelectObject(contextoDibujo, brochaPrev);
        DeleteObject(lapizBtn);
        DeleteObject(brochaBtn);

        SetBkMode(contextoDibujo, TRANSPARENT);
        SetTextColor(contextoDibujo, hover ? RGB(255, 255, 255) : RGB(201, 209, 217));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        RECT rTexto = rect;
        DrawTextW(contextoDibujo, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    };

    dibujarBotonNav(estadoDesconocido.botonAnterior, L"◀ Anterior", estadoDesconocido.hoverBotonAnterior);
    dibujarBotonNav(estadoDesconocido.botonSiguiente, L"Siguiente ▶", estadoDesconocido.hoverBotonSiguiente);

    std::wstring textoCentro;
    if (estadoDesconocido.archivosSeleccionados.size() > 1) {
        textoCentro = estadoDesconocido.nombreArchivo + L"  \u2022  " +
                      std::to_wstring(estadoDesconocido.indiceActual + 1) + L"/" +
                      std::to_wstring(estadoDesconocido.archivosSeleccionados.size());
    } else {
        textoCentro = estadoDesconocido.nombreArchivo;
    }
    if (!textoCentro.empty()) {
        RECT areaCentro = {
            estadoDesconocido.botonAnterior.right + 8,
            areaCliente.top,
            estadoDesconocido.botonSiguiente.left - 8,
            areaCliente.bottom
        };
        SetBkMode(contextoDibujo, TRANSPARENT);
        SetTextColor(contextoDibujo, RGB(139, 148, 158));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        DrawTextW(contextoDibujo, textoCentro.c_str(), -1, &areaCentro,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    }
}

inline void dibujarLienzoArchivoDesconocido(HDC contextoDestino,
                                           const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTitulo = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubtitulo = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteTextoNormal = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteBadge = CreateFontW(-11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int yActual = areaDibujo.top + 24;

    SelectObject(contextoDestino, fuenteTitulo);
    SetTextColor(contextoDestino, RGB(255, 255, 255));
    RECT areaTitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 28};
    DrawTextW(contextoDestino, L"Useful Sideview", -1, &areaTitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 32;

    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaSubtitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 22};
    DrawTextW(contextoDestino, L"Archivo no compatible / desconocido", -1, &areaSubtitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 34;

    int altoTarjeta = 146;
    RECT areaTarjeta{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + altoTarjeta};
    HBRUSH brochaTarjeta = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizTarjeta = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaTarjeta));
    HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoDestino, lapizTarjeta));
    RoundRect(contextoDestino, areaTarjeta.left, areaTarjeta.top,
              areaTarjeta.right, areaTarjeta.bottom, 8, 8);
    SelectObject(contextoDestino, lapizPrev);
    SelectObject(contextoDestino, brochaPrev);
    DeleteObject(lapizTarjeta);
    DeleteObject(brochaTarjeta);

    int yCard = areaTarjeta.top + 14;

    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaNombreArchivo{areaTarjeta.left + 16, yCard, areaTarjeta.right - 16, yCard + 20};
    DrawTextW(contextoDestino, estadoDesconocido.nombreArchivo.c_str(), -1, &areaNombreArchivo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    yCard += 26;

    // Badge "Archivo desconocido"
    RECT areaBadge{areaTarjeta.left + 16, yCard, areaTarjeta.left + 190, yCard + 22};
    HBRUSH brochaBadge = CreateSolidBrush(RGB(33, 38, 45));
    HPEN lapizBadge = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPrevB = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaBadge));
    HPEN lapizPrevB = static_cast<HPEN>(SelectObject(contextoDestino, lapizBadge));
    RoundRect(contextoDestino, areaBadge.left, areaBadge.top, areaBadge.right, areaBadge.bottom, 4, 4);
    SelectObject(contextoDestino, lapizPrevB);
    SelectObject(contextoDestino, brochaPrevB);
    DeleteObject(lapizBadge);
    DeleteObject(brochaBadge);

    SelectObject(contextoDestino, fuenteBadge);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    DrawTextW(contextoDestino, L"Archivo desconocido", -1, &areaBadge,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yCard += 28;

    SelectObject(contextoDestino, fuenteTextoNormal);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    std::wstring mensaje = L"No hay una previsualización disponible para este tipo de archivo";
    if (!estadoDesconocido.tamanoFormateado.empty()) {
        mensaje += L" (" + estadoDesconocido.tamanoFormateado + L")";
    }
    mensaje += L".";

    RECT areaMensaje{areaTarjeta.left + 16, yCard, areaTarjeta.right - 16, areaTarjeta.bottom - 12};
    DrawTextW(contextoDestino, mensaje.c_str(), -1, &areaMensaje,
              DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

    DeleteObject(fuenteTitulo);
    DeleteObject(fuenteSubtitulo);
    DeleteObject(fuenteTextoNormal);
    DeleteObject(fuenteBadge);
}

void dibujarInterfaz(HWND ventana) {
    PAINTSTRUCT pintura;
    HDC contextoPantalla = BeginPaint(ventana, &pintura);
    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);

    RECT areaCliente = disenoBarraProgreso.barraControles;
    int anchoCliente = areaCliente.right - areaCliente.left;
    int altoCliente = areaCliente.bottom - areaCliente.top;

    if (anchoCliente <= 0 || altoCliente <= 0) {
        EndPaint(ventana, &pintura);
        return;
    }

    HDC contextoMemoria = CreateCompatibleDC(contextoPantalla);
    HBITMAP mapaBitsMemoria =
        CreateCompatibleBitmap(contextoPantalla, anchoCliente, altoCliente);
    HBITMAP mapaBitsAnterior = static_cast<HBITMAP>(
        SelectObject(contextoMemoria, mapaBitsMemoria));

    HBRUSH pincelFondo = CreateSolidBrush(RGB(25, 25, 25));
    FillRect(contextoMemoria, &areaCliente, pincelFondo);
    DeleteObject(pincelFondo);
    SetBkMode(contextoMemoria, TRANSPARENT);
    SetTextColor(contextoMemoria, RGB(245, 245, 245));

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
        dibujarVistaTexto(contextoMemoria, areaCliente);
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);
        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento ||
        tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
        dibujarBarraControlesPendiente(contextoMemoria, areaCliente, tipoContenidoActivo.load());
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);
        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
        dibujarBarraControlesDesconocido(contextoMemoria, areaCliente);
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);
        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
        dibujarControlesComprimidos(contextoMemoria, ventana, areaCliente);
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);
        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
        dibujarBarraControlesImagen(contextoMemoria, areaCliente);
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);

        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

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
            static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTexto));
        SetTextColor(contextoMemoria, RGB(210, 210, 210));
        DrawTextW(contextoMemoria, textoTranscurrido.c_str(), -1,
                  &areaTranscurrido,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        DrawTextW(contextoMemoria, textoRestante.c_str(), -1, &areaRestante,
                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoMemoria, fuenteAnterior);
    }

    dibujarLinea(contextoMemoria, disenoBarraProgreso.limiteIzquierdo,
                 disenoBarraProgreso.limiteDerecho,
                 disenoBarraProgreso.centroVertical, 3,
                 tiempoDisponible && posicionamientoDisponible.load()
                     ? RGB(92, 92, 92)
                     : RGB(58, 58, 58));
    int posicionPerilla =
        calcularPosicionProgreso(disenoBarraProgreso, progresoNormalizado);
    dibujarLinea(contextoMemoria, disenoBarraProgreso.limiteIzquierdo,
                 posicionPerilla, disenoBarraProgreso.centroVertical, 4,
                 RGB(0, 120, 215));

    HBRUSH pincelPerilla = CreateSolidBrush(RGB(245, 245, 245));
    HBRUSH pincelPreviamenteSeleccionado =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, pincelPerilla));
    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(0, 120, 215));
    HPEN lapizPreviamenteSeleccionado =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizBorde));
    Ellipse(contextoMemoria, posicionPerilla - RADIO_PERILLA_PROGRESO,
            disenoBarraProgreso.centroVertical - RADIO_PERILLA_PROGRESO,
            posicionPerilla + RADIO_PERILLA_PROGRESO + 1,
            disenoBarraProgreso.centroVertical + RADIO_PERILLA_PROGRESO + 1);
    SelectObject(contextoMemoria, lapizPreviamenteSeleccionado);
    SelectObject(contextoMemoria, pincelPreviamenteSeleccionado);
    DeleteObject(lapizBorde);
    DeleteObject(pincelPerilla);

    bool reproduciendo =
        estadoReproduccion == MFP_MEDIAPLAYER_STATE_PLAYING;
    bool parpadeoActivo = (GetTickCount64() - estadoVideoMetadatos.tiempoInicioParpadeoNav < 400);

    dibujarBotonControl(contextoMemoria,
                         disenoBarraProgreso.botonAnterior,
                         GLIFO_ANTERIOR, parpadeoActivo, false, parpadeoActivo);
    dibujarBotonControl(contextoMemoria,
                         disenoBarraProgreso.botonReproduccion,
                         reproduciendo ? GLIFO_PAUSAR : GLIFO_REPRODUCIR,
                         reproduciendo, true);
    dibujarBotonControl(contextoMemoria,
                         disenoBarraProgreso.botonSiguiente,
                         GLIFO_SIGUIENTE, parpadeoActivo, false, parpadeoActivo);
    dibujarBotonControl(contextoMemoria, disenoBarraProgreso.botonSonido,
                         sonidoSilenciado ? GLIFO_SILENCIO : GLIFO_VOLUMEN,
                         !sonidoSilenciado, false);
    dibujarBotonControl(contextoMemoria,
                         disenoBarraProgreso.botonRepeticion,
                         GLIFO_REPETICION, repeticionHabilitada.load(),
                         false);

    std::wstring etiquetaVelocidad =
        formatearEtiquetaVelocidad(velocidadReproduccionActual.load());
    dibujarBotonTexto(contextoMemoria, disenoBarraProgreso.botonVelocidad,
                      etiquetaVelocidad.c_str(), menuVelocidadDesplegado);

    BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente, contextoMemoria,
           0, 0, SRCCOPY);

    SelectObject(contextoMemoria, mapaBitsAnterior);
    DeleteObject(mapaBitsMemoria);
    DeleteDC(contextoMemoria);

    EndPaint(ventana, &pintura);
}

void dibujarCabeceraMetadatosVideo(HDC contextoDestino, const RECT& areaMetadatos) {
    int anchoArea = areaMetadatos.right - areaMetadatos.left;
    int altoArea = areaMetadatos.bottom - areaMetadatos.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaMetadatos, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteBadge = CreateFontW(-11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteTitulo = CreateFontW(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubtitulo = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int posicionVertical = areaMetadatos.top + 8;

    // 1. Badge "🎬 VIDEO" y contador de posición / rango
    SelectObject(contextoDestino, fuenteBadge);

    // Pill "🎬 VIDEO"
    std::wstring etiquetaTipo = L"🎬 VIDEO";
    SIZE tamanoTipo{};
    GetTextExtentPoint32W(contextoDestino, etiquetaTipo.c_str(), static_cast<int>(etiquetaTipo.size()), &tamanoTipo);
    RECT areaPillTipo = {
        areaMetadatos.left + 14,
        posicionVertical,
        areaMetadatos.left + 14 + tamanoTipo.cx + 14,
        posicionVertical + 20
    };
    HBRUSH brochaPillTipo = CreateSolidBrush(RGB(30, 41, 59));
    HPEN lapizPillTipo = CreatePen(PS_SOLID, 1, RGB(56, 139, 253));
    HBRUSH brochaPreviaPill = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaPillTipo));
    HPEN lapizPrevioPill = static_cast<HPEN>(SelectObject(contextoDestino, lapizPillTipo));
    RoundRect(contextoDestino, areaPillTipo.left, areaPillTipo.top, areaPillTipo.right, areaPillTipo.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioPill);
    SelectObject(contextoDestino, brochaPreviaPill);
    DeleteObject(lapizPillTipo);
    DeleteObject(brochaPillTipo);

    SetTextColor(contextoDestino, RGB(88, 166, 255));
    DrawTextW(contextoDestino, etiquetaTipo.c_str(), -1, &areaPillTipo,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // Pill contador (Rango vs Carpeta vs Único)
    std::wstring etiquetaContador;
    if (estadoVideoMetadatos.esRangoSeleccionado) {
        etiquetaContador = std::to_wstring(estadoVideoMetadatos.indiceEnRango + 1) + L"/" +
                           std::to_wstring(estadoVideoMetadatos.totalEnRango) + L" (Rango)";
    } else if (estadoVideoMetadatos.totalEnCarpeta > 1) {
        etiquetaContador = std::to_wstring(estadoVideoMetadatos.indiceEnCarpeta + 1) + L"/" +
                           std::to_wstring(estadoVideoMetadatos.totalEnCarpeta) + L" (Carpeta)";
    } else {
        etiquetaContador = L"1/1 (Único)";
    }

    SIZE tamanoContador{};
    GetTextExtentPoint32W(contextoDestino, etiquetaContador.c_str(), static_cast<int>(etiquetaContador.size()), &tamanoContador);
    RECT areaPillContador = {
        areaPillTipo.right + 8,
        posicionVertical,
        areaPillTipo.right + 8 + tamanoContador.cx + 14,
        posicionVertical + 20
    };
    HBRUSH brochaPillCont = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizPillCont = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPreviaCont = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaPillCont));
    HPEN lapizPrevioCont = static_cast<HPEN>(SelectObject(contextoDestino, lapizPillCont));
    RoundRect(contextoDestino, areaPillContador.left, areaPillContador.top, areaPillContador.right, areaPillContador.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioCont);
    SelectObject(contextoDestino, brochaPreviaCont);
    DeleteObject(lapizPillCont);
    DeleteObject(brochaPillCont);

    SetTextColor(contextoDestino, RGB(139, 148, 158));
    DrawTextW(contextoDestino, etiquetaContador.c_str(), -1, &areaPillContador,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    posicionVertical += 25;

    // 2. Nombre del video
    SelectObject(contextoDestino, fuenteTitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaNombreVideo = {
        areaMetadatos.left + 14,
        posicionVertical,
        areaMetadatos.right - 14,
        posicionVertical + 22
    };
    std::wstring nombreMostrar = estadoVideoMetadatos.nombreArchivo.empty()
                                     ? PathFindFileNameW(rutaVideoActiva.c_str())
                                     : estadoVideoMetadatos.nombreArchivo;
    DrawTextW(contextoDestino, nombreMostrar.c_str(), -1, &areaNombreVideo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    posicionVertical += 24;

    // 3. Metadatos secundarios (Tamaño, formato, fecha)
    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    std::wstring infoSecundaria;
    if (!estadoVideoMetadatos.textoTamano.empty()) {
        infoSecundaria += L"📦 " + estadoVideoMetadatos.textoTamano;
    }
    if (!estadoVideoMetadatos.extensionFormato.empty()) {
        if (!infoSecundaria.empty()) infoSecundaria += L"   •   ";
        infoSecundaria += L"🏷️ " + estadoVideoMetadatos.extensionFormato;
    }
    if (!estadoVideoMetadatos.textoFechaModificacion.empty()) {
        if (!infoSecundaria.empty()) infoSecundaria += L"   •   ";
        infoSecundaria += L"📅 " + estadoVideoMetadatos.textoFechaModificacion;
    }
    RECT areaInfoSecundaria = {
        areaMetadatos.left + 14,
        posicionVertical,
        areaMetadatos.right - 14,
        posicionVertical + 18
    };
    DrawTextW(contextoDestino, infoSecundaria.c_str(), -1, &areaInfoSecundaria,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    // Línea divisoria inferior
    int ySeparador = areaMetadatos.bottom - 1;
    HPEN lapizSeparador = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevioSep = static_cast<HPEN>(SelectObject(contextoDestino, lapizSeparador));
    MoveToEx(contextoDestino, areaMetadatos.left + 14, ySeparador, nullptr);
    LineTo(contextoDestino, areaMetadatos.right - 14, ySeparador);
    SelectObject(contextoDestino, lapizPrevioSep);
    DeleteObject(lapizSeparador);

    DeleteObject(fuenteBadge);
    DeleteObject(fuenteTitulo);
    DeleteObject(fuenteSubtitulo);
}

