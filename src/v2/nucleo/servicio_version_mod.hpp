#pragma once

#include <string>

namespace SideViewV2 {

// Fuente de verdad legible por la UI. El compilador comprueba que coincida con
// el campo @version antes de crear el bundle distribuible.
inline constexpr wchar_t VERSION_MOD_V2[] = L"4.1.5";

class ServicioVersionModV2 final {
public:
    std::wstring TextoVersion() const {
        return L"Useful Side View v" + std::wstring(VERSION_MOD_V2);
    }
};

} // namespace SideViewV2
