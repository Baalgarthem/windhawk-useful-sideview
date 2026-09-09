#pragma once

#include <shlobj.h>

struct ElementoArbolVistaCarpetas {
    std::wstring nombre;
    std::wstring rutaCompleta;
    bool esDirectorio{false};
    uintmax_t tamanoBytes{0};
    int nivelJerarquia{0};
    int cantidadSubElementos{0};
};

struct SubcarpetaPesada {
    std::wstring nombre;
    uintmax_t tamanoBytes{0};
    double porcentaje{0.0};
};

struct ResumenVistaVacia {
    std::wstring rutaCarpeta;
    std::wstring nombreCarpeta;
    uintmax_t tamanoTotalBytes{0};
    int totalSubcarpetas{0};
    int totalArchivos{0};
    int cantidadTiposArchivos{0};
    std::wstring textoExtensionesFantasma;
    std::wstring fechaUltimoAcceso;
    FILETIME ftUltimaModificacion{};
    std::vector<SubcarpetaPesada> top4CarpetasMasPesadas;
    std::vector<std::wstring> carpetasVacias;
    RECT botonEliminarCarpetasVacias{};
    bool hoverBotonEliminarCarpetasVacias{false};
    std::wstring mensajeResultadoEliminacion;
    bool esUbicacionSistema{false};
    std::wstring nombreUbicacionSistema;
    std::wstring mensajeUbicacionSistema;
    bool calculado{false};
    bool calculando{false};
};

struct EstadoControladorVistaCarpetas {
    std::vector<std::wstring> carpetasSeleccionadas;
    size_t indiceCarpetaActual{0};
    std::vector<ElementoArbolVistaCarpetas> elementosArbol;
    std::wstring rutaCarpetaArbolCargada;
    FILETIME ftUltimaModificacionArbol{};
    int desplazamientoScrollY{0};
    int altoTotalArbol{0};
    int nivelExpansionActual{0}; // 0 = Raíz, 1 = Nivel 1, 2 = Nivel 2, ...
    RECT botonAlternarExpansion{};
    bool hoverBotonAlternarExpansion{false};

    // Metadatos de tamano total de carpeta
    std::wstring tamanoTotalCarpetaActualFormateado;
    uintmax_t tamanoTotalCarpetaActualBytes{0};

    // Navegacion entre carpetas seleccionadas
    RECT botonAnteriorCarpeta{};
    RECT botonSiguienteCarpeta{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};

    // Seccion Abrir con para carpetas
    std::vector<std::wstring> programasCarpetasRecordados;
    int indiceProgramaSeleccionado{0};
    bool menuProgramasAbierto{false};
    int opcionProgramaHover{-1};
    std::wstring programaTemporal;
    RECT botonAbrirCon{};
    RECT botonSelectorPrograma{};
    RECT botonEjecutarAbrir{};
    std::vector<RECT> areasOpcionesPrograma;
    RECT areaCajaArbol{};

    // Seccion Operaciones con carpeta
    RECT botonFusionCarpetas{};
    bool hoverBotonFusion{false};

    // Contenedor de acciones rápidas de carpeta
    RECT areaContenedorAccionesRapidas{};
    RECT botonCrearEnlaceSimbolico{};
    RECT botonCopiarContenidoPortapapeles{};
    bool hoverBotonCrearEnlace{false};
    bool hoverBotonCopiarContenido{false};

    // Vista vacia cuando no hay nada seleccionado en el Explorador
    ResumenVistaVacia resumenVistaVacia;
};

inline EstadoControladorVistaCarpetas estadoVistaCarpetas;

namespace {

inline std::wstring formatearTamanoCarpeta(uintmax_t tamanoEnBytes) {
    if (tamanoEnBytes < 1024) {
        return std::to_wstring(tamanoEnBytes) + L" B";
    }
    if (tamanoEnBytes < 1024 * 1024) {
        wchar_t bufer[32];
        swprintf_s(bufer, L"%.1f KB", static_cast<double>(tamanoEnBytes) / 1024.0);
        return bufer;
    }
    if (tamanoEnBytes < 1024ULL * 1024 * 1024) {
        wchar_t bufer[32];
        swprintf_s(bufer, L"%.1f MB", static_cast<double>(tamanoEnBytes) / (1024.0 * 1024.0));
        return bufer;
    }
    if (tamanoEnBytes < 1024ULL * 1024 * 1024 * 1024) {
        wchar_t bufer[32];
        swprintf_s(bufer, L"%.2f GB", static_cast<double>(tamanoEnBytes) / (1024.0 * 1024.0 * 1024.0));
        return bufer;
    }
    wchar_t bufer[32];
    swprintf_s(bufer, L"%.2f TB", static_cast<double>(tamanoEnBytes) / (1024.0 * 1024.0 * 1024.0 * 1024.0));
    return bufer;
}

inline uintmax_t calcularTamanoDirectorio(const std::wstring& rutaDirectorio, int limiteProfundidad = 64, int* contadorArchivosEscaneados = nullptr) {
    if (limiteProfundidad <= 0) return 0;
    if (contadorArchivosEscaneados && *contadorArchivosEscaneados >= 50000) return 0;

    std::wstring patron = rutaDirectorio + L"\\*";
    WIN32_FIND_DATAW datos;
    HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
    if (busqueda == INVALID_HANDLE_VALUE) return 0;

    uintmax_t tamanoTotal = 0;
    do {
        if (wcscmp(datos.cFileName, L".") == 0 || wcscmp(datos.cFileName, L"..") == 0) {
            continue;
        }
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            continue;
        }
        if (contadorArchivosEscaneados) {
            (*contadorArchivosEscaneados)++;
            if (*contadorArchivosEscaneados >= 50000) break;
        }
        std::wstring rutaHijo = rutaDirectorio + L"\\" + datos.cFileName;
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            tamanoTotal += calcularTamanoDirectorio(rutaHijo, limiteProfundidad - 1, contadorArchivosEscaneados);
        } else {
            uintmax_t tamanoArchivo = (static_cast<uintmax_t>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
            tamanoTotal += tamanoArchivo;
        }
    } while (FindNextFileW(busqueda, &datos));
    FindClose(busqueda);
    return tamanoTotal;
}

}  // namespace

inline void liberarRecursosVistaCarpetas() {
    estadoVistaCarpetas.carpetasSeleccionadas.clear();
    estadoVistaCarpetas.indiceCarpetaActual = 0;
    estadoVistaCarpetas.elementosArbol.clear();
    estadoVistaCarpetas.rutaCarpetaArbolCargada.clear();
    estadoVistaCarpetas.ftUltimaModificacionArbol = {};
    estadoVistaCarpetas.tamanoTotalCarpetaActualBytes = 0;
    estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado.clear();
    estadoVistaCarpetas.desplazamientoScrollY = 0;
    estadoVistaCarpetas.altoTotalArbol = 0;
    estadoVistaCarpetas.nivelExpansionActual = 0;
    estadoVistaCarpetas.hoverBotonAnterior = false;
    estadoVistaCarpetas.hoverBotonSiguiente = false;
    estadoVistaCarpetas.hoverBotonAlternarExpansion = false;
    estadoVistaCarpetas.menuProgramasAbierto = false;
    estadoVistaCarpetas.opcionProgramaHover = -1;
    estadoVistaCarpetas.programaTemporal.clear();
    estadoVistaCarpetas.areaContenedorAccionesRapidas = {};
    estadoVistaCarpetas.botonCrearEnlaceSimbolico = {};
    estadoVistaCarpetas.botonCopiarContenidoPortapapeles = {};
    estadoVistaCarpetas.hoverBotonCrearEnlace = false;
    estadoVistaCarpetas.hoverBotonCopiarContenido = false;
    estadoVistaCarpetas.resumenVistaVacia = {};
}

inline int obtenerMaximoNivelJerarquiaArbol() {
    int maxNivel = 0;
    for (const auto& elementoArbol : estadoVistaCarpetas.elementosArbol) {
        if (elementoArbol.nivelJerarquia > maxNivel) {
            maxNivel = elementoArbol.nivelJerarquia;
        }
    }
    return maxNivel;
}

inline void avanzarNivelExpansionArbol() {
    int maxNivel = obtenerMaximoNivelJerarquiaArbol();
    if (maxNivel <= 0) {
        estadoVistaCarpetas.nivelExpansionActual = 0;
        return;
    }
    if (estadoVistaCarpetas.nivelExpansionActual >= maxNivel) {
        estadoVistaCarpetas.nivelExpansionActual = 0;
    } else {
        estadoVistaCarpetas.nivelExpansionActual++;
    }
}

inline void inspeccionarEstructuraCarpeta(const std::wstring& rutaCarpeta, int nivel = 0, int maxNivel = 3) {
    if (nivel > maxNivel) return;
    if (estadoVistaCarpetas.elementosArbol.size() > 500) return;

    std::wstring patron = rutaCarpeta + L"\\*";
    WIN32_FIND_DATAW datos;
    HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
    if (busqueda == INVALID_HANDLE_VALUE) return;

    std::vector<ElementoArbolVistaCarpetas> subcarpetas;
    std::vector<ElementoArbolVistaCarpetas> archivos;

    do {
        if (wcscmp(datos.cFileName, L".") == 0 || wcscmp(datos.cFileName, L"..") == 0) {
            continue;
        }
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            continue;
        }

        std::wstring rutaCompleta = rutaCarpeta + L"\\" + datos.cFileName;
        bool esDir = (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        ElementoArbolVistaCarpetas elemento;
        elemento.nombre = datos.cFileName;
        elemento.rutaCompleta = rutaCompleta;
        elemento.esDirectorio = esDir;
        elemento.nivelJerarquia = nivel;

        if (esDir) {
            elemento.tamanoBytes = calcularTamanoDirectorio(rutaCompleta, 1);
            subcarpetas.push_back(elemento);
        } else {
            elemento.tamanoBytes = (static_cast<uintmax_t>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
            archivos.push_back(elemento);
        }
    } while (FindNextFileW(busqueda, &datos));
    FindClose(busqueda);

    std::sort(subcarpetas.begin(), subcarpetas.end(), [](const ElementoArbolVistaCarpetas& a, const ElementoArbolVistaCarpetas& b) {
        return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
    });
    std::sort(archivos.begin(), archivos.end(), [](const ElementoArbolVistaCarpetas& a, const ElementoArbolVistaCarpetas& b) {
        return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
    });

    for (const auto& directorioHijo : subcarpetas) {
        estadoVistaCarpetas.elementosArbol.push_back(directorioHijo);
        if (nivel < maxNivel && estadoVistaCarpetas.elementosArbol.size() < 500) {
            inspeccionarEstructuraCarpeta(directorioHijo.rutaCompleta, nivel + 1, maxNivel);
        }
    }
    for (const auto& archivoHijo : archivos) {
        if (estadoVistaCarpetas.elementosArbol.size() < 500) {
            estadoVistaCarpetas.elementosArbol.push_back(archivoHijo);
        }
    }
}

inline bool cargarCarpetaPorIndice(size_t indiceCarpeta) {
    if (indiceCarpeta >= estadoVistaCarpetas.carpetasSeleccionadas.size()) return false;
    estadoVistaCarpetas.indiceCarpetaActual = indiceCarpeta;

    const std::wstring& ruta = estadoVistaCarpetas.carpetasSeleccionadas[indiceCarpeta];
    WIN32_FILE_ATTRIBUTE_DATA datosAttr;
    bool tieneAttr = GetFileAttributesExW(ruta.c_str(), GetFileExInfoStandard, &datosAttr) != FALSE;

    if (tieneAttr && estadoVistaCarpetas.rutaCarpetaArbolCargada == ruta &&
        !estadoVistaCarpetas.elementosArbol.empty() &&
        memcmp(&datosAttr.ftLastWriteTime, &estadoVistaCarpetas.ftUltimaModificacionArbol, sizeof(FILETIME)) == 0) {
        return true;
    }

    estadoVistaCarpetas.elementosArbol.clear();
    estadoVistaCarpetas.desplazamientoScrollY = 0;
    estadoVistaCarpetas.nivelExpansionActual = 0;
    estadoVistaCarpetas.rutaCarpetaArbolCargada = ruta;
    if (tieneAttr) {
        estadoVistaCarpetas.ftUltimaModificacionArbol = datosAttr.ftLastWriteTime;
    }

    estadoVistaCarpetas.tamanoTotalCarpetaActualBytes = calcularTamanoDirectorio(ruta, 4);
    estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado =
        formatearTamanoCarpeta(estadoVistaCarpetas.tamanoTotalCarpetaActualBytes);

    inspeccionarEstructuraCarpeta(ruta, 0, 3);
    return true;
}

inline std::vector<std::wstring> obtenerSubcarpetasEnMismaCarpeta(const std::wstring& rutaCarpetaActual) {
    if (rutaCarpetaActual.empty()) return {};

    wchar_t buferDirectorio[MAX_PATH];
    wcsncpy_s(buferDirectorio, rutaCarpetaActual.c_str(), _TRUNCATE);
    PathRemoveFileSpecW(buferDirectorio);

    std::wstring patronBusqueda = std::wstring(buferDirectorio) + L"\\*";
    WIN32_FIND_DATAW datos;
    HANDLE busqueda = FindFirstFileW(patronBusqueda.c_str(), &datos);
    if (busqueda == INVALID_HANDLE_VALUE) return {};

    std::vector<std::wstring> listaCarpetas;
    do {
        if ((datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            wcscmp(datos.cFileName, L".") != 0 && wcscmp(datos.cFileName, L"..") != 0) {
            std::wstring rutaCompleta = std::wstring(buferDirectorio) + L"\\" + datos.cFileName;
            listaCarpetas.push_back(rutaCompleta);
        }
    } while (FindNextFileW(busqueda, &datos));
    FindClose(busqueda);

    std::sort(listaCarpetas.begin(), listaCarpetas.end(),
              [](const std::wstring& a, const std::wstring& b) {
                  return StrCmpLogicalW(a.c_str(), b.c_str()) < 0;
              });

    return listaCarpetas;
}

inline bool mostrarCarpetaAnterior() {
    if (estadoVistaCarpetas.carpetasSeleccionadas.size() > 1) {
        if (estadoVistaCarpetas.indiceCarpetaActual == 0) {
            estadoVistaCarpetas.indiceCarpetaActual = estadoVistaCarpetas.carpetasSeleccionadas.size() - 1;
        } else {
            estadoVistaCarpetas.indiceCarpetaActual--;
        }
        return cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
    }
    if (estadoVistaCarpetas.rutaCarpetaArbolCargada.empty()) return false;
    std::vector<std::wstring> listaVecinas =
        obtenerSubcarpetasEnMismaCarpeta(estadoVistaCarpetas.rutaCarpetaArbolCargada);
    if (listaVecinas.empty()) return false;
    auto it = std::find(listaVecinas.begin(), listaVecinas.end(),
                        estadoVistaCarpetas.rutaCarpetaArbolCargada);
    std::wstring destino;
    if (it == listaVecinas.end() || it == listaVecinas.begin()) {
        destino = listaVecinas.back();
    } else {
        destino = *(it - 1);
    }
    if (destino.empty() || destino == estadoVistaCarpetas.rutaCarpetaArbolCargada) return false;
    estadoVistaCarpetas.carpetasSeleccionadas = { destino };
    estadoVistaCarpetas.indiceCarpetaActual = 0;
    return cargarCarpetaPorIndice(0);
}

inline bool mostrarCarpetaSiguiente() {
    if (estadoVistaCarpetas.carpetasSeleccionadas.size() > 1) {
        estadoVistaCarpetas.indiceCarpetaActual =
            (estadoVistaCarpetas.indiceCarpetaActual + 1) % estadoVistaCarpetas.carpetasSeleccionadas.size();
        return cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
    }
    if (estadoVistaCarpetas.rutaCarpetaArbolCargada.empty()) return false;
    std::vector<std::wstring> listaVecinas =
        obtenerSubcarpetasEnMismaCarpeta(estadoVistaCarpetas.rutaCarpetaArbolCargada);
    if (listaVecinas.empty()) return false;
    auto it = std::find(listaVecinas.begin(), listaVecinas.end(),
                        estadoVistaCarpetas.rutaCarpetaArbolCargada);
    std::wstring destino;
    if (it == listaVecinas.end() || it + 1 == listaVecinas.end()) {
        destino = listaVecinas.front();
    } else {
        destino = *(it + 1);
    }
    if (destino.empty() || destino == estadoVistaCarpetas.rutaCarpetaArbolCargada) return false;
    estadoVistaCarpetas.carpetasSeleccionadas = { destino };
    estadoVistaCarpetas.indiceCarpetaActual = 0;
    return cargarCarpetaPorIndice(0);
}

inline void calcularResumenVistaVacia(const std::wstring& rutaCarpeta) {
    if (rutaCarpeta.empty()) {
        estadoVistaCarpetas.resumenVistaVacia = {};
        estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema = true;
        estadoVistaCarpetas.resumenVistaVacia.nombreUbicacionSistema = L"Ubicación del Sistema";
        estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
            L"Esta es una sección del sistema de Windows (Inicio, Galería o Este equipo). Las herramientas de vista previa se activarán automáticamente al navegar por carpetas y archivos locales.";
        estadoVistaCarpetas.resumenVistaVacia.calculado = true;
        return;
    }

    if (rutaCarpeta.rfind(L"::SISTEMA::", 0) == 0 || rutaCarpeta.rfind(L"::{", 0) == 0 || rutaCarpeta.rfind(L"shell:::", 0) == 0) {
        std::wstring nombreLimpio;
        if (rutaCarpeta.rfind(L"::SISTEMA::", 0) == 0) {
            nombreLimpio = rutaCarpeta.substr(11);
        } else {
            std::wstring min = rutaCarpeta;
            std::transform(min.begin(), min.end(), min.begin(), ::towlower);
            if (min.find(L"e88865ea-0e1c-4e20-9aa6-ed353b7c4777") != std::wstring::npos) nombreLimpio = L"Galería";
            else if (min.find(L"f874310e-b6b7-47dc-bc84-b9e6b38f5903") != std::wstring::npos) nombreLimpio = L"Inicio";
            else if (min.find(L"20d04fe0-3aea-1069-a2d8-08002b30309d") != std::wstring::npos) nombreLimpio = L"Este equipo";
            else if (min.find(L"645ff040-5081-101b-9f08-00aa002f954e") != std::wstring::npos) nombreLimpio = L"Papelera de reciclaje";
            else if (min.find(L"f02c1a0d-be21-4350-88b0-7367fc96ef3c") != std::wstring::npos) nombreLimpio = L"Red";
            else if (min.find(L"26ee0668-a00a-44d7-9371-beb064c98683") != std::wstring::npos) nombreLimpio = L"Panel de control";
            else nombreLimpio = L"Ubicación del Sistema";
        }

        if (nombreLimpio.find(L'{') != std::wstring::npos || nombreLimpio.find(L"::") != std::wstring::npos) {
            nombreLimpio = L"Ubicación del Sistema";
        }

        estadoVistaCarpetas.resumenVistaVacia = {};
        estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema = true;
        estadoVistaCarpetas.resumenVistaVacia.nombreUbicacionSistema = nombreLimpio;
        if (nombreLimpio == L"Galería" || nombreLimpio == L"Gallery") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"La Galería es una vista agregada de imágenes del sistema y dispositivos vinculados. La vista previa interactiva se activará cuando navegues por carpetas o selecciones imágenes locales.";
        } else if (nombreLimpio == L"Inicio" || nombreLimpio == L"Home") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"La sección Inicio contiene accesos rápidos y archivos recientes. El visor se activará en cuanto selecciones un archivo o ingreses a una carpeta local.";
        } else if (nombreLimpio == L"Este equipo" || nombreLimpio == L"This PC") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"En 'Este equipo' se muestran las unidades y dispositivos del sistema. Ingresa a cualquier unidad o carpeta local para visualizar su estructura y contenidos.";
        } else if (nombreLimpio == L"Papelera de reciclaje" || nombreLimpio == L"Recycle Bin") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"La Papelera de reciclaje almacena temporalmente los archivos eliminados del sistema.";
        } else if (nombreLimpio == L"Red" || nombreLimpio == L"Network") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"En la sección Red se muestran los equipos y carpetas compartidas disponibles en la red local.";
        } else {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"Esta sección corresponde a una ubicación virtual del sistema. Las vistas previas avanzadas se activarán al navegar o seleccionar archivos locales.";
        }
        estadoVistaCarpetas.resumenVistaVacia.calculado = true;
        return;
    }

    WIN32_FILE_ATTRIBUTE_DATA datosAttr;
    bool tieneAttr = GetFileAttributesExW(rutaCarpeta.c_str(), GetFileExInfoStandard, &datosAttr) != FALSE;

    if (tieneAttr && estadoVistaCarpetas.resumenVistaVacia.calculado &&
        !estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema &&
        estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta == rutaCarpeta &&
        memcmp(&datosAttr.ftLastWriteTime, &estadoVistaCarpetas.resumenVistaVacia.ftUltimaModificacion, sizeof(FILETIME)) == 0) {
        return;
    }

    if (!tieneAttr || !(datosAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        estadoVistaCarpetas.resumenVistaVacia = {};
        estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema = true;
        estadoVistaCarpetas.resumenVistaVacia.nombreUbicacionSistema = L"Ubicación del Sistema";
        estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
            L"Ubicación especial o virtual del sistema. El visor se activará automáticamente al explorar carpetas y archivos locales.";
        estadoVistaCarpetas.resumenVistaVacia.calculado = true;
        return;
    }

    estadoVistaCarpetas.resumenVistaVacia = {};
    estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema = false;
    estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta = rutaCarpeta;
    estadoVistaCarpetas.resumenVistaVacia.nombreCarpeta = PathFindFileNameW(rutaCarpeta.c_str());
    estadoVistaCarpetas.resumenVistaVacia.calculando = true;

    if (tieneAttr) {
        estadoVistaCarpetas.resumenVistaVacia.ftUltimaModificacion = datosAttr.ftLastWriteTime;
        FILETIME ftLocal;
        FileTimeToLocalFileTime(&datosAttr.ftLastAccessTime, &ftLocal);
        SYSTEMTIME st;
        FileTimeToSystemTime(&ftLocal, &st);
        wchar_t buferFecha[64];
        swprintf_s(buferFecha, L"%02u/%02u/%04u %02u:%02u",
                   st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
        estadoVistaCarpetas.resumenVistaVacia.fechaUltimoAcceso = buferFecha;
    }

    std::wstring patron = rutaCarpeta + L"\\*";
    WIN32_FIND_DATAW datos;
    HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
    if (busqueda == INVALID_HANDLE_VALUE) {
        estadoVistaCarpetas.resumenVistaVacia.calculando = false;
        estadoVistaCarpetas.resumenVistaVacia.calculado = true;
        return;
    }

    uintmax_t tamanoTotal = 0;
    int totalCarpetas = 0;
    int totalArchivos = 0;
    std::map<std::wstring, int> extensiones;
    std::vector<SubcarpetaPesada> subcarpetas;
    std::vector<std::wstring> carpetasVaciasDetectadas;

    do {
        if (wcscmp(datos.cFileName, L".") == 0 || wcscmp(datos.cFileName, L"..") == 0) {
            continue;
        }
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            continue;
        }
        std::wstring rutaHijo = rutaCarpeta + L"\\" + datos.cFileName;
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            totalCarpetas++;
            if (PathIsDirectoryEmptyW(rutaHijo.c_str())) {
                carpetasVaciasDetectadas.push_back(datos.cFileName);
            }
            int contadorEscaneados = 0;
            uintmax_t tamSub = calcularTamanoDirectorio(rutaHijo, 4, &contadorEscaneados);
            tamanoTotal += tamSub;
            SubcarpetaPesada subcarpetaItem;
            subcarpetaItem.nombre = datos.cFileName;
            subcarpetaItem.tamanoBytes = tamSub;
            subcarpetas.push_back(subcarpetaItem);
        } else {
            totalArchivos++;
            uintmax_t tamanoArchivo = (static_cast<uintmax_t>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
            tamanoTotal += tamanoArchivo;
            const wchar_t* extensionArchivo = PathFindExtensionW(datos.cFileName);
            if (extensionArchivo && *extensionArchivo) {
                std::wstring extensionMinusculas = extensionArchivo;
                std::transform(extensionMinusculas.begin(), extensionMinusculas.end(), extensionMinusculas.begin(), ::towlower);
                extensiones[extensionMinusculas]++;
            } else {
                extensiones[L"(sin extension)"]++;
            }
        }
    } while (FindNextFileW(busqueda, &datos));
    FindClose(busqueda);

    std::vector<SubcarpetaPesada> subcarpetasOrdenadas = subcarpetas;
    std::sort(subcarpetasOrdenadas.begin(), subcarpetasOrdenadas.end(), [](const SubcarpetaPesada& primera, const SubcarpetaPesada& segunda) {
        return primera.tamanoBytes > segunda.tamanoBytes;
    });

    std::vector<SubcarpetaPesada> top4;
    for (size_t posicionSubcarpeta = 0; posicionSubcarpeta < std::min<size_t>(4, subcarpetasOrdenadas.size()); ++posicionSubcarpeta) {
        SubcarpetaPesada subcarpetaItem = subcarpetasOrdenadas[posicionSubcarpeta];
        subcarpetaItem.porcentaje = (tamanoTotal > 0) ? (static_cast<double>(subcarpetaItem.tamanoBytes) / static_cast<double>(tamanoTotal)) : 0.0;
        top4.push_back(subcarpetaItem);
    }

    std::vector<std::pair<int, std::wstring>> listaOrdenadaExtensiones;
    for (const auto& [extensionClave, cantidadArchivos] : extensiones) {
        listaOrdenadaExtensiones.push_back({cantidadArchivos, extensionClave});
    }
    std::sort(listaOrdenadaExtensiones.begin(), listaOrdenadaExtensiones.end(),
              [](const std::pair<int, std::wstring>& primera, const std::pair<int, std::wstring>& segunda) {
                  if (primera.first != segunda.first) {
                      return primera.first > segunda.first;
                  }
                  return StrCmpLogicalW(primera.second.c_str(), segunda.second.c_str()) < 0;
              });

    std::wstring textoExtensionesAcumulado;
    for (size_t posicionExtension = 0; posicionExtension < listaOrdenadaExtensiones.size(); ++posicionExtension) {
        if (posicionExtension > 0) {
            textoExtensionesAcumulado += L", ";
        }
        textoExtensionesAcumulado += listaOrdenadaExtensiones[posicionExtension].second;
    }

    estadoVistaCarpetas.resumenVistaVacia.tamanoTotalBytes = tamanoTotal;
    estadoVistaCarpetas.resumenVistaVacia.totalSubcarpetas = totalCarpetas;
    estadoVistaCarpetas.resumenVistaVacia.totalArchivos = totalArchivos;
    estadoVistaCarpetas.resumenVistaVacia.cantidadTiposArchivos = static_cast<int>(extensiones.size());
    estadoVistaCarpetas.resumenVistaVacia.textoExtensionesFantasma = textoExtensionesAcumulado;
    estadoVistaCarpetas.resumenVistaVacia.top4CarpetasMasPesadas = top4;
    estadoVistaCarpetas.resumenVistaVacia.carpetasVacias = carpetasVaciasDetectadas;
    estadoVistaCarpetas.resumenVistaVacia.calculando = false;
    estadoVistaCarpetas.resumenVistaVacia.calculado = true;
}

inline bool eliminarCarpetasVacias() {
    if (estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta.empty() ||
        estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.empty()) {
        return false;
    }

    int eliminadas = 0;
    const std::wstring& rutaBase = estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta;
    for (const auto& nombreCarpeta : estadoVistaCarpetas.resumenVistaVacia.carpetasVacias) {
        std::wstring rutaCompleta = rutaBase + L"\\" + nombreCarpeta;
        if (PathIsDirectoryEmptyW(rutaCompleta.c_str())) {
            if (RemoveDirectoryW(rutaCompleta.c_str())) {
                eliminadas++;
            }
        }
    }

    std::wstring msgResultado;
    if (eliminadas > 0) {
        msgResultado = L"✓ Se eliminaron " + std::to_wstring(eliminadas) + L" carpetas vacías exitosamente.";
    }

    std::wstring rutaActual = rutaBase;
    estadoVistaCarpetas.resumenVistaVacia.calculado = false;
    calcularResumenVistaVacia(rutaActual);
    estadoVistaCarpetas.resumenVistaVacia.mensajeResultadoEliminacion = msgResultado;
    return true;
}

inline void seleccionarYAgregarProgramaCarpeta(HWND ventanaPadre) {
    wchar_t buferRutaArchivo[MAX_PATH] = L"";
    OPENFILENAMEW estructuraOfn{};
    estructuraOfn.lStructSize = sizeof(estructuraOfn);
    estructuraOfn.hwndOwner = ventanaPadre;
    estructuraOfn.lpstrFilter = L"Archivos Ejecutables (*.exe)\0*.exe\0Todos los archivos (*.*)\0*.*\0";
    estructuraOfn.lpstrFile = buferRutaArchivo;
    estructuraOfn.nMaxFile = MAX_PATH;
    estructuraOfn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    estructuraOfn.lpstrTitle = L"Seleccionar programa para abrir carpetas";

    if (GetOpenFileNameW(&estructuraOfn)) {
        std::wstring rutaSeleccionada(buferRutaArchivo);
        if (!rutaSeleccionada.empty()) {
            std::wstring nombreEjecutable = PathFindFileNameW(rutaSeleccionada.c_str());
            std::wstring mensaje = L"¿Desea recordar '" + nombreEjecutable + L"' en la lista de opciones para carpetas?";
            int res = MessageBoxW(ventanaPadre, mensaje.c_str(), L"Recordar programa", MB_YESNO | MB_ICONQUESTION);
            if (res == IDYES) {
                auto it = std::find(estadoVistaCarpetas.programasCarpetasRecordados.begin(),
                                    estadoVistaCarpetas.programasCarpetasRecordados.end(), rutaSeleccionada);
                if (it == estadoVistaCarpetas.programasCarpetasRecordados.end()) {
                    estadoVistaCarpetas.programasCarpetasRecordados.push_back(rutaSeleccionada);
                    estadoVistaCarpetas.indiceProgramaSeleccionado =
                        static_cast<int>(estadoVistaCarpetas.programasCarpetasRecordados.size() - 1);
                } else {
                    estadoVistaCarpetas.indiceProgramaSeleccionado =
                        static_cast<int>(std::distance(estadoVistaCarpetas.programasCarpetasRecordados.begin(), it));
                }
                estadoVistaCarpetas.programaTemporal.clear();
            } else {
                estadoVistaCarpetas.programaTemporal = rutaSeleccionada;
            }
        }
    }
}
inline std::wstring resolverProgramaParaCarpetas() {
    if (!estadoVistaCarpetas.programaTemporal.empty()) {
        return estadoVistaCarpetas.programaTemporal;
    }
    if (!estadoVistaCarpetas.programasCarpetasRecordados.empty() &&
        estadoVistaCarpetas.indiceProgramaSeleccionado >= 0 &&
        estadoVistaCarpetas.indiceProgramaSeleccionado <
            static_cast<int>(estadoVistaCarpetas.programasCarpetasRecordados.size())) {
        return estadoVistaCarpetas.programasCarpetasRecordados[static_cast<size_t>(
            estadoVistaCarpetas.indiceProgramaSeleccionado)];
    }
    if (!configuracion.programaCarpetasPorDefecto.empty()) {
        return configuracion.programaCarpetasPorDefecto;
    }
    if (estadoVistaCarpetas.programasCarpetasRecordados.size() == 1) {
        return estadoVistaCarpetas.programasCarpetasRecordados.front();
    }
    if (!estadoVistaCarpetas.programasCarpetasRecordados.empty()) {
        return estadoVistaCarpetas.programasCarpetasRecordados.front();
    }
    return {};
}

inline void mostrarMenuDesplegableProgramasCarpetas(HWND ventana, const RECT& botonRect) {
    if (estadoVistaCarpetas.programasCarpetasRecordados.empty() && estadoVistaCarpetas.programaTemporal.empty()) {
        seleccionarYAgregarProgramaCarpeta(ventana);
        InvalidateRect(ventana, nullptr, FALSE);
        return;
    }

    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    for (size_t posicionPrograma = 0; posicionPrograma < estadoVistaCarpetas.programasCarpetasRecordados.size(); ++posicionPrograma) {
        UINT banderasMenu = MF_STRING;
        if (static_cast<int>(posicionPrograma) == estadoVistaCarpetas.indiceProgramaSeleccionado) {
            banderasMenu |= MF_CHECKED;
        }
        std::wstring nombreProg = PathFindFileNameW(estadoVistaCarpetas.programasCarpetasRecordados[posicionPrograma].c_str());
        AppendMenuW(menu, banderasMenu, static_cast<UINT_PTR>(3000 + posicionPrograma), nombreProg.c_str());
    }

    if (!estadoVistaCarpetas.programasCarpetasRecordados.empty()) {
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    AppendMenuW(menu, MF_STRING, 3999, L"+ Añadir programa...");

    POINT puntoBoton{botonRect.left, botonRect.top};
    ClientToScreen(ventana, &puntoBoton);

    SetForegroundWindow(ventana);
    UINT comandoSeleccionado = static_cast<UINT>(TrackPopupMenuEx(
        menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_VERTICAL,
        puntoBoton.x, puntoBoton.y, ventana, nullptr));
    DestroyMenu(menu);

    if (comandoSeleccionado >= 3000 && comandoSeleccionado < 3000 + estadoVistaCarpetas.programasCarpetasRecordados.size()) {
        estadoVistaCarpetas.indiceProgramaSeleccionado = static_cast<int>(comandoSeleccionado - 3000);
        estadoVistaCarpetas.programaTemporal.clear();
        InvalidateRect(ventana, nullptr, FALSE);
    } else if (comandoSeleccionado == 3999) {
        seleccionarYAgregarProgramaCarpeta(ventana);
        InvalidateRect(ventana, nullptr, FALSE);
    }
}

inline void ejecutarAbrirCarpetasConPrograma(HWND ventanaPadre) {
    std::wstring rutaProg = resolverProgramaParaCarpetas();

    if (rutaProg.empty()) {
        seleccionarYAgregarProgramaCarpeta(ventanaPadre);
        rutaProg = resolverProgramaParaCarpetas();
        if (rutaProg.empty()) return;
    }

    for (const auto& rutaCarpeta : estadoVistaCarpetas.carpetasSeleccionadas) {
        ShellExecuteW(nullptr, L"open", rutaProg.c_str(),
                      (L"\"" + rutaCarpeta + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
    }
}

inline bool ejecutarCrearEnlaceSimbolico(HWND ventanaPadre) {
    if (estadoVistaCarpetas.carpetasSeleccionadas.empty() ||
        estadoVistaCarpetas.indiceCarpetaActual >= estadoVistaCarpetas.carpetasSeleccionadas.size()) {
        return false;
    }
    const std::wstring& rutaOrigen = estadoVistaCarpetas.carpetasSeleccionadas[estadoVistaCarpetas.indiceCarpetaActual];
    if (rutaOrigen.empty() || !PathFileExistsW(rutaOrigen.c_str())) return false;

    std::wstring nombreCarpetaOrigen = PathFindFileNameW(rutaOrigen.c_str());

    PunteroCom<IFileSaveDialog> dialogoGuardar;
    HRESULT resultadoCom = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER,
                                            IID_PPV_ARGS(dialogoGuardar.recibir()));
    if (FAILED(resultadoCom) || !dialogoGuardar.obtener()) {
        return false;
    }

    dialogoGuardar->SetTitle(L"Seleccionar ubicación para el enlace simbólico");
    dialogoGuardar->SetFileName(nombreCarpetaOrigen.c_str());
    COMDLG_FILTERSPEC filtroCom[] = {
        { L"Todos los archivos (*.*)", L"*.*" }
    };
    dialogoGuardar->SetFileTypes(1, filtroCom);

    if (SUCCEEDED(dialogoGuardar->Show(ventanaPadre))) {
        PunteroCom<IShellItem> elementoResultado;
        if (SUCCEEDED(dialogoGuardar->GetResult(elementoResultado.recibir())) && elementoResultado.obtener()) {
            PWSTR rutaElegida = nullptr;
            if (SUCCEEDED(elementoResultado->GetDisplayName(SIGDN_FILESYSPATH, &rutaElegida)) && rutaElegida) {
                std::wstring rutaDestinoEnlace(rutaElegida);
                CoTaskMemFree(rutaElegida);

                if (!rutaDestinoEnlace.empty()) {
                    BOOL enlaceCreado = CreateSymbolicLinkW(rutaDestinoEnlace.c_str(), rutaOrigen.c_str(),
                                                            SYMBOLIC_LINK_FLAG_DIRECTORY);
                    if (!enlaceCreado) {
                        enlaceCreado = CreateSymbolicLinkW(rutaDestinoEnlace.c_str(), rutaOrigen.c_str(),
                                                            SYMBOLIC_LINK_FLAG_DIRECTORY | 0x1);
                    }
                    if (!enlaceCreado) {
                        std::wstring rutaAccesoDirecto = rutaDestinoEnlace;
                        if (PathFindExtensionW(rutaAccesoDirecto.c_str()) == nullptr ||
                            lstrcmpiW(PathFindExtensionW(rutaAccesoDirecto.c_str()), L".lnk") != 0) {
                            rutaAccesoDirecto += L".lnk";
                        }
                        PunteroCom<IShellLinkW> accesoDirecto;
                        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(accesoDirecto.recibir())))) {
                            accesoDirecto->SetPath(rutaOrigen.c_str());
                            PunteroCom<IPersistFile> archivoPersistente;
                            if (SUCCEEDED(accesoDirecto->QueryInterface(IID_PPV_ARGS(archivoPersistente.recibir())))) {
                                if (SUCCEEDED(archivoPersistente->Save(rutaAccesoDirecto.c_str(), TRUE))) {
                                    enlaceCreado = TRUE;
                                    rutaDestinoEnlace = rutaAccesoDirecto;
                                }
                            }
                        }
                    }

                    if (enlaceCreado) {
                        std::wstring mensajeExito = L"✓ Enlace creado exitosamente en:\n" + rutaDestinoEnlace;
                        MessageBoxW(ventanaPadre, mensajeExito.c_str(), L"Crear Enlace", MB_OK | MB_ICONINFORMATION);
                        return true;
                    } else {
                        DWORD errorCodigo = GetLastError();
                        std::wstring mensajeError = L"No fue posible crear el enlace (error " + std::to_wstring(errorCodigo) + L").";
                        MessageBoxW(ventanaPadre, mensajeError.c_str(), L"Crear Enlace", MB_OK | MB_ICONERROR);
                    }
                }
            }
        }
    }
    return false;
}

inline bool ejecutarCopiarContenidoPortapapeles(HWND ventanaPadre) {
    if (estadoVistaCarpetas.carpetasSeleccionadas.empty() ||
        estadoVistaCarpetas.indiceCarpetaActual >= estadoVistaCarpetas.carpetasSeleccionadas.size()) {
        return false;
    }
    const std::wstring& rutaCarpetaOrigen = estadoVistaCarpetas.carpetasSeleccionadas[estadoVistaCarpetas.indiceCarpetaActual];
    if (rutaCarpetaOrigen.empty() || !PathFileExistsW(rutaCarpetaOrigen.c_str())) return false;

    std::wstring patronBusqueda = rutaCarpetaOrigen + L"\\*";
    WIN32_FIND_DATAW datosElemento{};
    HANDLE busqueda = FindFirstFileW(patronBusqueda.c_str(), &datosElemento);
    if (busqueda == INVALID_HANDLE_VALUE) return false;

    std::vector<std::wstring> subcarpetasRaiz;
    std::vector<std::pair<std::wstring, uintmax_t>> archivosRaiz;

    do {
        if (wcscmp(datosElemento.cFileName, L".") == 0 || wcscmp(datosElemento.cFileName, L"..") == 0) {
            continue;
        }
        if (datosElemento.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            subcarpetasRaiz.push_back(datosElemento.cFileName);
        } else {
            uintmax_t tamanoArchivo = (static_cast<uintmax_t>(datosElemento.nFileSizeHigh) << 32) | datosElemento.nFileSizeLow;
            archivosRaiz.push_back({ datosElemento.cFileName, tamanoArchivo });
        }
    } while (FindNextFileW(busqueda, &datosElemento));
    FindClose(busqueda);

    std::sort(subcarpetasRaiz.begin(), subcarpetasRaiz.end(), [](const std::wstring& primerDirectorio, const std::wstring& segundoDirectorio) {
        return StrCmpLogicalW(primerDirectorio.c_str(), segundoDirectorio.c_str()) < 0;
    });

    std::sort(archivosRaiz.begin(), archivosRaiz.end(), [](const std::pair<std::wstring, uintmax_t>& primerArchivo, const std::pair<std::wstring, uintmax_t>& segundoArchivo) {
        return StrCmpLogicalW(primerArchivo.first.c_str(), segundoArchivo.first.c_str()) < 0;
    });

    std::wstring textoAcumulado;
    textoAcumulado += L"Contenido de: ";
    textoAcumulado += PathFindFileNameW(rutaCarpetaOrigen.c_str());
    textoAcumulado += L"\r\n";
    textoAcumulado += L"Ruta: " + rutaCarpetaOrigen + L"\r\n";
    textoAcumulado += L"--------------------------------------------------\r\n";

    for (const auto& nombreSubcarpeta : subcarpetasRaiz) {
        textoAcumulado += L"📁 [Carpeta] " + nombreSubcarpeta + L"\r\n";
    }
    for (const auto& [nombreArchivo, tamanoBytes] : archivosRaiz) {
        textoAcumulado += L"📄 [Archivo] " + nombreArchivo + L" (" + formatearTamanoCarpeta(tamanoBytes) + L")\r\n";
    }

    textoAcumulado += L"--------------------------------------------------\r\n";
    textoAcumulado += L"Total: " + std::to_wstring(subcarpetasRaiz.size()) + L" carpetas, " +
                      std::to_wstring(archivosRaiz.size()) + L" archivos (" +
                      std::to_wstring(subcarpetasRaiz.size() + archivosRaiz.size()) + L" elementos)\r\n";

    if (!OpenClipboard(ventanaPadre)) {
        return false;
    }
    EmptyClipboard();

    size_t tamanoBufer = (textoAcumulado.size() + 1) * sizeof(wchar_t);
    HGLOBAL memoriaGlobal = GlobalAlloc(GMEM_MOVEABLE, tamanoBufer);
    if (memoriaGlobal) {
        wchar_t* punteroDestino = static_cast<wchar_t*>(GlobalLock(memoriaGlobal));
        if (punteroDestino) {
            memcpy(punteroDestino, textoAcumulado.c_str(), tamanoBufer);
            GlobalUnlock(memoriaGlobal);
            SetClipboardData(CF_UNICODETEXT, memoriaGlobal);
        }
    }
    CloseClipboard();

    MessageBoxW(ventanaPadre, L"✓ El contenido del directorio raíz ha sido copiado al portapapeles.",
                L"Copiar Contenido", MB_OK | MB_ICONINFORMATION);
    return true;
}

inline bool fusionarDirectorioRecursivo(const std::wstring& rutaOrigen, const std::wstring& rutaDestino) {
    CreateDirectoryW(rutaDestino.c_str(), nullptr);
    std::wstring patronBusqueda = rutaOrigen + L"\\*";
    WIN32_FIND_DATAW datosElemento{};
    HANDLE busquedaArchivos = FindFirstFileW(patronBusqueda.c_str(), &datosElemento);
    if (busquedaArchivos == INVALID_HANDLE_VALUE) return false;

    do {
        if (wcscmp(datosElemento.cFileName, L".") == 0 || wcscmp(datosElemento.cFileName, L"..") == 0) {
            continue;
        }
        std::wstring rutaElementoOrigen = rutaOrigen + L"\\" + datosElemento.cFileName;
        std::wstring rutaElementoDestino = rutaDestino + L"\\" + datosElemento.cFileName;

        if (datosElemento.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            fusionarDirectorioRecursivo(rutaElementoOrigen, rutaElementoDestino);
            RemoveDirectoryW(rutaElementoOrigen.c_str());
        } else {
            if (PathFileExistsW(rutaElementoDestino.c_str())) {
                const wchar_t* extensionPuntero = PathFindExtensionW(datosElemento.cFileName);
                std::wstring nombreSinExtension = datosElemento.cFileName;
                if (extensionPuntero && *extensionPuntero) {
                    nombreSinExtension = nombreSinExtension.substr(0, nombreSinExtension.length() - wcslen(extensionPuntero));
                }
                int sufijoIncremental = 2;
                std::wstring destinoAlternativo;
                do {
                    destinoAlternativo = rutaDestino + L"\\" + nombreSinExtension + L" (" +
                                         std::to_wstring(sufijoIncremental) + L")" +
                                         (extensionPuntero ? extensionPuntero : L"");
                    sufijoIncremental++;
                } while (PathFileExistsW(destinoAlternativo.c_str()));
                rutaElementoDestino = destinoAlternativo;
            }
            MoveFileExW(rutaElementoOrigen.c_str(), rutaElementoDestino.c_str(),
                       MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
        }
    } while (FindNextFileW(busquedaArchivos, &datosElemento));
    FindClose(busquedaArchivos);

    RemoveDirectoryW(rutaOrigen.c_str());
    return true;
}

struct ContextoDialogoNombreCarpeta {
    std::wstring nombreSugerido;
    std::wstring nombreResultado;
    bool aceptado{false};
    bool terminado{false};
    HWND controlTextoEdicion{nullptr};
    HWND controlBotonAceptar{nullptr};
    HWND controlBotonCancelar{nullptr};
    HFONT fuenteDialogo{nullptr};
};

inline LRESULT CALLBACK procesarVentanaDialogoNombreCarpeta(HWND ventana, UINT mensaje, WPARAM parametroW, LPARAM parametroL) {
    ContextoDialogoNombreCarpeta* contexto = reinterpret_cast<ContextoDialogoNombreCarpeta*>(
        GetWindowLongPtrW(ventana, GWLP_USERDATA));

    switch (mensaje) {
        case WM_CREATE: {
            CREATESTRUCTW* estructuraCreacion = reinterpret_cast<CREATESTRUCTW*>(parametroL);
            contexto = reinterpret_cast<ContextoDialogoNombreCarpeta*>(estructuraCreacion->lpCreateParams);
            SetWindowLongPtrW(ventana, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(contexto));

            contexto->fuenteDialogo = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                                  DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

            HWND etiquetaTexto = CreateWindowExW(0, L"STATIC", L"Introduzca el nombre de la carpeta final:",
                                                 WS_CHILD | WS_VISIBLE, 20, 16, 360, 20,
                                                 ventana, nullptr, nullptr, nullptr);
            SendMessageW(etiquetaTexto, WM_SETFONT, reinterpret_cast<WPARAM>(contexto->fuenteDialogo), TRUE);

            contexto->controlTextoEdicion = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", contexto->nombreSugerido.c_str(),
                                                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                                            20, 42, 360, 26,
                                                            ventana, reinterpret_cast<HMENU>(101), nullptr, nullptr);
            SendMessageW(contexto->controlTextoEdicion, WM_SETFONT, reinterpret_cast<WPARAM>(contexto->fuenteDialogo), TRUE);
            SendMessageW(contexto->controlTextoEdicion, EM_SETSEL, 0, -1);

            contexto->controlBotonAceptar = CreateWindowExW(0, L"BUTTON", L"Aceptar",
                                                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                                            175, 84, 95, 28,
                                                            ventana, reinterpret_cast<HMENU>(IDOK), nullptr, nullptr);
            SendMessageW(contexto->controlBotonAceptar, WM_SETFONT, reinterpret_cast<WPARAM>(contexto->fuenteDialogo), TRUE);

            contexto->controlBotonCancelar = CreateWindowExW(0, L"BUTTON", L"Cancelar",
                                                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                                             285, 84, 95, 28,
                                                             ventana, reinterpret_cast<HMENU>(IDCANCEL), nullptr, nullptr);
            SendMessageW(contexto->controlBotonCancelar, WM_SETFONT, reinterpret_cast<WPARAM>(contexto->fuenteDialogo), TRUE);

            SetFocus(contexto->controlTextoEdicion);
            return 0;
        }

        case WM_COMMAND: {
            WORD identificadorControl = LOWORD(parametroW);
            if (identificadorControl == IDOK) {
                if (contexto && contexto->controlTextoEdicion) {
                    wchar_t buferTexto[MAX_PATH];
                    GetWindowTextW(contexto->controlTextoEdicion, buferTexto, MAX_PATH);
                    std::wstring textoIngresado = buferTexto;

                    size_t primerNoEspacio = textoIngresado.find_first_not_of(L" \t\r\n");
                    if (primerNoEspacio != std::wstring::npos) {
                        size_t ultimoNoEspacio = textoIngresado.find_last_not_of(L" \t\r\n");
                        textoIngresado = textoIngresado.substr(primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);
                    } else {
                        textoIngresado.clear();
                    }

                    if (textoIngresado.empty()) {
                        MessageBoxW(ventana, L"El nombre de la carpeta no puede estar vacío.",
                                    L"Useful Sideview - Nombre Inválido", MB_ICONWARNING | MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
                        SetFocus(contexto->controlTextoEdicion);
                        return 0;
                    }

                    if (textoIngresado.find_first_of(L"\\/:*?\"<>|") != std::wstring::npos) {
                        MessageBoxW(ventana,
                                    L"El nombre no puede contener ninguno de los siguientes caracteres:\n\\ / : * ? \" < > |",
                                    L"Useful Sideview - Nombre Inválido", MB_ICONWARNING | MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
                        SetFocus(contexto->controlTextoEdicion);
                        return 0;
                    }

                    contexto->nombreResultado = textoIngresado;
                    contexto->aceptado = true;
                }
                DestroyWindow(ventana);
                return 0;
            }
            if (identificadorControl == IDCANCEL) {
                DestroyWindow(ventana);
                return 0;
            }
            break;
        }

        case WM_CLOSE: {
            DestroyWindow(ventana);
            return 0;
        }

        case WM_DESTROY: {
            if (contexto) {
                if (contexto->fuenteDialogo) {
                    DeleteObject(contexto->fuenteDialogo);
                    contexto->fuenteDialogo = nullptr;
                }
                contexto->terminado = true;
            }
            return 0;
        }

        default:
            break;
    }
    return DefWindowProcW(ventana, mensaje, parametroW, parametroL);
}

inline bool solicitarNombreCarpetaFinal(HWND ventanaPadre, const std::wstring& nombreSugerido, std::wstring& nombreElegido) {
    WNDCLASSEXW claseDialogo{};
    claseDialogo.cbSize = sizeof(WNDCLASSEXW);
    claseDialogo.lpfnWndProc = procesarVentanaDialogoNombreCarpeta;
    claseDialogo.hInstance = GetModuleHandleW(nullptr);
    claseDialogo.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    claseDialogo.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    claseDialogo.lpszClassName = L"UsefulSideviewNombreCarpetaDlg";
    RegisterClassExW(&claseDialogo);

    ContextoDialogoNombreCarpeta contexto;
    contexto.nombreSugerido = nombreSugerido;

    int anchoDialogo = 415;
    int altoDialogo = 160;
    RECT rectanguloPadre{};
    HWND ventanaFoco = GetForegroundWindow();
    if (ventanaFoco && IsWindow(ventanaFoco)) {
        GetWindowRect(ventanaFoco, &rectanguloPadre);
    } else if (ventanaPadre && IsWindow(ventanaPadre)) {
        GetWindowRect(ventanaPadre, &rectanguloPadre);
    } else {
        rectanguloPadre = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    }
    int posicionX = rectanguloPadre.left + (rectanguloPadre.right - rectanguloPadre.left - anchoDialogo) / 2;
    int posicionY = rectanguloPadre.top + (rectanguloPadre.bottom - rectanguloPadre.top - altoDialogo) / 2;

    HWND ventanaDialogo = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
        L"UsefulSideviewNombreCarpetaDlg",
        L"Useful Sideview - Elegir Nombre de Carpeta Final",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        posicionX, posicionY, anchoDialogo, altoDialogo,
        nullptr, nullptr, GetModuleHandleW(nullptr), &contexto
    );

    if (!ventanaDialogo) return false;

    SetForegroundWindow(ventanaDialogo);
    if (contexto.controlTextoEdicion) {
        SetFocus(contexto.controlTextoEdicion);
    }

    MSG mensajeBucle{};
    while (!contexto.terminado && IsWindow(ventanaDialogo)) {
        while (PeekMessageW(&mensajeBucle, nullptr, 0, 0, PM_REMOVE)) {
            if (mensajeBucle.message == WM_QUIT) {
                contexto.terminado = true;
                break;
            }
            if (!IsDialogMessageW(ventanaDialogo, &mensajeBucle)) {
                TranslateMessage(&mensajeBucle);
                DispatchMessageW(&mensajeBucle);
            }
        }
        if (!contexto.terminado && IsWindow(ventanaDialogo)) {
            MsgWaitForMultipleObjects(0, nullptr, FALSE, 10, QS_ALLINPUT);
        }
    }

    if (contexto.aceptado) {
        nombreElegido = contexto.nombreResultado;
        return true;
    }
    return false;
}

inline bool ejecutarFusionCarpetas(HWND ventanaPadre) {
    if (estadoVistaCarpetas.carpetasSeleccionadas.size() < 2) {
        return false;
    }

    wchar_t buferPadre[MAX_PATH];
    wcsncpy_s(buferPadre, estadoVistaCarpetas.carpetasSeleccionadas[0].c_str(), _TRUNCATE);
    PathRemoveFileSpecW(buferPadre);
    std::wstring rutaPadre = buferPadre;

    std::wstring nombreSugerido = PathFindFileNameW(estadoVistaCarpetas.carpetasSeleccionadas[0].c_str());
    std::wstring nombreElegido;

    if (!solicitarNombreCarpetaFinal(ventanaPadre, nombreSugerido, nombreElegido)) {
        return false;
    }

    std::wstring rutaDestino = rutaPadre + L"\\" + nombreElegido;
    size_t totalCarpetas = estadoVistaCarpetas.carpetasSeleccionadas.size();

    std::wstring mensajeConfirmacion =
        L"¿Desea fusionar las " + std::to_wstring(totalCarpetas) +
        L" carpetas seleccionadas en la carpeta de destino:\n📁 " +
        nombreElegido + L"\n(" + rutaDestino + L")?\n\n"
        L"Todos los archivos y subcarpetas se consolidarán en esta carpeta final.\n"
        L"Las carpetas de origen fusionadas se eliminarán tras mover su contenido.";

    int respuestaUsuario = MessageBoxW(
        nullptr,
        mensajeConfirmacion.c_str(),
        L"Useful Sideview - Confirmar Fusión de Carpetas",
        MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND
    );

    if (respuestaUsuario != IDYES) {
        return false;
    }

    CreateDirectoryW(rutaDestino.c_str(), nullptr);

    int carpetasFusionadasExitosamente = 0;
    for (size_t posicionCarpeta = 0; posicionCarpeta < estadoVistaCarpetas.carpetasSeleccionadas.size(); ++posicionCarpeta) {
        const std::wstring& rutaOrigen = estadoVistaCarpetas.carpetasSeleccionadas[posicionCarpeta];
        if (rutaOrigen != rutaDestino && PathFileExistsW(rutaOrigen.c_str())) {
            if (fusionarDirectorioRecursivo(rutaOrigen, rutaDestino)) {
                carpetasFusionadasExitosamente++;
            }
        }
    }

    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATHW, rutaPadre.c_str(), nullptr);
    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATHW, rutaDestino.c_str(), nullptr);

    estadoVistaCarpetas.carpetasSeleccionadas = { rutaDestino };
    estadoVistaCarpetas.rutaCarpetaArbolCargada.clear();
    estadoVistaCarpetas.indiceCarpetaActual = 0;
    cargarCarpetaPorIndice(0);

    std::wstring mensajeExito =
        L"✓ Fusión completada exitosamente. Se fusionaron " +
        std::to_wstring(carpetasFusionadasExitosamente) +
        L" carpetas en la carpeta final:\n📁 " + nombreElegido;

    MessageBoxW(
        nullptr,
        mensajeExito.c_str(),
        L"Useful Sideview - Fusión de Carpetas Exitosa",
        MB_ICONINFORMATION | MB_OK | MB_TOPMOST | MB_SETFOREGROUND
    );

    return true;
}

inline const ElementoArbolVistaCarpetas* obtenerElementoArbolEnPunto(POINT puntoRaton) {
    if (estadoVistaCarpetas.elementosArbol.empty()) return nullptr;
    if (!PtInRect(&estadoVistaCarpetas.areaCajaArbol, puntoRaton)) return nullptr;

    int yInicioArbol = estadoVistaCarpetas.areaCajaArbol.top + 8 - estadoVistaCarpetas.desplazamientoScrollY;
    constexpr int ALTO_FILA_ARBOL = 22;

    if (puntoRaton.y < yInicioArbol) return nullptr;

    int indiceElementoCalculado = (puntoRaton.y - yInicioArbol) / ALTO_FILA_ARBOL;
    if (indiceElementoCalculado >= 0 &&
        static_cast<size_t>(indiceElementoCalculado) < estadoVistaCarpetas.elementosArbol.size()) {
        return &estadoVistaCarpetas.elementosArbol[static_cast<size_t>(indiceElementoCalculado)];
    }
    return nullptr;
}

inline void ejecutarAbrirElementoArbol(const ElementoArbolVistaCarpetas& elemento) {
    if (elemento.rutaCompleta.empty()) return;
    HINSTANCE resultado = ShellExecuteW(nullptr, nullptr, elemento.rutaCompleta.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(resultado) <= 32) {
        ShellExecuteW(nullptr, L"open", elemento.rutaCompleta.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
}

inline void ejecutarRenombrarElementoArbol(HWND ventanaPadre, const ElementoArbolVistaCarpetas& elemento) {
    std::wstring rutaDirectorioPadre;
    wchar_t buferDirectorio[MAX_PATH] = L"";
    wcscpy_s(buferDirectorio, elemento.rutaCompleta.c_str());
    PathRemoveFileSpecW(buferDirectorio);
    rutaDirectorioPadre = buferDirectorio;

    wchar_t buferNuevoNombre[MAX_PATH] = L"";
    OPENFILENAMEW estructuraOfn{};
    estructuraOfn.lStructSize = sizeof(estructuraOfn);
    estructuraOfn.hwndOwner = ventanaPadre;
    estructuraOfn.lpstrInitialDir = rutaDirectorioPadre.c_str();
    wcscpy_s(buferNuevoNombre, elemento.nombre.c_str());
    estructuraOfn.lpstrFile = buferNuevoNombre;
    estructuraOfn.nMaxFile = MAX_PATH;
    estructuraOfn.lpstrTitle = L"Escriba o elija el nuevo nombre para el archivo";
    estructuraOfn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&estructuraOfn)) {
        std::wstring rutaNueva(buferNuevoNombre);
        if (!rutaNueva.empty() && rutaNueva != elemento.rutaCompleta) {
            if (MoveFileExW(elemento.rutaCompleta.c_str(), rutaNueva.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) {
                cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
                InvalidateRect(ventanaPadre, nullptr, FALSE);
            } else {
                DWORD errorRenombrado = GetLastError();
                MessageBoxW(ventanaPadre, (L"No se pudo renombrar el elemento (código de error: " + std::to_wstring(errorRenombrado) + L").").c_str(),
                            L"Error al renombrar", MB_OK | MB_ICONERROR);
            }
        }
    }
}

inline void ejecutarEliminarElementoArbol(HWND ventanaPadre, const ElementoArbolVistaCarpetas& elemento) {
    std::wstring mensajeConfirmacion = L"¿Desea enviar '" + elemento.nombre + L"' a la Papelera de reciclaje?";
    int respuestaUsuario = MessageBoxW(ventanaPadre, mensajeConfirmacion.c_str(), L"Confirmar eliminación",
                                       MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
    if (respuestaUsuario != IDYES) return;

    wchar_t buferDobleNulo[MAX_PATH + 2] = L"";
    wcsncpy_s(buferDobleNulo, elemento.rutaCompleta.c_str(), MAX_PATH);
    buferDobleNulo[wcslen(elemento.rutaCompleta.c_str()) + 1] = L'\0';

    SHFILEOPSTRUCTW operacionArchivo{};
    operacionArchivo.hwnd = ventanaPadre;
    operacionArchivo.wFunc = FO_DELETE;
    operacionArchivo.pFrom = buferDobleNulo;
    operacionArchivo.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

    if (SHFileOperationW(&operacionArchivo) == 0 && !operacionArchivo.fAnyOperationsAborted) {
        cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
        InvalidateRect(ventanaPadre, nullptr, FALSE);
    }
}

constexpr UINT ID_MENU_ARBOL_ABRIR = 5101;
constexpr UINT ID_MENU_ARBOL_RENOMBRAR = 5102;
constexpr UINT ID_MENU_ARBOL_ELIMINAR = 5103;

inline void mostrarMenuContextualElementoArbol(HWND ventanaPadre, const ElementoArbolVistaCarpetas& elemento, POINT puntoPantalla) {
    HMENU menuEmergente = CreatePopupMenu();
    if (!menuEmergente) return;

    AppendMenuW(menuEmergente, MF_STRING, ID_MENU_ARBOL_ABRIR, L"Abrir");
    AppendMenuW(menuEmergente, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menuEmergente, MF_STRING, ID_MENU_ARBOL_RENOMBRAR, L"Renombrar...");
    AppendMenuW(menuEmergente, MF_STRING, ID_MENU_ARBOL_ELIMINAR, L"Eliminar");

    SetForegroundWindow(ventanaPadre);
    UINT comandoSeleccionado = static_cast<UINT>(
        TrackPopupMenuEx(menuEmergente,
                         TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                         puntoPantalla.x, puntoPantalla.y,
                         ventanaPadre, nullptr));
    DestroyMenu(menuEmergente);

    if (comandoSeleccionado == ID_MENU_ARBOL_ABRIR) {
        ejecutarAbrirElementoArbol(elemento);
    } else if (comandoSeleccionado == ID_MENU_ARBOL_RENOMBRAR) {
        ejecutarRenombrarElementoArbol(ventanaPadre, elemento);
    } else if (comandoSeleccionado == ID_MENU_ARBOL_ELIMINAR) {
        ejecutarEliminarElementoArbol(ventanaPadre, elemento);
    }
}

inline void dibujarLienzoVistaCarpetas(HDC contextoDestino, const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTitulo = CreateFontW(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteItemArbol = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuentePequena = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int yActual = areaDibujo.top + 16;
    bool hayMultiplesCarpetas = estadoVistaCarpetas.carpetasSeleccionadas.size() > 1;

    std::wstring nombreCarpetaActual;
    std::wstring rutaCompletaActual;
    if (estadoVistaCarpetas.indiceCarpetaActual < estadoVistaCarpetas.carpetasSeleccionadas.size()) {
        rutaCompletaActual = estadoVistaCarpetas.carpetasSeleccionadas[estadoVistaCarpetas.indiceCarpetaActual];
        nombreCarpetaActual = PathFindFileNameW(rutaCompletaActual.c_str());
        if (nombreCarpetaActual.empty()) {
            nombreCarpetaActual = rutaCompletaActual;
        }
    }

    int anchoBoton = 32;
    int altoBoton = 26;
    estadoVistaCarpetas.botonAnteriorCarpeta = {
        areaDibujo.right - 88, yActual, areaDibujo.right - 88 + anchoBoton, yActual + altoBoton};
    estadoVistaCarpetas.botonSiguienteCarpeta = {
        areaDibujo.right - 48, yActual, areaDibujo.right - 48 + anchoBoton, yActual + altoBoton};

    auto dibujarBotonNav = [&](const RECT& r, const wchar_t* texto, bool hover, bool habilitado) {
        COLORREF colorFondo = !habilitado ? RGB(22, 27, 34) : (hover ? RGB(48, 54, 61) : RGB(33, 38, 45));
        COLORREF colorBorde = !habilitado ? RGB(33, 38, 45) : (hover ? RGB(110, 118, 129) : RGB(48, 54, 61));
        HBRUSH bBtn = CreateSolidBrush(colorFondo);
        HPEN lBtn = CreatePen(PS_SOLID, 1, colorBorde);
        HBRUSH bPrev = static_cast<HBRUSH>(SelectObject(contextoDestino, bBtn));
        HPEN lPrev = static_cast<HPEN>(SelectObject(contextoDestino, lBtn));
        RoundRect(contextoDestino, r.left, r.top, r.right, r.bottom, 6, 6);
        SelectObject(contextoDestino, lPrev);
        SelectObject(contextoDestino, bPrev);
        DeleteObject(lBtn);
        DeleteObject(bBtn);

        SetTextColor(contextoDestino, !habilitado ? RGB(75, 85, 99) : (hover ? RGB(255, 255, 255) : RGB(201, 209, 217)));
        HFONT fPrev = static_cast<HFONT>(SelectObject(contextoDestino, fuentePequena));
        RECT rTexto = r;
        DrawTextW(contextoDestino, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDestino, fPrev);
    };

    dibujarBotonNav(estadoVistaCarpetas.botonAnteriorCarpeta, L"◀", estadoVistaCarpetas.hoverBotonAnterior, true);
    dibujarBotonNav(estadoVistaCarpetas.botonSiguienteCarpeta, L"▶", estadoVistaCarpetas.hoverBotonSiguiente, true);

    SelectObject(contextoDestino, fuenteTitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaTitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 100, yActual + 22};
    std::wstring tituloVisual = L"📁 " + nombreCarpetaActual;
    if (hayMultiplesCarpetas) {
        tituloVisual += L" (" + std::to_wstring(estadoVistaCarpetas.indiceCarpetaActual + 1) +
                        L"/" + std::to_wstring(estadoVistaCarpetas.carpetasSeleccionadas.size()) + L")";
    }

    SIZE tamanoTextoTitulo{};
    GetTextExtentPoint32W(contextoDestino, tituloVisual.c_str(), static_cast<int>(tituloVisual.size()), &tamanoTextoTitulo);

    RECT areaTextoTitulo = areaTitulo;
    areaTextoTitulo.right = std::min<LONG>(areaTitulo.right, areaTitulo.left + tamanoTextoTitulo.cx);
    DrawTextW(contextoDestino, tituloVisual.c_str(), -1, &areaTextoTitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    if (!estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado.empty()) {
        SelectObject(contextoDestino, fuenteItemArbol);
        SetTextColor(contextoDestino, RGB(110, 118, 129));
        RECT areaTextoFantasma{areaTextoTitulo.right + 10, yActual + 2, areaDibujo.right - 100, yActual + 22};
        if (areaTextoFantasma.left < areaTextoFantasma.right) {
            DrawTextW(contextoDestino, estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado.c_str(), -1,
                      &areaTextoFantasma, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
    }

    yActual += 20;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaRutaCompleta{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, rutaCompletaActual.c_str(), -1, &areaRutaCompleta,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    yActual += 16;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaSub{areaDibujo.left + 24, yActual, areaDibujo.right - 130, yActual + 18};
    DrawTextW(contextoDestino, L"ESTRUCTURA DE ARCHIVOS (VISTA DE CARPETAS)", -1, &areaSub,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    int maxNivelJerarquia = obtenerMaximoNivelJerarquiaArbol();
    std::wstring textoBtnExpansion;
    if (estadoVistaCarpetas.nivelExpansionActual == 0) {
        textoBtnExpansion = L"▶ Raíz";
    } else if (estadoVistaCarpetas.nivelExpansionActual >= maxNivelJerarquia && maxNivelJerarquia > 0) {
        textoBtnExpansion = L"🔽 Todo";
    } else {
        textoBtnExpansion = L"▶ Nivel " + std::to_wstring(estadoVistaCarpetas.nivelExpansionActual);
    }

    estadoVistaCarpetas.botonAlternarExpansion = {areaDibujo.right - 120, yActual - 2, areaDibujo.right - 24, yActual + 18};
    dibujarBotonNav(estadoVistaCarpetas.botonAlternarExpansion, textoBtnExpansion.c_str(), estadoVistaCarpetas.hoverBotonAlternarExpansion, true);

    yActual += 20;

    int altoCajaArbol = std::max(80, std::min<int>(150, altoArea - (hayMultiplesCarpetas ? 410 : 300)));
    RECT areaCajaArbol{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + altoCajaArbol};
    estadoVistaCarpetas.areaCajaArbol = areaCajaArbol;

    HBRUSH brochaCaja = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizCaja = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPreviaCaja = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaCaja));
    HPEN lapizPrevioCaja = static_cast<HPEN>(SelectObject(contextoDestino, lapizCaja));
    RoundRect(contextoDestino, areaCajaArbol.left, areaCajaArbol.top, areaCajaArbol.right, areaCajaArbol.bottom, 8, 8);
    SelectObject(contextoDestino, lapizPrevioCaja);
    SelectObject(contextoDestino, brochaPreviaCaja);
    DeleteObject(lapizCaja);
    DeleteObject(brochaCaja);

    int yFilaArbol = areaCajaArbol.top + 8 - estadoVistaCarpetas.desplazamientoScrollY;
    int altoFila = 22;
    int elementosVisiblesContador = 0;

    HRGN regionRecorte = CreateRectRgn(areaCajaArbol.left + 2, areaCajaArbol.top + 2,
                                       areaCajaArbol.right - 2, areaCajaArbol.bottom - 2);
    SelectClipRgn(contextoDestino, regionRecorte);

    if (estadoVistaCarpetas.elementosArbol.empty()) {
        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(139, 148, 158));
        RECT areaVacio = areaCajaArbol;
        DrawTextW(contextoDestino, L"(Carpeta vacia o sin subelementos)", -1, &areaVacio,
                   DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    } else {
        for (const auto& elementoArbol : estadoVistaCarpetas.elementosArbol) {
            if (elementoArbol.nivelJerarquia > estadoVistaCarpetas.nivelExpansionActual) {
                continue;
            }
            elementosVisiblesContador++;
            if (yFilaArbol + altoFila >= areaCajaArbol.top && yFilaArbol <= areaCajaArbol.bottom) {
                int sangria = areaCajaArbol.left + 12 + elementoArbol.nivelJerarquia * 16;
                std::wstring prefijo = elementoArbol.esDirectorio ? L"📁 " : L"📄 ";
                std::wstring linea = prefijo + elementoArbol.nombre;

                SelectObject(contextoDestino, fuenteItemArbol);
                SetTextColor(contextoDestino, elementoArbol.esDirectorio ? RGB(88, 166, 255) : RGB(201, 209, 217));
                RECT areaElem{sangria, yFilaArbol, areaCajaArbol.right - 90, yFilaArbol + altoFila};
                DrawTextW(contextoDestino, linea.c_str(), -1, &areaElem,
                          DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                SelectObject(contextoDestino, fuentePequena);
                SetTextColor(contextoDestino, elementoArbol.esDirectorio ? RGB(88, 166, 255) : RGB(110, 118, 129));
                std::wstring tamanoTexto = formatearTamanoCarpeta(elementoArbol.tamanoBytes);
                RECT areaTamanoElem{areaCajaArbol.right - 88, yFilaArbol, areaCajaArbol.right - 12, yFilaArbol + altoFila};
                DrawTextW(contextoDestino, tamanoTexto.c_str(), -1, &areaTamanoElem,
                          DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }
            yFilaArbol += altoFila;
        }
    }
    SelectClipRgn(contextoDestino, nullptr);
    DeleteObject(regionRecorte);

    estadoVistaCarpetas.altoTotalArbol = elementosVisiblesContador * altoFila + 16;

    yActual += altoCajaArbol + 16;

    HPEN lapizSep = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSep = static_cast<HPEN>(SelectObject(contextoDestino, lapizSep));
    MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
    LineTo(contextoDestino, areaDibujo.right - 24, yActual);
    SelectObject(contextoDestino, lapizPrevSep);
    DeleteObject(lapizSep);

    yActual += 14;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaSubAbrir{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, L"ABRIR CARPETA CON", -1, &areaSubAbrir,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 20;

    std::wstring textoProgramaActual = L"Seleccionar programa...";
    std::wstring rutaResuelta = resolverProgramaParaCarpetas();
    if (!rutaResuelta.empty()) {
        textoProgramaActual = PathFindFileNameW(rutaResuelta.c_str());
    }

    int anchoSelector = areaDibujo.right - areaDibujo.left - 48 - 100;
    estadoVistaCarpetas.botonSelectorPrograma = {areaDibujo.left + 24, yActual, areaDibujo.left + 24 + anchoSelector, yActual + 30};
    estadoVistaCarpetas.botonEjecutarAbrir = {areaDibujo.right - 24 - 90, yActual, areaDibujo.right - 24, yActual + 30};

    bool selectorHover = (estadoVistaCarpetas.opcionProgramaHover == 9999);
    HBRUSH brochaSelector = CreateSolidBrush(selectorHover ? RGB(33, 38, 45) : RGB(22, 27, 34));
    HPEN lapizSelector = CreatePen(PS_SOLID, 1, selectorHover ? RGB(88, 166, 255) : RGB(48, 54, 61));
    HBRUSH brochaPreviaSelector = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaSelector));
    HPEN lapizPrevioSelector = static_cast<HPEN>(SelectObject(contextoDestino, lapizSelector));
    RoundRect(contextoDestino, estadoVistaCarpetas.botonSelectorPrograma.left,
              estadoVistaCarpetas.botonSelectorPrograma.top,
              estadoVistaCarpetas.botonSelectorPrograma.right,
              estadoVistaCarpetas.botonSelectorPrograma.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioSelector);
    SelectObject(contextoDestino, brochaPreviaSelector);
    DeleteObject(lapizSelector);
    DeleteObject(brochaSelector);

    SelectObject(contextoDestino, fuenteItemArbol);
    SetTextColor(contextoDestino, rutaResuelta.empty() ? RGB(110, 118, 129) : RGB(240, 246, 252));
    RECT areaTextoPrograma = estadoVistaCarpetas.botonSelectorPrograma;
    areaTextoPrograma.left += 10;
    areaTextoPrograma.right -= 24;
    DrawTextW(contextoDestino, textoProgramaActual.c_str(), -1, &areaTextoPrograma,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaFlecha{estadoVistaCarpetas.botonSelectorPrograma.right - 20,
                    estadoVistaCarpetas.botonSelectorPrograma.top,
                    estadoVistaCarpetas.botonSelectorPrograma.right - 6,
                    estadoVistaCarpetas.botonSelectorPrograma.bottom};
    DrawTextW(contextoDestino, estadoVistaCarpetas.menuProgramasAbierto ? L"▲" : L"▼", -1,
              &areaFlecha, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    bool ejecutarHover = (estadoVistaCarpetas.opcionProgramaHover == 9998);
    HBRUSH brochaEjecutar = CreateSolidBrush(ejecutarHover ? RGB(46, 160, 67) : RGB(35, 134, 54));
    HPEN lapizEjecutar = CreatePen(PS_SOLID, 1, RGB(46, 160, 67));
    HBRUSH brochaPreviaEjecutar = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaEjecutar));
    HPEN lapizPrevioEjecutar = static_cast<HPEN>(SelectObject(contextoDestino, lapizEjecutar));
    RoundRect(contextoDestino, estadoVistaCarpetas.botonEjecutarAbrir.left,
              estadoVistaCarpetas.botonEjecutarAbrir.top,
              estadoVistaCarpetas.botonEjecutarAbrir.right,
              estadoVistaCarpetas.botonEjecutarAbrir.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioEjecutar);
    SelectObject(contextoDestino, brochaPreviaEjecutar);
    DeleteObject(lapizEjecutar);
    DeleteObject(brochaEjecutar);

    SelectObject(contextoDestino, fuenteItemArbol);
    SetTextColor(contextoDestino, RGB(255, 255, 255));
    RECT areaTextoEjecutar = estadoVistaCarpetas.botonEjecutarAbrir;
    DrawTextW(contextoDestino, L"Abrir", -1, &areaTextoEjecutar,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 42;

    HPEN lapizSepAcciones = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSepAcciones = static_cast<HPEN>(SelectObject(contextoDestino, lapizSepAcciones));
    MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
    LineTo(contextoDestino, areaDibujo.right - 24, yActual);
    SelectObject(contextoDestino, lapizPrevSepAcciones);
    DeleteObject(lapizSepAcciones);

    yActual += 12;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaSubAcciones{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, L"ACCIONES RÁPIDAS", -1, &areaSubAcciones,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 20;

    int anchoContenedor = areaDibujo.right - areaDibujo.left - 48;
    int altoContenedorAcciones = 42;
    estadoVistaCarpetas.areaContenedorAccionesRapidas = {
        areaDibujo.left + 24, yActual,
        areaDibujo.left + 24 + anchoContenedor,
        yActual + altoContenedorAcciones
    };

    HBRUSH brochaContenedor = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizContenedor = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPreviaContenedor = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaContenedor));
    HPEN lapizPrevioContenedor = static_cast<HPEN>(SelectObject(contextoDestino, lapizContenedor));
    RoundRect(contextoDestino, estadoVistaCarpetas.areaContenedorAccionesRapidas.left,
              estadoVistaCarpetas.areaContenedorAccionesRapidas.top,
              estadoVistaCarpetas.areaContenedorAccionesRapidas.right,
              estadoVistaCarpetas.areaContenedorAccionesRapidas.bottom, 8, 8);
    SelectObject(contextoDestino, lapizPrevioContenedor);
    SelectObject(contextoDestino, brochaPreviaContenedor);
    DeleteObject(lapizContenedor);
    DeleteObject(brochaContenedor);

    int paddingInterno = 5;
    int anchoBotonAccion = (anchoContenedor - (paddingInterno * 3)) / 2;
    int yBotonesAccion = yActual + paddingInterno;
    int altoBotonAccion = altoContenedorAcciones - (paddingInterno * 2);

    estadoVistaCarpetas.botonCrearEnlaceSimbolico = {
        estadoVistaCarpetas.areaContenedorAccionesRapidas.left + paddingInterno,
        yBotonesAccion,
        estadoVistaCarpetas.areaContenedorAccionesRapidas.left + paddingInterno + anchoBotonAccion,
        yBotonesAccion + altoBotonAccion
    };

    estadoVistaCarpetas.botonCopiarContenidoPortapapeles = {
        estadoVistaCarpetas.botonCrearEnlaceSimbolico.right + paddingInterno,
        yBotonesAccion,
        estadoVistaCarpetas.botonCrearEnlaceSimbolico.right + paddingInterno + anchoBotonAccion,
        yBotonesAccion + altoBotonAccion
    };

    // 1. Botón Enlace Simbólico
    bool hoverEnlace = estadoVistaCarpetas.hoverBotonCrearEnlace;
    HBRUSH brochaEnlace = CreateSolidBrush(hoverEnlace ? RGB(33, 38, 45) : RGB(13, 17, 23));
    HPEN lapizEnlace = CreatePen(PS_SOLID, 1, hoverEnlace ? RGB(88, 166, 255) : RGB(48, 54, 61));
    HBRUSH brochaPreviaEnlace = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaEnlace));
    HPEN lapizPrevioEnlace = static_cast<HPEN>(SelectObject(contextoDestino, lapizEnlace));
    RoundRect(contextoDestino, estadoVistaCarpetas.botonCrearEnlaceSimbolico.left,
              estadoVistaCarpetas.botonCrearEnlaceSimbolico.top,
              estadoVistaCarpetas.botonCrearEnlaceSimbolico.right,
              estadoVistaCarpetas.botonCrearEnlaceSimbolico.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioEnlace);
    SelectObject(contextoDestino, brochaPreviaEnlace);
    DeleteObject(lapizEnlace);
    DeleteObject(brochaEnlace);

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, hoverEnlace ? RGB(255, 255, 255) : RGB(201, 209, 217));
    RECT areaTextoEnlace = estadoVistaCarpetas.botonCrearEnlaceSimbolico;
    DrawTextW(contextoDestino, L"🔗 Crear enlace", -1, &areaTextoEnlace,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // 2. Botón Copiar Contenido
    bool hoverCopiar = estadoVistaCarpetas.hoverBotonCopiarContenido;
    HBRUSH brochaCopiar = CreateSolidBrush(hoverCopiar ? RGB(33, 38, 45) : RGB(13, 17, 23));
    HPEN lapizCopiar = CreatePen(PS_SOLID, 1, hoverCopiar ? RGB(88, 166, 255) : RGB(48, 54, 61));
    HBRUSH brochaPreviaCopiar = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaCopiar));
    HPEN lapizPrevioCopiar = static_cast<HPEN>(SelectObject(contextoDestino, lapizCopiar));
    RoundRect(contextoDestino, estadoVistaCarpetas.botonCopiarContenidoPortapapeles.left,
              estadoVistaCarpetas.botonCopiarContenidoPortapapeles.top,
              estadoVistaCarpetas.botonCopiarContenidoPortapapeles.right,
              estadoVistaCarpetas.botonCopiarContenidoPortapapeles.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioCopiar);
    SelectObject(contextoDestino, brochaPreviaCopiar);
    DeleteObject(lapizCopiar);
    DeleteObject(brochaCopiar);

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, hoverCopiar ? RGB(255, 255, 255) : RGB(201, 209, 217));
    RECT areaTextoCopiar = estadoVistaCarpetas.botonCopiarContenidoPortapapeles;
    DrawTextW(contextoDestino, L"📋 Copiar contenido", -1, &areaTextoCopiar,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    if (hayMultiplesCarpetas) {
        yActual += 42;

        HPEN lapizSepOp = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
        HPEN lapizPrevSepOp = static_cast<HPEN>(SelectObject(contextoDestino, lapizSepOp));
        MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
        LineTo(contextoDestino, areaDibujo.right - 24, yActual);
        SelectObject(contextoDestino, lapizPrevSepOp);
        DeleteObject(lapizSepOp);

        yActual += 12;

        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(139, 148, 158));
        RECT areaSubOperaciones{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
        DrawTextW(contextoDestino, L"OPERACIONES CON CARPETA", -1, &areaSubOperaciones,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yActual += 20;

        estadoVistaCarpetas.botonFusionCarpetas = {areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 32};
        bool hoverFusion = estadoVistaCarpetas.hoverBotonFusion;
        HBRUSH brochaFusion = CreateSolidBrush(hoverFusion ? RGB(48, 54, 61) : RGB(33, 38, 45));
        HPEN lapizFusion = CreatePen(PS_SOLID, 1, hoverFusion ? RGB(88, 166, 255) : RGB(48, 54, 61));
        HBRUSH brochaPreviaFusion = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaFusion));
        HPEN lapizPrevioFusion = static_cast<HPEN>(SelectObject(contextoDestino, lapizFusion));
        RoundRect(contextoDestino, estadoVistaCarpetas.botonFusionCarpetas.left,
                  estadoVistaCarpetas.botonFusionCarpetas.top,
                  estadoVistaCarpetas.botonFusionCarpetas.right,
                  estadoVistaCarpetas.botonFusionCarpetas.bottom, 6, 6);
        SelectObject(contextoDestino, lapizPrevioFusion);
        SelectObject(contextoDestino, brochaPreviaFusion);
        DeleteObject(lapizFusion);
        DeleteObject(brochaFusion);

        SelectObject(contextoDestino, fuenteItemArbol);
        SetTextColor(contextoDestino, hoverFusion ? RGB(255, 255, 255) : RGB(230, 237, 243));
        RECT areaTextoFusion = estadoVistaCarpetas.botonFusionCarpetas;
        DrawTextW(contextoDestino, L"🔀 Fusión de carpetas", -1, &areaTextoFusion,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    } else {
        estadoVistaCarpetas.botonFusionCarpetas = {};
    }

    DeleteObject(fuenteTitulo);
    DeleteObject(fuenteItemArbol);
    DeleteObject(fuentePequena);
}

inline void dibujarLienzoVistaVacia(HDC contextoDestino, const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTituloGrande = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubtitulo = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteItemVistaVacia = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuentePequena = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int yActual = areaDibujo.top + 20;

    if (estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema) {
        SelectObject(contextoDestino, fuenteTituloGrande);
        SetTextColor(contextoDestino, RGB(255, 255, 255));
        RECT areaTituloSistema{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 28};
        DrawTextW(contextoDestino, L"Useful Sideview", -1, &areaTituloSistema,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yActual += 32;

        SelectObject(contextoDestino, fuenteSubtitulo);
        SetTextColor(contextoDestino, RGB(88, 166, 255));
        RECT areaNombreSeccion{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 22};
        std::wstring etiquetaSeccion = L"📁 " + estadoVistaCarpetas.resumenVistaVacia.nombreUbicacionSistema;
        DrawTextW(contextoDestino, etiquetaSeccion.c_str(), -1, &areaNombreSeccion,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yActual += 30;

        int altoTarjetaInfo = 110;
        RECT areaTarjetaInfo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + altoTarjetaInfo};
        HBRUSH brochaInfo = CreateSolidBrush(RGB(22, 27, 34));
        HPEN lapizInfo = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
        HBRUSH brochaPrevInfo = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaInfo));
        HPEN lapizPrevInfo = static_cast<HPEN>(SelectObject(contextoDestino, lapizInfo));
        RoundRect(contextoDestino, areaTarjetaInfo.left, areaTarjetaInfo.top,
                  areaTarjetaInfo.right, areaTarjetaInfo.bottom, 8, 8);
        SelectObject(contextoDestino, lapizPrevInfo);
        SelectObject(contextoDestino, brochaPrevInfo);
        DeleteObject(lapizInfo);
        DeleteObject(brochaInfo);

        SelectObject(contextoDestino, fuenteItemVistaVacia);
        SetTextColor(contextoDestino, RGB(201, 209, 217));
        RECT areaTextoInfo{areaTarjetaInfo.left + 16, areaTarjetaInfo.top + 14,
                           areaTarjetaInfo.right - 16, areaTarjetaInfo.bottom - 14};
        DrawTextW(contextoDestino,
                  estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema.c_str(),
                  -1, &areaTextoInfo,
                  DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

        DeleteObject(fuenteTituloGrande);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteItemVistaVacia);
        DeleteObject(fuentePequena);
        return;
    }

    SelectObject(contextoDestino, fuenteTituloGrande);
    SetTextColor(contextoDestino, RGB(88, 166, 255));
    RECT areaTitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 28};
    DrawTextW(contextoDestino, L"Useful Sideview", -1, &areaTitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 30;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaDesc{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, L"VISTA VACÍA: RESUMEN DE LA CARPETA ACTUAL", -1, &areaDesc,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 22;

    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaNombreCarpeta{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 20};
    std::wstring txtNombre = L"📁 " + (estadoVistaCarpetas.resumenVistaVacia.nombreCarpeta.empty()
                                         ? estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta
                                         : estadoVistaCarpetas.resumenVistaVacia.nombreCarpeta);
    DrawTextW(contextoDestino, txtNombre.c_str(), -1, &areaNombreCarpeta,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    yActual += 20;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaRutaVistaVacia{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta.c_str(), -1, &areaRutaVistaVacia,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    yActual += 24;

    int anchoTarjeta = (areaDibujo.right - areaDibujo.left - 48 - 12) / 2;
    int altoTarjeta = 66;

    auto dibujarTarjetaMetrica = [&](int posicionHorizontal, int posicionVertical, const wchar_t* tituloMetrica,
                                     const std::wstring& textoValor, COLORREF colorValor,
                                     const std::wstring& textoFantasma = L"") {
        RECT rectanguloTarjeta{posicionHorizontal, posicionVertical, posicionHorizontal + anchoTarjeta, posicionVertical + altoTarjeta};
        HBRUSH brochaTarjeta = CreateSolidBrush(RGB(22, 27, 34));
        HPEN lapizTarjeta = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
        HBRUSH brochaPrevia = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaTarjeta));
        HPEN lapizPrevio = static_cast<HPEN>(SelectObject(contextoDestino, lapizTarjeta));
        RoundRect(contextoDestino, rectanguloTarjeta.left, rectanguloTarjeta.top, rectanguloTarjeta.right, rectanguloTarjeta.bottom, 6, 6);
        SelectObject(contextoDestino, lapizPrevio);
        SelectObject(contextoDestino, brochaPrevia);
        DeleteObject(lapizTarjeta);
        DeleteObject(brochaTarjeta);

        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(139, 148, 158));
        RECT rectanguloTitulo{posicionHorizontal + 10, posicionVertical + 6, posicionHorizontal + anchoTarjeta - 10, posicionVertical + 19};
        DrawTextW(contextoDestino, tituloMetrica, -1, &rectanguloTitulo, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(contextoDestino, fuenteSubtitulo);
        SetTextColor(contextoDestino, colorValor);
        RECT rectanguloValor{posicionHorizontal + 10, posicionVertical + 21, posicionHorizontal + anchoTarjeta - 10, posicionVertical + 41};
        DrawTextW(contextoDestino, textoValor.c_str(), -1, &rectanguloValor, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        if (!textoFantasma.empty()) {
            SelectObject(contextoDestino, fuentePequena);
            SetTextColor(contextoDestino, RGB(110, 118, 129));
            RECT rectanguloFantasma{posicionHorizontal + 10, posicionVertical + 43, posicionHorizontal + anchoTarjeta - 10, posicionVertical + 59};
            DrawTextW(contextoDestino, textoFantasma.c_str(), -1, &rectanguloFantasma,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
    };

    // 1. Peso total
    dibujarTarjetaMetrica(areaDibujo.left + 24, yActual, L"Peso total",
                          formatearTamanoCarpeta(estadoVistaCarpetas.resumenVistaVacia.tamanoTotalBytes),
                          RGB(88, 166, 255));
    // 2. Cantidad de carpetas
    dibujarTarjetaMetrica(areaDibujo.left + 24 + anchoTarjeta + 12, yActual, L"Subcarpetas",
                          std::to_wstring(estadoVistaCarpetas.resumenVistaVacia.totalSubcarpetas) + L" carpetas",
                          RGB(63, 185, 80));

    yActual += altoTarjeta + 10;

    // 3. Tipos de archivo (con texto fantasma discreto de las extensiones)
    dibujarTarjetaMetrica(areaDibujo.left + 24, yActual, L"Tipos de archivos",
                          std::to_wstring(estadoVistaCarpetas.resumenVistaVacia.cantidadTiposArchivos) + L" extensiones",
                          RGB(210, 153, 34),
                          estadoVistaCarpetas.resumenVistaVacia.textoExtensionesFantasma);
    // 4. Ultimo acceso
    dibujarTarjetaMetrica(areaDibujo.left + 24 + anchoTarjeta + 12, yActual, L"Último acceso",
                          estadoVistaCarpetas.resumenVistaVacia.fechaUltimoAcceso.empty() ? L"N/D" : estadoVistaCarpetas.resumenVistaVacia.fechaUltimoAcceso,
                          RGB(163, 113, 247));

    yActual += altoTarjeta + 22;

    HPEN lapizSep = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSep = static_cast<HPEN>(SelectObject(contextoDestino, lapizSep));
    MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
    LineTo(contextoDestino, areaDibujo.right - 24, yActual);
    SelectObject(contextoDestino, lapizPrevSep);
    DeleteObject(lapizSep);

    yActual += 16;

    // 5. Top 4 subcarpetas mas pesadas
    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaTopTit{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 20};
    DrawTextW(contextoDestino, L"Top 4 Subcarpetas más pesadas", -1, &areaTopTit,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 26;

    if (estadoVistaCarpetas.resumenVistaVacia.top4CarpetasMasPesadas.empty()) {
        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(139, 148, 158));
        RECT areaSinSubcarpetas{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 30};
        DrawTextW(contextoDestino, L"(No se encontraron subcarpetas en el directorio actual)", -1, &areaSinSubcarpetas,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    } else {
        int umbralBajo = configuracion.umbralPorcentajeBajoMedioVistaVacia.load();
        int umbralAlto = configuracion.umbralPorcentajeMedioAltoVistaVacia.load();
        COLORREF colorBajo = configuracion.colorRangoBajoVistaVacia.load();
        COLORREF colorMedio = configuracion.colorRangoMedioVistaVacia.load();
        COLORREF colorAlto = configuracion.colorRangoAltoVistaVacia.load();

        for (const auto& subcarpeta : estadoVistaCarpetas.resumenVistaVacia.top4CarpetasMasPesadas) {
            SelectObject(contextoDestino, fuenteItemVistaVacia);
            SetTextColor(contextoDestino, RGB(201, 209, 217));
            RECT areaNombreSubcarpeta{areaDibujo.left + 24, yActual, areaDibujo.right - 140, yActual + 18};
            std::wstring nombreLimpio = L"📁 " + subcarpeta.nombre;
            DrawTextW(contextoDestino, nombreLimpio.c_str(), -1, &areaNombreSubcarpeta,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

            SelectObject(contextoDestino, fuentePequena);
            SetTextColor(contextoDestino, RGB(139, 148, 158));
            double porcentajeSubcarpeta = subcarpeta.porcentaje * 100.0;
            wchar_t buferTamanoPorcentaje[64];
            swprintf_s(buferTamanoPorcentaje, L"%ls (%.1f%%)",
                       formatearTamanoCarpeta(subcarpeta.tamanoBytes).c_str(),
                       porcentajeSubcarpeta);
            RECT areaTamanoSubcarpeta{areaDibujo.right - 136, yActual, areaDibujo.right - 24, yActual + 18};
            DrawTextW(contextoDestino, buferTamanoPorcentaje, -1, &areaTamanoSubcarpeta,
                      DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            yActual += 20;

            int anchoBarraTotal = areaDibujo.right - areaDibujo.left - 48;
            int anchoBarraProgreso = std::max(4, static_cast<int>(static_cast<double>(anchoBarraTotal) * subcarpeta.porcentaje));

            RECT areaFondoBarra{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 6};
            HBRUSH brochaFondoBarra = CreateSolidBrush(RGB(33, 38, 45));
            FillRect(contextoDestino, &areaFondoBarra, brochaFondoBarra);
            DeleteObject(brochaFondoBarra);

            COLORREF colorBarra;
            if (porcentajeSubcarpeta < static_cast<double>(umbralBajo)) {
                colorBarra = colorBajo;
            } else if (porcentajeSubcarpeta < static_cast<double>(umbralAlto)) {
                colorBarra = colorMedio;
            } else {
                colorBarra = colorAlto;
            }

            RECT areaRellenoBarra{areaDibujo.left + 24, yActual, areaDibujo.left + 24 + anchoBarraProgreso, yActual + 6};
            HBRUSH brochaRellenoBarra = CreateSolidBrush(colorBarra);
            FillRect(contextoDestino, &areaRellenoBarra, brochaRellenoBarra);
            DeleteObject(brochaRellenoBarra);

            yActual += 14;
        }
    }

    yActual += 8;
    HPEN lapizSep2 = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSep2 = static_cast<HPEN>(SelectObject(contextoDestino, lapizSep2));
    MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
    LineTo(contextoDestino, areaDibujo.right - 24, yActual);
    SelectObject(contextoDestino, lapizPrevSep2);
    DeleteObject(lapizSep2);

    yActual += 14;

    // 6. Sección de Carpetas Vacías
    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaVaciasTit{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 20};
    int totalVacias = static_cast<int>(estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.size());
    std::wstring tituloVacias = L"📁 Carpetas Vacías (" + std::to_wstring(totalVacias) + L")";
    DrawTextW(contextoDestino, tituloVacias.c_str(), -1, &areaVaciasTit,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 24;

    if (totalVacias == 0) {
        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(63, 185, 80));
        RECT areaSinVacias{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 22};
        std::wstring mensajeTextoVacias = estadoVistaCarpetas.resumenVistaVacia.mensajeResultadoEliminacion.empty()
            ? L"✓ No se detectaron carpetas vacías en este directorio."
            : estadoVistaCarpetas.resumenVistaVacia.mensajeResultadoEliminacion;
        DrawTextW(contextoDestino, mensajeTextoVacias.c_str(), -1, &areaSinVacias,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias = {};
    } else {
        size_t limiteCarpetasMostrar = std::min<size_t>(3, estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.size());
        for (size_t posicionCarpetaVacia = 0; posicionCarpetaVacia < limiteCarpetasMostrar; ++posicionCarpetaVacia) {
            SelectObject(contextoDestino, fuenteItemVistaVacia);
            SetTextColor(contextoDestino, RGB(201, 209, 217));
            RECT areaItemVacia{areaDibujo.left + 28, yActual, areaDibujo.right - 24, yActual + 18};
            std::wstring textoCarpetaVacia = L"• 📁 " + estadoVistaCarpetas.resumenVistaVacia.carpetasVacias[posicionCarpetaVacia];
            DrawTextW(contextoDestino, textoCarpetaVacia.c_str(), -1, &areaItemVacia,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
            yActual += 20;
        }
        if (estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.size() > 3) {
            SelectObject(contextoDestino, fuentePequena);
            SetTextColor(contextoDestino, RGB(139, 148, 158));
            RECT areaMasVacias{areaDibujo.left + 28, yActual, areaDibujo.right - 24, yActual + 16};
            std::wstring textoMasCarpetas = L"+ " + std::to_wstring(estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.size() - 3) + L" carpetas vacías más...";
            DrawTextW(contextoDestino, textoMasCarpetas.c_str(), -1, &areaMasVacias,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            yActual += 18;
        }

        yActual += 6;

        int anchoBoton = std::min<int>(220, static_cast<int>(areaDibujo.right - areaDibujo.left - 48));
        int altoBoton = 30;
        RECT areaBoton{areaDibujo.left + 24, yActual, areaDibujo.left + 24 + anchoBoton, yActual + altoBoton};
        estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias = areaBoton;

        bool hoverBotonEliminar = estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias;
        HBRUSH brochaBotonEliminar = CreateSolidBrush(hoverBotonEliminar ? RGB(218, 54, 51) : RGB(33, 38, 45));
        HPEN lapizBotonEliminar = CreatePen(PS_SOLID, 1, hoverBotonEliminar ? RGB(248, 81, 73) : RGB(248, 81, 73));
        HBRUSH brochaPreviaBoton = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaBotonEliminar));
        HPEN lapizPrevioBoton = static_cast<HPEN>(SelectObject(contextoDestino, lapizBotonEliminar));
        RoundRect(contextoDestino, areaBoton.left, areaBoton.top, areaBoton.right, areaBoton.bottom, 6, 6);
        SelectObject(contextoDestino, lapizPrevioBoton);
        SelectObject(contextoDestino, brochaPreviaBoton);
        DeleteObject(lapizBotonEliminar);
        DeleteObject(brochaBotonEliminar);

        SelectObject(contextoDestino, fuenteSubtitulo);
        SetTextColor(contextoDestino, hoverBotonEliminar ? RGB(255, 255, 255) : RGB(248, 81, 73));
        DrawTextW(contextoDestino, L"🗑️ Eliminar carpetas vacías", -1, &areaBoton,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    DeleteObject(fuenteTituloGrande);
    DeleteObject(fuenteSubtitulo);
    DeleteObject(fuenteItemVistaVacia);
    DeleteObject(fuentePequena);
}