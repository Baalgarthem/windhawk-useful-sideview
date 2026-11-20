#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/ui/ventana_base.hpp"
#include "v2/nucleo/servicios_vistas.hpp"
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <algorithm>

namespace SideViewV2 {

class VistaComics : public IPanelContenido {
public:
    explicit VistaComics(std::shared_ptr<ServicioExtensionV2> extensiones,
                         std::shared_ptr<ServicioNavegacionV2> navegacion,
                         std::shared_ptr<ServicioPrevisualizacionMediaV2> previsualizacion)
        : m_extensiones(std::move(extensiones)), m_navegacion(std::move(navegacion)), m_previsualizacion(std::move(previsualizacion)) {}

    ~VistaComics() override { Liberar(); }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        m_archivos = archivos;
        m_indice = 0;
        CargarPortadas();
    }

    void Liberar() override {
        for (HBITMAP portada : m_portadas) if (portada) DeleteObject(portada);
        m_portadas.clear();
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM, LPARAM lParam, HWND hwnd) override {
        if (msg != WM_LBUTTONDOWN) return false;
        POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (PtInRect(&m_rcAnterior, pt) && m_archivos.size() > 1) {
            m_indice = m_navegacion->Anterior(m_indice, m_archivos.size());
            InvalidateRect(hwnd, nullptr, FALSE); return true;
        }
        if (PtInRect(&m_rcSiguiente, pt) && m_archivos.size() > 1) {
            m_indice = m_navegacion->Siguiente(m_indice, m_archivos.size());
            InvalidateRect(hwnd, nullptr, FALSE); return true;
        }
        if (PtInRect(&m_rcCambiarExtension, pt)) {
            MostrarMenuExtensiones(hwnd); return true;
        }
        if (PtInRect(&m_rcComparar, pt)) {
            if (m_archivos.size() == 2) {
                PostMessageW(hwnd, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
            }
            return true;
        }
        return false;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        if (!hdc || rect.right <= rect.left || rect.bottom <= rect.top) return;
        if (m_indice >= m_archivos.size()) m_indice = 0;
        HBRUSH fondo = CreateSolidBrush(tema.fondo); FillRect(hdc, &rect, fondo); DeleteObject(fondo);
        SetBkMode(hdc, TRANSPARENT);
        HFONT titulo = CreateFontW(-17, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT texto = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT anterior = static_cast<HFONT>(SelectObject(hdc, titulo));
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rcTitulo{rect.left + 16, rect.top + 10, rect.right - 16, rect.top + 35};
        std::wstring nombre = m_archivos.empty() ? L"Sin comics seleccionados" :
            L"📚 " + std::wstring(PathFindFileNameW(m_archivos[m_indice].c_str()));
        DrawTextW(hdc, nombre.c_str(), -1, &rcTitulo, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(hdc, texto); SetTextColor(hdc, tema.textoSecundario);
        RECT rcSub{rect.left + 16, rect.top + 36, rect.right - 16, rect.top + 58};
        std::wstring sub = std::to_wstring(m_archivos.size()) + L" comic(s) · Cover de la primera página";
        DrawTextW(hdc, sub.c_str(), -1, &rcSub, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        int yBoton = rect.top + 72;
        m_rcAnterior = {rect.left + 16, yBoton, rect.left + 50, yBoton + 30};
        m_rcSiguiente = {rect.left + 56, yBoton, rect.left + 90, yBoton + 30};
        DibujarBoton(hdc, m_rcAnterior, L"◀", tema, m_archivos.size() > 1);
        DibujarBoton(hdc, m_rcSiguiente, L"▶", tema, m_archivos.size() > 1);
        m_rcCambiarExtension = {rect.right - 170, yBoton, rect.right - 16, yBoton + 30};
        DibujarBoton(hdc, m_rcCambiarExtension, L"Cambiar extensión", tema, true);
        m_rcComparar = {rect.right - 296, yBoton, rect.right - 178, yBoton + 30};
        DibujarBoton(hdc, m_rcComparar, L"Comparar archivos", tema,
                     m_archivos.size() == 2);

        // Contenedor cuadrado con grid fijo 2x2. Evita que los covers se
        // conviertan en una fila horizontal y mantiene una composición
        // estable con una, dos, tres o cuatro selecciones.
        int top = yBoton + 48;
        int bottom = rect.bottom - 18;
        int anchoDisponible = rect.right - rect.left - 32;
        int altoDisponible = bottom - top - 30;
        int ladoContenedor = std::max(160, std::min(anchoDisponible, altoDisponible));
        ladoContenedor = std::min(ladoContenedor, std::max(160, anchoDisponible));
        int gridLeft = rect.left + ((rect.right - rect.left) - ladoContenedor) / 2;
        int gridTop = top + std::max(0, (altoDisponible - ladoContenedor) / 2);
        RECT contenedorGrid{gridLeft, gridTop, gridLeft + ladoContenedor, gridTop + ladoContenedor};
        HBRUSH fondoGrid = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &contenedorGrid, fondoGrid);
        DeleteObject(fondoGrid);
        HPEN bordeGrid = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anteriorGrid = static_cast<HPEN>(SelectObject(hdc, bordeGrid));
        HBRUSH sinRelleno = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorRelleno = static_cast<HBRUSH>(SelectObject(hdc, sinRelleno));
        RoundRect(hdc, contenedorGrid.left, contenedorGrid.top, contenedorGrid.right,
                  contenedorGrid.bottom, 10, 10);
        SelectObject(hdc, anteriorRelleno);
        SelectObject(hdc, anteriorGrid);
        DeleteObject(bordeGrid);

        int separacion = 8;
        int ladoCelda = (ladoContenedor - separacion * 3) / 2;
        size_t cantidad = std::min<size_t>(4, m_archivos.size());
        for (size_t i = 0; i < cantidad; ++i) {
            int fila = static_cast<int>(i / 2);
            int columna = static_cast<int>(i % 2);
            int tileLeft = contenedorGrid.left + separacion + columna * (ladoCelda + separacion);
            int tileTop = contenedorGrid.top + separacion + fila * (ladoCelda + separacion);
            RECT tile{tileLeft, tileTop, tileLeft + ladoCelda, tileTop + ladoCelda};
            HBRUSH tarjeta = CreateSolidBrush(tema.fondoCabecera); FillRect(hdc, &tile, tarjeta); DeleteObject(tarjeta);
            if (i < m_portadas.size() && m_portadas[i]) {
                BITMAP bm{};
                if (GetObjectW(m_portadas[i], sizeof(bm), &bm) != sizeof(bm) ||
                    bm.bmWidth <= 0 || bm.bmHeight == 0) {
                    continue;
                }
                HDC mem = CreateCompatibleDC(hdc); if (!mem) continue; HGDIOBJ old = SelectObject(mem, m_portadas[i]); if (!old || old == HGDI_ERROR) { DeleteDC(mem); continue; }
                int margen = 8; int destinoAncho = ladoCelda - margen * 2;
                int anchoBitmap = std::max(1, static_cast<int>(bm.bmWidth));
                int altoDisponiblePortada = ladoCelda - margen * 2;
                int altoPortada = MulDiv(static_cast<int>(bm.bmHeight), destinoAncho, anchoBitmap);
                int destinoAlto = std::min(altoDisponiblePortada, altoPortada);
                int destinoX = tile.left + (ladoCelda - destinoAncho) / 2;
                int destinoY = tile.top + (ladoCelda - destinoAlto) / 2;
                int modoAnterior = SetStretchBltMode(hdc, HALFTONE);
                SetBrushOrgEx(hdc, 0, 0, nullptr);
                StretchBlt(hdc, destinoX, destinoY, destinoAncho, destinoAlto,
                           mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
                SetStretchBltMode(hdc, modoAnterior);
                SelectObject(mem, old); DeleteDC(mem);
            } else {
                SetTextColor(hdc, tema.acento);
                DrawTextW(hdc, L"COVER\nno disponible", -1, &tile, DT_CENTER | DT_VCENTER | DT_NOPREFIX);
            }
        }
        if (m_archivos.size() > 4) {
            RECT contador{contenedorGrid.left, contenedorGrid.bottom + 4,
                          contenedorGrid.right, bottom};
            SetTextColor(hdc, tema.textoSecundario);
            std::wstring extra = L"+" + std::to_wstring(m_archivos.size() - 4) + L" comics adicionales";
            DrawTextW(hdc, extra.c_str(), -1, &contador,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
        SelectObject(hdc, anterior); DeleteObject(titulo); DeleteObject(texto);
    }

private:
    void DibujarBoton(HDC hdc, const RECT& rc, const wchar_t* texto,
                      const TemaVisual& tema, bool habilitado) {
        HBRUSH fondo = CreateSolidBrush(habilitado ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &rc, fondo); DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anteriorPen = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH transparente = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrush = static_cast<HBRUSH>(SelectObject(hdc, transparente));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
        SelectObject(hdc, anteriorBrush); SelectObject(hdc, anteriorPen); DeleteObject(borde);
        SetTextColor(hdc, habilitado ? tema.textoPrincipal : tema.textoSecundario);
        RECT textoRect = rc;
        DrawTextW(hdc, texto, -1, &textoRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
    }

    void CargarPortadas() {
        Liberar();
        size_t cantidad = std::min<size_t>(4, m_archivos.size());
        for (size_t i = 0; i < cantidad; ++i) {
            HBITMAP bitmap = m_previsualizacion->CargarMiniatura(m_archivos[i], SIZE{420, 620});
            // Conservamos una entrada por celda; un thumbnail ausente se
            // representa como fallback visual y nunca se usa como bitmap.
            m_portadas.push_back(bitmap);
        }
    }

    void MostrarMenuExtensiones(HWND hwnd) {
        HMENU menu = CreatePopupMenu();
        const wchar_t* opciones[] = {L".cbz", L".cbr", L".cb7", L".cbt", L".cba", L".zip", L".rar"};
        for (UINT i = 0; i < ARRAYSIZE(opciones); ++i) AppendMenuW(menu, MF_STRING, i + 1, opciones[i]);
        POINT p{m_rcCambiarExtension.left, m_rcCambiarExtension.bottom}; ClientToScreen(hwnd, &p);
        UINT cmd = TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, p.x, p.y, 0, hwnd, nullptr);
        DestroyMenu(menu);
        if (cmd >= 1 && cmd <= ARRAYSIZE(opciones)) {
            size_t cantidad = m_extensiones->Cambiar(m_archivos, opciones[cmd - 1]);
            MessageBoxW(hwnd, (L"Se cambiaron " + std::to_wstring(cantidad) + L" archivo(s)." ).c_str(), L"Cambiar extensión", MB_OK | MB_ICONINFORMATION);
        }
    }

    std::vector<std::wstring> m_archivos;
    std::vector<HBITMAP> m_portadas;
    std::shared_ptr<ServicioExtensionV2> m_extensiones;
    std::shared_ptr<ServicioNavegacionV2> m_navegacion;
    std::shared_ptr<ServicioPrevisualizacionMediaV2> m_previsualizacion;
    size_t m_indice{0};
    RECT m_rcAnterior{}, m_rcSiguiente{}, m_rcCambiarExtension{}, m_rcComparar{};
};
}
