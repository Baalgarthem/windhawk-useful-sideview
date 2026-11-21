#pragma once

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace SideViewV2 {

struct ItemArbolCarpetaV2 {
    std::wstring nombre;
    std::wstring rutaCompleta;
    bool esDirectorio{false};
    uintmax_t tamanoBytes{0};
    int nivelJerarquia{0};
};

struct CarpetaPesadaV2 {
    std::wstring nombre;
    uintmax_t tamanoBytes{0};
    double porcentaje{0.0};
};

struct ResultadoInspeccionCarpetaV2 {
    std::vector<ItemArbolCarpetaV2> elementos;
    std::vector<CarpetaPesadaV2> pesadas;
    std::vector<std::wstring> vacias;
    uintmax_t tamanoTotalBytes{0};
    size_t totalSubcarpetas{0};
    size_t totalArchivos{0};
};

class ServicioInspeccionCarpetasV2 {
public:
    ResultadoInspeccionCarpetaV2 Ejecutar(const std::wstring& ruta, size_t limitePesadas) const {
        ResultadoInspeccionCarpetaV2 resultado;
        if (ruta.empty()) return resultado;

        // La detección de carpetas vacías no depende del árbol que se dibuja.
        // Ese árbol está deliberadamente limitado para mantener fluida la UI,
        // mientras que este recorrido tiene su propio presupuesto y alcanza
        // toda la jerarquía disponible sin depender de la superficie de
        // Explorer que esté activa (normal o GPU).
        Inspeccionar(ruta, 0, 4, resultado);
        size_t visitadosParaVacias = 0;
        DetectarCarpetasVacias(ruta, 0, visitadosParaVacias, resultado);
        CalcularPesadas(ruta, limitePesadas, resultado);
        return resultado;
    }

private:
    static constexpr size_t MAX_ELEMENTOS_ARBOL = 500;
    static constexpr size_t MAX_CARPETAS_VACIAS = 5000;
    static constexpr size_t MAX_ELEMENTOS_DETECCION_VACIAS = 200000;
    static constexpr size_t MAX_ELEMENTOS_TAMANO = 100000;
    static constexpr int MAX_PROFUNDIDAD_TAMANO = 64;

    static std::wstring PatronBusqueda(const std::wstring& ruta) {
        if (ruta.empty()) return {};
        std::wstring patron = ruta;
        if (patron.back() != L'\\') patron.push_back(L'\\');
        patron.push_back(L'*');
        return patron;
    }

    static void Inspeccionar(const std::wstring& ruta, int nivel, int maxNivel,
                             ResultadoInspeccionCarpetaV2& resultado) {
        if (nivel >= maxNivel || resultado.elementos.size() >= MAX_ELEMENTOS_ARBOL) return;
        WIN32_FIND_DATAW datos{};
        const std::wstring patron = PatronBusqueda(ruta);
        HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
        if (busqueda == INVALID_HANDLE_VALUE) return;
        std::vector<ItemArbolCarpetaV2> subcarpetas;
        std::vector<ItemArbolCarpetaV2> archivos;
        do {
            if (!wcscmp(datos.cFileName, L".") || !wcscmp(datos.cFileName, L"..")) continue;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
            ItemArbolCarpetaV2 item;
            item.nombre = datos.cFileName;
            item.rutaCompleta = ruta + L"\\" + datos.cFileName;
            item.esDirectorio = (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            item.nivelJerarquia = nivel;
            if (item.esDirectorio) {
                ++resultado.totalSubcarpetas;
                ConservarPrimeros(subcarpetas, std::move(item));
            } else {
                ++resultado.totalArchivos;
                item.tamanoBytes = TamanoDeDatos(datos);
                ConservarPrimeros(archivos, std::move(item));
            }
        } while (FindNextFileW(busqueda, &datos));
        FindClose(busqueda);
        Ordenar(subcarpetas); Ordenar(archivos);
        for (const auto& carpeta : subcarpetas) {
            if (resultado.elementos.size() >= MAX_ELEMENTOS_ARBOL) break;
            resultado.elementos.push_back(carpeta);
            if (nivel + 1 < maxNivel) Inspeccionar(carpeta.rutaCompleta, nivel + 1, maxNivel, resultado);
        }
        for (const auto& archivo : archivos) {
            if (resultado.elementos.size() >= MAX_ELEMENTOS_ARBOL) break;
            resultado.elementos.push_back(archivo);
        }
    }

    // Recorre exclusivamente el sistema de archivos. No consulta HWND,
    // IShellView ni el árbol limitado de la vista, por lo que el resultado es
    // idéntico en Explorer nativo y cuando el mod GPU presenta un overlay.
    // Los puntos de reanálisis se omiten para no seguir junctions/symlinks ni
    // provocar ciclos. Una carpeta sólo se marca vacía si la enumeración fue
    // completa y no contenía entradas normales.
    static void DetectarCarpetasVacias(const std::wstring& ruta, int nivel,
                                       size_t& visitados,
                                       ResultadoInspeccionCarpetaV2& resultado) {
        if (ruta.empty() || visitados >= MAX_ELEMENTOS_DETECCION_VACIAS ||
            resultado.vacias.size() >= MAX_CARPETAS_VACIAS) {
            return;
        }

        WIN32_FIND_DATAW datos{};
        const std::wstring patron = PatronBusqueda(ruta);
        HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
        if (busqueda == INVALID_HANDLE_VALUE) return;

        std::vector<std::wstring> subcarpetas;
        bool contieneEntradasNormales = false;
        bool enumeracionCompleta = true;
        bool errorEnumeracion = false;

        for (;;) {
            if (wcscmp(datos.cFileName, L".") != 0 &&
                wcscmp(datos.cFileName, L"..") != 0) {
                if (visitados >= MAX_ELEMENTOS_DETECCION_VACIAS) {
                    enumeracionCompleta = false;
                    break;
                }
                ++visitados;

                // Reparse points no forman parte de la inspección. Además de
                // evitar ciclos, esto conserva el significado existente de
                // "vacía" para una carpeta sin archivos/directorios reales.
                if ((datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0) {
                    contieneEntradasNormales = true;
                    if ((datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                        subcarpetas.push_back(ruta + L"\\" + datos.cFileName);
                    }
                }
            }

            if (FindNextFileW(busqueda, &datos)) continue;
            if (GetLastError() != ERROR_NO_MORE_FILES) {
                enumeracionCompleta = false;
                errorEnumeracion = true;
            }
            break;
        }
        FindClose(busqueda);

        if (enumeracionCompleta && !contieneEntradasNormales && nivel > 0 &&
            resultado.vacias.size() < MAX_CARPETAS_VACIAS) {
            resultado.vacias.push_back(ruta);
        }

        // Si la enumeración terminó por un error, no se puede afirmar que los
        // subdirectorios recopilados representen todo el árbol. En cambio, si
        // sólo se agotó el presupuesto global, aún es seguro aprovechar los
        // elementos ya observados hasta llegar al mismo límite.
        if (errorEnumeracion) return;
        for (const auto& subcarpeta : subcarpetas) {
            if (visitados >= MAX_ELEMENTOS_DETECCION_VACIAS ||
                resultado.vacias.size() >= MAX_CARPETAS_VACIAS) break;
            DetectarCarpetasVacias(subcarpeta, nivel + 1, visitados, resultado);
        }
    }

    static void Ordenar(std::vector<ItemArbolCarpetaV2>& elementos) {
        std::sort(elementos.begin(), elementos.end(), [](const auto& a, const auto& b) {
            return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
        });
    }

    static uintmax_t TamanoDeDatos(const WIN32_FIND_DATAW& datos) {
        return (static_cast<uintmax_t>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
    }

    static uintmax_t SumarSaturado(uintmax_t izquierdo, uintmax_t derecho) {
        const uintmax_t maximo = std::numeric_limits<uintmax_t>::max();
        return derecho > maximo - izquierdo ? maximo : izquierdo + derecho;
    }

    static void ConservarPrimeros(std::vector<ItemArbolCarpetaV2>& elementos,
                                  ItemArbolCarpetaV2 elemento) {
        if (elementos.size() < MAX_ELEMENTOS_ARBOL) {
            elementos.push_back(std::move(elemento));
            return;
        }

        auto ultimo = std::max_element(elementos.begin(), elementos.end(),
            [](const auto& izquierdo, const auto& derecho) {
                return StrCmpLogicalW(izquierdo.nombre.c_str(), derecho.nombre.c_str()) < 0;
            });
        if (StrCmpLogicalW(elemento.nombre.c_str(), ultimo->nombre.c_str()) < 0) {
            *ultimo = std::move(elemento);
        }
    }

    static uintmax_t CalcularTamano(const std::wstring& ruta, int profundidad, size_t& visitados) {
        if (profundidad >= MAX_PROFUNDIDAD_TAMANO || visitados >= MAX_ELEMENTOS_TAMANO) return 0;
        WIN32_FIND_DATAW datos{};
        const std::wstring patron = PatronBusqueda(ruta);
        HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
        if (busqueda == INVALID_HANDLE_VALUE) return 0;
        uintmax_t total = 0;
        do {
            if (!wcscmp(datos.cFileName, L".") || !wcscmp(datos.cFileName, L"..")) continue;
            if (++visitados >= MAX_ELEMENTOS_TAMANO) break;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
            std::wstring elemento = ruta + L"\\" + datos.cFileName;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                total += CalcularTamano(elemento, profundidad + 1, visitados);
            } else {
                total = SumarSaturado(total, TamanoDeDatos(datos));
            }
        } while (FindNextFileW(busqueda, &datos));
        FindClose(busqueda);
        return total;
    }

    static void CalcularPesadas(const std::wstring& ruta, size_t limite,
                                ResultadoInspeccionCarpetaV2& resultado) {
        WIN32_FIND_DATAW datos{};
        const std::wstring patron = PatronBusqueda(ruta);
        HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
        if (busqueda == INVALID_HANDLE_VALUE) return;
        uintmax_t totalCompleto = 0;
        do {
            if (!wcscmp(datos.cFileName, L".") || !wcscmp(datos.cFileName, L"..")) continue;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
            if (!(datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                totalCompleto = SumarSaturado(totalCompleto, TamanoDeDatos(datos));
                continue;
            }
            size_t visitados = 0;
            uintmax_t tamano = CalcularTamano(ruta + L"\\" + datos.cFileName, 0, visitados);
            totalCompleto = SumarSaturado(totalCompleto, tamano);
            if (tamano > 0 && limite > 0) {
                resultado.pesadas.push_back({datos.cFileName, tamano, 0.0});
            }
        } while (FindNextFileW(busqueda, &datos));
        FindClose(busqueda);
        resultado.tamanoTotalBytes = totalCompleto;
        std::sort(resultado.pesadas.begin(), resultado.pesadas.end(), [](const auto& a, const auto& b) { return a.tamanoBytes > b.tamanoBytes; });
        if (resultado.pesadas.size() > limite) resultado.pesadas.resize(limite);
        for (auto& carpeta : resultado.pesadas) {
            carpeta.porcentaje = totalCompleto > 0
                ? (static_cast<double>(carpeta.tamanoBytes) / static_cast<double>(totalCompleto)) * 100.0
                : 0.0;
        }
    }
};
}
