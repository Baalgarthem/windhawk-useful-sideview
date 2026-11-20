#pragma once

#include "v2/vistas/vista_comparador_archivos.hpp"
#include "v2/nucleo/servicio_comparador_imagenes.hpp"
#include <windows.h>
#include <windowsx.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

class VistaComparadorImagenesV2 final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorImagenesV2(
        std::shared_ptr<ServicioComparadorImagenesV2> servicio)
        : VistaComparadorArchivosV2(servicio), m_servicio(std::move(servicio)) {}

    ~VistaComparadorImagenesV2() override { Liberar(); }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring& carpeta) override {
        LiberarImagenes();
        VistaComparadorArchivosV2::AlCambiarSeleccion(archivos, carpeta);
        if (archivos.size() == 2 && m_servicio) {
            m_imagenIzquierda = m_servicio->CargarImagen(archivos[0]);
            m_imagenDerecha = m_servicio->CargarImagen(archivos[1]);
        }
        m_estadoImagen = {};
        m_hover = 0;
    }

    void Liberar() override {
        LiberarImagenes();
        m_estadoImagen = {};
        m_hover = 0;
        VistaComparadorArchivosV2::Liberar();
    }

protected:
    const wchar_t* NombreVista() const override { return L"Comparar imágenes"; }

    void RenderizarContenido(HDC hdc, const RECT& rect,
                             const TemaVisual& tema, HFONT fuente) override {
        DibujarContenedor(hdc, rect, tema);
        if (!m_imagenIzquierda || !m_imagenDerecha) {
            SetTextColor(hdc, tema.textoSecundario);
            RECT mensaje = rect;
            DrawTextW(hdc, L"No se pudieron cargar ambas imágenes.", -1, &mensaje,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS |
                          DT_NOPREFIX);
            return;
        }

        if (m_estadoImagen.modoPantallaCompleta) {
            RenderizarPantallaCompleta(hdc, rect, tema, fuente);
        } else {
            RenderizarLadoALado(hdc, rect, tema, fuente);
        }
    }

    bool ProcesarMensajeContenido(UINT mensaje, WPARAM, LPARAM lParam,
                                  HWND hwndPanel) override {
        POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (mensaje == WM_MOUSEMOVE) {
            const int hoverNuevo = PtInRect(&m_rcBotonFullIzquierda, punto) ? 1
                : PtInRect(&m_rcBotonFullDerecha, punto) ? 2
                : PtInRect(&m_rcImagenIzquierda, punto) ? 3
                : PtInRect(&m_rcImagenDerecha, punto) ? 4 : 0;
            if (hoverNuevo != m_hover) {
                m_hover = hoverNuevo;
                InvalidateRect(hwndPanel, nullptr, FALSE);
            }
            return hoverNuevo != 0;
        }
        if (mensaje != WM_LBUTTONDOWN || !m_servicio) return false;

        if (PtInRect(&m_rcBotonFullIzquierda, punto)) {
            m_servicio->AlternarPantallaCompleta(m_estadoImagen, 0);
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (PtInRect(&m_rcBotonFullDerecha, punto)) {
            m_servicio->AlternarPantallaCompleta(m_estadoImagen, 1);
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (m_estadoImagen.modoPantallaCompleta &&
            ((m_estadoImagen.indicePantallaCompleta == 0 &&
              PtInRect(&m_rcImagenDerecha, punto)) ||
             (m_estadoImagen.indicePantallaCompleta == 1 &&
              PtInRect(&m_rcImagenIzquierda, punto)))) {
            m_servicio->IntercambiarPantallaCompleta(m_estadoImagen);
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        return false;
    }

private:
    void RenderizarLadoALado(HDC hdc, const RECT& rect,
                             const TemaVisual& tema, HFONT fuente) {
        const int separacion = 10;
        const int ancho = std::max(
            1, (static_cast<int>(rect.right - rect.left) - separacion) / 2);
        m_rcImagenIzquierda = {rect.left, rect.top,
                               rect.left + ancho, rect.bottom};
        m_rcImagenDerecha = {m_rcImagenIzquierda.right + separacion, rect.top,
                             rect.right, rect.bottom};
        DibujarPanelImagen(hdc, m_rcImagenIzquierda, m_imagenIzquierda,
                           L"IMAGEN 1", NombreArchivo(0), false, 0, tema, fuente);
        DibujarPanelImagen(hdc, m_rcImagenDerecha, m_imagenDerecha,
                           L"IMAGEN 2", NombreArchivo(1), false, 1, tema, fuente);
    }

    void RenderizarPantallaCompleta(HDC hdc, const RECT& rect,
                                    const TemaVisual& tema, HFONT fuente) {
        m_rcImagenIzquierda = rect;
        m_rcImagenDerecha = rect;
        const int anchoPip = std::max(
            180, static_cast<int>(rect.right - rect.left) / 4);
        const int altoPip = std::max(
            130, static_cast<int>(rect.bottom - rect.top) / 4);
        RECT pip{rect.right - anchoPip - 14, rect.bottom - altoPip - 14,
                 rect.right - 14, rect.bottom - 14};
        if (m_estadoImagen.indicePantallaCompleta == 0) {
            m_rcImagenDerecha = pip;
        } else {
            m_rcImagenIzquierda = pip;
        }

        DibujarPanelImagen(hdc, m_rcImagenIzquierda, m_imagenIzquierda,
                           L"IMAGEN 1", NombreArchivo(0),
                           m_estadoImagen.indicePantallaCompleta != 0, 0,
                           tema, fuente);
        DibujarPanelImagen(hdc, m_rcImagenDerecha, m_imagenDerecha,
                           L"IMAGEN 2", NombreArchivo(1),
                           m_estadoImagen.indicePantallaCompleta != 1, 1,
                           tema, fuente);
    }

    std::wstring NombreArchivo(std::size_t indice) const {
        if (m_estado.izquierda.ruta.empty() || indice > 1) return L"Archivo";
        const std::wstring& ruta = indice == 0
            ? m_estado.izquierda.ruta : m_estado.derecha.ruta;
        const wchar_t* nombre = PathFindFileNameW(ruta.c_str());
        return nombre && *nombre ? nombre : L"Archivo";
    }

    void DibujarPanelImagen(HDC hdc, const RECT& rect, HBITMAP imagen,
                            const wchar_t* etiqueta, const std::wstring& nombre,
                            bool esPip, int indice, const TemaVisual& tema,
                            HFONT fuente) {
        DibujarContenedor(hdc, rect, tema);
        RECT cabecera{rect.left + 1, rect.top + 1, rect.right - 1,
                      rect.top + 31};
        HBRUSH fondo = CreateSolidBrush(esPip ? tema.fondoCabecera : tema.fondoTarjeta);
        FillRect(hdc, &cabecera, fondo);
        DeleteObject(fondo);

        SelectObject(hdc, fuente);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{rect.left + 10, rect.top + 4, rect.right - 42, rect.top + 28};
        std::wstring texto = esPip ? L"PIP · " : L"FULL SCREEN · ";
        if (!m_estadoImagen.modoPantallaCompleta) texto = etiqueta;
        else texto += std::wstring(etiqueta) + L" — " + nombre;
        DrawTextW(hdc, texto.c_str(), -1, &titulo,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS |
                      DT_NOPREFIX);

        RECT boton{rect.right - 34, rect.top + 4, rect.right - 6, rect.top + 28};
        if (indice == 0) m_rcBotonFullIzquierda = boton;
        else m_rcBotonFullDerecha = boton;
        DibujarBotonImagen(hdc, boton, L"⛶", tema,
                           m_hover == (indice == 0 ? 1 : 2));

        RECT contenido{rect.left + 8, rect.top + 36,
                       rect.right - 8, rect.bottom - 8};
        DibujarBitmap(hdc, contenido, imagen);
    }

    void DibujarBitmap(HDC hdc, const RECT& rect, HBITMAP bitmap) const {
        BITMAP datos{};
        if (!bitmap || GetObjectW(bitmap, sizeof(datos), &datos) != sizeof(datos) ||
            datos.bmWidth <= 0 || datos.bmHeight == 0) {
            return;
        }
        const int anchoDisponible = std::max(
            1, static_cast<int>(rect.right - rect.left));
        const int altoDisponible = std::max(
            1, static_cast<int>(rect.bottom - rect.top));
        int ancho = anchoDisponible;
        int alto = MulDiv(std::abs(static_cast<int>(datos.bmHeight)), ancho,
                          std::max(1, static_cast<int>(datos.bmWidth)));
        if (alto > altoDisponible) {
            alto = altoDisponible;
            ancho = MulDiv(static_cast<int>(datos.bmWidth), alto,
                           std::max(1, std::abs(static_cast<int>(datos.bmHeight))));
        }
        HDC memoria = CreateCompatibleDC(hdc);
        if (!memoria) return;
        HGDIOBJ anterior = SelectObject(memoria, bitmap);
        if (!anterior || anterior == HGDI_ERROR) {
            DeleteDC(memoria);
            return;
        }
        const int modoAnterior = SetStretchBltMode(hdc, HALFTONE);
        SetBrushOrgEx(hdc, 0, 0, nullptr);
        StretchBlt(hdc, rect.left + (anchoDisponible - ancho) / 2,
                   rect.top + (altoDisponible - alto) / 2, ancho, alto,
                   memoria, 0, 0, datos.bmWidth, std::abs(datos.bmHeight),
                   SRCCOPY);
        SetStretchBltMode(hdc, modoAnterior);
        SelectObject(memoria, anterior);
        DeleteDC(memoria);
    }

    void DibujarBotonImagen(HDC hdc, const RECT& rect, const wchar_t* texto,
                            const TemaVisual& tema, bool resaltado) const {
        HBRUSH fondo = CreateSolidBrush(resaltado ? tema.acento : tema.botonFondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT textoRect = rect;
        DrawTextW(hdc, texto, -1, &textoRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    void LiberarImagenes() {
        if (m_servicio) {
            m_servicio->LiberarImagen(m_imagenIzquierda);
            m_servicio->LiberarImagen(m_imagenDerecha);
        }
        m_imagenIzquierda = nullptr;
        m_imagenDerecha = nullptr;
    }

    std::shared_ptr<ServicioComparadorImagenesV2> m_servicio;
    HBITMAP m_imagenIzquierda{nullptr};
    HBITMAP m_imagenDerecha{nullptr};
    EstadoComparadorImagenesV2 m_estadoImagen;
    int m_hover{0};
    RECT m_rcImagenIzquierda{};
    RECT m_rcImagenDerecha{};
    RECT m_rcBotonFullIzquierda{};
    RECT m_rcBotonFullDerecha{};
};

} // namespace SideViewV2
