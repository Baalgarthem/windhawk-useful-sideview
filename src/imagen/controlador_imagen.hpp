#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <wincodec.h>
#include <algorithm>
#include <string>
#include <vector>

struct EstadoVisorImagen {
    std::wstring rutaImagenActual;
    std::vector<std::wstring> imagenesSeleccionadas;
    size_t indiceImagenSeleccionada{0};
    Gdiplus::Image* imagenCargada{nullptr};
    UINT anchoOriginal{0};
    UINT altoOriginal{0};
    size_t posicionEnCarpeta{0};
    size_t totalEnCarpeta{0};
    bool esAnimada{false};
    UINT totalCuadros{1};
    UINT indiceCuadroActual{0};
    std::vector<UINT> retardosCuadros;
    RECT botonAnterior{};
    RECT botonSiguiente{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};
};

inline EstadoVisorImagen estadoImagen;

inline void liberarRecursosImagen() {
    if (ventanaVideo) {
        KillTimer(ventanaVideo, ID_TEMPORIZADOR_ANIMACION_GIF);
    }
    if (estadoImagen.imagenCargada) {
        delete estadoImagen.imagenCargada;
        estadoImagen.imagenCargada = nullptr;
    }
    estadoImagen.rutaImagenActual.clear();
    estadoImagen.imagenesSeleccionadas.clear();
    estadoImagen.indiceImagenSeleccionada = 0;
    estadoImagen.anchoOriginal = 0;
    estadoImagen.altoOriginal = 0;
    estadoImagen.posicionEnCarpeta = 0;
    estadoImagen.totalEnCarpeta = 0;
    estadoImagen.esAnimada = false;
    estadoImagen.totalCuadros = 1;
    estadoImagen.indiceCuadroActual = 0;
    estadoImagen.retardosCuadros.clear();
    estadoImagen.hoverBotonAnterior = false;
    estadoImagen.hoverBotonSiguiente = false;
}

inline Gdiplus::Bitmap* decodificarImagenConWic(
    const std::wstring& rutaArchivo) {
    PunteroCom<IWICImagingFactory> fabricaWic;
    HRESULT resultadoFabrica = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(fabricaWic.recibir()));
    if (FAILED(resultadoFabrica) || !fabricaWic.obtener()) {
        resultadoFabrica = CoCreateInstance(
            CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(fabricaWic.recibir()));
        if (FAILED(resultadoFabrica) || !fabricaWic.obtener()) return nullptr;
    }

    PunteroCom<IWICBitmapDecoder> decodificador;
    HRESULT resultadoDecodificador = fabricaWic->CreateDecoderFromFilename(
        rutaArchivo.c_str(), nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnDemand, decodificador.recibir());
    if (FAILED(resultadoDecodificador) || !decodificador.obtener())
        return nullptr;

    PunteroCom<IWICBitmapFrameDecode> cuadroDecodificado;
    HRESULT resultadoCuadro =
        decodificador->GetFrame(0, cuadroDecodificado.recibir());
    if (FAILED(resultadoCuadro) || !cuadroDecodificado.obtener()) return nullptr;

    PunteroCom<IWICFormatConverter> convertidorFormato;
    HRESULT resultadoConvertidor =
        fabricaWic->CreateFormatConverter(convertidorFormato.recibir());
    if (FAILED(resultadoConvertidor) || !convertidorFormato.obtener())
        return nullptr;

    HRESULT resultadoInicio = convertidorFormato->Initialize(
        cuadroDecodificado.obtener(), GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(resultadoInicio)) return nullptr;

    UINT anchoImagen = 0;
    UINT altoImagen = 0;
    HRESULT resultadoDimensiones =
        convertidorFormato->GetSize(&anchoImagen, &altoImagen);
    if (FAILED(resultadoDimensiones) || anchoImagen == 0 || altoImagen == 0)
        return nullptr;

    UINT pasoBytesFila = anchoImagen * 4;
    UINT tamanoTotalBufer = pasoBytesFila * altoImagen;
    std::vector<BYTE> buferPixeles(tamanoTotalBufer);

    HRESULT resultadoCopia = convertidorFormato->CopyPixels(
        nullptr, pasoBytesFila, tamanoTotalBufer, buferPixeles.data());
    if (FAILED(resultadoCopia)) return nullptr;

    Gdiplus::Bitmap* mapaBitsTemporal = new Gdiplus::Bitmap(
        static_cast<INT>(anchoImagen), static_cast<INT>(altoImagen),
        static_cast<INT>(pasoBytesFila), PixelFormat32bppPARGB,
        buferPixeles.data());

    if (!mapaBitsTemporal ||
        mapaBitsTemporal->GetLastStatus() != Gdiplus::Ok) {
        if (mapaBitsTemporal) delete mapaBitsTemporal;
        return nullptr;
    }

    Gdiplus::Bitmap* mapaBitsFinal = mapaBitsTemporal->Clone(
        0, 0, static_cast<INT>(anchoImagen), static_cast<INT>(altoImagen),
        PixelFormat32bppPARGB);
    delete mapaBitsTemporal;

    return mapaBitsFinal;
}

inline void inicializarAnimacionGifSiAplica() {
    if (!estadoImagen.imagenCargada) return;

    GUID dimensionTiempo = Gdiplus::FrameDimensionTime;
    UINT cantidadDimensiones =
        estadoImagen.imagenCargada->GetFrameDimensionsCount();
    if (cantidadDimensiones == 0) return;

    std::vector<GUID> listaDimensiones(cantidadDimensiones);
    if (estadoImagen.imagenCargada->GetFrameDimensionsList(
            listaDimensiones.data(), cantidadDimensiones) != Gdiplus::Ok) {
        return;
    }

    bool soporteDimensionTiempo = false;
    for (UINT posicionDimension = 0; posicionDimension < cantidadDimensiones;
         ++posicionDimension) {
        if (listaDimensiones[posicionDimension] == dimensionTiempo) {
            soporteDimensionTiempo = true;
            break;
        }
    }

    if (!soporteDimensionTiempo) return;

    UINT cantidadCuadros =
        estadoImagen.imagenCargada->GetFrameCount(&dimensionTiempo);
    if (cantidadCuadros <= 1) return;

    estadoImagen.esAnimada = true;
    estadoImagen.totalCuadros = cantidadCuadros;
    estadoImagen.indiceCuadroActual = 0;
    estadoImagen.retardosCuadros.clear();

    UINT tamanoPropiedad =
        estadoImagen.imagenCargada->GetPropertyItemSize(PropertyTagFrameDelay);
    if (tamanoPropiedad > 0) {
        std::vector<BYTE> buferPropiedad(tamanoPropiedad);
        Gdiplus::PropertyItem* elementoPropiedad =
            reinterpret_cast<Gdiplus::PropertyItem*>(buferPropiedad.data());
        if (estadoImagen.imagenCargada->GetPropertyItem(
                PropertyTagFrameDelay, tamanoPropiedad, elementoPropiedad) ==
            Gdiplus::Ok) {
            long* arregloRetardos =
                reinterpret_cast<long*>(elementoPropiedad->value);
            UINT totalRetardosLeidos =
                elementoPropiedad->length / static_cast<UINT>(sizeof(long));
            for (UINT posicionCuadro = 0; posicionCuadro < cantidadCuadros;
                 ++posicionCuadro) {
                UINT retardoMilisegundos = 100;
                if (posicionCuadro < totalRetardosLeidos) {
                    long valorRetardoLeido = arregloRetardos[posicionCuadro];
                    if (valorRetardoLeido > 0) {
                        retardoMilisegundos =
                            static_cast<UINT>(valorRetardoLeido * 10);
                    }
                }
                if (retardoMilisegundos < 20) {
                    retardoMilisegundos = 100;
                }
                estadoImagen.retardosCuadros.push_back(retardoMilisegundos);
            }
        }
    }

    if (estadoImagen.retardosCuadros.empty()) {
        estadoImagen.retardosCuadros.assign(cantidadCuadros, 100);
    }

    if (ventanaVideo) {
        UINT retardoInicial = estadoImagen.retardosCuadros.front();
        SetTimer(ventanaVideo, ID_TEMPORIZADOR_ANIMACION_GIF, retardoInicial,
                 nullptr);
    }
}

inline void avanzarCuadroAnimacionGif(HWND ventanaObjetivo) {
    if (!estadoImagen.esAnimada || estadoImagen.totalCuadros <= 1 ||
        !estadoImagen.imagenCargada) {
        return;
    }

    estadoImagen.indiceCuadroActual =
        (estadoImagen.indiceCuadroActual + 1) % estadoImagen.totalCuadros;
    GUID dimensionTiempo = Gdiplus::FrameDimensionTime;
    estadoImagen.imagenCargada->SelectActiveFrame(
        &dimensionTiempo, estadoImagen.indiceCuadroActual);

    UINT siguienteRetardo = 100;
    if (estadoImagen.indiceCuadroActual <
        estadoImagen.retardosCuadros.size()) {
        siguienteRetardo =
            estadoImagen.retardosCuadros[estadoImagen.indiceCuadroActual];
    }
    SetTimer(ventanaObjetivo, ID_TEMPORIZADOR_ANIMACION_GIF, siguienteRetardo,
             nullptr);
    InvalidateRect(ventanaObjetivo, nullptr, FALSE);
}

inline bool cargarImagenPorRuta(const std::wstring& rutaNuevaImagen) {
    if (rutaNuevaImagen.empty()) {
        liberarRecursosImagen();
        return false;
    }

    liberarRecursosImagen();

    estadoImagen.imagenCargada =
        Gdiplus::Image::FromFile(rutaNuevaImagen.c_str());
    if (!estadoImagen.imagenCargada ||
        estadoImagen.imagenCargada->GetLastStatus() != Gdiplus::Ok) {
        if (estadoImagen.imagenCargada) {
            delete estadoImagen.imagenCargada;
            estadoImagen.imagenCargada = nullptr;
        }
        estadoImagen.imagenCargada = decodificarImagenConWic(rutaNuevaImagen);
    }

    if (!estadoImagen.imagenCargada ||
        estadoImagen.imagenCargada->GetLastStatus() != Gdiplus::Ok) {
        liberarRecursosImagen();
        return false;
    }

    estadoImagen.rutaImagenActual = rutaNuevaImagen;
    estadoImagen.anchoOriginal = estadoImagen.imagenCargada->GetWidth();
    estadoImagen.altoOriginal = estadoImagen.imagenCargada->GetHeight();

    inicializarAnimacionGifSiAplica();

    std::vector<std::wstring> listaImagenes =
        obtenerImagenesEnMismaCarpeta(rutaNuevaImagen);
    estadoImagen.totalEnCarpeta = listaImagenes.size();
    auto iterador = std::find(listaImagenes.begin(), listaImagenes.end(),
                              rutaNuevaImagen);
    if (iterador != listaImagenes.end()) {
        estadoImagen.posicionEnCarpeta =
            static_cast<size_t>(
                std::distance(listaImagenes.begin(), iterador)) +
            1;
    } else {
        estadoImagen.posicionEnCarpeta = 1;
    }

    return true;
}

inline void dibujarLienzoImagen(HDC contextoDestino, const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    if (!estadoImagen.imagenCargada || estadoImagen.anchoOriginal == 0 ||
        estadoImagen.altoOriginal == 0) {
        return;
    }

    float proporcionHorizontal =
        static_cast<float>(anchoArea) /
        static_cast<float>(estadoImagen.anchoOriginal);
    float proporcionVertical =
        static_cast<float>(altoArea) /
        static_cast<float>(estadoImagen.altoOriginal);
    float proporcionEscala =
        std::min(proporcionHorizontal, proporcionVertical);

    int anchoEscalado = static_cast<int>(
        static_cast<float>(estadoImagen.anchoOriginal) * proporcionEscala);
    int altoEscalado = static_cast<int>(
        static_cast<float>(estadoImagen.altoOriginal) * proporcionEscala);

    int coordenadaX =
        areaDibujo.left + (anchoArea - anchoEscalado) / 2;
    int coordenadaY =
        areaDibujo.top + (altoArea - altoEscalado) / 2;

    Gdiplus::Graphics graficosGdi(contextoDestino);
    graficosGdi.SetInterpolationMode(
        Gdiplus::InterpolationModeHighQualityBicubic);
    graficosGdi.DrawImage(estadoImagen.imagenCargada, coordenadaX,
                          coordenadaY, anchoEscalado, altoEscalado);
}

inline bool mostrarImagenAnterior() {
    if (estadoImagen.imagenesSeleccionadas.size() > 1) {
        estadoImagen.indiceImagenSeleccionada =
            (estadoImagen.indiceImagenSeleccionada == 0)
                ? (estadoImagen.imagenesSeleccionadas.size() - 1)
                : (estadoImagen.indiceImagenSeleccionada - 1);
        std::vector<std::wstring> seleccionPrevia = estadoImagen.imagenesSeleccionadas;
        size_t indicePrevio = estadoImagen.indiceImagenSeleccionada;
        bool ok = cargarImagenPorRuta(seleccionPrevia[indicePrevio]);
        estadoImagen.imagenesSeleccionadas = seleccionPrevia;
        estadoImagen.indiceImagenSeleccionada = indicePrevio;
        return ok;
    }
    if (estadoImagen.rutaImagenActual.empty()) return false;
    std::wstring rutaAnterior =
        obtenerImagenAnteriorEnCarpeta(estadoImagen.rutaImagenActual);
    if (rutaAnterior.empty()) return false;
    estadoImagen.imagenesSeleccionadas = { rutaAnterior };
    estadoImagen.indiceImagenSeleccionada = 0;
    return cargarImagenPorRuta(rutaAnterior);
}

inline bool mostrarImagenSiguiente() {
    if (estadoImagen.imagenesSeleccionadas.size() > 1) {
        estadoImagen.indiceImagenSeleccionada =
            (estadoImagen.indiceImagenSeleccionada + 1) %
            estadoImagen.imagenesSeleccionadas.size();
        std::vector<std::wstring> seleccionPrevia = estadoImagen.imagenesSeleccionadas;
        size_t indicePrevio = estadoImagen.indiceImagenSeleccionada;
        bool ok = cargarImagenPorRuta(seleccionPrevia[indicePrevio]);
        estadoImagen.imagenesSeleccionadas = seleccionPrevia;
        estadoImagen.indiceImagenSeleccionada = indicePrevio;
        return ok;
    }
    if (estadoImagen.rutaImagenActual.empty()) return false;
    std::wstring rutaSiguiente =
        obtenerImagenSiguienteEnCarpeta(estadoImagen.rutaImagenActual);
    if (rutaSiguiente.empty()) return false;
    estadoImagen.imagenesSeleccionadas = { rutaSiguiente };
    estadoImagen.indiceImagenSeleccionada = 0;
    return cargarImagenPorRuta(rutaSiguiente);
}

inline void dibujarBarraControlesImagen(HDC contextoDibujo, const RECT& areaCliente) {
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

    estadoImagen.botonAnterior = {
        areaCliente.left + 12,
        areaCliente.top + margenVertical,
        areaCliente.left + 12 + anchoBoton,
        areaCliente.top + margenVertical + altoBoton
    };

    estadoImagen.botonSiguiente = {
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

    dibujarBotonNav(estadoImagen.botonAnterior, L"◀ Anterior", estadoImagen.hoverBotonAnterior);
    dibujarBotonNav(estadoImagen.botonSiguiente, L"Siguiente ▶", estadoImagen.hoverBotonSiguiente);

    std::wstring textoCentro;
    if (estadoImagen.anchoOriginal > 0 && estadoImagen.altoOriginal > 0) {
        if (estadoImagen.imagenesSeleccionadas.size() > 1) {
            textoCentro = std::to_wstring(estadoImagen.anchoOriginal) + L" \u00D7 " +
                          std::to_wstring(estadoImagen.altoOriginal) + L" px  \u2022  " +
                          std::to_wstring(estadoImagen.indiceImagenSeleccionada + 1) + L"/" +
                          std::to_wstring(estadoImagen.imagenesSeleccionadas.size());
        } else {
            textoCentro = std::to_wstring(estadoImagen.anchoOriginal) + L" \u00D7 " +
                          std::to_wstring(estadoImagen.altoOriginal) + L" px";
        }
    }
    if (!textoCentro.empty()) {
        RECT areaCentro = {
            estadoImagen.botonAnterior.right + 8,
            areaCliente.top,
            estadoImagen.botonSiguiente.left - 8,
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

