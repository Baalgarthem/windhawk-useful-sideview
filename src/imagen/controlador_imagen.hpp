#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <wincodec.h>
#include <shlobj.h>
#include <algorithm>
#include <string>
#include <vector>
#include "nucleo/puntero_com.hpp"

enum class IdAccionImagen {
    Girar90Horario,
    Girar90Antihorario,
    CopiarAlPortapapeles,
    EstablecerFondoPantalla,
};

struct ElementoAccionImagen {
    IdAccionImagen id;
    std::wstring etiqueta;
    std::wstring glifo;
    RECT areaBoton{};
    bool hover{false};
};

struct ElementoMiniaturaGaleria {
    std::wstring rutaArchivo;
    std::wstring nombreArchivo;
    RECT areaTarjeta{};
    bool esActiva{false};
    bool hover{false};
};

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
    IStream* flujoStreamMemoria{nullptr};

    // Navegación
    RECT botonAnterior{};
    RECT botonSiguiente{};
    RECT botonGaleria{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};
    bool hoverBotonGaleria{false};

    // Modo Galería
    bool modoGaleria{false};
    int desplazamientoScrollGaleria{0};
    int altoTotalGaleria{0};
    std::vector<ElementoMiniaturaGaleria> miniaturasGaleria;

    // Acciones modulares con imágenes
    std::vector<ElementoAccionImagen> accionesDisponibles;
};

inline EstadoVisorImagen estadoImagen;

inline void inicializarAccionesImagen() {
    estadoImagen.accionesDisponibles.clear();
    estadoImagen.accionesDisponibles.push_back({
        IdAccionImagen::Girar90Horario,
        L"Girar 90°",
        L"🔄"
    });
    estadoImagen.accionesDisponibles.push_back({
        IdAccionImagen::CopiarAlPortapapeles,
        L"Copiar",
        L"📋"
    });
    estadoImagen.accionesDisponibles.push_back({
        IdAccionImagen::EstablecerFondoPantalla,
        L"Fondo",
        L"🖼️"
    });
}

inline void liberarRecursosImagen() {
    if (ventanaVideo) {
        KillTimer(ventanaVideo, ID_TEMPORIZADOR_ANIMACION_GIF);
    }
    if (estadoImagen.imagenCargada) {
        delete estadoImagen.imagenCargada;
        estadoImagen.imagenCargada = nullptr;
    }
    if (estadoImagen.flujoStreamMemoria) {
        estadoImagen.flujoStreamMemoria->Release();
        estadoImagen.flujoStreamMemoria = nullptr;
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
    estadoImagen.hoverBotonGaleria = false;
    estadoImagen.modoGaleria = false;
    estadoImagen.desplazamientoScrollGaleria = 0;
    estadoImagen.altoTotalGaleria = 0;
    estadoImagen.miniaturasGaleria.clear();
    estadoImagen.accionesDisponibles.clear();
}

inline IStream* crearFlujoEnMemoriaDesdeArchivo(const std::wstring& rutaArchivo) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return nullptr;

    LARGE_INTEGER tamanoArchivo{};
    if (!GetFileSizeEx(manejadorArchivo, &tamanoArchivo) ||
        tamanoArchivo.QuadPart == 0 ||
        tamanoArchivo.QuadPart > 128 * 1024 * 1024) {
        CloseHandle(manejadorArchivo);
        return nullptr;
    }

    HGLOBAL memoriaGlobal = GlobalAlloc(GMEM_MOVEABLE, static_cast<SIZE_T>(tamanoArchivo.QuadPart));
    if (!memoriaGlobal) {
        CloseHandle(manejadorArchivo);
        return nullptr;
    }

    void* punteroMemoria = GlobalLock(memoriaGlobal);
    if (!punteroMemoria) {
        GlobalFree(memoriaGlobal);
        CloseHandle(manejadorArchivo);
        return nullptr;
    }

    DWORD bytesLeidos = 0;
    if (!ReadFile(manejadorArchivo, punteroMemoria, static_cast<DWORD>(tamanoArchivo.QuadPart), &bytesLeidos, nullptr) ||
        bytesLeidos != static_cast<DWORD>(tamanoArchivo.QuadPart)) {
        GlobalUnlock(memoriaGlobal);
        GlobalFree(memoriaGlobal);
        CloseHandle(manejadorArchivo);
        return nullptr;
    }
    GlobalUnlock(memoriaGlobal);
    CloseHandle(manejadorArchivo);

    IStream* flujoStream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(memoriaGlobal, TRUE, &flujoStream))) {
        GlobalFree(memoriaGlobal);
        return nullptr;
    }
    return flujoStream;
}

inline Gdiplus::Bitmap* decodificarImagenConWic(
    const std::wstring& rutaArchivo) {
    IStream* flujoStream = crearFlujoEnMemoriaDesdeArchivo(rutaArchivo);
    if (!flujoStream) return nullptr;

    PunteroCom<IWICImagingFactory> fabricaWic;
    HRESULT resultadoFabrica = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(fabricaWic.recibir()));
    if (FAILED(resultadoFabrica) || !fabricaWic.obtener()) {
        resultadoFabrica = CoCreateInstance(
            CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(fabricaWic.recibir()));
        if (FAILED(resultadoFabrica) || !fabricaWic.obtener()) {
            flujoStream->Release();
            return nullptr;
        }
    }

    PunteroCom<IWICBitmapDecoder> decodificador;
    HRESULT resultadoDecodificador = fabricaWic->CreateDecoderFromStream(
        flujoStream, nullptr, WICDecodeMetadataCacheOnDemand,
        decodificador.recibir());
    flujoStream->Release();
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
        cuadroDecodificado.obtener(), GUID_WICPixelFormat32bppBGRA,
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

    Gdiplus::Bitmap* mapaBitsDestino = new Gdiplus::Bitmap(
        static_cast<INT>(anchoImagen), static_cast<INT>(altoImagen),
        PixelFormat32bppARGB);
    if (!mapaBitsDestino || mapaBitsDestino->GetLastStatus() != Gdiplus::Ok) {
        if (mapaBitsDestino) delete mapaBitsDestino;
        return nullptr;
    }

    Gdiplus::BitmapData datosMapaBits{};
    Gdiplus::Rect rectanguloArea(0, 0, static_cast<INT>(anchoImagen), static_cast<INT>(altoImagen));
    if (mapaBitsDestino->LockBits(&rectanguloArea, Gdiplus::ImageLockModeWrite,
                                  PixelFormat32bppARGB, &datosMapaBits) == Gdiplus::Ok) {
        BYTE* punteroDestino = static_cast<BYTE*>(datosMapaBits.Scan0);
        const BYTE* punteroOrigen = buferPixeles.data();
        for (UINT fila = 0; fila < altoImagen; ++fila) {
            memcpy(punteroDestino + fila * static_cast<UINT>(datosMapaBits.Stride),
                   punteroOrigen + fila * pasoBytesFila,
                   pasoBytesFila);
        }
        mapaBitsDestino->UnlockBits(&datosMapaBits);
    }

    return mapaBitsDestino;
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

inline void actualizarListaGaleria() {
    estadoImagen.miniaturasGaleria.clear();
    std::vector<std::wstring> listaRutas;
    if (estadoImagen.imagenesSeleccionadas.size() > 1) {
        listaRutas = estadoImagen.imagenesSeleccionadas;
    } else if (!estadoImagen.rutaImagenActual.empty()) {
        listaRutas = obtenerImagenesEnMismaCarpeta(estadoImagen.rutaImagenActual);
    }
    for (size_t posicionElemento = 0; posicionElemento < listaRutas.size(); ++posicionElemento) {
        ElementoMiniaturaGaleria elemento;
        elemento.rutaArchivo = listaRutas[posicionElemento];
        elemento.nombreArchivo = PathFindFileNameW(listaRutas[posicionElemento].c_str());
        elemento.esActiva = (lstrcmpiW(listaRutas[posicionElemento].c_str(), estadoImagen.rutaImagenActual.c_str()) == 0);
        elemento.hover = false;
        estadoImagen.miniaturasGaleria.push_back(elemento);
    }
}

inline bool cargarImagenPorRuta(const std::wstring& rutaNuevaImagen) {
    if (rutaNuevaImagen.empty()) {
        liberarRecursosImagen();
        return false;
    }

    std::vector<std::wstring> seleccionPrevia = estadoImagen.imagenesSeleccionadas;
    size_t indicePrevio = estadoImagen.indiceImagenSeleccionada;
    bool modoGaleriaPrevio = estadoImagen.modoGaleria;

    liberarRecursosImagen();

    estadoImagen.imagenesSeleccionadas = seleccionPrevia;
    estadoImagen.indiceImagenSeleccionada = indicePrevio;
    estadoImagen.modoGaleria = modoGaleriaPrevio;

    // 1. Decodificación WIC en memoria (rápida, segura y 0 descriptores de archivo abiertos)
    estadoImagen.imagenCargada = decodificarImagenConWic(rutaNuevaImagen);

    // 2. Si WIC no lo decodificó (ej. GIF animado o formato especial), decodificar vía stream en memoria
    if (!estadoImagen.imagenCargada || estadoImagen.imagenCargada->GetLastStatus() != Gdiplus::Ok) {
        if (estadoImagen.imagenCargada) {
            delete estadoImagen.imagenCargada;
            estadoImagen.imagenCargada = nullptr;
        }
        estadoImagen.flujoStreamMemoria = crearFlujoEnMemoriaDesdeArchivo(rutaNuevaImagen);
        if (estadoImagen.flujoStreamMemoria) {
            LARGE_INTEGER liInicio{};
            estadoImagen.flujoStreamMemoria->Seek(liInicio, STREAM_SEEK_SET, nullptr);
            estadoImagen.imagenCargada = Gdiplus::Bitmap::FromStream(estadoImagen.flujoStreamMemoria);
            if (!estadoImagen.imagenCargada || estadoImagen.imagenCargada->GetLastStatus() != Gdiplus::Ok) {
                if (estadoImagen.imagenCargada) {
                    delete estadoImagen.imagenCargada;
                    estadoImagen.imagenCargada = nullptr;
                }
                estadoImagen.flujoStreamMemoria->Release();
                estadoImagen.flujoStreamMemoria = nullptr;
            }
        }
    }

    // 3. Fallback de emergencia
    if (!estadoImagen.imagenCargada ||
        estadoImagen.imagenCargada->GetLastStatus() != Gdiplus::Ok) {
        if (estadoImagen.imagenCargada) {
            delete estadoImagen.imagenCargada;
            estadoImagen.imagenCargada = nullptr;
        }
        estadoImagen.imagenCargada = Gdiplus::Image::FromFile(rutaNuevaImagen.c_str());
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
    inicializarAccionesImagen();

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

    actualizarListaGaleria();

    return true;
}

inline bool girarImagenActual(bool sentidoHorario = true) {
    if (!estadoImagen.imagenCargada) return false;
    Gdiplus::RotateFlipType tipoRotacion = sentidoHorario
        ? Gdiplus::Rotate90FlipNone
        : Gdiplus::Rotate270FlipNone;
    if (estadoImagen.imagenCargada->RotateFlip(tipoRotacion) == Gdiplus::Ok) {
        std::swap(estadoImagen.anchoOriginal, estadoImagen.altoOriginal);
        return true;
    }
    return false;
}

inline bool copiarImagenActualAlPortapapeles(HWND ventana) {
    if (estadoImagen.rutaImagenActual.empty()) return false;
    if (!OpenClipboard(ventana)) return false;
    EmptyClipboard();

    std::wstring ruta = estadoImagen.rutaImagenActual;
    size_t bytes = sizeof(DROPFILES) + (ruta.size() + 2) * sizeof(wchar_t);
    HGLOBAL hGlobal = GlobalAlloc(GHND, bytes);
    if (hGlobal) {
        DROPFILES* estructuraDropFiles = reinterpret_cast<DROPFILES*>(GlobalLock(hGlobal));
        if (estructuraDropFiles) {
            estructuraDropFiles->pFiles = sizeof(DROPFILES);
            estructuraDropFiles->fWide = TRUE;
            wchar_t* buffer = reinterpret_cast<wchar_t*>(reinterpret_cast<BYTE*>(estructuraDropFiles) + sizeof(DROPFILES));
            wcscpy_s(buffer, ruta.size() + 1, ruta.c_str());
            buffer[ruta.size() + 1] = L'\0';
            GlobalUnlock(hGlobal);
            SetClipboardData(CF_HDROP, hGlobal);
        }
    }
    CloseClipboard();
    return true;
}

inline bool establecerComoFondoDePantalla() {
    if (estadoImagen.rutaImagenActual.empty()) return false;
    return SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0,
        const_cast<wchar_t*>(estadoImagen.rutaImagenActual.c_str()),
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) != FALSE;
}

inline bool ejecutarAccionImagen(IdAccionImagen id, HWND ventanaPadre) {
    switch (id) {
        case IdAccionImagen::Girar90Horario:
            return girarImagenActual(true);
        case IdAccionImagen::Girar90Antihorario:
            return girarImagenActual(false);
        case IdAccionImagen::CopiarAlPortapapeles:
            return copiarImagenActualAlPortapapeles(ventanaPadre);
        case IdAccionImagen::EstablecerFondoPantalla:
            return establecerComoFondoDePantalla();
        default:
            return false;
    }
}

inline void dibujarLienzoGaleriaImagenes(HDC contextoDestino, const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTitulo = CreateFontW(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSub = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    RECT areaCabecera{areaDibujo.left + 12, areaDibujo.top + 8, areaDibujo.right - 12, areaDibujo.top + 28};
    SetBkMode(contextoDestino, TRANSPARENT);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDestino, fuenteTitulo));
    std::wstring tituloGaleria = L"Galería de imágenes (" +
        std::to_wstring(estadoImagen.miniaturasGaleria.size()) + L")";
    DrawTextW(contextoDestino, tituloGaleria.c_str(), -1, &areaCabecera, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(contextoDestino, fuenteSub);

    int margenLateral = 12;
    int espacioEntreTarjetas = 8;
    int anchoDisponible = std::max(100, anchoArea - margenLateral * 2);
    size_t cantidadColumnas = static_cast<size_t>(std::max(2, std::min(4, anchoDisponible / 110)));
    int anchoTarjeta = (anchoDisponible - (static_cast<int>(cantidadColumnas) - 1) * espacioEntreTarjetas) / static_cast<int>(cantidadColumnas);
    int altoTarjeta = 95;

    int yInicio = areaDibujo.top + 34 - estadoImagen.desplazamientoScrollGaleria;

    for (size_t posicionMiniatura = 0; posicionMiniatura < estadoImagen.miniaturasGaleria.size(); ++posicionMiniatura) {
        int columnaActual = static_cast<int>(posicionMiniatura % cantidadColumnas);
        int filaActual = static_cast<int>(posicionMiniatura / cantidadColumnas);
        int posicionTarjetaX = areaDibujo.left + margenLateral + columnaActual * (anchoTarjeta + espacioEntreTarjetas);
        int posicionTarjetaY = yInicio + filaActual * (altoTarjeta + espacioEntreTarjetas);

        RECT areaTarjeta{posicionTarjetaX, posicionTarjetaY, posicionTarjetaX + anchoTarjeta, posicionTarjetaY + altoTarjeta};
        estadoImagen.miniaturasGaleria[posicionMiniatura].areaTarjeta = areaTarjeta;

        if (areaTarjeta.bottom < areaDibujo.top || areaTarjeta.top > areaDibujo.bottom) {
            continue;
        }

        bool esActiva = estadoImagen.miniaturasGaleria[posicionMiniatura].esActiva;
        bool hover = estadoImagen.miniaturasGaleria[posicionMiniatura].hover;

        COLORREF colorFondo = esActiva ? RGB(30, 41, 59) : (hover ? RGB(26, 32, 44) : RGB(22, 27, 34));
        COLORREF colorBorde = esActiva ? RGB(0, 120, 215) : (hover ? RGB(88, 166, 255) : RGB(48, 54, 61));

        HBRUSH brochaCard = CreateSolidBrush(colorFondo);
        HPEN lapizCard = CreatePen(PS_SOLID, esActiva ? 2 : 1, colorBorde);
        HBRUSH brochaPreviaCard = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaCard));
        HPEN lapizPrevioCard = static_cast<HPEN>(SelectObject(contextoDestino, lapizCard));
        RoundRect(contextoDestino, areaTarjeta.left, areaTarjeta.top, areaTarjeta.right, areaTarjeta.bottom, 6, 6);
        SelectObject(contextoDestino, lapizPrevioCard);
        SelectObject(contextoDestino, brochaPreviaCard);
        DeleteObject(lapizCard);
        DeleteObject(brochaCard);

        RECT areaIcono{areaTarjeta.left + 4, areaTarjeta.top + 6, areaTarjeta.right - 4, areaTarjeta.top + 48};
        SelectObject(contextoDestino, fuenteTitulo);
        SetTextColor(contextoDestino, esActiva ? RGB(88, 166, 255) : RGB(139, 148, 158));
        DrawTextW(contextoDestino, L"🖼️", -1, &areaIcono, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        RECT areaNombre{areaTarjeta.left + 6, areaTarjeta.top + 52, areaTarjeta.right - 6, areaTarjeta.bottom - 4};
        SelectObject(contextoDestino, fuenteSub);
        SetTextColor(contextoDestino, esActiva ? RGB(255, 255, 255) : (hover ? RGB(240, 246, 252) : RGB(201, 209, 217)));
        DrawTextW(contextoDestino, estadoImagen.miniaturasGaleria[posicionMiniatura].nombreArchivo.c_str(), -1,
                  &areaNombre, DT_CENTER | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    size_t totalFilas = (estadoImagen.miniaturasGaleria.size() + cantidadColumnas - 1) / cantidadColumnas;
    estadoImagen.altoTotalGaleria = static_cast<int>(totalFilas) * (altoTarjeta + espacioEntreTarjetas) + 40;

    SelectObject(contextoDestino, fuentePrev);
    DeleteObject(fuenteSub);
    DeleteObject(fuenteTitulo);
}

inline void dibujarLienzoImagen(HDC contextoDestino, const RECT& areaDibujo) {
    if (estadoImagen.modoGaleria) {
        dibujarLienzoGaleriaImagenes(contextoDestino, areaDibujo);
        return;
    }

    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(15, 15, 15));
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

    // Fila 1: Botones de navegación (◀ Anterior, 🖼️ Galería, Siguiente ▶)
    int anchoBotonNav = 80;
    int altoBotonNav = 24;
    int yFilaNav = areaCliente.top + 6;

    estadoImagen.botonAnterior = {
        areaCliente.left + 12,
        yFilaNav,
        areaCliente.left + 12 + anchoBotonNav,
        yFilaNav + altoBotonNav
    };

    estadoImagen.botonSiguiente = {
        areaCliente.right - 12 - anchoBotonNav,
        yFilaNav,
        areaCliente.right - 12,
        yFilaNav + altoBotonNav
    };

    int anchoBotonGaleria = 90;
    int xCentro = (areaCliente.left + areaCliente.right) / 2;
    estadoImagen.botonGaleria = {
        xCentro - anchoBotonGaleria / 2,
        yFilaNav,
        xCentro + anchoBotonGaleria / 2,
        yFilaNav + altoBotonNav
    };

    auto dibujarBoton = [&](const RECT& rect, const wchar_t* texto, bool hover, bool activo = false) {
        COLORREF colorFondo = activo ? RGB(0, 120, 215) : (hover ? RGB(48, 54, 61) : RGB(33, 38, 45));
        COLORREF colorBorde = activo ? RGB(0, 140, 240) : (hover ? RGB(110, 118, 129) : RGB(48, 54, 61));
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
        SetTextColor(contextoDibujo, (activo || hover) ? RGB(255, 255, 255) : RGB(201, 209, 217));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        RECT rTexto = rect;
        DrawTextW(contextoDibujo, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    };

    dibujarBoton(estadoImagen.botonAnterior, L"◀ Anterior", estadoImagen.hoverBotonAnterior);
    dibujarBoton(estadoImagen.botonGaleria, estadoImagen.modoGaleria ? L"🖼️ Cerrar" : L"🖼️ Galería",
                 estadoImagen.hoverBotonGaleria, estadoImagen.modoGaleria);
    dibujarBoton(estadoImagen.botonSiguiente, L"Siguiente ▶", estadoImagen.hoverBotonSiguiente);

    // Fila 2: Separador y Sección "ACCIONES CON IMÁGENES"
    int yFilaAcciones = yFilaNav + altoBotonNav + 6;
    if (estadoImagen.accionesDisponibles.empty()) {
        inicializarAccionesImagen();
    }

    HPEN lapizSep = CreatePen(PS_SOLID, 1, RGB(38, 44, 52));
    HPEN lapizPrevioSep = static_cast<HPEN>(SelectObject(contextoDibujo, lapizSep));
    MoveToEx(contextoDibujo, areaCliente.left + 12, yFilaAcciones, nullptr);
    LineTo(contextoDibujo, areaCliente.right - 12, yFilaAcciones);
    SelectObject(contextoDibujo, lapizPrevioSep);
    DeleteObject(lapizSep);

    yFilaAcciones += 4;
    int anchoBotonAccion = 96;
    int altoBotonAccion = 22;
    int espacioAcciones = 8;
    int totalAnchoAcciones = static_cast<int>(estadoImagen.accionesDisponibles.size()) * (anchoBotonAccion + espacioAcciones) - espacioAcciones;
    int xInicioAcciones = std::max<int>(areaCliente.left + 12, (areaCliente.left + areaCliente.right - totalAnchoAcciones) / 2);

    for (size_t posicionAccion = 0; posicionAccion < estadoImagen.accionesDisponibles.size(); ++posicionAccion) {
        RECT areaBotonAccion = {
            xInicioAcciones + static_cast<int>(posicionAccion) * (anchoBotonAccion + espacioAcciones),
            yFilaAcciones,
            xInicioAcciones + static_cast<int>(posicionAccion) * (anchoBotonAccion + espacioAcciones) + anchoBotonAccion,
            yFilaAcciones + altoBotonAccion
        };
        estadoImagen.accionesDisponibles[posicionAccion].areaBoton = areaBotonAccion;
        std::wstring etiquetaCompleta = estadoImagen.accionesDisponibles[posicionAccion].glifo + L" " + estadoImagen.accionesDisponibles[posicionAccion].etiqueta;
        dibujarBoton(areaBotonAccion, etiquetaCompleta.c_str(), estadoImagen.accionesDisponibles[posicionAccion].hover);
    }
}


