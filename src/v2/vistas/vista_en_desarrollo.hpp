#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/ui/ventana_base.hpp"

namespace SideViewV2 {

class VistaEnDesarrollo : public IPanelContenido {
public:
    VistaEnDesarrollo(const std::wstring& nombreVista, const std::wstring& iconoVista, const std::wstring& descripcion)
        : m_nombreVista(nombreVista), m_iconoVista(iconoVista), m_descripcion(descripcion) {}

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) override {
        m_archivos = archivos;
        m_carpeta = carpeta;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        int ancho = rect.right - rect.left;
        int alto = rect.bottom - rect.top;
        if (ancho <= 0 || alto <= 0) return;

        // Fondo
        HBRUSH brochaFondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, brochaFondo);
        DeleteObject(brochaFondo);

        // Cabecera Fluent
        int altoCabecera = 54;
        RECT rectCabecera{rect.left, rect.top, rect.right, rect.top + altoCabecera};
        HBRUSH brochaCabecera = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &rectCabecera, brochaCabecera);
        DeleteObject(brochaCabecera);

        // Bordes separadores
        HPEN plumaBorde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaBorde));
        MoveToEx(hdc, rect.left, rect.top + altoCabecera, nullptr);
        LineTo(hdc, rect.right, rect.top + altoCabecera);
        MoveToEx(hdc, rect.left, rect.top, nullptr);
        LineTo(hdc, rect.left, rect.bottom);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaBorde);

        SetBkMode(hdc, TRANSPARENT);

        HFONT fuenteTitulo = CreateFontW(-16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT fuenteSubtitulo = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteGrande = CreateFontW(-28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT fuenteInsignia = CreateFontW(-11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");

        HFONT fuenteVieja = static_cast<HFONT>(SelectObject(hdc, fuenteTitulo));

        // Título de vista
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring headerText = m_iconoVista + L"  " + m_nombreVista;
        RECT rectTextoTitulo{rect.left + 16, rect.top + 8, rect.right - 16, rect.top + 28};
        DrawTextW(hdc, headerText.c_str(), -1, &rectTextoTitulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Subtítulo
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rectTextoSub{rect.left + 16, rect.top + 28, rect.right - 16, rect.top + 46};
        DrawTextW(hdc, L"Módulo de Previsualización Especializada", -1, &rectTextoSub,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Tarjeta Central informativa
        int margen = 20;
        int topTarjeta = rect.top + altoCabecera + 24;
        RECT rcTarjeta{rect.left + margen, topTarjeta, rect.right - margen, topTarjeta + 220};

        HBRUSH brTarjeta = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rcTarjeta, brTarjeta);
        DeleteObject(brTarjeta);

        plumaBorde = CreatePen(PS_SOLID, 1, tema.borde);
        plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaBorde));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rcTarjeta.left, rcTarjeta.top, rcTarjeta.right, rcTarjeta.bottom, 12, 12);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaBorde);

        // Insignia "(En desarrollo)"
        RECT rcBadge{rcTarjeta.left + 16, rcTarjeta.top + 16, rcTarjeta.left + 130, rcTarjeta.top + 40};
        HBRUSH brBadge = CreateSolidBrush(RGB(220, 100, 30)); // Naranja de advertencia amigable
        FillRect(hdc, &rcBadge, brBadge);
        DeleteObject(brBadge);

        SelectObject(hdc, fuenteInsignia);
        SetTextColor(hdc, RGB(255, 255, 255));
        DrawTextW(hdc, L"EN DESARROLLO", -1, &rcBadge, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // Icono gigante
        SelectObject(hdc, fuenteGrande);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rcIcono{rcTarjeta.left + 16, rcTarjeta.top + 48, rcTarjeta.right - 16, rcTarjeta.top + 88};
        DrawTextW(hdc, m_iconoVista.c_str(), -1, &rcIcono, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        // Título del módulo
        SelectObject(hdc, fuenteTitulo);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rcModTitle{rcTarjeta.left + 16, rcTarjeta.top + 92, rcTarjeta.right - 16, rcTarjeta.top + 116};
        DrawTextW(hdc, m_nombreVista.c_str(), -1, &rcModTitle, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        // Descripción del módulo
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rcModDesc{rcTarjeta.left + 16, rcTarjeta.top + 120, rcTarjeta.right - 16, rcTarjeta.top + 175};
        DrawTextW(hdc, m_descripcion.c_str(), -1, &rcModDesc, DT_WORDBREAK | DT_NOPREFIX);

        // Elementos seleccionados
        std::wstring infoSeleccion;
        if (m_archivos.empty()) {
            infoSeleccion = L"Sin archivos seleccionados.";
        } else if (m_archivos.size() == 1) {
            infoSeleccion = L"Archivo: " + m_archivos.front();
        } else {
            infoSeleccion = L"Selección múltiple: " + std::to_wstring(m_archivos.size()) + L" elementos";
        }
        RECT rcModSel{rcTarjeta.left + 16, rcTarjeta.top + 180, rcTarjeta.right - 16, rcTarjeta.bottom - 10};
        DrawTextW(hdc, infoSeleccion.c_str(), -1, &rcModSel, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        SelectObject(hdc, fuenteVieja);
        DeleteObject(fuenteTitulo);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteGrande);
        DeleteObject(fuenteInsignia);
    }

private:
    std::wstring m_nombreVista;
    std::wstring m_iconoVista;
    std::wstring m_descripcion;
    std::vector<std::wstring> m_archivos;
    std::wstring m_carpeta;
};

} // namespace SideViewV2
