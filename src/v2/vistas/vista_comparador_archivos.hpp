#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/nucleo/servicio_comparador_archivos.hpp"
#include <windowsx.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

class VistaComparadorArchivosV2 : public IPanelContenido {
public:
    explicit VistaComparadorArchivosV2(
        std::shared_ptr<ServicioComparadorArchivosV2> servicio)
        : m_servicio(std::move(servicio)) {}

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        m_archivos = archivos;
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
        }
        if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
            PostMessageW(hwndPanel, WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS, 0, 0);
            return true;
        }
        return false;
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

        RECT info{rect.left + margen, rect.top + 58, rect.right - margen, rect.top + 194};
        DibujarInformacionComun(hdc, info, tema, texto);
        m_rcAreaEspecifica = {rect.left + margen, rect.top + 208,
                              rect.right - margen, rect.bottom - margen};
        RenderizarContenido(hdc, m_rcAreaEspecifica, tema, texto);

        SelectObject(hdc, anterior);
        DeleteObject(titulo);
        DeleteObject(texto);
    }

protected:
    virtual const wchar_t* NombreVista() const { return L"Comparar archivos"; }

    virtual void RenderizarContenido(HDC hdc, const RECT& rect,
                                     const TemaVisual& tema, HFONT fuente) {
        SetTextColor(hdc, tema.textoSecundario);
        RECT mensaje = rect;
        DrawTextW(hdc, m_estado.mensaje.c_str(), -1, &mensaje,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    void CargarComparacionComun(const std::vector<std::wstring>& archivos) {
        m_estado = m_servicio ? m_servicio->CrearComparacion(archivos)
                               : EstadoComparadorArchivosV2{};
    }

    const EstadoComparadorArchivosV2& EstadoComun() const { return m_estado; }
    const RECT& AreaEspecifica() const { return m_rcAreaEspecifica; }

    void DibujarInformacionComun(HDC hdc, const RECT& info,
                                 const TemaVisual& tema, HFONT fuente) {
        HBRUSH fondo = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &info, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, info.left, info.top, info.right, info.bottom, 8, 8);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);

        SelectObject(hdc, fuente);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{info.left + 12, info.top + 8, info.right - 12, info.top + 28};
        DrawTextW(hdc, L"INFORMACIÓN GENERAL", -1, &titulo,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        const int mitad = (info.right - info.left) / 2;
        DibujarDato(hdc, info, info.left + 12, info.left + mitad - 12,
                    m_estado.izquierda, L"Archivo 1", tema);
        DibujarDato(hdc, info, info.left + mitad + 12, info.right - 12,
                    m_estado.derecha, L"Archivo 2", tema);
        SetTextColor(hdc, tema.acento);
        RECT diferencia{info.left + 12, info.bottom - 26, info.right - 12, info.bottom - 8};
        std::wstring peso = L"Diferencia de peso: " +
            (m_servicio ? m_servicio->FormatearDiferencia(
                m_estado.izquierda.bytes, m_estado.derecha.bytes) : L"+0 B");
        DrawTextW(hdc, peso.c_str(), -1, &diferencia,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
    }

    void DibujarContenedor(HDC hdc, const RECT& rect, const TemaVisual& tema) {
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
                      const TemaVisual& tema) {
        HBRUSH fondo = CreateSolidBrush(tema.botonFondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT textoRect = rect;
        DrawTextW(hdc, texto, -1, &textoRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    static HFONT CrearFuente(int altura, int peso) {
        return CreateFontW(altura, 0, 0, 0, peso, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                           L"Segoe UI Variable Text");
    }

private:
    void DibujarDato(HDC hdc, const RECT& info, int izquierda, int derecha,
                     const DatosArchivoComparacionV2& datos,
                     const wchar_t* etiqueta, const TemaVisual& tema) {
        RECT nombre{izquierda, info.top + 36, derecha, info.top + 58};
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring texto = std::wstring(etiqueta) + L": " + datos.nombre;
        DrawTextW(hdc, texto.c_str(), -1, &nombre,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        RECT tamano{izquierda, info.top + 60, derecha, info.top + 82};
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring peso = L"Peso: " + (m_servicio ? m_servicio->FormatearTamano(datos.bytes) : L"0 B");
        DrawTextW(hdc, peso.c_str(), -1, &tamano,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        RECT hash{izquierda, info.top + 84, derecha, info.top + 106};
        std::wstring sha = L"SHA-256: " + datos.sha256;
        DrawTextW(hdc, sha.c_str(), -1, &hash,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    std::shared_ptr<ServicioComparadorArchivosV2> m_servicio;
    EstadoComparadorArchivosV2 m_estado;
    std::vector<std::wstring> m_archivos;
    HWND m_panel{nullptr};
    RECT m_rcVolver{};
    RECT m_rcAreaEspecifica{};
};

} // namespace SideViewV2
