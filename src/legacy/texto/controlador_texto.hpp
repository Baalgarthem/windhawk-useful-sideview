#pragma once

#include <windows.h>
#include <shlwapi.h>
#include <urlmon.h>
#include <gdiplus.h>
#include <algorithm>
#include <string>
#include <vector>
Gdiplus::Bitmap* decodificarImagenConWic(const std::wstring& rutaArchivo);

enum class TipoElementoMarkdown {
    Encabezado1,
    Encabezado2,
    Encabezado3,
    Encabezado4,
    Parrafo,
    BloqueCodigo,
    CitaBloque,
    ListaDesordenada,
    ListaOrdenada,
    CasillaVerificacion,
    ReglaHorizontal,
    SeparadorSeccionPersonalizado,
    Imagen,
    VideoYouTube,
    EnlaceWeb,
    TablaMarkdown
};

enum class EstiloFragmento {
    Normal,
    Negrita,
    Cursiva,
    Subrayado,
    CodigoPildora
};

struct FragmentoTexto {
    std::wstring texto;
    EstiloFragmento estilo{EstiloFragmento::Normal};
};

struct FilaTablaMarkdown {
    std::vector<std::wstring> celdas;
    bool esCabecera{false};
};

struct ElementoMarkdown {
    TipoElementoMarkdown tipo{TipoElementoMarkdown::Parrafo};
    std::wstring textoContenido;
    std::wstring textoSecundario;
    std::vector<FragmentoTexto> fragmentos;
    std::vector<FilaTablaMarkdown> filasTabla;
    int nivelEncabezado{0};
    int sangriaLista{0};
    bool casillaMarcada{false};
    int posicionVerticalY{0};
    int altoCalculado{0};
};

struct ElementoIndiceBarras {
    std::wstring tituloEncabezado;
    int nivelEncabezado{1};
    int posicionDestinoY{0};
    RECT areaVisualBarra{};
    RECT areaBarra{};
};

struct ElementoClicableMarkdown {
    RECT areaClic{};
    std::wstring destinoUrl;
    bool esVideo{false};
    bool esVideoYouTube{false};
    bool esEnlaceWeb{false};
};

struct EntradaCacheBitmap {
    std::wstring rutaUbicacion;
    Gdiplus::Bitmap* mapaBits{nullptr};
};

struct EstadoControladorTexto {
    std::vector<std::wstring> textosSeleccionados;
    std::wstring rutaTextoActual;
    size_t indiceTextoSeleccionado{0};
    std::wstring nombreArchivo;
    bool esFormatoMarkdown{true};
    size_t posicionEnCarpeta{1};
    size_t totalEnCarpeta{1};
    RECT botonAnterior{};
    RECT botonSiguiente{};
    RECT botonEditar{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};
    bool hoverBotonEditar{false};
    std::vector<ElementoMarkdown> elementosMarkdown;
    std::vector<ElementoIndiceBarras> barrasIndice;
    std::vector<ElementoClicableMarkdown> elementosClicables;
    std::vector<EntradaCacheBitmap> cacheBitmaps;
    int desplazamientoScrollY{0};
    int altoTotalContenido{0};
    int posicionBarraHover{-1};
    bool panelIndiceDesplegado{false};
    size_t totalLineasOriginales{0};
};

inline EstadoControladorTexto estadoTexto;

inline void limpiarContenidoTextoCargado() {
    estadoTexto.rutaTextoActual.clear();
    estadoTexto.nombreArchivo.clear();
    estadoTexto.esFormatoMarkdown = true;
    estadoTexto.posicionEnCarpeta = 1;
    estadoTexto.totalEnCarpeta = 1;
    estadoTexto.elementosMarkdown.clear();
    estadoTexto.barrasIndice.clear();
    estadoTexto.elementosClicables.clear();
    for (auto& entrada : estadoTexto.cacheBitmaps) {
        if (entrada.mapaBits) {
            delete entrada.mapaBits;
        }
    }
    estadoTexto.cacheBitmaps.clear();
    estadoTexto.desplazamientoScrollY = 0;
    estadoTexto.altoTotalContenido = 0;
    estadoTexto.posicionBarraHover = -1;
    estadoTexto.panelIndiceDesplegado = false;
    estadoTexto.totalLineasOriginales = 0;
}

inline void liberarRecursosTexto() {
    limpiarContenidoTextoCargado();
    estadoTexto.textosSeleccionados.clear();
    estadoTexto.indiceTextoSeleccionado = 0;
    estadoTexto.botonAnterior = {};
    estadoTexto.botonSiguiente = {};
    estadoTexto.botonEditar = {};
    estadoTexto.hoverBotonAnterior = false;
    estadoTexto.hoverBotonSiguiente = false;
    estadoTexto.hoverBotonEditar = false;
}

inline std::wstring extraerIdVideoYouTube(const std::wstring& urlOTexto) {
    size_t posicionWatch = urlOTexto.find(L"watch?v=");
    if (posicionWatch != std::wstring::npos) {
        std::wstring idCandidato = urlOTexto.substr(posicionWatch + 8);
        size_t posicionAmp = idCandidato.find(L'&');
        if (posicionAmp != std::wstring::npos) {
            idCandidato = idCandidato.substr(0, posicionAmp);
        }
        size_t posicionCierre = idCandidato.find_first_of(L" \t\r\n)\">");
        if (posicionCierre != std::wstring::npos) {
            idCandidato = idCandidato.substr(0, posicionCierre);
        }
        if (!idCandidato.empty()) return idCandidato;
    }

    size_t posicionYoutuBe = urlOTexto.find(L"youtu.be/");
    if (posicionYoutuBe != std::wstring::npos) {
        std::wstring idCandidato = urlOTexto.substr(posicionYoutuBe + 9);
        size_t posicionCierre = idCandidato.find_first_of(L" \t\r\n?&)\">");
        if (posicionCierre != std::wstring::npos) {
            idCandidato = idCandidato.substr(0, posicionCierre);
        }
        if (!idCandidato.empty()) return idCandidato;
    }

    size_t posicionEmbed = urlOTexto.find(L"youtube.com/embed/");
    if (posicionEmbed != std::wstring::npos) {
        std::wstring idCandidato = urlOTexto.substr(posicionEmbed + 18);
        size_t posicionCierre = idCandidato.find_first_of(L" \t\r\n?&)\">");
        if (posicionCierre != std::wstring::npos) {
            idCandidato = idCandidato.substr(0, posicionCierre);
        }
        if (!idCandidato.empty()) return idCandidato;
    }

    return {};
}

inline Gdiplus::Bitmap* obtenerBitmapParaMarkdown(
    const std::wstring& rutaUbicacion,
    const std::wstring& rutaBaseTexto) {
    if (rutaUbicacion.empty()) return nullptr;

    for (const auto& entrada : estadoTexto.cacheBitmaps) {
        if (entrada.rutaUbicacion == rutaUbicacion) {
            return entrada.mapaBits;
        }
    }

    Gdiplus::Bitmap* mapaBitsCargado = nullptr;

    bool esUrlRemota = (rutaUbicacion.rfind(L"http://", 0) == 0 ||
                        rutaUbicacion.rfind(L"https://", 0) == 0);

    if (esUrlRemota) {
        wchar_t rutaCarpetaTemporal[MAX_PATH];
        if (GetTempPathW(MAX_PATH, rutaCarpetaTemporal) > 0) {
            std::wstring carpetaCache =
                std::wstring(rutaCarpetaTemporal) + L"UsefulSideViewMdCache";
            CreateDirectoryW(carpetaCache.c_str(), nullptr);

            DWORD hashValor = 5381;
            for (wchar_t caracter : rutaUbicacion) {
                hashValor = ((hashValor << 5) + hashValor) + static_cast<DWORD>(caracter);
            }
            std::wstring rutaArchivoTemporal =
                carpetaCache + L"\\" + std::to_wstring(hashValor) + L".img";

            if (GetFileAttributesW(rutaArchivoTemporal.c_str()) ==
                INVALID_FILE_ATTRIBUTES) {
                estadoTexto.cacheBitmaps.push_back(
                    {rutaUbicacion, nullptr});
                return nullptr;
            }

            mapaBitsCargado = decodificarImagenConWic(rutaArchivoTemporal.c_str());
        }
    } else {
        std::wstring rutaFinalLocal = rutaUbicacion;
        if (PathIsRelativeW(rutaUbicacion.c_str()) &&
            !rutaBaseTexto.empty()) {
            wchar_t buferDirectorio[MAX_PATH];
            wcsncpy_s(buferDirectorio, rutaBaseTexto.c_str(), MAX_PATH);
            PathRemoveFileSpecW(buferDirectorio);

            wchar_t rutaCombinada[MAX_PATH];
            if (PathCombineW(rutaCombinada, buferDirectorio,
                             rutaUbicacion.c_str())) {
                rutaFinalLocal = rutaCombinada;
            }
        }

        mapaBitsCargado = decodificarImagenConWic(rutaFinalLocal.c_str());
    }

    if (mapaBitsCargado &&
        mapaBitsCargado->GetLastStatus() == Gdiplus::Ok) {
        estadoTexto.cacheBitmaps.push_back(
            {rutaUbicacion, mapaBitsCargado});
        return mapaBitsCargado;
    }

    if (mapaBitsCargado) delete mapaBitsCargado;
    estadoTexto.cacheBitmaps.push_back({rutaUbicacion, nullptr});
    return nullptr;
}

inline std::wstring leerArchivoTextoUtf8(const std::wstring& rutaArchivo) {
    // Causa 3: Reintentar apertura si el archivo esta bloqueado temporalmente
    HANDLE manejadorArchivo = INVALID_HANDLE_VALUE;
    for (int intento = 0; intento < 3; ++intento) {
        manejadorArchivo = CreateFileW(
            rutaArchivo.c_str(), GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (manejadorArchivo != INVALID_HANDLE_VALUE) break;
        if (GetLastError() != ERROR_SHARING_VIOLATION) break;
        Sleep(5);
    }
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return {};

    DWORD tamanoArchivoAlto = 0;
    DWORD tamanoArchivoBajo =
        GetFileSize(manejadorArchivo, &tamanoArchivoAlto);
    if (tamanoArchivoBajo == INVALID_FILE_SIZE) {
        CloseHandle(manejadorArchivo);
        return {};
    }
    if (tamanoArchivoBajo == 0) {
        CloseHandle(manejadorArchivo);
        return L"*(Archivo vacío)*";
    }

    constexpr DWORD LIMITE_MAXIMO_LECTURA = 2 * 1024 * 1024;
    DWORD bytesALeer = std::min<DWORD>(tamanoArchivoBajo, LIMITE_MAXIMO_LECTURA);
    std::vector<char> buferBytes(bytesALeer + 2, '\0');
    DWORD bytesLeidos = 0;
    if (!ReadFile(manejadorArchivo, buferBytes.data(), bytesALeer,
                  &bytesLeidos, nullptr) ||
        bytesLeidos == 0) {
        CloseHandle(manejadorArchivo);
        return {};
    }
    CloseHandle(manejadorArchivo);

    const unsigned char* bytesRaw =
        reinterpret_cast<const unsigned char*>(buferBytes.data());
    DWORD longitudEfectiva = bytesLeidos;

    // Detección de BOM UTF-16 LE (0xFF, 0xFE)
    if (longitudEfectiva >= 2 && bytesRaw[0] == 0xFF && bytesRaw[1] == 0xFE) {
        size_t cantidadCaracteres = (longitudEfectiva - 2) / sizeof(wchar_t);
        const wchar_t* punteroW =
            reinterpret_cast<const wchar_t*>(buferBytes.data() + 2);
        return std::wstring(punteroW, cantidadCaracteres);
    }

    // Detección de BOM UTF-8 (0xEF, 0xBB, 0xBF)
    const char* punteroTexto = buferBytes.data();
    if (longitudEfectiva >= 3 &&
        bytesRaw[0] == 0xEF && bytesRaw[1] == 0xBB && bytesRaw[2] == 0xBF) {
        punteroTexto += 3;
        longitudEfectiva -= 3;
    }

    int longitudWide = MultiByteToWideChar(
        CP_UTF8, 0, punteroTexto, static_cast<int>(longitudEfectiva), nullptr,
        0);
    if (longitudWide <= 0) {
        longitudWide = MultiByteToWideChar(
            CP_ACP, 0, punteroTexto, static_cast<int>(longitudEfectiva),
            nullptr, 0);
        if (longitudWide <= 0) return {};
        std::wstring textoWide(static_cast<size_t>(longitudWide), L'\0');
        MultiByteToWideChar(CP_ACP, 0, punteroTexto,
                           static_cast<int>(longitudEfectiva),
                           textoWide.data(), longitudWide);
        return textoWide;
    }

    std::wstring textoWide(static_cast<size_t>(longitudWide), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, punteroTexto,
                       static_cast<int>(longitudEfectiva), textoWide.data(),
                       longitudWide);
    return textoWide;
}

inline std::wstring_view recortarEspaciosVista(std::wstring_view cadenaVista) {
    size_t primero = cadenaVista.find_first_not_of(L" \t\r\n");
    if (primero == std::wstring_view::npos) return {};
    size_t ultimo = cadenaVista.find_last_not_of(L" \t\r\n");
    return cadenaVista.substr(primero, ultimo - primero + 1);
}

inline std::wstring recortarEspacios(const std::wstring& cadena) {
    size_t primero = cadena.find_first_not_of(L" \t\r\n");
    if (primero == std::wstring::npos) return L"";
    size_t ultimo = cadena.find_last_not_of(L" \t\r\n");
    return cadena.substr(primero, ultimo - primero + 1);
}

inline std::vector<FragmentoTexto> parsearFragmentosTexto(const std::wstring& texto) {
    std::vector<FragmentoTexto> fragmentos;
    size_t longitudTexto = texto.size();
    size_t posicionActual = 0;
    std::wstring acumulado;

    while (posicionActual < longitudTexto) {
        if (texto[posicionActual] == L'`') {
            size_t posicionFin = texto.find(L'`', posicionActual + 1);
            if (posicionFin != std::wstring::npos) {
                if (!acumulado.empty()) {
                    fragmentos.push_back({acumulado, EstiloFragmento::Normal});
                    acumulado.clear();
                }
                fragmentos.push_back({texto.substr(posicionActual + 1, posicionFin - posicionActual - 1), EstiloFragmento::CodigoPildora});
                posicionActual = posicionFin + 1;
                continue;
            }
        }
        if (posicionActual + 1 < longitudTexto && texto[posicionActual] == L'*' && texto[posicionActual + 1] == L'*') {
            size_t posicionFin = texto.find(L"**", posicionActual + 2);
            if (posicionFin != std::wstring::npos) {
                if (!acumulado.empty()) {
                    fragmentos.push_back({acumulado, EstiloFragmento::Normal});
                    acumulado.clear();
                }
                fragmentos.push_back({texto.substr(posicionActual + 2, posicionFin - posicionActual - 2), EstiloFragmento::Negrita});
                posicionActual = posicionFin + 2;
                continue;
            }
        }
        if (posicionActual + 1 < longitudTexto && texto[posicionActual] == L'=' && texto[posicionActual + 1] == L'=') {
            size_t posicionFin = texto.find(L"==", posicionActual + 2);
            if (posicionFin != std::wstring::npos) {
                if (!acumulado.empty()) {
                    fragmentos.push_back({acumulado, EstiloFragmento::Normal});
                    acumulado.clear();
                }
                fragmentos.push_back({texto.substr(posicionActual + 2, posicionFin - posicionActual - 2), EstiloFragmento::Subrayado});
                posicionActual = posicionFin + 2;
                continue;
            }
        }
        if (texto[posicionActual] == L'_') {
            size_t posicionFin = texto.find(L'_', posicionActual + 1);
            if (posicionFin != std::wstring::npos && posicionFin > posicionActual + 1) {
                if (!acumulado.empty()) {
                    fragmentos.push_back({acumulado, EstiloFragmento::Normal});
                    acumulado.clear();
                }
                fragmentos.push_back({texto.substr(posicionActual + 1, posicionFin - posicionActual - 1), EstiloFragmento::Cursiva});
                posicionActual = posicionFin + 1;
                continue;
            }
        }
        acumulado += texto[posicionActual];
        posicionActual++;
    }

    if (!acumulado.empty()) {
        fragmentos.push_back({acumulado, EstiloFragmento::Normal});
    }

    return fragmentos;
}

inline void parsearContenidoMarkdown(const std::wstring& contenidoCompleto) {
    estadoTexto.elementosMarkdown.clear();
    estadoTexto.barrasIndice.clear();

    if (contenidoCompleto.empty()) {
        ElementoMarkdown elementoVacio;
        elementoVacio.tipo = TipoElementoMarkdown::Parrafo;
        elementoVacio.textoContenido = L"*(Archivo sin contenido)*";
        estadoTexto.elementosMarkdown.push_back(elementoVacio);
        return;
    }

    bool dentroDeBloqueCodigo = false;
    std::wstring acumuladorBloqueCodigo;
    std::wstring lenguajeCodigo;

    size_t posicionInicio = 0;
    size_t longitudTexto = contenidoCompleto.size();

    while (posicionInicio < longitudTexto) {
        size_t posicionFin = contenidoCompleto.find(L'\n', posicionInicio);
        if (posicionFin == std::wstring::npos) {
            posicionFin = longitudTexto;
        }

        std::wstring lineaActual = contenidoCompleto.substr(
            posicionInicio, posicionFin - posicionInicio);
        if (!lineaActual.empty() && lineaActual.back() == L'\r') {
            lineaActual.pop_back();
        }

        posicionInicio = posicionFin + 1;
        estadoTexto.totalLineasOriginales++;

        if (estadoTexto.elementosMarkdown.size() >= 2500) {
            ElementoMarkdown elementoAviso;
            elementoAviso.tipo = TipoElementoMarkdown::Parrafo;
            elementoAviso.textoContenido = L"... [Vista previa limitada a los primeros 2,500 bloques por rendimiento]";
            estadoTexto.elementosMarkdown.push_back(elementoAviso);
            break;
        }

        // Detección de cercado de código ```
        if (lineaActual.rfind(L"```", 0) == 0) {
            if (dentroDeBloqueCodigo) {
                ElementoMarkdown elementoCodigo;
                elementoCodigo.tipo = TipoElementoMarkdown::BloqueCodigo;
                elementoCodigo.textoContenido = acumuladorBloqueCodigo;
                elementoCodigo.textoSecundario = lenguajeCodigo;
                estadoTexto.elementosMarkdown.push_back(elementoCodigo);

                acumuladorBloqueCodigo.clear();
                lenguajeCodigo.clear();
                dentroDeBloqueCodigo = false;
            } else {
                dentroDeBloqueCodigo = true;
                if (lineaActual.size() > 3) {
                    lenguajeCodigo = lineaActual.substr(3);
                }
            }
            continue;
        }

        if (dentroDeBloqueCodigo) {
            acumuladorBloqueCodigo += lineaActual + L"\n";
            continue;
        }

        // Línea vacía
        if (lineaActual.find_first_not_of(L" \t") == std::wstring::npos) {
            ElementoMarkdown elementoVacio;
            elementoVacio.tipo = TipoElementoMarkdown::Parrafo;
            elementoVacio.textoContenido = L"";
            estadoTexto.elementosMarkdown.push_back(elementoVacio);
            continue;
        }

        // Tablas Markdown
        if (lineaActual.find(L'|') != std::wstring::npos) {
            auto dividirCeldas = [](const std::wstring& linea) -> std::vector<std::wstring> {
                std::vector<std::wstring> celdas;
                size_t inicio = 0;
                if (!linea.empty() && linea.front() == L'|') inicio = 1;
                size_t fin = linea.size();
                if (!linea.empty() && linea.back() == L'|') fin = linea.size() - 1;
                size_t posicionCelda = inicio;
                while (posicionCelda < fin) {
                    size_t posicionProximoSeparador = linea.find(L'|', posicionCelda);
                    if (posicionProximoSeparador == std::wstring::npos || posicionProximoSeparador > fin) {
                        posicionProximoSeparador = fin;
                    }
                    std::wstring_view vistaSub = std::wstring_view(linea).substr(posicionCelda, posicionProximoSeparador - posicionCelda);
                    std::wstring_view vistaRecortada = recortarEspaciosVista(vistaSub);
                    celdas.emplace_back(vistaRecortada);
                    posicionCelda = posicionProximoSeparador + 1;
                }
                return celdas;
            };

            auto esFilaSeparador = [](const std::wstring& linea) -> bool {
                for (wchar_t caracterSeparador : linea) {
                    if (caracterSeparador != L'|' && caracterSeparador != L'-' && caracterSeparador != L':' &&
                        caracterSeparador != L' ' && caracterSeparador != L'\t') {
                        return false;
                    }
                }
                return linea.find(L'-') != std::wstring::npos;
            };

            if (!estadoTexto.elementosMarkdown.empty() &&
                estadoTexto.elementosMarkdown.back().tipo == TipoElementoMarkdown::TablaMarkdown) {
                if (!esFilaSeparador(lineaActual)) {
                    FilaTablaMarkdown fila;
                    fila.celdas = dividirCeldas(lineaActual);
                    fila.esCabecera = false;
                    if (!fila.celdas.empty()) {
                        estadoTexto.elementosMarkdown.back().filasTabla.push_back(fila);
                    }
                }
                continue;
            } else {
                ElementoMarkdown elementoTabla;
                elementoTabla.tipo = TipoElementoMarkdown::TablaMarkdown;
                FilaTablaMarkdown filaCabecera;
                filaCabecera.celdas = dividirCeldas(lineaActual);
                filaCabecera.esCabecera = true;
                if (!filaCabecera.celdas.empty()) {
                    elementoTabla.filasTabla.push_back(filaCabecera);
                    estadoTexto.elementosMarkdown.push_back(elementoTabla);
                }
                continue;
            }
        }

        // Sintaxis especial de separador con título: ---[texto]---
        std::wstring lineaRecortada = recortarEspacios(lineaActual);
        if (lineaRecortada.rfind(L"---[", 0) == 0 &&
            lineaRecortada.size() >= 7 &&
            lineaRecortada.rfind(L"]---") == lineaRecortada.size() - 4) {
            std::wstring tituloSeparador = lineaRecortada.substr(4, lineaRecortada.size() - 8);
            tituloSeparador = recortarEspacios(tituloSeparador);

            ElementoMarkdown elementoSep;
            elementoSep.tipo = TipoElementoMarkdown::SeparadorSeccionPersonalizado;
            elementoSep.textoContenido = tituloSeparador;
            elementoSep.nivelEncabezado = 2;
            estadoTexto.elementosMarkdown.push_back(elementoSep);

            if (!tituloSeparador.empty()) {
                ElementoIndiceBarras barraIndice;
                barraIndice.tituloEncabezado = tituloSeparador;
                barraIndice.nivelEncabezado = 2;
                barraIndice.posicionDestinoY = 0;
                estadoTexto.barrasIndice.push_back(barraIndice);
            }
            continue;
        }

        // Regla horizontal --- o ***
        if (lineaActual == L"---" || lineaActual == L"***" ||
            lineaActual == L"___") {
            ElementoMarkdown elementoRegla;
            elementoRegla.tipo = TipoElementoMarkdown::ReglaHorizontal;
            estadoTexto.elementosMarkdown.push_back(elementoRegla);
            continue;
        }

        // Encabezados #
        if (lineaActual.rfind(L"#### ", 0) == 0) {
            ElementoMarkdown elementoH4;
            elementoH4.tipo = TipoElementoMarkdown::Encabezado4;
            elementoH4.textoContenido = lineaActual.substr(5);
            elementoH4.nivelEncabezado = 4;
            estadoTexto.elementosMarkdown.push_back(elementoH4);
            continue;
        }
        if (lineaActual.rfind(L"### ", 0) == 0) {
            ElementoMarkdown elementoH3;
            elementoH3.tipo = TipoElementoMarkdown::Encabezado3;
            elementoH3.textoContenido = lineaActual.substr(4);
            elementoH3.nivelEncabezado = 3;
            estadoTexto.elementosMarkdown.push_back(elementoH3);
            continue;
        }
        if (lineaActual.rfind(L"## ", 0) == 0) {
            ElementoMarkdown elementoH2;
            elementoH2.tipo = TipoElementoMarkdown::Encabezado2;
            elementoH2.textoContenido = lineaActual.substr(3);
            elementoH2.nivelEncabezado = 2;
            estadoTexto.elementosMarkdown.push_back(elementoH2);
            continue;
        }
        if (lineaActual.rfind(L"# ", 0) == 0) {
            ElementoMarkdown elementoH1;
            elementoH1.tipo = TipoElementoMarkdown::Encabezado1;
            elementoH1.textoContenido = lineaActual.substr(2);
            elementoH1.nivelEncabezado = 1;
            estadoTexto.elementosMarkdown.push_back(elementoH1);
            continue;
        }

        // Citas en bloque >
        if (lineaActual.rfind(L"> ", 0) == 0 || lineaActual == L">") {
            ElementoMarkdown elementoCita;
            elementoCita.tipo = TipoElementoMarkdown::CitaBloque;
            elementoCita.textoContenido =
                (lineaActual.size() > 2) ? lineaActual.substr(2) : L"";
            estadoTexto.elementosMarkdown.push_back(elementoCita);
            continue;
        }

        // Casillas de verificación - [ ] o - [x]
        if (lineaActual.rfind(L"- [ ] ", 0) == 0 ||
            lineaActual.rfind(L"* [ ] ", 0) == 0) {
            ElementoMarkdown elementoCasilla;
            elementoCasilla.tipo = TipoElementoMarkdown::CasillaVerificacion;
            elementoCasilla.casillaMarcada = false;
            elementoCasilla.textoContenido = lineaActual.substr(6);
            estadoTexto.elementosMarkdown.push_back(elementoCasilla);
            continue;
        }
        if (lineaActual.rfind(L"- [x] ", 0) == 0 ||
            lineaActual.rfind(L"- [X] ", 0) == 0 ||
            lineaActual.rfind(L"* [x] ", 0) == 0) {
            ElementoMarkdown elementoCasilla;
            elementoCasilla.tipo = TipoElementoMarkdown::CasillaVerificacion;
            elementoCasilla.casillaMarcada = true;
            elementoCasilla.textoContenido = lineaActual.substr(6);
            estadoTexto.elementosMarkdown.push_back(elementoCasilla);
            continue;
        }

        // Listas ordenadas (1. , 2. , etc.)
        if (lineaActual.size() >= 3 && iswdigit(lineaActual[0])) {
            size_t punto = lineaActual.find(L". ");
            if (punto != std::wstring::npos && punto <= 4) {
                ElementoMarkdown elementoListaOrd;
                elementoListaOrd.tipo = TipoElementoMarkdown::ListaOrdenada;
                elementoListaOrd.textoSecundario = lineaActual.substr(0, punto + 1);
                elementoListaOrd.textoContenido = lineaActual.substr(punto + 2);
                estadoTexto.elementosMarkdown.push_back(elementoListaOrd);
                continue;
            }
        }

        // Listas desordenadas - o *
        if (lineaActual.rfind(L"- ", 0) == 0 ||
            lineaActual.rfind(L"* ", 0) == 0) {
            ElementoMarkdown elementoLista;
            elementoLista.tipo = TipoElementoMarkdown::ListaDesordenada;
            elementoLista.textoContenido = lineaActual.substr(2);
            estadoTexto.elementosMarkdown.push_back(elementoLista);
            continue;
        }

        // Imágenes ![alt](url)
        if (lineaActual.rfind(L"![", 0) == 0) {
            size_t posicionCierreAlt = lineaActual.find(L"](");
            size_t posicionCierreUrl = lineaActual.find(L')', posicionCierreAlt);
            if (posicionCierreAlt != std::wstring::npos &&
                posicionCierreUrl != std::wstring::npos) {
                std::wstring textoAlt =
                    lineaActual.substr(2, posicionCierreAlt - 2);
                std::wstring rutaUrl = lineaActual.substr(
                    posicionCierreAlt + 2,
                    posicionCierreUrl - posicionCierreAlt - 2);

                std::wstring idYouTube = extraerIdVideoYouTube(rutaUrl);
                if (!idYouTube.empty()) {
                    ElementoMarkdown elementoVideo;
                    elementoVideo.tipo = TipoElementoMarkdown::VideoYouTube;
                    elementoVideo.textoContenido =
                        textoAlt.empty() ? L"Video de YouTube" : textoAlt;
                    elementoVideo.textoSecundario = rutaUrl;
                    estadoTexto.elementosMarkdown.push_back(elementoVideo);
                } else {
                    ElementoMarkdown elementoImagen;
                    elementoImagen.tipo = TipoElementoMarkdown::Imagen;
                    elementoImagen.textoContenido = textoAlt;
                    elementoImagen.textoSecundario = rutaUrl;
                    estadoTexto.elementosMarkdown.push_back(elementoImagen);
                }
                continue;
            }
        }

        // Enlaces [texto](url) o video YouTube directo
        std::wstring idDirectoYouTube = extraerIdVideoYouTube(lineaActual);
        if (!idDirectoYouTube.empty()) {
            ElementoMarkdown elementoVideo;
            elementoVideo.tipo = TipoElementoMarkdown::VideoYouTube;
            elementoVideo.textoContenido = L"Reproducir video de YouTube";
            elementoVideo.textoSecundario = lineaActual;
            estadoTexto.elementosMarkdown.push_back(elementoVideo);
            continue;
        }

        if (lineaActual.rfind(L"[", 0) == 0) {
            size_t posicionCierreTexto = lineaActual.find(L"](");
            size_t posicionCierreUrl =
                lineaActual.find(L')', posicionCierreTexto);
            if (posicionCierreTexto != std::wstring::npos &&
                posicionCierreUrl != std::wstring::npos) {
                std::wstring textoEnlace =
                    lineaActual.substr(1, posicionCierreTexto - 1);
                std::wstring rutaUrl = lineaActual.substr(
                    posicionCierreTexto + 2,
                    posicionCierreUrl - posicionCierreTexto - 2);

                std::wstring idYt = extraerIdVideoYouTube(rutaUrl);
                if (!idYt.empty()) {
                    ElementoMarkdown elementoVideo;
                    elementoVideo.tipo = TipoElementoMarkdown::VideoYouTube;
                    elementoVideo.textoContenido =
                        textoEnlace.empty() ? L"Video de YouTube" : textoEnlace;
                    elementoVideo.textoSecundario = rutaUrl;
                    estadoTexto.elementosMarkdown.push_back(elementoVideo);
                } else {
                    ElementoMarkdown elementoEnlace;
                    elementoEnlace.tipo = TipoElementoMarkdown::EnlaceWeb;
                    elementoEnlace.textoContenido = textoEnlace;
                    elementoEnlace.textoSecundario = rutaUrl;
                    estadoTexto.elementosMarkdown.push_back(elementoEnlace);
                }
                continue;
            }
        }

        // Párrafo estándar
        ElementoMarkdown elementoParrafo;
        elementoParrafo.tipo = TipoElementoMarkdown::Parrafo;
        elementoParrafo.textoContenido = lineaActual;
        estadoTexto.elementosMarkdown.push_back(elementoParrafo);
    }

    if (dentroDeBloqueCodigo && !acumuladorBloqueCodigo.empty()) {
        ElementoMarkdown elementoCodigo;
        elementoCodigo.tipo = TipoElementoMarkdown::BloqueCodigo;
        elementoCodigo.textoContenido = acumuladorBloqueCodigo;
        elementoCodigo.textoSecundario = lenguajeCodigo;
        estadoTexto.elementosMarkdown.push_back(elementoCodigo);
    }
}

inline void parsearContenidoTextoPlano(const std::wstring& contenidoCompleto) {
    estadoTexto.elementosMarkdown.clear();
    estadoTexto.barrasIndice.clear();

    if (contenidoCompleto.empty()) {
        ElementoMarkdown elementoVacio;
        elementoVacio.tipo = TipoElementoMarkdown::Parrafo;
        elementoVacio.textoContenido = L"*(Archivo de texto sin contenido)*";
        estadoTexto.elementosMarkdown.push_back(elementoVacio);
        return;
    }

    size_t posicionInicio = 0;
    size_t longitudTexto = contenidoCompleto.size();

    while (posicionInicio < longitudTexto) {
        size_t posicionFin = contenidoCompleto.find(L'\n', posicionInicio);
        if (posicionFin == std::wstring::npos) {
            posicionFin = longitudTexto;
        }

        std::wstring lineaActual = contenidoCompleto.substr(
            posicionInicio, posicionFin - posicionInicio);
        if (!lineaActual.empty() && lineaActual.back() == L'\r') {
            lineaActual.pop_back();
        }

        posicionInicio = posicionFin + 1;
        estadoTexto.totalLineasOriginales++;

        if (estadoTexto.elementosMarkdown.size() >= 2500) {
            ElementoMarkdown elementoAviso;
            elementoAviso.tipo = TipoElementoMarkdown::Parrafo;
            elementoAviso.textoContenido = L"... [Vista previa limitada a las primeras 2,500 líneas por rendimiento]";
            estadoTexto.elementosMarkdown.push_back(elementoAviso);
            break;
        }

        ElementoMarkdown elementoLinea;
        elementoLinea.tipo = TipoElementoMarkdown::Parrafo;
        elementoLinea.textoContenido = lineaActual;
        estadoTexto.elementosMarkdown.push_back(elementoLinea);
    }
}

inline bool cargarTextoPorRuta(const std::wstring& rutaTexto) {
    if (rutaTexto.empty()) {
        limpiarContenidoTextoCargado();
        return false;
    }

    limpiarContenidoTextoCargado();
    estadoTexto.rutaTextoActual = rutaTexto;

    const wchar_t* punteroNombre = PathFindFileNameW(rutaTexto.c_str());
    estadoTexto.nombreArchivo =
        punteroNombre ? punteroNombre : rutaTexto;

    const wchar_t* extension = PathFindExtensionW(rutaTexto.c_str());
    estadoTexto.esFormatoMarkdown = esExtensionMarkdown(extension);

    std::vector<std::wstring> listaArchivosTexto =
        obtenerArchivosTextoEnMismaCarpeta(rutaTexto);
    estadoTexto.totalEnCarpeta =
        std::max<size_t>(1, listaArchivosTexto.size());
    auto iterador = std::find(listaArchivosTexto.begin(), listaArchivosTexto.end(),
                              rutaTexto);
    if (iterador != listaArchivosTexto.end()) {
        estadoTexto.posicionEnCarpeta =
            static_cast<size_t>(
                std::distance(listaArchivosTexto.begin(), iterador)) +
            1;
    } else {
        estadoTexto.posicionEnCarpeta = 1;
    }

    std::wstring contenidoTexto = leerArchivoTextoUtf8(rutaTexto);
    if (estadoTexto.esFormatoMarkdown) {
        parsearContenidoMarkdown(contenidoTexto);
    } else {
        parsearContenidoTextoPlano(contenidoTexto);
    }
    return true;
}

inline bool mostrarTextoAnterior() {
    if (estadoTexto.textosSeleccionados.size() > 1) {
        estadoTexto.indiceTextoSeleccionado =
            (estadoTexto.indiceTextoSeleccionado == 0)
                ? (estadoTexto.textosSeleccionados.size() - 1)
                : (estadoTexto.indiceTextoSeleccionado - 1);
        return cargarTextoPorRuta(
            estadoTexto.textosSeleccionados[estadoTexto.indiceTextoSeleccionado]);
    }
    if (estadoTexto.rutaTextoActual.empty()) return false;
    std::wstring rutaAnterior =
        obtenerArchivoTextoAnteriorEnCarpeta(estadoTexto.rutaTextoActual);
    if (rutaAnterior.empty() ||
        rutaAnterior == estadoTexto.rutaTextoActual) {
        return false;
    }
    estadoTexto.textosSeleccionados = { rutaAnterior };
    estadoTexto.indiceTextoSeleccionado = 0;
    return cargarTextoPorRuta(rutaAnterior);
}

inline bool mostrarTextoSiguiente() {
    if (estadoTexto.textosSeleccionados.size() > 1) {
        estadoTexto.indiceTextoSeleccionado =
            (estadoTexto.indiceTextoSeleccionado + 1) %
            estadoTexto.textosSeleccionados.size();
        return cargarTextoPorRuta(
            estadoTexto.textosSeleccionados[estadoTexto.indiceTextoSeleccionado]);
    }
    if (estadoTexto.rutaTextoActual.empty()) return false;
    std::wstring rutaSiguiente =
        obtenerArchivoTextoSiguienteEnCarpeta(estadoTexto.rutaTextoActual);
    if (rutaSiguiente.empty() ||
        rutaSiguiente == estadoTexto.rutaTextoActual) {
        return false;
    }
    estadoTexto.textosSeleccionados = { rutaSiguiente };
    estadoTexto.indiceTextoSeleccionado = 0;
    return cargarTextoPorRuta(rutaSiguiente);
}

inline void desplazarScrollTexto(int incrementoDelta) {
    int maximoScroll = std::max(0, estadoTexto.altoTotalContenido - 200);
    estadoTexto.desplazamientoScrollY = std::max(
        0, std::min(estadoTexto.desplazamientoScrollY + incrementoDelta,
                    maximoScroll));
}

inline int dibujarFragmentosEnriquecidos(
    HDC hdc, const std::wstring& texto, int xInicio, int yInicio, int anchoMax,
    HFONT fuenteCuerpo, HFONT fuenteNegrita, HFONT fuenteCursiva,
    HFONT fuenteSubrayada, HFONT fuenteCodigoPildora) {
    if (texto.empty()) return 0;

    std::vector<FragmentoTexto> fragmentos = parsearFragmentosTexto(texto);
    if (fragmentos.size() == 1 && fragmentos[0].estilo == EstiloFragmento::Normal) {
        SelectObject(hdc, fuenteCuerpo);
        SetTextColor(hdc, RGB(230, 237, 243));
        RECT r{xInicio, yInicio, xInicio + anchoMax, yInicio + 2000};
        int alto = DrawTextW(hdc, texto.c_str(), -1, &r, DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
        DrawTextW(hdc, texto.c_str(), -1, &r, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
        return alto;
    }

    int xActual = xInicio;
    int yActual = yInicio;
    int altoLinea = 20;

    for (const auto& frag : fragmentos) {
        HFONT fuenteElegida = fuenteCuerpo;
        COLORREF colorTexto = RGB(230, 237, 243);

        if (frag.estilo == EstiloFragmento::Negrita) {
            fuenteElegida = fuenteNegrita;
            colorTexto = RGB(255, 255, 255);
        } else if (frag.estilo == EstiloFragmento::Cursiva) {
            fuenteElegida = fuenteCursiva;
            colorTexto = RGB(230, 237, 243);
        } else if (frag.estilo == EstiloFragmento::Subrayado) {
            fuenteElegida = fuenteSubrayada;
            colorTexto = RGB(88, 166, 255);
        } else if (frag.estilo == EstiloFragmento::CodigoPildora) {
            fuenteElegida = fuenteCodigoPildora;
            colorTexto = RGB(240, 136, 62);
        }

        SelectObject(hdc, fuenteElegida);
        SetTextColor(hdc, colorTexto);

        size_t p = 0;
        while (p < frag.texto.size()) {
            size_t proxEspacio = frag.texto.find_first_of(L" \t", p);
            if (proxEspacio == std::wstring::npos) proxEspacio = frag.texto.size();
            std::wstring palabra = frag.texto.substr(p, proxEspacio - p);
            if (proxEspacio < frag.texto.size()) palabra += frag.texto[proxEspacio];

            SIZE tamPalabra{0, 0};
            GetTextExtentPoint32W(hdc, palabra.c_str(), static_cast<int>(palabra.size()), &tamPalabra);

            if (xActual + tamPalabra.cx > xInicio + anchoMax && xActual > xInicio) {
                xActual = xInicio;
                yActual += altoLinea;
            }

            if (frag.estilo == EstiloFragmento::CodigoPildora) {
                RECT rPildora{xActual - 2, yActual + 1, xActual + tamPalabra.cx + 2, yActual + altoLinea - 1};
                HBRUSH bPildora = CreateSolidBrush(RGB(45, 51, 59));
                HPEN lPildora = CreatePen(PS_SOLID, 1, RGB(68, 76, 86));
                HBRUSH bPrev = static_cast<HBRUSH>(SelectObject(hdc, bPildora));
                HPEN lPrev = static_cast<HPEN>(SelectObject(hdc, lPildora));
                RoundRect(hdc, rPildora.left, rPildora.top, rPildora.right, rPildora.bottom, 4, 4);
                SelectObject(hdc, lPrev);
                SelectObject(hdc, bPrev);
                DeleteObject(lPildora);
                DeleteObject(bPildora);
            }

            RECT rTexto{xActual, yActual, xActual + tamPalabra.cx, yActual + altoLinea};
            DrawTextW(hdc, palabra.c_str(), -1, &rTexto, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            xActual += tamPalabra.cx;
            p = (proxEspacio < frag.texto.size()) ? proxEspacio + 1 : frag.texto.size();
        }
    }

    return (yActual - yInicio) + altoLinea;
}

inline void dibujarLienzoTexto(HDC contextoDestino,
                               const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    // Fondo GitHub Dark: RGB(13, 17, 23)
    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteH1 = CreateFontW(
        -22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteH2 = CreateFontW(
        -18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteH3 = CreateFontW(
        -15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteH4 = CreateFontW(
        -13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteCuerpo = CreateFontW(
        -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteNegrita = CreateFontW(
        -13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteCursiva = CreateFontW(
        -13, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubrayada = CreateFontW(
        -13, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteCodigo = CreateFontW(
        -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
    HFONT fuenteCodigoPildora = CreateFontW(
        -12, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
    HFONT fuentePequena = CreateFontW(
        -11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int margenIzquierdo = areaDibujo.left + 24;
    int margenDerechoTexto = areaDibujo.right - 44;

    int posicionVerticalActual =
        areaDibujo.top + 20 - estadoTexto.desplazamientoScrollY;

    estadoTexto.barrasIndice.clear();

    for (size_t posicionElemento = 0;
         posicionElemento < estadoTexto.elementosMarkdown.size();
         ++posicionElemento) {
        ElementoMarkdown& elemento =
            estadoTexto.elementosMarkdown[posicionElemento];
        elemento.posicionVerticalY = posicionVerticalActual;

        if (elemento.altoCalculado == 0) {
            if (elemento.tipo == TipoElementoMarkdown::Encabezado1) {
                elemento.altoCalculado = 36;
            } else if (elemento.tipo == TipoElementoMarkdown::Encabezado2) {
                elemento.altoCalculado = 32;
            } else if (elemento.tipo == TipoElementoMarkdown::Encabezado3) {
                elemento.altoCalculado = 28;
            } else if (elemento.tipo == TipoElementoMarkdown::Encabezado4) {
                elemento.altoCalculado = 24;
            } else if (elemento.tipo == TipoElementoMarkdown::ReglaHorizontal) {
                elemento.altoCalculado = 14;
            } else if (elemento.tipo == TipoElementoMarkdown::Imagen) {
                elemento.altoCalculado = 180;
            } else if (elemento.tipo == TipoElementoMarkdown::VideoYouTube) {
                elemento.altoCalculado = 240;
            } else {
                elemento.altoCalculado = elemento.textoContenido.empty() ? 8 : 22;
            }
        }

        bool esVisible =
            (posicionVerticalActual + elemento.altoCalculado >=
                 areaDibujo.top - 100 &&
             posicionVerticalActual <= areaDibujo.bottom + 100);

        if (!esVisible) {
            if (estadoTexto.esFormatoMarkdown) {
                if (elemento.tipo == TipoElementoMarkdown::Encabezado1) {
                    ElementoIndiceBarras barraH1;
                    barraH1.tituloEncabezado = elemento.textoContenido;
                    barraH1.nivelEncabezado = 1;
                    barraH1.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH1);
                } else if (elemento.tipo == TipoElementoMarkdown::Encabezado2) {
                    ElementoIndiceBarras barraH2;
                    barraH2.tituloEncabezado = elemento.textoContenido;
                    barraH2.nivelEncabezado = 2;
                    barraH2.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH2);
                } else if (elemento.tipo == TipoElementoMarkdown::Encabezado3) {
                    ElementoIndiceBarras barraH3;
                    barraH3.tituloEncabezado = elemento.textoContenido;
                    barraH3.nivelEncabezado = 3;
                    barraH3.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH3);
                }
            }
            posicionVerticalActual += elemento.altoCalculado;
            continue;
        }

        switch (elemento.tipo) {
            case TipoElementoMarkdown::Encabezado1: {
                SelectObject(contextoDestino, fuenteH1);
                SetTextColor(contextoDestino, RGB(245, 245, 245));
                RECT areaEncabezado{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEncabezado,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEncabezado,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                // Línea divisoria inferior GitHub H1
                HPEN lapizDivisor = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizDivisor));
                MoveToEx(contextoDestino, margenIzquierdo,
                         posicionVerticalActual + altoTexto + 6, nullptr);
                LineTo(contextoDestino, margenDerechoTexto,
                       posicionVerticalActual + altoTexto + 6);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(lapizDivisor);

                if (estadoTexto.esFormatoMarkdown) {
                    ElementoIndiceBarras barraH1;
                    barraH1.tituloEncabezado = elemento.textoContenido;
                    barraH1.nivelEncabezado = 1;
                    barraH1.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH1);
                }

                elemento.altoCalculado = altoTexto + 18;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Encabezado2: {
                SelectObject(contextoDestino, fuenteH2);
                SetTextColor(contextoDestino, RGB(245, 245, 245));
                RECT areaEncabezado{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEncabezado,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEncabezado,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                // Línea divisoria inferior GitHub H2
                HPEN lapizDivisor = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizDivisor));
                MoveToEx(contextoDestino, margenIzquierdo,
                         posicionVerticalActual + altoTexto + 4, nullptr);
                LineTo(contextoDestino, margenDerechoTexto,
                       posicionVerticalActual + altoTexto + 4);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(lapizDivisor);

                if (estadoTexto.esFormatoMarkdown) {
                    ElementoIndiceBarras barraH2;
                    barraH2.tituloEncabezado = elemento.textoContenido;
                    barraH2.nivelEncabezado = 2;
                    barraH2.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH2);
                }

                elemento.altoCalculado = altoTexto + 14;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Encabezado3: {
                SelectObject(contextoDestino, fuenteH3);
                SetTextColor(contextoDestino, RGB(245, 245, 245));
                RECT areaEncabezado{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEncabezado,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEncabezado,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                if (estadoTexto.esFormatoMarkdown) {
                    ElementoIndiceBarras barraH3;
                    barraH3.tituloEncabezado = elemento.textoContenido;
                    barraH3.nivelEncabezado = 3;
                    barraH3.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH3);
                }

                elemento.altoCalculado = altoTexto + 10;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Encabezado4: {
                SelectObject(contextoDestino, fuenteH4);
                SetTextColor(contextoDestino, RGB(220, 220, 220));
                RECT areaEncabezado{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEncabezado,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEncabezado,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                elemento.altoCalculado = altoTexto + 8;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::BloqueCodigo: {
                SelectObject(contextoDestino, fuenteCodigo);
                RECT areaCalculo{margenIzquierdo + 12,
                                 posicionVerticalActual + 10,
                                 margenDerechoTexto - 12,
                                 posicionVerticalActual + 2000};
                int altoCodigo = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaCalculo,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);

                int altoCaja = altoCodigo + 20;
                RECT areaCajaCodigo{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + altoCaja};

                HBRUSH brochaCaja = CreateSolidBrush(RGB(22, 27, 34));
                HPEN lapizCaja = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HBRUSH brochaPreviaCaja = static_cast<HBRUSH>(
                    SelectObject(contextoDestino, brochaCaja));
                HPEN lapizPrevioCaja = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizCaja));

                RoundRect(contextoDestino, areaCajaCodigo.left,
                          areaCajaCodigo.top, areaCajaCodigo.right,
                          areaCajaCodigo.bottom, 6, 6);

                SelectObject(contextoDestino, brochaPreviaCaja);
                SelectObject(contextoDestino, lapizPrevioCaja);
                DeleteObject(brochaCaja);
                DeleteObject(lapizCaja);

                SetTextColor(contextoDestino, RGB(201, 209, 217));
                RECT areaTextoCodigo{
                    margenIzquierdo + 12, posicionVerticalActual + 10,
                    margenDerechoTexto - 12, posicionVerticalActual + altoCaja - 10};
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaTextoCodigo,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                elemento.altoCalculado = altoCaja + 12;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::CitaBloque: {
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino, RGB(139, 148, 158));
                RECT areaCita{margenIzquierdo + 16, posicionVerticalActual,
                               margenDerechoTexto,
                               posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaCita,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaCita, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                // Barra vertical izquierda azul Fluent/GitHub
                HBRUSH brochaBarra = CreateSolidBrush(RGB(88, 166, 255));
                RECT areaBarraCita{margenIzquierdo, posicionVerticalActual,
                                   margenIzquierdo + 4,
                                   posicionVerticalActual + altoTexto};
                FillRect(contextoDestino, &areaBarraCita, brochaBarra);
                DeleteObject(brochaBarra);

                elemento.altoCalculado = altoTexto + 8;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::ListaDesordenada: {
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino, RGB(88, 166, 255));
                RECT areaVina{margenIzquierdo + 4, posicionVerticalActual,
                              margenIzquierdo + 18,
                              posicionVerticalActual + 20};
                DrawTextW(contextoDestino, L"\u2022", -1, &areaVina,
                          DT_LEFT | DT_NOPREFIX);

                SetTextColor(contextoDestino, RGB(230, 237, 243));
                RECT areaTexto{margenIzquierdo + 20, posicionVerticalActual,
                               margenDerechoTexto,
                               posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaTexto,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaTexto, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                elemento.altoCalculado = altoTexto + 6;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::CasillaVerificacion: {
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino,
                             elemento.casillaMarcada ? RGB(88, 166, 255)
                                                     : RGB(139, 148, 158));
                RECT areaCasilla{margenIzquierdo + 4, posicionVerticalActual,
                                 margenIzquierdo + 22,
                                 posicionVerticalActual + 20};
                DrawTextW(contextoDestino,
                          elemento.casillaMarcada ? L"\u2611" : L"\u2610", -1,
                          &areaCasilla, DT_LEFT | DT_NOPREFIX);

                SetTextColor(contextoDestino, elemento.casillaMarcada
                                                  ? RGB(139, 148, 158)
                                                  : RGB(230, 237, 243));
                RECT areaTexto{margenIzquierdo + 24, posicionVerticalActual,
                               margenDerechoTexto,
                               posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaTexto,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaTexto, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                elemento.altoCalculado = altoTexto + 6;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::ReglaHorizontal: {
                HPEN lapizRegla = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizRegla));
                MoveToEx(contextoDestino, margenIzquierdo,
                         posicionVerticalActual + 6, nullptr);
                LineTo(contextoDestino, margenDerechoTexto,
                       posicionVerticalActual + 6);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(lapizRegla);

                elemento.altoCalculado = 14;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::SeparadorSeccionPersonalizado: {
                SelectObject(contextoDestino, fuenteH4);
                RECT areaCalculo{0, 0, margenDerechoTexto - margenIzquierdo, 100};
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaCalculo, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
                int anchoTexto = areaCalculo.right - areaCalculo.left;
                int centroY = posicionVerticalActual + 14;
                int centroX = (margenIzquierdo + margenDerechoTexto) / 2;

                HPEN lapizSeparador = CreatePen(PS_SOLID, 2, RGB(48, 54, 61));
                HPEN lapizPrevio = static_cast<HPEN>(SelectObject(contextoDestino, lapizSeparador));

                if (anchoTexto > 0 && !elemento.textoContenido.empty()) {
                    int mitadTexto = anchoTexto / 2 + 10;
                    int xFinIzq = std::max(margenIzquierdo, centroX - mitadTexto);
                    int xIniDer = std::min(margenDerechoTexto, centroX + mitadTexto);

                    MoveToEx(contextoDestino, margenIzquierdo, centroY, nullptr);
                    LineTo(contextoDestino, xFinIzq, centroY);

                    MoveToEx(contextoDestino, xIniDer, centroY, nullptr);
                    LineTo(contextoDestino, margenDerechoTexto, centroY);

                    SelectObject(contextoDestino, lapizPrevio);
                    DeleteObject(lapizSeparador);

                    SetTextColor(contextoDestino, RGB(121, 192, 255));
                    RECT areaTextoSep{xFinIzq, posicionVerticalActual, xIniDer, posicionVerticalActual + 28};
                    DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                              &areaTextoSep, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                } else {
                    MoveToEx(contextoDestino, margenIzquierdo, centroY, nullptr);
                    LineTo(contextoDestino, margenDerechoTexto, centroY);
                    SelectObject(contextoDestino, lapizPrevio);
                    DeleteObject(lapizSeparador);
                }

                if (estadoTexto.esFormatoMarkdown && !elemento.textoContenido.empty()) {
                    ElementoIndiceBarras barraSep;
                    barraSep.tituloEncabezado = elemento.textoContenido;
                    barraSep.nivelEncabezado = 2;
                    barraSep.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraSep);
                }

                elemento.altoCalculado = 28;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Imagen: {
                Gdiplus::Bitmap* mapaBits = obtenerBitmapParaMarkdown(
                    elemento.textoSecundario,
                    estadoTexto.rutaTextoActual);

                int anchoDisponible = margenDerechoTexto - margenIzquierdo;
                int altoRenderizado = 160;
                int anchoRenderizado = anchoDisponible;

                if (mapaBits && mapaBits->GetLastStatus() == Gdiplus::Ok) {
                    UINT anchoOriginal = mapaBits->GetWidth();
                    UINT altoOriginal = mapaBits->GetHeight();
                    if (anchoOriginal > 0 && altoOriginal > 0) {
                        double escala = std::min(
                            static_cast<double>(anchoDisponible) /
                                static_cast<double>(anchoOriginal),
                            280.0 / static_cast<double>(altoOriginal));
                        anchoRenderizado = std::max(
                            40, static_cast<int>(static_cast<double>(anchoOriginal) * escala));
                        altoRenderizado = std::max(
                            40, static_cast<int>(static_cast<double>(altoOriginal) * escala));
                    }

                    Gdiplus::Graphics lienzoGdiplus(contextoDestino);
                    lienzoGdiplus.SetInterpolationMode(
                        Gdiplus::InterpolationModeHighQualityBicubic);
                    lienzoGdiplus.DrawImage(mapaBits, margenIzquierdo,
                                           posicionVerticalActual,
                                           anchoRenderizado, altoRenderizado);

                    HPEN lapizBordeImagen =
                        CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                    HPEN lapizPrevioImg = static_cast<HPEN>(
                        SelectObject(contextoDestino, lapizBordeImagen));
                    HBRUSH brochaNula =
                        static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
                    HBRUSH brochaPreviaImg = static_cast<HBRUSH>(
                        SelectObject(contextoDestino, brochaNula));

                    Rectangle(contextoDestino, margenIzquierdo,
                              posicionVerticalActual,
                              margenIzquierdo + anchoRenderizado,
                              posicionVerticalActual + altoRenderizado);

                    SelectObject(contextoDestino, lapizPrevioImg);
                    SelectObject(contextoDestino, brochaPreviaImg);
                    DeleteObject(lapizBordeImagen);
                } else {
                    RECT areaCajaImagen{margenIzquierdo, posicionVerticalActual,
                                        margenDerechoTexto,
                                        posicionVerticalActual + 80};
                    HBRUSH brochaCaja = CreateSolidBrush(RGB(22, 27, 34));
                    HPEN lapizCaja = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                    HBRUSH brochaPrevia = static_cast<HBRUSH>(
                        SelectObject(contextoDestino, brochaCaja));
                    HPEN lapizPrevio = static_cast<HPEN>(
                        SelectObject(contextoDestino, lapizCaja));

                    RoundRect(contextoDestino, areaCajaImagen.left,
                              areaCajaImagen.top, areaCajaImagen.right,
                              areaCajaImagen.bottom, 6, 6);

                    SelectObject(contextoDestino, brochaPrevia);
                    SelectObject(contextoDestino, lapizPrevio);
                    DeleteObject(brochaCaja);
                    DeleteObject(lapizCaja);

                    SelectObject(contextoDestino, fuentePequena);
                    SetTextColor(contextoDestino, RGB(139, 148, 158));
                    std::wstring etiquetaImagen =
                        L"[Imagen: " +
                        (elemento.textoContenido.empty()
                             ? elemento.textoSecundario
                             : elemento.textoContenido) +
                        L"]";
                    DrawTextW(contextoDestino, etiquetaImagen.c_str(), -1,
                              &areaCajaImagen,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE |
                                  DT_NOPREFIX);
                    altoRenderizado = 80;
                }

                if (!elemento.textoContenido.empty()) {
                    SelectObject(contextoDestino, fuentePequena);
                    SetTextColor(contextoDestino, RGB(139, 148, 158));
                    RECT areaPie{margenIzquierdo,
                                 posicionVerticalActual + altoRenderizado + 4,
                                 margenDerechoTexto,
                                 posicionVerticalActual + altoRenderizado + 24};
                    DrawTextW(contextoDestino, elemento.textoContenido.c_str(),
                              -1, &areaPie,
                              DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
                    altoRenderizado += 24;
                }

                ElementoClicableMarkdown clicable;
                clicable.areaClic = {margenIzquierdo, posicionVerticalActual,
                                     margenIzquierdo + anchoRenderizado,
                                     posicionVerticalActual + altoRenderizado};
                clicable.destinoUrl = elemento.textoSecundario;
                clicable.esVideoYouTube = false;
                estadoTexto.elementosClicables.push_back(clicable);

                elemento.altoCalculado = altoRenderizado + 12;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::VideoYouTube: {
                std::wstring idVideo =
                    extraerIdVideoYouTube(elemento.textoSecundario);
                std::wstring urlMiniatura =
                    idVideo.empty()
                        ? L""
                        : (L"https://img.youtube.com/vi/" + idVideo +
                           L"/hqdefault.jpg");

                Gdiplus::Bitmap* miniatura =
                    obtenerBitmapParaMarkdown(urlMiniatura, L"");

                int anchoDisponible = margenDerechoTexto - margenIzquierdo;
                int anchoVideo = std::min(anchoDisponible, 340);
                int altoVideo = (anchoVideo * 9) / 16;

                RECT areaTarjeta{margenIzquierdo, posicionVerticalActual,
                                 margenIzquierdo + anchoVideo,
                                 posicionVerticalActual + altoVideo + 32};

                HBRUSH brochaFondoTarjeta =
                    CreateSolidBrush(RGB(22, 27, 34));
                HPEN lapizBordeTarjeta =
                    CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HBRUSH brochaPrevia = static_cast<HBRUSH>(
                    SelectObject(contextoDestino, brochaFondoTarjeta));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizBordeTarjeta));

                RoundRect(contextoDestino, areaTarjeta.left, areaTarjeta.top,
                          areaTarjeta.right, areaTarjeta.bottom, 8, 8);

                SelectObject(contextoDestino, brochaPrevia);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(brochaFondoTarjeta);
                DeleteObject(lapizBordeTarjeta);

                if (miniatura && miniatura->GetLastStatus() == Gdiplus::Ok) {
                    Gdiplus::Graphics lienzoGdiplus(contextoDestino);
                    lienzoGdiplus.SetInterpolationMode(
                        Gdiplus::InterpolationModeHighQualityBicubic);
                    lienzoGdiplus.DrawImage(miniatura, margenIzquierdo + 2,
                                           posicionVerticalActual + 2,
                                           anchoVideo - 4, altoVideo - 4);
                }

                // Botón central rojo YouTube Play (\u25B6)
                int centroX = margenIzquierdo + (anchoVideo / 2);
                int centroY = posicionVerticalActual + (altoVideo / 2);
                RECT areaBotonPlay{centroX - 22, centroY - 14, centroX + 22,
                                   centroY + 14};

                HBRUSH brochaBotonPlay = CreateSolidBrush(RGB(255, 0, 0));
                HBRUSH brochaPreviaPlay = static_cast<HBRUSH>(
                    SelectObject(contextoDestino, brochaBotonPlay));
                RoundRect(contextoDestino, areaBotonPlay.left,
                          areaBotonPlay.top, areaBotonPlay.right,
                          areaBotonPlay.bottom, 8, 8);
                SelectObject(contextoDestino, brochaPreviaPlay);
                DeleteObject(brochaBotonPlay);

                SelectObject(contextoDestino, fuentePequena);
                SetTextColor(contextoDestino, RGB(255, 255, 255));
                DrawTextW(contextoDestino, L"\u25B6", -1, &areaBotonPlay,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                // Pie de la tarjeta YouTube
                RECT areaPieYouTube{margenIzquierdo + 8,
                                    posicionVerticalActual + altoVideo + 2,
                                    margenIzquierdo + anchoVideo - 8,
                                    areaTarjeta.bottom};
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino, RGB(245, 245, 245));
                std::wstring etiquetaPie =
                    elemento.textoContenido.empty()
                        ? L"YouTube \u2022 Reproducir video"
                        : (elemento.textoContenido + L" \u2022 YouTube");
                DrawTextW(contextoDestino, etiquetaPie.c_str(), -1,
                          &areaPieYouTube,
                          DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                ElementoClicableMarkdown clicable;
                clicable.areaClic = areaTarjeta;
                clicable.destinoUrl = elemento.textoSecundario;
                clicable.esVideoYouTube = true;
                estadoTexto.elementosClicables.push_back(clicable);

                elemento.altoCalculado = altoVideo + 44;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::EnlaceWeb: {
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino, RGB(88, 166, 255));
                RECT areaEnlace{margenIzquierdo, posicionVerticalActual,
                                 margenDerechoTexto,
                                 posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEnlace,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEnlace, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                HPEN lapizEnlace = CreatePen(PS_SOLID, 1, RGB(88, 166, 255));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizEnlace));
                MoveToEx(contextoDestino, margenIzquierdo,
                         posicionVerticalActual + altoTexto + 2, nullptr);
                LineTo(contextoDestino, areaEnlace.right,
                       posicionVerticalActual + altoTexto + 2);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(lapizEnlace);

                ElementoClicableMarkdown clicable;
                clicable.areaClic = areaEnlace;
                clicable.destinoUrl = elemento.textoSecundario;
                clicable.esVideoYouTube = false;
                estadoTexto.elementosClicables.push_back(clicable);

                elemento.altoCalculado = altoTexto + 8;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::TablaMarkdown: {
                if (elemento.filasTabla.empty()) break;
                size_t numColumnas = 0;
                for (const auto& fila : elemento.filasTabla) {
                    numColumnas = std::max(numColumnas, fila.celdas.size());
                }
                if (numColumnas == 0) numColumnas = 1;

                int anchoTabla = margenDerechoTexto - margenIzquierdo;
                int anchoCol = anchoTabla / static_cast<int>(numColumnas);
                int altoFila = 28;
                int altoTabla = static_cast<int>(elemento.filasTabla.size()) * altoFila;

                RECT areaTabla{margenIzquierdo, posicionVerticalActual,
                               margenIzquierdo + anchoTabla,
                               posicionVerticalActual + altoTabla};

                HBRUSH brochaFondoTabla = CreateSolidBrush(RGB(13, 17, 23));
                HPEN lapizBordeTabla = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HBRUSH brochaPrevTabla = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaFondoTabla));
                HPEN lapizPrevTabla = static_cast<HPEN>(SelectObject(contextoDestino, lapizBordeTabla));

                RoundRect(contextoDestino, areaTabla.left, areaTabla.top, areaTabla.right, areaTabla.bottom, 8, 8);

                SelectObject(contextoDestino, lapizPrevTabla);
                SelectObject(contextoDestino, brochaPrevTabla);
                DeleteObject(lapizBordeTabla);
                DeleteObject(brochaFondoTabla);

                for (size_t idxFila = 0; idxFila < elemento.filasTabla.size(); ++idxFila) {
                    const auto& fila = elemento.filasTabla[idxFila];
                    int yFila = posicionVerticalActual + static_cast<int>(idxFila) * altoFila;

                    if (fila.esCabecera) {
                        HBRUSH brochaCabecera = CreateSolidBrush(RGB(22, 27, 34));
                        RECT areaCabecera{margenIzquierdo + 1, yFila + 1,
                                          margenIzquierdo + anchoTabla - 1, yFila + altoFila};
                        FillRect(contextoDestino, &areaCabecera, brochaCabecera);
                        DeleteObject(brochaCabecera);
                    } else if (idxFila % 2 == 1) {
                        HBRUSH brochaFilaPar = CreateSolidBrush(RGB(22, 27, 34));
                        RECT areaFilaPar{margenIzquierdo + 1, yFila,
                                         margenIzquierdo + anchoTabla - 1, yFila + altoFila};
                        FillRect(contextoDestino, &areaFilaPar, brochaFilaPar);
                        DeleteObject(brochaFilaPar);
                    }

                    if (idxFila > 0) {
                        HPEN lapizDiv = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                        HPEN lapizPrevDiv = static_cast<HPEN>(SelectObject(contextoDestino, lapizDiv));
                        MoveToEx(contextoDestino, margenIzquierdo, yFila, nullptr);
                        LineTo(contextoDestino, margenIzquierdo + anchoTabla, yFila);
                        SelectObject(contextoDestino, lapizPrevDiv);
                        DeleteObject(lapizDiv);
                    }

                    for (size_t idxCol = 0; idxCol < numColumnas; ++idxCol) {
                        int xCol = margenIzquierdo + static_cast<int>(idxCol) * anchoCol;
                        int xColFin = (idxCol + 1 == numColumnas) ? (margenIzquierdo + anchoTabla) : (xCol + anchoCol);

                        if (idxCol > 0) {
                            HPEN lapizDivV = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                            HPEN lapizPrevDivV = static_cast<HPEN>(SelectObject(contextoDestino, lapizDivV));
                            MoveToEx(contextoDestino, xCol, yFila, nullptr);
                            LineTo(contextoDestino, xCol, yFila + altoFila);
                            SelectObject(contextoDestino, lapizPrevDivV);
                            DeleteObject(lapizDivV);
                        }

                        std::wstring textoCelda = (idxCol < fila.celdas.size()) ? fila.celdas[idxCol] : L"";
                        RECT areaTextoCelda{xCol + 8, yFila, xColFin - 8, yFila + altoFila};
                        HFONT fuenteCelda = fila.esCabecera ? fuenteNegrita : fuenteCuerpo;
                        HFONT fuentePrevia = static_cast<HFONT>(SelectObject(contextoDestino, fuenteCelda));
                        SetTextColor(contextoDestino, fila.esCabecera ? RGB(240, 246, 252) : RGB(201, 209, 217));
                        DrawTextW(contextoDestino, textoCelda.c_str(), -1, &areaTextoCelda,
                                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
                        SelectObject(contextoDestino, fuentePrevia);
                    }
                }

                elemento.altoCalculado = altoTabla + 14;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::ListaOrdenada: {
                SelectObject(contextoDestino, fuenteNegrita);
                SetTextColor(contextoDestino, RGB(88, 166, 255));
                RECT areaNum{margenIzquierdo + 4, posicionVerticalActual,
                             margenIzquierdo + 24, posicionVerticalActual + 20};
                DrawTextW(contextoDestino, elemento.textoSecundario.c_str(), -1, &areaNum,
                          DT_LEFT | DT_NOPREFIX);

                int altoTexto = dibujarFragmentosEnriquecidos(
                    contextoDestino, elemento.textoContenido, margenIzquierdo + 26,
                    posicionVerticalActual, margenDerechoTexto - (margenIzquierdo + 26),
                    fuenteCuerpo, fuenteNegrita, fuenteCursiva, fuenteSubrayada,
                    fuenteCodigoPildora);

                elemento.altoCalculado = std::max(20, altoTexto) + 6;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Parrafo:
            default: {
                if (elemento.textoContenido.empty()) {
                    elemento.altoCalculado = 8;
                    posicionVerticalActual += elemento.altoCalculado;
                    break;
                }
                int altoTexto = dibujarFragmentosEnriquecidos(
                    contextoDestino, elemento.textoContenido, margenIzquierdo,
                    posicionVerticalActual, margenDerechoTexto - margenIzquierdo,
                    fuenteCuerpo, fuenteNegrita, fuenteCursiva, fuenteSubrayada,
                    fuenteCodigoPildora);

                elemento.altoCalculado = std::max(18, altoTexto) + 8;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
        }
    }

    estadoTexto.altoTotalContenido =
        posicionVerticalActual + estadoTexto.desplazamientoScrollY -
        areaDibujo.top + 40;

    // -------------------------------------------------------------
    // RENDERIZADO DEL ÍNDICE VERTICAL CON BARRAS (TOC BARS)
    // EXCLUSIVO PARA ARCHIVOS MARKDOWN (.MD)
    // -------------------------------------------------------------
    if (estadoTexto.esFormatoMarkdown &&
        !estadoTexto.barrasIndice.empty()) {
        int totalBarras =
            static_cast<int>(estadoTexto.barrasIndice.size());
        int alturaEspaciadoBarras = 14;
        int alturaTotalIndice = totalBarras * alturaEspaciadoBarras;
        int topeIndiceY =
            areaDibujo.top +
            std::max<int>(20, (altoArea - alturaTotalIndice) / 2);
        int bordeDerechoIndice = areaDibujo.right - 12;

        for (int posicionBarra = 0; posicionBarra < totalBarras;
             ++posicionBarra) {
            ElementoIndiceBarras& barra =
                estadoTexto.barrasIndice[static_cast<size_t>(posicionBarra)];
            int posicionYBarra = topeIndiceY + posicionBarra * alturaEspaciadoBarras;

            int anchoBarra = 12;
            if (barra.nivelEncabezado == 1) {
                anchoBarra = 18;
            } else if (barra.nivelEncabezado == 2) {
                anchoBarra = 14;
            }

            RECT areaRectBarra{bordeDerechoIndice - anchoBarra, posicionYBarra,
                               bordeDerechoIndice, posicionYBarra + 6};
            barra.areaBarra = areaRectBarra;

            bool esBarraHover = (posicionBarra == estadoTexto.posicionBarraHover);
            COLORREF colorBarra = esBarraHover
                                      ? RGB(88, 166, 255)
                                      : RGB(48, 54, 61);

            HBRUSH brochaBarra = CreateSolidBrush(colorBarra);
            FillRect(contextoDestino, &areaRectBarra, brochaBarra);
            DeleteObject(brochaBarra);
        }

        // Si hay hover sobre una barra, desplegar tarjeta con el título
        if (estadoTexto.posicionBarraHover >= 0 &&
            estadoTexto.posicionBarraHover < totalBarras) {
            const ElementoIndiceBarras& barraActiva =
                estadoTexto.barrasIndice[static_cast<size_t>(
                    estadoTexto.posicionBarraHover)];
            int centroY = (barraActiva.areaBarra.top +
                           barraActiva.areaBarra.bottom) /
                          2;

            SelectObject(contextoDestino, fuentePequena);
            RECT areaCalculoTooltip{0, 0, 300, 100};
            DrawTextW(contextoDestino, barraActiva.tituloEncabezado.c_str(),
                      -1, &areaCalculoTooltip,
                      DT_LEFT | DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);

            int anchoTooltip = areaCalculoTooltip.right - areaCalculoTooltip.left + 16;
            int altoTooltip = 24;

            RECT areaTooltip{
                bordeDerechoIndice - 30 - anchoTooltip, centroY - altoTooltip / 2,
                bordeDerechoIndice - 30, centroY + altoTooltip / 2};

            HBRUSH brochaTooltip = CreateSolidBrush(RGB(38, 38, 38));
            HPEN lapizTooltip = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
            HBRUSH brochaPreviaTooltip = static_cast<HBRUSH>(
                SelectObject(contextoDestino, brochaTooltip));
            HPEN lapizPrevioTooltip = static_cast<HPEN>(
                SelectObject(contextoDestino, lapizTooltip));

            RoundRect(contextoDestino, areaTooltip.left, areaTooltip.top,
                      areaTooltip.right, areaTooltip.bottom, 4, 4);

            SelectObject(contextoDestino, brochaPreviaTooltip);
            SelectObject(contextoDestino, lapizPrevioTooltip);
            DeleteObject(brochaTooltip);
            DeleteObject(lapizTooltip);

            SetTextColor(contextoDestino, RGB(245, 245, 245));
            RECT areaTextoTooltip = areaTooltip;
            areaTextoTooltip.left += 8;
            areaTextoTooltip.right -= 8;
            DrawTextW(contextoDestino, barraActiva.tituloEncabezado.c_str(),
                      -1, &areaTextoTooltip,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
    }

    DeleteObject(fuenteH1);
    DeleteObject(fuenteH2);
    DeleteObject(fuenteH3);
    DeleteObject(fuenteH4);
    DeleteObject(fuenteCuerpo);
    DeleteObject(fuenteNegrita);
    DeleteObject(fuenteCursiva);
    DeleteObject(fuenteSubrayada);
    DeleteObject(fuenteCodigo);
    DeleteObject(fuenteCodigoPildora);
    DeleteObject(fuentePequena);
}

inline void dibujarVistaTexto(HDC contextoDestino,
                              const RECT& areaDibujo) {
    // Barra de estado y navegación superior para archivos de texto y markdown (34px)
    HBRUSH brochaBarra = CreateSolidBrush(RGB(25, 25, 25));
    FillRect(contextoDestino, &areaDibujo, brochaBarra);
    DeleteObject(brochaBarra);

    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevio =
        static_cast<HPEN>(SelectObject(contextoDestino, lapizBorde));
    MoveToEx(contextoDestino, areaDibujo.left, areaDibujo.bottom - 1, nullptr);
    LineTo(contextoDestino, areaDibujo.right, areaDibujo.bottom - 1);
    SelectObject(contextoDestino, lapizPrevio);
    DeleteObject(lapizBorde);

    // Botón Anterior
    RECT areaBotonAnt{areaDibujo.left + 10, areaDibujo.top + 4,
                      areaDibujo.left + 42, areaDibujo.bottom - 4};
    estadoTexto.botonAnterior = areaBotonAnt;

    HBRUSH brochaAnt = CreateSolidBrush(
        estadoTexto.hoverBotonAnterior ? RGB(65, 65, 65) : RGB(40, 40, 40));
    HPEN lapizAnt = CreatePen(
        PS_SOLID, 1,
        estadoTexto.hoverBotonAnterior ? RGB(100, 100, 100) : RGB(55, 55, 55));
    HBRUSH brochaPreviaAnt =
        static_cast<HBRUSH>(SelectObject(contextoDestino, brochaAnt));
    HPEN lapizPrevioAnt =
        static_cast<HPEN>(SelectObject(contextoDestino, lapizAnt));
    RoundRect(contextoDestino, areaBotonAnt.left, areaBotonAnt.top,
              areaBotonAnt.right, areaBotonAnt.bottom, 4, 4);
    SelectObject(contextoDestino, brochaPreviaAnt);
    SelectObject(contextoDestino, lapizPrevioAnt);
    DeleteObject(brochaAnt);
    DeleteObject(lapizAnt);

    // Botón Siguiente
    RECT areaBotonSig{areaDibujo.left + 48, areaDibujo.top + 4,
                      areaDibujo.left + 80, areaDibujo.bottom - 4};
    estadoTexto.botonSiguiente = areaBotonSig;

    HBRUSH brochaSig = CreateSolidBrush(
        estadoTexto.hoverBotonSiguiente ? RGB(65, 65, 65) : RGB(40, 40, 40));
    HPEN lapizSig = CreatePen(
        PS_SOLID, 1,
        estadoTexto.hoverBotonSiguiente ? RGB(100, 100, 100) : RGB(55, 55, 55));
    HBRUSH brochaPreviaSig =
        static_cast<HBRUSH>(SelectObject(contextoDestino, brochaSig));
    HPEN lapizPrevioSig =
        static_cast<HPEN>(SelectObject(contextoDestino, lapizSig));
    RoundRect(contextoDestino, areaBotonSig.left, areaBotonSig.top,
              areaBotonSig.right, areaBotonSig.bottom, 4, 4);
    SelectObject(contextoDestino, brochaPreviaSig);
    SelectObject(contextoDestino, lapizPrevioSig);
    DeleteObject(brochaSig);
    DeleteObject(lapizSig);

    // Botón Editar
    RECT areaBotonEditar{areaDibujo.left + 88, areaDibujo.top + 4,
                         areaDibujo.left + 172, areaDibujo.bottom - 4};
    estadoTexto.botonEditar = areaBotonEditar;

    HBRUSH brochaEdit = CreateSolidBrush(
        estadoTexto.hoverBotonEditar ? RGB(65, 65, 65) : RGB(40, 40, 40));
    HPEN lapizEdit = CreatePen(
        PS_SOLID, 1,
        estadoTexto.hoverBotonEditar ? RGB(100, 100, 100) : RGB(55, 55, 55));
    HBRUSH brochaPreviaEdit =
        static_cast<HBRUSH>(SelectObject(contextoDestino, brochaEdit));
    HPEN lapizPrevioEdit =
        static_cast<HPEN>(SelectObject(contextoDestino, lapizEdit));
    RoundRect(contextoDestino, areaBotonEditar.left, areaBotonEditar.top,
              areaBotonEditar.right, areaBotonEditar.bottom, 4, 4);
    SelectObject(contextoDestino, brochaPreviaEdit);
    SelectObject(contextoDestino, lapizPrevioEdit);
    DeleteObject(brochaEdit);
    DeleteObject(lapizEdit);

    HFONT fuenteBotones = CreateFontW(
        -12, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuentePrevia =
        static_cast<HFONT>(SelectObject(contextoDestino, fuenteBotones));

    SetBkMode(contextoDestino, TRANSPARENT);
    SetTextColor(contextoDestino, RGB(245, 245, 245));
    DrawTextW(contextoDestino, L"◀", -1, &areaBotonAnt,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    DrawTextW(contextoDestino, L"▶", -1, &areaBotonSig,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    DrawTextW(contextoDestino, L"✏️ Editar", -1, &areaBotonEditar,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // Texto de estado
    HFONT fuenteEstado = CreateFontW(
        -11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    SelectObject(contextoDestino, fuenteEstado);
    DeleteObject(fuenteBotones);

    SetTextColor(contextoDestino, RGB(139, 148, 158));
    std::wstring tipoDoc =
        estadoTexto.esFormatoMarkdown ? L"Markdown" : L"Texto Plano";
    std::wstring textoEstado;
    if (estadoTexto.textosSeleccionados.size() > 1) {
        textoEstado =
            tipoDoc + L"  \u2022  " + estadoTexto.nombreArchivo +
            L"  \u2022  " +
            std::to_wstring(estadoTexto.totalLineasOriginales) +
            L" líneas  \u2022  " +
            std::to_wstring(estadoTexto.indiceTextoSeleccionado + 1) + L"/" +
            std::to_wstring(estadoTexto.textosSeleccionados.size());
    } else {
        textoEstado =
            tipoDoc + L"  \u2022  " + estadoTexto.nombreArchivo +
            L"  \u2022  " +
            std::to_wstring(estadoTexto.totalLineasOriginales) +
            L" líneas";
    }

    RECT areaTexto{areaDibujo.left + 182, areaDibujo.top, areaDibujo.right - 10,
                   areaDibujo.bottom};
    DrawTextW(contextoDestino, textoEstado.c_str(), -1, &areaTexto,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    SelectObject(contextoDestino, fuentePrevia);
    DeleteObject(fuenteEstado);
}


