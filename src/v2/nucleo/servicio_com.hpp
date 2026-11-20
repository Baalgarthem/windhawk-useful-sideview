#pragma once

#include <windows.h>

namespace SideViewV2 {

class AmbitoComV2 final {
public:
    AmbitoComV2() : m_resultado(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) {}

    ~AmbitoComV2() {
        if (m_resultado == S_OK) CoUninitialize();
    }

    bool Disponible() const {
        return SUCCEEDED(m_resultado) || m_resultado == RPC_E_CHANGED_MODE;
    }

private:
    HRESULT m_resultado{E_FAIL};
};

} // namespace SideViewV2
