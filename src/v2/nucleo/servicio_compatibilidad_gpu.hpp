#pragma once

#include <windows.h>

namespace SideViewV2 {

// Contrato de interoperabilidad observado en
// refs/gpu-accelerated-explorer-view.wh.cpp. No se enlaza contra ese mod ni
// se invoca su código: sólo se consume su propiedad HWND documentada en el
// propio bundle para localizar la superficie visual que reemplaza a
// SHELLDLL_DefView.
struct SuperficieExploradorV2 {
    HWND vistaShell{nullptr};
    HWND superficieVisual{nullptr};
    HWND contenedor{nullptr};
    RECT limites{};
    bool gpuAcelerada{false};

    bool EsValida() const noexcept {
        return vistaShell && superficieVisual && contenedor &&
               limites.right > limites.left && limites.bottom > limites.top;
    }
};

class ServicioCompatibilidadGpuV2 final {
public:
    // El mod GPU coloca este valor en la vista Shell y lo apunta a su overlay.
    // El nombre se mantiene literal para permitir cooperación entre mods sin
    // una dependencia binaria o un include cruzado.
    static constexpr wchar_t PROPIEDAD_OVERLAY_GPU[] =
        L"WindhawkExplorerGpuOverlayOwnsShellView";

    SuperficieExploradorV2 Resolver(HWND frame, HWND tab, HWND vistaShell) const noexcept {
        SuperficieExploradorV2 resultado{};
        if (!IsWindow(frame) || !IsWindow(vistaShell)) return resultado;

        resultado.vistaShell = vistaShell;
        const HWND overlay = reinterpret_cast<HWND>(
            GetPropW(vistaShell, PROPIEDAD_OVERLAY_GPU));
        if (EsOverlayGpuActivo(frame, overlay)) {
            RECT limitesOverlay{};
            if (CapturarLimites(overlay, frame, limitesOverlay)) {
                resultado.superficieVisual = overlay;
                resultado.contenedor = frame;
                resultado.limites = limitesOverlay;
                resultado.gpuAcelerada = true;
                return resultado;
            }
        }

        HWND contenedor = GetParent(vistaShell);
        if (!contenedor) contenedor = tab ? tab : frame;
        if (!IsWindow(contenedor)) return {};

        resultado.contenedor = contenedor;
        resultado.superficieVisual = vistaShell;
        if (!CapturarLimites(vistaShell, contenedor, resultado.limites)) return {};
        return resultado;
    }

    bool ActualizarLimites(SuperficieExploradorV2& superficie) const noexcept {
        if (!superficie.EsValida() || !IsWindow(superficie.superficieVisual) ||
            !IsWindow(superficie.contenedor)) return false;
        RECT limites{};
        if (!CapturarLimites(superficie.superficieVisual, superficie.contenedor, limites))
            return false;
        superficie.limites = limites;
        return true;
    }

private:
    static bool CapturarLimites(HWND ventana, HWND contenedor, RECT& destino) noexcept {
        if (!IsWindow(ventana) || !IsWindow(contenedor) ||
            !GetWindowRect(ventana, &destino)) return false;
        MapWindowPoints(HWND_DESKTOP, contenedor,
                        reinterpret_cast<POINT*>(&destino), 2);
        return destino.right > destino.left && destino.bottom > destino.top;
    }

    static bool EsOverlayGpuActivo(HWND frame, HWND overlay) noexcept {
        return IsWindow(overlay) && IsWindowVisible(overlay) &&
               GetParent(overlay) == frame &&
               GetAncestor(overlay, GA_ROOT) == frame;
    }
};

} // namespace SideViewV2
