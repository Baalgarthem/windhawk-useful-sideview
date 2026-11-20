#pragma once

#include "v2/vistas/vista_comparador_archivos.hpp"
#include "v2/nucleo/servicio_diff.hpp"
#include <windowsx.h>
#include <algorithm>
#include <memory>

namespace SideViewV2 {

class VistaComparadorTextoV2 final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorTextoV2(std::shared_ptr<ServicioComparadorTextoV2> servicio)
        : VistaComparadorArchivosV2(servicio), m_diff(std::move(servicio)) {}

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring& carpeta) override {
        VistaComparadorArchivosV2::AlCambiarSeleccion(archivos, carpeta);
        m_estadoDiff = m_diff ? m_diff->CrearDiff(archivos) : EstadoDiffV2{};
        m_scroll = 0;
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam,
                              HWND hwndPanel) override {
        if (VistaComparadorArchivosV2::ProcesarMensajeRaton(msg, wParam, lParam, hwndPanel)) return true;
        if (msg == WM_MOUSEWHEEL) {
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -3 : 3;
            const int filas = static_cast<int>(m_estadoDiff.lineas.size());
            m_scroll = std::max(0, std::min(m_scroll + delta, std::max(0, filas - 8)));
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        return false;
    }

protected:
    const wchar_t* NombreVista() const override { return L"Comparar texto · Diff estilo GitHub"; }

    void RenderizarContenido(HDC hdc, const RECT& rect, const TemaVisual& tema,
                             HFONT fuente) override {
        DibujarContenedor(hdc, rect, tema);
        RECT cabeceraIzquierda{rect.left + 10, rect.top + 8,
                               rect.left + (rect.right - rect.left) / 2 - 6, rect.top + 32};
        RECT cabeceraDerecha{rect.left + (rect.right - rect.left) / 2 + 6, rect.top + 8,
                             rect.right - 10, rect.top + 32};
        SetTextColor(hdc, tema.textoPrincipal);
        DrawTextW(hdc, L"Archivo 1", -1, &cabeceraIzquierda,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        DrawTextW(hdc, L"Archivo 2", -1, &cabeceraDerecha,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        const int mitad = (rect.right - rect.left) / 2;
        const int altoFila = 22;
        const int inicio = rect.top + 36;
        const int visibles = std::max(0, static_cast<int>(
            (rect.bottom - inicio - 6) / altoFila));
        SelectObject(hdc, fuente);
        for (int visible = 0; visible < visibles; ++visible) {
            const std::size_t indice = static_cast<std::size_t>(m_scroll + visible);
            if (indice >= m_estadoDiff.lineas.size()) break;
            const LineaDiffV2& linea = m_estadoDiff.lineas[indice];
            const int y = inicio + visible * altoFila;
            COLORREF color = tema.fondoTarjeta;
            if (linea.tipo == TipoLineaDiffV2::Agregada) color = RGB(218, 245, 220);
            else if (linea.tipo == TipoLineaDiffV2::Eliminada) color = RGB(250, 220, 220);
            else if (linea.tipo == TipoLineaDiffV2::Modificada) color = RGB(250, 240, 204);
            HBRUSH fondoFila = CreateSolidBrush(tema.modoOscuro ? Mezclar(color, tema.fondo, 35) : color);
            RECT filaIzq{rect.left + 2, y, rect.left + mitad - 2, y + altoFila};
            RECT filaDer{rect.left + mitad + 2, y, rect.right - 2, y + altoFila};
            FillRect(hdc, &filaIzq, fondoFila);
            FillRect(hdc, &filaDer, fondoFila);
            DeleteObject(fondoFila);
            SetTextColor(hdc, tema.textoSecundario);
            wchar_t numeroIzq[24]{}, numeroDer[24]{};
            swprintf_s(numeroIzq, L"%4zu", linea.lineaIzquierda);
            swprintf_s(numeroDer, L"%4zu", linea.lineaDerecha);
            RECT ni{filaIzq.left + 4, y, filaIzq.left + 42, y + altoFila};
            RECT nd{filaDer.left + 4, y, filaDer.left + 42, y + altoFila};
            DrawTextW(hdc, numeroIzq, -1, &ni, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            DrawTextW(hdc, numeroDer, -1, &nd, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SetTextColor(hdc, linea.tipo == TipoLineaDiffV2::Agregada ? RGB(25, 130, 50) :
                               linea.tipo == TipoLineaDiffV2::Eliminada ? RGB(170, 40, 40) :
                               tema.textoPrincipal);
            RECT ti{filaIzq.left + 50, y, filaIzq.right - 6, y + altoFila};
            RECT td{filaDer.left + 50, y, filaDer.right - 6, y + altoFila};
            DrawTextW(hdc, linea.izquierda.c_str(), -1, &ti,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
            DrawTextW(hdc, linea.derecha.c_str(), -1, &td,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
        SetTextColor(hdc, tema.textoSecundario);
        RECT estado{rect.left + 10, rect.bottom - 24, rect.right - 10, rect.bottom - 6};
        DrawTextW(hdc, m_estadoDiff.mensaje.c_str(), -1, &estado,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

private:
    static COLORREF Mezclar(COLORREF frente, COLORREF fondo, int porcentaje) {
        const int p = std::clamp(porcentaje, 0, 100);
        return RGB((GetRValue(frente) * p + GetRValue(fondo) * (100 - p)) / 100,
                   (GetGValue(frente) * p + GetGValue(fondo) * (100 - p)) / 100,
                   (GetBValue(frente) * p + GetBValue(fondo) * (100 - p)) / 100);
    }

    std::shared_ptr<ServicioComparadorTextoV2> m_diff;
    EstadoDiffV2 m_estadoDiff;
    int m_scroll{0};
};

} // namespace SideViewV2
