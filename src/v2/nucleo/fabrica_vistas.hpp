#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/nucleo/enrutador_vistas.hpp"
#include "v2/vistas/vista_walkthrough.hpp"
#include "v2/vistas/vista_comprimidos.hpp"
#include "v2/vistas/vista_comics.hpp"
#include "v2/vistas/vista_carpetas.hpp"
#include "v2/vistas/vista_en_desarrollo.hpp"
#include "v2/vistas/vista_medios.hpp"
#include "v2/vistas/vista_texto_codigo.hpp"
#include "v2/vistas/vista_comparador_videos.hpp"
#include "v2/nucleo/servicios_vistas.hpp"
#include "v2/nucleo/configuracion_v2.hpp"
#include <memory>
#include <vector>
#include <string>

namespace SideViewV2 {

class FabricaVistas {
public:
    static DependenciasVistaV2& Dependencias() {
        static DependenciasVistaV2 dependencias;
        return dependencias;
    }

    static std::shared_ptr<IPanelContenido> CrearVistaComparadorVideos(
        const std::vector<std::wstring>& seleccion = {}) {
        auto vista = std::make_shared<VistaComparadorVideos>(Dependencias().comparadorVideos);
        vista->AlCambiarSeleccion(seleccion, L"");
        return vista;
    }

    static std::shared_ptr<IPanelContenido> CrearVistaParaSeleccion(
        const std::vector<std::wstring>& seleccion,
        const std::wstring& carpeta,
        bool excluirComparadorExperimental = false) {

        TipoVistaContenido tipo = ClasificarSeleccion(seleccion, carpeta);

        if (!excluirComparadorExperimental &&
            tipo == TipoVistaContenido::Video && seleccion.size() == 2 &&
            ComparadorVideosExperimentalDisponibleV2()) {
            return CrearVistaComparadorVideos(seleccion);
        }

        std::shared_ptr<IPanelContenido> vista;

        switch (tipo) {
            case TipoVistaContenido::Tutorial:
                vista = std::make_shared<VistaWalkthrough>(Dependencias().enlaces);
                break;

            case TipoVistaContenido::Comprimidos:
                vista = std::make_shared<VistaComprimidos>(Dependencias().abrirConComprimidos, Dependencias().navegacion, Dependencias().extensiones, Dependencias().contenidoComprimido);
                break;

            case TipoVistaContenido::Comics:
                vista = std::make_shared<VistaComics>(Dependencias().extensiones, Dependencias().navegacion, Dependencias().previsualizacionMedia);
                break;

            case TipoVistaContenido::Carpetas:
                vista = std::make_shared<VistaCarpetas>(Dependencias().abrirConCarpetas,
                    Dependencias().navegacion, Dependencias().fusionCarpetas,
                    Dependencias().inspeccionCarpetas, Dependencias().comparadorVideos);
                break;

            case TipoVistaContenido::Texto:
                vista = std::make_shared<VistaTextoCodigo>(Dependencias().texto, Dependencias().navegacion, false);
                break;

            case TipoVistaContenido::Imagen:
                vista = std::make_shared<VistaImagenes>(Dependencias().navegacion, Dependencias().abrirConImagenes, Dependencias().previsualizacionMedia);
                break;

            case TipoVistaContenido::Video:
                vista = std::make_shared<VistaVideos>(Dependencias().navegacion, Dependencias().abrirConVideos, Dependencias().previsualizacionMedia);
                break;

            case TipoVistaContenido::Documentos:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de Documentos de Oficina",
                    L"📑",
                    L"Visualización embebida de archivos PDF, Word (DOCX), Excel (XLSX) y presentaciones (PPTX)."
                );
                break;

            case TipoVistaContenido::Codigo:
                vista = std::make_shared<VistaTextoCodigo>(Dependencias().texto, Dependencias().navegacion, true);
                break;

            default:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Previsualizador Universal",
                    L"🔍",
                    L"Inspección de metadatos de archivos del sistema y delegado IPreviewHandler."
                );
                break;
        }

        if (vista) {
            vista->AlCambiarSeleccion(seleccion, carpeta);
        }
        return vista;
    }

};

} // namespace SideViewV2
