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

struct EstadoControladorComprimidos {
    std::vector<std::wstring> archivosSeleccionados;
    size_t indiceArchivoActual{0};
    size_t totalSeleccionados{0};
    bool tieneComicsSeleccionados{false};
    bool esModoComic{false};

    // Modo Cómic: portadas
    std::vector<ElementoComprimido> elementosSeleccionados;

    // Modo Comprimidos Comunes: TreeMap
    std::vector<ElementoArbolComprimido> elementosArbol;
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
    std::vector<std::wstring> lineasRegistro;
    int desplazamientoScrollLog{0};
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

inline std::wstring obtenerMarcaDeTiempoActual() {
    SYSTEMTIME tiempoSistema;
    GetLocalTime(&tiempoSistema);
    wchar_t buferHora[32];
    swprintf_s(buferHora, L"[%02u:%02u:%02u] ",
               static_cast<unsigned int>(tiempoSistema.wHour),
               static_cast<unsigned int>(tiempoSistema.wMinute),
               static_cast<unsigned int>(tiempoSistema.wSecond));
    return buferHora;
}

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
        rutaArchivo.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
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

inline void agregarLineaRegistro(const std::wstring& lineaMensaje) {
    std::wstring lineaConHora =
        obtenerMarcaDeTiempoActual() + lineaMensaje;
    estadoComprimidos.lineasRegistro.push_back(lineaConHora);
    if (estadoComprimidos.lineasRegistro.size() > 50) {
        estadoComprimidos.lineasRegistro.erase(
            estadoComprimidos.lineasRegistro.begin());
    }
    estadoComprimidos.desplazamientoScrollLog = static_cast<int>(
        std::max<size_t>(0, estadoComprimidos.lineasRegistro.size() - 5));
}

inline void inspeccionarEstructuraArchivoZip(const std::wstring& rutaArchivo) {
    estadoComprimidos.elementosArbol.clear();
    estadoComprimidos.desplazamientoScrollY = 0;
    estadoComprimidos.protegidoConContrasena = false;

    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return;

    CabeceraArchivoZipLocal cabeceraLocal{};
    DWORD bytesLeidos = 0;
    std::map<std::wstring, ElementoArbolComprimido> elementosPrimerNivel;

    while (ReadFile(manejadorArchivo, &cabeceraLocal, sizeof(cabeceraLocal),
                    &bytesLeidos, nullptr) &&
           bytesLeidos == sizeof(cabeceraLocal)) {
        if (cabeceraLocal.firma != 0x04034b50) break;

        // Bit 0 de bandera general indica cifrado / contraseña
        if ((cabeceraLocal.banderaGeneral & 0x0001) != 0) {
            estadoComprimidos.protegidoConContrasena = true;
        }

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

            std::string nombreUtf8(nombreEntrada.data());
            for (char& c : nombreUtf8) {
                if (c == '/') c = '\\';
            }

            int longitudCaracteresAnchos = MultiByteToWideChar(CP_UTF8, 0, nombreUtf8.c_str(), -1, nullptr, 0);
            if (longitudCaracteresAnchos <= 0) {
                longitudCaracteresAnchos = MultiByteToWideChar(CP_ACP, 0, nombreUtf8.c_str(), -1, nullptr, 0);
            }
            std::wstring rutaRelativaW;
            if (longitudCaracteresAnchos > 0) {
                std::vector<wchar_t> buferCaracteresAnchos(static_cast<size_t>(longitudCaracteresAnchos));
                MultiByteToWideChar(CP_UTF8, 0, nombreUtf8.c_str(), -1, buferCaracteresAnchos.data(), longitudCaracteresAnchos);
                rutaRelativaW = buferCaracteresAnchos.data();
            }

            if (!rutaRelativaW.empty()) {
                // Extraer componente del primer nivel (root)
                size_t primerSep = rutaRelativaW.find(L'\\');
                if (primerSep != std::wstring::npos) {
                    std::wstring nombreCarpetaRaiz = rutaRelativaW.substr(0, primerSep);
                    if (!nombreCarpetaRaiz.empty()) {
                        auto& elem = elementosPrimerNivel[nombreCarpetaRaiz];
                        elem.nombre = nombreCarpetaRaiz;
                        elem.rutaCompleta = nombreCarpetaRaiz;
                        elem.esDirectorio = true;
                        elem.nivelJerarquia = 0;
                        elem.tamanoBytes += cabeceraLocal.tamanoDescomprimido;
                    }
                } else {
                    auto& elem = elementosPrimerNivel[rutaRelativaW];
                    elem.nombre = rutaRelativaW;
                    elem.rutaCompleta = rutaRelativaW;
                    elem.esDirectorio = false;
                    elem.nivelJerarquia = 0;
                    elem.tamanoBytes = cabeceraLocal.tamanoDescomprimido;
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

    std::vector<ElementoArbolComprimido> listaCarpetas;
    std::vector<ElementoArbolComprimido> listaArchivos;
    for (const auto& [clave, elem] : elementosPrimerNivel) {
        if (elem.esDirectorio) {
            listaCarpetas.push_back(elem);
        } else {
            listaArchivos.push_back(elem);
        }
    }

    std::sort(listaCarpetas.begin(), listaCarpetas.end(), [](const auto& a, const auto& b) {
        return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
    });
    std::sort(listaArchivos.begin(), listaArchivos.end(), [](const auto& a, const auto& b) {
        return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
    });

    std::vector<ElementoArbolComprimido> listaFinal;
    listaFinal.insert(listaFinal.end(), listaCarpetas.begin(), listaCarpetas.end());
    listaFinal.insert(listaFinal.end(), listaArchivos.begin(), listaArchivos.end());

    estadoComprimidos.elementosArbol = listaFinal;
}

inline bool cargarComprimidoPorIndice(size_t indiceComprimido) {
    if (indiceComprimido >= estadoComprimidos.archivosSeleccionados.size()) return false;
    estadoComprimidos.indiceArchivoActual = indiceComprimido;
    const std::wstring& ruta = estadoComprimidos.archivosSeleccionados[indiceComprimido];

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
                agregarLineaRegistro(
                    L"Programa recordado: " + nombreEjecutable);
            } else {
                estadoComprimidos.programaTemporal = rutaSeleccionada;
                agregarLineaRegistro(
                    L"Programa temporal seleccionado: " + nombreEjecutable);
            }
        }
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

    std::wstring nombreEjecutable = PathFindFileNameW(rutaPrograma.c_str());
    size_t archivosAbiertos = 0;

    const auto& listaArchivos = estadoComprimidos.archivosSeleccionados;
    for (const auto& ruta : listaArchivos) {
        HINSTANCE resultadoEjecucion = ShellExecuteW(
            nullptr, L"open", rutaPrograma.c_str(),
            (L"\"" + ruta + L"\"").c_str(), nullptr,
            SW_SHOWNORMAL);
        if (reinterpret_cast<INT_PTR>(resultadoEjecucion) > 32) {
            archivosAbiertos++;
        }
    }

    if (archivosAbiertos > 0) {
        std::wstring tipoEtiqueta = estadoComprimidos.tieneComicsSeleccionados
                                        ? L" cómic(s)"
                                        : L" archivo(s)";
        agregarLineaRegistro(
            L"Abriendo " + std::to_wstring(archivosAbiertos) +
            tipoEtiqueta + L" con " + nombreEjecutable + L"...");
    } else {
        agregarLineaRegistro(
            L"No se encontraron archivos válidos para abrir.");
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

    agregarLineaRegistro(std::to_wstring(listaRutas.size()) +
                         L" archivo(s) comprimido(s) seleccionado(s)");

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
    const wchar_t* etiquetaFormato =
        FORMATOS_DESTINO[estadoComprimidos.indiceFormatoDestino];

    agregarLineaRegistro(L"Iniciando normalización a " +
                         std::wstring(etiquetaFormato) + L"...");

    for (const ElementoComprimido& elemento :
         estadoComprimidos.elementosSeleccionados) {
        wchar_t rutaDestino[MAX_PATH];
        wcsncpy_s(rutaDestino, elemento.rutaArchivo.c_str(), _TRUNCATE);
        PathRenameExtensionW(rutaDestino, extensionDestino);

        if (lstrcmpiW(elemento.rutaArchivo.c_str(), rutaDestino) == 0) {
            agregarLineaRegistro(elemento.nombreArchivo +
                                 L" ya tiene el formato de destino");
            continue;
        }

        BOOL resultadoCopia =
            CopyFileW(elemento.rutaArchivo.c_str(), rutaDestino, FALSE);
        if (resultadoCopia) {
            std::wstring nombreFinal = PathFindFileNameW(rutaDestino);
            agregarLineaRegistro(elemento.nombreArchivo + L" -> " +
                                 nombreFinal + L" [Completado]");
        } else {
            agregarLineaRegistro(elemento.nombreArchivo +
                                 L" [Error de conversión]");
        }
    }

    agregarLineaRegistro(L"Conversión finalizada con éxito.");
    estadoComprimidos.conversionEnCurso = false;
}

inline void desplazarScrollLog(int deltaLineas) {
    int lineasTotales =
        static_cast<int>(estadoComprimidos.lineasRegistro.size());
    estadoComprimidos.desplazamientoScrollLog = std::max(
        0, std::min(estadoComprimidos.desplazamientoScrollLog + deltaLineas,
                    lineasTotales - 1));
}

