#pragma once
#include "v2/nucleo/servicio_navegacion.hpp"
#include "v2/nucleo/servicio_abrir_con.hpp"
#include "v2/nucleo/servicio_extension.hpp"
#include "v2/nucleo/servicios_carpetas.hpp"
#include "v2/nucleo/servicio_texto.hpp"
#include "v2/nucleo/servicio_previsualizacion_media.hpp"
#include "v2/nucleo/servicio_comprimidos.hpp"
#include "v2/nucleo/servicio_enlaces.hpp"
#include "v2/nucleo/servicio_inspeccion_carpetas.hpp"
#include "v2/nucleo/servicio_comparador_videos.hpp"
#include "v2/nucleo/servicio_reproductor_video.hpp"
#include <memory>
namespace SideViewV2 {
struct DependenciasVistaV2 {
    std::shared_ptr<ServicioNavegacionV2> navegacion{std::make_shared<ServicioNavegacionV2>()};
    std::shared_ptr<ServicioExtensionV2> extensiones{std::make_shared<ServicioExtensionV2>()};
    std::shared_ptr<ServicioAbrirConV2> abrirConCarpetas{std::make_shared<ServicioAbrirConV2>(L"carpetas", std::vector<std::wstring>{L"code.exe",L"explorer.exe",L"cmd.exe",L"powershell.exe"})};
    std::shared_ptr<ServicioAbrirConV2> abrirConComprimidos{std::make_shared<ServicioAbrirConV2>(L"comprimidos")};
    std::shared_ptr<ServicioAbrirConV2> abrirConImagenes{std::make_shared<ServicioAbrirConV2>(L"imagenes")};
    std::shared_ptr<ServicioAbrirConV2> abrirConVideos{std::make_shared<ServicioAbrirConV2>(L"videos")};
    std::shared_ptr<ServicioTextoV2> texto{std::make_shared<ServicioTextoV2>()};
    std::shared_ptr<ServicioContenidoComprimidoV2> contenidoComprimido{std::make_shared<ServicioContenidoComprimidoV2>()};
    std::shared_ptr<ServicioEnlacesV2> enlaces{std::make_shared<ServicioEnlacesV2>()};
    std::shared_ptr<ServicioPrevisualizacionMediaV2> previsualizacionMedia{std::make_shared<ServicioPrevisualizacionMediaV2>()};
    std::shared_ptr<ServicioFusionCarpetasV2> fusionCarpetas{std::make_shared<ServicioFusionCarpetasV2>()};
    std::shared_ptr<ServicioInspeccionCarpetasV2> inspeccionCarpetas{std::make_shared<ServicioInspeccionCarpetasV2>()};
    std::shared_ptr<ServicioReproductorVideoV2> reproductorVideo{std::make_shared<ServicioReproductorVideoV2>()};
    std::shared_ptr<ServicioComparadorVideosV2> comparadorVideos{std::make_shared<ServicioComparadorVideosV2>(reproductorVideo)};
};
}
