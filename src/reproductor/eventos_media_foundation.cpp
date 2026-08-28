class ReceptorEventosMediaFoundation final : public IMFPMediaPlayerCallback {
    std::atomic<ULONG> referencias_{1};

  public:
    STDMETHODIMP QueryInterface(REFIID interfaz, void** resultado) override {
        if (!resultado) return E_POINTER;
        if (interfaz == IID_IUnknown ||
            interfaz == __uuidof(IMFPMediaPlayerCallback)) {
            *resultado = static_cast<IMFPMediaPlayerCallback*>(this);
            AddRef();
            return S_OK;
        }
        *resultado = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return ++referencias_; }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG restantes = --referencias_;
        if (!restantes) delete this;
        return restantes;
    }

    void STDMETHODCALLTYPE OnMediaPlayerEvent(MFP_EVENT_HEADER* evento) override {
        if (!evento) return;

        // Media Foundation crea el elemento de forma asíncrona. Solo este
        // evento garantiza que ya puede asignarse al reproductor.
        if (evento->eEventType == MFP_EVENT_TYPE_MEDIAITEM_CREATED &&
            SUCCEEDED(evento->hrEvent) && reproductor) {
            auto eventoCreacion =
                reinterpret_cast<MFP_MEDIAITEM_CREATED_EVENT*>(evento);
            reproductor->SetMediaItem(eventoCreacion->pMediaItem);
        }
        if (evento->eEventType == MFP_EVENT_TYPE_MEDIAITEM_SET &&
            SUCCEEDED(evento->hrEvent) && reproductor) {
            reproductor->SetMute(!configuracion.sonidoHabilitado.load());
            if (configuracion.reproduccionAutomatica.load()) reproductor->Play();
        }
        if (ventanaReproductor) {
            PostMessageW(ventanaReproductor, MENSAJE_ACTUALIZAR_INTERFAZ, 0, 0);
        }
    }
};
