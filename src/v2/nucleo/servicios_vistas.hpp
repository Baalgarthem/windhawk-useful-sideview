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
#include "v2/nucleo/servicio_diff.hpp"
#include "v2/nucleo/servicio_reproductor_video.hpp"
#include "v2/nucleo/servicio_persistencia_windhawk.hpp"
#include "v2/nucleo/servicio_version_mod.hpp"
#include "v2/nucleo/servicio_compatibilidad_gpu.hpp"
#include "v2/nucleo/servicio_notas.hpp"
#include "v2/nucleo/configuracion_v2.hpp"
#include <memory>
namespace SideViewV2 {
struct DependenciasVistaV2 {
    std::shared_ptr<ServicioNavegacionV2> navegacion{std::make_shared<ServicioNavegacionV2>()};
    std::shared_ptr<ServicioExtensionV2> extensiones{std::make_shared<ServicioExtensionV2>()};
    std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia{std::make_shared<ServicioPersistenciaWindhawkV2>()};
    std::shared_ptr<ServicioVersionModV2> versionMod{std::make_shared<ServicioVersionModV2>()};
    std::shared_ptr<ServicioCompatibilidadGpuV2> compatibilidadGpu{std::make_shared<ServicioCompatibilidadGpuV2>()};
    std::shared_ptr<ServicioNotasV2> notas{std::make_shared<ServicioNotasV2>(
        persistencia, [] { return SincronizacionNotasWinKlokDisponibleV2(); })};
    std::shared_ptr<ServicioAbrirConV2> abrirConCarpetas{std::make_shared<ServicioAbrirConV2>(persistencia, L"carpetas", std::vector<std::wstring>{L"code.exe",L"explorer.exe",L"cmd.exe",L"powershell.exe"})};
    std::shared_ptr<ServicioAbrirConV2> abrirConComprimidos{std::make_shared<ServicioAbrirConV2>(persistencia, L"comprimidos")};
    std::shared_ptr<ServicioAbrirConV2> abrirConImagenes{std::make_shared<ServicioAbrirConV2>(persistencia, L"imagenes")};
    std::shared_ptr<ServicioAbrirConV2> abrirConVideos{std::make_shared<ServicioAbrirConV2>(persistencia, L"videos")};
    std::shared_ptr<ServicioTextoV2> texto{std::make_shared<ServicioTextoV2>()};
    std::shared_ptr<ServicioComparadorArchivosV2> comparadorArchivos{std::make_shared<ServicioComparadorArchivosV2>()};
    std::shared_ptr<ServicioComparadorTextoV2> comparadorTexto{std::make_shared<ServicioComparadorTextoV2>(texto)};
    std::shared_ptr<ServicioComparadorDocumentosV2> comparadorDocumentos{std::make_shared<ServicioComparadorDocumentosV2>()};
    std::shared_ptr<ServicioContenidoComprimidoV2> contenidoComprimido{std::make_shared<ServicioContenidoComprimidoV2>()};
    std::shared_ptr<ServicioEnlacesV2> enlaces{std::make_shared<ServicioEnlacesV2>()};
    std::shared_ptr<ServicioPrevisualizacionMediaV2> previsualizacionMedia{std::make_shared<ServicioPrevisualizacionMediaV2>()};
    std::shared_ptr<ServicioFusionCarpetasV2> fusionCarpetas{std::make_shared<ServicioFusionCarpetasV2>()};
    std::shared_ptr<ServicioInspeccionCarpetasV2> inspeccionCarpetas{std::make_shared<ServicioInspeccionCarpetasV2>()};
    std::shared_ptr<ServicioReproductorVideoV2> reproductorVideo{std::make_shared<ServicioReproductorVideoV2>()};
    std::shared_ptr<ServicioComparadorVideosV2> comparadorVideos{std::make_shared<ServicioComparadorVideosV2>(reproductorVideo)};
};
}
