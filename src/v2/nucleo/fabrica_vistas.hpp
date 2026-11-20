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
#include "v2/vistas/vista_comparador_imagenes.hpp"
#include "v2/vistas/vista_comparador_archivos.hpp"
#include "v2/vistas/vista_comparador_texto.hpp"
#include "v2/vistas/vista_comparador_documentos.hpp"
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

    static bool SeleccionaDosDelMismoTipo(
        const std::vector<std::wstring>& seleccion,
        TipoVistaContenido* tipoResultado = nullptr) {
        return PuedeCompararSeleccionV2(seleccion, tipoResultado);
    }

    static bool DebeCrearComparador(
        const std::vector<std::wstring>& seleccion) {
        TipoVistaContenido tipo{};
        if (!SeleccionaDosDelMismoTipo(seleccion, &tipo)) return false;
        return tipo != TipoVistaContenido::Video ||
               ComparadorVideosExperimentalDisponibleV2();
    }

    static std::shared_ptr<IPanelContenido> CrearComparadorParaSeleccion(
        const std::vector<std::wstring>& seleccion,
        const std::wstring& carpeta) {
        if (!DebeCrearComparador(seleccion)) return nullptr;

        TipoVistaContenido tipo{};
        if (!SeleccionaDosDelMismoTipo(seleccion, &tipo)) return nullptr;
        if (tipo == TipoVistaContenido::Video) {
            return CrearVistaComparadorVideos(seleccion);
        }
        if (tipo == TipoVistaContenido::Imagen) {
            auto vista = std::make_shared<VistaComparadorImagenesV2>(
                Dependencias().comparadorImagenes);
            vista->AlCambiarSeleccion(seleccion, carpeta);
            return vista;
        }
        if (tipo == TipoVistaContenido::Texto ||
            tipo == TipoVistaContenido::Codigo) {
            auto vista = std::make_shared<VistaComparadorTextoV2>(
                Dependencias().comparadorTexto);
            vista->AlCambiarSeleccion(seleccion, carpeta);
            return vista;
        }
        if (tipo == TipoVistaContenido::Documentos) {
            auto vista = std::make_shared<VistaComparadorDocumentosV2>(
                Dependencias().comparadorDocumentos);
            vista->AlCambiarSeleccion(seleccion, carpeta);
            return vista;
        }
        auto vista = std::make_shared<VistaComparadorArchivosV2>(
            Dependencias().comparadorArchivos);
        vista->AlCambiarSeleccion(seleccion, carpeta);
        return vista;
    }

    static std::shared_ptr<IPanelContenido> CrearVistaParaSeleccion(
        const std::vector<std::wstring>& seleccion,
        const std::wstring& carpeta) {

        TipoVistaContenido tipo = ClasificarSeleccion(seleccion, carpeta);

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
                    Dependencias().inspeccionCarpetas, Dependencias().versionMod,
                    Dependencias().notas);
                break;

            case TipoVistaContenido::Texto:
                vista = std::make_shared<VistaTextoCodigo>(Dependencias().texto, Dependencias().navegacion, false);
                break;

            case TipoVistaContenido::Imagen:
                vista = std::make_shared<VistaImagenes>(Dependencias().navegacion, Dependencias().abrirConImagenes, Dependencias().previsualizacionMedia);
                break;

            case TipoVistaContenido::Video:
                vista = std::make_shared<VistaVideos>(
                    Dependencias().navegacion,
                    Dependencias().abrirConVideos,
                    Dependencias().previsualizacionMedia,
                    Dependencias().reproductorVideo);
                break;

            case TipoVistaContenido::Documentos:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de Documentos de Oficina",
                    L"📑",
                    L"Visualización embebida de archivos PDF y Word (DOC/DOCX).",
                    true
                );
                break;

            case TipoVistaContenido::Presentaciones:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de diapositivas",
                    L"📊",
                    L"Vista de diapositivas para archivos PowerPoint (PPT/PPTX/PPS/PPSX)."
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
