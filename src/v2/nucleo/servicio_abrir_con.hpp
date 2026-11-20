#pragma once

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <algorithm>
#include <cwchar>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include "v2/nucleo/servicio_persistencia_windhawk.hpp"

namespace SideViewV2 {

class ServicioAbrirConV2 {
public:
    ServicioAbrirConV2(
        std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia,
        std::wstring clavePersistencia,
        std::vector<std::wstring> inicial = {})
        : m_persistencia(std::move(persistencia)),
          m_clavePersistencia(std::move(clavePersistencia)),
          m_programas(std::move(inicial)) {
        CargarPersistencia();
    }

    std::vector<std::wstring> Programas() const {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        return m_programas;
    }

    std::wstring ProgramaActual() const {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        return m_ultimo.empty() && m_programas.size() == 1
            ? m_programas.front() : m_ultimo;
    }

    void Seleccionar(const std::wstring& ruta) {
        if (ruta.empty()) return;
        std::lock_guard<std::mutex> lock(m_bloqueo);
        const auto encontrado = std::find_if(
            m_programas.begin(), m_programas.end(),
            [&ruta](const std::wstring& programa) {
                return EsMismoPrograma(programa, ruta);
            });
        if (encontrado == m_programas.end()) m_programas.push_back(ruta);
        if (!m_ultimo.empty() && EsMismoPrograma(m_ultimo, ruta)) return;
        m_ultimo = ruta;
        GuardarPersistenciaBloqueada();
    }

    bool AbrirDialogo(HWND owner) {
        wchar_t ruta[MAX_PATH]{};
        OPENFILENAMEW dialogo{};
        dialogo.lStructSize = sizeof(dialogo);
        dialogo.hwndOwner = owner;
        dialogo.lpstrFilter = L"Programas ejecutables (*.exe)\0*.exe\0"
                              L"Todos los archivos (*.*)\0*.*\0";
        dialogo.lpstrFile = ruta;
        dialogo.nMaxFile = ARRAYSIZE(ruta);
        dialogo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (!GetOpenFileNameW(&dialogo)) return false;
        Seleccionar(ruta);
        return true;
    }

    bool Ejecutar(const std::vector<std::wstring>& archivos) {
        const std::wstring programa = ProgramaActual();
        if (programa.empty()) return false;

        // Registra el uso también en las vistas que no tienen un selector
        // visible propio, antes de lanzar el programa elegido.
        Seleccionar(programa);

        bool algunoAbierto = false;
        for (const auto& archivo : archivos) {
            HINSTANCE resultado = ShellExecuteW(
                nullptr, L"open", programa.c_str(),
                (L"\"" + archivo + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
            if (reinterpret_cast<INT_PTR>(resultado) > 32) algunoAbierto = true;
        }
        return algunoAbierto;
    }

private:
    static constexpr int kVersionPersistencia = 1;
    static constexpr int kCantidadMaximaSegura = 100000;

    static bool EsMismoPrograma(const std::wstring& izquierdo,
                                const std::wstring& derecho) {
        return _wcsicmp(izquierdo.c_str(), derecho.c_str()) == 0;
    }

    std::wstring ClaveNueva(const wchar_t* sufijo) const {
        return m_persistencia
            ? m_persistencia->CrearClave(L"abrirCon." + m_clavePersistencia, sufijo)
            : std::wstring{};
    }

    static std::wstring ClaveLegacy(const std::wstring& dominio,
                                    const wchar_t* sufijo) {
        return ServicioPersistenciaWindhawkV2::CrearClaveLegacy(
            dominio, sufijo);
    }

    void AgregarSiNoExiste(const std::wstring& programa) {
        if (programa.empty()) return;
        const auto encontrado = std::find_if(
            m_programas.begin(), m_programas.end(),
            [&programa](const std::wstring& existente) {
                return EsMismoPrograma(existente, programa);
            });
        if (encontrado == m_programas.end()) m_programas.push_back(programa);
    }

    bool CargarFormatoNuevo() {
        if (!m_persistencia) return false;
        if (m_persistencia->LeerEntero(ClaveNueva(L"schema"), 0) <
            kVersionPersistencia) return false;

        const int cantidad = m_persistencia->LeerEntero(
            ClaveNueva(L"count"), -1);
        if (cantidad < 0 || cantidad > kCantidadMaximaSegura) return false;

        for (int i = 0; i < cantidad; ++i) {
            const std::wstring clave = ClaveNueva(
                (L"program." + std::to_wstring(i)).c_str());
            AgregarSiNoExiste(m_persistencia->LeerCadena(clave));
        }
        m_ultimo = m_persistencia->LeerCadena(ClaveNueva(L"last"));
        AgregarSiNoExiste(m_ultimo);
        return true;
    }

    bool CargarFormatoLegacy() {
        if (!m_persistencia || m_clavePersistencia.empty()) return false;
        const std::wstring textoCantidad = m_persistencia->LeerCadena(
            ClaveLegacy(m_clavePersistencia, L"count"));
        if (textoCantidad.empty()) return false;

        wchar_t* fin = nullptr;
        const unsigned long long cantidadRaw = wcstoull(
            textoCantidad.c_str(), &fin, 10);
        if (fin == textoCantidad.c_str() || *fin != L'\0' ||
            cantidadRaw > static_cast<unsigned long long>(kCantidadMaximaSegura) ||
            cantidadRaw > std::numeric_limits<size_t>::max()) {
            return false;
        }

        const size_t cantidad = static_cast<size_t>(cantidadRaw);
        for (size_t i = 0; i < cantidad; ++i) {
            const std::wstring clave = ClaveLegacy(
                m_clavePersistencia,
                (L"program." + std::to_wstring(i)).c_str());
            AgregarSiNoExiste(m_persistencia->LeerCadena(clave));
        }
        m_ultimo = m_persistencia->LeerCadena(
            ClaveLegacy(m_clavePersistencia, L"last"));
        AgregarSiNoExiste(m_ultimo);
        return true;
    }

    void CargarPersistencia() {
        if (m_clavePersistencia.empty() || !m_persistencia) return;
        if (CargarFormatoNuevo()) return;

        // Migración idempotente: conserva las claves históricas y publica el
        // formato nuevo únicamente después de cargar un estado válido.
        if (CargarFormatoLegacy()) GuardarPersistenciaBloqueada();
    }

    void GuardarPersistenciaBloqueada() {
        if (!m_persistencia || m_clavePersistencia.empty()) return;

        bool correcto = true;
        for (size_t i = 0; i < m_programas.size(); ++i) {
            const std::wstring clave = ClaveNueva(
                (L"program." + std::to_wstring(i)).c_str());
            correcto = m_persistencia->GuardarCadena(clave, m_programas[i]) &&
                       correcto;
        }
        correcto = m_persistencia->GuardarCadena(
            ClaveNueva(L"last"), m_ultimo) && correcto;
        correcto = m_persistencia->GuardarEntero(
            ClaveNueva(L"schema"), kVersionPersistencia) && correcto;

        // El contador es el marcador de commit lógico de la instantánea.
        correcto = m_persistencia->GuardarEntero(
            ClaveNueva(L"count"), static_cast<int>(m_programas.size())) &&
                   correcto;
        if (!correcto) {
            Wh_Log(L"[UsefulSideView] No fue posible guardar Abrir con (%s)",
                   m_clavePersistencia.c_str());
        }
    }

    std::shared_ptr<ServicioPersistenciaWindhawkV2> m_persistencia;
    std::wstring m_clavePersistencia;
    std::vector<std::wstring> m_programas;
    std::wstring m_ultimo;
    mutable std::mutex m_bloqueo;
};

} // namespace SideViewV2
