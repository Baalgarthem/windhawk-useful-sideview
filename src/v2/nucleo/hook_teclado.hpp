#pragma once

#include <windows.h>
#include <vector>
#include <atomic>
#include <string>
#include "v2/nucleo/gestor_ventanas.hpp"

namespace SideViewV2 {

inline HANDLE g_hiloTeclado = nullptr;
inline DWORD g_idHiloTeclado = 0;
inline HHOOK g_hookTeclado = nullptr;
inline std::atomic_bool g_descargandoMod{false};

// Configuración de atajo dinámica
inline std::atomic<DWORD> g_teclaAtajo{'K'};
inline std::atomic_bool g_requiereCtrl{true};
inline std::atomic_bool g_requiereAlt{false};
inline std::atomic_bool g_requiereShift{false};

inline void ConfigurarAtajoDesdeString(const wchar_t* strAtajo) {
    if (!strAtajo || *strAtajo == L'\0') {
        g_teclaAtajo.store('K');
        g_requiereCtrl.store(true);
        g_requiereAlt.store(false);
        g_requiereShift.store(false);
        return;
    }

    std::wstring s = strAtajo;
    std::transform(s.begin(), s.end(), s.begin(), ::towupper);

    bool c = (s.find(L"CTRL") != std::wstring::npos);
    bool a = (s.find(L"ALT") != std::wstring::npos);
    bool sh = (s.find(L"SHIFT") != std::wstring::npos);

    // Buscar la letra o tecla principal
    DWORD vk = 'K';
    for (wchar_t ch : s) {
        if (ch >= L'A' && ch <= L'Z') {
            // Verificar que no sea parte de CTRL, ALT o SHIFT
            if (s.find(L"CTRL") != std::wstring::npos && (ch == 'C' || ch == 'T' || ch == 'R' || ch == 'L')) continue;
            if (s.find(L"ALT") != std::wstring::npos && (ch == 'A' || ch == 'L' || ch == 'T')) continue;
            if (s.find(L"SHIFT") != std::wstring::npos && (ch == 'S' || ch == 'H' || ch == 'I' || ch == 'F' || ch == 'T')) continue;
            vk = static_cast<DWORD>(ch);
            break;
        } else if (ch >= L'0' && ch <= L'9') {
            vk = static_cast<DWORD>(ch);
            break;
        }
    }

    if (s.find(L"SPACE") != std::wstring::npos) vk = VK_SPACE;
    if (s.find(L"F1") != std::wstring::npos) vk = VK_F1;
    if (s.find(L"F2") != std::wstring::npos) vk = VK_F2;
    if (s.find(L"F3") != std::wstring::npos) vk = VK_F3;
    if (s.find(L"F4") != std::wstring::npos) vk = VK_F4;

    g_teclaAtajo.store(vk);
    g_requiereCtrl.store(c);
    g_requiereAlt.store(a);
    g_requiereShift.store(sh);
}

inline bool EsClaseExplorer(HWND hwnd) {
    if (!hwnd) return false;
    wchar_t nombreClase[64]{};
    GetClassNameW(hwnd, nombreClase, ARRAYSIZE(nombreClase));
    return (_wcsicmp(nombreClase, L"CabinetWClass") == 0 ||
            _wcsicmp(nombreClase, L"ExploreWClass") == 0);
}

inline bool EsFocoEnVistaArchivos(HWND top) {
    if (!top) return false;
    DWORD idHilo = GetWindowThreadProcessId(top, nullptr);
    GUITHREADINFO info{};
    info.cbSize = sizeof(info);
    if (!GetGUIThreadInfo(idHilo, &info)) return false;

    HWND foco = info.hwndFocus ? info.hwndFocus : top;
    wchar_t nombreClase[64]{};
    GetClassNameW(foco, nombreClase, ARRAYSIZE(nombreClase));

    // Si el usuario está escribiendo texto en un cuadro de edición o árbol, no interceptamos
    if (_wcsicmp(nombreClase, L"Edit") == 0 ||
        _wcsicmp(nombreClase, L"RichEdit") == 0 ||
        _wcsicmp(nombreClase, L"RICHEDIT50W") == 0 ||
        _wcsicmp(nombreClase, L"ComboBox") == 0 ||
        _wcsicmp(nombreClase, L"ComboBoxEx32") == 0 ||
        _wcsicmp(nombreClase, L"SysTreeView32") == 0) {
        return false;
    }

    if (_wcsicmp(nombreClase, L"DirectUIHWND") == 0 ||
        _wcsicmp(nombreClase, L"DUIViewWndClassName") == 0 ||
        _wcsicmp(nombreClase, L"SysListView32") == 0 ||
        _wcsicmp(nombreClase, L"SHELLDLL_DefView") == 0 ||
        _wcsicmp(nombreClase, L"ShellTabWindowClass") == 0 ||
        _wcsicmp(nombreClase, L"CtrlNotifySink") == 0 ||
        _wcsicmp(nombreClase, CLASE_PANEL_V2) == 0 ||
        foco == top) {
        return true;
    }

    return false;
}

inline LRESULT CALLBACK ProcedimientoHookTeclado(int code, WPARAM wParam, LPARAM lParam) {
    if (code < 0 || g_descargandoMod.load()) {
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }

    auto* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        DWORD targetKey = g_teclaAtajo.load();
        if (kbd->vkCode == targetKey) {
            bool ctrlPresionado = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool altPresionado = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
            bool shiftPresionado = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

            if (ctrlPresionado == g_requiereCtrl.load() &&
                altPresionado == g_requiereAlt.load() &&
                shiftPresionado == g_requiereShift.load()) {

                HWND ventanaFrente = GetForegroundWindow();
                if (EsClaseExplorer(ventanaFrente)) {
                    DWORD idProcesoFrente = 0;
                    GetWindowThreadProcessId(ventanaFrente, &idProcesoFrente);
                    if (idProcesoFrente == GetCurrentProcessId()) {
                        if (EsFocoEnVistaArchivos(ventanaFrente)) {
                            PostMessageW(ventanaFrente, WM_SIDEVIEW_TOGGLE, 0, 0);
                            return 1; // Consumir atajo
                        }
                    }
                }
            }
        }
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
}

inline DWORD WINAPI HiloHookProc(LPVOID) {
    HINSTANCE hInst = GetModuleHandleW(nullptr);
    g_hookTeclado = SetWindowsHookExW(WH_KEYBOARD_LL, ProcedimientoHookTeclado, hInst, 0);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_hookTeclado) {
        UnhookWindowsHookEx(g_hookTeclado);
        g_hookTeclado = nullptr;
    }
    return 0;
}

inline bool IniciarHookTeclado() {
    DWORD idHilo = 0;
    g_hiloTeclado = CreateThread(nullptr, 0, HiloHookProc, nullptr, 0, &idHilo);
    if (!g_hiloTeclado) return false;
    g_idHiloTeclado = idHilo;
    return true;
}

inline void DetenerHookTeclado() {
    if (!g_hiloTeclado) return;
    while (WaitForSingleObject(g_hiloTeclado, 50) == WAIT_TIMEOUT) {
        PostThreadMessageW(g_idHiloTeclado, WM_QUIT, 0, 0);
    }
    CloseHandle(g_hiloTeclado);
    g_hiloTeclado = nullptr;
    g_idHiloTeclado = 0;
}

} // namespace SideViewV2
