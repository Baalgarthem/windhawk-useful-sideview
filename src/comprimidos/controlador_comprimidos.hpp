#pragma once

#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06020000
#endif

#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <compressapi.h>
#include <commdlg.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

enum class CategoriaElementoComprimido {
    Carpetas,
    Videos,
    Imagenes,
    Audios,
    Documentos,
    Codigo,
    Texto,
    Comprimidos,
    Otros,
    TotalCategorias
};

struct ElementoComprimido {
    std::wstring rutaArchivo;
    std::wstring nombreArchivo;
    std::wstring extensionFormato;
    std::wstring textoTamano;
    Gdiplus::Image* portadaCargada{nullptr};
    bool esComic{false};
};

struct ElementoArbolComprimido {
    std::wstring nombre;
    std::wstring rutaCompleta;
    bool esDirectorio{false};
    int nivelJerarquia{0};
    uintmax_t tamanoBytes{0};
};

struct ElementoContenidoComprimido {
    std::wstring nombre;
    std::wstring rutaRelativa;
    CategoriaElementoComprimido categoria{CategoriaElementoComprimido::Otros};
    bool esDirectorio{false};
    uintmax_t tamanoBytes{0};
    RECT areaElemento{};
};

struct SeccionContenidoComprimido {
    CategoriaElementoComprimido categoria{CategoriaElementoComprimido::Otros};
    std::wstring tituloSeccion;
    std::wstring iconoSeccion;
    std::vector<ElementoContenidoComprimido> elementos;
    RECT areaCabecera{};
};

struct EstadoControladorComprimidos {
    std::vector<std::wstring> archivosSeleccionados;
    size_t indiceArchivoActual{0};
    size_t totalSeleccionados{0};
    bool tieneComicsSeleccionados{false};
    bool esModoComic{false};

    // Modo Cómic: portadas
    std::vector<ElementoComprimido> elementosSeleccionados;

    // Modo Comprimidos Comunes: Secciones estructuradas por categoría
    std::vector<ElementoArbolComprimido> elementosArbol;
    std::vector<ElementoContenidoComprimido> todosLosElementos;
    std::vector<SeccionContenidoComprimido> seccionesContenido;
    std::wstring rutaComprimidoArbolCargado;
    FILETIME ftUltimaModificacionArbol{};
    int desplazamientoScrollY{0};
    int altoTotalArbol{0};
    RECT areaCajaArbol{};

    // Navegación entre comprimidos múltiples
    RECT botonAnteriorComprimido{};
    RECT botonSiguienteComprimido{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};

    // Programas y Conversión
    std::vector<std::wstring> programasComicsRecordados;
    std::vector<std::wstring> programasComunesRecordados;
    int indiceProgramaSeleccionado{0};
    bool menuProgramasAbierto{false};
    int opcionProgramaHover{-1};
    std::wstring programaTemporal;
    int indiceFormatoDestino{0};
    bool menuDesplegableAbierto{false};
    int opcionFormatoHover{-1};
    bool conversionEnCurso{false};

    // Protección con contraseña para comprimidos
    bool protegidoConContrasena{false};
    bool contrasenaDesbloqueada{false};
    RECT areaBotonDesbloquear{};
    bool hoverBotonDesbloquear{false};
};

inline EstadoControladorComprimidos estadoComprimidos;

inline std::vector<std::wstring>& obtenerListaProgramasActiva() {
    return estadoComprimidos.tieneComicsSeleccionados
               ? estadoComprimidos.programasComicsRecordados
               : estadoComprimidos.programasComunesRecordados;
}

inline const std::vector<std::wstring>& obtenerListaProgramasActivaConst() {
    return estadoComprimidos.tieneComicsSeleccionados
               ? estadoComprimidos.programasComicsRecordados
               : estadoComprimidos.programasComunesRecordados;
}

namespace {

inline std::wstring formatearTamanoArchivo(uintmax_t tamanoEnBytes) {
    if (tamanoEnBytes < 1024) {
        return std::to_wstring(tamanoEnBytes) + L" B";
    }
    if (tamanoEnBytes < 1024 * 1024) {
        wchar_t buferTamano[32];
        swprintf_s(buferTamano, L"%.1f KB",
                   static_cast<double>(tamanoEnBytes) / 1024.0);
        return buferTamano;
    }
    wchar_t buferTamano[32];
    swprintf_s(buferTamano, L"%.1f MB",
               static_cast<double>(tamanoEnBytes) / (1024.0 * 1024.0));
    return buferTamano;
}

#pragma pack(push, 1)
struct CabeceraArchivoZipLocal {
    uint32_t firma;
    uint16_t versionMinima;
    uint16_t banderaGeneral;
    uint16_t metodoCompresion;
    uint16_t horaModificacion;
    uint16_t fechaModificacion;
    uint32_t sumaCrc32;
    uint32_t tamanoComprimido;
    uint32_t tamanoDescomprimido;
    uint16_t longitudNombreArchivo;
    uint16_t longitudCampoExtra;
};
#pragma pack(pop)

inline Gdiplus::Image* extraerPortadaDesdeShell(const std::wstring& rutaArchivo) {
    Gdiplus::Image* imagenExtraida = nullptr;
    IShellItem* elementoShell = nullptr;
    if (SUCCEEDED(SHCreateItemFromParsingName(rutaArchivo.c_str(), nullptr,
                                             IID_PPV_ARGS(&elementoShell)))) {
        IShellItemImageFactory* fabricaImagenes = nullptr;
        if (SUCCEEDED(elementoShell->QueryInterface(
                IID_PPV_ARGS(&fabricaImagenes)))) {
            HBITMAP mapaBitsHBitmap = nullptr;
            SIZE tamanoDeseado{320, 480};
            HRESULT resultadoImagen = fabricaImagenes->GetImage(
                tamanoDeseado, SIIGBF_BIGGERSIZEOK, &mapaBitsHBitmap);
            if (SUCCEEDED(resultadoImagen) && mapaBitsHBitmap) {
                Gdiplus::Bitmap* mapaBitsTemp =
                    Gdiplus::Bitmap::FromHBITMAP(mapaBitsHBitmap, nullptr);
                if (mapaBitsTemp &&
                    mapaBitsTemp->GetLastStatus() == Gdiplus::Ok) {
                    imagenExtraida = mapaBitsTemp;
                } else if (mapaBitsTemp) {
                    delete mapaBitsTemp;
                }
                DeleteObject(mapaBitsHBitmap);
            }
            fabricaImagenes->Release();
        }
        elementoShell->Release();
    }
    return imagenExtraida;
}

inline Gdiplus::Image* extraerPrimeraImagenDeZip(const std::wstring& rutaArchivo) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return nullptr;

    CabeceraArchivoZipLocal cabeceraLocal{};
    DWORD bytesLeidos = 0;
    while (ReadFile(manejadorArchivo, &cabeceraLocal, sizeof(cabeceraLocal),
                    &bytesLeidos, nullptr) &&
           bytesLeidos == sizeof(cabeceraLocal)) {
        if (cabeceraLocal.firma != 0x04034b50) break;

        if (cabeceraLocal.longitudNombreArchivo > 0 &&
            cabeceraLocal.longitudNombreArchivo < 1024) {
            std::vector<char> nombreEntrada(
                cabeceraLocal.longitudNombreArchivo + 1, 0);
            if (!ReadFile(manejadorArchivo, nombreEntrada.data(),
                          cabeceraLocal.longitudNombreArchivo, &bytesLeidos,
                          nullptr) ||
                bytesLeidos != cabeceraLocal.longitudNombreArchivo) {
                break;
            }

            if (cabeceraLocal.longitudCampoExtra > 0) {
                SetFilePointer(manejadorArchivo,
                               cabeceraLocal.longitudCampoExtra, nullptr,
                               FILE_CURRENT);
            }

            std::string nombreArchivoEntrada(nombreEntrada.data());
            std::string extensionEntrada;
            size_t posicionPunto = nombreArchivoEntrada.find_last_of('.');
            if (posicionPunto != std::string::npos) {
                extensionEntrada =
                    nombreArchivoEntrada.substr(posicionPunto);
            }
            for (char& caracter : extensionEntrada) {
                caracter = static_cast<char>(tolower(caracter));
            }

            bool esImagen = (extensionEntrada == ".jpg" ||
                             extensionEntrada == ".jpeg" ||
                             extensionEntrada == ".png" ||
                             extensionEntrada == ".webp" ||
                             extensionEntrada == ".bmp");

            if (esImagen && cabeceraLocal.tamanoDescomprimido > 0 &&
                cabeceraLocal.tamanoDescomprimido < 32 * 1024 * 1024) {
                if (cabeceraLocal.metodoCompresion == 0) {
                    HGLOBAL memoriaGlobal = GlobalAlloc(
                        GMEM_MOVEABLE, cabeceraLocal.tamanoDescomprimido);
                    if (memoriaGlobal) {
                        void* direccionMemoria = GlobalLock(memoriaGlobal);
                        if (direccionMemoria &&
                            ReadFile(manejadorArchivo, direccionMemoria,
                                     cabeceraLocal.tamanoDescomprimido,
                                     &bytesLeidos, nullptr) &&
                            bytesLeidos == cabeceraLocal.tamanoDescomprimido) {
                            GlobalUnlock(memoriaGlobal);
                            IStream* flujoStream = nullptr;
                            if (SUCCEEDED(CreateStreamOnHGlobal(
                                    memoriaGlobal, TRUE, &flujoStream))) {
                                Gdiplus::Bitmap* mapaBitsTemp =
                                    Gdiplus::Bitmap::FromStream(flujoStream);
                                Gdiplus::Image* imagenPortada = nullptr;
                                if (mapaBitsTemp &&
                                    mapaBitsTemp->GetLastStatus() ==
                                        Gdiplus::Ok) {
                                    imagenPortada = mapaBitsTemp->Clone(
                                        0, 0,
                                        static_cast<INT>(
                                            mapaBitsTemp->GetWidth()),
                                        static_cast<INT>(
                                            mapaBitsTemp->GetHeight()),
                                        PixelFormat32bppARGB);
                                }
                                if (mapaBitsTemp) delete mapaBitsTemp;
                                flujoStream->Release();
                                if (imagenPortada &&
                                    imagenPortada->GetLastStatus() ==
                                        Gdiplus::Ok) {
                                    CloseHandle(manejadorArchivo);
                                    return imagenPortada;
                                }
                                if (imagenPortada) delete imagenPortada;
                            }
                        } else {
                            GlobalUnlock(memoriaGlobal);
                            GlobalFree(memoriaGlobal);
                        }
                    }
                } else if (cabeceraLocal.metodoCompresion == 8 &&
                           cabeceraLocal.tamanoComprimido > 0) {
                    std::vector<BYTE> buferComprimido(
                        cabeceraLocal.tamanoComprimido);
                    if (ReadFile(manejadorArchivo, buferComprimido.data(),
                                 cabeceraLocal.tamanoComprimido, &bytesLeidos,
                                 nullptr) &&
                        bytesLeidos == cabeceraLocal.tamanoComprimido) {
                        constexpr DWORD ALGORITMO_DEFLATE_RAW =
                            COMPRESS_ALGORITHM_MSZIP | COMPRESS_RAW;
                        DECOMPRESSOR_HANDLE descompresor = nullptr;
                        if (CreateDecompressor(ALGORITMO_DEFLATE_RAW,
                                               nullptr, &descompresor)) {
                            HGLOBAL memoriaGlobal = GlobalAlloc(
                                GMEM_MOVEABLE,
                                cabeceraLocal.tamanoDescomprimido);
                            if (memoriaGlobal) {
                                void* direccionMemoria =
                                    GlobalLock(memoriaGlobal);
                                SIZE_T tamanoDescomprimidoReal = 0;
                                if (direccionMemoria &&
                                    Decompress(
                                        descompresor, buferComprimido.data(),
                                        cabeceraLocal.tamanoComprimido,
                                        direccionMemoria,
                                        cabeceraLocal.tamanoDescomprimido,
                                        &tamanoDescomprimidoReal)) {
                                    GlobalUnlock(memoriaGlobal);
                                    IStream* flujoStream = nullptr;
                                    if (SUCCEEDED(CreateStreamOnHGlobal(
                                            memoriaGlobal, TRUE,
                                            &flujoStream))) {
                                        Gdiplus::Bitmap* mapaBitsTemp =
                                            Gdiplus::Bitmap::FromStream(
                                                flujoStream);
                                        Gdiplus::Image* imagenPortada =
                                            nullptr;
                                        if (mapaBitsTemp &&
                                            mapaBitsTemp->GetLastStatus() ==
                                                Gdiplus::Ok) {
                                            imagenPortada =
                                                mapaBitsTemp->Clone(
                                                    0, 0,
                                                    static_cast<INT>(
                                                        mapaBitsTemp
                                                            ->GetWidth()),
                                                    static_cast<INT>(
                                                        mapaBitsTemp
                                                            ->GetHeight()),
                                                    PixelFormat32bppARGB);
                                        }
                                        if (mapaBitsTemp) delete mapaBitsTemp;
                                        flujoStream->Release();
                                        if (imagenPortada &&
                                            imagenPortada->GetLastStatus() ==
                                                Gdiplus::Ok) {
                                            CloseDecompressor(descompresor);
                                            CloseHandle(manejadorArchivo);
                                            return imagenPortada;
                                        }
                                        if (imagenPortada) delete imagenPortada;
                                    }
                                } else {
                                    GlobalUnlock(memoriaGlobal);
                                    GlobalFree(memoriaGlobal);
                                }
                            }
                            CloseDecompressor(descompresor);
                        }
                    }
                }
            } else if (cabeceraLocal.tamanoComprimido > 0) {
                SetFilePointer(
                    manejadorArchivo,
                    static_cast<LONG>(cabeceraLocal.tamanoComprimido), nullptr,
                    FILE_CURRENT);
            }
        } else {
            break;
        }
    }

    CloseHandle(manejadorArchivo);
    return nullptr;
}

inline Gdiplus::Image* obtenerPortadaDeComprimido(const std::wstring& rutaActual) {
    Gdiplus::Image* imagenPortada = extraerPortadaDesdeShell(rutaActual);
    if (imagenPortada) return imagenPortada;

    const wchar_t* extension = PathFindExtensionW(rutaActual.c_str());
    if (lstrcmpiW(extension, L".zip") == 0 ||
        lstrcmpiW(extension, L".cbz") == 0) {
        imagenPortada = extraerPrimeraImagenDeZip(rutaActual);
        if (imagenPortada) return imagenPortada;
    }
    return nullptr;
}

}  // namespace

inline void liberarRecursosComprimido() {
    for (ElementoComprimido& elemento :
         estadoComprimidos.elementosSeleccionados) {
        if (elemento.portadaCargada) {
            delete elemento.portadaCargada;
            elemento.portadaCargada = nullptr;
        }
    }
    estadoComprimidos.elementosSeleccionados.clear();
    estadoComprimidos.archivosSeleccionados.clear();
    estadoComprimidos.elementosArbol.clear();
    estadoComprimidos.todosLosElementos.clear();
    estadoComprimidos.seccionesContenido.clear();
    estadoComprimidos.rutaComprimidoArbolCargado.clear();
    estadoComprimidos.ftUltimaModificacionArbol = {};
    estadoComprimidos.desplazamientoScrollY = 0;
    estadoComprimidos.altoTotalArbol = 0;
    estadoComprimidos.totalSeleccionados = 0;
    estadoComprimidos.indiceArchivoActual = 0;
    estadoComprimidos.tieneComicsSeleccionados = false;
    estadoComprimidos.esModoComic = false;
    estadoComprimidos.menuDesplegableAbierto = false;
    estadoComprimidos.opcionFormatoHover = -1;
    estadoComprimidos.menuProgramasAbierto = false;
    estadoComprimidos.opcionProgramaHover = -1;
    estadoComprimidos.hoverBotonAnterior = false;
    estadoComprimidos.hoverBotonSiguiente = false;
    estadoComprimidos.protegidoConContrasena = false;
    estadoComprimidos.contrasenaDesbloqueada = false;
    estadoComprimidos.areaBotonDesbloquear = {};
    estadoComprimidos.hoverBotonDesbloquear = false;
}

inline CategoriaElementoComprimido clasificarCategoriaElementoComprimido(const std::wstring& ruta, bool esDirectorio) {
    if (esDirectorio) {
        return CategoriaElementoComprimido::Carpetas;
    }
    const wchar_t* extensionPuntero = PathFindExtensionW(ruta.c_str());
    if (!extensionPuntero || !*extensionPuntero) {
        return CategoriaElementoComprimido::Otros;
    }
    std::wstring extension = extensionPuntero;
    for (wchar_t& caracter : extension) {
        caracter = static_cast<wchar_t>(towlower(caracter));
    }

    if (extension == L".mp4" || extension == L".mkv" || extension == L".avi" ||
        extension == L".mov" || extension == L".wmv" || extension == L".flv" ||
        extension == L".webm" || extension == L".m4v" || extension == L".ts" ||
        extension == L".3gp" || extension == L".mpeg" || extension == L".mpg") {
        return CategoriaElementoComprimido::Videos;
    }

    if (extension == L".jpg" || extension == L".jpeg" || extension == L".png" ||
        extension == L".webp" || extension == L".bmp" || extension == L".gif" ||
        extension == L".ico" || extension == L".svg" || extension == L".tiff" ||
        extension == L".tif" || extension == L".avif" || extension == L".heic" ||
        extension == L".raw" || extension == L".psd") {
        return CategoriaElementoComprimido::Imagenes;
    }

    if (extension == L".mp3" || extension == L".wav" || extension == L".flac" ||
        extension == L".aac" || extension == L".m4a" || extension == L".ogg" ||
        extension == L".opus" || extension == L".wma" || extension == L".mid" ||
        extension == L".midi" || extension == L".aiff" || extension == L".alac") {
        return CategoriaElementoComprimido::Audios;
    }

    if (extension == L".pdf" || extension == L".doc" || extension == L".docx" ||
        extension == L".xls" || extension == L".xlsx" || extension == L".ppt" ||
        extension == L".pptx" || extension == L".epub" || extension == L".mobi" ||
        extension == L".csv" || extension == L".rtf" || extension == L".odt" ||
        extension == L".ods" || extension == L".odp" || extension == L".pages") {
        return CategoriaElementoComprimido::Documentos;
    }

    if (extension == L".cpp" || extension == L".hpp" || extension == L".c" ||
        extension == L".h" || extension == L".cs" || extension == L".py" ||
        extension == L".js" || extension == L".ts" || extension == L".jsx" ||
        extension == L".tsx" || extension == L".html" || extension == L".htm" ||
        extension == L".css" || extension == L".scss" || extension == L".sass" ||
        extension == L".json" || extension == L".xml" || extension == L".yaml" ||
        extension == L".yml" || extension == L".sql" || extension == L".sh" ||
        extension == L".ps1" || extension == L".bat" || extension == L".cmd" ||
        extension == L".java" || extension == L".rs" || extension == L".go" ||
        extension == L".php" || extension == L".rb" || extension == L".swift" ||
        extension == L".kt" || extension == L".dart" || extension == L".lua" ||
        extension == L".wh.cpp") {
        return CategoriaElementoComprimido::Codigo;
    }

    if (extension == L".txt" || extension == L".log" || extension == L".md" ||
        extension == L".ini" || extension == L".cfg" || extension == L".conf" ||
        extension == L".reg" || extension == L".inf" || extension == L".properties") {
        return CategoriaElementoComprimido::Texto;
    }

    if (extension == L".zip" || extension == L".rar" || extension == L".7z" ||
        extension == L".tar" || extension == L".gz" || extension == L".bz2" ||
        extension == L".xz" || extension == L".cab" || extension == L".iso" ||
        extension == L".cbz" || extension == L".cbr" || extension == L".tgz" ||
        extension == L".z") {
        return CategoriaElementoComprimido::Comprimidos;
    }

    return CategoriaElementoComprimido::Otros;
}

inline std::wstring obtenerTituloCategoriaComprimido(CategoriaElementoComprimido categoria) {
    switch (categoria) {
        case CategoriaElementoComprimido::Carpetas:
            return L"Carpetas y directorios";
        case CategoriaElementoComprimido::Videos:
            return L"Archivos de vídeo";
        case CategoriaElementoComprimido::Imagenes:
            return L"Imágenes y fotografías";
        case CategoriaElementoComprimido::Audios:
            return L"Archivos de audio";
        case CategoriaElementoComprimido::Documentos:
            return L"Documentos y ofimática";
        case CategoriaElementoComprimido::Codigo:
            return L"Código y scripts";
        case CategoriaElementoComprimido::Texto:
            return L"Archivos de texto y notas";
        case CategoriaElementoComprimido::Comprimidos:
            return L"Archivos comprimidos";
        case CategoriaElementoComprimido::Otros:
        default:
            return L"Otros archivos y binarios";
    }
}

inline std::wstring obtenerIconoCategoriaComprimido(CategoriaElementoComprimido categoria) {
    switch (categoria) {
        case CategoriaElementoComprimido::Carpetas:
            return L"📁";
        case CategoriaElementoComprimido::Videos:
            return L"🎬";
        case CategoriaElementoComprimido::Imagenes:
            return L"🖼️";
        case CategoriaElementoComprimido::Audios:
            return L"🎵";
        case CategoriaElementoComprimido::Documentos:
            return L"📄";
        case CategoriaElementoComprimido::Codigo:
            return L"💻";
        case CategoriaElementoComprimido::Texto:
            return L"📝";
        case CategoriaElementoComprimido::Comprimidos:
            return L"📦";
        case CategoriaElementoComprimido::Otros:
        default:
            return L"💾";
    }
}

inline void agruparContenidosEnSecciones(const std::vector<ElementoContenidoComprimido>& listaElementos) {
    estadoComprimidos.todosLosElementos = listaElementos;
    estadoComprimidos.seccionesContenido.clear();

    std::vector<ElementoContenidoComprimido> elementosPorCategoria[static_cast<size_t>(CategoriaElementoComprimido::TotalCategorias)];

    for (const auto& elementoActual : listaElementos) {
        size_t indiceCategoria = static_cast<size_t>(elementoActual.categoria);
        if (indiceCategoria < static_cast<size_t>(CategoriaElementoComprimido::TotalCategorias)) {
            elementosPorCategoria[indiceCategoria].push_back(elementoActual);
        }
    }

    for (size_t posicionCategoria = 0; posicionCategoria < static_cast<size_t>(CategoriaElementoComprimido::TotalCategorias); ++posicionCategoria) {
        std::sort(elementosPorCategoria[posicionCategoria].begin(), elementosPorCategoria[posicionCategoria].end(),
            [](const ElementoContenidoComprimido& primerElemento, const ElementoContenidoComprimido& segundoElemento) {
                return StrCmpLogicalW(primerElemento.rutaRelativa.c_str(), segundoElemento.rutaRelativa.c_str()) < 0;
            });
    }

    for (size_t posicionCategoria = 0; posicionCategoria < static_cast<size_t>(CategoriaElementoComprimido::TotalCategorias); ++posicionCategoria) {
        if (!elementosPorCategoria[posicionCategoria].empty()) {
            SeccionContenidoComprimido seccion;
            seccion.categoria = static_cast<CategoriaElementoComprimido>(posicionCategoria);
            seccion.tituloSeccion = obtenerTituloCategoriaComprimido(seccion.categoria);
            seccion.iconoSeccion = obtenerIconoCategoriaComprimido(seccion.categoria);
            seccion.elementos = elementosPorCategoria[posicionCategoria];
            estadoComprimidos.seccionesContenido.push_back(seccion);
        }
    }
}

inline bool inspeccionarArchivoZipCompleto(const std::wstring& rutaArchivo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return false;

    CabeceraArchivoZipLocal cabeceraLocal{};
    DWORD bytesLeidos = 0;
    std::map<std::wstring, ElementoContenidoComprimido> mapaElementos;
    std::map<std::wstring, bool> mapaCarpetasDetectadas;

    while (ReadFile(manejadorArchivo, &cabeceraLocal, sizeof(cabeceraLocal),
                    &bytesLeidos, nullptr) &&
           bytesLeidos == sizeof(cabeceraLocal)) {
        if (cabeceraLocal.firma != 0x04034b50) break;

        if ((cabeceraLocal.banderaGeneral & 0x0001) != 0) {
            estadoComprimidos.protegidoConContrasena = true;
        }

        if (cabeceraLocal.longitudNombreArchivo > 0 &&
            cabeceraLocal.longitudNombreArchivo < 4096) {
            std::vector<char> nombreEntrada(
                cabeceraLocal.longitudNombreArchivo + 1, 0);
            if (!ReadFile(manejadorArchivo, nombreEntrada.data(),
                          cabeceraLocal.longitudNombreArchivo, &bytesLeidos,
                          nullptr) ||
                bytesLeidos != cabeceraLocal.longitudNombreArchivo) {
                break;
            }

            if (cabeceraLocal.longitudCampoExtra > 0) {
                SetFilePointer(manejadorArchivo,
                               cabeceraLocal.longitudCampoExtra, nullptr,
                               FILE_CURRENT);
            }

            std::string nombreUtf8(nombreEntrada.data());
            for (char& caracter : nombreUtf8) {
                if (caracter == '/') caracter = '\\';
            }

            int longitudAncha = MultiByteToWideChar(CP_UTF8, 0, nombreUtf8.c_str(), -1, nullptr, 0);
            if (longitudAncha <= 0) {
                longitudAncha = MultiByteToWideChar(CP_ACP, 0, nombreUtf8.c_str(), -1, nullptr, 0);
            }
            std::wstring rutaRelativaW;
            if (longitudAncha > 0) {
                std::vector<wchar_t> buferAncho(static_cast<size_t>(longitudAncha));
                MultiByteToWideChar(CP_UTF8, 0, nombreUtf8.c_str(), -1, buferAncho.data(), longitudAncha);
                rutaRelativaW = buferAncho.data();
            }

            if (!rutaRelativaW.empty()) {
                bool esDir = false;
                while (!rutaRelativaW.empty() && rutaRelativaW.back() == L'\\') {
                    esDir = true;
                    rutaRelativaW.pop_back();
                }

                if (esDir) {
                    if (!rutaRelativaW.empty() && !mapaCarpetasDetectadas[rutaRelativaW]) {
                        mapaCarpetasDetectadas[rutaRelativaW] = true;
                        ElementoContenidoComprimido elemDir;
                        elemDir.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                        elemDir.rutaRelativa = rutaRelativaW;
                        elemDir.esDirectorio = true;
                        elemDir.tamanoBytes = 0;
                        elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                        mapaElementos[rutaRelativaW] = elemDir;
                    }
                } else if (!rutaRelativaW.empty()) {
                    size_t posicionBarra = 0;
                    while ((posicionBarra = rutaRelativaW.find(L'\\', posicionBarra)) != std::wstring::npos) {
                        std::wstring subDirectorio = rutaRelativaW.substr(0, posicionBarra);
                        if (!subDirectorio.empty() && !mapaCarpetasDetectadas[subDirectorio]) {
                            mapaCarpetasDetectadas[subDirectorio] = true;
                            ElementoContenidoComprimido elemDir;
                            elemDir.nombre = PathFindFileNameW(subDirectorio.c_str());
                            elemDir.rutaRelativa = subDirectorio;
                            elemDir.esDirectorio = true;
                            elemDir.tamanoBytes = 0;
                            elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                            mapaElementos[subDirectorio] = elemDir;
                        }
                        posicionBarra++;
                    }

                    ElementoContenidoComprimido elem;
                    elem.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                    elem.rutaRelativa = rutaRelativaW;
                    elem.esDirectorio = false;
                    elem.tamanoBytes = cabeceraLocal.tamanoDescomprimido;
                    elem.categoria = clasificarCategoriaElementoComprimido(rutaRelativaW, false);
                    mapaElementos[rutaRelativaW] = elem;
                }
            }

            if (cabeceraLocal.tamanoComprimido > 0) {
                SetFilePointer(
                    manejadorArchivo,
                    static_cast<LONG>(cabeceraLocal.tamanoComprimido), nullptr,
                    FILE_CURRENT);
            }
        } else {
            break;
        }
    }
    CloseHandle(manejadorArchivo);

    for (const auto& [claveRuta, elemento] : mapaElementos) {
        salidaElementos.push_back(elemento);
    }
    return !salidaElementos.empty();
}

inline bool inspeccionarArchivoRarCompleto(const std::wstring& rutaArchivo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return false;

    BYTE firma[8] = {0};
    DWORD bytesLeidos = 0;
    if (!ReadFile(manejadorArchivo, firma, 7, &bytesLeidos, nullptr) || bytesLeidos < 7) {
        CloseHandle(manejadorArchivo);
        return false;
    }

    bool esRar4 = (firma[0] == 0x52 && firma[1] == 0x61 && firma[2] == 0x72 && firma[3] == 0x21 && firma[4] == 0x1a && firma[5] == 0x07 && firma[6] == 0x00);
    bool esRar5 = (firma[0] == 0x52 && firma[1] == 0x61 && firma[2] == 0x72 && firma[3] == 0x21 && firma[4] == 0x1a && firma[5] == 0x07 && firma[6] == 0x01);

    if (!esRar4 && !esRar5) {
        CloseHandle(manejadorArchivo);
        return false;
    }

    std::map<std::wstring, ElementoContenidoComprimido> mapaElementos;
    std::map<std::wstring, bool> mapaCarpetasDetectadas;

    if (esRar4) {
        while (true) {
            BYTE cabeceraBloque[7];
            if (!ReadFile(manejadorArchivo, cabeceraBloque, 7, &bytesLeidos, nullptr) || bytesLeidos < 7) {
                break;
            }
            BYTE tipoBloque = cabeceraBloque[2];
            uint16_t banderas = static_cast<uint16_t>(cabeceraBloque[3] | (cabeceraBloque[4] << 8));
            uint16_t tamanoCabecera = static_cast<uint16_t>(cabeceraBloque[5] | (cabeceraBloque[6] << 8));

            if (tamanoCabecera < 7) break;

            uint32_t tamanoCuerpo = tamanoCabecera - 7;
            std::vector<BYTE> cuerpoCabecera(tamanoCuerpo);
            if (tamanoCuerpo > 0) {
                if (!ReadFile(manejadorArchivo, cuerpoCabecera.data(), tamanoCuerpo, &bytesLeidos, nullptr) || bytesLeidos < tamanoCuerpo) {
                    break;
                }
            }

            uint32_t tamanoEmpaquetado = 0;
            if (tipoBloque == 0x74) {
                if (cuerpoCabecera.size() >= 21) {
                    tamanoEmpaquetado = static_cast<uint32_t>(cuerpoCabecera[0] | (cuerpoCabecera[1] << 8) | (cuerpoCabecera[2] << 16) | (cuerpoCabecera[3] << 24));
                    uint32_t tamanoDesemp = static_cast<uint32_t>(cuerpoCabecera[4] | (cuerpoCabecera[5] << 8) | (cuerpoCabecera[6] << 16) | (cuerpoCabecera[7] << 24));
                    uint16_t tamanoNombre = static_cast<uint16_t>(cuerpoCabecera[19] | (cuerpoCabecera[20] << 8));

                    if ((banderas & 0x0004) != 0) {
                        estadoComprimidos.protegidoConContrasena = true;
                    }
                    bool esDirectorio = (banderas & 0x00E0) == 0x00E0 || (banderas & 0x0020) != 0;

                    size_t offsetNombre = 25;
                    if ((banderas & 0x0100) != 0 && cuerpoCabecera.size() >= 33) {
                        offsetNombre += 8;
                    }
                    if (cuerpoCabecera.size() >= offsetNombre + tamanoNombre && tamanoNombre > 0) {
                        std::string nombreStr(reinterpret_cast<char*>(cuerpoCabecera.data() + offsetNombre), tamanoNombre);
                        for (char& caracter : nombreStr) {
                            if (caracter == '/') caracter = '\\';
                        }
                        int longitudAncha = MultiByteToWideChar(CP_UTF8, 0, nombreStr.c_str(), -1, nullptr, 0);
                        if (longitudAncha <= 0) {
                            longitudAncha = MultiByteToWideChar(CP_ACP, 0, nombreStr.c_str(), -1, nullptr, 0);
                        }
                        if (longitudAncha > 0) {
                            std::vector<wchar_t> buferAncho(static_cast<size_t>(longitudAncha));
                            MultiByteToWideChar(CP_UTF8, 0, nombreStr.c_str(), -1, buferAncho.data(), longitudAncha);
                            std::wstring rutaRelativaW = buferAncho.data();

                            while (!rutaRelativaW.empty() && rutaRelativaW.back() == L'\\') {
                                esDirectorio = true;
                                rutaRelativaW.pop_back();
                            }

                            if (!rutaRelativaW.empty()) {
                                if (esDirectorio) {
                                    if (!mapaCarpetasDetectadas[rutaRelativaW]) {
                                        mapaCarpetasDetectadas[rutaRelativaW] = true;
                                        ElementoContenidoComprimido elemDir;
                                        elemDir.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                                        elemDir.rutaRelativa = rutaRelativaW;
                                        elemDir.esDirectorio = true;
                                        elemDir.tamanoBytes = 0;
                                        elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                                        mapaElementos[rutaRelativaW] = elemDir;
                                    }
                                } else {
                                    size_t posicionBarra = 0;
                                    while ((posicionBarra = rutaRelativaW.find(L'\\', posicionBarra)) != std::wstring::npos) {
                                        std::wstring subDirectorio = rutaRelativaW.substr(0, posicionBarra);
                                        if (!subDirectorio.empty() && !mapaCarpetasDetectadas[subDirectorio]) {
                                            mapaCarpetasDetectadas[subDirectorio] = true;
                                            ElementoContenidoComprimido elemDir;
                                            elemDir.nombre = PathFindFileNameW(subDirectorio.c_str());
                                            elemDir.rutaRelativa = subDirectorio;
                                            elemDir.esDirectorio = true;
                                            elemDir.tamanoBytes = 0;
                                            elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                                            mapaElementos[subDirectorio] = elemDir;
                                        }
                                        posicionBarra++;
                                    }
                                    ElementoContenidoComprimido elem;
                                    elem.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                                    elem.rutaRelativa = rutaRelativaW;
                                    elem.esDirectorio = false;
                                    elem.tamanoBytes = tamanoDesemp;
                                    elem.categoria = clasificarCategoriaElementoComprimido(rutaRelativaW, false);
                                    mapaElementos[rutaRelativaW] = elem;
                                }
                            }
                        }
                    }
                }
            }

            if ((banderas & 0x8000) != 0 && tamanoEmpaquetado > 0) {
                SetFilePointer(manejadorArchivo, static_cast<LONG>(tamanoEmpaquetado), nullptr, FILE_CURRENT);
            }
        }
    }
    CloseHandle(manejadorArchivo);

    for (const auto& [claveRuta, elemento] : mapaElementos) {
        salidaElementos.push_back(elemento);
    }
    return !salidaElementos.empty();
}

inline bool inspeccionarArchivoTarCompleto(const std::wstring& rutaArchivo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return false;

    BYTE bloque[512];
    DWORD bytesLeidos = 0;
    std::map<std::wstring, ElementoContenidoComprimido> mapaElementos;
    std::map<std::wstring, bool> mapaCarpetasDetectadas;

    while (ReadFile(manejadorArchivo, bloque, 512, &bytesLeidos, nullptr) && bytesLeidos == 512) {
        if (bloque[0] == 0) break;

        char nombreEntrada[101] = {0};
        memcpy(nombreEntrada, bloque, 100);
        char prefijoEntrada[156] = {0};
        memcpy(prefijoEntrada, bloque + 345, 155);

        std::string rutaCompleta = (prefijoEntrada[0] != '\0')
            ? (std::string(prefijoEntrada) + "/" + std::string(nombreEntrada))
            : std::string(nombreEntrada);

        for (char& caracter : rutaCompleta) {
            if (caracter == '/') caracter = '\\';
        }

        char tamanoOctal[13] = {0};
        memcpy(tamanoOctal, bloque + 124, 12);
        uintmax_t tamanoArchivo = strtoull(tamanoOctal, nullptr, 8);

        char tipoEntrada = static_cast<char>(bloque[156]);
        bool esDirectorio = (tipoEntrada == '5') || (!rutaCompleta.empty() && rutaCompleta.back() == '\\');

        int longitudAncha = MultiByteToWideChar(CP_UTF8, 0, rutaCompleta.c_str(), -1, nullptr, 0);
        if (longitudAncha <= 0) {
            longitudAncha = MultiByteToWideChar(CP_ACP, 0, rutaCompleta.c_str(), -1, nullptr, 0);
        }
        if (longitudAncha > 0) {
            std::vector<wchar_t> buferAncho(static_cast<size_t>(longitudAncha));
            MultiByteToWideChar(CP_UTF8, 0, rutaCompleta.c_str(), -1, buferAncho.data(), longitudAncha);
            std::wstring rutaRelativaW = buferAncho.data();

            while (!rutaRelativaW.empty() && rutaRelativaW.back() == L'\\') {
                esDirectorio = true;
                rutaRelativaW.pop_back();
            }

            if (!rutaRelativaW.empty()) {
                if (esDirectorio) {
                    if (!mapaCarpetasDetectadas[rutaRelativaW]) {
                        mapaCarpetasDetectadas[rutaRelativaW] = true;
                        ElementoContenidoComprimido elemDir;
                        elemDir.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                        elemDir.rutaRelativa = rutaRelativaW;
                        elemDir.esDirectorio = true;
                        elemDir.tamanoBytes = 0;
                        elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                        mapaElementos[rutaRelativaW] = elemDir;
                    }
                } else {
                    size_t posicionBarra = 0;
                    while ((posicionBarra = rutaRelativaW.find(L'\\', posicionBarra)) != std::wstring::npos) {
                        std::wstring subDirectorio = rutaRelativaW.substr(0, posicionBarra);
                        if (!subDirectorio.empty() && !mapaCarpetasDetectadas[subDirectorio]) {
                            mapaCarpetasDetectadas[subDirectorio] = true;
                            ElementoContenidoComprimido elemDir;
                            elemDir.nombre = PathFindFileNameW(subDirectorio.c_str());
                            elemDir.rutaRelativa = subDirectorio;
                            elemDir.esDirectorio = true;
                            elemDir.tamanoBytes = 0;
                            elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                            mapaElementos[subDirectorio] = elemDir;
                        }
                        posicionBarra++;
                    }
                    ElementoContenidoComprimido elem;
                    elem.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                    elem.rutaRelativa = rutaRelativaW;
                    elem.esDirectorio = false;
                    elem.tamanoBytes = tamanoArchivo;
                    elem.categoria = clasificarCategoriaElementoComprimido(rutaRelativaW, false);
                    mapaElementos[rutaRelativaW] = elem;
                }
            }
        }

        uintmax_t bloquesSaltar = (tamanoArchivo + 511) / 512;
        if (bloquesSaltar > 0) {
            LARGE_INTEGER salto{};
            salto.QuadPart = static_cast<LONGLONG>(bloquesSaltar * 512);
            SetFilePointerEx(manejadorArchivo, salto, nullptr, FILE_CURRENT);
        }
    }
    CloseHandle(manejadorArchivo);

    for (const auto& [claveRuta, elemento] : mapaElementos) {
        salidaElementos.push_back(elemento);
    }
    return !salidaElementos.empty();
}

inline void enumerarRecursivoFolderItems(IDispatch* carpetaDispatch, const std::wstring& rutaPrefijo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    if (!carpetaDispatch) return;

    DISPPARAMS paramsVacio{};
    VARIANT resultadoItems{};
    VariantInit(&resultadoItems);
    DISPID dispidItems = 0;
    OLECHAR* nombreItems = const_cast<OLECHAR*>(L"Items");
    if (SUCCEEDED(carpetaDispatch->GetIDsOfNames(IID_NULL, &nombreItems, 1, LOCALE_USER_DEFAULT, &dispidItems)) &&
        SUCCEEDED(carpetaDispatch->Invoke(dispidItems, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsVacio, &resultadoItems, nullptr, nullptr)) &&
        resultadoItems.vt == VT_DISPATCH && resultadoItems.pdispVal) {

        IDispatch* itemsDispatch = resultadoItems.pdispVal;
        VARIANT resultadoCount{};
        VariantInit(&resultadoCount);
        DISPID dispidCount = 0;
        OLECHAR* nombreCount = const_cast<OLECHAR*>(L"Count");
        if (SUCCEEDED(itemsDispatch->GetIDsOfNames(IID_NULL, &nombreCount, 1, LOCALE_USER_DEFAULT, &dispidCount)) &&
            SUCCEEDED(itemsDispatch->Invoke(dispidCount, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resultadoCount, nullptr, nullptr)) &&
            resultadoCount.vt == VT_I4) {

            long totalItems = resultadoCount.lVal;
            DISPID dispidItem = 0;
            OLECHAR* nombreItem = const_cast<OLECHAR*>(L"Item");
            itemsDispatch->GetIDsOfNames(IID_NULL, &nombreItem, 1, LOCALE_USER_DEFAULT, &dispidItem);

            DISPID dispidIsFolder = 0;
            OLECHAR* nombreIsFolder = const_cast<OLECHAR*>(L"IsFolder");
            DISPID dispidName = 0;
            OLECHAR* nombreName = const_cast<OLECHAR*>(L"Name");
            DISPID dispidSize = 0;
            OLECHAR* nombreSize = const_cast<OLECHAR*>(L"Size");
            DISPID dispidGetFolder = 0;
            OLECHAR* nombreGetFolder = const_cast<OLECHAR*>(L"GetFolder");

            for (long pos = 0; pos < totalItems; ++pos) {
                VARIANT argPos{};
                argPos.vt = VT_I4;
                argPos.lVal = pos;
                DISPPARAMS paramsItem{};
                paramsItem.cArgs = 1;
                paramsItem.rgvarg = &argPos;
                VARIANT resultadoItem{};
                VariantInit(&resultadoItem);

                if (SUCCEEDED(itemsDispatch->Invoke(dispidItem, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsItem, &resultadoItem, nullptr, nullptr)) &&
                    resultadoItem.vt == VT_DISPATCH && resultadoItem.pdispVal) {

                    IDispatch* itemDispatch = resultadoItem.pdispVal;

                    if (dispidIsFolder == 0) itemDispatch->GetIDsOfNames(IID_NULL, &nombreIsFolder, 1, LOCALE_USER_DEFAULT, &dispidIsFolder);
                    if (dispidName == 0) itemDispatch->GetIDsOfNames(IID_NULL, &nombreName, 1, LOCALE_USER_DEFAULT, &dispidName);
                    if (dispidSize == 0) itemDispatch->GetIDsOfNames(IID_NULL, &nombreSize, 1, LOCALE_USER_DEFAULT, &dispidSize);
                    if (dispidGetFolder == 0) itemDispatch->GetIDsOfNames(IID_NULL, &nombreGetFolder, 1, LOCALE_USER_DEFAULT, &dispidGetFolder);

                    VARIANT resIsFolder{};
                    VariantInit(&resIsFolder);
                    itemDispatch->Invoke(dispidIsFolder, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resIsFolder, nullptr, nullptr);
                    bool esFolder = (resIsFolder.vt == VT_BOOL && resIsFolder.boolVal == VARIANT_TRUE);

                    VARIANT resName{};
                    VariantInit(&resName);
                    itemDispatch->Invoke(dispidName, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resName, nullptr, nullptr);
                    std::wstring nombreElemento = (resName.vt == VT_BSTR && resName.bstrVal) ? resName.bstrVal : L"";
                    VariantClear(&resName);

                    VARIANT resSize{};
                    VariantInit(&resSize);
                    itemDispatch->Invoke(dispidSize, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resSize, nullptr, nullptr);
                    uintmax_t tamanoElem = (resSize.vt == VT_I4) ? static_cast<uintmax_t>(resSize.lVal) : ((resSize.vt == VT_UI4) ? resSize.ulVal : 0);
                    VariantClear(&resSize);

                    if (!nombreElemento.empty()) {
                        std::wstring rutaRelativa = rutaPrefijo.empty() ? nombreElemento : (rutaPrefijo + L"\\" + nombreElemento);
                        ElementoContenidoComprimido elem;
                        elem.nombre = nombreElemento;
                        elem.rutaRelativa = rutaRelativa;
                        elem.esDirectorio = esFolder;
                        elem.tamanoBytes = tamanoElem;
                        elem.categoria = clasificarCategoriaElementoComprimido(rutaRelativa, esFolder);
                        salidaElementos.push_back(elem);

                        if (esFolder) {
                            VARIANT resSubFolder{};
                            VariantInit(&resSubFolder);
                            if (SUCCEEDED(itemDispatch->Invoke(dispidGetFolder, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resSubFolder, nullptr, nullptr)) &&
                                resSubFolder.vt == VT_DISPATCH && resSubFolder.pdispVal) {
                                enumerarRecursivoFolderItems(resSubFolder.pdispVal, rutaRelativa, salidaElementos);
                                resSubFolder.pdispVal->Release();
                            }
                        }
                    }
                    itemDispatch->Release();
                }
            }
        }
        itemsDispatch->Release();
    }
}

inline bool inspeccionarArchivoViaShellDispatch(const std::wstring& rutaArchivo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    CLSID clsidShell;
    if (FAILED(CLSIDFromProgID(L"Shell.Application", &clsidShell))) return false;

    IDispatch* shellDispatch = nullptr;
    if (FAILED(CoCreateInstance(clsidShell, nullptr, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IDispatch, reinterpret_cast<void**>(&shellDispatch))) || !shellDispatch) {
        return false;
    }

    DISPID dispidNameSpace = 0;
    OLECHAR* nombreNameSpace = const_cast<OLECHAR*>(L"NameSpace");
    if (FAILED(shellDispatch->GetIDsOfNames(IID_NULL, &nombreNameSpace, 1, LOCALE_USER_DEFAULT, &dispidNameSpace))) {
        shellDispatch->Release();
        return false;
    }

    VARIANT argRuta{};
    argRuta.vt = VT_BSTR;
    argRuta.bstrVal = SysAllocString(rutaArchivo.c_str());
    DISPPARAMS paramsNameSpace{};
    paramsNameSpace.cArgs = 1;
    paramsNameSpace.rgvarg = &argRuta;

    VARIANT resFolder{};
    VariantInit(&resFolder);
    HRESULT hr = shellDispatch->Invoke(dispidNameSpace, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsNameSpace, &resFolder, nullptr, nullptr);
    SysFreeString(argRuta.bstrVal);
    shellDispatch->Release();

    if (SUCCEEDED(hr) && resFolder.vt == VT_DISPATCH && resFolder.pdispVal) {
        enumerarRecursivoFolderItems(resFolder.pdispVal, L"", salidaElementos);
        resFolder.pdispVal->Release();
        return !salidaElementos.empty();
    }
    return false;
}

inline void inspeccionarEstructuraArchivoZip(const std::wstring& rutaArchivo) {
    estadoComprimidos.seccionesContenido.clear();
    estadoComprimidos.todosLosElementos.clear();
    estadoComprimidos.elementosArbol.clear();
    estadoComprimidos.desplazamientoScrollY = 0;
    estadoComprimidos.protegidoConContrasena = false;

    const wchar_t* extension = PathFindExtensionW(rutaArchivo.c_str());

    // El escaneo multinivel solo aplica para archivos comprimidos comunes.
    // Los archivos de comics (.cbz, .cbr, .cb7, .cbt, .cba) quedan excluidos.
    if (esExtensionComic(extension)) {
        return;
    }

    std::vector<ElementoContenidoComprimido> listaElementos;

    bool parseado = false;
    if (lstrcmpiW(extension, L".zip") == 0 || lstrcmpiW(extension, L".jar") == 0 ||
        lstrcmpiW(extension, L".apk") == 0) {
        parseado = inspeccionarArchivoZipCompleto(rutaArchivo, listaElementos);
    } else if (lstrcmpiW(extension, L".rar") == 0) {
        parseado = inspeccionarArchivoRarCompleto(rutaArchivo, listaElementos);
    } else if (lstrcmpiW(extension, L".tar") == 0) {
        parseado = inspeccionarArchivoTarCompleto(rutaArchivo, listaElementos);
    }

    if (!parseado || listaElementos.empty()) {
        inspeccionarArchivoViaShellDispatch(rutaArchivo, listaElementos);
    }

    if (listaElementos.empty()) {
        inspeccionarArchivoZipCompleto(rutaArchivo, listaElementos);
    }

    agruparContenidosEnSecciones(listaElementos);

    for (const auto& elementoActual : estadoComprimidos.todosLosElementos) {
        ElementoArbolComprimido itemArbol;
        itemArbol.nombre = elementoActual.nombre;
        itemArbol.rutaCompleta = elementoActual.rutaRelativa;
        itemArbol.esDirectorio = elementoActual.esDirectorio;
        itemArbol.nivelJerarquia = 0;
        itemArbol.tamanoBytes = elementoActual.tamanoBytes;
        estadoComprimidos.elementosArbol.push_back(itemArbol);
    }
}

inline bool cargarComprimidoPorIndice(size_t indiceComprimido) {
    if (indiceComprimido >= estadoComprimidos.archivosSeleccionados.size()) return false;
    estadoComprimidos.indiceArchivoActual = indiceComprimido;
    const std::wstring& ruta = estadoComprimidos.archivosSeleccionados[indiceComprimido];

    const wchar_t* extension = PathFindExtensionW(ruta.c_str());
    if (esExtensionComic(extension)) {
        // Los comics quedan excluidos del escaneo multinivel y visor de árbol
        return true;
    }

    WIN32_FILE_ATTRIBUTE_DATA datosAttr;
    bool tieneAttr = GetFileAttributesExW(ruta.c_str(), GetFileExInfoStandard, &datosAttr) != FALSE;

    if (tieneAttr && estadoComprimidos.rutaComprimidoArbolCargado == ruta &&
        !estadoComprimidos.elementosArbol.empty() &&
        memcmp(&datosAttr.ftLastWriteTime, &estadoComprimidos.ftUltimaModificacionArbol, sizeof(FILETIME)) == 0) {
        return true;
    }

    estadoComprimidos.rutaComprimidoArbolCargado = ruta;
    if (tieneAttr) {
        estadoComprimidos.ftUltimaModificacionArbol = datosAttr.ftLastWriteTime;
    }
    inspeccionarEstructuraArchivoZip(ruta);
    return true;
}

inline bool mostrarComprimidoAnterior() {
    if (estadoComprimidos.archivosSeleccionados.size() <= 1) return false;
    if (estadoComprimidos.indiceArchivoActual == 0) {
        estadoComprimidos.indiceArchivoActual = estadoComprimidos.archivosSeleccionados.size() - 1;
    } else {
        estadoComprimidos.indiceArchivoActual--;
    }
    return cargarComprimidoPorIndice(estadoComprimidos.indiceArchivoActual);
}

inline bool mostrarComprimidoSiguiente() {
    if (estadoComprimidos.archivosSeleccionados.size() <= 1) return false;
    estadoComprimidos.indiceArchivoActual =
        (estadoComprimidos.indiceArchivoActual + 1) % estadoComprimidos.archivosSeleccionados.size();
    return cargarComprimidoPorIndice(estadoComprimidos.indiceArchivoActual);
}

inline const ElementoArbolComprimido* obtenerElementoArbolComprimidoEnPunto(POINT puntoRaton) {
    if (estadoComprimidos.esModoComic || estadoComprimidos.elementosArbol.empty()) return nullptr;
    if (!PtInRect(&estadoComprimidos.areaCajaArbol, puntoRaton)) return nullptr;

    int yInicioArbol = estadoComprimidos.areaCajaArbol.top + 8 - estadoComprimidos.desplazamientoScrollY;
    int indiceElementoCalculado = (puntoRaton.y - yInicioArbol) / 22;
    if (indiceElementoCalculado >= 0 &&
        static_cast<size_t>(indiceElementoCalculado) < estadoComprimidos.elementosArbol.size()) {
        return &estadoComprimidos.elementosArbol[static_cast<size_t>(indiceElementoCalculado)];
    }
    return nullptr;
}

inline void desplazarScrollComprimido(int incrementoDelta) {
    int maximoScroll = std::max(0, estadoComprimidos.altoTotalArbol - 100);
    estadoComprimidos.desplazamientoScrollY = std::max(
        0, std::min(estadoComprimidos.desplazamientoScrollY + incrementoDelta, maximoScroll));
}

inline void seleccionarYAgregarPrograma(HWND ventanaPadre) {
    wchar_t buferRutaArchivo[MAX_PATH] = L"";
    OPENFILENAMEW estructuraOfn{};
    estructuraOfn.lStructSize = sizeof(estructuraOfn);
    estructuraOfn.hwndOwner = ventanaPadre;
    estructuraOfn.lpstrFilter =
        L"Archivos Ejecutables (*.exe)\0*.exe\0Todos los archivos (*.*)\0*.*\0";
    estructuraOfn.lpstrFile = buferRutaArchivo;
    estructuraOfn.nMaxFile = MAX_PATH;
    estructuraOfn.Flags =
        OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    estructuraOfn.lpstrTitle =
        estadoComprimidos.tieneComicsSeleccionados
            ? L"Seleccionar programa para abrir cómics (.cbz, .cbr)"
            : L"Seleccionar programa para comprimidos comunes (.zip, .rar, .7z, .tar)";

    if (GetOpenFileNameW(&estructuraOfn)) {
        std::wstring rutaSeleccionada(buferRutaArchivo);
        if (!rutaSeleccionada.empty()) {
            std::wstring nombreEjecutable =
                PathFindFileNameW(rutaSeleccionada.c_str());

            std::wstring mensajePregunta =
                estadoComprimidos.tieneComicsSeleccionados
                    ? (L"¿Desea recordar '" + nombreEjecutable +
                       L"' en la lista de opciones para cómics?")
                    : (L"¿Desea recordar '" + nombreEjecutable +
                       L"' en la lista de opciones para comprimidos comunes?");

            int respuesta = MessageBoxW(
                ventanaPadre, mensajePregunta.c_str(),
                L"Recordar programa", MB_YESNO | MB_ICONQUESTION);

            std::vector<std::wstring>& listaActiva =
                obtenerListaProgramasActiva();

            if (respuesta == IDYES) {
                auto iteradorPrograma = std::find(
                    listaActiva.begin(), listaActiva.end(), rutaSeleccionada);
                if (iteradorPrograma == listaActiva.end()) {
                    listaActiva.push_back(rutaSeleccionada);
                    estadoComprimidos.indiceProgramaSeleccionado =
                        static_cast<int>(listaActiva.size() - 1);
                } else {
                    estadoComprimidos.indiceProgramaSeleccionado =
                        static_cast<int>(std::distance(
                            listaActiva.begin(), iteradorPrograma));
                }
                estadoComprimidos.programaTemporal.clear();
            } else {
                estadoComprimidos.programaTemporal = rutaSeleccionada;
            }
        }
    }
}

inline void mostrarMenuDesplegableProgramasComprimidos(HWND ventana, const RECT& botonRect) {
    const std::vector<std::wstring>& listaProgramas = obtenerListaProgramasActivaConst();
    if (listaProgramas.empty() && estadoComprimidos.programaTemporal.empty()) {
        seleccionarYAgregarPrograma(ventana);
        InvalidateRect(ventana, nullptr, FALSE);
        return;
    }

    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    for (size_t posicionPrograma = 0; posicionPrograma < listaProgramas.size(); ++posicionPrograma) {
        UINT banderasMenu = MF_STRING;
        if (static_cast<int>(posicionPrograma) == estadoComprimidos.indiceProgramaSeleccionado) {
            banderasMenu |= MF_CHECKED;
        }
        std::wstring nombreProg = PathFindFileNameW(listaProgramas[posicionPrograma].c_str());
        AppendMenuW(menu, banderasMenu, static_cast<UINT_PTR>(2000 + posicionPrograma), nombreProg.c_str());
    }

    if (!listaProgramas.empty()) {
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    AppendMenuW(menu, MF_STRING, 2999, L"+ Añadir programa...");

    POINT puntoBoton{botonRect.left, botonRect.top};
    ClientToScreen(ventana, &puntoBoton);

    SetForegroundWindow(ventana);
    UINT comandoSeleccionado = static_cast<UINT>(TrackPopupMenuEx(
        menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_VERTICAL,
        puntoBoton.x, puntoBoton.y, ventana, nullptr));
    DestroyMenu(menu);

    if (comandoSeleccionado >= 2000 && comandoSeleccionado < 2000 + listaProgramas.size()) {
        estadoComprimidos.indiceProgramaSeleccionado = static_cast<int>(comandoSeleccionado - 2000);
        estadoComprimidos.programaTemporal.clear();
        InvalidateRect(ventana, nullptr, FALSE);
    } else if (comandoSeleccionado == 2999) {
        seleccionarYAgregarPrograma(ventana);
        InvalidateRect(ventana, nullptr, FALSE);
    }
}

inline void mostrarMenuDesplegableFormatosComprimidos(HWND ventana, const RECT& botonRect) {
    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    for (int posicionFormato = 0; posicionFormato < TOTAL_FORMATOS_DESTINO; ++posicionFormato) {
        UINT banderasMenu = MF_STRING;
        if (posicionFormato == estadoComprimidos.indiceFormatoDestino) {
            banderasMenu |= MF_CHECKED;
        }
        AppendMenuW(menu, banderasMenu, static_cast<UINT_PTR>(1000 + posicionFormato), FORMATOS_DESTINO[posicionFormato]);
    }

    POINT puntoBoton{botonRect.left, botonRect.top};
    ClientToScreen(ventana, &puntoBoton);

    SetForegroundWindow(ventana);
    UINT comandoSeleccionado = static_cast<UINT>(TrackPopupMenuEx(
        menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_VERTICAL,
        puntoBoton.x, puntoBoton.y, ventana, nullptr));
    DestroyMenu(menu);

    if (comandoSeleccionado >= 1000 && comandoSeleccionado < 1000 + TOTAL_FORMATOS_DESTINO) {
        estadoComprimidos.indiceFormatoDestino = static_cast<int>(comandoSeleccionado - 1000);
        InvalidateRect(ventana, nullptr, FALSE);
    }
}

inline std::wstring resolverProgramaParaComprimidos() {
    if (!estadoComprimidos.programaTemporal.empty()) {
        return estadoComprimidos.programaTemporal;
    }
    const std::vector<std::wstring>& listaActiva =
        obtenerListaProgramasActivaConst();
    if (!listaActiva.empty() &&
        estadoComprimidos.indiceProgramaSeleccionado >= 0 &&
        estadoComprimidos.indiceProgramaSeleccionado <
            static_cast<int>(listaActiva.size())) {
        return listaActiva[static_cast<size_t>(
            estadoComprimidos.indiceProgramaSeleccionado)];
    }
    if (estadoComprimidos.tieneComicsSeleccionados) {
        if (!configuracion.programaComicPorDefecto.empty()) {
            return configuracion.programaComicPorDefecto;
        }
        if (estadoComprimidos.programasComicsRecordados.size() == 1) {
            return estadoComprimidos.programasComicsRecordados.front();
        }
    } else {
        if (!configuracion.programaComprimidosPorDefecto.empty()) {
            return configuracion.programaComprimidosPorDefecto;
        }
        if (estadoComprimidos.programasComunesRecordados.size() == 1) {
            return estadoComprimidos.programasComunesRecordados.front();
        }
    }
    if (!listaActiva.empty()) {
        return listaActiva.front();
    }
    return {};
}

inline void ejecutarAbrirArchivosConPrograma(HWND ventanaPadre) {
    std::wstring rutaPrograma = resolverProgramaParaComprimidos();

    if (rutaPrograma.empty()) {
        seleccionarYAgregarPrograma(ventanaPadre);
        rutaPrograma = resolverProgramaParaComprimidos();
        if (rutaPrograma.empty()) return;
    }

    const auto& listaArchivos = estadoComprimidos.archivosSeleccionados;
    for (const auto& ruta : listaArchivos) {
        ShellExecuteW(
            nullptr, L"open", rutaPrograma.c_str(),
            (L"\"" + ruta + L"\"").c_str(), nullptr,
            SW_SHOWNORMAL);
    }
}

inline bool cargarArchivosComprimidos(
    const std::vector<std::wstring>& listaRutas) {
    liberarRecursosComprimido();
    if (listaRutas.empty()) return false;

    estadoComprimidos.archivosSeleccionados = listaRutas;
    estadoComprimidos.totalSeleccionados = listaRutas.size();
    estadoComprimidos.indiceArchivoActual = 0;

    estadoComprimidos.tieneComicsSeleccionados = false;
    for (const std::wstring& ruta : listaRutas) {
        const wchar_t* extension = PathFindExtensionW(ruta.c_str());
        if (esExtensionComic(extension)) {
            estadoComprimidos.tieneComicsSeleccionados = true;
            break;
        }
    }
    estadoComprimidos.esModoComic = estadoComprimidos.tieneComicsSeleccionados;

    if (estadoComprimidos.esModoComic) {
        int limiteGrid = configuracion.limiteCuadriculaComprimidos.load();
        if (limiteGrid != 4 && limiteGrid != 6 && limiteGrid != 8 &&
            limiteGrid != 10) {
            limiteGrid = 4;
        }
        size_t limiteCarga = std::min<size_t>(
            static_cast<size_t>(limiteGrid), listaRutas.size());

        for (size_t posicion = 0; posicion < limiteCarga; ++posicion) {
            const std::wstring& rutaActual = listaRutas[posicion];
            ElementoComprimido elemento;
            elemento.rutaArchivo = rutaActual;
            elemento.nombreArchivo = PathFindFileNameW(rutaActual.c_str());
            const wchar_t* extension = PathFindExtensionW(rutaActual.c_str());
            elemento.extensionFormato = extension ? extension : L"";
            elemento.esComic = esExtensionComic(extension);

            WIN32_FILE_ATTRIBUTE_DATA atributos{};
            if (GetFileAttributesExW(rutaActual.c_str(), GetFileExInfoStandard,
                                     &atributos)) {
                ULARGE_INTEGER tamanoGrande{};
                tamanoGrande.LowPart = atributos.nFileSizeLow;
                tamanoGrande.HighPart = atributos.nFileSizeHigh;
                elemento.textoTamano =
                    formatearTamanoArchivo(tamanoGrande.QuadPart);
            }

            elemento.portadaCargada = obtenerPortadaDeComprimido(rutaActual);
            estadoComprimidos.elementosSeleccionados.push_back(elemento);
        }
    } else {
        cargarComprimidoPorIndice(0);
    }

    return true;
}

inline void ejecutarConversionArchivos() {
    if (estadoComprimidos.totalSeleccionados == 0 ||
        estadoComprimidos.conversionEnCurso) {
        return;
    }

    estadoComprimidos.conversionEnCurso = true;
    const wchar_t* extensionDestino =
        EXTENSIONES_DESTINO[estadoComprimidos.indiceFormatoDestino];

    for (const ElementoComprimido& elemento :
         estadoComprimidos.elementosSeleccionados) {
        wchar_t rutaDestino[MAX_PATH];
        wcsncpy_s(rutaDestino, elemento.rutaArchivo.c_str(), _TRUNCATE);
        PathRenameExtensionW(rutaDestino, extensionDestino);

        if (lstrcmpiW(elemento.rutaArchivo.c_str(), rutaDestino) == 0) {
            continue;
        }

        CopyFileW(elemento.rutaArchivo.c_str(), rutaDestino, FALSE);
    }

    estadoComprimidos.conversionEnCurso = false;
}


