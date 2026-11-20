#pragma once

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <windhawk_api.h>
#include <algorithm>
#include <cwchar>
#include <mutex>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace SideViewV2 {

class ServicioAbrirConV2 {
public:
    ServicioAbrirConV2(std::wstring clavePersistencia,
                       std::vector<std::wstring> inicial = {})
        : m_clavePersistencia(std::move(clavePersistencia)),
          m_programas(std::move(inicial)) {
        CargarPersistencia();
    }

    std::vector<std::wstring> Programas() const {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        return m_programas;
    }

    std::wstring ProgramaActual() const {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        return m_ultimo.empty() && m_programas.size() == 1 ? m_programas.front() : m_ultimo;
    }

    void Seleccionar(const std::wstring& ruta) {
        if (ruta.empty()) return;
        std::lock_guard<std::mutex> lock(m_bloqueo);
        bool agregado = false;
        size_t indiceNuevo = 0;
        if (std::find_if(m_programas.begin(), m_programas.end(),
                         [&ruta](const std::wstring& programa) { return EsMismoPrograma(programa, ruta); }) == m_programas.end()) {
            m_programas.push_back(ruta);
            agregado = true;
            indiceNuevo = m_programas.size() - 1;
        }
        if (!agregado && EsMismoPrograma(m_ultimo, ruta)) return;
        m_ultimo = ruta;
        GuardarPersistenciaBloqueada(agregado ? indiceNuevo : m_programas.size());
    }

    bool AbrirDialogo(HWND owner) {
        wchar_t ruta[MAX_PATH]{};
        OPENFILENAMEW dialogo{};
        dialogo.lStructSize = sizeof(dialogo);
        dialogo.hwndOwner = owner;
        dialogo.lpstrFilter = L"Programas ejecutables (*.exe)\0*.exe\0Todos los archivos (*.*)\0*.*\0";
        dialogo.lpstrFile = ruta;
        dialogo.nMaxFile = ARRAYSIZE(ruta);
        dialogo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (!GetOpenFileNameW(&dialogo)) return false;
        Seleccionar(ruta);
        return true;
    }

    bool Ejecutar(const std::vector<std::wstring>& archivos) const {
        std::wstring programa = ProgramaActual();
        if (programa.empty()) return false;
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
    static bool EsMismoPrograma(const std::wstring& izquierdo, const std::wstring& derecho) {
        return _wcsicmp(izquierdo.c_str(), derecho.c_str()) == 0;
    }

    std::wstring Clave(const wchar_t* sufijo) const {
        return L"abrirCon.v2." + m_clavePersistencia + L"." + sufijo;
    }

    static std::wstring LeerCadena(const std::wstring& clave) {
        size_t capacidad = 512;
        while (capacidad <= 32768) {
            std::vector<wchar_t> buffer(capacidad, L'\0');
            size_t copiados = Wh_GetStringValue(clave.c_str(), buffer.data(), buffer.size());
            if (copiados > 0 && copiados < buffer.size()) return std::wstring(buffer.data(), copiados);
            capacidad *= 2;
        }
        return {};
    }

    void CargarPersistencia() {
        if (m_clavePersistencia.empty()) return;
        std::wstring textoCantidad = LeerCadena(Clave(L"count").c_str());
        if (textoCantidad.empty()) return;
        wchar_t* fin = nullptr;
        unsigned long long cantidadRaw = wcstoull(textoCantidad.c_str(), &fin, 10);
        if (fin == textoCantidad.c_str() || *fin != L'\0' || cantidadRaw > 1000000ULL || cantidadRaw > std::numeric_limits<size_t>::max()) return;
        size_t cantidad = static_cast<size_t>(cantidadRaw);
        for (size_t i = 0; i < cantidad; ++i) {
            std::wstring indice = std::to_wstring(i);
            std::wstring programa = LeerCadena(Clave((L"program." + indice).c_str()));
            if (!programa.empty() && std::find_if(m_programas.begin(), m_programas.end(),
                                                  [&programa](const std::wstring& existente) {
                                                      return EsMismoPrograma(existente, programa);
                                                  }) == m_programas.end()) {
                m_programas.push_back(std::move(programa));
            }
        }
        m_ultimo = LeerCadena(Clave(L"last").c_str());
        if (!m_ultimo.empty() && std::find_if(m_programas.begin(), m_programas.end(),
                                              [this](const std::wstring& existente) {
                                                  return EsMismoPrograma(existente, m_ultimo);
                                              }) == m_programas.end()) {
            m_programas.push_back(m_ultimo);
        }
    }

    void GuardarPersistenciaBloqueada(size_t indiceNuevo) {
        if (m_clavePersistencia.empty()) return;
        if (indiceNuevo < m_programas.size()) {
            std::wstring indice = std::to_wstring(indiceNuevo);
            std::wstring clavePrograma = Clave((L"program." + indice).c_str());
            Wh_SetStringValue(clavePrograma.c_str(), m_programas[indiceNuevo].c_str());
        }
        std::wstring cantidad = std::to_wstring(m_programas.size());
        Wh_SetStringValue(Clave(L"count").c_str(), cantidad.c_str());
        Wh_SetStringValue(Clave(L"last").c_str(), m_ultimo.c_str());
    }

    std::wstring m_clavePersistencia;
    std::vector<std::wstring> m_programas;
    std::wstring m_ultimo;
    mutable std::mutex m_bloqueo;
};

} // namespace SideViewV2
