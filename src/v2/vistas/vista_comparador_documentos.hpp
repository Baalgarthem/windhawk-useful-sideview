#pragma once

#include "v2/vistas/vista_comparador_archivos.hpp"
#include "v2/nucleo/servicio_diff.hpp"

namespace SideViewV2 {

class VistaComparadorDocumentosV2 final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorDocumentosV2(
        std::shared_ptr<ServicioComparadorDocumentosV2> servicio)
        : VistaComparadorArchivosV2(std::move(servicio)) {}

protected:
    const wchar_t* NombreVista() const override { return L"Comparar documentos"; }

    void RenderizarContenido(HDC hdc, const RECT& rect, const TemaVisual& tema,
                             HFONT) override {
        DibujarContenedor(hdc, rect, tema);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{rect.left + 16, rect.top + 20, rect.right - 16, rect.top + 48};
        DrawTextW(hdc, L"Comparación documental", -1, &titulo,
                  DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
        SetTextColor(hdc, tema.textoSecundario);
        RECT mensaje{rect.left + 24, rect.top + 64, rect.right - 24, rect.bottom - 24};
        DrawTextW(hdc,
                  L"El comparador documental hereda nombre, peso y SHA-256.\n"
                  L"La extracción visual específica del formato se conectará mediante otro adaptador.",
                  -1, &mensaje, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX);
    }
};

} // namespace SideViewV2
