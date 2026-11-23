#pragma once

#include "v2/nucleo/servicio_persistencia_windhawk.hpp"
#include "v2/nucleo/ciclo_vida_v2.hpp"

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <windhawk_api.h>

#include <algorithm>
#include <cwctype>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace SideViewV2 {

struct EntradaScriptBoxV2 {
    std::wstring nombre;
    std::wstring ruta;
};

// Servicio atómico del Script Box. Las ubicaciones son persistentes y se
// mantienen en orden de incorporación. Una ubicación elegida no autoriza por
// sí sola la ejecución: cada script también debe aparecer en la lista blanca
// de ajustes "scriptBoxScripts".
class ServicioScriptBoxV2 final {
public:
    explicit ServicioScriptBoxV2(
        std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia)
        : m_persistencia(std::move(persistencia)) {
        RecargarConfiguracion();
    }

    void RecargarConfiguracion() {
        m_incluidos = SepararLineas(LeerAjusteCadena(L"scriptBoxScripts"));
        CargarUbicaciones();
    }

    const std::vector<std::wstring>& Ubicaciones() const {
        return m_ubicaciones;
    }

    std::vector<EntradaScriptBoxV2> ScriptsDeUbicacion(
        std::size_t indiceUbicacion) const {
        std::vector<EntradaScriptBoxV2> resultado;
        if (indiceUbicacion >= m_ubicaciones.size()) return resultado;

        const std::wstring& raiz = m_ubicaciones[indiceUbicacion];
        if (!EsDirectorio(raiz)) return resultado;

        for (const std::wstring& incluido : m_incluidos) {
            std::wstring candidato = incluido;
            if (!EsRutaAbsoluta(candidato)) {
                candidato = raiz + L"\\" + candidato;
            }
            candidato = NormalizarRuta(candidato);
            if (!RutaDentroDe(candidato, raiz) || !EsArchivoRegular(candidato)) continue;
            if (!EsExtensionAdmitida(ExtensionMinuscula(candidato))) continue;

            bool repetido = false;
            for (const auto& existente : resultado) {
                if (_wcsicmp(existente.ruta.c_str(), candidato.c_str()) == 0) {
                    repetido = true;
                    break;
                }
            }
            if (repetido) continue;

            const wchar_t* nombre = PathFindFileNameW(candidato.c_str());
            resultado.push_back({nombre ? nombre : candidato, std::move(candidato)});
        }
        return resultado;
    }

    bool SeleccionarUbicacion(HWND hwndPadre) {
        if (g_descargandoMod.load(std::memory_order_acquire)) return false;

        BROWSEINFOW info{};
        info.hwndOwner = hwndPadre;
        info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
        info.lpszTitle = L"Selecciona una carpeta de scripts de Script Box";
        PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&info);
        if (!pidl) return false;

        wchar_t ruta[MAX_PATH * 4]{};
        const bool obtenida = SHGetPathFromIDListW(pidl, ruta) != FALSE;
        CoTaskMemFree(pidl);
        if (!obtenida || !ruta[0]) return false;

        const std::wstring normalizada = NormalizarRuta(ruta);
        if (normalizada.empty() || !EsDirectorio(normalizada)) return false;

        AgregarUbicacion(normalizada, false);
        GuardarUbicaciones();
        return true;
    }

    bool Ejecutar(std::size_t indiceUbicacion, std::size_t indiceScript) const {
        if (g_descargandoMod.load(std::memory_order_acquire)) return false;
        const std::vector<EntradaScriptBoxV2> scripts = ScriptsDeUbicacion(indiceUbicacion);
        if (indiceScript >= scripts.size()) return false;
        return EjecutarRuta(scripts[indiceScript].ruta);
    }

private:
    static std::wstring Clave(const wchar_t* sufijo) {
        return L"useful-sideview.v3.scriptBox." + std::wstring(sufijo ? sufijo : L"");
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
            std::wstring linea = Recortar(texto.substr(
                inicio, fin == std::wstring::npos ? std::wstring::npos : fin - inicio));
            if (!linea.empty()) resultado.push_back(std::move(linea));
            if (fin == std::wstring::npos) break;
            inicio = fin + 1;
            while (inicio < texto.size() &&
                   (texto[inicio] == L'\r' || texto[inicio] == L'\n')) {
                ++inicio;
            }
        }
        return resultado;
    }

    static std::wstring NormalizarRuta(const std::wstring& ruta) {
        if (ruta.empty()) return {};
        wchar_t completa[MAX_PATH * 4]{};
        const DWORD longitud = GetFullPathNameW(
            ruta.c_str(), ARRAYSIZE(completa), completa, nullptr);
        if (longitud == 0 || longitud >= ARRAYSIZE(completa)) return {};
        std::wstring resultado(completa, longitud);
        while (resultado.size() > 3 && !resultado.empty() &&
               (resultado.back() == L'\\' || resultado.back() == L'/')) {
            resultado.pop_back();
        }
        return resultado;
    }

    static bool EsRutaAbsoluta(const std::wstring& ruta) {
        return (ruta.size() >= 2 && ruta[1] == L':') ||
               (ruta.size() >= 2 && ruta[0] == L'\\' && ruta[1] == L'\\');
    }

    static std::wstring ExtensionMinuscula(const std::wstring& ruta) {
        const std::size_t punto = ruta.find_last_of(L'.');
        const std::size_t separador = ruta.find_last_of(L"\\/");
        if (punto == std::wstring::npos ||
            (separador != std::wstring::npos && punto < separador)) {
            return {};
        }
        std::wstring extension = ruta.substr(punto);
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](wchar_t caracter) {
                           return static_cast<wchar_t>(std::towlower(caracter));
                       });
        return extension;
    }

    static bool EsExtensionAdmitida(const std::wstring& extension) {
        return extension == L".ps1" || extension == L".cmd" || extension == L".bat" ||
               extension == L".vbs" || extension == L".js" || extension == L".py";
    }

    static bool EsDirectorio(const std::wstring& ruta) {
        const DWORD atributos = GetFileAttributesW(ruta.c_str());
        return atributos != INVALID_FILE_ATTRIBUTES &&
               (atributos & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    static bool EsArchivoRegular(const std::wstring& ruta) {
        const DWORD atributos = GetFileAttributesW(ruta.c_str());
        return atributos != INVALID_FILE_ATTRIBUTES &&
               (atributos & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    static bool RutaDentroDe(const std::wstring& ruta, const std::wstring& raiz) {
        if (ruta.empty() || raiz.empty()) return false;
        if (_wcsnicmp(ruta.c_str(), raiz.c_str(), raiz.size()) != 0) return false;
        return ruta.size() == raiz.size() ||
               (ruta.size() > raiz.size() &&
                (ruta[raiz.size()] == L'\\' || ruta[raiz.size()] == L'/'));
    }

    static bool EjecutarConInterprete(const wchar_t* interprete,
                                      const std::wstring& parametros,
                                      const wchar_t* directorio) {
        if (!interprete || !*interprete) return false;
        SHELLEXECUTEINFOW ejecutar{};
        ejecutar.cbSize = sizeof(ejecutar);
        ejecutar.fMask = SEE_MASK_NOASYNC;
        ejecutar.lpVerb = L"open";
        ejecutar.lpFile = interprete;
        ejecutar.lpParameters = parametros.c_str();
        ejecutar.lpDirectory = directorio;
        ejecutar.nShow = SW_SHOWNORMAL;
        return ShellExecuteExW(&ejecutar) != FALSE;
    }

    static bool EjecutarRuta(const std::wstring& ruta) {
        if (!EsArchivoRegular(ruta)) return false;

        wchar_t directorio[MAX_PATH * 4]{};
        wcsncpy_s(directorio, ruta.c_str(), _TRUNCATE);
        PathRemoveFileSpecW(directorio);

        const std::wstring extension = ExtensionMinuscula(ruta);
        if (extension == L".ps1") {
            const std::wstring parametros =
                L"-NoLogo -NoProfile -ExecutionPolicy Bypass -File \"" + ruta + L"\"";
            return EjecutarConInterprete(L"powershell.exe", parametros, directorio);
        }
        if (extension == L".py") {
            const std::wstring parametros = L"-3 \"" + ruta + L"\"";
            if (EjecutarConInterprete(L"py.exe", parametros, directorio)) return true;
            return EjecutarConInterprete(L"python.exe", L"\"" + ruta + L"\"", directorio);
        }

        const HINSTANCE resultado = ShellExecuteW(
            nullptr, L"open", ruta.c_str(), nullptr, directorio, SW_SHOWNORMAL);
        return reinterpret_cast<INT_PTR>(resultado) > 32;
    }

    void AgregarUbicacion(const std::wstring& ruta, bool alFrente) {
        const std::wstring normalizada = NormalizarRuta(ruta);
        if (normalizada.empty()) return;
        for (const std::wstring& existente : m_ubicaciones) {
            if (_wcsicmp(existente.c_str(), normalizada.c_str()) == 0) return;
        }
        if (alFrente) {
            m_ubicaciones.insert(m_ubicaciones.begin(), normalizada);
        } else {
            m_ubicaciones.push_back(normalizada);
        }
    }

    void CargarUbicaciones() {
        m_ubicaciones.clear();

        if (m_persistencia) {
            const std::wstring serializadas =
                m_persistencia->LeerCadena(Clave(L"ubicaciones"));
            for (const std::wstring& ruta : SepararLineas(serializadas)) {
                AgregarUbicacion(ruta, false);
            }

            // Migración transparente de la versión anterior de Script Box.
            if (serializadas.empty()) {
                AgregarUbicacion(
                    m_persistencia->LeerCadena(Clave(L"ubicacion")), false);
            }
        }

        // Un ajuste explícito nuevo se incorpora al frente; los demás ajustes
        // se agregan sin alterar el orden persistido por el usuario.
        AgregarUbicacion(LeerAjusteCadena(L"scriptBoxUbicacion"), true);
        for (const std::wstring& ruta : SepararLineas(
                 LeerAjusteCadena(L"scriptBoxUbicaciones"))) {
            AgregarUbicacion(ruta, false);
        }
    }

    void GuardarUbicaciones() const {
        if (!m_persistencia) return;
        std::wstring serializadas;
        for (const std::wstring& ruta : m_ubicaciones) {
            if (!serializadas.empty()) serializadas.push_back(L'\n');
            serializadas += ruta;
        }
        m_persistencia->GuardarCadena(Clave(L"ubicaciones"), serializadas);
        if (!m_ubicaciones.empty()) {
            // Mantiene compatibilidad con builds V2 anteriores que sólo
            // conocían una ubicación.
            m_persistencia->GuardarCadena(Clave(L"ubicacion"), m_ubicaciones.front());
        }
    }

    std::shared_ptr<ServicioPersistenciaWindhawkV2> m_persistencia;
    std::vector<std::wstring> m_ubicaciones;
    std::vector<std::wstring> m_incluidos;
};

} // namespace SideViewV2
