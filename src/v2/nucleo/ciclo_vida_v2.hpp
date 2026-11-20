#pragma once

#include <windows.h>
#include <atomic>
#include <utility>
#include <vector>

namespace SideViewV2 {

// Estado global de quiescencia. Todas las entradas a callbacks que pueden
// ejecutarse fuera del hilo de configuración deben consultar esta bandera antes
// de publicar trabajo nuevo.
inline std::atomic_bool g_descargandoMod{false};
inline std::atomic_uint g_callbacksActivosV2{0};

struct AmbitoCallbackV2 {
    AmbitoCallbackV2() {
        g_callbacksActivosV2.fetch_add(1, std::memory_order_acq_rel);
    }

    ~AmbitoCallbackV2() {
        g_callbacksActivosV2.fetch_sub(1, std::memory_order_acq_rel);
    }

    AmbitoCallbackV2(const AmbitoCallbackV2&) = delete;
    AmbitoCallbackV2& operator=(const AmbitoCallbackV2&) = delete;
};

inline HINSTANCE ObtenerInstanciaModV2() {
    HMODULE modulo = nullptr;
    const auto direccion = reinterpret_cast<LPCWSTR>(
        reinterpret_cast<void*>(&ObtenerInstanciaModV2));
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            direccion, &modulo)) {
        return nullptr;
    }
    return modulo;
}

inline void DrenarCallbacksV2(DWORD tiempoMaximoMs = 2000) {
    const ULONGLONG inicio = GetTickCount64();
    while (g_callbacksActivosV2.load(std::memory_order_acquire) != 0) {
        if (GetTickCount64() - inicio >= tiempoMaximoMs) break;
        Sleep(1);
    }
}

inline BOOL CALLBACK EnumerarVentanasClaseV2(HWND hwnd, LPARAM lParam) {
    DWORD proceso = 0;
    GetWindowThreadProcessId(hwnd, &proceso);
    if (proceso != GetCurrentProcessId()) return TRUE;

    auto* datos = reinterpret_cast<std::pair<PCWSTR, std::vector<HWND>>*>(lParam);
    if (!datos || !datos->first) return TRUE;

    wchar_t clase[96]{};
    if (GetClassNameW(hwnd, clase, ARRAYSIZE(clase)) > 0 &&
        _wcsicmp(clase, datos->first) == 0) {
        datos->second.push_back(hwnd);
    }
    return TRUE;
}

inline void CerrarVentanasClaseV2(PCWSTR nombreClase) {
    if (!nombreClase || !*nombreClase) return;
    std::vector<HWND> ventanas;
    std::pair<PCWSTR, std::vector<HWND>> datos{nombreClase, {}};
    EnumWindows(EnumerarVentanasClaseV2, reinterpret_cast<LPARAM>(&datos));
    ventanas.swap(datos.second);
    for (HWND hwnd : ventanas) {
        if (IsWindow(hwnd)) SendMessageW(hwnd, WM_CLOSE, 0, 0);
    }
}

} // namespace SideViewV2
