class ReceptorEventosMediaFoundation final : public IMFPMediaPlayerCallback {
    std::atomic<ULONG> referencias_{1};

  public:
    STDMETHODIMP QueryInterface(REFIID identificadorInterfaz,
                                void** direccionInterfaz) override {
        if (!direccionInterfaz) return E_POINTER;
        if (identificadorInterfaz == IID_IUnknown ||
            identificadorInterfaz == __uuidof(IMFPMediaPlayerCallback)) {
            *direccionInterfaz = static_cast<IMFPMediaPlayerCallback*>(this);
            AddRef();
            return S_OK;
        }
        *direccionInterfaz = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return ++referencias_; }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG referenciasRestantes = --referencias_;
        if (!referenciasRestantes) delete this;
        return referenciasRestantes;
    }

    void STDMETHODCALLTYPE OnMediaPlayerEvent(
        MFP_EVENT_HEADER* eventoReproductor) override {
        if (!eventoReproductor) return;

        if (eventoReproductor->eEventType ==
                MFP_EVENT_TYPE_MEDIAITEM_CREATED &&
            SUCCEEDED(eventoReproductor->hrEvent) && reproductor) {
            auto eventoElementoCreado =
                MFP_GET_MEDIAITEM_CREATED_EVENT(eventoReproductor);
            reproductor->SetMediaItem(eventoElementoCreado->pMediaItem);
        }

        if (eventoReproductor->eEventType == MFP_EVENT_TYPE_MEDIAITEM_SET &&
            SUCCEEDED(eventoReproductor->hrEvent) && reproductor) {
            auto eventoElementoEstablecido =
                MFP_GET_MEDIAITEM_SET_EVENT(eventoReproductor);
            MFP_MEDIAITEM_CHARACTERISTICS caracteristicasElemento = 0;
            bool permitePosicionamiento =
                eventoElementoEstablecido &&
                eventoElementoEstablecido->pMediaItem &&
                SUCCEEDED(eventoElementoEstablecido->pMediaItem
                              ->GetCharacteristics(
                                  &caracteristicasElemento)) &&
                (caracteristicasElemento & MFP_MEDIAITEM_CAN_SEEK) != 0 &&
                (caracteristicasElemento & MFP_MEDIAITEM_HAS_SLOW_SEEK) == 0;
            posicionamientoDisponible.store(permitePosicionamiento);
            reproductor->SetMute(!configuracion.sonidoHabilitado.load());
            float velocidadConfigurada = velocidadReproduccionActual.load();
            if (velocidadConfigurada > 0.0f && velocidadConfigurada != 1.0f) {
                reproductor->SetRate(velocidadConfigurada);
            }
            if (configuracion.reproduccionAutomatica.load() ||
                navegacionManualEnCurso.exchange(false)) {
                reproductor->Play();
            }
            if (ventanaDelegadoReproductor &&
                IsWindow(ventanaDelegadoReproductor)) {
                SendMessageW(ventanaDelegadoReproductor,
                             MENSAJE_ACTUALIZAR_CACHE_TIEMPO, 0, 0);
            }
        }

        if (eventoReproductor->eEventType == MFP_EVENT_TYPE_POSITION_SET) {
            bool debeReproducir =
                reproducirTrasCambioPosicion.exchange(false);
            if (SUCCEEDED(eventoReproductor->hrEvent) && debeReproducir &&
                reproductor) {
                reproductor->Play();
            }
            if (ventanaDelegadoReproductor &&
                IsWindow(ventanaDelegadoReproductor)) {
                SendMessageW(ventanaDelegadoReproductor,
                             MENSAJE_ACTUALIZAR_CACHE_TIEMPO, 0, 0);
            }
            cambioPosicionPendiente.store(false);
        }

        if (eventoReproductor->eEventType ==
                MFP_EVENT_TYPE_PLAYBACK_ENDED &&
            SUCCEEDED(eventoReproductor->hrEvent) &&
            repeticionHabilitada.load() && reproductor) {
            buscarEnReproduccion(0.0, true);
        }

        if (FAILED(eventoReproductor->hrEvent)) {
            Wh_Log(L"Evento de Media Foundation fallido: tipo %u, HRESULT "
                   L"0x%08X",
                   static_cast<unsigned int>(eventoReproductor->eEventType),
                   static_cast<unsigned int>(eventoReproductor->hrEvent));
        }

        if (ventanaControles) {
            PostMessageW(ventanaControles, MENSAJE_ACTUALIZAR_INTERFAZ, 0, 0);
        }
    }
};
