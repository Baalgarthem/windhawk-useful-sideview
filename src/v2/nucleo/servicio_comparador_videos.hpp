#pragma once

#include "v2/nucleo/servicio_comparador_archivos.hpp"
#include "v2/nucleo/servicio_reproductor_video.hpp"
#include <propkey.h>
#include <shobjidl.h>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

using DatosVideoComparacionV2 = DatosArchivoComparacionV2;

struct EstadoComparadorVideosV2 : public EstadoComparadorArchivosV2 {
    double posicionSegundos{0.0};
    double duracionSegundos{600.0};
    bool reproduciendo{false};
    bool simulado{true};
    bool modoPantallaCompleta{false};
    int indicePantallaCompleta{0};
};

// Comparador especializado que hereda las reglas comunes de peso, nombre y
// SHA-256, y agrega el reloj sincronizado y los dos reproductores inyectados.
class ServicioComparadorVideosV2 : public ServicioComparadorArchivosV2 {
public:
    explicit ServicioComparadorVideosV2(
        std::shared_ptr<ServicioReproductorVideoV2> reproductor =
            std::make_shared<ServicioReproductorVideoV2>())
        : m_reproductor(std::move(reproductor)) {}

    EstadoComparadorVideosV2 CrearFlujo(
        const std::vector<std::wstring>& seleccion = {}) const {
        EstadoComparadorVideosV2 estado;
        EstadoComparadorArchivosV2 comun = CrearComparacion(seleccion);
        estado.izquierda = EnriquecerVideo(comun.izquierda);
        estado.derecha = EnriquecerVideo(comun.derecha);
        estado.simulado = seleccion.size() != 2;
        estado.mensaje = seleccion.size() == 2
            ? L"Sincronización estricta activa."
            : L"Selecciona dos videos para iniciar la comparación.";
        return estado;
    }

    std::unique_ptr<IReproductorVideoV2> CrearReproductor() const {
        return m_reproductor ? m_reproductor->Crear() : nullptr;
    }

    void AlternarPantallaCompleta(EstadoComparadorVideosV2& estado, int indice) const {
        if (indice < 0 || indice > 1) return;
        estado.modoPantallaCompleta = true;
        estado.indicePantallaCompleta = indice;
    }

    void IntercambiarPantallaCompleta(EstadoComparadorVideosV2& estado) const {
        if (estado.modoPantallaCompleta)
            estado.indicePantallaCompleta = 1 - estado.indicePantallaCompleta;
    }

    void AlternarEstadoReproduccion(EstadoComparadorVideosV2& estado,
                                    IReproductorVideoV2* izquierda,
                                    IReproductorVideoV2* derecha) const {
        const bool reproducir = !estado.reproduciendo;
        if (reproducir) {
            if (izquierda) izquierda->Reproducir();
            if (derecha) derecha->Reproducir();
        } else {
            if (izquierda) izquierda->Pausar();
            if (derecha) derecha->Pausar();
        }
        estado.reproduciendo = reproducir;
    }

    void BuscarAmbos(EstadoComparadorVideosV2& estado, double segundos,
                     IReproductorVideoV2* izquierda,
                     IReproductorVideoV2* derecha) const {
        estado.posicionSegundos = std::clamp(segundos, 0.0, estado.duracionSegundos);
        if (izquierda) izquierda->Buscar(estado.posicionSegundos);
        if (derecha) derecha->Buscar(estado.posicionSegundos);
    }

    std::wstring FormatearTiempo(double segundos) const {
        const int total = std::max(0, static_cast<int>(std::lround(segundos)));
        wchar_t buffer[32]{};
        swprintf_s(buffer, L"%02d:%02d", (total % 3600) / 60, total % 60);
        return buffer;
    }

private:
    static DatosVideoComparacionV2 EnriquecerVideo(DatosVideoComparacionV2 datos) {
        if (datos.ruta.empty()) return datos;
        IShellItem2* elemento = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(
                datos.ruta.c_str(), nullptr, IID_IShellItem2,
                reinterpret_cast<void**>(&elemento))) && elemento) {
            ULONG fpsMil = 0;
            if (SUCCEEDED(elemento->GetUInt32(PKEY_Video_FrameRate, &fpsMil)) && fpsMil) {
                wchar_t fps[32]{};
                swprintf_s(fps, L"%.2f fps", static_cast<double>(fpsMil) / 1000.0);
                datos.fps = fps;
            }
            elemento->Release();
        }
        return datos;
    }

    std::shared_ptr<ServicioReproductorVideoV2> m_reproductor;
};

} // namespace SideViewV2
