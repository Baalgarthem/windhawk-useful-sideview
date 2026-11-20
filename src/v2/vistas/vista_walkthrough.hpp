#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/ui/ventana_base.hpp"
#include "v2/nucleo/servicio_enlaces.hpp"
#include <memory>

namespace SideViewV2 {

class VistaWalkthrough : public IPanelContenido {
public:
    explicit VistaWalkthrough(std::shared_ptr<ServicioEnlacesV2> enlaces) : m_enlaces(std::move(enlaces)) {}
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) override {
        m_carpeta = carpeta;
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) override {
        if (msg == WM_LBUTTONDOWN) {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (PtInRect(&m_rectLinkGithub, pt)) {
                m_enlaces->Abrir(m_urlGithub);
                return true;
            }
        }
        return false;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        int ancho = rect.right - rect.left;
        int alto = rect.bottom - rect.top;
        if (ancho <= 0 || alto <= 0) return;

        // 1. Fondo principal
        HBRUSH brochaFondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, brochaFondo);
        DeleteObject(brochaFondo);

        // 2. Cabecera Fluent
        int altoCabecera = 54;
        RECT rectCabecera{rect.left, rect.top, rect.right, rect.top + altoCabecera};
        HBRUSH brochaCabecera = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &rectCabecera, brochaCabecera);
        DeleteObject(brochaCabecera);

        // Línea divisoria suave bajo la cabecera
        HPEN plumaBorde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaBorde));
        MoveToEx(hdc, rect.left, rect.top + altoCabecera, nullptr);
        LineTo(hdc, rect.right, rect.top + altoCabecera);

        // Borde izquierdo vertical separador entre vista de Explorer y el panel
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
        HFONT fuenteBold = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");

        HFONT fuenteVieja = static_cast<HFONT>(SelectObject(hdc, fuenteTitulo));

        // Título Cabecera
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rectTextoTitulo{rect.left + 16, rect.top + 8, rect.right - 16, rect.top + 28};
        DrawTextW(hdc, L"Useful Side View • Guía Rápida", -1, &rectTextoTitulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rectTextoSub{rect.left + 16, rect.top + 28, rect.right - 16, rect.top + 46};
        DrawTextW(hdc, L"Pantalla Tutorial (Walkthrough) • Sin selección activa", -1, &rectTextoSub,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        int margen = 16;
        int yCursor = rect.top + altoCabecera + 14;

        // Tarjeta 1: Cómo se usa el mod
        RECT rcTarjeta1{rect.left + margen, yCursor, rect.right - margen, yCursor + 115};
        DibujarTarjeta(hdc, rcTarjeta1, tema);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.acento);
        RECT rcT1Tit{rcTarjeta1.left + 12, rcTarjeta1.top + 8, rcTarjeta1.right - 12, rcTarjeta1.top + 26};
        DrawTextW(hdc, L"🚀 Cómo utilizar Useful Side View", -1, &rcT1Tit, DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring descUso =
            L"• Selecciona cualquier archivo o carpeta en Explorer para ver su previsualización automática.\n"
            L"• Presiona el comando de activación para alternar el panel lateral.\n"
            L"• Si no hay archivos seleccionados, verás siempre esta pantalla de tutorial.";
        RECT rcT1Desc{rcTarjeta1.left + 12, rcTarjeta1.top + 30, rcTarjeta1.right - 12, rcTarjeta1.bottom - 8};
        DrawTextW(hdc, descUso.c_str(), -1, &rcT1Desc, DT_WORDBREAK | DT_NOPREFIX);

        yCursor = rcTarjeta1.bottom + 12;

        // Tarjeta 2: Vistas Disponibles
        RECT rcTarjeta2{rect.left + margen, yCursor, rect.right - margen, yCursor + 155};
        DibujarTarjeta(hdc, rcTarjeta2, tema);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.acento);
        RECT rcT2Tit{rcTarjeta2.left + 12, rcTarjeta2.top + 8, rcTarjeta2.right - 12, rcTarjeta2.top + 26};
        DrawTextW(hdc, L"📂 Centro de Vistas Especializadas", -1, &rcT2Tit, DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring descVistas =
            L"📦 Comprimidos & Cómics: Visor de portadas, navegación secuencial, descompresión y convertidor.\n"
            L"📄 Texto & Markdown: Lector GitHub con tablas, checklist y barras (En desarrollo).\n"
            L"🖼️ Imágenes & GIFs: Reproducción fluida en bucle infinito y miniaturas (En desarrollo).\n"
            L"🎬 Video Suite: Reproductor de video interactivo por Media Foundation (En desarrollo).\n"
            L"📑 Documentos & Código: Visor multipropósito de documentos (En desarrollo).";
        RECT rcT2Desc{rcTarjeta2.left + 12, rcTarjeta2.top + 30, rcTarjeta2.right - 12, rcTarjeta2.bottom - 8};
        DrawTextW(hdc, descVistas.c_str(), -1, &rcT2Desc, DT_WORDBREAK | DT_NOPREFIX);

        yCursor = rcTarjeta2.bottom + 12;

        // Tarjeta 3: Comandos y Ubicación
        RECT rcTarjeta3{rect.left + margen, yCursor, rect.right - margen, yCursor + 95};
        DibujarTarjeta(hdc, rcTarjeta3, tema);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.acento);
        RECT rcT3Tit{rcTarjeta3.left + 12, rcTarjeta3.top + 8, rcTarjeta3.right - 12, rcTarjeta3.top + 26};
        DrawTextW(hdc, L"⌨️ Comandos y Ubicación", -1, &rcT3Tit, DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring descComandos =
            L"• Comando de Alternancia: [ Ctrl + K ] (Configurable en Windhawk)\n"
            L"• Ubicación del Mod: Inyectado cooperativamente en explorer.exe (Sin DLLs invasivas)\n"
            L"• Carpeta activa: " + (m_carpeta.empty() ? L"(Raíz o Equipo)" : m_carpeta);
        RECT rcT3Desc{rcTarjeta3.left + 12, rcTarjeta3.top + 30, rcTarjeta3.right - 12, rcTarjeta3.bottom - 8};
        DrawTextW(hdc, descComandos.c_str(), -1, &rcT3Desc, DT_WORDBREAK | DT_NOPREFIX);

        yCursor = rcTarjeta3.bottom + 12;

        // Tarjeta 4: Enlace GitHub
        RECT rcTarjeta4{rect.left + margen, yCursor, rect.right - margen, yCursor + 50};
        DibujarTarjeta(hdc, rcTarjeta4, tema);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rcT4Texto{rcTarjeta4.left + 12, rcTarjeta4.top + 14, rcTarjeta4.left + 140, rcTarjeta4.top + 34};
        DrawTextW(hdc, L"Repositorio oficial:", -1, &rcT4Texto, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        // Enlace clicable en azul acento
        m_rectLinkGithub = {rcT4Texto.right + 4, rcTarjeta4.top + 14, rcTarjeta4.right - 12, rcTarjeta4.top + 34};
        SetTextColor(hdc, tema.acento);
        DrawTextW(hdc, m_urlGithub.c_str(), -1, &m_rectLinkGithub, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Limpieza de fuentes
        SelectObject(hdc, fuenteVieja);
        DeleteObject(fuenteTitulo);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteBold);
    }

private:
    void DibujarTarjeta(HDC hdc, const RECT& rc, const TemaVisual& tema) {
        HBRUSH br = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rc, br);
        DeleteObject(br);

        HPEN pen = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    std::wstring m_carpeta;
    std::wstring m_urlGithub{L"https://github.com/ramonrey/windhawk-useful-sideview"};
    std::shared_ptr<ServicioEnlacesV2> m_enlaces;
    RECT m_rectLinkGithub{};
};

} // namespace SideViewV2
