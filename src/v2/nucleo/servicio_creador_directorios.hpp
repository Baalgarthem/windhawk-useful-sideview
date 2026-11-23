#pragma once

#include "v2/nucleo/servicio_persistencia_windhawk.hpp"
#include "v2/nucleo/ciclo_vida_v2.hpp"

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <windhawk_api.h>

#include <algorithm>
#include <cwctype>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace SideViewV2 {

enum class EstiloContadorDirectoriosV2 {
    Arabigo,
    Romano,
    Cardinal,
    Ordinal,
    Completo,
    Abreviado,
};

struct ElementoPersonalizadoDirectoriosV2 {
    std::wstring nombre;
    std::vector<std::wstring> valores;
};

struct SolicitudCreadorDirectoriosV2 {
    std::wstring raiz;
    std::wstring plantilla;
    SYSTEMTIME fechaInicio{};
    SYSTEMTIME fechaFin{};
    std::wstring prefijo;
    std::wstring sufijo;
    std::wstring idioma{L"es"};
    EstiloContadorDirectoriosV2 estiloContador{EstiloContadorDirectoriosV2::Arabigo};
};

struct ResultadoCreadorDirectoriosV2 {
    bool valido{false};
    std::size_t creadas{0};
    std::vector<std::wstring> errores;
};

// Servicio de dominio del Creador de directorios. La vista no conoce tokens,
// formatos, rangos ni llamadas de filesystem: sólo solicita el diálogo y
// recibe el resultado. Toda la lógica permanece inyectable y desmontable.
class ServicioCreadorDirectoriosV2 final {
public:
    explicit ServicioCreadorDirectoriosV2(
        std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia)
        : m_persistencia(std::move(persistencia)) {
        CargarElementosPersonalizados();
    }

    const std::vector<ElementoPersonalizadoDirectoriosV2>& ElementosPersonalizados() const {
        return m_elementos;
    }

    void RecargarConfiguracion() {
        CargarElementosPersonalizados();
    }

    bool GuardarElementoPersonalizado(const std::wstring& nombre,
                                      const std::vector<std::wstring>& valores) {
        const std::wstring token = NormalizarToken(nombre);
        if (token.empty() || valores.empty()) return false;

        std::vector<std::wstring> limpias;
        for (const std::wstring& valor : valores) {
            const std::wstring limpio = LimpiarNombre(valor);
            if (!limpio.empty()) limpias.push_back(limpio);
        }
        if (limpias.empty()) return false;

        for (auto& elemento : m_elementos) {
            if (_wcsicmp(elemento.nombre.c_str(), token.c_str()) == 0) {
                elemento.valores = std::move(limpias);
                GuardarElementosPersonalizados();
                return true;
            }
        }
        m_elementos.push_back({token, std::move(limpias)});
        GuardarElementosPersonalizados();
        return true;
    }

    ResultadoCreadorDirectoriosV2 CrearEstructura(
        const SolicitudCreadorDirectoriosV2& solicitud) const {
        ResultadoCreadorDirectoriosV2 resultado;
        const std::wstring raiz = NormalizarRuta(solicitud.raiz);
        if (!EsDirectorio(raiz)) {
            resultado.errores.push_back(L"La ruta raíz no existe o no es una carpeta.");
            return resultado;
        }
        if (solicitud.plantilla.empty()) {
            resultado.errores.push_back(L"La plantilla no contiene niveles de carpeta.");
            return resultado;
        }
        if (!FechaValida(solicitud.fechaInicio) || !FechaValida(solicitud.fechaFin) ||
            CompararFecha(solicitud.fechaInicio, solicitud.fechaFin) > 0) {
            resultado.errores.push_back(L"El rango de fechas no es válido.");
            return resultado;
        }

        std::vector<std::wstring> niveles = SepararLineas(solicitud.plantilla);
        if (niveles.empty()) {
            resultado.errores.push_back(L"La plantilla no contiene niveles válidos.");
            return resultado;
        }

        ContextoGeneracionV2 contexto{};
        contexto.fecha = solicitud.fechaInicio;
        contexto.contador = 1;
        contexto.solicitud = &solicitud;

        std::vector<std::wstring> rutas{raiz};
        constexpr std::size_t limiteDirectorios = 10000;
        for (const std::wstring& nivel : niveles) {
            std::vector<std::wstring> nuevasRutas;
            for (const std::wstring& rutaPadre : rutas) {
                // El prefijo y el sufijo forman parte de la plantilla para
                // que también puedan contener tokens y provocar expansiones
                // de fecha/rango, no sólo texto literal.
                const std::vector<std::wstring> nombres = ExpandirSegmento(
                    solicitud.prefijo + nivel + solicitud.sufijo, contexto);
                for (const std::wstring& nombreSinLimpiar : nombres) {
                    const std::wstring nombre = LimpiarNombre(nombreSinLimpiar);
                    if (nombre.empty()) continue;
                    if (nuevasRutas.size() >= limiteDirectorios) {
                        resultado.errores.push_back(L"La plantilla supera el límite seguro de 10,000 carpetas.");
                        resultado.valido = false;
                        return resultado;
                    }
                    std::wstring ruta = rutaPadre;
                    if (!ruta.empty() && ruta.back() != L'\\') ruta.push_back(L'\\');
                    ruta += nombre;
                    if (!CrearDirectorio(ruta)) {
                        resultado.errores.push_back(L"No se pudo crear: " + ruta);
                        continue;
                    }
                    nuevasRutas.push_back(std::move(ruta));
                    ++resultado.creadas;
                    ++contexto.contador;
                }
            }
            rutas.swap(nuevasRutas);
            if (rutas.empty()) break;
        }

        resultado.valido = resultado.errores.empty();
        return resultado;
    }

    bool MostrarDialogo(HWND hwndPadre) {
        if (g_descargandoMod.load(std::memory_order_acquire)) return false;
        return MostrarDialogoInterno(hwndPadre);
    }

private:
    struct ContextoGeneracionV2 {
        SYSTEMTIME fecha{};
        int contador{1};
        const SolicitudCreadorDirectoriosV2* solicitud{nullptr};
    };

    enum : int {
        ID_RAIZ = 2101,
        ID_EXAMINAR = 2102,
        ID_PRESET = 2103,
        ID_FECHA_INICIO = 2104,
        ID_FECHA_FIN = 2105,
        ID_PREFIJO = 2106,
        ID_SUFIJO = 2107,
        ID_ESTILO = 2108,
        ID_IDIOMA = 2109,
        ID_PLANTILLA = 2110,
        ID_ELEMENTO = 2111,
        ID_GUARDAR_ELEMENTO = 2112,
        ID_CREAR = 2113,
        ID_CANCELAR = 2114,
    };

    struct EstadoDialogoV2 {
        ServicioCreadorDirectoriosV2* servicio{nullptr};
        HWND padre{nullptr};
        HWND campoRaiz{nullptr};
        HWND selectorPreset{nullptr};
        HWND fechaInicio{nullptr};
        HWND fechaFin{nullptr};
        HWND campoPrefijo{nullptr};
        HWND campoSufijo{nullptr};
        HWND selectorEstilo{nullptr};
        HWND selectorIdioma{nullptr};
        HWND campoPlantilla{nullptr};
        HWND campoElemento{nullptr};
        bool terminado{false};
        bool aceptado{false};
    };

    static constexpr wchar_t CLASE_DIALOGO[] = L"UsefulSideView_CreadorDirectorios_V2";

    static std::wstring Clave(const wchar_t* sufijo) {
        return L"useful-sideview.v3.creadorDirectorios." + std::wstring(sufijo ? sufijo : L"");
    }

    static std::wstring LeerAjusteCadena(PCWSTR nombre) {
        if (!nombre) return {};
        PCWSTR valor = Wh_GetStringSetting(nombre);
        if (!valor) return {};
        std::wstring resultado(valor);
        Wh_FreeStringSetting(valor);
        return resultado;
    }

    static std::wstring Recortar(std::wstring valor) {
        auto noEspacio = [](wchar_t caracter) {
            return !std::iswspace(static_cast<wint_t>(caracter));
        };
        valor.erase(valor.begin(), std::find_if(valor.begin(), valor.end(), noEspacio));
        valor.erase(std::find_if(valor.rbegin(), valor.rend(), noEspacio).base(), valor.end());
        return valor;
    }

    static std::vector<std::wstring> SepararLineas(const std::wstring& texto) {
        std::vector<std::wstring> resultado;
        std::size_t inicio = 0;
        while (inicio <= texto.size()) {
            const std::size_t fin = texto.find_first_of(L"\r\n", inicio);
            std::wstring linea = Recortar(texto.substr(inicio, fin == std::wstring::npos
                                                                ? std::wstring::npos
                                                                : fin - inicio));
            if (!linea.empty()) resultado.push_back(std::move(linea));
            if (fin == std::wstring::npos) break;
            inicio = fin + 1;
            while (inicio < texto.size() && (texto[inicio] == L'\r' || texto[inicio] == L'\n')) {
                ++inicio;
            }
        }
        return resultado;
    }

    static std::vector<std::wstring> SepararValores(const std::wstring& texto) {
        std::vector<std::wstring> resultado;
        std::size_t inicio = 0;
        while (inicio <= texto.size()) {
            const std::size_t fin = texto.find(L'|', inicio);
            const std::wstring valor = Recortar(texto.substr(inicio, fin == std::wstring::npos
                                                                        ? std::wstring::npos
                                                                        : fin - inicio));
            if (!valor.empty()) resultado.push_back(valor);
            if (fin == std::wstring::npos) break;
            inicio = fin + 1;
        }
        return resultado;
    }

    static std::wstring EscaparCampo(const std::wstring& valor) {
        std::wstring resultado;
        resultado.reserve(valor.size());
        for (wchar_t caracter : valor) {
            if (caracter == L'\\' || caracter == L'|' || caracter == L'\n' || caracter == L'\r') {
                resultado.push_back(L'\\');
            }
            resultado.push_back(caracter == L'\n' || caracter == L'\r' ? L' ' : caracter);
        }
        return resultado;
    }

    static std::vector<std::wstring> SepararEscapado(const std::wstring& texto,
                                                      wchar_t separador) {
        std::vector<std::wstring> resultado;
        std::wstring actual;
        bool escapado = false;
        for (wchar_t caracter : texto) {
            if (escapado) {
                actual.push_back(caracter);
                escapado = false;
            } else if (caracter == L'\\') {
                escapado = true;
            } else if (caracter == separador) {
                resultado.push_back(std::move(actual));
                actual.clear();
            } else {
                actual.push_back(caracter);
            }
        }
        if (escapado) actual.push_back(L'\\');
        resultado.push_back(std::move(actual));
        return resultado;
    }

    void CargarElementosPersonalizados() {
        m_elementos.clear();
        if (!m_persistencia) return;
        const std::wstring serializado = m_persistencia->LeerCadena(Clave(L"elementos"));
        for (const std::wstring& linea : SepararLineas(serializado)) {
            const std::vector<std::wstring> campos = SepararEscapado(linea, L'|');
            if (campos.size() < 2) continue;
            ElementoPersonalizadoDirectoriosV2 elemento;
            elemento.nombre = NormalizarToken(campos[0]);
            for (std::size_t i = 1; i < campos.size(); ++i) {
                if (!campos[i].empty()) elemento.valores.push_back(campos[i]);
            }
            if (!elemento.nombre.empty() && !elemento.valores.empty()) {
                m_elementos.push_back(std::move(elemento));
            }
        }
    }

    void GuardarElementosPersonalizados() const {
        if (!m_persistencia) return;
        std::wstring serializado;
        for (const auto& elemento : m_elementos) {
            if (!serializado.empty()) serializado.push_back(L'\n');
            serializado += EscaparCampo(elemento.nombre);
            for (const std::wstring& valor : elemento.valores) {
                serializado.push_back(L'|');
                serializado += EscaparCampo(valor);
            }
        }
        m_persistencia->GuardarCadena(Clave(L"elementos"), serializado);
    }

    static std::wstring NormalizarRuta(const std::wstring& ruta) {
        if (ruta.empty()) return {};
        wchar_t completa[MAX_PATH * 4]{};
        const DWORD longitud = GetFullPathNameW(ruta.c_str(), ARRAYSIZE(completa), completa, nullptr);
        if (longitud == 0 || longitud >= ARRAYSIZE(completa)) return {};
        std::wstring resultado(completa, longitud);
        while (resultado.size() > 3 && !resultado.empty() &&
               (resultado.back() == L'\\' || resultado.back() == L'/')) {
            resultado.pop_back();
        }
        return resultado;
    }

    static bool EsDirectorio(const std::wstring& ruta) {
        const DWORD atributos = GetFileAttributesW(ruta.c_str());
        return atributos != INVALID_FILE_ATTRIBUTES &&
               (atributos & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    static bool FechaValida(const SYSTEMTIME& fecha) {
        FILETIME archivo{};
        return SystemTimeToFileTime(&fecha, &archivo) != FALSE;
    }

    static int CompararFecha(const SYSTEMTIME& izquierda, const SYSTEMTIME& derecha) {
        FILETIME a{}, b{};
        if (!SystemTimeToFileTime(&izquierda, &a) || !SystemTimeToFileTime(&derecha, &b)) return 0;
        ULARGE_INTEGER ua{};
        ua.LowPart = a.dwLowDateTime;
        ua.HighPart = a.dwHighDateTime;
        ULARGE_INTEGER ub{};
        ub.LowPart = b.dwLowDateTime;
        ub.HighPart = b.dwHighDateTime;
        return ua.QuadPart < ub.QuadPart ? -1 : (ua.QuadPart > ub.QuadPart ? 1 : 0);
    }

    static int DiasEnMes(WORD anio, WORD mes) {
        static constexpr int dias[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (mes < 1 || mes > 12) return 30;
        if (mes == 2 && ((anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0))) return 29;
        return dias[mes];
    }

    static std::wstring NormalizarToken(const std::wstring& nombre) {
        std::wstring token = Recortar(nombre);
        if (token.size() >= 4 && token.substr(0, 2) == L"{{" && token.substr(token.size() - 2) == L"}}") {
            token = token.substr(2, token.size() - 4);
        }
        std::transform(token.begin(), token.end(), token.begin(), [](wchar_t caracter) {
            return static_cast<wchar_t>(std::towupper(caracter));
        });
        for (wchar_t& caracter : token) {
            if (!(std::iswalnum(static_cast<wint_t>(caracter)) || caracter == L'_')) caracter = L'_';
        }
        return token;
    }

    static std::wstring LimpiarNombre(const std::wstring& valor) {
        std::wstring resultado = Recortar(valor);
        for (wchar_t& caracter : resultado) {
            if (caracter == L'\\' || caracter == L'/' || caracter == L':' || caracter == L'*' ||
                caracter == L'?' || caracter == L'\"' || caracter == L'<' || caracter == L'>' ||
                caracter == L'|') {
                caracter = L'_';
            }
        }
        while (!resultado.empty() && (resultado.back() == L'.' || resultado.back() == L' ')) {
            resultado.pop_back();
        }
        if (resultado.empty() || resultado == L"." || resultado == L"..") return L"_";
        return resultado;
    }

    static std::wstring Romanos(int valor) {
        if (valor <= 0 || valor > 3999) return std::to_wstring(valor);
        static constexpr std::pair<int, const wchar_t*> valores[] = {
            {1000, L"M"}, {900, L"CM"}, {500, L"D"}, {400, L"CD"},
            {100, L"C"}, {90, L"XC"}, {50, L"L"}, {40, L"XL"},
            {10, L"X"}, {9, L"IX"}, {5, L"V"}, {4, L"IV"}, {1, L"I"}};
        std::wstring resultado;
        for (const auto& par : valores) {
            while (valor >= par.first) {
                resultado += par.second;
                valor -= par.first;
            }
        }
        return resultado;
    }

    static std::wstring CardinalEsp(int valor) {
        static constexpr const wchar_t* unidades[] = {
            L"cero", L"uno", L"dos", L"tres", L"cuatro", L"cinco", L"seis", L"siete", L"ocho", L"nueve",
            L"diez", L"once", L"doce", L"trece", L"catorce", L"quince", L"dieciséis", L"diecisiete", L"dieciocho", L"diecinueve"};
        static constexpr const wchar_t* decenas[] = {
            L"", L"", L"veinte", L"treinta", L"cuarenta", L"cincuenta", L"sesenta", L"setenta", L"ochenta", L"noventa"};
        if (valor >= 0 && valor < 20) return unidades[valor];
        if (valor < 100) {
            const int decena = valor / 10;
            const int unidad = valor % 10;
            return unidad == 0 ? decenas[decena] : std::wstring(decenas[decena]) + L" y " + unidades[unidad];
        }
        if (valor == 100) return L"cien";
        if (valor < 200) return L"ciento " + CardinalEsp(valor - 100);
        if (valor < 1000) {
            const int centenas = valor / 100;
            static constexpr const wchar_t* nombres[] = {L"", L"ciento", L"doscientos", L"trescientos", L"cuatrocientos", L"quinientos", L"seiscientos", L"setecientos", L"ochocientos", L"novecientos"};
            const int resto = valor % 100;
            return resto == 0 ? nombres[centenas] : std::wstring(nombres[centenas]) + L" " + CardinalEsp(resto);
        }
        return std::to_wstring(valor);
    }

    static std::wstring CardinalEn(int valor) {
        static constexpr const wchar_t* unidades[] = {
            L"zero", L"one", L"two", L"three", L"four", L"five", L"six", L"seven", L"eight", L"nine",
            L"ten", L"eleven", L"twelve", L"thirteen", L"fourteen", L"fifteen", L"sixteen", L"seventeen", L"eighteen", L"nineteen"};
        static constexpr const wchar_t* decenas[] = {L"", L"", L"twenty", L"thirty", L"forty", L"fifty", L"sixty", L"seventy", L"eighty", L"ninety"};
        if (valor >= 0 && valor < 20) return unidades[valor];
        if (valor < 100) return (valor % 10 == 0) ? decenas[valor / 10] : std::wstring(decenas[valor / 10]) + L"-" + unidades[valor % 10];
        if (valor < 1000) return std::wstring(unidades[valor / 100]) + L" hundred" + (valor % 100 ? L" " + CardinalEn(valor % 100) : L"");
        return std::to_wstring(valor);
    }

    static std::wstring FormatearContador(int valor,
                                          EstiloContadorDirectoriosV2 estilo,
                                          const std::wstring& idioma) {
        if (estilo == EstiloContadorDirectoriosV2::Arabigo) return std::to_wstring(valor);
        if (estilo == EstiloContadorDirectoriosV2::Romano) return Romanos(valor);
        const bool ingles = _wcsicmp(idioma.c_str(), L"en") == 0;
        if (estilo == EstiloContadorDirectoriosV2::Cardinal || estilo == EstiloContadorDirectoriosV2::Completo) {
            return ingles ? CardinalEn(valor) : CardinalEsp(valor);
        }
        if (estilo == EstiloContadorDirectoriosV2::Ordinal) {
            if (ingles) return std::to_wstring(valor) + (valor == 1 ? L"st" : (valor == 2 ? L"nd" : (valor == 3 ? L"rd" : L"th")));
            return std::to_wstring(valor) + L".º";
        }
        return ingles ? std::to_wstring(valor) + L"th" : std::to_wstring(valor) + L".º";
    }

    static std::wstring NombreMes(WORD mes, const std::wstring& idioma, bool corto) {
        static constexpr const wchar_t* es[] = {L"", L"Enero", L"Febrero", L"Marzo", L"Abril", L"Mayo", L"Junio", L"Julio", L"Agosto", L"Septiembre", L"Octubre", L"Noviembre", L"Diciembre"};
        static constexpr const wchar_t* en[] = {L"", L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December"};
        static constexpr const wchar_t* esCorto[] = {L"", L"Ene", L"Feb", L"Mar", L"Abr", L"May", L"Jun", L"Jul", L"Ago", L"Sep", L"Oct", L"Nov", L"Dic"};
        static constexpr const wchar_t* enCorto[] = {L"", L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"};
        if (mes < 1 || mes > 12) return {};
        const bool ingles = _wcsicmp(idioma.c_str(), L"en") == 0;
        return corto ? (ingles ? enCorto[mes] : esCorto[mes]) : (ingles ? en[mes] : es[mes]);
    }

    std::wstring ReemplazarBasicos(std::wstring texto, const ContextoGeneracionV2& contexto) const {
        const SolicitudCreadorDirectoriosV2& solicitud = *contexto.solicitud;
        const std::wstring idioma = solicitud.idioma.empty() ? L"es" : solicitud.idioma;
        auto reemplazar = [&texto](const std::wstring& token, const std::wstring& valor) {
            std::size_t posicion = 0;
            while ((posicion = texto.find(token, posicion)) != std::wstring::npos) {
                texto.replace(posicion, token.size(), valor);
                posicion += valor.size();
            }
        };
        wchar_t anio[16]{};
        swprintf_s(anio, L"%04u", contexto.fecha.wYear);
        wchar_t mes[8]{};
        swprintf_s(mes, L"%02u", contexto.fecha.wMonth);
        wchar_t dia[8]{};
        swprintf_s(dia, L"%02u", contexto.fecha.wDay);
        reemplazar(L"{{YEAR}}", anio);
        reemplazar(L"{{YYYY}}", anio);
        reemplazar(L"{{MONTH}}", mes);
        reemplazar(L"{{DAY}}", dia);
        reemplazar(L"{{MONTH_NAME}}", NombreMes(contexto.fecha.wMonth, idioma, false));
        reemplazar(L"{{MONTH_SHORT}}", NombreMes(contexto.fecha.wMonth, idioma, true));
        std::wstring fechaSemantica = NombreMes(contexto.fecha.wMonth, idioma, false) + L" " + std::to_wstring(contexto.fecha.wDay) + L" " + anio;
        reemplazar(L"{{DATE}}", fechaSemantica);
        reemplazar(L"{{DATE_SEMANTIC}}", fechaSemantica);
        reemplazar(L"{{COUNTER}}", FormatearContador(contexto.contador, solicitud.estiloContador, idioma));
        reemplazar(L"{{COUNTER_ARABIC}}", std::to_wstring(contexto.contador));
        reemplazar(L"{{COUNTER_ROMAN}}", Romanos(contexto.contador));
        reemplazar(L"{{COUNTER_CARDINAL}}", idioma == L"en"
                                              ? CardinalEn(contexto.contador)
                                              : CardinalEsp(contexto.contador));
        reemplazar(L"{{COUNTER_ORDINAL}}", FormatearContador(contexto.contador, EstiloContadorDirectoriosV2::Ordinal, idioma));

        return texto;
    }

    std::vector<std::wstring> ExpandirSegmento(const std::wstring& segmento,
                                                ContextoGeneracionV2 contexto) const {
        const std::wstring recortado = Recortar(segmento);
        if (recortado.empty()) return {};

        const std::size_t contadorInicio = recortado.find(L"{{COUNTER:");
        if (contadorInicio != std::wstring::npos) {
            const std::size_t fin = recortado.find(L"}}", contadorInicio);
            if (fin != std::wstring::npos) {
                const std::wstring rango = recortado.substr(contadorInicio + 10, fin - contadorInicio - 10);
                const std::size_t separador = rango.find(L'-');
                if (separador != std::wstring::npos) {
                    const int primero = _wtoi(rango.substr(0, separador).c_str());
                    const int ultimo = _wtoi(rango.substr(separador + 1).c_str());
                    if (primero > 0 && ultimo >= primero && ultimo - primero <= 1000) {
                        std::vector<std::wstring> resultado;
                        for (int valor = primero; valor <= ultimo; ++valor) {
                            ContextoGeneracionV2 copia = contexto;
                            copia.contador = valor;
                            std::wstring reemplazado = recortado;
                            reemplazado.replace(contadorInicio, fin + 2 - contadorInicio,
                                                FormatearContador(valor, contexto.solicitud->estiloContador, contexto.solicitud->idioma));
                            const auto siguientes = ExpandirSegmento(reemplazado, copia);
                            resultado.insert(resultado.end(), siguientes.begin(), siguientes.end());
                        }
                        return resultado;
                    }
                }
            }
        }

        const std::size_t posicionAnio = recortado.find(L"{{YEAR}}");
        if (posicionAnio != std::wstring::npos || recortado.find(L"{{YYYY}}") != std::wstring::npos) {
            std::vector<std::wstring> resultado;
            const WORD primero = contexto.solicitud->fechaInicio.wYear;
            const WORD ultimo = contexto.solicitud->fechaFin.wYear;
            for (WORD anio = primero; anio <= ultimo; ++anio) {
                ContextoGeneracionV2 copia = contexto;
                copia.fecha.wYear = anio;
                const auto siguientes = ExpandirSegmento(ReemplazarBasicos(recortado, copia), copia);
                resultado.insert(resultado.end(), siguientes.begin(), siguientes.end());
                if (anio == 0xFFFF) break;
            }
            return resultado;
        }
        if (recortado.find(L"{{MONTH}}") != std::wstring::npos ||
            recortado.find(L"{{MONTH_NAME}}") != std::wstring::npos ||
            recortado.find(L"{{MONTH_SHORT}}") != std::wstring::npos) {
            std::vector<std::wstring> resultado;
            for (WORD mes = 1; mes <= 12; ++mes) {
                ContextoGeneracionV2 copia = contexto;
                copia.fecha.wMonth = mes;
                copia.fecha.wDay = std::min<WORD>(contexto.fecha.wDay, static_cast<WORD>(DiasEnMes(copia.fecha.wYear, mes)));
                const auto siguientes = ExpandirSegmento(ReemplazarBasicos(recortado, copia), copia);
                resultado.insert(resultado.end(), siguientes.begin(), siguientes.end());
            }
            return resultado;
        }
        if (recortado.find(L"{{SEMESTER}}") != std::wstring::npos ||
            recortado.find(L"{{SEMESTRE}}") != std::wstring::npos ||
            recortado.find(L"{{SEMESTER_NAME}}") != std::wstring::npos) {
            std::vector<std::wstring> valores;
            for (const auto& elemento : m_elementos) {
                if (_wcsicmp(elemento.nombre.c_str(), L"SEMESTRE") == 0) valores = elemento.valores;
            }
            if (valores.empty()) valores = {L"Primer semestre", L"Segundo semestre", L"Tercer semestre"};
            std::vector<std::wstring> resultado;
            for (const auto& valor : valores) {
                std::wstring reemplazado = recortado;
                const std::wstring token = recortado.find(L"{{SEMESTRE}}") != std::wstring::npos ? L"{{SEMESTRE}}" : (recortado.find(L"{{SEMESTER_NAME}}") != std::wstring::npos ? L"{{SEMESTER_NAME}}" : L"{{SEMESTER}}");
                std::size_t posicion = reemplazado.find(token);
                if (posicion != std::wstring::npos) reemplazado.replace(posicion, token.size(), valor);
                const auto siguientes = ExpandirSegmento(ReemplazarBasicos(reemplazado, contexto), contexto);
                resultado.insert(resultado.end(), siguientes.begin(), siguientes.end());
            }
            return resultado;
        }

        for (const auto& elemento : m_elementos) {
            const std::wstring token = L"{{" + elemento.nombre + L"}}";
            if (recortado.find(token) == std::wstring::npos) continue;
            std::vector<std::wstring> resultado;
            for (const std::wstring& valor : elemento.valores) {
                std::wstring reemplazado = recortado;
                std::size_t posicion = 0;
                while ((posicion = reemplazado.find(token, posicion)) != std::wstring::npos) {
                    reemplazado.replace(posicion, token.size(), valor);
                    posicion += valor.size();
                }
                const auto siguientes = ExpandirSegmento(reemplazado, contexto);
                resultado.insert(resultado.end(), siguientes.begin(), siguientes.end());
            }
            return resultado;
        }

        if (recortado.find(L"{{DATE}}") != std::wstring::npos || recortado.find(L"{{DATE_SEMANTIC}}") != std::wstring::npos) {
            std::vector<std::wstring> resultado;
            SYSTEMTIME fecha = contexto.solicitud->fechaInicio;
            const SYSTEMTIME fin = contexto.solicitud->fechaFin;
            std::size_t total = 0;
            while (CompararFecha(fecha, fin) <= 0 && total < 3661) {
                ContextoGeneracionV2 copia = contexto;
                copia.fecha = fecha;
                const auto siguientes = ExpandirSegmento(ReemplazarBasicos(recortado, copia), copia);
                resultado.insert(resultado.end(), siguientes.begin(), siguientes.end());
                ++total;
                ++fecha.wDay;
                if (fecha.wDay > DiasEnMes(fecha.wYear, fecha.wMonth)) {
                    fecha.wDay = 1;
                    ++fecha.wMonth;
                    if (fecha.wMonth > 12) {
                        fecha.wMonth = 1;
                        ++fecha.wYear;
                    }
                }
            }
            return resultado;
        }
        return {ReemplazarBasicos(recortado, contexto)};
    }

    static bool CrearDirectorio(const std::wstring& ruta) {
        const int resultado = SHCreateDirectoryExW(nullptr, ruta.c_str(), nullptr);
        return resultado == ERROR_SUCCESS || resultado == ERROR_ALREADY_EXISTS ||
               GetLastError() == ERROR_ALREADY_EXISTS;
    }

    static void AgregarTexto(HWND padre, const wchar_t* texto, int x, int y, int ancho, int alto) {
        CreateWindowW(L"STATIC", texto, WS_CHILD | WS_VISIBLE, x, y, ancho, alto,
                      padre, nullptr, ObtenerInstanciaModV2(), nullptr);
    }

    static HWND CrearEdicion(HWND padre, int id, int x, int y, int ancho, int alto,
                             DWORD estilo = ES_AUTOHSCROLL) {
        return CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                               WS_CHILD | WS_VISIBLE | estilo, x, y, ancho, alto,
                               padre, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                               ObtenerInstanciaModV2(), nullptr);
    }

    static HWND CrearCombo(HWND padre, int id, int x, int y, int ancho, int alto) {
        return CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                             CBS_DROPDOWNLIST | WS_VSCROLL, x, y, ancho, alto,
                             padre, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                             ObtenerInstanciaModV2(), nullptr);
    }

    static void CargarTexto(HWND control, const std::wstring& texto) {
        if (control) SetWindowTextW(control, texto.c_str());
    }

    static std::wstring LeerTexto(HWND control) {
        if (!control) return {};
        const int longitud = GetWindowTextLengthW(control);
        if (longitud <= 0) return {};
        std::wstring resultado(static_cast<std::size_t>(longitud) + 1, L'\0');
        GetWindowTextW(control, resultado.data(), longitud + 1);
        resultado.resize(static_cast<std::size_t>(longitud));
        return resultado;
    }

    static void SeleccionarCarpeta(EstadoDialogoV2* estado) {
        if (!estado) return;
        BROWSEINFOW info{};
        info.hwndOwner = estado->padre;
        info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
        info.lpszTitle = L"Selecciona la carpeta raíz de la estructura";
        PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&info);
        if (!pidl) return;
        wchar_t ruta[MAX_PATH * 4]{};
        const bool obtenida = SHGetPathFromIDListW(pidl, ruta) != FALSE;
        CoTaskMemFree(pidl);
        if (obtenida) CargarTexto(estado->campoRaiz, ruta);
    }

    static void AplicarPreset(EstadoDialogoV2* estado, int indice) {
        if (!estado || !estado->campoPlantilla) return;
        const wchar_t* plantilla = indice == 1 ? L"{{YEAR}}\r\n{{MONTH_NAME}}" :
                                    (indice == 2 ? L"Universidad\r\nSemestres\r\n{{SEMESTRE}}" :
                                                    L"{{YEAR}}\r\n{{MONTH_NAME}}");
        CargarTexto(estado->campoPlantilla, plantilla);
    }

    static EstiloContadorDirectoriosV2 EstiloDesdeCombo(HWND combo) {
        const LRESULT indice = SendMessageW(combo, CB_GETCURSEL, 0, 0);
        switch (indice) {
            case 1: return EstiloContadorDirectoriosV2::Romano;
            case 2: return EstiloContadorDirectoriosV2::Cardinal;
            case 3: return EstiloContadorDirectoriosV2::Ordinal;
            case 4: return EstiloContadorDirectoriosV2::Completo;
            case 5: return EstiloContadorDirectoriosV2::Abreviado;
            default: return EstiloContadorDirectoriosV2::Arabigo;
        }
    }

    static SolicitudCreadorDirectoriosV2 LeerSolicitud(const EstadoDialogoV2* estado) {
        SolicitudCreadorDirectoriosV2 solicitud;
        solicitud.raiz = LeerTexto(estado->campoRaiz);
        solicitud.plantilla = LeerTexto(estado->campoPlantilla);
        solicitud.prefijo = LeerTexto(estado->campoPrefijo);
        solicitud.sufijo = LeerTexto(estado->campoSufijo);
        solicitud.estiloContador = EstiloDesdeCombo(estado->selectorEstilo);
        const LRESULT idioma = SendMessageW(estado->selectorIdioma, CB_GETCURSEL, 0, 0);
        solicitud.idioma = idioma == 1 ? L"en" : L"es";
        DateTime_GetSystemtime(estado->fechaInicio, &solicitud.fechaInicio);
        DateTime_GetSystemtime(estado->fechaFin, &solicitud.fechaFin);
        return solicitud;
    }

    static void GuardarElementoDesdeDialogo(EstadoDialogoV2* estado) {
        if (!estado || !estado->servicio) return;
        const std::wstring texto = LeerTexto(estado->campoElemento);
        const std::size_t igual = texto.find(L'=');
        if (igual == std::wstring::npos) {
            MessageBoxW(estado->padre, L"Usa el formato nombre=valor1|valor2|valor3.",
                        L"Elemento personalizado", MB_OK | MB_ICONWARNING);
            return;
        }
        const std::wstring nombre = texto.substr(0, igual);
        const std::vector<std::wstring> valores = SepararValores(texto.substr(igual + 1));
        if (!estado->servicio->GuardarElementoPersonalizado(nombre, valores)) {
            MessageBoxW(estado->padre, L"No se pudo guardar el elemento personalizado.",
                        L"Elemento personalizado", MB_OK | MB_ICONWARNING);
            return;
        }
        MessageBoxW(estado->padre, L"Elemento guardado. Puedes usarlo como {{NOMBRE}} en futuras plantillas.",
                    L"Elemento personalizado", MB_OK | MB_ICONINFORMATION);
    }

    static void IntentarCrear(EstadoDialogoV2* estado) {
        if (!estado || !estado->servicio) return;
        const SolicitudCreadorDirectoriosV2 solicitud = LeerSolicitud(estado);
        if (estado->servicio->m_persistencia && !solicitud.raiz.empty()) {
            estado->servicio->m_persistencia->GuardarCadena(Clave(L"ultimaRaiz"), solicitud.raiz);
        }
        const ResultadoCreadorDirectoriosV2 resultado = estado->servicio->CrearEstructura(solicitud);
        if (resultado.valido) {
            const std::wstring mensaje = L"Estructura creada correctamente. Directorios creados o ya existentes: " +
                                          std::to_wstring(resultado.creadas) + L".";
            MessageBoxW(estado->padre, mensaje.c_str(), L"Creador de directorios",
                        MB_OK | MB_ICONINFORMATION);
            estado->aceptado = true;
            estado->terminado = true;
            DestroyWindow(GetParent(estado->campoRaiz));
            return;
        }
        std::wstring mensaje = L"No se pudo completar la estructura.\r\n\r\n";
        const std::size_t maxErrores = std::min<std::size_t>(resultado.errores.size(), 8);
        for (std::size_t i = 0; i < maxErrores; ++i) mensaje += L"• " + resultado.errores[i] + L"\r\n";
        MessageBoxW(estado->padre, mensaje.c_str(), L"Creador de directorios",
                    MB_OK | MB_ICONWARNING);
    }

    static LRESULT CALLBACK ProcedimientoDialogo(HWND hwnd, UINT mensaje,
                                                  WPARAM wParam, LPARAM lParam) {
        auto* estado = reinterpret_cast<EstadoDialogoV2*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (mensaje == WM_NCCREATE) {
            auto* crear = reinterpret_cast<CREATESTRUCTW*>(lParam);
            estado = reinterpret_cast<EstadoDialogoV2*>(crear->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(estado));
        }
        if (!estado) return DefWindowProcW(hwnd, mensaje, wParam, lParam);
        if (mensaje == WM_CREATE) {
            INITCOMMONCONTROLSEX controles{sizeof(controles), ICC_DATE_CLASSES};
            InitCommonControlsEx(&controles);
            HINSTANCE instancia = ObtenerInstanciaModV2();
            AgregarTexto(hwnd, L"Ruta raíz", 16, 14, 120, 20);
            estado->campoRaiz = CrearEdicion(hwnd, ID_RAIZ, 140, 12, 390, 24);
            CreateWindowW(L"BUTTON", L"Examinar...", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                          536, 12, 90, 24, hwnd, reinterpret_cast<HMENU>(ID_EXAMINAR), instancia, nullptr);

            AgregarTexto(hwnd, L"Plantilla rápida", 16, 48, 120, 20);
            estado->selectorPreset = CrearCombo(hwnd, ID_PRESET, 140, 46, 300, 180);
            SendMessageW(estado->selectorPreset, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Personalizada"));
            SendMessageW(estado->selectorPreset, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Años y meses"));
            SendMessageW(estado->selectorPreset, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Universidad y semestres"));
            SendMessageW(estado->selectorPreset, CB_SETCURSEL, 1, 0);

            AgregarTexto(hwnd, L"Fecha inicial", 16, 82, 120, 20);
            estado->fechaInicio = CreateWindowExW(0, DATETIMEPICK_CLASSW, L"", WS_CHILD | WS_VISIBLE | DTS_SHORTDATECENTURYFORMAT,
                                                  140, 80, 150, 24, hwnd, reinterpret_cast<HMENU>(ID_FECHA_INICIO), instancia, nullptr);
            AgregarTexto(hwnd, L"Fecha final", 310, 82, 100, 20);
            estado->fechaFin = CreateWindowExW(0, DATETIMEPICK_CLASSW, L"", WS_CHILD | WS_VISIBLE | DTS_SHORTDATECENTURYFORMAT,
                                               410, 80, 150, 24, hwnd, reinterpret_cast<HMENU>(ID_FECHA_FIN), instancia, nullptr);
            SYSTEMTIME inicio{}; inicio.wYear = 2011; inicio.wMonth = 1; inicio.wDay = 1;
            SYSTEMTIME fin{}; fin.wYear = 2020; fin.wMonth = 12; fin.wDay = 31;
            DateTime_SetSystemtime(estado->fechaInicio, GDT_VALID, &inicio);
            DateTime_SetSystemtime(estado->fechaFin, GDT_VALID, &fin);

            AgregarTexto(hwnd, L"Prefijo", 16, 116, 120, 20);
            estado->campoPrefijo = CrearEdicion(hwnd, ID_PREFIJO, 140, 114, 150, 24);
            AgregarTexto(hwnd, L"Sufijo", 310, 116, 100, 20);
            estado->campoSufijo = CrearEdicion(hwnd, ID_SUFIJO, 410, 114, 150, 24);

            AgregarTexto(hwnd, L"Contador", 16, 150, 120, 20);
            estado->selectorEstilo = CrearCombo(hwnd, ID_ESTILO, 140, 148, 190, 180);
            for (const wchar_t* texto : {L"Arábigos (1, 2, 3)", L"Romanos (I, II, III)", L"Cardinales", L"Ordinales", L"Completos", L"Abreviados"}) {
                SendMessageW(estado->selectorEstilo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(texto));
            }
            SendMessageW(estado->selectorEstilo, CB_SETCURSEL, 0, 0);
            AgregarTexto(hwnd, L"Idioma", 350, 150, 60, 20);
            estado->selectorIdioma = CrearCombo(hwnd, ID_IDIOMA, 410, 148, 150, 180);
            SendMessageW(estado->selectorIdioma, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Español"));
            SendMessageW(estado->selectorIdioma, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"English"));
            const std::wstring idiomaConfigurado = LeerAjusteCadena(L"creadorDirectoriosIdioma");
            SendMessageW(estado->selectorIdioma, CB_SETCURSEL,
                         _wcsicmp(idiomaConfigurado.c_str(), L"en") == 0 ? 1 : 0, 0);

            AgregarTexto(hwnd, L"Estructura (una línea por nivel; usa {{YEAR}}, {{MONTH_NAME}}, {{DATE}}, {{COUNTER:1-10}}, {{SEMESTRE}})", 16, 184, 590, 22);
            estado->campoPlantilla = CrearEdicion(hwnd, ID_PLANTILLA, 16, 208, 610, 92,
                                                  ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL);
            AplicarPreset(estado, 1);

            AgregarTexto(hwnd, L"Guardar elemento (nombre=valor1|valor2|valor3)", 16, 314, 360, 20);
            estado->campoElemento = CrearEdicion(hwnd, ID_ELEMENTO, 16, 338, 460, 24);
            CreateWindowW(L"BUTTON", L"Guardar elemento", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                          486, 338, 140, 24, hwnd, reinterpret_cast<HMENU>(ID_GUARDAR_ELEMENTO), instancia, nullptr);

            CreateWindowW(L"BUTTON", L"Crear estructura", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                          380, 388, 130, 30, hwnd, reinterpret_cast<HMENU>(ID_CREAR), instancia, nullptr);
            CreateWindowW(L"BUTTON", L"Cancelar", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                          520, 388, 106, 30, hwnd, reinterpret_cast<HMENU>(ID_CANCELAR), instancia, nullptr);

            const std::wstring ultimaRaiz = estado->servicio->m_persistencia
                ? estado->servicio->m_persistencia->LeerCadena(Clave(L"ultimaRaiz"))
                : std::wstring{};
            CargarTexto(estado->campoRaiz, ultimaRaiz);
            SetFocus(estado->campoRaiz);
            return 0;
        }
        if (mensaje == WM_COMMAND) {
            const int id = LOWORD(wParam);
            const int notificacion = HIWORD(wParam);
            if (id == ID_PRESET && notificacion == CBN_SELCHANGE) {
                AplicarPreset(estado, static_cast<int>(SendMessageW(estado->selectorPreset, CB_GETCURSEL, 0, 0)));
                return 0;
            }
            if (id == ID_EXAMINAR) {
                SeleccionarCarpeta(estado);
                return 0;
            }
            if (id == ID_GUARDAR_ELEMENTO) {
                GuardarElementoDesdeDialogo(estado);
                return 0;
            }
            if (id == ID_CREAR || id == IDOK) {
                IntentarCrear(estado);
                return 0;
            }
            if (id == ID_CANCELAR || id == IDCANCEL) {
                estado->terminado = true;
                DestroyWindow(hwnd);
                return 0;
            }
        }
        if (mensaje == WM_CLOSE) {
            estado->terminado = true;
            DestroyWindow(hwnd);
            return 0;
        }
        if (mensaje == WM_NCDESTROY) {
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        }
        return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    }

    bool MostrarDialogoInterno(HWND hwndPadre) {
        static std::once_flag registro;
        static bool registrada = false;
        std::call_once(registro, [] {
            WNDCLASSW clase{};
            clase.lpfnWndProc = ProcedimientoDialogo;
            clase.hInstance = ObtenerInstanciaModV2();
            clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            clase.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            clase.lpszClassName = CLASE_DIALOGO;
            if (!clase.hInstance) return;
            if (RegisterClassW(&clase)) {
                registrada = true;
            } else if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
                WNDCLASSW existente{};
                registrada = GetClassInfoW(clase.hInstance, CLASE_DIALOGO, &existente) &&
                             existente.lpfnWndProc == ProcedimientoDialogo;
            }
        });
        if (!registrada) return false;
        HINSTANCE instancia = ObtenerInstanciaModV2();
        if (!instancia) return false;

        EstadoDialogoV2 estado{};
        estado.servicio = this;
        estado.padre = hwndPadre;
        if (hwndPadre) EnableWindow(hwndPadre, FALSE);
        HWND ventana = CreateWindowExW(WS_EX_DLGMODALFRAME, CLASE_DIALOGO,
                                       L"Useful Side View - Creador de directorios",
                                       WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                       CW_USEDEFAULT, CW_USEDEFAULT, 660, 470,
                                       hwndPadre, nullptr, instancia, &estado);
        if (!ventana) {
            if (hwndPadre) EnableWindow(hwndPadre, TRUE);
            return false;
        }
        RECT rcPadre{}, rcVentana{};
        if (hwndPadre && GetWindowRect(hwndPadre, &rcPadre) && GetWindowRect(ventana, &rcVentana)) {
            SetWindowPos(ventana, nullptr,
                         rcPadre.left + ((rcPadre.right - rcPadre.left) - (rcVentana.right - rcVentana.left)) / 2,
                         rcPadre.top + ((rcPadre.bottom - rcPadre.top) - (rcVentana.bottom - rcVentana.top)) / 2,
                         0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        ShowWindow(ventana, SW_SHOW);
        UpdateWindow(ventana);
        MSG mensaje{};
        while (!estado.terminado) {
            const BOOL obtenido = GetMessageW(&mensaje, nullptr, 0, 0);
            if (obtenido <= 0) {
                estado.terminado = true;
                break;
            }
            if (!IsDialogMessageW(ventana, &mensaje)) {
                TranslateMessage(&mensaje);
                DispatchMessageW(&mensaje);
            }
        }
        if (hwndPadre) {
            EnableWindow(hwndPadre, TRUE);
            SetForegroundWindow(hwndPadre);
        }
        return estado.aceptado;
    }

    std::shared_ptr<ServicioPersistenciaWindhawkV2> m_persistencia;
    std::vector<ElementoPersonalizadoDirectoriosV2> m_elementos;
};

inline constexpr wchar_t ServicioCreadorDirectoriosV2::CLASE_DIALOGO[];

} // namespace SideViewV2
