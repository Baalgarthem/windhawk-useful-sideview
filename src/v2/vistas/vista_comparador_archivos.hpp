#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/nucleo/servicio_comparador_archivos.hpp"
#include <windowsx.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

// Chrome común del comparador. La vista dibuja y enruta eventos; las reglas
// de comparación y navegación permanecen en los servicios inyectados.
class VistaComparadorArchivosV2 : public IPanelContenido {
public:
    explicit VistaComparadorArchivosV2(
        std::shared_ptr<ServicioComparadorArchivosV2> servicio)
        : m_servicio(std::move(servicio)) {}

    void Liberar() override {
        if (m_panel && GetCapture() == m_panel) ReleaseCapture();
        m_estado = {};
        m_modoDiffView = false;
        m_panel = nullptr;
        m_rcVolver = {};
        m_rcBotonDiffView = {};
        m_rcAreaEspecifica = {};
        m_rcResumen = {};
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        m_modoDiffView = false;
        CargarComparacionComun(archivos);
    }

    void AlActivar(HWND panel) override { m_panel = panel; }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam,
                              HWND hwndPanel) override {
        if (msg == WM_LBUTTONDOWN) {
            POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (PtInRect(&m_rcVolver, punto)) {
                PostMessageW(hwndPanel, WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS, 0, 0);
                return true;
            }
            if (TieneDiffView() && PtInRect(&m_rcBotonDiffView, punto)) {
                m_modoDiffView = !m_modoDiffView;
                AlCambiarModoDiff(m_modoDiffView);
                InvalidateRect(hwndPanel, nullptr, FALSE);
                return true;
            }
        }
        if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
            PostMessageW(hwndPanel, WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS, 0, 0);
            return true;
        }
        return ProcesarMensajeContenido(msg, wParam, lParam, hwndPanel);
    }

    bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM lParam,
                                HWND hwndPanel) override {
        return ProcesarMensajeRaton(msg, wParam, lParam, hwndPanel);
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        if (!hdc || rect.right <= rect.left || rect.bottom <= rect.top) return;
        HBRUSH fondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetBkMode(hdc, TRANSPARENT);

        HFONT titulo = CrearFuente(-17, FW_SEMIBOLD);
        HFONT texto = CrearFuente(-12, FW_NORMAL);
        HFONT anterior = static_cast<HFONT>(SelectObject(hdc, titulo));
        const int margen = 16;
        m_rcVolver = {rect.left + margen, rect.top + 12,
                      rect.left + 124, rect.top + 40};
        DibujarBoton(hdc, m_rcVolver, L"←  Volver", tema);

        SetTextColor(hdc, tema.textoPrincipal);
        RECT encabezado{rect.left + 142, rect.top + 12, rect.right - margen, rect.top + 40};
        DrawTextW(hdc, NombreVista(), -1, &encabezado,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        m_rcBotonDiffView = {};
        if (TieneDiffView()) {
            m_rcBotonDiffView = {rect.right - 178, rect.top + 12,
                                 rect.right - margen, rect.top + 40};
            DibujarBoton(hdc, m_rcBotonDiffView,
                         m_modoDiffView ? L"Vista lado a lado" : L"Diff View", tema);
        }

        const int altoResumen = 154;
        m_rcAreaEspecifica = {rect.left + margen, rect.top + 54,
                              rect.right - margen,
                              std::max(rect.top + 56, rect.bottom - altoResumen - 24)};
        m_rcResumen = {rect.left + margen, m_rcAreaEspecifica.bottom + 12,
                       rect.right - margen, rect.bottom - margen};

        SelectObject(hdc, texto);
        if (m_modoDiffView && TieneDiffView()) {
            RenderizarDiffView(hdc, m_rcAreaEspecifica, tema, texto);
        } else {
            RenderizarContenido(hdc, m_rcAreaEspecifica, tema, texto);
        }
        DibujarResumenTresColumnas(hdc, m_rcResumen, tema, texto);

        SelectObject(hdc, anterior);
        DeleteObject(titulo);
        DeleteObject(texto);
    }

protected:
    virtual const wchar_t* NombreVista() const { return L"Comparar archivos"; }
    virtual bool TieneDiffView() const { return false; }
    virtual void AlCambiarModoDiff(bool) {}
    virtual bool ProcesarMensajeContenido(UINT, WPARAM, LPARAM, HWND) { return false; }

    virtual void RenderizarContenido(HDC hdc, const RECT& rect,
                                     const TemaVisual& tema, HFONT fuente) {
        DibujarContenedor(hdc, rect, tema);
        SelectObject(hdc, fuente);
        SetTextColor(hdc, tema.textoSecundario);
        RECT mensaje = rect;
        DrawTextW(hdc, m_estado.mensaje.c_str(), -1, &mensaje,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    virtual void RenderizarDiffView(HDC hdc, const RECT& rect,
                                    const TemaVisual& tema, HFONT fuente) {
        RenderizarContenido(hdc, rect, tema, fuente);
    }

    void CargarComparacionComun(const std::vector<std::wstring>& archivos) {
        m_estado = m_servicio ? m_servicio->CrearComparacion(archivos)
                               : EstadoComparadorArchivosV2{};
    }

    const RECT& AreaEspecifica() const { return m_rcAreaEspecifica; }
    bool EstaEnDiffView() const { return m_modoDiffView; }

    void DibujarContenedor(HDC hdc, const RECT& rect, const TemaVisual& tema) const {
        if (rect.right <= rect.left || rect.bottom <= rect.top) return;
        HBRUSH fondo = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);
    }

    void DibujarBoton(HDC hdc, const RECT& rect, const wchar_t* texto,
                      const TemaVisual& tema) const {
        HBRUSH fondo = CreateSolidBrush(tema.botonFondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 6, 6);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT textoRect = rect;
        DrawTextW(hdc, texto, -1, &textoRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    static HFONT CrearFuente(int altura, int peso) {
        return CreateFontW(altura, 0, 0, 0, peso, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                           L"Segoe UI Variable Text");
    }

    static std::wstring TextoConteo(std::uintmax_t valor) {
        return valor == 0 ? L"No aplica" : std::to_wstring(valor);
    }

    void DibujarResumenTresColumnas(HDC hdc, const RECT& rect,
                                    const TemaVisual& tema, HFONT fuente) const {
        DibujarContenedor(hdc, rect, tema);
        SelectObject(hdc, fuente);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{rect.left + 12, rect.top + 8, rect.right - 12, rect.top + 28};
        DrawTextW(hdc, L"RESUMEN DE LA COMPARACIÓN", -1, &titulo,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        const int anchoColumna = (rect.right - rect.left - 24) / 3;
        const int yEncabezado = rect.top + 30;
        const int yDatos = rect.top + 50;
        const int altoFila = 18;
        const RECT columnas[3] = {
            {rect.left + 12, rect.left + 12 + anchoColumna, 0, 0},
            {rect.left + 12 + anchoColumna, rect.left + 12 + anchoColumna * 2, 0, 0},
            {rect.left + 12 + anchoColumna * 2, rect.right - 12, 0, 0}};
        const wchar_t* encabezados[] = {L"ARCHIVO 1", L"ARCHIVO 2", L"DIFERENCIAS"};
        for (int i = 0; i < 3; ++i) {
            RECT cab{columnas[i].left, yEncabezado, columnas[i].right, yEncabezado + 18};
            SetTextColor(hdc, tema.acento);
            DrawTextW(hdc, encabezados[i], -1, &cab,
                      DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        }

        const std::wstring diferenciasSha =
            m_estado.izquierda.sha256 == L"No disponible" ||
            m_estado.derecha.sha256 == L"No disponible"
                ? L"No aplica"
                : (m_estado.izquierda.sha256 == m_estado.derecha.sha256 ? L"Iguales" : L"Diferentes");
        const std::wstring diferenciaLineas =
            m_estado.izquierda.lineas && m_estado.derecha.lineas
                ? std::to_wstring(static_cast<long long>(m_estado.derecha.lineas) -
                                  static_cast<long long>(m_estado.izquierda.lineas))
                : L"No aplica";
        const std::wstring diferenciaPaginas =
            m_estado.izquierda.paginas && m_estado.derecha.paginas
                ? std::to_wstring(static_cast<long long>(m_estado.derecha.paginas) -
                                  static_cast<long long>(m_estado.izquierda.paginas))
                : L"No aplica";
        const std::wstring filas[5][3] = {
            {m_estado.izquierda.nombre, m_estado.derecha.nombre, L"No aplica"},
            {m_estado.izquierda.sha256, m_estado.derecha.sha256, diferenciasSha},
            {m_servicio ? m_servicio->FormatearTamano(m_estado.izquierda.bytes) : L"0 B",
             m_servicio ? m_servicio->FormatearTamano(m_estado.derecha.bytes) : L"0 B",
             m_servicio ? m_servicio->FormatearDiferencia(m_estado.izquierda.bytes, m_estado.derecha.bytes) : L"+0 B"},
            {TextoConteo(m_estado.izquierda.lineas), TextoConteo(m_estado.derecha.lineas), diferenciaLineas},
            {TextoConteo(m_estado.izquierda.paginas), TextoConteo(m_estado.derecha.paginas), diferenciaPaginas}
        };
        const wchar_t* etiquetas[] = {L"Nombre: ", L"SHA-256: ", L"Peso: ", L"Líneas: ", L"Páginas: "};
        for (int fila = 0; fila < 5; ++fila) {
            for (int columna = 0; columna < 3; ++columna) {
                RECT dato{columnas[columna].left, yDatos + fila * altoFila,
                          columnas[columna].right, yDatos + fila * altoFila + altoFila};
                SetTextColor(hdc, columna == 2 ? tema.acento : tema.textoSecundario);
                std::wstring valor = (columna == 0 ? etiquetas[fila] : L"") + filas[fila][columna];
                DrawTextW(hdc, valor.c_str(), -1, &dato,
                          DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
            }
        }
    }

    void DibujarControlDesplazamiento(HDC hdc, const RECT& rect,
                                       std::size_t maximo, std::size_t actual,
                                       const TemaVisual& tema) const {
        if (rect.right <= rect.left || rect.bottom <= rect.top) return;
        const int y = rect.top + (rect.bottom - rect.top) / 2;
        RECT linea{rect.left + 8, y - 2, rect.right - 8, y + 2};
        HBRUSH brocha = CreateSolidBrush(tema.borde);
        FillRect(hdc, &linea, brocha);
        DeleteObject(brocha);
        const double proporcion = maximo == 0 ? 0.0 :
            static_cast<double>(std::min(actual, maximo)) / static_cast<double>(maximo);
        const int x = linea.left + static_cast<int>((linea.right - linea.left) * proporcion);
        brocha = CreateSolidBrush(tema.acento);
        Ellipse(hdc, x - 6, y - 6, x + 6, y + 6);
        DeleteObject(brocha);
    }

    std::size_t DesplazamientoDesdePunto(const RECT& rect, POINT punto,
                                         std::size_t maximo) const {
        const int inicio = rect.left + 8;
        const int fin = rect.right - 8;
        if (fin <= inicio || maximo == 0) return 0;
        const double proporcion = std::clamp(
            static_cast<double>(punto.x - inicio) / static_cast<double>(fin - inicio), 0.0, 1.0);
        return static_cast<std::size_t>(proporcion * static_cast<double>(maximo));
    }

    HWND m_panel{nullptr};
    RECT m_rcVolver{};
    RECT m_rcBotonDiffView{};
    RECT m_rcAreaEspecifica{};
    RECT m_rcResumen{};
    bool m_modoDiffView{false};
    EstadoComparadorArchivosV2 m_estado;
    std::shared_ptr<ServicioComparadorArchivosV2> m_servicio;
};

} // namespace SideViewV2
