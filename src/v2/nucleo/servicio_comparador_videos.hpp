#pragma once

#include "v2/nucleo/servicio_reproductor_video.hpp"
#include <propkey.h>
#include <shobjidl.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cwchar>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace SideViewV2 {

struct DatosVideoComparacionV2 {
    std::wstring ruta;
    std::wstring nombre;
    std::wstring fps{L"Pendiente"};
    std::uintmax_t bytes{0};
};

struct EstadoComparadorVideosV2 {
    DatosVideoComparacionV2 izquierda;
    DatosVideoComparacionV2 derecha;
    double posicionSegundos{0.0};
    double duracionSegundos{600.0};
    bool reproduciendo{false};
    bool simulado{true};
    bool modoPantallaCompleta{false};
    int indicePantallaCompleta{0};
    std::wstring mensaje{L"Selecciona dos videos para iniciar la comparación."};
};

// Orquesta únicamente el contrato del comparador. Los reproductores reales
// viven en instancias independientes creadas por ServicioReproductorVideoV2.
class ServicioComparadorVideosV2 {
public:
    explicit ServicioComparadorVideosV2(
        std::shared_ptr<ServicioReproductorVideoV2> reproductor =
            std::make_shared<ServicioReproductorVideoV2>())
        : m_reproductor(std::move(reproductor)) {}

    EstadoComparadorVideosV2 CrearFlujo(
        const std::vector<std::wstring>& seleccion = {}) const {
        EstadoComparadorVideosV2 estado;
        estado.izquierda = CrearVideoDemo(L"VIDEO 1 (simulado)");
        estado.derecha = CrearVideoDemo(L"VIDEO 2 (simulado)");
        if (seleccion.size() == 2) {
            estado.izquierda = LeerDatosVideo(seleccion[0], L"Video 1");
            estado.derecha = LeerDatosVideo(seleccion[1], L"Video 2");
            estado.simulado = false;
            estado.mensaje = L"Sincronización estricta activa.";
        }
        return estado;
    }

    EstadoComparadorVideosV2 CrearFlujoSimulado() const { return CrearFlujo(); }

    std::unique_ptr<IReproductorVideoV2> CrearReproductor() const {
        return m_reproductor ? m_reproductor->Crear() : nullptr;
    }

    void AlternarPantallaCompleta(EstadoComparadorVideosV2& estado, int indice) const {
        if (indice < 0 || indice > 1) return;
        if (!estado.modoPantallaCompleta) {
            estado.modoPantallaCompleta = true;
            estado.indicePantallaCompleta = indice;
        } else {
            estado.indicePantallaCompleta = indice;
        }
    }

    void IntercambiarPantallaCompleta(EstadoComparadorVideosV2& estado) const {
        if (estado.modoPantallaCompleta) {
            estado.indicePantallaCompleta = 1 - estado.indicePantallaCompleta;
        }
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

    void PausarAmbos(EstadoComparadorVideosV2& estado,
                     IReproductorVideoV2* izquierda,
                     IReproductorVideoV2* derecha) const {
        if (izquierda) izquierda->Pausar();
        if (derecha) derecha->Pausar();
        estado.reproduciendo = false;
    }

    void BuscarAmbos(EstadoComparadorVideosV2& estado, double segundos,
                     IReproductorVideoV2* izquierda,
                     IReproductorVideoV2* derecha) const {
        estado.posicionSegundos = std::clamp(segundos, 0.0, estado.duracionSegundos);
        if (izquierda) izquierda->Buscar(estado.posicionSegundos);
        if (derecha) derecha->Buscar(estado.posicionSegundos);
    }

    std::wstring FormatearTamano(std::uintmax_t bytes) const {
        static constexpr const wchar_t* unidades[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
        double valor = static_cast<double>(bytes);
        std::size_t unidad = 0;
        constexpr std::size_t cantidadUnidades = sizeof(unidades) / sizeof(unidades[0]);
        while (valor >= 1024.0 && unidad + 1 < cantidadUnidades) {
            valor /= 1024.0;
            ++unidad;
        }
        std::wostringstream salida;
        salida << std::fixed << std::setprecision(
            unidad == 0 ? 0 : valor < 10.0 ? 2 : 1)
               << valor << L" " << unidades[unidad];
        return salida.str();
    }

    std::wstring FormatearDiferencia(std::uintmax_t izquierda,
                                     std::uintmax_t derecha) const {
        const bool negativa = derecha < izquierda;
        const std::uintmax_t magnitud = negativa ? izquierda - derecha : derecha - izquierda;
        return std::wstring(negativa ? L"-" : L"+") + FormatearTamano(magnitud);
    }

    std::wstring FormatearTiempo(double segundos) const {
        const int total = std::max(0, static_cast<int>(std::lround(segundos)));
        const int horas = total / 3600;
        const int minutos = (total % 3600) / 60;
        const int segundosRestantes = total % 60;
        wchar_t buffer[32]{};
        if (horas > 0) {
            swprintf_s(buffer, L"%02d:%02d:%02d", horas, minutos, segundosRestantes);
        } else {
            swprintf_s(buffer, L"%02d:%02d", minutos, segundosRestantes);
        }
        return buffer;
    }

private:
    DatosVideoComparacionV2 CrearVideoDemo(const wchar_t* nombre) const {
        DatosVideoComparacionV2 video;
        video.nombre = nombre;
        video.fps = L"Pendiente";
        return video;
    }

    DatosVideoComparacionV2 LeerDatosVideo(const std::wstring& ruta,
                                           const wchar_t* etiqueta) const {
        DatosVideoComparacionV2 video;
        video.ruta = ruta;
        const wchar_t* nombre = PathFindFileNameW(ruta.c_str());
        video.nombre = (nombre && *nombre) ? nombre : etiqueta;
        WIN32_FILE_ATTRIBUTE_DATA datos{};
        if (GetFileAttributesExW(ruta.c_str(), GetFileExInfoStandard, &datos)) {
            ULARGE_INTEGER tamano{};
            tamano.HighPart = datos.nFileSizeHigh;
            tamano.LowPart = datos.nFileSizeLow;
            video.bytes = static_cast<std::uintmax_t>(tamano.QuadPart);
        }
        IShellItem2* elemento = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(
                ruta.c_str(), nullptr, IID_IShellItem2,
                reinterpret_cast<void**>(&elemento))) && elemento) {
            ULONG fpsMil = 0;
            if (SUCCEEDED(elemento->GetUInt32(PKEY_Video_FrameRate, &fpsMil)) && fpsMil) {
                wchar_t fps[32]{};
                swprintf_s(fps, L"%.2f", static_cast<double>(fpsMil) / 1000.0);
                video.fps = fps;
            }
            elemento->Release();
        }
        return video;
    }

    std::shared_ptr<ServicioReproductorVideoV2> m_reproductor;
};

} // namespace SideViewV2
