#pragma once

#include "v2/vistas/vista_comparador_archivos.hpp"
#include "v2/nucleo/configuracion_v2.hpp"
#include "v2/nucleo/servicio_diff.hpp"
#include <windowsx.h>
#include <algorithm>
#include <memory>

namespace SideViewV2 {

class VistaComparadorDocumentosV2 final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorDocumentosV2(
        std::shared_ptr<ServicioComparadorDocumentosV2> servicio)
        : VistaComparadorArchivosV2(servicio), m_diff(std::move(servicio)) {}

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring& carpeta) override {
        VistaComparadorArchivosV2::AlCambiarSeleccion(archivos, carpeta);
        m_estadoDiff = m_diff ? m_diff->CrearDiff(archivos) : EstadoDiffV2{};
        m_scrollIzquierda = 0;
        m_scrollDerecha = 0;
        m_scrollGeneral = 0;
    }

protected:
    const wchar_t* NombreVista() const override { return L"Comparar documentos"; }
    bool TieneDiffView() const override { return true; }

    void AlCambiarModoDiff(bool) override {
        m_scrollIzquierda = 0;
        m_scrollDerecha = 0;
        m_scrollGeneral = 0;
        m_sliderActivo = 0;
    }

    bool ProcesarMensajeContenido(UINT msg, WPARAM wParam, LPARAM lParam,
                                  HWND hwndPanel) override {
        if (msg == WM_MOUSEWHEEL) {
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -3 : 3;
            if (EstaEnDiffView()) {
                m_scrollGeneral = std::clamp(m_scrollGeneral + delta, 0, MaximoGeneral());
            } else {
                m_scrollIzquierda = std::clamp(m_scrollIzquierda + delta, 0, MaximoIzquierda());
                m_scrollDerecha = std::clamp(m_scrollDerecha + delta, 0, MaximoDerecha());
            }
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (msg == WM_KEYDOWN && (wParam == VK_UP || wParam == VK_DOWN)) {
            const int delta = wParam == VK_UP ? -1 : 1;
            if (EstaEnDiffView()) m_scrollGeneral = std::clamp(m_scrollGeneral + delta, 0, MaximoGeneral());
            else {
                m_scrollIzquierda = std::clamp(m_scrollIzquierda + delta, 0, MaximoIzquierda());
                m_scrollDerecha = std::clamp(m_scrollDerecha + delta, 0, MaximoDerecha());
            }
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (msg == WM_LBUTTONDOWN) {
            const POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (EstaEnDiffView() && PtInRect(&m_rcSliderGeneral, punto)) m_sliderActivo = 3;
            else if (!EstaEnDiffView() && PtInRect(&m_rcSliderIzquierda, punto)) m_sliderActivo = 1;
            else if (!EstaEnDiffView() && PtInRect(&m_rcSliderDerecha, punto)) m_sliderActivo = 2;
            else if (!EstaEnDiffView() && PtInRect(&m_rcSliderGeneral, punto)) m_sliderActivo = 3;
            else return false;
            ActualizarSlider({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, hwndPanel);
            SetCapture(hwndPanel);
            return true;
        }
        if (msg == WM_MOUSEMOVE && m_sliderActivo != 0) {
            ActualizarSlider({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, hwndPanel);
            return true;
        }
        if (msg == WM_LBUTTONUP && m_sliderActivo != 0) {
            m_sliderActivo = 0;
            ReleaseCapture();
            return true;
        }
        return false;
    }

    void RenderizarContenido(HDC hdc, const RECT& rect, const TemaVisual& tema,
                             HFONT fuente) override {
        DibujarContenedor(hdc, rect, tema);
        const int mitad = (rect.right - rect.left) / 2;
        const int altoFila = 22;
        const int inicio = rect.top + 34;
        const int pie = rect.bottom - 58;
        const int visibles = std::max(0, (pie - inicio) / altoFila);
        RECT cabeceraIzquierda{rect.left + 8, rect.top + 8, rect.left + mitad - 4, rect.top + 28};
        RECT cabeceraDerecha{rect.left + mitad + 4, rect.top + 8, rect.right - 8, rect.top + 28};
        SetTextColor(hdc, tema.textoPrincipal);
        DrawTextW(hdc, L"Documento 1", -1, &cabeceraIzquierda,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        DrawTextW(hdc, L"Documento 2", -1, &cabeceraDerecha,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(hdc, fuente);
        for (int visible = 0; visible < visibles; ++visible) {
            const int y = inicio + visible * altoFila;
            const std::size_t izquierda = static_cast<std::size_t>(m_scrollIzquierda + visible);
            const std::size_t derecha = static_cast<std::size_t>(m_scrollDerecha + visible);
            DibujarLinea(hdc, rect, mitad, y, altoFila,
                         izquierda < m_estadoDiff.lineas.size() ? &m_estadoDiff.lineas[izquierda] : nullptr,
                         true, tema);
            DibujarLinea(hdc, rect, mitad, y, altoFila,
                         derecha < m_estadoDiff.lineas.size() ? &m_estadoDiff.lineas[derecha] : nullptr,
                         false, tema);
        }
        DibujarSlidersNormales(hdc, rect, mitad, tema);
    }

    void RenderizarDiffView(HDC hdc, const RECT& rect, const TemaVisual& tema,
                            HFONT fuente) override {
        DibujarContenedor(hdc, rect, tema);
        const int altoFila = 24;
        const int inicio = rect.top + 34;
        const int visibles = std::max(0, (static_cast<int>(rect.bottom) - 30 - inicio) / altoFila);
        const bool semantico = ModoSemanticoDiffViewDisponibleV2();
        RECT encabezado{rect.left + 12, rect.top + 8, rect.right - 12, rect.top + 30};
        SetTextColor(hdc, tema.acento);
        DrawTextW(hdc, semantico ? L"Diff semántico · explicación del documento" : L"Diff clásico del documento",
                  -1, &encabezado, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(hdc, fuente);
        for (int visible = 0; visible < visibles; ++visible) {
            const std::size_t indice = static_cast<std::size_t>(m_scrollGeneral + visible);
            if (indice >= m_estadoDiff.lineas.size()) break;
            const LineaDiffV2& linea = m_estadoDiff.lineas[indice];
            const int y = inicio + visible * altoFila;
            COLORREF color = tema.fondoTarjeta;
            if (linea.tipo == TipoLineaDiffV2::Agregada) color = RGB(218, 245, 220);
            else if (linea.tipo == TipoLineaDiffV2::Eliminada) color = RGB(250, 220, 220);
            else if (linea.tipo == TipoLineaDiffV2::Modificada) color = RGB(250, 240, 204);
            HBRUSH brocha = CreateSolidBrush(tema.modoOscuro ? Mezclar(color, tema.fondo, 35) : color);
            RECT fila{rect.left + 4, y, rect.right - 4, y + altoFila};
            FillRect(hdc, &fila, brocha);
            DeleteObject(brocha);
            const std::wstring texto = semantico ? TextoSemantico(linea) : TextoClasico(linea);
            SetTextColor(hdc, linea.tipo == TipoLineaDiffV2::Igual ? tema.textoPrincipal : tema.acento);
            RECT contenido{fila.left + 8, y, fila.right - 8, y + altoFila};
            DrawTextW(hdc, texto.c_str(), -1, &contenido,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
        m_rcSliderGeneral = {rect.left + 12, rect.bottom - 24, rect.right - 12, rect.bottom - 8};
        m_rcSliderIzquierda = {};
        m_rcSliderDerecha = {};
        DibujarControlDesplazamiento(hdc, m_rcSliderGeneral,
                                     static_cast<std::size_t>(MaximoGeneral()),
                                     static_cast<std::size_t>(m_scrollGeneral), tema);
    }

private:
    static COLORREF Mezclar(COLORREF frente, COLORREF fondo, int porcentaje) {
        const int p = std::clamp(porcentaje, 0, 100);
        return RGB((GetRValue(frente) * p + GetRValue(fondo) * (100 - p)) / 100,
                   (GetGValue(frente) * p + GetGValue(fondo) * (100 - p)) / 100,
                   (GetBValue(frente) * p + GetBValue(fondo) * (100 - p)) / 100);
    }

    static std::wstring TextoClasico(const LineaDiffV2& linea) {
        return L"[" + std::to_wstring(linea.lineaIzquierda) + L" | " +
               std::to_wstring(linea.lineaDerecha) + L"] " + linea.izquierda +
               (linea.tipo == TipoLineaDiffV2::Modificada ? L"  →  " : L"") + linea.derecha;
    }

    static std::wstring TextoSemantico(const LineaDiffV2& linea) {
        switch (linea.tipo) {
            case TipoLineaDiffV2::Agregada: return L"Añadido en el documento 2: " + linea.derecha;
            case TipoLineaDiffV2::Eliminada: return L"Eliminado del documento 1: " + linea.izquierda;
            case TipoLineaDiffV2::Modificada: return L"Cambio: " + linea.izquierda + L"  →  " + linea.derecha;
            case TipoLineaDiffV2::Igual: return L"Sin cambios: " + linea.izquierda;
        }
        return {};
    }

    void DibujarLinea(HDC hdc, const RECT& rect, int mitad, int y, int alto,
                      const LineaDiffV2* linea, bool izquierda,
                      const TemaVisual& tema) const {
        if (!linea) return;
        const int inicio = izquierda ? rect.left + 2 : rect.left + mitad + 2;
        const int fin = izquierda ? rect.left + mitad - 2 : rect.right - 2;
        RECT fila{inicio, y, fin, y + alto};
        COLORREF color = tema.fondoTarjeta;
        if (linea->tipo == TipoLineaDiffV2::Agregada) color = RGB(218, 245, 220);
        else if (linea->tipo == TipoLineaDiffV2::Eliminada) color = RGB(250, 220, 220);
        else if (linea->tipo == TipoLineaDiffV2::Modificada) color = RGB(250, 240, 204);
        HBRUSH brocha = CreateSolidBrush(tema.modoOscuro ? Mezclar(color, tema.fondo, 35) : color);
        FillRect(hdc, &fila, brocha);
        DeleteObject(brocha);
        const std::wstring& texto = izquierda ? linea->izquierda : linea->derecha;
        const std::size_t numero = izquierda ? linea->lineaIzquierda : linea->lineaDerecha;
        wchar_t numeroTexto[24]{};
        swprintf_s(numeroTexto, L"%4zu", numero);
        SetTextColor(hdc, tema.textoSecundario);
        RECT numeroRect{fila.left + 4, y, fila.left + 42, y + alto};
        DrawTextW(hdc, numeroTexto, -1, &numeroRect,
                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT contenido{fila.left + 50, y, fila.right - 6, y + alto};
        DrawTextW(hdc, texto.c_str(), -1, &contenido,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    void DibujarSlidersNormales(HDC hdc, const RECT& rect, int mitad,
                                const TemaVisual& tema) {
        m_rcSliderIzquierda = {rect.left + 10, rect.bottom - 38,
                               rect.left + mitad - 8, rect.bottom - 24};
        m_rcSliderDerecha = {rect.left + mitad + 8, rect.bottom - 38,
                             rect.right - 10, rect.bottom - 24};
        m_rcSliderGeneral = {rect.left + 10, rect.bottom - 20,
                             rect.right - 10, rect.bottom - 6};
        DibujarControlDesplazamiento(hdc, m_rcSliderIzquierda,
                                     static_cast<std::size_t>(MaximoIzquierda()),
                                     static_cast<std::size_t>(m_scrollIzquierda), tema);
        DibujarControlDesplazamiento(hdc, m_rcSliderDerecha,
                                     static_cast<std::size_t>(MaximoDerecha()),
                                     static_cast<std::size_t>(m_scrollDerecha), tema);
        DibujarControlDesplazamiento(hdc, m_rcSliderGeneral,
                                     static_cast<std::size_t>(std::min(MaximoIzquierda(), MaximoDerecha())),
                                     static_cast<std::size_t>(std::min(m_scrollIzquierda, m_scrollDerecha)), tema);
        SetTextColor(hdc, tema.textoSecundario);
        RECT etiqueta{rect.left + 10, rect.bottom - 54, rect.right - 10, rect.bottom - 40};
        DrawTextW(hdc, L"Arriba: desplazamiento independiente · Abajo: sincronizado",
                  -1, &etiqueta, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    int MaximoGeneral() const { return MaximoContenido(); }
    int MaximoIzquierda() const { return MaximoContenido(); }
    int MaximoDerecha() const { return MaximoContenido(); }

    int MaximoContenido() const {
        const int alto = static_cast<int>(AreaEspecifica().bottom - AreaEspecifica().top);
        const int visibles = std::max(1, (alto - 64) / 24);
        return std::max(0, static_cast<int>(m_estadoDiff.lineas.size()) - visibles);
    }

    void ActualizarSlider(POINT punto, HWND hwndPanel) {
        if (m_sliderActivo == 3) {
            const int limite = std::min(MaximoIzquierda(), MaximoDerecha());
            const int valor = static_cast<int>(DesplazamientoDesdePunto(
                m_rcSliderGeneral, punto, static_cast<std::size_t>(EstaEnDiffView() ? MaximoGeneral() : limite)));
            m_scrollGeneral = std::min(valor, MaximoGeneral());
            if (!EstaEnDiffView()) {
                m_scrollIzquierda = std::min(valor, MaximoIzquierda());
                m_scrollDerecha = std::min(valor, MaximoDerecha());
            }
        } else if (m_sliderActivo == 1) {
            m_scrollIzquierda = static_cast<int>(DesplazamientoDesdePunto(
                m_rcSliderIzquierda, punto, static_cast<std::size_t>(MaximoIzquierda())));
        } else if (m_sliderActivo == 2) {
            m_scrollDerecha = static_cast<int>(DesplazamientoDesdePunto(
                m_rcSliderDerecha, punto, static_cast<std::size_t>(MaximoDerecha())));
        }
        InvalidateRect(hwndPanel, nullptr, FALSE);
    }

    std::shared_ptr<ServicioComparadorDocumentosV2> m_diff;
    EstadoDiffV2 m_estadoDiff;
    int m_scrollIzquierda{0};
    int m_scrollDerecha{0};
    int m_scrollGeneral{0};
    int m_sliderActivo{0};
    RECT m_rcSliderIzquierda{};
    RECT m_rcSliderDerecha{};
    RECT m_rcSliderGeneral{};
};

} // namespace SideViewV2
