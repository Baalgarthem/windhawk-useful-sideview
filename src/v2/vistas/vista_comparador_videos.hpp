#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/nucleo/configuracion_v2.hpp"
#include "v2/nucleo/servicio_comparador_videos.hpp"
#include "v2/vistas/vista_comparador_archivos.hpp"
#include <windows.h>
#include <windowsx.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

class VistaComparadorVideos final : public VistaComparadorArchivosV2 {
public:
    explicit VistaComparadorVideos(std::shared_ptr<ServicioComparadorVideosV2> servicio)
        : VistaComparadorArchivosV2(servicio), m_servicio(std::move(servicio)) {}

    ~VistaComparadorVideos() override { Liberar(); }

    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos,
                            const std::wstring&) override {
        CargarComparacionComun(archivos);
        LiberarReproductores();
        m_estado = m_servicio ? m_servicio->CrearFlujo(archivos)
                               : EstadoComparadorVideosV2{};
        m_arrastrandoTiempo = false;
        m_hoverVideo = 0;
        if (m_panel) MontarReproductores();
    }

    void AlActivar(HWND panel) override {
        m_panel = panel;
        MontarReproductores();
    }

    void ActualizarConfiguracion() override {
        if (!ComparadorVideosPantallaCompletaDisponibleV2()) {
            m_estado.modoPantallaCompleta = false;
        }
    }

    void Liberar() override {
        if (m_panel && GetCapture() == m_panel) ReleaseCapture();
        LiberarReproductores();
        m_estado = {};
        m_arrastrandoTiempo = false;
        m_hoverVideo = 0;
        m_panel = nullptr;
        VistaComparadorArchivosV2::Liberar();
    }

    bool ProcesarMensajeVentana(UINT msg, WPARAM wParam, LPARAM,
                                HWND hwndPanel) override {
        if (msg == WM_SIDEVIEW_COMPARADOR_TICK) {
            ActualizarEstadoDesdeReproductores();
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (msg == WM_SIDEVIEW_COMPARADOR_PIP_CLICK &&
            m_estado.modoPantallaCompleta &&
            static_cast<int>(wParam) != m_estado.indicePantallaCompleta) {
            // El propio reproductor informa que su superficie es el PiP. El
            // swap sólo cambia geometría/rol; nunca pausa ni busca el video.
            if (m_servicio) m_servicio->IntercambiarPantallaCompleta(m_estado);
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        return false;
    }

    bool ProcesarMensajeRaton(UINT msg, WPARAM, LPARAM lParam,
                              HWND hwndPanel) override {
        POINT punto{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

        if (msg == WM_MOUSEMOVE) {
            int hoverNuevo = 0;
            if (PtInRect(&m_rcBotonFullIzquierda, punto)) hoverNuevo = 1;
            else if (PtInRect(&m_rcBotonFullDerecha, punto)) hoverNuevo = 2;
            else if (PtInRect(&m_rcVideoIzquierda, punto)) hoverNuevo = 3;
            else if (PtInRect(&m_rcVideoDerecha, punto)) hoverNuevo = 4;
            if (hoverNuevo != m_hoverVideo) {
                m_hoverVideo = hoverNuevo;
                InvalidateRect(hwndPanel, nullptr, FALSE);
            }
            if (m_arrastrandoTiempo && m_servicio) {
                m_servicio->BuscarAmbos(m_estado, PosicionDesdePunto(punto),
                                        m_reproductorIzquierda.get(),
                                        m_reproductorDerecha.get());
                InvalidateRect(hwndPanel, nullptr, FALSE);
            }
            return m_arrastrandoTiempo || hoverNuevo != 0;
        }

        if (msg == WM_LBUTTONUP) {
            m_arrastrandoTiempo = false;
            ReleaseCapture();
            return true;
        }

        if (msg != WM_LBUTTONDOWN) return false;
        SetFocus(hwndPanel);

        if (PtInRect(&m_rcVolver, punto)) {
            PostMessageW(hwndPanel, WM_SIDEVIEW_CLOSE_COMPARADOR_ARCHIVOS, 0, 0);
            return true;
        }
        if (ComparadorVideosPantallaCompletaDisponibleV2()) {
            if (PtInRect(&m_rcBotonFullIzquierda, punto)) {
                if (m_servicio) m_servicio->AlternarPantallaCompleta(m_estado, 0);
                InvalidateRect(hwndPanel, nullptr, FALSE);
                return true;
            }
            if (PtInRect(&m_rcBotonFullDerecha, punto)) {
                if (m_servicio) m_servicio->AlternarPantallaCompleta(m_estado, 1);
                InvalidateRect(hwndPanel, nullptr, FALSE);
                return true;
            }
        }
        if (PtInRect(&m_rcLineaTiempo, punto)) {
            m_arrastrandoTiempo = true;
            SetCapture(hwndPanel);
            if (m_servicio) m_servicio->BuscarAmbos(
                m_estado, PosicionDesdePunto(punto), m_reproductorIzquierda.get(),
                m_reproductorDerecha.get());
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (PtInRect(&m_rcReproducir, punto)) {
            if (m_servicio) m_servicio->AlternarEstadoReproduccion(
                m_estado, m_reproductorIzquierda.get(), m_reproductorDerecha.get());
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (PtInRect(&m_rcRetroceder, punto)) {
            BuscarRelativo(-10.0);
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (PtInRect(&m_rcAvanzar, punto)) {
            BuscarRelativo(10.0);
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        if (PtInRect(&m_rcConfirmarSincronizacion, punto)) {
            if (m_servicio) {
                m_servicio->ActualizarDuraciones(
                    m_estado,
                    m_reproductorIzquierda ? m_reproductorIzquierda->Duracion() : 0.0,
                    m_reproductorDerecha ? m_reproductorDerecha->Duracion() : 0.0);
                m_servicio->ConfirmarSincronizacion(m_estado);
            }
            InvalidateRect(hwndPanel, nullptr, FALSE);
            return true;
        }
        return false;
    }

    bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM,
                                HWND hwndPanel) override {
        if (msg != WM_KEYDOWN || !m_servicio) return false;
        switch (wParam) {
            case VK_SPACE:
                m_servicio->AlternarEstadoReproduccion(
                    m_estado, m_reproductorIzquierda.get(), m_reproductorDerecha.get());
                break;
            case VK_LEFT:
                BuscarRelativo(-10.0);
                break;
            case VK_RIGHT:
                BuscarRelativo(10.0);
                break;
            default:
                return false;
        }
        InvalidateRect(hwndPanel, nullptr, FALSE);
        return true;
    }

    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        HBRUSH fondo = CreateSolidBrush(tema.fondo);
        FillRect(hdc, &rect, fondo);
        DeleteObject(fondo);
        SetBkMode(hdc, TRANSPARENT);

        HFONT titulo = CrearFuente(-18, FW_SEMIBOLD);
        HFONT texto = CrearFuente(-13, FW_NORMAL);
        HFONT negrita = CrearFuente(-13, FW_BOLD);
        HFONT anterior = static_cast<HFONT>(SelectObject(hdc, titulo));

        const int margen = 16;
        const int espacio = 12;
        const int anchoCliente = static_cast<int>(rect.right - rect.left);
        const int altoCliente = static_cast<int>(rect.bottom - rect.top);
        const int ancho = std::max(1, anchoCliente);
        const int anchoVideo = std::max(1, (ancho - margen * 2 - espacio) / 2);
        const int altoVideo = std::max(220, std::min(520, altoCliente / 2));
        const int yVideo = rect.top + 52;

        m_rcVolver = {rect.left + margen, rect.top + 12,
                      rect.left + 124, rect.top + 40};
        DibujarBoton(hdc, m_rcVolver, L"←  Volver", tema, true);

        if (m_estado.modoPantallaCompleta &&
            ComparadorVideosPantallaCompletaDisponibleV2()) {
            ConstruirLayoutPantallaCompleta(hdc, rect, margen, yVideo, altoVideo, tema);
        } else {
            m_estado.modoPantallaCompleta = false;
            m_rcVideoIzquierda = {rect.left + margen, yVideo,
                                  rect.left + margen + anchoVideo, yVideo + altoVideo};
            m_rcVideoDerecha = {m_rcVideoIzquierda.right + espacio, yVideo,
                                rect.right - margen, yVideo + altoVideo};
            ActualizarRectReproductores();
            DibujarMarcoVideo(hdc, m_rcVideoIzquierda, m_estado.izquierda,
                              L"Video 1", false, 0, tema);
            DibujarMarcoVideo(hdc, m_rcVideoDerecha, m_estado.derecha,
                              L"Video 2", false, 1, tema);
        }

        const int yControles = std::max(m_rcVideoIzquierda.bottom,
                                        m_rcVideoDerecha.bottom) + 18;
        m_rcLineaTiempo = {rect.left + margen, yControles,
                           rect.right - margen, yControles + 34};
        DibujarContenedor(hdc, m_rcLineaTiempo, tema);
        DibujarLineaTiempo(hdc, tema);

        const int yBotones = m_rcLineaTiempo.bottom + 8;
        m_rcRetroceder = {rect.left + margen, yBotones,
                          rect.left + margen + 92, yBotones + 30};
        m_rcReproducir = {m_rcRetroceder.right + 8, yBotones,
                          m_rcRetroceder.right + 116, yBotones + 30};
        m_rcAvanzar = {m_rcReproducir.right + 8, yBotones,
                       m_rcReproducir.right + 100, yBotones + 30};
        m_rcConfirmarSincronizacion = {m_rcAvanzar.right + 8, yBotones,
                                       m_rcAvanzar.right + 190, yBotones + 30};
        DibujarBoton(hdc, m_rcRetroceder, L"-10 s", tema, true);
        DibujarBoton(hdc, m_rcReproducir,
                     m_estado.reproduciendo ? L"Pausar" : L"Reproducir", tema, true);
        DibujarBoton(hdc, m_rcAvanzar, L"+10 s", tema, true);
        DibujarBoton(hdc, m_rcConfirmarSincronizacion,
                     m_estado.sincronizacionConfirmada ? L"Sincronización confirmada" : L"Confirmar sincronización",
                     tema, m_estado.sincronizacionConfirmada);

        SelectObject(hdc, texto);
        SetTextColor(hdc, tema.textoSecundario);
        RECT tiempo{m_rcConfirmarSincronizacion.right + 12, yBotones,
                    rect.right - margen, yBotones + 30};
        std::wstring etiquetaTiempo = m_servicio
            ? m_servicio->FormatearTiempo(m_estado.posicionSegundos) + L" / " +
              m_servicio->FormatearTiempo(m_estado.duracionSegundos)
            : L"00:00 / 10:00";
        DrawTextW(hdc, etiquetaTiempo.c_str(), -1, &tiempo,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        const int yInfo = yBotones + 46;
        RECT info{rect.left + margen, yInfo, rect.right - margen,
                  std::max(yInfo + 148, static_cast<int>(rect.bottom - margen))};
        DibujarContenedor(hdc, info, tema);
        SelectObject(hdc, negrita);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT tituloInfo{info.left + 12, info.top + 10,
                        info.right - 12, info.top + 32};
        DrawTextW(hdc, L"INFORMACIÓN DE LA COMPARACIÓN", -1, &tituloInfo,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(hdc, texto);
        DibujarDato(hdc, info, m_estado.izquierda, L"Video 1", 0, tema);
        DibujarDato(hdc, info, m_estado.derecha, L"Video 2", 1, tema);
        SetTextColor(hdc, tema.acento);
        RECT diferencia{info.left + 12, info.bottom - 30,
                        info.right - 12, info.bottom - 10};
        std::wstring diferenciaTexto = L"Diferencia de peso: " +
            (m_servicio ? m_servicio->FormatearDiferencia(
                m_estado.izquierda.bytes, m_estado.derecha.bytes) : L"+0 B");
        DrawTextW(hdc, diferenciaTexto.c_str(), -1, &diferencia,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        SetTextColor(hdc, tema.textoSecundario);
        RECT hashIzquierda{info.left + 12, info.top + 124,
                           info.left + (info.right - info.left) / 2 - 12,
                           info.top + 144};
        RECT hashDerecha{info.left + (info.right - info.left) / 2 + 12,
                         info.top + 124, info.right - 12, info.top + 144};
        std::wstring shaIzquierda = L"SHA-256: " + m_estado.izquierda.sha256;
        std::wstring shaDerecha = L"SHA-256: " + m_estado.derecha.sha256;
        DrawTextW(hdc, shaIzquierda.c_str(), -1, &hashIzquierda,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        DrawTextW(hdc, shaDerecha.c_str(), -1, &hashDerecha,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        SetTextColor(hdc, tema.textoSecundario);
        RECT mensaje{info.left + 12, info.top + 34,
                     info.right - 12, info.top + 54};
        DrawTextW(hdc, m_estado.mensaje.c_str(), -1, &mensaje,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        if (m_hoverVideo >= 3 && m_hoverVideo <= 4) {
            const DatosVideoComparacionV2& video = m_hoverVideo == 3
                ? m_estado.izquierda : m_estado.derecha;
            RECT base = m_hoverVideo == 3 ? m_rcVideoIzquierda : m_rcVideoDerecha;
            RECT tooltip{base.left + 10, base.top + 34,
                         base.right - 10, base.top + 62};
            HBRUSH fondoTooltip = CreateSolidBrush(tema.fondoCabecera);
            FillRect(hdc, &tooltip, fondoTooltip);
            DeleteObject(fondoTooltip);
            SetTextColor(hdc, tema.textoPrincipal);
            DrawTextW(hdc, video.nombre.c_str(), -1, &tooltip,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE |
                      DT_END_ELLIPSIS | DT_NOPREFIX);
        }

        SelectObject(hdc, anterior);
        DeleteObject(titulo);
        DeleteObject(texto);
        DeleteObject(negrita);
    }

private:
    static HFONT CrearFuente(int altura, int peso) {
        return CreateFontW(altura, 0, 0, 0, peso, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                           L"Segoe UI Variable Text");
    }

    void MontarReproductores() {
        if (!m_panel || !m_servicio) return;
        LiberarReproductores();
        m_reproductorIzquierda = m_servicio->CrearReproductor();
        m_reproductorDerecha = m_servicio->CrearReproductor();
        if (m_reproductorIzquierda) {
            m_reproductorIzquierda->Montar(m_panel, 0, m_estado.izquierda.ruta);
        }
        if (m_reproductorDerecha) {
            m_reproductorDerecha->Montar(m_panel, 1, m_estado.derecha.ruta);
        }
    }

    void LiberarReproductores() {
        if (m_reproductorIzquierda) m_reproductorIzquierda->Liberar();
        if (m_reproductorDerecha) m_reproductorDerecha->Liberar();
        m_reproductorIzquierda.reset();
        m_reproductorDerecha.reset();
    }

    void ActualizarEstadoDesdeReproductores() {
        const double duracionIzquierda = m_reproductorIzquierda
            ? m_reproductorIzquierda->Duracion() : 0.0;
        const double duracionDerecha = m_reproductorDerecha
            ? m_reproductorDerecha->Duracion() : 0.0;
        if (m_servicio) {
            m_servicio->ActualizarDuraciones(m_estado, duracionIzquierda, duracionDerecha);
        }

        IReproductorVideoV2* reloj = m_reproductorIzquierda &&
            m_reproductorIzquierda->EstaListo() ? m_reproductorIzquierda.get() :
            m_reproductorDerecha.get();
        if (reloj) {
            m_estado.posicionSegundos = std::clamp(
                reloj->Posicion(), 0.0, m_estado.duracionSegundos);
        }
    }

    void BuscarRelativo(double desplazamiento) {
        if (!m_servicio) return;
        m_servicio->BuscarAmbos(m_estado,
                                m_estado.posicionSegundos + desplazamiento,
                                m_reproductorIzquierda.get(),
                                m_reproductorDerecha.get());
    }

    double PosicionDesdePunto(POINT punto) const {
        const int inicio = m_rcLineaTiempo.left + 14;
        const int fin = m_rcLineaTiempo.right - 14;
        if (fin <= inicio) return 0.0;
        const double porcentaje = std::clamp(
            static_cast<double>(punto.x - inicio) / (fin - inicio), 0.0, 1.0);
        return porcentaje * m_estado.duracionSegundos;
    }

    RECT BotonPantallaCompleta(const RECT& video) const {
        return {video.right - 34, video.top + 4, video.right - 6, video.top + 28};
    }

    void ConstruirLayoutPantallaCompleta(HDC hdc, const RECT& rect, int margen,
                                         int yVideo, int altoVideo,
                                         const TemaVisual& tema) {
        RECT completa{rect.left + margen, yVideo, rect.right - margen, yVideo + altoVideo};
        const int anchoCompleta = static_cast<int>(completa.right - completa.left);
        const int altoCompleta = static_cast<int>(completa.bottom - completa.top);
        const int anchoPip = std::max(190, anchoCompleta / 4);
        const int altoPip = std::max(130, altoCompleta / 4);
        RECT pip{completa.right - anchoPip - 18, completa.bottom - altoPip - 18,
                 completa.right - 18, completa.bottom - 18};
        if (m_estado.indicePantallaCompleta == 0) {
            m_rcVideoIzquierda = completa;
            m_rcVideoDerecha = pip;
        } else {
            m_rcVideoDerecha = completa;
            m_rcVideoIzquierda = pip;
        }
        ActualizarRectReproductores();
        if (m_estado.indicePantallaCompleta == 0) {
            DibujarMarcoVideo(hdc, m_rcVideoIzquierda, m_estado.izquierda,
                              L"Video 1", false, 0, tema);
            DibujarMarcoVideo(hdc, m_rcVideoDerecha, m_estado.derecha,
                              L"Video 2", true, 1, tema);
        } else {
            DibujarMarcoVideo(hdc, m_rcVideoIzquierda, m_estado.izquierda,
                              L"Video 1", true, 0, tema);
            DibujarMarcoVideo(hdc, m_rcVideoDerecha, m_estado.derecha,
                              L"Video 2", false, 1, tema);
        }
    }

    void ActualizarRectReproductores() {
        if (!m_reproductorIzquierda || !m_reproductorDerecha) return;
        if (m_estado.modoPantallaCompleta) {
            const bool izquierdaFull = m_estado.indicePantallaCompleta == 0;
            if (izquierdaFull) {
                m_reproductorIzquierda->ActualizarRect(m_rcVideoIzquierda, false);
                m_reproductorDerecha->ActualizarRect(m_rcVideoDerecha, true);
            } else {
                m_reproductorDerecha->ActualizarRect(m_rcVideoDerecha, false);
                m_reproductorIzquierda->ActualizarRect(m_rcVideoIzquierda, true);
            }
        } else {
            m_reproductorIzquierda->ActualizarRect(m_rcVideoIzquierda, false);
            m_reproductorDerecha->ActualizarRect(m_rcVideoDerecha, false);
        }
    }

    void DibujarLineaTiempo(HDC hdc, const TemaVisual& tema) {
        RECT linea{m_rcLineaTiempo.left + 14, m_rcLineaTiempo.top + 15,
                   m_rcLineaTiempo.right - 14, m_rcLineaTiempo.top + 19};
        HBRUSH fondo = CreateSolidBrush(tema.borde);
        FillRect(hdc, &linea, fondo);
        DeleteObject(fondo);
        const double progreso = m_estado.duracionSegundos > 0.0
            ? std::clamp(m_estado.posicionSegundos / m_estado.duracionSegundos, 0.0, 1.0)
            : 0.0;
        const int x = linea.left + static_cast<int>((linea.right - linea.left) * progreso);
        HBRUSH relleno = CreateSolidBrush(tema.acento);
        RECT tramo{linea.left, linea.top, x, linea.bottom};
        FillRect(hdc, &tramo, relleno);
        DeleteObject(relleno);
        Ellipse(hdc, x - 6, linea.top - 5, x + 6, linea.bottom + 5);
    }

    void DibujarMarcoVideo(HDC hdc, const RECT& rect,
                           const DatosVideoComparacionV2& video,
                           const wchar_t* etiqueta, bool esPip, int indice,
                           const TemaVisual& tema) {
        DibujarContenedor(hdc, rect, tema);
        RECT cabecera{rect.left + 1, rect.top + 1, rect.right - 1, rect.top + 31};
        HBRUSH fondo = CreateSolidBrush(esPip ? tema.fondoCabecera : tema.fondoTarjeta);
        FillRect(hdc, &cabecera, fondo);
        DeleteObject(fondo);
        SetTextColor(hdc, tema.textoPrincipal);
        RECT titulo{rect.left + 10, rect.top + 4, rect.right - 42, rect.top + 28};
        std::wstring texto = esPip ? L"PIP · " : L"FULL SCREEN · ";
        texto += etiqueta;
        texto += L" — ";
        texto += video.nombre;
        DrawTextW(hdc, texto.c_str(), -1, &titulo,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        if (ComparadorVideosPantallaCompletaDisponibleV2()) {
            RECT boton = BotonPantallaCompleta(rect);
            if (indice == 0) m_rcBotonFullIzquierda = boton;
            else m_rcBotonFullDerecha = boton;
            DibujarBoton(hdc, boton, L"⛶", tema,
                         m_hoverVideo == (indice == 0 ? 1 : 2));
        }
    }

    void DibujarDato(HDC hdc, const RECT& info,
                     const DatosVideoComparacionV2& video,
                     const wchar_t* etiqueta, int columna,
                     const TemaVisual& tema) {
        const int mitad = (info.right - info.left) / 2;
        const int izquierda = info.left + 12 + columna * mitad;
        const int derecha = info.left + (columna + 1) * mitad - 12;
        RECT fila{izquierda, info.top + 60, derecha, info.top + 82};
        SetTextColor(hdc, tema.textoPrincipal);
        std::wstring texto = std::wstring(etiqueta) + L": " + video.nombre;
        DrawTextW(hdc, texto.c_str(), -1, &fila,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        RECT meta{izquierda, info.top + 82, derecha, info.top + 102};
        SetTextColor(hdc, tema.textoSecundario);
        std::wstring datos = L"FPS: " + video.fps + L"  ·  Tamaño: " +
            (m_servicio ? m_servicio->FormatearTamano(video.bytes) : L"0 B");
        DrawTextW(hdc, datos.c_str(), -1, &meta,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    void DibujarBoton(HDC hdc, const RECT& rc, const wchar_t* texto,
                      const TemaVisual& tema, bool resaltado) {
        HBRUSH fondo = CreateSolidBrush(resaltado ? tema.acento : tema.botonFondo);
        FillRect(hdc, &rc, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);
        SetTextColor(hdc, resaltado ? RGB(255, 255, 255) : tema.textoPrincipal);
        RECT textoRect = rc;
        DrawTextW(hdc, texto, -1, &textoRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    void DibujarContenedor(HDC hdc, const RECT& rc, const TemaVisual& tema) {
        if (!hdc || rc.right <= rc.left || rc.bottom <= rc.top) return;
        HBRUSH fondo = CreateSolidBrush(tema.fondoTarjeta);
        FillRect(hdc, &rc, fondo);
        DeleteObject(fondo);
        HPEN borde = CreatePen(PS_SOLID, 1, tema.borde);
        HPEN anterior = static_cast<HPEN>(SelectObject(hdc, borde));
        HBRUSH nulo = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HBRUSH anteriorBrocha = static_cast<HBRUSH>(SelectObject(hdc, nulo));
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
        SelectObject(hdc, anteriorBrocha);
        SelectObject(hdc, anterior);
        DeleteObject(borde);
    }

    std::shared_ptr<ServicioComparadorVideosV2> m_servicio;
    std::unique_ptr<IReproductorVideoV2> m_reproductorIzquierda;
    std::unique_ptr<IReproductorVideoV2> m_reproductorDerecha;
    EstadoComparadorVideosV2 m_estado;
    HWND m_panel{nullptr};
    bool m_arrastrandoTiempo{false};
    int m_hoverVideo{0};

    RECT m_rcVolver{};
    RECT m_rcVideoIzquierda{};
    RECT m_rcVideoDerecha{};
    RECT m_rcBotonFullIzquierda{};
    RECT m_rcBotonFullDerecha{};
    RECT m_rcLineaTiempo{};
    RECT m_rcRetroceder{};
    RECT m_rcReproducir{};
    RECT m_rcAvanzar{};
    RECT m_rcConfirmarSincronizacion{};
};

} // namespace SideViewV2
