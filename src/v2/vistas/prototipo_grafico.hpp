#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/ui/ventana_base.hpp"
#include <shlwapi.h>

namespace SideViewV2 {

class PrototipoPanelGrafico : public IPanelContenido {
public:
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring& carpeta) override {
        m_archivos = archivos;
        m_carpeta = carpeta;
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

        // Configuración de texto
        SetBkMode(hdc, TRANSPARENT);

        // Fuentes estándar de sistema
        HFONT fuenteTitulo = CreateFontW(-16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
        HFONT fuenteSubtitulo = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteGrande = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");

        HFONT fuenteVieja = static_cast<HFONT>(SelectObject(hdc, fuenteTitulo));

        // Título Cabecera
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rectTextoTitulo{rect.left + 16, rect.top + 8, rect.right - 16, rect.top + 28};
        DrawTextW(hdc, L"Useful Side View (v2)", -1, &rectTextoTitulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Subtítulo con atajo de teclado
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rectTextoSub{rect.left + 16, rect.top + 28, rect.right - 16, rect.top + 46};
        DrawTextW(hdc, L"Prototipo Gráfico Canónico • Alternar con [ Ctrl + K ]", -1, &rectTextoSub,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // 3. Tarjeta central de estado
        int margenLateral = 16;
        int margenSuperior = rect.top + altoCabecera + 16;
        int altoTarjeta = 150;
        RECT rectTarjeta{rect.left + margenLateral, margenSuperior,
                         rect.right - margenLateral, margenSuperior + altoTarjeta};

        HBRUSH brochaTarjeta = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rectTarjeta, brochaTarjeta);
        DeleteObject(brochaTarjeta);

        // Borde redondeado suave para la tarjeta
        HPEN plumaTarjeta = CreatePen(PS_SOLID, 1, tema.borde);
        plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaTarjeta));
        HBRUSH brochaNula = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH brochaVieja = static_cast<HBRUSH>(SelectObject(hdc, brochaNula));
        RoundRect(hdc, rectTarjeta.left, rectTarjeta.top, rectTarjeta.right, rectTarjeta.bottom, 10, 10);
        SelectObject(hdc, brochaVieja);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaTarjeta);

        // Contenido de la tarjeta de estado
        SelectObject(hdc, fuenteTitulo);
        SetTextColor(hdc, tema.acento);
        RECT rectTituloTarjeta{rectTarjeta.left + 14, rectTarjeta.top + 12, rectTarjeta.right - 14, rectTarjeta.top + 32};
        DrawTextW(hdc, L"Estado de Selección Shell", -1, &rectTituloTarjeta, DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoPrincipal);

        std::wstring lineaSeleccion;
        if (m_archivos.empty()) {
            lineaSeleccion = L"Sin selección activa en la lista de archivos.";
        } else if (m_archivos.size() == 1) {
            lineaSeleccion = L"Archivo: " + m_archivos.front();
        } else {
            lineaSeleccion = L"Elementos seleccionados: " + std::to_wstring(m_archivos.size()) + L" archivos";
        }

        RECT rectDetalleSeleccion{rectTarjeta.left + 14, rectTarjeta.top + 38, rectTarjeta.right - 14, rectTarjeta.top + 80};
        DrawTextW(hdc, lineaSeleccion.c_str(), -1, &rectDetalleSeleccion,
                  DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Carpeta contenedora
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring textoCarpeta = L"Carpeta: " + (m_carpeta.empty() ? L"(Detectando...)" : m_carpeta);
        RECT rectCarpeta{rectTarjeta.left + 14, rectTarjeta.top + 88, rectTarjeta.right - 14, rectTarjeta.top + 138};
        DrawTextW(hdc, textoCarpeta.c_str(), -1, &rectCarpeta,
                  DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

        // 4. Panel de vista previa modular (Píldora informativa)
        int topPildora = rectTarjeta.bottom + 20;
        RECT rectPildora{rect.left + margenLateral, topPildora, rect.right - margenLateral, topPildora + 180};

        HBRUSH brochaPildora = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rectPildora, brochaPildora);
        DeleteObject(brochaPildora);

        plumaTarjeta = CreatePen(PS_SOLID, 1, tema.borde);
        plumaVieja = static_cast<HPEN>(SelectObject(hdc, plumaTarjeta));
        brochaVieja = static_cast<HBRUSH>(SelectObject(hdc, brochaNula));
        RoundRect(hdc, rectPildora.left, rectPildora.top, rectPildora.right, rectPildora.bottom, 10, 10);
        SelectObject(hdc, brochaVieja);
        SelectObject(hdc, plumaVieja);
        DeleteObject(plumaTarjeta);

        SelectObject(hdc, fuenteGrande);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT rectHeaderModulo{rectPildora.left + 14, rectPildora.top + 14, rectPildora.right - 14, rectPildora.top + 42};
        DrawTextW(hdc, L"Arquitectura Canónica Win11", -1, &rectHeaderModulo, DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring textoExplicativo =
            L"✓ Ventana hermana inyectada (Sin alterar DirectUI de Explorer)\n"
            L"✓ Reflow nativo de columnas sin parpadeo (Anti-Flicker BitBlt)\n"
            L"✓ Subclaseo desacoplado de pestañas ShellTabWindowClass\n"
            L"✓ Atajo global interactivo registrado [ Ctrl + K ]\n"
            L"✓ Preparado para inyección de Vistas: Texto, Imagen, Cómic y Video";
        RECT rectListaCheck{rectPildora.left + 14, rectPildora.top + 48, rectPildora.right - 14, rectPildora.bottom - 10};
        DrawTextW(hdc, textoExplicativo.c_str(), -1, &rectListaCheck, DT_WORDBREAK | DT_NOPREFIX);

        // Limpieza de fuentes GDI
        SelectObject(hdc, fuenteVieja);
        DeleteObject(fuenteTitulo);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteGrande);
    }

private:
    std::vector<std::wstring> m_archivos;
    std::wstring m_carpeta;
};

} // namespace SideViewV2
