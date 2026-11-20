#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>
namespace SideViewV2 {
class ServicioEnlacesV2 {
public:
    bool Abrir(const std::wstring& destino) const {
        if (destino.empty()) return false;
        return reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr, L"open", destino.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) > 32;
    }
};
}
