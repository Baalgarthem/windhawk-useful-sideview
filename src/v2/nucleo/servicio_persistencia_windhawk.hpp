#pragma once

#include <windhawk_api.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace SideViewV2 {

// Adaptador único para los valores persistentes propios del mod. Los ajustes
// visibles siguen usando Wh_*Setting; este servicio sólo encapsula el estado
// operativo que debe sobrevivir a reinicios de Explorer y del equipo.
class ServicioPersistenciaWindhawkV2 final {
public:
    explicit ServicioPersistenciaWindhawkV2(
        std::wstring prefijo = L"useful-sideview.v3")
        : m_prefijo(std::move(prefijo)) {}

    int LeerEntero(const std::wstring& clave, int predeterminado) const {
        return Wh_GetIntValue(clave.c_str(), predeterminado);
    }

    bool GuardarEntero(const std::wstring& clave, int valor) const {
        return Wh_SetIntValue(clave.c_str(), valor) != FALSE;
    }

    std::wstring LeerCadena(const std::wstring& clave) const {
        constexpr std::size_t capacidadInicial = 256;
        constexpr std::size_t capacidadMaxima = 65536;
        std::size_t capacidad = capacidadInicial;
        while (capacidad <= capacidadMaxima) {
            std::vector<wchar_t> buffer(capacidad, L'\0');
            const std::size_t copiados = Wh_GetStringValue(
                clave.c_str(), buffer.data(), buffer.size());
            if (copiados > 0 && copiados < buffer.size()) {
                return std::wstring(buffer.data(), copiados);
            }
            // La API devuelve cero tanto para ausencia como para un buffer
            // insuficiente. Reintentar con crecimiento acotado cubre rutas
            // largas sin reservar memoria sin límite.
            capacidad *= 2;
        }
        return {};
    }

    bool GuardarCadena(const std::wstring& clave,
                       const std::wstring& valor) const {
        return Wh_SetStringValue(clave.c_str(), valor.c_str()) != FALSE;
    }

    bool Eliminar(const std::wstring& clave) const {
        return Wh_DeleteValue(clave.c_str()) != FALSE;
    }

    std::wstring CrearClave(const std::wstring& dominio,
                            const wchar_t* sufijo) const {
        return m_prefijo + L"." + dominio + L"." + (sufijo ? sufijo : L"");
    }

    static std::wstring CrearClaveLegacy(const std::wstring& dominio,
                                         const wchar_t* sufijo) {
        return L"abrirCon.v2." + dominio + L"." +
               (sufijo ? sufijo : L"");
    }

private:
    std::wstring m_prefijo;
};

} // namespace SideViewV2
