#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/ui/ventana_base.hpp"
#include "v2/nucleo/enrutador_vistas.hpp"
#include "v2/nucleo/servicios_vistas.hpp"
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>

namespace SideViewV2 {

class VistaComprimidos : public IPanelContenido {
public:
    explicit VistaComprimidos(std::shared_ptr<ServicioAbrirConV2> servicio,
                              std::shared_ptr<ServicioNavegacionV2> navegacion,
                              std::shared_ptr<ServicioExtensionV2> extensiones,
                              std::shared_ptr<ServicioContenidoComprimidoV2> contenido)
        : m_servicio(std::move(servicio)), m_navegacion(std::move(navegacion)),
          m_extensiones(std::move(extensiones)), m_contenido(std::move(contenido)) {
        m_formatosDisponibles = { L"zip", L"rar", L"7z", L"tar", L"cbz", L"cbr" };
        m_formatoSeleccionado = L"zip";
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override {
        m_archivos = archivos;
        m_indiceArchivo = 0;
        m_desplazamientoScrollY = 0;
        CargarArchivoActual();
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwndPanel) override {
        POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

        if (msg == WM_MOUSEWHEEL) {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int incremento = (delta > 0 ? -40 : 40);
            int maxScroll = std::max(0, static_cast<int>(m_entradas.size() * 26) - 200);
            m_desplazamientoScrollY = std::max(0, std::min(m_desplazamientoScrollY + incremento, maxScroll));
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }

        if (msg == WM_LBUTTONDOWN) {
            // Botón Anterior
            if (PtInRect(&m_rcBotonAnterior, pt)) {
                if (m_archivos.size() > 1) {
                    m_indiceArchivo = m_navegacion->Anterior(m_indiceArchivo, m_archivos.size());
                    CargarArchivoActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Siguiente
            if (PtInRect(&m_rcBotonSiguiente, pt)) {
                if (m_archivos.size() > 1) {
                    m_indiceArchivo = m_navegacion->Siguiente(m_indiceArchivo, m_archivos.size());
                    CargarArchivoActual();
                    InvalidateRect(hwndPanel, nullptr, FALSE);
                    return true;
                }
            }
            // Botón Abrir Con
            if (PtInRect(&m_rcBotonAbrirCon, pt)) {
                EjecutarAbrirCon(hwndPanel);
                return true;
            }
            if (PtInRect(&m_rcBotonComparar, pt)) {
                if (PuedeCompararSeleccionV2(m_archivos)) {
                    PostMessageW(hwndPanel, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
                }
                return true;
            }
            // Botón Desplegable Formatos
            if (PtInRect(&m_rcBotonDesplegableFormato, pt)) {
                MostrarMenuFormatos(hwndPanel);
                return true;
            }
            // Botón Cambiar extensión
            if (PtInRect(&m_rcBotonCambiarExtension, pt)) {
                EjecutarConversion(hwndPanel);
                return true;
            }
        }

        return false;
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
        HFONT fuenteBold = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
        HFONT fuenteVieja = static_cast<HFONT>(SelectObject(hdc, fuenteTitulo));

        // Título de vista
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring nombreArchivoActual = L"Sin archivos comprimidos";
        if (!m_archivos.empty() && m_indiceArchivo < m_archivos.size()) {
            const wchar_t* pName = PathFindFileNameW(m_archivos[m_indiceArchivo].c_str());
            nombreArchivoActual = pName ? pName : m_archivos[m_indiceArchivo];
        }

        RECT rectTextoTitulo{rect.left + 16, rect.top + 8, rect.right - 16, rect.top + 28};
        std::wstring strCabecera = L"📦 " + nombreArchivoActual;
        DrawTextW(hdc, strCabecera.c_str(), -1, &rectTextoTitulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Subtítulo con contador
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring strSub = L"Archivo " + std::to_wstring(m_archivos.empty() ? 0 : m_indiceArchivo + 1) +
                              L" de " + std::to_wstring(m_archivos.size()) + L" • Suite de Comprimidos & Cómics";
        RECT rectTextoSub{rect.left + 16, rect.top + 28, rect.right - 16, rect.top + 46};
        DrawTextW(hdc, strSub.c_str(), -1, &rectTextoSub,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        // Barra superior: navegación y conversión.
        int margen = 14;
        int yBarra = rect.top + altoCabecera + 12;
        int altoBotones = 30;

        // Botón Anterior
        m_rcBotonAnterior = {rect.left + margen, yBarra, rect.left + margen + 32, yBarra + altoBotones};
        DibujarBoton(hdc, m_rcBotonAnterior, L"◀", tema, m_archivos.size() > 1);

        // Botón Siguiente
        m_rcBotonSiguiente = {m_rcBotonAnterior.right + 6, yBarra, m_rcBotonAnterior.right + 38, yBarra + altoBotones};
        DibujarBoton(hdc, m_rcBotonSiguiente, L"▶", tema, m_archivos.size() > 1);

        // Selector de Formato (Drop)
        m_rcBotonDesplegableFormato = {m_rcBotonSiguiente.right + 10, yBarra, m_rcBotonSiguiente.right + 85, yBarra + altoBotones};
        DibujarBoton(hdc, m_rcBotonDesplegableFormato, (m_formatoSeleccionado + L" ▾").c_str(), tema, true);

        // Operación de cambio de extensión
        m_rcBotonCambiarExtension = {m_rcBotonDesplegableFormato.right + 6, yBarra, rect.right - margen, yBarra + altoBotones};
        DibujarBoton(hdc, m_rcBotonCambiarExtension, L"Cambiar extensión", tema, true);

        int yAbrir = rect.bottom - margen - altoBotones;
        m_rcBotonAbrirCon = {rect.left + margen, yAbrir, rect.right - margen, yAbrir + altoBotones};
        std::wstring programa = m_servicio->ProgramaActual();
        std::wstring etiqueta = L"Abrir con: ";
        etiqueta += programa.empty() ? L"(click para seleccionar programa) ▾" : PathFindFileNameW(programa.c_str());
        DibujarBoton(hdc, m_rcBotonAbrirCon, etiqueta.c_str(), tema, true);

        int yComparar = yAbrir - altoBotones - 8;
        m_rcBotonComparar = {rect.left + margen, yComparar,
                             rect.left + margen + 126, yComparar + altoBotones};
        DibujarBoton(hdc, m_rcBotonComparar, L"Comparar archivos", tema,
                     PuedeCompararSeleccionV2(m_archivos));

        // Área de Contenido: Árbol de archivos comprimidos
        int topLista = yBarra + altoBotones + 14;
        RECT rcContenedorLista{rect.left + margen, topLista, rect.right - margen, yComparar - 12};
        DibujarContenedor(hdc, rcContenedorLista, tema);

        // Header de la lista
        RECT rcHeaderLista{rcContenedorLista.left, rcContenedorLista.top, rcContenedorLista.right, rcContenedorLista.top + 28};
        HBRUSH brHeaderL = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &rcHeaderLista, brHeaderL);
        DeleteObject(brHeaderL);

        SelectObject(hdc, fuenteBold);
        SetTextColor(hdc, tema.acento);
        RECT rcColNombre{rcHeaderLista.left + 10, rcHeaderLista.top, rcHeaderLista.right - 90, rcHeaderLista.bottom};
        DrawTextW(hdc, L"Nombre del elemento", -1, &rcColNombre, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

        RECT rcColTamano{rcHeaderLista.right - 85, rcHeaderLista.top, rcHeaderLista.right - 10, rcHeaderLista.bottom};
        DrawTextW(hdc, L"Tamaño", -1, &rcColTamano, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOPREFIX);

        // Renderizado de las filas
        SelectObject(hdc, fuenteSubtitulo);
        SetTextColor(hdc, tema.textoPrincipal);

        int yFila = rcHeaderLista.bottom + 4 - m_desplazamientoScrollY;
        int altoFila = 24;

        for (const auto& entrada : m_entradas) {
            if (yFila + altoFila >= rcHeaderLista.bottom && yFila <= rcContenedorLista.bottom - 4) {
                std::wstring iconoItem = entrada.esDirectorio ? L"📁 " : L"📄 ";
                std::wstring textoFila = iconoItem + entrada.nombre;

                RECT rcFilaNom{rcContenedorLista.left + 10, yFila, rcContenedorLista.right - 90, yFila + altoFila};
                DrawTextW(hdc, textoFila.c_str(), -1, &rcFilaNom, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

                std::wstring strTam = entrada.esDirectorio ? L"-" : FormatearTamano(entrada.tamanoBytes);
                RECT rcFilaTam{rcContenedorLista.right - 85, yFila, rcContenedorLista.right - 10, yFila + altoFila};
                DrawTextW(hdc, strTam.c_str(), -1, &rcFilaTam, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOPREFIX);
            }
            yFila += altoFila;
        }

        if (m_entradas.empty()) {
            RECT rcVacio{rcContenedorLista.left, rcHeaderLista.bottom, rcContenedorLista.right, rcContenedorLista.bottom};
            SetTextColor(hdc, tema.textoSecundario);
            DrawTextW(hdc, L"(Estructura no indexada o cómic con visor de portada)", -1, &rcVacio,
                      DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        SelectObject(hdc, fuenteVieja);
        DeleteObject(fuenteTitulo);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteBold);
    }

private:
    void CargarArchivoActual() {
        m_entradas.clear();
        if (m_archivos.empty() || m_indiceArchivo >= m_archivos.size()) return;
        m_entradas = m_contenido->Cargar(m_archivos[m_indiceArchivo]);
    }

    void DibujarBoton(HDC hdc, const RECT& rc, const wchar_t* texto, const TemaVisual& tema, bool habilitado) {
        HBRUSH br = CreateSolidBrush(habilitado ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &rc, br);
        DeleteObject(br);

        HPEN pen = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH nullBr = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(hdc, nullBr));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        SetTextColor(hdc, habilitado ? tema.textoPrincipal : tema.textoSecundario);
        RECT rcTexto = rc;
        DrawTextW(hdc, texto, -1, &rcTexto, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
    }

    void DibujarContenedor(HDC hdc, const RECT& rc, const TemaVisual& tema) {
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

    std::wstring FormatearTamano(uintmax_t bytes) {
        if (bytes < 1024) return std::to_wstring(bytes) + L" B";
        if (bytes < 1024 * 1024) {
            wchar_t b[32];
            swprintf_s(b, L"%.1f KB", static_cast<double>(bytes) / 1024.0);
            return b;
        }
        wchar_t b[32];
        swprintf_s(b, L"%.1f MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
        return b;
    }

    void EjecutarAbrirCon(HWND hwndPadre) {
        std::wstring programa = m_servicio->ProgramaActual();
        if (!programa.empty()) {
            m_servicio->Seleccionar(programa);
            m_servicio->Ejecutar(m_archivos);
            return;
        }
        if (m_servicio->AbrirDialogo(hwndPadre)) {
            m_servicio->Ejecutar(m_archivos);
        }
    }

    void MostrarMenuFormatos(HWND hwndPadre) {
        HMENU hMenu = CreatePopupMenu();
        for (size_t i = 0; i < m_formatosDisponibles.size(); ++i) {
            UINT flags = MF_STRING;
            if (m_formatosDisponibles[i] == m_formatoSeleccionado) {
                flags |= MF_CHECKED;
            }
            AppendMenuW(hMenu, flags, i + 1, m_formatosDisponibles[i].c_str());
        }

        POINT ptMenu{m_rcBotonDesplegableFormato.left, m_rcBotonDesplegableFormato.bottom};
        ClientToScreen(hwndPadre, &ptMenu);
        int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, ptMenu.x, ptMenu.y, 0, hwndPadre, nullptr);
        DestroyMenu(hMenu);

        if (cmd > 0 && cmd <= static_cast<int>(m_formatosDisponibles.size())) {
            m_formatoSeleccionado = m_formatosDisponibles[static_cast<std::size_t>(cmd - 1)];
            InvalidateRect(hwndPadre, nullptr, FALSE);
        }
    }

    void EjecutarConversion(HWND hwndPadre) {
        size_t cantidad = m_extensiones->Cambiar(m_archivos, m_formatoSeleccionado);
        std::wstring mensaje = L"Se cambiaron " + std::to_wstring(cantidad) + L" archivo(s) a ." + m_formatoSeleccionado + L".";
        MessageBoxW(hwndPadre, mensaje.c_str(), L"Cambiar extensión", MB_OK | MB_ICONINFORMATION);
    }

    std::vector<std::wstring> m_archivos;
    size_t m_indiceArchivo{0};
    int m_desplazamientoScrollY{0};
    std::vector<EntradaComprimidoV2> m_entradas;

    std::wstring m_formatoSeleccionado;
    std::vector<std::wstring> m_formatosDisponibles;
    std::shared_ptr<ServicioAbrirConV2> m_servicio;
    std::shared_ptr<ServicioNavegacionV2> m_navegacion;
    std::shared_ptr<ServicioExtensionV2> m_extensiones;
    std::shared_ptr<ServicioContenidoComprimidoV2> m_contenido;

    RECT m_rcBotonAnterior{};
    RECT m_rcBotonSiguiente{};
    RECT m_rcBotonAbrirCon{};
    RECT m_rcBotonDesplegableFormato{};
    RECT m_rcBotonCambiarExtension{};
    RECT m_rcBotonComparar{};
};

} // namespace SideViewV2
