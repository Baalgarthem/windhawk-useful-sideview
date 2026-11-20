#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/nucleo/configuracion_v2.hpp"
#include "v2/nucleo/enrutador_vistas.hpp"
#include "v2/nucleo/servicio_texto.hpp"
#include "v2/nucleo/servicio_navegacion.hpp"
#include <windowsx.h>
#include <algorithm>
#include <memory>
#include <string>

namespace SideViewV2 {

class VistaTextoCodigo : public IPanelContenido {
public:
    VistaTextoCodigo(std::shared_ptr<ServicioTextoV2> texto,
                     std::shared_ptr<ServicioNavegacionV2> nav,
                     bool codigo)
        : m_texto(std::move(texto)), m_nav(std::move(nav)), m_codigo(codigo) {}

    void Liberar() override {
        m_documento = {};
        m_archivos.clear();
        m_indiceArchivo = 0;
        m_lineaInicial = 0;
        m_rcAnterior = {};
        m_rcSiguiente = {};
        m_rcComparar = {};
        m_rcCopiar = {};
        m_rcRenombrar = {};
        m_rcIndice = {};
    }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        Liberar();
        m_archivos = archivos;
        Cargar();
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM, LPARAM lParam,
                              HWND hwnd) override {
        if (msg != WM_LBUTTONDOWN) return false;
        POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

        if (PtInRect(&m_rcAnterior, punto)) {
            Cambiar(-1, hwnd);
            return true;
        }
        if (PtInRect(&m_rcSiguiente, punto)) {
            Cambiar(1, hwnd);
            return true;
        }
        if (PtInRect(&m_rcCopiar, punto)) {
            if (m_texto) m_texto->Copiar(Unir());
            return true;
        }
        if (PtInRect(&m_rcRenombrar, punto) && !m_documento.ruta.empty()) {
            if (m_texto && m_texto->Renombrar(hwnd, m_documento.ruta)) Cargar();
            return true;
        }
        if (PtInRect(&m_rcComparar, punto)) {
            if (PuedeCompararSeleccionV2(m_archivos)) {
                PostMessageW(hwnd, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
            }
            return true;
        }

        for (size_t i = 0; i < m_documento.indice.size(); ++i) {
            RECT elemento{
                m_rcIndice.left,
                m_rcIndice.top + static_cast<int>(i * kAltoEntradaIndice),
                m_rcIndice.right,
                m_rcIndice.top + static_cast<int>((i + 1) * kAltoEntradaIndice)
            };
            if (PtInRect(&elemento, punto)) {
                const size_t numeroLinea = m_documento.indice[i];
                if (numeroLinea < m_documento.lineas.size()) {
                    m_lineaInicial = numeroLinea;
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                return true;
            }
        }
        return PtInRect(&m_rcIndice, punto);
    }

    bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM,
                                HWND hwnd) override {
        if (msg != WM_KEYDOWN) return false;
        if (wParam == VK_LEFT) {
            Cambiar(-1, hwnd);
            return true;
        }
        if (wParam == VK_RIGHT) {
            Cambiar(1, hwnd);
            return true;
        }
        if (wParam == VK_HOME) {
            m_lineaInicial = 0;
            InvalidateRect(hwnd, nullptr, FALSE);
            return true;
        }
        return false;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        if (!hdc || rect.right <= rect.left || rect.bottom <= rect.top) return;

        HBRUSH fondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetBkMode(hdc, TRANSPARENT);

        const int tamanoFuente = m_codigo ? TamanoFuenteCodigoV2() : TamanoFuenteTextoV2();
        HFONT fuente = CrearFuenteDocumento(tamanoFuente, m_codigo);
        HGDIOBJ fuenteAnterior = nullptr;
        if (fuente) fuenteAnterior = SelectObject(hdc, fuente);

        const int top = 8;
        m_rcAnterior = {rect.left + 8, rect.top + top,
                        rect.left + 72, rect.top + top + 28};
        m_rcSiguiente = {rect.left + 78, rect.top + top,
                         rect.left + 142, rect.top + top + 28};
        m_rcComparar = {rect.right - 300, rect.top + top,
                        rect.right - 178, rect.top + top + 28};
        m_rcCopiar = {rect.right - 170, rect.top + top,
                      rect.right - 88, rect.top + top + 28};
        m_rcRenombrar = {rect.right - 82, rect.top + top,
                         rect.right - 8, rect.top + top + 28};

        DibujarBoton(hdc, m_rcAnterior, L"◀", tema);
        DibujarBoton(hdc, m_rcSiguiente, L"▶", tema);
        DibujarBoton(hdc, m_rcComparar, L"Comparar", tema,
                     PuedeCompararSeleccionV2(m_archivos));
        DibujarBoton(hdc, m_rcCopiar, L"Copiar", tema);
        DibujarBoton(hdc, m_rcRenombrar, L"Renombrar", tema);

        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{rect.left + 154, rect.top + top,
                    rect.right - 180, rect.top + top + 28};
        std::wstring nombre = m_documento.ruta;
        const size_t separador = nombre.find_last_of(L"\\/");
        if (separador != std::wstring::npos) nombre = nombre.substr(separador + 1);
        DrawTextW(hdc, nombre.c_str(), -1, &titulo,
                  DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        m_rcIndice = {rect.left + 8, rect.top + 48,
                      rect.left + 190, rect.bottom - 8};
        HBRUSH fondoIndice = CreateSolidBrush(tema.fondoCabecera);
        FillRect(hdc, &m_rcIndice, fondoIndice);
        DeleteObject(fondoIndice);
        DibujarIndice(hdc, tema);

        const bool mostrarNumeros = g_mostrarNumerosLineaV2.load(std::memory_order_acquire);
        const bool ajustarLinea = AjusteLineaTextoCodigoDisponibleV2();
        const int xContenido = rect.left + 202 + (mostrarNumeros ? 52 : 4);
        const int anchoContenido = std::max(
            1, static_cast<int>(rect.right - 8) - xContenido);
        const int altoBase = std::max(16, tamanoFuente + 7);
        int y = rect.top + 54;

        SetTextColor(hdc, tema.textoPrincipal);
        for (size_t i = m_lineaInicial;
             i < m_documento.lineas.size() && y < rect.bottom - 8; ++i) {
            const std::wstring& linea = m_documento.lineas[i];
            const int altoDisponible = rect.bottom - 8 - y;
            if (altoDisponible <= 0) break;

            int altoLinea = altoBase;
            if (ajustarLinea) {
                RECT calculo{xContenido, 0, xContenido + anchoContenido, 0};
                const int altoCalculado = DrawTextW(
                    hdc, linea.c_str(), -1, &calculo,
                    DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
                if (altoCalculado > 0) {
                    altoLinea = std::max(
                        altoBase, static_cast<int>(calculo.bottom - calculo.top));
                }
            }
            altoLinea = std::min(altoLinea, altoDisponible);
            DibujarLinea(hdc, linea, tema, rect, xContenido, y,
                         anchoContenido, altoLinea, mostrarNumeros, i,
                         ajustarLinea,
                         m_codigo && EsClave(linea) ? RGB(180, 100, 220)
                                                    : ColorLinea(linea, tema));
            y += altoLinea;
        }

        if (fuente) {
            if (fuenteAnterior && fuenteAnterior != HGDI_ERROR) {
                SelectObject(hdc, fuenteAnterior);
            }
            DeleteObject(fuente);
        }
    }

private:
    static constexpr int kAltoEntradaIndice = 22;

    void Cargar() {
        if (!m_texto || m_archivos.empty() || m_indiceArchivo >= m_archivos.size()) {
            m_documento = {};
            m_lineaInicial = 0;
            return;
        }
        m_documento = m_texto->Cargar(m_archivos[m_indiceArchivo]);
        m_lineaInicial = 0;
    }

    void Cambiar(int delta, HWND hwnd) {
        if (m_archivos.size() < 2 || !m_nav) return;
        m_indiceArchivo = delta < 0
            ? m_nav->Anterior(m_indiceArchivo, m_archivos.size())
            : m_nav->Siguiente(m_indiceArchivo, m_archivos.size());
        Cargar();
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    std::wstring Unir() const {
        size_t capacidad = 0;
        for (const auto& linea : m_documento.lineas) capacidad += linea.size() + 2;
        std::wstring resultado;
        resultado.reserve(capacidad);
        for (const auto& linea : m_documento.lineas) {
            resultado += linea;
            resultado += L"\r\n";
        }
        return resultado;
    }

    void DibujarIndice(HDC hdc, const TemaVisual& tema) const {
        SetTextColor(hdc, tema.textoSecundario);
        const size_t filasDisponibles = m_rcIndice.bottom > m_rcIndice.top
            ? static_cast<size_t>((m_rcIndice.bottom - m_rcIndice.top) /
                                   kAltoEntradaIndice)
            : 0;
        const size_t entradas = std::min(filasDisponibles, m_documento.indice.size());
        for (size_t i = 0; i < entradas; ++i) {
            const size_t numeroLinea = m_documento.indice[i];
            if (numeroLinea >= m_documento.lineas.size()) continue;
            RECT elemento{
                m_rcIndice.left + 8,
                m_rcIndice.top + static_cast<int>(i * kAltoEntradaIndice),
                m_rcIndice.right - 4,
                m_rcIndice.top + static_cast<int>((i + 1) * kAltoEntradaIndice)
            };
            DrawTextW(hdc, m_documento.lineas[numeroLinea].c_str(), -1, &elemento,
                      DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
    }

    static COLORREF ColorLinea(const std::wstring& linea, const TemaVisual& tema) {
        const bool encabezado = !linea.empty() && linea[0] == L'#';
        const bool lista = linea.size() > 1 &&
            (linea[0] == L'-' || linea[0] == L'*' || linea[0] == L'>');
        if (encabezado) return RGB(70, 150, 230);
        if (lista) return RGB(90, 180, 120);
        return tema.textoPrincipal;
    }

    static bool EsClave(const std::wstring& linea) {
        static const wchar_t* palabrasClave[] = {
            L"class ", L"struct ", L"void ", L"int ", L"return ",
            L"#include", L"function ", L"const ", L"public:", L"private:"
        };
        for (const wchar_t* palabra : palabrasClave) {
            if (linea.find(palabra) != std::wstring::npos) return true;
        }
        return false;
    }

    static HFONT CrearFuenteDocumento(int tamano, bool codigo) {
        return CreateFontW(
            -tamano, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            codigo ? L"Consolas" : L"Segoe UI");
    }

    void DibujarLinea(HDC hdc, const std::wstring& linea,
                      const TemaVisual& tema, const RECT& rect,
                      int x, int y, int ancho, int alto,
                      bool mostrarNumeros, size_t numeroLinea,
                      bool ajustarLinea, COLORREF color) const {
        if (mostrarNumeros) {
            SetTextColor(hdc, tema.textoSecundario);
            wchar_t numero[32]{};
            swprintf_s(numero, L"%5zu", numeroLinea + 1);
            TextOutW(hdc, rect.left + 202, y, numero,
                     static_cast<int>(wcslen(numero)));
        }

        SetTextColor(hdc, color);
        RECT destino{x, y, x + ancho, y + alto};
        const UINT flags = ajustarLinea
            ? DT_WORDBREAK | DT_NOPREFIX
            : DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;
        DrawTextW(hdc, linea.c_str(), -1, &destino, flags);
    }

    void DibujarBoton(HDC hdc, const RECT& rect, const wchar_t* texto,
                      const TemaVisual& tema, bool habilitado = true) const {
        HBRUSH fondo = CreateSolidBrush(habilitado ? tema.botonFondo : tema.fondoCabecera);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetTextColor(hdc, habilitado ? tema.textoPrincipal : tema.textoSecundario);
        RECT textoRect = rect;
        DrawTextW(hdc, texto, -1, &textoRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    std::shared_ptr<ServicioTextoV2> m_texto;
    std::shared_ptr<ServicioNavegacionV2> m_nav;
    bool m_codigo{};
    std::vector<std::wstring> m_archivos;
    size_t m_indiceArchivo{};
    size_t m_lineaInicial{};
    DocumentoTextoV2 m_documento;
    RECT m_rcAnterior{}, m_rcSiguiente{}, m_rcComparar{}, m_rcCopiar{},
        m_rcRenombrar{}, m_rcIndice{};
};

} // namespace SideViewV2
