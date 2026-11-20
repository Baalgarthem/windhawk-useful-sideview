#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/nucleo/configuracion_v2.hpp"
#include "v2/nucleo/enrutador_vistas.hpp"
#include "v2/nucleo/servicios_vistas.hpp"
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <memory>

namespace SideViewV2 {

inline void DibujarBotonMediaV2(HDC hdc, const RECT& rc, const wchar_t* texto,
                                const TemaVisual& tema, bool activo = true) {
    HBRUSH fondo = CreateSolidBrush(activo ? tema.botonFondo : tema.fondoCabecera);
    FillRect(hdc, &rc, fondo); DeleteObject(fondo);
    SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, activo ? tema.textoPrincipal : tema.textoSecundario);
    RECT textoRect = rc;
    DrawTextW(hdc, texto, -1, &textoRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

inline std::wstring TamanoMediaV2(const std::wstring& ruta) {
    WIN32_FILE_ATTRIBUTE_DATA datos{};
    if (!GetFileAttributesExW(ruta.c_str(), GetFileExInfoStandard, &datos)) return L"Tamaño no disponible";
    ULONGLONG bytes = (static_cast<ULONGLONG>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
    if (bytes < 1024) return std::to_wstring(bytes) + L" B";
    if (bytes < 1024 * 1024) return std::to_wstring(bytes / 1024) + L" KB";
    if (bytes < 1024ULL * 1024 * 1024) return std::to_wstring(bytes / (1024 * 1024)) + L" MB";
    return std::to_wstring(bytes / (1024ULL * 1024 * 1024)) + L" GB";
}

class VistaMediaV2 : public IPanelContenido {
protected:
    VistaMediaV2(std::shared_ptr<ServicioNavegacionV2> navegacion,
                 std::shared_ptr<ServicioAbrirConV2> abrirCon,
                 std::shared_ptr<ServicioPrevisualizacionMediaV2> previsualizacion)
        : m_navegacion(std::move(navegacion)), m_abrirCon(std::move(abrirCon)),
          m_previsualizacion(std::move(previsualizacion)) {}

    bool ProcesarNavegacion(UINT msg, WPARAM wParam, LPARAM lParam, HWND hwnd) {
        if (msg == WM_KEYDOWN && (wParam == VK_LEFT || wParam == VK_RIGHT)) {
            CambiarIndice(wParam == VK_LEFT ? -1 : 1); InvalidateRect(hwnd, nullptr, FALSE); return true;
        }
        if (msg != WM_LBUTTONDOWN) return false;
        POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (PtInRect(&m_rcAnterior, pt)) { CambiarIndice(-1); InvalidateRect(hwnd, nullptr, FALSE); return true; }
        if (PtInRect(&m_rcSiguiente, pt)) { CambiarIndice(1); InvalidateRect(hwnd, nullptr, FALSE); return true; }
        if (PtInRect(&m_rcAbrirCon, pt)) {
            if (m_archivos.empty()) return true;
            if (m_abrirCon->ProgramaActual().empty()) m_abrirCon->AbrirDialogo(hwnd);
            m_abrirCon->Ejecutar({m_archivos[m_indice]}); return true;
        }
        if (MostrarComparador() && PtInRect(&m_rcComparar, pt)) {
            if (PuedeCompararSeleccionV2(m_archivos)) {
                PostMessageW(hwnd, WM_SIDEVIEW_REQUEST_COMPARADOR_ARCHIVOS, 0, 0);
            }
            return true;
        }
        return false;
    }

    void DibujarCabecera(HDC hdc, const RECT& rect, const TemaVisual& tema,
                         const wchar_t* titulo, const std::wstring& nombre) {
        SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, tema.textoPrincipal);
        RECT rTitulo{rect.left + 16, rect.top + 10, rect.right - 16, rect.top + 36};
        DrawTextW(hdc, titulo, -1, &rTitulo, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        SetTextColor(hdc, tema.textoSecundario);
        RECT rNombre{rect.left + 16, rect.top + 38, rect.right - 16, rect.top + 60};
        std::wstring detalle = nombre;
        if (!m_archivos.empty()) detalle += L"  ·  " + TamanoMediaV2(m_archivos[m_indice]);
        DrawTextW(hdc, detalle.c_str(), -1, &rNombre, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        int y = rect.top + 72;
        m_rcAnterior = {rect.left + 16, y, rect.left + 52, y + 30};
        m_rcSiguiente = {rect.left + 58, y, rect.left + 94, y + 30};
        m_rcComparar = {rect.right - 250, y, rect.right - 148, y + 30};
        m_rcAbrirCon = {rect.right - 140, y, rect.right - 16, y + 30};
        DibujarBotonMediaV2(hdc, m_rcAnterior, L"◀", tema, m_archivos.size() > 1);
        DibujarBotonMediaV2(hdc, m_rcSiguiente, L"▶", tema, m_archivos.size() > 1);
        if (MostrarComparador()) {
            DibujarBotonMediaV2(hdc, m_rcComparar, L"Comparar", tema,
                                PuedeCompararSeleccionV2(m_archivos));
        }
        DibujarBotonMediaV2(hdc, m_rcAbrirCon, L"Abrir con", tema);
    }

    void DibujarPreview(HDC hdc, const RECT& rect, const TemaVisual& tema, HBITMAP bitmap) {
        RECT contenedor{rect.left + 16, rect.top + 118, rect.right - 16, rect.bottom - 16};
        HBRUSH fondo = CreateSolidBrush(tema.fondoTarjeta); FillRect(hdc, &contenedor, fondo); DeleteObject(fondo);
        if (!bitmap) { SetTextColor(hdc, tema.textoSecundario); DrawTextW(hdc, L"Vista previa no disponible", -1, &contenedor, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX); return; }
        BITMAP bm{};
        if (GetObjectW(bitmap, sizeof(bm), &bm) != sizeof(bm) || bm.bmWidth <= 0 || bm.bmHeight == 0) {
            SetTextColor(hdc, tema.textoSecundario);
            DrawTextW(hdc, L"Vista previa no disponible", -1, &contenedor,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            return;
        }
        int w = contenedor.right - contenedor.left - 24, h = contenedor.bottom - contenedor.top - 24;
        int pw = w, ph = MulDiv(static_cast<int>(bm.bmHeight), w, std::max(1, static_cast<int>(bm.bmWidth)));
        if (ph > h) { ph = h; pw = MulDiv(static_cast<int>(bm.bmWidth), h, std::max(1, static_cast<int>(bm.bmHeight))); }
        HDC mem = CreateCompatibleDC(hdc); if (!mem) return; HGDIOBJ old = SelectObject(mem, bitmap); if (!old || old == HGDI_ERROR) { DeleteDC(mem); return; }
        int modoAnterior = SetStretchBltMode(hdc, HALFTONE);
        SetBrushOrgEx(hdc, 0, 0, nullptr);
        StretchBlt(hdc, contenedor.left + (w - pw) / 2 + 12, contenedor.top + (h - ph) / 2 + 12, pw, ph, mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        SetStretchBltMode(hdc, modoAnterior);
        SelectObject(mem, old); DeleteDC(mem);
    }

    void CambiarIndice(int delta) {
        if (m_archivos.size() < 2) return;
        m_indice = delta < 0 ? m_navegacion->Anterior(m_indice, m_archivos.size()) : m_navegacion->Siguiente(m_indice, m_archivos.size());
        CargarActual();
    }

    void ReemplazarPreview(HBITMAP nuevo) {
        if (m_preview && m_preview != nuevo) DeleteObject(m_preview);
        m_preview = nuevo;
    }

    void Liberar() override {
        if (m_preview) DeleteObject(m_preview);
        m_preview = nullptr;
        m_archivos.clear();
        m_indice = 0;
        m_rcAnterior = {};
        m_rcSiguiente = {};
        m_rcAbrirCon = {};
        m_rcComparar = {};
    }

    virtual void CargarActual() = 0;
    virtual bool MostrarComparador() const { return true; }
    std::vector<std::wstring> m_archivos; size_t m_indice{0}; HBITMAP m_preview{nullptr};
    std::shared_ptr<ServicioNavegacionV2> m_navegacion;
    std::shared_ptr<ServicioAbrirConV2> m_abrirCon;
    std::shared_ptr<ServicioPrevisualizacionMediaV2> m_previsualizacion;
    RECT m_rcAnterior{}, m_rcSiguiente{}, m_rcAbrirCon{}, m_rcComparar{};
};

class VistaImagenes final : public VistaMediaV2 {
public:
    VistaImagenes(std::shared_ptr<ServicioNavegacionV2> n, std::shared_ptr<ServicioAbrirConV2> a, std::shared_ptr<ServicioPrevisualizacionMediaV2> p) : VistaMediaV2(std::move(n), std::move(a), std::move(p)) {}
    ~VistaImagenes() override { Liberar(); }
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override { Liberar(); m_archivos = archivos; m_indice = 0; CargarActual(); }
    bool ProcesarMensajeRaton(UINT m, WPARAM w, LPARAM l, HWND h) override {
        if (m == WM_LBUTTONDOWN) {
            POINT punto{GET_X_LPARAM(l), GET_Y_LPARAM(l)};
            if (PtInRect(&m_rcPantallaCompleta, punto)) {
                PostMessageW(h, WM_SIDEVIEW_TOGGLE_PANEL_FULLSCREEN, 0, 0);
                return true;
            }
        }
        return ProcesarNavegacion(m, w, l, h);
    }
    bool ProcesarMensajeTeclado(UINT m, WPARAM w, LPARAM l, HWND h) override { return ProcesarNavegacion(m, w, l, h); }
    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        HBRUSH b = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, b);
        DeleteObject(b);
        const std::wstring nombre = m_archivos.empty()
            ? L"Sin imágenes" : PathFindFileNameW(m_archivos[m_indice].c_str());
        DibujarCabecera(hdc, rect, tema, L"🖼 Imágenes", nombre);
        m_rcPantallaCompleta = {m_rcSiguiente.right + 6, m_rcSiguiente.top,
                               m_rcSiguiente.right + 42, m_rcSiguiente.bottom};
        DibujarBotonMediaV2(hdc, m_rcPantallaCompleta, L"⛶", tema, true);
        DibujarPreview(hdc, rect, tema, m_preview);
    }
    void Liberar() override {
        m_rcPantallaCompleta = {};
        VistaMediaV2::Liberar();
    }
private:
    void CargarActual() override {
        ReemplazarPreview(m_archivos.empty() || !m_previsualizacion
            ? nullptr : m_previsualizacion->CargarImagen(m_archivos[m_indice]));
    }
    RECT m_rcPantallaCompleta{};
};

class VistaVideos final : public VistaMediaV2 {
public:
    VistaVideos(std::shared_ptr<ServicioNavegacionV2> n,
                std::shared_ptr<ServicioAbrirConV2> a,
                std::shared_ptr<ServicioPrevisualizacionMediaV2> p,
                std::shared_ptr<ServicioReproductorVideoV2> r)
        : VistaMediaV2(std::move(n), std::move(a), std::move(p)),
          m_servicioReproductor(std::move(r)) {}
    ~VistaVideos() override { Liberar(); }
    void AlActivar(HWND panel) override {
        m_panel = panel;
        CargarActual();
    }
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        HWND panel = m_panel;
        const bool panelPantallaCompleta = m_panelPantallaCompleta;
        Liberar();
        m_panel = panel;
        m_panelPantallaCompleta = panelPantallaCompleta;
        m_archivos = archivos;
        m_indice = 0;
        CargarActual();
    }
    void ActualizarConfiguracion() override {
        const bool debeUsarReproductor = DebeUsarReproductor();
        if (debeUsarReproductor == (m_reproductor != nullptr)) return;
        DesmontarReproductor();
        ReemplazarPreview(nullptr);
        CargarActual();
    }
    void AlCambiarPantallaCompletaPanel(bool pantallaCompleta) override {
        if (m_panelPantallaCompleta == pantallaCompleta) return;
        m_panelPantallaCompleta = pantallaCompleta;
        const bool debeUsarReproductor = DebeUsarReproductor();
        if (debeUsarReproductor == (m_reproductor != nullptr)) return;
        DesmontarReproductor();
        ReemplazarPreview(nullptr);
        CargarActual();
    }
    bool ProcesarMensajeRaton(UINT m, WPARAM w, LPARAM l, HWND h) override {
        if (m == WM_LBUTTONDOWN) {
            POINT punto{GET_X_LPARAM(l), GET_Y_LPARAM(l)};
            if (PtInRect(&m_rcPantallaCompleta, punto)) {
                PostMessageW(h, WM_SIDEVIEW_TOGGLE_PANEL_FULLSCREEN, 0, 0);
                return true;
            }
        }
        return ProcesarNavegacion(m, w, l, h);
    }
    bool ProcesarMensajeTeclado(UINT m, WPARAM w, LPARAM l, HWND h) override { return ProcesarNavegacion(m, w, l, h); }
    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        HBRUSH b = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, b);
        DeleteObject(b);

        const std::wstring nombre = m_archivos.empty()
            ? L"Sin videos" : PathFindFileNameW(m_archivos[m_indice].c_str());
        DibujarCabecera(
            hdc, rect, tema,
            DebeUsarReproductor()
                ? L"🎬 Videos · Reproductor"
                : L"🎬 Videos · Vista previa",
            nombre);

        m_rcPantallaCompleta = {m_rcSiguiente.right + 6, m_rcSiguiente.top,
                                m_rcSiguiente.right + 42, m_rcSiguiente.bottom};
        DibujarBotonMediaV2(hdc, m_rcPantallaCompleta, L"⛶", tema, true);

        const RECT contenedor{rect.left + 16, rect.top + 118,
                              rect.right - 16, rect.bottom - 16};
        if (m_reproductor) {
            // El reproductor reserva internamente la franja superior del
            // marco para que el título permanezca dibujado por la vista.
            RECT ranura = contenedor;
            ranura.top -= 30;
            m_reproductor->ActualizarRect(ranura, false);
        } else {
            DibujarPreview(hdc, rect, tema, m_preview);
        }
    }

    void Liberar() override {
        DesmontarReproductor();
        VistaMediaV2::Liberar();
        m_panel = nullptr;
        m_panelPantallaCompleta = false;
        m_rcPantallaCompleta = {};
    }

private:
    void CargarActual() override {
        if (m_archivos.empty() || m_indice >= m_archivos.size()) {
            DesmontarReproductor();
            ReemplazarPreview(nullptr);
            return;
        }

        if (DebeUsarReproductor() && MontarReproductorActual()) {
            ReemplazarPreview(nullptr);
            return;
        }

        DesmontarReproductor();
        ReemplazarPreview(m_previsualizacion
            ? m_previsualizacion->CargarMiniatura(m_archivos[m_indice])
            : nullptr);
    }

    bool MontarReproductorActual() {
        if (!m_panel || !m_servicioReproductor) return false;
        DesmontarReproductor();
        std::unique_ptr<IReproductorVideoV2> reproductor =
            m_servicioReproductor->Crear();
        if (!reproductor || !reproductor->Montar(m_panel, 0, m_archivos[m_indice])) {
            return false;
        }
        m_reproductor = std::move(reproductor);
        return true;
    }

    bool DebeUsarReproductor() const {
        return ReproductorVideoEnVistaDisponibleV2() || m_panelPantallaCompleta;
    }

    void DesmontarReproductor() {
        if (m_reproductor) {
            m_reproductor->Liberar();
            m_reproductor.reset();
        }
    }

    HWND m_panel{nullptr};
    bool m_panelPantallaCompleta{false};
    RECT m_rcPantallaCompleta{};
    std::shared_ptr<ServicioReproductorVideoV2> m_servicioReproductor;
    std::unique_ptr<IReproductorVideoV2> m_reproductor;
};
}
