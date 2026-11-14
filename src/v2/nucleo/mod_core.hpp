#pragma once

#include <windows.h>
#include <vector>
#include <windhawk_api.h>
#include <windhawk_utils.h>
#include "v2/nucleo/gestor_ventanas.hpp"
#include "v2/nucleo/hook_teclado.hpp"

namespace SideViewV2 {

using CreateWindowExW_t = decltype(&CreateWindowExW);
inline CreateWindowExW_t CreateWindowExW_Original = nullptr;

inline HWND WINAPI CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName,
                                        LPCWSTR lpWindowName, DWORD dwStyle,
                                        int X, int Y, int nWidth, int nHeight,
                                        HWND hWndParent, HMENU hMenu,
                                        HINSTANCE hInstance, LPVOID lpParam) {
    HWND hwnd = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName,
                                         dwStyle, X, Y, nWidth, nHeight,
                                         hWndParent, hMenu, hInstance, lpParam);
    if (!hwnd || g_descargandoMod.load()) {
        return hwnd;
    }

    if (!IS_INTRESOURCE(lpClassName) && (_wcsicmp(lpClassName, L"CabinetWClass") == 0 ||
                                         _wcsicmp(lpClassName, L"ExploreWClass") == 0)) {
        WindhawkUtils::SetWindowSubclassFromAnyThread(hwnd, ProcedimientoSubclaseTop, 0);
    }

    return hwnd;
}

inline BOOL CALLBACK EnumVentanasExplorerCallback(HWND hwnd, LPARAM lParam) {
    DWORD idProceso = 0;
    GetWindowThreadProcessId(hwnd, &idProceso);
    if (idProceso != GetCurrentProcessId()) return TRUE;

    if (EsClaseExplorer(hwnd)) {
        reinterpret_cast<std::vector<HWND>*>(lParam)->push_back(hwnd);
    }
    return TRUE;
}

inline std::vector<HWND> ObtenerVentanasExplorerActuales() {
    std::vector<HWND> ventanas;
    EnumWindows(EnumVentanasExplorerCallback, reinterpret_cast<LPARAM>(&ventanas));
    return ventanas;
}

inline void CargarConfiguracionesV2() {
    PCWSTR atajoStr = Wh_GetStringSetting(L"atajoTeclado");
    if (atajoStr && *atajoStr != L'\0') {
        ConfigurarAtajoDesdeString(atajoStr);
        Wh_FreeStringSetting(atajoStr);
    } else {
        ConfigurarAtajoDesdeString(L"Ctrl+K");
    }

    int limite = Wh_GetIntSetting(L"limiteCarpetasPesadas");
    if (limite > 0) {
        g_limiteCarpetasPesadasConfig.store(limite);
    } else {
        g_limiteCarpetasPesadasConfig.store(3);
    }
}

inline bool InicializarModV2() {
    WM_SIDEVIEW_TOGGLE = RegisterWindowMessageW(L"UsefulSideView_WM_Toggle_V2");
    WM_SIDEVIEW_SETTINGS = RegisterWindowMessageW(L"UsefulSideView_WM_Settings_V2");
    WM_SIDEVIEW_CLEANUP = RegisterWindowMessageW(L"UsefulSideView_WM_Cleanup_V2");

    if (!WM_SIDEVIEW_TOGGLE || !WM_SIDEVIEW_SETTINGS || !WM_SIDEVIEW_CLEANUP) {
        return false;
    }

    CargarConfiguracionesV2();

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    if (!RegistrarClasePanelV2(hInst)) {
        return false;
    }

    if (!WindhawkUtils::SetFunctionHook(CreateWindowExW, CreateWindowExW_Hook,
                                        &CreateWindowExW_Original)) {
        UnregisterClassW(CLASE_PANEL_V2, hInst);
        return false;
    }

    return true;
}

inline void DespuesDeIniciarModV2() {
    // Subclasear las ventanas existentes de Explorer
    auto ventanas = ObtenerVentanasExplorerActuales();
    for (HWND w : ventanas) {
        WindhawkUtils::SetWindowSubclassFromAnyThread(w, ProcedimientoSubclaseTop, 0);
    }

    // Iniciar el hook de teclado en su propio hilo
    IniciarHookTeclado();
}

inline void AlCambiarConfiguracionesV2() {
    CargarConfiguracionesV2();
}

inline void DesinicializarModV2() {
    g_descargandoMod = true;
    DetenerHookTeclado();

    // Limpiar estados de todas las ventanas
    {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        for (auto& par : g_estadosPorVentana) {
            if (par.second) {
                DestruirEstado(par.second, false);
            }
        }
        g_estadosPorVentana.clear();
    }

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    UnregisterClassW(CLASE_PANEL_V2, hInst);
}

} // namespace SideViewV2
