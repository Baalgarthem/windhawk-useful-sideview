// ==WindhawkMod==
// @id              useful-sideview
// @name            Useful Side View
// @description     Transforma el panel de vista previa de Windows 11 en una potente suite multimedia: reproductor de video con barra interactiva, visor de imágenes con soporte WebP/GIF en loop, lector nativo de Markdown estilo GitHub y suite para cómics/comprimidos con extracción de portadas, conversor y 'Abrir con' inteligente.
// @version         2.0.1
// @author          Baalgarthem
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lmfplay -lmfplat -lmfuuid -lshlwapi -lgdi32 -luuid -lgdiplus -lwindowscodecs -lurlmon -lcabinet -lcomdlg32 -lcomctl32
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Useful Side View

**Useful Side View** transforma el panel de vista previa lateral de Windows 11 (`Alt + P`) en un centro de previsualización integral, rápido, responsivo y enriquecido para todo tipo de archivos.

---

##  Principales Virtudes y Capacidades

### 1. 🎬 Reproductor de Video Completo y Fluido
- **Compatibilidad Extensa**: Soporta `.mp4`, `.mkv`, `.avi`, `.mov`, `.wmv`, `.webm`, `.flv`, `.m4v`, `.ts`, `.mts` y formatos Media Foundation.
- **Controles Fluent Design**: Play/Pausa al hacer clic en el video o en los controles, silenciado/audio, repetición en bucle (*loop*) y selector de velocidad (0.5x, 0.75x, 1.0x, 1.25x, 1.5x, 2.0x).
- **Línea de Tiempo Interactiva**: Búsqueda precisa por clic y arrastre fluido con cálculo temporal exacto.
- **Navegación de Carpeta**: Botones para saltar al video anterior o siguiente de forma inmediata.

### 2. 🖼️ Visor de Imágenes de Alta Compatibilidad y GIFs en Loop
- **Formatos Modernos y Tradicionales**: Soporta `.png`, `.jpg`, `.jpeg`, `.webp`, `.gif`, `.bmp`, `.tiff`, `.tif`, `.avif`, `.heic`, `.jfif` e `.ico`.
- **Reproducción de GIFs en Bucle Continuo**: Detección automática de fotogramas y tiempos de retardo nativos para reproducción fluida e infinita.
- **Navegación Intuitiva**: Botones anterior/siguiente integrados y atajos de teclado para recorrer las imágenes de la carpeta.

### 3. 📦 Suite Integral para Cómics y Archivos Comprimidos
- **Cómics y Archivos Soportados**: Cómics (`.cbz`, `.cbr`) y comprimidos convencionales (`.zip`, `.rar`, `.7z`, `.tar`, `.iso`).
- **Extracción de Portadas de Alta Fidelidad**: Carga de miniaturas nativas de Explorer e inspección interna Deflate/ZIP para mostrar la primera página del cómic.
- **Cuadrícula Adaptativa y Contador `+N`**: Visualización de selecciones múltiples en cuadrículas configurables de 4, 6, 8 o 10 archivos con insignia flotante para elementos excedentes.
- **Selector 'Abrir con' Inteligente y Apertura Secuencial**:
  - Distingue automáticamente entre lectores de cómics y descompresores comunes según la selección.
  - Abre en orden todos los archivos seleccionados con el programa memorizado o elegido.
- **Conversor y Registro en Tiempo Real**: Menú desplegable para conversión rápida de formatos con recuadro de log scrolleable y marcas de tiempo.

### 4. 📄 Visor de Archivos de Texto (.md y .txt)
- **Sintaxis Estándar y Estética GitHub**: Renderizado nativo de encabezados H1 a H4 con divisores visuales, párrafos, bloques de código sombreados, citas en bloque, listas y casillas interactivas para Markdown.
- **Índice Vertical de Barras Interactivo (Exclusivo Markdown)**: Elegante barra vertical lateral que despliega títulos al pasar el cursor y permite salto instantáneo al hacer clic.
- **Soporte Masivo Virtualizado**: Viewport culling para archivos masivos sin trabar memoria ni rendimiento.
- **Barra de Navegación Inferior**: Botones '◀ Anterior' y 'Siguiente ▶' para navegar entre archivos de texto de la carpeta.
- **Multimedia y Enlaces Web**: Carga de imágenes locales y remotas, tarjetas interactivas de videos de YouTube con miniaturas y botón Play, e hipervínculos clicables en el navegador.

### 5. ⚙️ Personalización y Robustez de Sistema
- **Redimensionamiento Reactivo**: Interfaz fluida y redibujado instantáneo que se ajusta a cualquier tamaño del panel lateral.
- **Bloqueo Granular de Tamaño**: Posibilidad de fijar el tamaño de la vista previa de forma independiente para videos, imágenes, comprimidos o textos.
- **Cero Fugas de Memoria y Rendimiento Ligero**: Arquitectura en C++23 nativo con administración segura de recursos GDI, GDI+ y COM.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
# ==============================================================================
# 🎬 VIDEOS
# ==============================================================================
- autoplay: false
  $name: "[Videos] Reproducción automática"
  $description: "Inicia la reproducción del video de forma automática al seleccionarlo."
- sound: false
  $name: "[Videos] Audio habilitado por defecto"
  $description: "Activa el sonido del reproductor al cargar un video (inicia silenciado si está desactivado)."
- lock_preview_size_video: false
  $name: "[Videos] Bloquear tamaño de vista previa"
  $description: "Bloquea el redimensionamiento del panel para videos. Desactivado permite redimensionar libremente."

# ==============================================================================
# 🖼️ IMÁGENES Y GIFS
# ==============================================================================
- lock_preview_size_image: false
  $name: "[Imágenes] Bloquear tamaño de vista previa"
  $description: "Bloquea el redimensionamiento del panel para imágenes y GIFs animados. Desactivado permite redimensionar libremente."

# ==============================================================================
# 📄 TEXTO Y MARKDOWN
# ==============================================================================
- lock_preview_size_text: false
  $name: "[Texto y Markdown] Bloquear tamaño de vista previa"
  $description: "Bloquea el redimensionamiento del panel para archivos .txt y .md. Desactivado permite redimensionar libremente."

# ==============================================================================
# 📑 DOCUMENTOS (PDF Y WORD)
# ==============================================================================
- lock_preview_size_documents: false
  $name: "[Documentos] Bloquear tamaño de vista previa"
  $description: "Bloquea el redimensionamiento del panel para documentos PDF, Word y ofimática (.pdf, .docx, .doc, etc.). Desactivado permite redimensionar libremente."

# ==============================================================================
# 💻 CÓDIGOS DE PROGRAMACIÓN
# ==============================================================================
- lock_preview_size_code: false
  $name: "[Código] Bloquear tamaño de vista previa"
  $description: "Bloquea el redimensionamiento del panel para archivos de código (.cpp, .py, .js, .ts, .cs, .html, etc.). Desactivado permite redimensionar libremente."

# ==============================================================================
# 📦 CÓMICS Y COMPRIMIDOS
# ==============================================================================
- lock_preview_size_compressed: false
  $name: "[Comprimidos] Bloquear tamaño de vista previa"
  $description: "Bloquea el redimensionamiento del panel para comprimidos y cómics. Desactivado permite redimensionar libremente."
- compressed_grid_limit: 4
  $name: "[Comprimidos] Límite de archivos en cuadrícula"
  $description: "Número máximo de portadas/elementos mostrados en la cuadrícula de previsualización al seleccionar múltiples archivos."
  $options:
    - 4: "4 archivos (2x2)"
    - 6: "6 archivos (2x3)"
    - 8: "8 archivos (2x4)"
    - 10: "10 archivos (2x5)"
- comic_default_program: ""
  $name: "[Cómics] Programa por defecto para cómics (.cbz, .cbr)"
  $description: "Programa preferido para abrir cómics (ej. C:\\Program Files\\CDisplayEx\\CDisplayEx.exe). Si solo hay 1 programa en la lista de recordados, este se marcará automáticamente por defecto."
- comic_remembered_programs: ""
  $name: "[Cómics] Programas recordados para cómics (rutas separadas por ;)"
  $description: "Lista de aplicaciones para cómics disponibles en el selector 'Abrir con' (ej. ruta1.exe;ruta2.exe). Cuando solamente hay un programa recordado, se marcará automáticamente por defecto."
- compressed_default_program: ""
  $name: "[Comprimidos] Programa por defecto para comprimidos comunes (.zip, .rar, .7z)"
  $description: "Programa preferido para abrir comprimidos comunes (ej. C:\\Program Files\\7-Zip\\7zFM.exe). Si solo hay 1 programa en la lista de recordados, este se marcará automáticamente por defecto."
- compressed_remembered_programs: ""
  $name: "[Comprimidos] Programas recordados para comprimidos (rutas separadas por ;)"
  $description: "Lista de aplicaciones descompresoras disponibles en el selector 'Abrir con' (ej. ruta1.exe;ruta2.exe). Cuando solamente hay un programa recordado, se marcará automáticamente por defecto."

# ==============================================================================
# 📁 VISTA DE CARPETAS (AL SELECCIONAR CARPETAS)
# ==============================================================================
- lock_preview_size_folder: false
  $name: "[Vista de Carpetas] Bloquear tamaño de vista previa de carpetas"
  $description: "Bloquea el redimensionamiento del panel en la vista de carpetas seleccionadas. Desactivado permite redimensionar libremente."
- folder_tree_expanded_by_default: false
  $name: "[Vista de Carpetas] Iniciar en nivel raíz por defecto"
  $description: "Por defecto la vista de carpetas inicia en nivel raíz. Si se activa, iniciará con todos los niveles expandidos."
- folder_tree_interactive_files: false
  $name: "[Vista de Carpetas] Permitir interacción con archivos en la estructura"
  $description: "Habilita la interacción directa con los archivos y subcarpetas mostrados en el árbol de la carpeta seleccionada (doble clic para abrir el archivo, y clic derecho para mostrar opciones de renombrar y eliminar)."
- folder_default_program: ""
  $name: "[Vista de Carpetas] Programa por defecto para abrir carpetas"
  $description: "Programa preferido para abrir carpetas seleccionadas en 'Abrir carpeta con' (ej. Code.exe, Explorer.exe). Si solo hay 1 programa en la lista de recordados, este se marcará automáticamente por defecto."
- folder_remembered_programs: ""
  $name: "[Vista de Carpetas] Programas recordados para carpetas (rutas separadas por ;)"
  $description: "Lista de programas recordados disponibles en el selector 'Abrir carpeta con' (rutas separadas por ;). Cuando solamente hay un programa recordado, se marcará automáticamente por defecto."

# ==============================================================================
# 📂 VISTA VACÍA (SIN SELECCIÓN)
# ==============================================================================
- lock_preview_size_empty_view: false
  $name: "[Vista Vacía] Bloquear tamaño de vista previa sin selección"
  $description: "Bloquea el redimensionamiento del panel en la vista vacía cuando no hay ningún elemento seleccionado en el Explorador. Desactivado permite redimensionar libremente."
- empty_view_threshold_low_medium: 40
  $name: "[Vista Vacía - Top 4] Umbral porcentaje rango bajo/medio (%)"
  $description: "Porcentaje del peso total a partir del cual una subcarpeta pasa de rango bajo a rango medio (por defecto: 40%). Subcarpetas por debajo de este valor usan el color de rango bajo."
- empty_view_threshold_medium_high: 70
  $name: "[Vista Vacía - Top 4] Umbral porcentaje rango medio/alto (%)"
  $description: "Porcentaje del peso total a partir del cual una subcarpeta pasa de rango medio a rango alto (por defecto: 70%). Debe ser mayor que el umbral bajo/medio."
- empty_view_color_low: "#3FB950"
  $name: "[Vista Vacía - Top 4] Color rango bajo (< 40%)"
  $description: "Color en formato HEX (#3FB950) o RGB (63, 185, 80) para subcarpetas que ocupan menos del porcentaje bajo."
- empty_view_color_medium: "#D29922"
  $name: "[Vista Vacía - Top 4] Color rango medio (40% - 70%)"
  $description: "Color en formato HEX (#D29922) o RGB (210, 153, 34) para subcarpetas en el rango medio."
- empty_view_color_high: "#F85149"
  $name: "[Vista Vacía - Top 4] Color rango alto (>= 70%)"
  $description: "Color en formato HEX (#F85149) o RGB (248, 81, 73) para subcarpetas que superan el porcentaje alto."
*/
// ==/WindhawkModSettings==

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <gdiplus.h>
#include <wincodec.h>
#include <mfapi.h>
#include <mfplay.h>
#include <exdisp.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <shlwapi.h>
#include <algorithm>
#include <atomic>
#include <map>
#include <new>
#include <string>
#include <vector>

namespace {

// El orden documenta las dependencias y permite producir una sola unidad de
// traducción, requisito del formato distribuible de Windhawk.
//
// clasificador_archivos.hpp clasifica tipos de archivos y extensiones.
// controlador_imagen.hpp gestiona carga, dimensionado y renderizado con GDI+.
// controlador_texto.hpp y controlador_comprimidos.hpp son módulos plantilla.
// controles.cpp define buscarEnReproduccion y la caché de tiempo.
// ventanas.cpp define el delegado del hilo reproductor (HWND_MESSAGE).
// panel_previsualizacion.cpp crea las ventanas visibles.
// hilo_reproductor.cpp posee IMFPMediaPlayer y actualiza la caché.
#include "nucleo/constantes.hpp"
#include "nucleo/configuracion.hpp"
#include "archivos/clasificador_archivos.hpp"
#include "nucleo/estado.hpp"
#include "nucleo/puntero_com.hpp"
#include "archivos/seleccion_explorador.cpp"
#include "imagen/controlador_imagen.hpp"
#include "texto/controlador_texto.hpp"
#include "comprimidos/controlador_comprimidos.hpp"
#include "carpetas/controlador_carpetas.hpp"
#include "interfaz/diseno_barra_progreso.hpp"
#include "reproductor/calculos_tiempo.hpp"
#include "reproductor/conversion_tiempo_media_foundation.hpp"
#include "reproductor/controles.cpp"
#include "reproductor/eventos_media_foundation.cpp"
#include "interfaz/renderizado.cpp"
#include "interfaz/ventanas.cpp"
#include "integracion/panel_previsualizacion.cpp"
#include "ejecucion/hilo_reproductor.cpp"

}  // namespace

#include "windhawk/ciclo_vida.cpp"
