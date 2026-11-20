#pragma once

#include "v2/nucleo/servicio_comparador_archivos.hpp"
#include "v2/nucleo/servicio_previsualizacion_media.hpp"
#include <windows.h>
#include <memory>
#include <string>

namespace SideViewV2 {

struct EstadoComparadorImagenesV2 {
    bool modoPantallaCompleta{false};
    int indicePantallaCompleta{0};
};

// Servicio atómico del comparador visual. La vista sólo consume bitmaps y
// dibuja el layout; la carga, liberación y transición full screen/PiP quedan
// inyectadas en este banco de funciones.
class ServicioComparadorImagenesV2 final : public ServicioComparadorArchivosV2 {
public:
    ServicioComparadorImagenesV2(
        std::shared_ptr<ServicioPrevisualizacionMediaV2> previsualizacion)
        : m_previsualizacion(std::move(previsualizacion)) {}

    HBITMAP CargarImagen(const std::wstring& ruta) const {
        return m_previsualizacion ? m_previsualizacion->CargarImagen(ruta) : nullptr;
    }

    void LiberarImagen(HBITMAP imagen) const {
        if (imagen) DeleteObject(imagen);
    }

    void AlternarPantallaCompleta(EstadoComparadorImagenesV2& estado,
                                  int indice) const {
        if (indice < 0 || indice > 1) return;
        if (estado.modoPantallaCompleta &&
            estado.indicePantallaCompleta == indice) {
            estado.modoPantallaCompleta = false;
            return;
        }
        estado.modoPantallaCompleta = true;
        estado.indicePantallaCompleta = indice;
    }

    void IntercambiarPantallaCompleta(EstadoComparadorImagenesV2& estado) const {
        if (!estado.modoPantallaCompleta) return;
        estado.indicePantallaCompleta =
            estado.indicePantallaCompleta == 0 ? 1 : 0;
    }

private:
    std::shared_ptr<ServicioPrevisualizacionMediaV2> m_previsualizacion;
};

} // namespace SideViewV2
