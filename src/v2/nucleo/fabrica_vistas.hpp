#pragma once

#include "v2/ui/panel_interfaz.hpp"
#include "v2/nucleo/enrutador_vistas.hpp"
#include "v2/vistas/vista_walkthrough.hpp"
#include "v2/vistas/vista_comprimidos.hpp"
#include "v2/vistas/vista_carpetas.hpp"
#include "v2/vistas/vista_en_desarrollo.hpp"
#include <memory>
#include <vector>
#include <string>

namespace SideViewV2 {

class FabricaVistas {
public:
    static std::shared_ptr<IPanelContenido> CrearVistaParaSeleccion(
        const std::vector<std::wstring>& seleccion,
        const std::wstring& carpeta) {

        TipoVistaContenido tipo = ClasificarSeleccion(seleccion, carpeta);

        std::shared_ptr<IPanelContenido> vista;

        switch (tipo) {
            case TipoVistaContenido::Tutorial:
                vista = std::make_shared<VistaWalkthrough>();
                break;

            case TipoVistaContenido::Comprimidos:
                vista = std::make_shared<VistaComprimidos>();
                break;

            case TipoVistaContenido::Carpetas:
                vista = std::make_shared<VistaCarpetas>();
                break;

            case TipoVistaContenido::Texto:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de Texto & Markdown",
                    L"📄",
                    L"Renderizado GitHub Flavored Markdown con tablas, listas de verificación, bloques de código sombreados e índice vertical interactivo."
                );
                break;

            case TipoVistaContenido::Imagen:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de Imágenes & GIFs en Bucle",
                    L"🖼️",
                    L"Decodificación acelerada por hardware de WebP, AVIF, PNG, JPG y GIFs animados en reproducción continua infinita."
                );
                break;

            case TipoVistaContenido::Video:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Suite de Reproducción de Video",
                    L"🎬",
                    L"Reproductor multimedia interactivo por Media Foundation con búsqueda precisa, selector de velocidad y línea de tiempo."
                );
                break;

            case TipoVistaContenido::Documentos:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Visor de Documentos de Oficina",
                    L"📑",
                    L"Visualización embebida de archivos PDF, Word (DOCX), Excel (XLSX) y presentaciones (PPTX)."
                );
                break;

            case TipoVistaContenido::Codigo:
                vista = std::make_shared<VistaEnDesarrollo>(
                    L"Inspector de Código Fuente",
                    L"💻",
                    L"Resaltado de sintaxis moderno para más de 30 lenguajes de programación y numeración de líneas."
                );
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
