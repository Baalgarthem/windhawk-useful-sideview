#pragma once

#include <windows.h>
#include <atomic>
#include <string>
#include <vector>
#include <thread>
#include <exception>
#include <mutex>
#include <cwchar>

namespace SideViewV2 {

struct ResultadoFusionCarpetasV2 {
    size_t elementosTransferidos{0};
    size_t errores{0};
};

class ServicioFusionCarpetasV2 {
public:
    static constexpr UINT WM_FUSION_COMPLETADA = WM_APP + 0x4A1;

    ~ServicioFusionCarpetasV2() { FinalizarTrabajo(); }

    bool EjecutarEnSegundoPlano(const std::vector<std::wstring>& origenes,
                                const std::wstring& destino,
                                HWND hwndNotificacion) {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        if (m_hilo.joinable()) {
            if (!m_trabajoTerminado.load(std::memory_order_acquire)) return false;
            m_hilo.join();
        }
        m_cancelar.store(false, std::memory_order_release);
        m_trabajoTerminado.store(false, std::memory_order_release);
        try {
            m_hilo = std::thread([this, origenes, destino, hwndNotificacion]() {
                ResultadoFusionCarpetasV2 resultado{};
                try {
                    resultado = Ejecutar(origenes, destino);
                } catch (...) {
                    // Ninguna excepción del trabajo de archivos debe escapar del hilo del mod.
                    resultado.errores = 1;
                }
                {
                    std::lock_guard<std::mutex> resultadoLock(m_bloqueo);
                    m_ultimoResultado = resultado;
                }
                m_trabajoTerminado.store(true, std::memory_order_release);
                if (IsWindow(hwndNotificacion)) PostMessageW(hwndNotificacion, WM_FUSION_COMPLETADA, 0, 0);
            });
            return true;
        } catch (const std::exception&) {
            // La operación queda sin iniciar si el sistema no puede crear el hilo.
            return false;
        }
    }

    ResultadoFusionCarpetasV2 UltimoResultado() const {
        std::lock_guard<std::mutex> lock(m_bloqueo);
        return m_ultimoResultado;
    }

    void FinalizarTrabajo() {
        m_cancelar.store(true, std::memory_order_release);
        std::thread trabajo;
        {
            std::lock_guard<std::mutex> lock(m_bloqueo);
            if (!m_hilo.joinable()) return;
            trabajo = std::move(m_hilo);
        }
        if (trabajo.joinable()) trabajo.join();
    }

    ResultadoFusionCarpetasV2 Ejecutar(const std::vector<std::wstring>& origenes,
                                       const std::wstring& destino) {
        ResultadoFusionCarpetasV2 resultado{};
        if (origenes.empty() || destino.empty()) { resultado.errores = 1; return resultado; }
        const std::wstring destinoCompleto = RutaCompleta(destino);
        std::vector<std::wstring> origenesCompletos;
        origenesCompletos.reserve(origenes.size());
        for (const auto& origen : origenes) origenesCompletos.push_back(RutaCompleta(origen));
        for (size_t i = 0; i < origenes.size(); ++i) {
            if (m_cancelar.load(std::memory_order_acquire)) {
                resultado.errores = origenes.size();
                return resultado;
            }
            const std::wstring& origenCompleto = origenesCompletos[i];
            if (EsMismaRutaOContenida(origenCompleto, destinoCompleto)) {
                resultado.errores = origenes.size();
                return resultado;
            }
            for (size_t j = i + 1; j < origenes.size(); ++j) {
                const std::wstring& otroOrigen = origenesCompletos[j];
                if (EsMismaRutaOContenida(origenCompleto, otroOrigen) ||
                    EsMismaRutaOContenida(otroOrigen, origenCompleto)) {
                    resultado.errores = origenes.size();
                    return resultado;
                }
            }
        }
        DWORD atributos = GetFileAttributesW(destino.c_str());
        if (atributos != INVALID_FILE_ATTRIBUTES && (!(atributos & FILE_ATTRIBUTE_DIRECTORY) || (atributos & FILE_ATTRIBUTE_REPARSE_POINT))) {
            resultado.errores = origenes.size();
            return resultado;
        }
        if (atributos == INVALID_FILE_ATTRIBUTES && !CreateDirectoryW(destino.c_str(), nullptr)) {
            resultado.errores = origenes.size();
            return resultado;
        }
        for (const auto& origen : origenes) {
            if (m_cancelar.load(std::memory_order_acquire)) {
                ++resultado.errores;
                break;
            }
            DWORD origenAtributos = GetFileAttributesW(origen.c_str());
            if (origenAtributos == INVALID_FILE_ATTRIBUTES || !(origenAtributos & FILE_ATTRIBUTE_DIRECTORY) || (origenAtributos & FILE_ATTRIBUTE_REPARSE_POINT)) {
                ++resultado.errores;
                continue;
            }
            size_t erroresAntes = resultado.errores;
            Transferir(origen, destino, resultado, 0);
            if (resultado.errores == erroresAntes && !RemoveDirectoryW(origen.c_str())) ++resultado.errores;
        }
        return resultado;
    }

private:
    static std::wstring RutaCompleta(const std::wstring& ruta) {
        DWORD capacidad = MAX_PATH;
        while (capacidad <= 32768) {
            std::vector<wchar_t> buffer(capacidad, L'\0');
            DWORD longitud = GetFullPathNameW(ruta.c_str(), capacidad, buffer.data(), nullptr);
            if (longitud == 0) return ruta;
            if (longitud < capacidad) {
                std::wstring completa(buffer.data(), longitud);
                while (completa.size() > 3 &&
                       (completa.back() == L'\\' || completa.back() == L'/')) {
                    completa.pop_back();
                }
                return completa;
            }
            capacidad *= 2;
        }
        return ruta;
    }

    static bool EsMismaRutaOContenida(const std::wstring& padre,
                                      const std::wstring& posibleDescendiente) {
        if (_wcsicmp(padre.c_str(), posibleDescendiente.c_str()) == 0) return true;
        if (posibleDescendiente.size() <= padre.size() ||
            _wcsnicmp(padre.c_str(), posibleDescendiente.c_str(), padre.size()) != 0) {
            return false;
        }
        wchar_t separador = posibleDescendiente[padre.size()];
        return separador == L'\\' || separador == L'/';
    }

    void Transferir(const std::wstring& origen, const std::wstring& destinoActual,
                    ResultadoFusionCarpetasV2& resultado, int profundidad) const {
        if (profundidad >= 64) { ++resultado.errores; return; }
        WIN32_FIND_DATAW datos{};
        HANDLE busqueda = FindFirstFileW((origen + L"\\*").c_str(), &datos);
        if (busqueda == INVALID_HANDLE_VALUE) { ++resultado.errores; return; }
        bool enumeracionCompleta = true;
        do {
            if (m_cancelar.load(std::memory_order_acquire)) {
                ++resultado.errores;
                enumeracionCompleta = false;
                break;
            }
            if (!wcscmp(datos.cFileName, L".") || !wcscmp(datos.cFileName, L"..")) continue;
            std::wstring origenItem = origen + L"\\" + datos.cFileName;
            std::wstring destinoItem = destinoActual + L"\\" + datos.cFileName;
            if (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) continue;
                DWORD destinoAtributos = GetFileAttributesW(destinoItem.c_str());
                if (destinoAtributos != INVALID_FILE_ATTRIBUTES && !(destinoAtributos & FILE_ATTRIBUTE_DIRECTORY)) { ++resultado.errores; continue; }
                if (destinoAtributos == INVALID_FILE_ATTRIBUTES && !CreateDirectoryW(destinoItem.c_str(), nullptr)) { ++resultado.errores; continue; }
                size_t erroresAntes = resultado.errores;
                Transferir(origenItem, destinoItem, resultado, profundidad + 1);
                if (resultado.errores == erroresAntes && RemoveDirectoryW(origenItem.c_str())) {
                    ++resultado.elementosTransferidos;
                }
            } else if (MoveFileExW(origenItem.c_str(), destinoItem.c_str(), MOVEFILE_COPY_ALLOWED)) {
                ++resultado.elementosTransferidos;
            } else {
                ++resultado.errores;
            }
        } while (FindNextFileW(busqueda, &datos));
        if (enumeracionCompleta && GetLastError() != ERROR_NO_MORE_FILES) ++resultado.errores;
        FindClose(busqueda);
    }
    std::thread m_hilo;
    std::atomic_bool m_cancelar{false};
    std::atomic_bool m_trabajoTerminado{false};
    ResultadoFusionCarpetasV2 m_ultimoResultado{};
    mutable std::mutex m_bloqueo;
};
}
