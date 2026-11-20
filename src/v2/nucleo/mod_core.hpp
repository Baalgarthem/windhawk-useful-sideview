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

struct GuardiaReentradaCreateWindowV2 {
    bool& bandera;
    explicit GuardiaReentradaCreateWindowV2(bool& valor) : bandera(valor) {
        bandera = true;
    }
    ~GuardiaReentradaCreateWindowV2() { bandera = false; }
    GuardiaReentradaCreateWindowV2(const GuardiaReentradaCreateWindowV2&) = delete;
    GuardiaReentradaCreateWindowV2& operator=(const GuardiaReentradaCreateWindowV2&) = delete;
};

inline HWND WINAPI CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName,
                                        LPCWSTR lpWindowName, DWORD dwStyle,
                                        int X, int Y, int nWidth, int nHeight,
                                        HWND hWndParent, HMENU hMenu,
                                        HINSTANCE hInstance, LPVOID lpParam) {
    AmbitoCallbackV2 ambitoCallback;
    if (!CreateWindowExW_Original) return nullptr;
    HWND hwnd = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName,
                                         dwStyle, X, Y, nWidth, nHeight,
                                         hWndParent, hMenu, hInstance, lpParam);
    if (!hwnd || g_descargandoMod.load(std::memory_order_acquire)) {
        return hwnd;
    }

    // La instalación de una subclase puede provocar creación de ventanas
    // auxiliares. Evitar reentrada aquí impide encadenar el hook sobre sí
    // mismo durante la construcción de Explorer.
    if (g_dentroHookCreateWindowV2) return hwnd;
    GuardiaReentradaCreateWindowV2 guardia(g_dentroHookCreateWindowV2);

    if (lpClassName && !IS_INTRESOURCE(lpClassName) &&
        (_wcsicmp(lpClassName, L"CabinetWClass") == 0 ||
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
    } else {
        ConfigurarAtajoDesdeString(L"Ctrl+K");
    }
    if (atajoStr) {
        Wh_FreeStringSetting(atajoStr);
    }

    int limite = Wh_GetIntSetting(L"limiteCarpetasPesadas");
    g_limiteCarpetasPesadasConfig.store(std::max(1, std::min(limite, 100)));
    g_mostrarNumerosLineaV2.store(Wh_GetIntSetting(L"mostrarNumerosLinea") != 0);
    g_tamanoFuenteTextoV2.store(
        NormalizarTamanoFuenteV2(Wh_GetIntSetting(L"tamanoFuenteTexto")),
        std::memory_order_release);
    g_tamanoFuenteCodigoV2.store(
        NormalizarTamanoFuenteV2(Wh_GetIntSetting(L"tamanoFuenteCodigo")),
        std::memory_order_release);
    g_ajusteLineaTextoCodigoV2.store(
        Wh_GetIntSetting(L"ajusteLineaTextoCodigo") != 0,
        std::memory_order_release);
    g_reproductorVideoEnVistaV2.store(
        Wh_GetIntSetting(L"reproductorVideoEnVista") != 0,
        std::memory_order_release);
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
    g_callbacksActivosV2.store(0, std::memory_order_release);
    WM_SIDEVIEW_TOGGLE = RegisterWindowMessageW(L"UsefulSideView_WM_Toggle_V2");
    WM_SIDEVIEW_SETTINGS = RegisterWindowMessageW(L"UsefulSideView_WM_Settings_V2");

    if (!WM_SIDEVIEW_TOGGLE || !WM_SIDEVIEW_SETTINGS) {
        return false;
    }

    CargarConfiguracionesV2();

    HINSTANCE hInst = ObtenerInstanciaModV2();
    if (!hInst) return false;
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
    if (g_descargandoMod.load(std::memory_order_acquire)) return;
    // Subclasear las ventanas existentes de Explorer
    auto ventanas = ObtenerVentanasExplorerActuales();
    for (HWND w : ventanas) {
        WindhawkUtils::SetWindowSubclassFromAnyThread(w, ProcedimientoSubclaseTop, 0);
    }

    // Iniciar el hook de teclado en su propio hilo
    IniciarHookTeclado();
}

inline void AlCambiarConfiguracionesV2() {
    if (g_descargandoMod.load(std::memory_order_acquire)) return;
    CargarConfiguracionesV2();
    std::vector<HWND> paneles;
    {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        paneles.reserve(g_estadosPorVentana.size());
        for (auto& par : g_estadosPorVentana) {
            if (par.second && par.second->panel && IsWindow(par.second->panel)) {
                paneles.push_back(par.second->panel);
            }
        }
    }
    for (HWND panel : paneles) {
        if (!g_descargandoMod.load(std::memory_order_acquire) && IsWindow(panel)) {
            PostMessageW(panel, WM_SIDEVIEW_SETTINGS, 0, 0);
        }
    }
}

inline void PrepararDesinicializacionModV2() {
    g_descargandoMod.store(true, std::memory_order_release);
    DetenerHookTeclado();
}

inline void DesinicializarModV2() {
    PrepararDesinicializacionModV2();

    // Las ventanas de diálogo son modales y tienen su propio ciclo de
    // mensajes. Cerrarlas antes de descargar la DLL evita que su WNDPROC quede
    // apuntando a memoria liberada.
    CerrarVentanasClaseV2(L"UsefulSideView_PromptNombreCarpeta_V2");
    CerrarVentanasClaseV2(L"UsefulSideView_PromptNota_V2");

    // No se borra el mapa antes de enviar la orden: cada panel debe destruirse
    // en el hilo que lo creó. Liberarlo directamente desde el hilo de Windhawk
    // deja ventanas huérfanas cuando DestroyWindow cruza hilos.
    std::vector<EstadoPanelV2*> estados;
    {
        std::lock_guard<std::mutex> lock(g_bloqueoEstados);
        estados.reserve(g_estadosPorVentana.size());
        for (auto& par : g_estadosPorVentana) {
            if (par.second) estados.push_back(par.second);
        }
    }
    for (EstadoPanelV2* estado : estados) {
        if (!estado) continue;
        HWND panel = estado->panel;
        if (panel && IsWindow(panel)) {
            SendMessageW(panel, WM_SIDEVIEW_UNLOAD_V2, 0, 0);
        } else if (estado->parent && IsWindow(estado->parent)) {
            SendMessageW(estado->parent, WM_SIDEVIEW_UNLOAD_V2, 0,
                         reinterpret_cast<LPARAM>(estado));
        } else if (estado->top && IsWindow(estado->top)) {
            SendMessageW(estado->top, WM_SIDEVIEW_UNLOAD_V2, 0,
                         reinterpret_cast<LPARAM>(estado));
        } else {
            // Todas las HWND asociadas ya desaparecieron; sólo en este caso
            // es seguro completar la liberación localmente.
            DestruirEstado(estado, true);
        }
    }

    // La subclase superior se instala en todas las ventanas de Explorer y no
    // pertenece a un EstadoPanelV2 concreto. Se retira después de dar una
    // oportunidad al top-level para limpiar cualquier estado cuyo panel y
    // contenedor ya hayan desaparecido.
    for (HWND ventana : ObtenerVentanasExplorerActuales()) {
        if (IsWindow(ventana)) {
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(
                ventana, ProcedimientoSubclaseTop);
        }
    }

    DrenarCallbacksV2();

    HINSTANCE hInst = ObtenerInstanciaModV2();
    if (hInst) UnregisterClassW(CLASE_PANEL_V2, hInst);
    DesregistrarClasePromptNombreCarpetaV2();
    ServicioNotasV2::DesregistrarClasePromptNotaV2();
    ServicioReproductorVideoV2::DesregistrarClaseV2();
}

} // namespace SideViewV2
