#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include <windows.h>
#include <windowsx.h>
#include <mfapi.h>
#include <mfmediaengine.h>
#include <shlwapi.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cwchar>
#include <memory>
#include <mutex>
#include <new>
#include <oleauto.h>
#include <string>

namespace SideViewV2 {

class IReproductorVideoV2 {
public:
    virtual ~IReproductorVideoV2() = default;
    virtual bool Montar(HWND panel, int ranura, const std::wstring& ruta) = 0;
    virtual void ActualizarRect(const RECT& rect, bool esPip) = 0;
    virtual bool Reproducir() = 0;
    virtual void Pausar() = 0;
    virtual void Buscar(double segundos) = 0;
    virtual bool EstaListo() const = 0;
    virtual bool EstaReproduciendo() const = 0;
    virtual double Posicion() const = 0;
    virtual double Duracion() const = 0;
    virtual const std::wstring& Mensaje() const = 0;
    virtual void Liberar() = 0;
};

class MediaEngineNotifyV2 final : public IMFMediaEngineNotify {
public:
    explicit MediaEngineNotifyV2(HWND destino) : m_destino(destino) {}

    void Desconectar() { m_destino.store(nullptr, std::memory_order_release); }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** salida) override {
        if (!salida) return E_POINTER;
        *salida = nullptr;
        if (IsEqualIID(iid, IID_IUnknown) ||
            IsEqualIID(iid, IID_IMFMediaEngineNotify)) {
            *salida = static_cast<IMFMediaEngineNotify*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return static_cast<ULONG>(InterlockedIncrement(&m_referencias));
    }

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG restantes = static_cast<ULONG>(InterlockedDecrement(&m_referencias));
        if (restantes == 0) delete this;
        return restantes;
    }

    HRESULT STDMETHODCALLTYPE EventNotify(DWORD evento, DWORD_PTR param1,
                                           DWORD) override {
        if (evento == MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE) {
            if (param1) SetEvent(reinterpret_cast<HANDLE>(param1));
            return S_OK;
        }
        HWND destino = m_destino.load(std::memory_order_acquire);
        if (destino) {
            PostMessageW(destino, WM_SIDEVIEW_VIDEO_EVENT_V2,
                         static_cast<WPARAM>(evento), static_cast<LPARAM>(param1));
        }
        return S_OK;
    }

private:
    ~MediaEngineNotifyV2() = default;
    LONG m_referencias{1};
    std::atomic<HWND> m_destino{nullptr};
};

class ReproductorMediaFoundationV2 final : public IReproductorVideoV2 {
public:
    ~ReproductorMediaFoundationV2() override { Liberar(); }

    bool Montar(HWND panel, int ranura, const std::wstring& ruta) override {
        Liberar();
        m_panel = panel;
        m_ranura = ranura;
        m_ruta = ruta;
        m_mensaje.clear();

        RegistrarClase();
        m_ventana = CreateWindowExW(
            0, ClaseVideo(), L"", WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 1, 1, panel, nullptr, GetModuleHandleW(nullptr), this);
        if (!m_ventana) {
            m_mensaje = L"No se pudo crear el espacio de reproducción.";
            return false;
        }

        if (ruta.empty()) {
            m_mensaje = L"Selecciona dos videos para iniciar la comparación.";
            return false;
        }

        DWORD atributos = GetFileAttributesW(ruta.c_str());
        if (atributos == INVALID_FILE_ATTRIBUTES ||
            (atributos & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            m_mensaje = L"No se pudo abrir el archivo de video.";
            return false;
        }

        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr)) {
            m_mensaje = L"Media Foundation no está disponible en este sistema.";
            return false;
        }
        m_mfIniciado = true;

        MediaEngineNotifyV2* callback = new (std::nothrow) MediaEngineNotifyV2(m_ventana);
        if (!callback) {
            m_mensaje = L"No hubo memoria suficiente para iniciar el reproductor.";
            LiberarMediaFoundation();
            return false;
        }
        m_callback = callback;

        IMFAttributes* atributosMedia = nullptr;
        IMFMediaEngineClassFactory* fabrica = nullptr;
        hr = MFCreateAttributes(&atributosMedia, 3);
        if (SUCCEEDED(hr)) hr = atributosMedia->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, callback);
        if (SUCCEEDED(hr)) hr = atributosMedia->SetUINT64(
            MF_MEDIA_ENGINE_PLAYBACK_HWND, reinterpret_cast<UINT64>(m_ventana));
        if (SUCCEEDED(hr)) hr = CoCreateInstance(
            CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_IMFMediaEngineClassFactory, reinterpret_cast<void**>(&fabrica));
        if (SUCCEEDED(hr)) hr = fabrica->CreateInstance(
            0, atributosMedia, &m_engine);
        if (SUCCEEDED(hr)) hr = m_engine->QueryInterface(
            IID_IMFMediaEngineEx, reinterpret_cast<void**>(&m_engineEx));
        if (SUCCEEDED(hr)) hr = m_engine->SetAutoPlay(FALSE);
        if (SUCCEEDED(hr)) hr = m_engine->SetPreload(MF_MEDIA_ENGINE_PRELOAD_AUTOMATIC);

        if (SUCCEEDED(hr)) {
            wchar_t url[2048]{};
            DWORD longitud = ARRAYSIZE(url);
            hr = UrlCreateFromPathW(ruta.c_str(), url, &longitud, 0);
            if (hr == E_POINTER && longitud > ARRAYSIZE(url)) {
                std::wstring urlAmplia(longitud, L'\0');
                hr = UrlCreateFromPathW(ruta.c_str(), urlAmplia.data(), &longitud, 0);
                if (SUCCEEDED(hr)) {
                    BSTR origen = SysAllocString(urlAmplia.c_str());
                    hr = origen ? m_engine->SetSource(origen) : E_OUTOFMEMORY;
                    SysFreeString(origen);
                }
            } else if (SUCCEEDED(hr)) {
                BSTR origen = SysAllocString(url);
                hr = origen ? m_engine->SetSource(origen) : E_OUTOFMEMORY;
                SysFreeString(origen);
            }
        }

        if (atributosMedia) atributosMedia->Release();
        if (fabrica) fabrica->Release();
        if (FAILED(hr)) {
            m_mensaje = L"No se pudo reproducir este formato de video.";
            LiberarBackend();
            return false;
        }

        m_mensaje = L"Cargando video...";
        return true;
    }

    void ActualizarRect(const RECT& rect, bool esPip) override {
        m_esPip = esPip;
        if (!IsWindow(m_ventana)) return;

        RECT render = rect;
        render.left += 1;
        render.right = std::max(render.left + 1, render.right - 1);
        render.top += 30; // Reserva la franja superior para título y controles.
        render.bottom = std::max(render.top + 1, render.bottom - 1);
        SetWindowPos(m_ventana, HWND_TOP, render.left, render.top,
                     render.right - render.left, render.bottom - render.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
        if (m_engineEx) ActualizarFrame();
    }

    bool Reproducir() override {
        m_reproduccionSolicitada = true;
        if (!m_engine || !m_listo) return false;
        HRESULT hr = m_engine->Play();
        if (SUCCEEDED(hr)) {
            m_reproduciendo = true;
            SetTimer(m_ventana, kTimerReproductor, 33, nullptr);
        }
        return SUCCEEDED(hr);
    }

    void Pausar() override {
        m_reproduccionSolicitada = false;
        m_reproduciendo = false;
        if (IsWindow(m_ventana)) KillTimer(m_ventana, kTimerReproductor);
        if (m_engine) m_engine->Pause();
    }

    void Buscar(double segundos) override {
        m_posicionPendiente = std::max(0.0, segundos);
        if (m_engine && m_listo) {
            m_engine->SetCurrentTime(m_posicionPendiente);
        }
    }

    bool EstaListo() const override { return m_listo; }
    bool EstaReproduciendo() const override { return m_reproduciendo; }

    double Posicion() const override {
        if (m_engine && m_listo) {
            double actual = m_engine->GetCurrentTime();
            if (std::isfinite(actual) && actual >= 0.0) return actual;
        }
        return m_posicionPendiente;
    }

    double Duracion() const override {
        if (m_engine && m_listo) {
            double duracion = m_engine->GetDuration();
            if (std::isfinite(duracion) && duracion > 0.0) return duracion;
        }
        return 0.0;
    }

    const std::wstring& Mensaje() const override { return m_mensaje; }

    void Liberar() override {
        LiberarBackend();
        if (IsWindow(m_ventana)) DestroyWindow(m_ventana);
        m_ventana = nullptr;
        m_panel = nullptr;
        m_listo = false;
        m_reproduciendo = false;
        m_reproduccionSolicitada = false;
        m_posicionPendiente = 0.0;
    }

private:
    static constexpr UINT_PTR kTimerReproductor = 17;

    static const wchar_t* ClaseVideo() {
        return L"UsefulSideView_MediaFoundationVideo_V2";
    }

    static void RegistrarClase() {
        static std::once_flag unaVez;
        std::call_once(unaVez, [] {
            WNDCLASSW clase{};
            clase.lpfnWndProc = ProcedimientoVideo;
            clase.hInstance = GetModuleHandleW(nullptr);
            clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            clase.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
            clase.lpszClassName = ClaseVideo();
            RegisterClassW(&clase);
        });
    }

    static LRESULT CALLBACK ProcedimientoVideo(HWND hwnd, UINT mensaje,
                                               WPARAM wParam, LPARAM lParam) {
        auto* reproductor = reinterpret_cast<ReproductorMediaFoundationV2*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (mensaje == WM_NCCREATE) {
            auto* crear = reinterpret_cast<CREATESTRUCTW*>(lParam);
            reproductor = static_cast<ReproductorMediaFoundationV2*>(crear->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reproductor));
        }
        if (!reproductor) return DefWindowProcW(hwnd, mensaje, wParam, lParam);
        if (mensaje == WM_NCDESTROY) {
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            return DefWindowProcW(hwnd, mensaje, wParam, lParam);
        }
        switch (mensaje) {
            case WM_ERASEBKGND:
                return 1;
            case WM_SIZE:
                reproductor->ActualizarFrame();
                return 0;
            case WM_TIMER:
                if (wParam == kTimerReproductor && reproductor->m_reproduciendo &&
                    reproductor->m_panel) {
                    PostMessageW(reproductor->m_panel, WM_SIDEVIEW_COMPARADOR_TICK,
                                 static_cast<WPARAM>(reproductor->m_ranura), 0);
                    return 0;
                }
                break;
            case WM_LBUTTONDOWN:
                if (reproductor->m_esPip && reproductor->m_panel) {
                    PostMessageW(reproductor->m_panel, WM_SIDEVIEW_COMPARADOR_PIP_CLICK,
                                 static_cast<WPARAM>(reproductor->m_ranura), 0);
                    return 0;
                }
                break;
            case WM_PAINT: {
                PAINTSTRUCT ps{};
                HDC hdc = BeginPaint(hwnd, &ps);
                if (reproductor->m_listo && reproductor->m_engineEx) {
                    reproductor->ActualizarFrame();
                } else {
                    RECT rect{};
                    GetClientRect(hwnd, &rect);
                    FillRect(hdc, &rect, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
                    SetBkMode(hdc, TRANSPARENT);
                    SetTextColor(hdc, RGB(220, 220, 220));
                    DrawTextW(hdc, reproductor->m_mensaje.c_str(), -1, &rect,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE |
                              DT_END_ELLIPSIS | DT_NOPREFIX);
                }
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_SIDEVIEW_VIDEO_EVENT_V2:
                reproductor->ProcesarEvento(static_cast<DWORD>(wParam),
                                             static_cast<HRESULT>(lParam));
                return 0;
        }
        return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    }

    void ProcesarEvento(DWORD evento, HRESULT error) {
        if (!m_engine) return;
        switch (evento) {
            case MF_MEDIA_ENGINE_EVENT_ERROR:
                m_listo = false;
                m_reproduciendo = false;
                m_reproduccionSolicitada = false;
                if (IsWindow(m_ventana)) KillTimer(m_ventana, kTimerReproductor);
                m_mensaje = L"No se pudo reproducir este archivo: formato no admitido o archivo corrupto.";
                (void)error;
                break;
            case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
            case MF_MEDIA_ENGINE_EVENT_DURATIONCHANGE:
                if (m_posicionPendiente > 0.0) m_engine->SetCurrentTime(m_posicionPendiente);
                break;
            case MF_MEDIA_ENGINE_EVENT_CANPLAY:
            case MF_MEDIA_ENGINE_EVENT_CANPLAYTHROUGH:
                m_listo = true;
                m_mensaje.clear();
                if (m_posicionPendiente > 0.0) m_engine->SetCurrentTime(m_posicionPendiente);
                if (m_reproduccionSolicitada) Reproducir();
                break;
            case MF_MEDIA_ENGINE_EVENT_PLAY:
            case MF_MEDIA_ENGINE_EVENT_PLAYING:
                m_listo = true;
                m_reproduciendo = true;
                m_mensaje.clear();
                SetTimer(m_ventana, kTimerReproductor, 33, nullptr);
                break;
            case MF_MEDIA_ENGINE_EVENT_PAUSE:
                m_reproduciendo = false;
                if (IsWindow(m_ventana)) KillTimer(m_ventana, kTimerReproductor);
                break;
            case MF_MEDIA_ENGINE_EVENT_ENDED:
                Pausar();
                Buscar(0.0);
                break;
            default:
                break;
        }
        InvalidateRect(m_ventana, nullptr, FALSE);
        if (m_panel) PostMessageW(m_panel, WM_SIDEVIEW_COMPARADOR_TICK,
                                  static_cast<WPARAM>(m_ranura), 0);
    }

    void ActualizarFrame() {
        if (!m_engineEx || !IsWindow(m_ventana)) return;
        RECT destino{};
        GetClientRect(m_ventana, &destino);
        MFARGB borde{};
        borde.rgbAlpha = 255;
        m_engineEx->UpdateVideoStream(nullptr, &destino, &borde);
    }

    template <typename T>
    static void LiberarCom(T*& objeto) {
        if (objeto) {
            objeto->Release();
            objeto = nullptr;
        }
    }

    void LiberarMediaFoundation() {
        if (m_mfIniciado) {
            MFShutdown();
            m_mfIniciado = false;
        }
    }

    void LiberarBackend() {
        Pausar();
        if (m_callback) m_callback->Desconectar();
        if (m_engine) m_engine->Shutdown();
        LiberarCom(m_engineEx);
        LiberarCom(m_engine);
        if (m_callback) {
            m_callback->Release();
            m_callback = nullptr;
        }
        LiberarMediaFoundation();
        m_listo = false;
        m_reproduciendo = false;
        m_reproduccionSolicitada = false;
    }

    HWND m_panel{nullptr};
    HWND m_ventana{nullptr};
    int m_ranura{0};
    bool m_esPip{false};
    bool m_mfIniciado{false};
    bool m_listo{false};
    bool m_reproduciendo{false};
    bool m_reproduccionSolicitada{false};
    double m_posicionPendiente{0.0};
    std::wstring m_ruta;
    std::wstring m_mensaje;
    IMFMediaEngine* m_engine{nullptr};
    IMFMediaEngineEx* m_engineEx{nullptr};
    MediaEngineNotifyV2* m_callback{nullptr};
};

class ServicioReproductorVideoV2 {
public:
    std::unique_ptr<IReproductorVideoV2> Crear() const {
        // Punto de sustitución futuro: otro backend puede implementar la misma
        // interfaz sin cambiar la vista ni el comparador.
        return std::make_unique<ReproductorMediaFoundationV2>();
    }
};

} // namespace SideViewV2
