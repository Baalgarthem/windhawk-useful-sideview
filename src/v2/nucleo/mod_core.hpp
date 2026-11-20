#pragma once

#include <windows.h>
#include <algorithm>
#include <vector>
#include <windhawk_api.h>
#include <windhawk_utils.h>
#include "v2/nucleo/gestor_ventanas.hpp"
#include "v2/nucleo/hook_teclado.hpp"
#include "v2/nucleo/configuracion_v2.hpp"

namespace SideViewV2 {

using CreateWindowExW_t = decltype(&CreateWindowExW);
inline CreateWindowExW_t CreateWindowExW_Original = nullptr;
inline thread_local bool g_dentroHookCreateWindowV2{false};

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

    // La instalación de una subclase puede provocar creación de ventanas
    // auxiliares. Evitar reentrada aquí impide encadenar el hook sobre sí
    // mismo durante la construcción de Explorer.
    if (g_dentroHookCreateWindowV2) return hwnd;
    g_dentroHookCreateWindowV2 = true;

    if (!IS_INTRESOURCE(lpClassName) && (_wcsicmp(lpClassName, L"CabinetWClass") == 0 ||
                                         _wcsicmp(lpClassName, L"ExploreWClass") == 0)) {
        WindhawkUtils::SetWindowSubclassFromAnyThread(hwnd, ProcedimientoSubclaseTop, 0);
    }

    g_dentroHookCreateWindowV2 = false;

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
    } else {
        ConfigurarAtajoDesdeString(L"Ctrl+K");
    }
    if (atajoStr) {
        Wh_FreeStringSetting(atajoStr);
    }

    int limite = Wh_GetIntSetting(L"limiteCarpetasPesadas");
    g_limiteCarpetasPesadasConfig.store(std::max(1, std::min(limite, 100)));
    g_mostrarNumerosLineaV2.store(Wh_GetIntSetting(L"mostrarNumerosLinea") != 0);
    g_comparadorVideosExperimentalV2.store(
        Wh_GetIntSetting(L"comparadorVideosExperimental") != 0,
        std::memory_order_release);
    g_comparadorVideosPantallaCompletaV2.store(
        Wh_GetIntSetting(L"comparadorVideosPantallaCompleta") != 0,
        std::memory_order_release);
    g_mostrarVersionModV2.store(
        Wh_GetIntSetting(L"mostrarVersionMod") != 0,
        std::memory_order_release);
    g_modoSemanticoDiffViewV2.store(
        Wh_GetIntSetting(L"modoSemanticoDiffView") != 0,
        std::memory_order_release);
    g_sincronizacionNotasWinKlokV2.store(
        Wh_GetIntSetting(L"sincronizacionNotasWinKlok") != 0,
        std::memory_order_release);
}

inline bool InicializarModV2() {
    g_descargandoMod.store(false, std::memory_order_release);
    WM_SIDEVIEW_TOGGLE = RegisterWindowMessageW(L"UsefulSideView_WM_Toggle_V2");
    WM_SIDEVIEW_SETTINGS = RegisterWindowMessageW(L"UsefulSideView_WM_Settings_V2");

    if (!WM_SIDEVIEW_TOGGLE || !WM_SIDEVIEW_SETTINGS) {
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
    std::lock_guard<std::mutex> lock(g_bloqueoEstados);
    for (auto& par : g_estadosPorVentana) {
        if (par.second && par.second->panel && IsWindow(par.second->panel)) {
            PostMessageW(par.second->panel, WM_SIDEVIEW_SETTINGS, 0, 0);
        }
    }
}

inline void DesinicializarModV2() {
    g_descargandoMod = true;
    DetenerHookTeclado();

    // Extraer el registro antes de destruir ventanas. DestroyWindow puede
    // provocar reentrada y nunca debe ejecutarse sosteniendo este mutex.
    std::vector<EstadoPanelV2*> estados;
    {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        estados.reserve(g_estadosPorVentana.size());
        for (auto& par : g_estadosPorVentana) if (par.second) estados.push_back(par.second);
        g_estadosPorVentana.clear();
    }
    for (EstadoPanelV2* estado : estados) DestruirEstado(estado, false);

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    UnregisterClassW(CLASE_PANEL_V2, hInst);
}

} // namespace SideViewV2
