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

// Servicio atómico del toolbox Script Box. La lista visible no se obtiene
// enumerando toda una carpeta: sólo se resuelven los archivos expresamente
// incluidos por el usuario en los ajustes. Esto evita ejecutar o mostrar
// scripts no autorizados por la configuración del mod.
class ServicioScriptBoxV2 final {
public:
    explicit ServicioScriptBoxV2(
        std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia)
        : m_persistencia(std::move(persistencia)) {
        RecargarConfiguracion();
    }

    void RecargarConfiguracion() {
        const std::wstring rutaConfigurada = LeerAjusteCadena(L"scriptBoxUbicacion");
        const std::wstring rutaPersistida = m_persistencia
            ? m_persistencia->LeerCadena(Clave(L"ubicacion"))
            : std::wstring{};
        m_ubicacion = rutaConfigurada.empty() ? rutaPersistida : rutaConfigurada;
        m_ubicacion = NormalizarRuta(m_ubicacion);

        m_incluidos = SepararLineas(LeerAjusteCadena(L"scriptBoxScripts"));
        m_disponibles.clear();
        ResolverEntradas();
    }

    const std::wstring& Ubicacion() const { return m_ubicacion; }

    const std::vector<EntradaScriptBoxV2>& Disponibles() const {
        return m_disponibles;
    }

    bool SeleccionarUbicacion(HWND hwndPadre) {
        if (g_descargandoMod.load(std::memory_order_acquire)) return false;

        BROWSEINFOW info{};
        info.hwndOwner = hwndPadre;
        info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
        info.lpszTitle = L"Selecciona la carpeta de scripts de Script Box";
        PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&info);
        if (!pidl) return false;

        wchar_t ruta[MAX_PATH * 4]{};
        const bool obtenida = SHGetPathFromIDListW(pidl, ruta) != FALSE;
        CoTaskMemFree(pidl);
        if (!obtenida || !ruta[0]) return false;

        m_ubicacion = NormalizarRuta(ruta);
        if (m_persistencia) {
            m_persistencia->GuardarCadena(Clave(L"ubicacion"), m_ubicacion);
        }
        ResolverEntradas();
        return true;
    }

    bool Ejecutar(std::size_t indice) const {
        if (g_descargandoMod.load(std::memory_order_acquire) ||
            indice >= m_disponibles.size()) {
            return false;
        }

        const std::wstring& ruta = m_disponibles[indice].ruta;
        if (!EsArchivoRegular(ruta)) return false;

        wchar_t directorio[MAX_PATH * 4]{};
        wcsncpy_s(directorio, ruta.c_str(), _TRUNCATE);
        PathRemoveFileSpecW(directorio);

        const std::wstring extension = ExtensionMinuscula(ruta);
        if (extension == L".ps1") {
            std::wstring parametros = L"-NoLogo -NoProfile -ExecutionPolicy Bypass -File \"" +
                                       ruta + L"\"";
            return EjecutarConInterprete(L"powershell.exe", parametros, directorio);
        }
        if (extension == L".py") {
            const std::wstring parametros = L"-3 \"" + ruta + L"\"";
            if (EjecutarConInterprete(L"py.exe", parametros, directorio)) return true;
            return EjecutarConInterprete(L"python.exe", L"\"" + ruta + L"\"", directorio);
        }

        const HINSTANCE resultado = ShellExecuteW(nullptr, L"open", ruta.c_str(),
                                                  nullptr, directorio, SW_SHOWNORMAL);
        return reinterpret_cast<INT_PTR>(resultado) > 32;
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

    static std::wstring ExtensionMinuscula(const std::wstring& ruta) {
        const std::size_t punto = ruta.find_last_of(L'.');
        const std::size_t separador = ruta.find_last_of(L"\\/");
        if (punto == std::wstring::npos || (separador != std::wstring::npos && punto < separador)) {
            return {};
        }
        std::wstring extension = ruta.substr(punto);
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](wchar_t caracter) { return static_cast<wchar_t>(std::towlower(caracter)); });
        return extension;
    }

    static bool EsExtensionAdmitida(const std::wstring& extension) {
        return extension == L".ps1" || extension == L".cmd" || extension == L".bat" ||
               extension == L".vbs" || extension == L".js" || extension == L".py";
    }

    static bool EsArchivoRegular(const std::wstring& ruta) {
        const DWORD atributos = GetFileAttributesW(ruta.c_str());
        return atributos != INVALID_FILE_ATTRIBUTES &&
               (atributos & FILE_ATTRIBUTE_DIRECTORY) == 0;
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

    static bool RutaDentroDe(const std::wstring& ruta, const std::wstring& raiz) {
        if (ruta.empty() || raiz.empty()) return false;
        if (_wcsnicmp(ruta.c_str(), raiz.c_str(), raiz.size()) != 0) return false;
        return ruta.size() == raiz.size() ||
               (ruta.size() > raiz.size() &&
                (ruta[raiz.size()] == L'\\' || ruta[raiz.size()] == L'/'));
    }

    void ResolverEntradas() {
        if (m_ubicacion.empty() || !EsArchivoRegular(m_ubicacion)) {
            const DWORD atributos = GetFileAttributesW(m_ubicacion.c_str());
            if (m_ubicacion.empty() || atributos == INVALID_FILE_ATTRIBUTES ||
                (atributos & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                m_disponibles.clear();
                return;
            }
        }

        for (const std::wstring& incluido : m_incluidos) {
            std::wstring candidato = incluido;
            if (candidato.find(L':') == std::wstring::npos &&
                !(candidato.size() >= 2 && candidato[0] == L'\\' && candidato[1] == L'\\')) {
                candidato = m_ubicacion + L"\\" + candidato;
            }
            candidato = NormalizarRuta(candidato);
            if (!RutaDentroDe(candidato, m_ubicacion) || !EsArchivoRegular(candidato)) continue;
            if (!EsExtensionAdmitida(ExtensionMinuscula(candidato))) continue;

            bool repetido = false;
            for (const auto& existente : m_disponibles) {
                if (_wcsicmp(existente.ruta.c_str(), candidato.c_str()) == 0) {
                    repetido = true;
                    break;
                }
            }
            if (repetido) continue;

            const wchar_t* nombre = PathFindFileNameW(candidato.c_str());
            m_disponibles.push_back({nombre ? nombre : candidato, std::move(candidato)});
        }
    }

    std::shared_ptr<ServicioPersistenciaWindhawkV2> m_persistencia;
    std::wstring m_ubicacion;
    std::vector<std::wstring> m_incluidos;
    std::vector<EntradaScriptBoxV2> m_disponibles;
};

} // namespace SideViewV2
