/**** start inlining useful-sideview.cpp ****/
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
/**** start inlining nucleo/constantes.hpp ****/

constexpr wchar_t CLASE_VENTANA_REPRODUCTOR[] = L"WindhawkUsefulSideView";
constexpr wchar_t CLASE_VENTANA_VIDEO[] = L"WindhawkUsefulSideViewVideo";
constexpr wchar_t CLASE_VENTANA_CONTROLES[] = L"WindhawkUsefulSideViewControls";
constexpr wchar_t CLASE_VENTANA_DELEGADO[] = L"WindhawkUsefulSideViewDelegate";
constexpr UINT_PTR ID_TEMPORIZADOR_INTERFAZ = 1;
constexpr UINT_PTR ID_TEMPORIZADOR_ANIMACION_GIF = 2;
constexpr UINT INTERVALO_REFRESCO_MS = 200;
constexpr int ALTO_CONTROLES = 96;
constexpr int TAMANO_BOTON_CONTROL = 34;
constexpr int SEPARACION_BOTONES = 8;
constexpr int CANTIDAD_BOTONES_CENTRALES = 5;
constexpr int ANCHO_GRUPO_BOTONES =
    TAMANO_BOTON_CONTROL * CANTIDAD_BOTONES_CENTRALES +
    SEPARACION_BOTONES * (CANTIDAD_BOTONES_CENTRALES - 1);
constexpr int MARGEN_BARRA_PROGRESO = 16;
constexpr int RADIO_PERILLA_PROGRESO = 6;
constexpr double PASO_BARRA_PROGRESO_TECLADO = 0.05;
constexpr UINT MENSAJE_ACTUALIZAR_INTERFAZ = WM_APP + 1;
constexpr UINT MENSAJE_ACTUALIZAR_CACHE_TIEMPO = WM_APP + 2;
constexpr UINT MENSAJE_EJECUTAR_BUSQUEDA = WM_APP + 3;

constexpr wchar_t GLIFO_REPRODUCIR[] = L"\uE768";
constexpr wchar_t GLIFO_PAUSAR[] = L"\uE769";
constexpr wchar_t GLIFO_ANTERIOR[] = L"\uE892";
constexpr wchar_t GLIFO_SIGUIENTE[] = L"\uE893";
constexpr wchar_t GLIFO_VOLUMEN[] = L"\uE767";
constexpr wchar_t GLIFO_SILENCIO[] = L"\uE74F";
constexpr wchar_t GLIFO_REPETICION[] = L"\uE8EE";

constexpr wchar_t CLASE_VENTANA_MENU_VELOCIDAD[] =
    L"WindhawkUsefulSideViewSpeedMenu";

constexpr int ANCHO_BOTON_VELOCIDAD = 46;
constexpr int ALTO_BOTON_VELOCIDAD = 26;
constexpr int ANCHO_PANEL_VELOCIDAD = 64;
constexpr int ALTO_OPCION_VELOCIDAD = 24;
constexpr int TOTAL_OPCIONES_VELOCIDAD = 6;
constexpr int ALTO_PANEL_VELOCIDAD =
    ALTO_OPCION_VELOCIDAD * TOTAL_OPCIONES_VELOCIDAD + 8;
constexpr float VALORES_VELOCIDAD[TOTAL_OPCIONES_VELOCIDAD] = {
    0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 2.0f};
constexpr const wchar_t* ETIQUETAS_VELOCIDAD[TOTAL_OPCIONES_VELOCIDAD] = {
    L"0.5x", L"0.75x", L"1.0x", L"1.25x", L"1.5x", L"2.0x"};


constexpr int ALTO_CONTROLES_COMPRIMIDOS = 104;
constexpr int TOTAL_FORMATOS_DESTINO = 5;
constexpr const wchar_t* FORMATOS_DESTINO[TOTAL_FORMATOS_DESTINO] = {
    L"CBZ (.cbz)", L"ZIP (.zip)", L"CBR (.cbr)", L"7Z (.7z)", L"TAR (.tar)"};
constexpr const wchar_t* EXTENSIONES_DESTINO[TOTAL_FORMATOS_DESTINO] = {
    L".cbz", L".zip", L".cbr", L".7z", L".tar"};

constexpr COLORREF COLOR_FONDO_REPRODUCTOR = RGB(15, 15, 15);

constexpr int ALTO_METADATOS_VIDEO = 90;
constexpr UINT_PTR ID_TEMPORIZADOR_PARPADEO_NAV = 1005;

constexpr wchar_t GLIFO_COMPRIMIDO[] = L"\uE8B7";
constexpr wchar_t GLIFO_CHEVRON_ARRIBA[] = L"\uE70E";


/**** ended inlining nucleo/constantes.hpp ****/
/**** start inlining nucleo/configuracion.hpp ****/

#include <string>
#include <vector>
#include <atomic>

enum class TipoContenidoArchivo;

struct Configuracion {
    // Windhawk notifica cambios desde otro hilo. Los atómicos evitan bloquear
    // el camino crítico del reproductor y eliminan carreras de datos.
    std::atomic_bool reproduccionAutomatica{false};
    std::atomic_bool sonidoHabilitado{false};
    std::atomic_bool bloquearTamanoVideo{false};
    std::atomic_bool bloquearTamanoImagen{false};
    std::atomic_bool bloquearTamanoComprimidos{false};
    std::atomic_bool bloquearTamanoTexto{false};
    std::atomic_bool bloquearTamanoDocumentos{false};
    std::atomic_bool bloquearTamanoCodigo{false};
    std::atomic_bool bloquearTamanoVistaCarpetas{false};
    std::atomic_bool bloquearTamanoVistaVacia{false};
    std::atomic_bool bloquearTamanoDesconocido{false};
    std::atomic<int> limiteCuadriculaComprimidos{4};
    std::wstring programaComicPorDefecto;
    std::vector<std::wstring> programasComicsRecordados;
    std::wstring programaComprimidosPorDefecto;
    std::vector<std::wstring> programasComprimidosRecordados;
    std::wstring programaCarpetasPorDefecto;
    std::vector<std::wstring> programasCarpetasRecordados;
    std::atomic_bool interaccionArchivosVistaCarpetas{false};
    std::atomic_bool arbolCarpetasExpandidoPorDefecto{false};
    std::atomic<int> umbralPorcentajeBajoMedioVistaVacia{40};
    std::atomic<int> umbralPorcentajeMedioAltoVistaVacia{70};
    std::atomic<COLORREF> colorRangoBajoVistaVacia{RGB(63, 185, 80)};
    std::atomic<COLORREF> colorRangoMedioVistaVacia{RGB(210, 153, 34)};
    std::atomic<COLORREF> colorRangoAltoVistaVacia{RGB(248, 81, 73)};
};

/**** ended inlining nucleo/configuracion.hpp ****/
/**** start inlining archivos/clasificador_archivos.hpp ****/

#include <shlwapi.h>
#include <string>

enum class TipoContenidoArchivo {
    Ninguno,
    Video,
    Imagen,
    Texto,
    Documento,
    Codigo,
    Comprimido,
    VistaCarpetas,
    VistaVacia,
    Desconocido
};

namespace {

inline void normalizarExtensionAMinusculas(const wchar_t* origen, wchar_t* destino, size_t capacidad) {
    if (!origen || !destino || capacidad == 0) return;
    size_t indiceCaracter = 0;
    while (origen[indiceCaracter] != L'\0' && indiceCaracter + 1 < capacidad) {
        destino[indiceCaracter] = towlower(origen[indiceCaracter]);
        ++indiceCaracter;
    }
    destino[indiceCaracter] = L'\0';
}

inline bool esExtensionDeListaOrdenada(const wchar_t* extensionMin,
                                      const wchar_t* const* listaOrdenada,
                                      size_t cantidad) {
    if (!extensionMin || *extensionMin == L'\0' || cantidad == 0) return false;
    return std::binary_search(
        listaOrdenada, listaOrdenada + cantidad, extensionMin,
        [](const wchar_t* extensionA, const wchar_t* extensionB) {
            return wcscmp(extensionA, extensionB) < 0;
        });
}

}  // namespace

inline bool esExtensionVideo(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t extMin[32]{};
    normalizarExtensionAMinusculas(extension, extMin, ARRAYSIZE(extMin));
    static const wchar_t* const EXTENSIONES_VIDEO[] = {
        L".3gp", L".avi", L".flv", L".m2ts", L".m4v",
        L".mkv", L".mov", L".mp4", L".mpeg", L".mpg",
        L".mts", L".ts",  L".vob", L".webm", L".wmv"};
    return esExtensionDeListaOrdenada(extMin, EXTENSIONES_VIDEO,
                                      ARRAYSIZE(EXTENSIONES_VIDEO));
}

inline bool esExtensionImagen(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t extMin[32]{};
    normalizarExtensionAMinusculas(extension, extMin, ARRAYSIZE(extMin));
    static const wchar_t* const EXTENSIONES_IMAGEN[] = {
        L".avif", L".bmp",  L".emf", L".gif",  L".heic",
        L".ico",  L".jfif", L".jpeg", L".jpg", L".png",
        L".svg",  L".tif",  L".tiff", L".webp", L".wmf"};
    return esExtensionDeListaOrdenada(extMin, EXTENSIONES_IMAGEN,
                                      ARRAYSIZE(EXTENSIONES_IMAGEN));
}

inline bool esExtensionMarkdown(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t extMin[32]{};
    normalizarExtensionAMinusculas(extension, extMin, ARRAYSIZE(extMin));
    static const wchar_t* const EXTENSIONES_MARKDOWN[] = {
        L".markdown", L".md", L".mdown", L".mkd", L".mkdn"};
    return esExtensionDeListaOrdenada(extMin, EXTENSIONES_MARKDOWN,
                                      ARRAYSIZE(EXTENSIONES_MARKDOWN));
}

inline bool esExtensionTexto(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t extMin[32]{};
    normalizarExtensionAMinusculas(extension, extMin, ARRAYSIZE(extMin));
    static const wchar_t* const EXTENSIONES_TEXTO[] = {
        L".log", L".markdown", L".md", L".mdown", L".mkd", L".mkdn", L".txt"};
    return esExtensionDeListaOrdenada(extMin, EXTENSIONES_TEXTO,
                                      ARRAYSIZE(EXTENSIONES_TEXTO));
}

inline bool esExtensionDocumento(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t extMin[32]{};
    normalizarExtensionAMinusculas(extension, extMin, ARRAYSIZE(extMin));
    static const wchar_t* const EXTENSIONES_DOCUMENTO[] = {
        L".doc",  L".docm", L".docx", L".dot",  L".dotm",
        L".dotx", L".epub", L".odt",  L".oxps", L".pdf",
        L".rtf",  L".wps",  L".xps"};
    return esExtensionDeListaOrdenada(extMin, EXTENSIONES_DOCUMENTO,
                                      ARRAYSIZE(EXTENSIONES_DOCUMENTO));
}

inline bool esExtensionCodigo(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t extMin[32]{};
    normalizarExtensionAMinusculas(extension, extMin, ARRAYSIZE(extMin));
    static const wchar_t* const EXTENSIONES_CODIGO[] = {
        L".asm",  L".bat",   L".c",     L".cc",    L".cfg",   L".cjs",
        L".cmd",  L".conf",  L".cpp",   L".cs",    L".css",   L".cxx",
        L".dart", L".env",   L".fs",    L".go",    L".h",     L".hpp",
        L".hs",   L".htm",   L".html",  L".hxx",   L".ini",   L".inl",
        L".ipynb", L".java", L".js",    L".json",  L".jsx",   L".kt",
        L".kts",  L".less",  L".lua",   L".mjs",   L".php",   L".pl",
        L".pm",   L".ps1",   L".psd1",  L".psm1",  L".py",    L".pyw",
        L".r",    L".rb",    L".rs",    L".s",     L".sass",  L".scala",
        L".scss", L".sh",    L".sql",   L".svelte", L".swift", L".toml",
        L".ts",   L".tsx",   L".vb",    L".vbs",   L".vue",   L".xml",
        L".yaml", L".yml",   L".zig",   L".zsh"};
    return esExtensionDeListaOrdenada(extMin, EXTENSIONES_CODIGO,
                                      ARRAYSIZE(EXTENSIONES_CODIGO));
}

inline bool esExtensionComic(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t extMin[32]{};
    normalizarExtensionAMinusculas(extension, extMin, ARRAYSIZE(extMin));
    static const wchar_t* const EXTENSIONES_COMIC[] = {
        L".cb7", L".cba", L".cbr", L".cbt", L".cbz"};
    return esExtensionDeListaOrdenada(extMin, EXTENSIONES_COMIC,
                                      ARRAYSIZE(EXTENSIONES_COMIC));
}

inline bool esExtensionComprimido(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t extMin[32]{};
    normalizarExtensionAMinusculas(extension, extMin, ARRAYSIZE(extMin));
    static const wchar_t* const EXTENSIONES_COMPRIMIDO[] = {
        L".7z",  L".bz2", L".cab", L".cb7", L".cba",
        L".cbr", L".cbt", L".cbz", L".gz",  L".iso",
        L".rar", L".tar", L".tgz", L".xz",  L".zip"};
    return esExtensionDeListaOrdenada(extMin, EXTENSIONES_COMPRIMIDO,
                                      ARRAYSIZE(EXTENSIONES_COMPRIMIDO));
}

inline bool esRutaCarpeta(const wchar_t* rutaArchivo) {
    if (!rutaArchivo || *rutaArchivo == L'\0') {
        return false;
    }
    if (wcsncmp(rutaArchivo, L"::SISTEMA::", 11) == 0) {
        return true;
    }
    DWORD atributos = GetFileAttributesW(rutaArchivo);
    return (atributos != INVALID_FILE_ATTRIBUTES &&
            (atributos & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

inline TipoContenidoArchivo clasificarTipoArchivo(const wchar_t* rutaArchivo) {
    if (!rutaArchivo || *rutaArchivo == L'\0') {
        return TipoContenidoArchivo::Ninguno;
    }
    if (wcsncmp(rutaArchivo, L"::SISTEMA::", 11) == 0) {
        return TipoContenidoArchivo::VistaVacia;
    }
    if (esRutaCarpeta(rutaArchivo)) {
        return TipoContenidoArchivo::VistaCarpetas;
    }
    const wchar_t* extension = PathFindExtensionW(rutaArchivo);
    if (esExtensionVideo(extension)) {
        return TipoContenidoArchivo::Video;
    }
    if (esExtensionImagen(extension)) {
        return TipoContenidoArchivo::Imagen;
    }
    if (esExtensionDocumento(extension)) {
        return TipoContenidoArchivo::Documento;
    }
    if (esExtensionCodigo(extension)) {
        return TipoContenidoArchivo::Codigo;
    }
    if (esExtensionTexto(extension)) {
        return TipoContenidoArchivo::Texto;
    }
    if (esExtensionComprimido(extension)) {
        return TipoContenidoArchivo::Comprimido;
    }
    return TipoContenidoArchivo::Desconocido;
}

inline bool esArchivoCompatible(const wchar_t* rutaArchivo) {
    TipoContenidoArchivo tipo = clasificarTipoArchivo(rutaArchivo);
    return tipo != TipoContenidoArchivo::Ninguno &&
           tipo != TipoContenidoArchivo::VistaVacia;
}

inline TipoContenidoArchivo clasificarSeleccionArchivos(
    const std::vector<std::wstring>& listaArchivos) {
    if (listaArchivos.empty()) return TipoContenidoArchivo::Ninguno;
    if (listaArchivos.size() == 1) {
        return clasificarTipoArchivo(listaArchivos.front().c_str());
    }

    size_t conteoVideo = 0;
    size_t conteoImagen = 0;
    size_t conteoDocumento = 0;
    size_t conteoCodigo = 0;
    size_t conteoTexto = 0;
    size_t conteoComprimido = 0;
    size_t conteoCarpetas = 0;
    size_t conteoDesconocido = 0;

    for (const auto& ruta : listaArchivos) {
        TipoContenidoArchivo tipo = clasificarTipoArchivo(ruta.c_str());
        if (tipo == TipoContenidoArchivo::Video) conteoVideo++;
        else if (tipo == TipoContenidoArchivo::Imagen) conteoImagen++;
        else if (tipo == TipoContenidoArchivo::Documento) conteoDocumento++;
        else if (tipo == TipoContenidoArchivo::Codigo) conteoCodigo++;
        else if (tipo == TipoContenidoArchivo::Texto) conteoTexto++;
        else if (tipo == TipoContenidoArchivo::Comprimido) conteoComprimido++;
        else if (tipo == TipoContenidoArchivo::VistaCarpetas) conteoCarpetas++;
        else conteoDesconocido++;
    }

    size_t total = conteoVideo + conteoImagen + conteoDocumento +
                   conteoCodigo + conteoTexto + conteoComprimido + conteoCarpetas + conteoDesconocido;
    if (total == 0) return TipoContenidoArchivo::Desconocido;

    if (conteoComprimido == total) return TipoContenidoArchivo::Comprimido;
    if (conteoCarpetas == total) return TipoContenidoArchivo::VistaCarpetas;
    if (conteoImagen == total) return TipoContenidoArchivo::Imagen;
    if (conteoDocumento == total) return TipoContenidoArchivo::Documento;
    if (conteoCodigo == total) return TipoContenidoArchivo::Codigo;
    if (conteoVideo == total) return TipoContenidoArchivo::Video;
    if (conteoTexto == total) return TipoContenidoArchivo::Texto;
    if (conteoDesconocido == total) return TipoContenidoArchivo::Desconocido;

    if (conteoImagen > 0) return TipoContenidoArchivo::Imagen;
    if (conteoVideo > 0) return TipoContenidoArchivo::Video;
    if (conteoDocumento > 0) return TipoContenidoArchivo::Documento;
    if (conteoCodigo > 0) return TipoContenidoArchivo::Codigo;
    if (conteoComprimido > 0) return TipoContenidoArchivo::Comprimido;
    if (conteoTexto > 0) return TipoContenidoArchivo::Texto;
    if (conteoCarpetas > 0) return TipoContenidoArchivo::VistaCarpetas;
    return TipoContenidoArchivo::Desconocido;
}
/**** ended inlining archivos/clasificador_archivos.hpp ****/
/**** start inlining nucleo/estado.hpp ****/

#include <commctrl.h>

constexpr UINT_PTR ID_SUBCLASE_VISTA_ARCHIVOS = 1001;
constexpr UINT_PTR ID_SUBCLASE_VENTANA_EXPLORADOR = 1002;

Configuracion configuracion;
std::wstring rutaVideoActiva;
std::vector<std::wstring> listaVideosSeleccionados;
size_t indiceVideoSeleccionado = 0;
std::wstring rutaArchivoActivo;
std::atomic<TipoContenidoArchivo> tipoContenidoActivo{TipoContenidoArchivo::Ninguno};
ULONG_PTR tokenGdiplus = 0;

std::atomic_bool detenerHilo{false};
std::atomic_bool configuracionPendiente{true};
HANDLE hiloReproductor = nullptr;
HANDLE eventoDetener = nullptr;
HWND ventanaReproductor = nullptr;
HWND ventanaVideo = nullptr;
HWND ventanaControles = nullptr;
HWND ventanaDelegadoReproductor = nullptr;
HWND ventanaExploradorAnfitriona = nullptr;
HWND ventanaVistaArchivosActiva = nullptr;
IMFPMediaPlayer* reproductor = nullptr;
HFONT fuenteIconos = nullptr;
HFONT fuenteTexto = nullptr;
HFONT fuenteTextoPequeno = nullptr;

class ReceptorEventosMediaFoundation;
ReceptorEventosMediaFoundation* receptorEventos = nullptr;

// La barra pertenece al mismo hilo que su ventana, por lo que estas variables
// no necesitan atómicos ni sincronización adicional.
bool arrastrandoBarraProgreso = false;
double proporcionProgreso = 0.0;
std::atomic_bool cambioPosicionPendiente{false};
std::atomic_bool reproducirTrasCambioPosicion{false};
std::atomic_bool posicionamientoDisponible{false};
std::atomic_bool repeticionHabilitada{false};
std::atomic<double> proporcionPosicionSolicitada{0.0};
std::atomic<long long> posicionReproduccionCache{0};
std::atomic<long long> duracionReproduccionCache{0};
std::atomic_bool tiempoReproduccionDisponible{false};

std::atomic<float> velocidadReproduccionActual{1.0f};
std::atomic_bool navegacionManualEnCurso{false};
HWND ventanaMenuVelocidad = nullptr;
bool menuVelocidadDesplegado = false;
int opcionVelocidadResaltada = -1;
BYTE opacidadMenuVelocidad = 255;
bool animandoCierreMenuVelocidad = false;

struct EstadoVideoMetadatos {
    std::wstring rutaVideo;
    std::wstring nombreArchivo;
    std::wstring extensionFormato;
    std::wstring textoTamano;
    std::wstring textoDuracion;
    std::wstring textoFechaModificacion;
    int totalEnRango{0};
    int indiceEnRango{0};
    int totalEnCarpeta{0};
    int indiceEnCarpeta{0};
    bool esRangoSeleccionado{false};
    ULONGLONG tiempoInicioParpadeoNav{0};
};

inline EstadoVideoMetadatos estadoVideoMetadatos;

inline void liberarRecursosVideo() {
    estadoVideoMetadatos.rutaVideo.clear();
    estadoVideoMetadatos.nombreArchivo.clear();
    estadoVideoMetadatos.extensionFormato.clear();
    estadoVideoMetadatos.textoTamano.clear();
    estadoVideoMetadatos.textoDuracion.clear();
    estadoVideoMetadatos.textoFechaModificacion.clear();
    estadoVideoMetadatos.totalEnRango = 0;
    estadoVideoMetadatos.indiceEnRango = 0;
    estadoVideoMetadatos.totalEnCarpeta = 0;
    estadoVideoMetadatos.indiceEnCarpeta = 0;
    estadoVideoMetadatos.esRangoSeleccionado = false;
    estadoVideoMetadatos.tiempoInicioParpadeoNav = 0;
}

struct EstadoPendienteImplementacion {
    std::wstring rutaActual;
    std::wstring nombreArchivo;
    std::wstring extension;
    std::vector<std::wstring> archivosSeleccionados;
    size_t indiceActual{0};
    RECT botonAnterior{};
    RECT botonSiguiente{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};
};

inline EstadoPendienteImplementacion estadoDocumentos;
inline EstadoPendienteImplementacion estadoCodigo;

inline void liberarRecursosDocumentos() {
    estadoDocumentos.rutaActual.clear();
    estadoDocumentos.nombreArchivo.clear();
    estadoDocumentos.extension.clear();
    estadoDocumentos.archivosSeleccionados.clear();
    estadoDocumentos.indiceActual = 0;
    estadoDocumentos.hoverBotonAnterior = false;
    estadoDocumentos.hoverBotonSiguiente = false;
}

inline void liberarRecursosCodigo() {
    estadoCodigo.rutaActual.clear();
    estadoCodigo.nombreArchivo.clear();
    estadoCodigo.extension.clear();
    estadoCodigo.archivosSeleccionados.clear();
    estadoCodigo.indiceActual = 0;
    estadoCodigo.hoverBotonAnterior = false;
    estadoCodigo.hoverBotonSiguiente = false;
}

struct EstadoArchivoDesconocido {
    std::wstring rutaActual;
    std::wstring nombreArchivo;
    std::wstring extension;
    std::wstring tamanoFormateado;
    std::vector<std::wstring> archivosSeleccionados;
    size_t indiceActual{0};
    RECT botonAnterior{};
    RECT botonSiguiente{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};
};

inline EstadoArchivoDesconocido estadoDesconocido;

inline void liberarRecursosDesconocido() {
    estadoDesconocido.rutaActual.clear();
    estadoDesconocido.nombreArchivo.clear();
    estadoDesconocido.extension.clear();
    estadoDesconocido.tamanoFormateado.clear();
    estadoDesconocido.archivosSeleccionados.clear();
    estadoDesconocido.indiceActual = 0;
    estadoDesconocido.hoverBotonAnterior = false;
    estadoDesconocido.hoverBotonSiguiente = false;
}

/**** ended inlining nucleo/estado.hpp ****/
/**** start inlining nucleo/puntero_com.hpp ****/

// Administra una referencia COM y garantiza Release al salir del alcance.
// La clase expresa propiedad sin incorporar otro framework al proceso Explorer.
template <typename TipoInterfaz>
class PunteroCom {
  public:
    PunteroCom() = default;
    PunteroCom(const PunteroCom&) = delete;
    PunteroCom& operator=(const PunteroCom&) = delete;

    ~PunteroCom() { liberar(); }

    TipoInterfaz* obtener() const { return interfaz_; }
    TipoInterfaz* operator->() const { return interfaz_; }
    explicit operator bool() const { return interfaz_ != nullptr; }

    TipoInterfaz** recibir() {
        liberar();
        return &interfaz_;
    }

    void liberar() {
        if (!interfaz_) return;
        interfaz_->Release();
        interfaz_ = nullptr;
    }

  private:
    TipoInterfaz* interfaz_ = nullptr;
};
/**** ended inlining nucleo/puntero_com.hpp ****/
/**** start inlining archivos/seleccion_explorador.cpp ****/
/**** skipping file: archivos/clasificador_archivos.hpp ****/

inline bool esRutaDeVideo(const wchar_t* rutaArchivo) {
    if (!rutaArchivo || !*rutaArchivo || PathIsDirectoryW(rutaArchivo)) {
        return false;
    }
    return clasificarTipoArchivo(rutaArchivo) == TipoContenidoArchivo::Video;
}

inline bool esRutaDeImagen(const wchar_t* rutaArchivo) {
    if (!rutaArchivo || !*rutaArchivo || PathIsDirectoryW(rutaArchivo)) {
        return false;
    }
    return clasificarTipoArchivo(rutaArchivo) == TipoContenidoArchivo::Imagen;
}

inline std::vector<std::wstring> obtenerArchivosEnMismaCarpetaPorTipo(
    const std::wstring& rutaArchivoActual,
    TipoContenidoArchivo tipoRequerido) {
    if (rutaArchivoActual.empty()) return {};

    wchar_t buferDirectorio[MAX_PATH];
    wcsncpy_s(buferDirectorio, rutaArchivoActual.c_str(), _TRUNCATE);
    PathRemoveFileSpecW(buferDirectorio);

    std::wstring patronBusqueda = std::wstring(buferDirectorio) + L"\\*";
    WIN32_FIND_DATAW datosArchivo;
    HANDLE manejadorBusqueda =
        FindFirstFileW(patronBusqueda.c_str(), &datosArchivo);
    if (manejadorBusqueda == INVALID_HANDLE_VALUE) return {};

    std::vector<std::wstring> listaArchivos;
    do {
        if ((datosArchivo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            std::wstring rutaCompleta = std::wstring(buferDirectorio) + L"\\" +
                                        datosArchivo.cFileName;
            bool coincideTipo = false;
            if (tipoRequerido == TipoContenidoArchivo::Video) {
                coincideTipo = esRutaDeVideo(rutaCompleta.c_str());
            } else if (tipoRequerido == TipoContenidoArchivo::Imagen) {
                coincideTipo = esRutaDeImagen(rutaCompleta.c_str());
            } else {
                coincideTipo =
                    clasificarTipoArchivo(rutaCompleta.c_str()) == tipoRequerido;
            }
            if (coincideTipo) {
                listaArchivos.push_back(rutaCompleta);
            }
        }
    } while (FindNextFileW(manejadorBusqueda, &datosArchivo));
    FindClose(manejadorBusqueda);

    std::sort(listaArchivos.begin(), listaArchivos.end(),
              [](const std::wstring& primerRuta,
                 const std::wstring& segundaRuta) {
                  return StrCmpLogicalW(primerRuta.c_str(),
                                        segundaRuta.c_str()) < 0;
              });

    return listaArchivos;
}

inline std::vector<std::wstring> obtenerVideosEnMismaCarpeta(
    const std::wstring& rutaVideoActual) {
    return obtenerArchivosEnMismaCarpetaPorTipo(
        rutaVideoActual, TipoContenidoArchivo::Video);
}

inline std::vector<std::wstring> obtenerImagenesEnMismaCarpeta(
    const std::wstring& rutaImagenActual) {
    return obtenerArchivosEnMismaCarpetaPorTipo(
        rutaImagenActual, TipoContenidoArchivo::Imagen);
}

inline std::wstring obtenerArchivoAnteriorEnCarpeta(
    const std::wstring& rutaArchivoActual,
    TipoContenidoArchivo tipoRequerido) {
    std::vector<std::wstring> listaArchivos =
        obtenerArchivosEnMismaCarpetaPorTipo(rutaArchivoActual, tipoRequerido);
    if (listaArchivos.empty()) return {};

    auto iteradorActual =
        std::find(listaArchivos.begin(), listaArchivos.end(), rutaArchivoActual);
    if (iteradorActual == listaArchivos.end()) {
        return listaArchivos.front();
    }
    if (iteradorActual == listaArchivos.begin()) {
        return listaArchivos.back();
    }
    return *(iteradorActual - 1);
}

inline std::wstring obtenerArchivoSiguienteEnCarpeta(
    const std::wstring& rutaArchivoActual,
    TipoContenidoArchivo tipoRequerido) {
    std::vector<std::wstring> listaArchivos =
        obtenerArchivosEnMismaCarpetaPorTipo(rutaArchivoActual, tipoRequerido);
    if (listaArchivos.empty()) return {};

    auto iteradorActual =
        std::find(listaArchivos.begin(), listaArchivos.end(), rutaArchivoActual);
    if (iteradorActual == listaArchivos.end() ||
        iteradorActual + 1 == listaArchivos.end()) {
        return listaArchivos.front();
    }
    return *(iteradorActual + 1);
}

inline std::wstring obtenerVideoAnteriorEnCarpeta(
    const std::wstring& rutaVideoActual) {
    std::vector<std::wstring> listaVideos =
        obtenerVideosEnMismaCarpeta(rutaVideoActual);
    if (listaVideos.empty()) return {};

    auto iteradorActual =
        std::find(listaVideos.begin(), listaVideos.end(), rutaVideoActual);
    if (iteradorActual == listaVideos.end()) {
        return listaVideos.front();
    }
    if (iteradorActual == listaVideos.begin()) {
        return listaVideos.back();
    }
    return *(iteradorActual - 1);
}

inline std::wstring obtenerVideoSiguienteEnCarpeta(
    const std::wstring& rutaVideoActual) {
    std::vector<std::wstring> listaVideos =
        obtenerVideosEnMismaCarpeta(rutaVideoActual);
    if (listaVideos.empty()) return {};

    auto iteradorActual =
        std::find(listaVideos.begin(), listaVideos.end(), rutaVideoActual);
    if (iteradorActual == listaVideos.end() ||
        iteradorActual + 1 == listaVideos.end()) {
        return listaVideos.front();
    }
    return *(iteradorActual + 1);
}

inline std::wstring obtenerImagenAnteriorEnCarpeta(
    const std::wstring& rutaImagenActual) {
    return obtenerArchivoAnteriorEnCarpeta(
        rutaImagenActual, TipoContenidoArchivo::Imagen);
}

inline std::wstring obtenerImagenSiguienteEnCarpeta(
    const std::wstring& rutaImagenActual) {
    return obtenerArchivoSiguienteEnCarpeta(
        rutaImagenActual, TipoContenidoArchivo::Imagen);
}

inline std::vector<std::wstring> obtenerArchivosTextoEnMismaCarpeta(
    const std::wstring& rutaTextoActual) {
    return obtenerArchivosEnMismaCarpetaPorTipo(
        rutaTextoActual, TipoContenidoArchivo::Texto);
}

inline std::wstring obtenerArchivoTextoAnteriorEnCarpeta(
    const std::wstring& rutaTextoActual) {
    return obtenerArchivoAnteriorEnCarpeta(
        rutaTextoActual, TipoContenidoArchivo::Texto);
}

inline std::wstring obtenerArchivoTextoSiguienteEnCarpeta(
    const std::wstring& rutaTextoActual) {
    return obtenerArchivoSiguienteEnCarpeta(
        rutaTextoActual, TipoContenidoArchivo::Texto);
}

[[maybe_unused]] inline std::wstring obtenerDocumentoAnteriorEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoAnteriorEnCarpeta(rutaActual, TipoContenidoArchivo::Documento);
}

[[maybe_unused]] inline std::wstring obtenerDocumentoSiguienteEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoSiguienteEnCarpeta(rutaActual, TipoContenidoArchivo::Documento);
}

[[maybe_unused]] inline std::wstring obtenerCodigoAnteriorEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoAnteriorEnCarpeta(rutaActual, TipoContenidoArchivo::Codigo);
}

[[maybe_unused]] inline std::wstring obtenerCodigoSiguienteEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoSiguienteEnCarpeta(rutaActual, TipoContenidoArchivo::Codigo);
}

[[maybe_unused]] inline std::wstring obtenerComprimidoAnteriorEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoAnteriorEnCarpeta(rutaActual, TipoContenidoArchivo::Comprimido);
}

[[maybe_unused]] inline std::wstring obtenerComprimidoSiguienteEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoSiguienteEnCarpeta(rutaActual, TipoContenidoArchivo::Comprimido);
}

inline HWND normalizarVentanaExploradorAnfitriona(HWND ventanaCandidata) {
    if (!ventanaCandidata || !IsWindow(ventanaCandidata)) return nullptr;
    HWND ventanaRaiz = GetAncestor(ventanaCandidata, GA_ROOT);
    if (ventanaRaiz && IsWindow(ventanaRaiz)) {
        wchar_t claseRaiz[64];
        if (GetClassNameW(ventanaRaiz, claseRaiz, ARRAYSIZE(claseRaiz)) &&
            lstrcmpW(claseRaiz, L"CabinetWClass") == 0) {
            return ventanaRaiz;
        }
    }
    wchar_t claseCandidata[64];
    if (GetClassNameW(ventanaCandidata, claseCandidata, ARRAYSIZE(claseCandidata)) &&
        lstrcmpW(claseCandidata, L"CabinetWClass") == 0) {
        return ventanaCandidata;
    }
    return nullptr;
}

[[maybe_unused]] inline HWND encontrarContenedorPestanaOExplorador(HWND ventanaVistaDef, HWND ventanaExploradorFallback) {
    if (ventanaVistaDef && IsWindow(ventanaVistaDef)) {
        HWND ventanaRaiz = normalizarVentanaExploradorAnfitriona(ventanaVistaDef);
        if (ventanaRaiz) return ventanaRaiz;
    }
    return normalizarVentanaExploradorAnfitriona(ventanaExploradorFallback);
}

inline HWND obtenerVistaArchivosVisible(HWND ventanaAnfitriona) {
    if (!ventanaAnfitriona || !IsWindow(ventanaAnfitriona)) return nullptr;

    HWND vistaConFoco = nullptr;
    HWND ventanaFoco = GetFocus();
    if (ventanaFoco && IsWindow(ventanaFoco) &&
        (IsChild(ventanaAnfitriona, ventanaFoco) || ventanaFoco == ventanaAnfitriona)) {
        HWND cursorVentana = ventanaFoco;
        while (cursorVentana && cursorVentana != ventanaAnfitriona) {
            wchar_t claseCursor[64];
            if (GetClassNameW(cursorVentana, claseCursor, ARRAYSIZE(claseCursor)) &&
                lstrcmpW(claseCursor, L"SHELLDLL_DefView") == 0 &&
                IsWindowVisible(cursorVentana)) {
                vistaConFoco = cursorVentana;
                break;
            }
            cursorVentana = GetParent(cursorVentana);
        }
    }
    if (vistaConFoco) {
        ventanaVistaArchivosActiva = vistaConFoco;
        return vistaConFoco;
    }

    struct DatosBusquedaVista {
        HWND mejorVista{nullptr};
        int mayorArea{0};
    } datosBusqueda;

    EnumChildWindows(
        ventanaAnfitriona,
        [](HWND hwndHijo, LPARAM lParam) -> BOOL {
            wchar_t claseHijo[64];
            if (GetClassNameW(hwndHijo, claseHijo, ARRAYSIZE(claseHijo)) &&
                lstrcmpW(claseHijo, L"SHELLDLL_DefView") == 0 &&
                IsWindowVisible(hwndHijo)) {
                
                HWND padreHijo = GetParent(hwndHijo);
                if (padreHijo && !IsWindowVisible(padreHijo)) return TRUE;

                RECT rectanguloHijo{};
                GetWindowRect(hwndHijo, &rectanguloHijo);
                int anchoVentana = rectanguloHijo.right - rectanguloHijo.left;
                int altoVentana = rectanguloHijo.bottom - rectanguloHijo.top;
                int areaVentana = anchoVentana * altoVentana;
                if (rectanguloHijo.left > -1000 && anchoVentana > 50 && altoVentana > 50) {
                    auto* d = reinterpret_cast<DatosBusquedaVista*>(lParam);
                    if (areaVentana > d->mayorArea) {
                        d->mayorArea = areaVentana;
                        d->mejorVista = hwndHijo;
                    }
                }
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&datosBusqueda));

    if (datosBusqueda.mejorVista) {
        ventanaVistaArchivosActiva = datosBusqueda.mejorVista;
    }
    return datosBusqueda.mejorVista;
}

inline std::vector<std::wstring> obtenerArchivosSeleccionados(
    IShellWindows* ventanasShell,
    std::wstring* rutaCarpetaActualContenedora = nullptr) {
    DWORD procesoActual = GetCurrentProcessId();

    HWND ventanaPrimerPlano = GetForegroundWindow();
    if (ventanaPrimerPlano) {
        DWORD identificadorProcesoFoco = 0;
        GetWindowThreadProcessId(ventanaPrimerPlano, &identificadorProcesoFoco);
        if (identificadorProcesoFoco == procesoActual) {
            HWND ventanaNormalizada = normalizarVentanaExploradorAnfitriona(ventanaPrimerPlano);
            if (ventanaNormalizada && IsWindowVisible(ventanaNormalizada)) {
                ventanaExploradorAnfitriona = ventanaNormalizada;
            }
        }
    }

    if (!ventanaExploradorAnfitriona || !IsWindow(ventanaExploradorAnfitriona) || !IsWindowVisible(ventanaExploradorAnfitriona)) {
        HWND ventanaCandidataExplorador = FindWindowExW(nullptr, nullptr, L"CabinetWClass", nullptr);
        while (ventanaCandidataExplorador) {
            DWORD identificadorProcesoCandidata = 0;
            GetWindowThreadProcessId(ventanaCandidataExplorador, &identificadorProcesoCandidata);
            if (identificadorProcesoCandidata == procesoActual && IsWindowVisible(ventanaCandidataExplorador)) {
                ventanaExploradorAnfitriona = normalizarVentanaExploradorAnfitriona(ventanaCandidataExplorador);
                break;
            }
            ventanaCandidataExplorador = FindWindowExW(nullptr, ventanaCandidataExplorador, L"CabinetWClass", nullptr);
        }
    }

    if (!ventanasShell || !ventanaExploradorAnfitriona) return {};

    long cantidadVentanasShell = 0;
    if (FAILED(ventanasShell->get_Count(&cantidadVentanasShell))) return {};

    HWND vistaArchivosVisibleEnPantalla = obtenerVistaArchivosVisible(ventanaExploradorAnfitriona);

    auto extraerDatosElementoShell = [&](long indiceItem,
                                         std::vector<std::wstring>* rutasOut,
                                         std::wstring* rutaDirOut,
                                         HWND* hwndDefViewOut,
                                         HWND* hwndAnfitrionaOut) -> bool {
        rutasOut->clear();
        rutaDirOut->clear();
        if (hwndDefViewOut) *hwndDefViewOut = nullptr;
        if (hwndAnfitrionaOut) *hwndAnfitrionaOut = nullptr;

        VARIANT identificadorVentanaShell;
        VariantInit(&identificadorVentanaShell);
        identificadorVentanaShell.vt = VT_I4;
        identificadorVentanaShell.lVal = indiceItem;

        PunteroCom<IDispatch> despachoVentana;
        HRESULT resultadoElemento = ventanasShell->Item(
            identificadorVentanaShell, despachoVentana.recibir());
        VariantClear(&identificadorVentanaShell);
        if (FAILED(resultadoElemento) || !despachoVentana) return false;

        PunteroCom<IWebBrowserApp> explorador;
        if (FAILED(despachoVentana->QueryInterface(
                IID_PPV_ARGS(explorador.recibir()))) || !explorador) {
            return false;
        }

        SHANDLE_PTR identificadorVentana = 0;
        if (FAILED(explorador->get_HWND(&identificadorVentana))) return false;

        HWND ventanaExplorador = reinterpret_cast<HWND>(identificadorVentana);
        HWND ventanaRaizExplorador = normalizarVentanaExploradorAnfitriona(ventanaExplorador);
        if (!ventanaRaizExplorador) return false;

        DWORD identificadorProceso = 0;
        GetWindowThreadProcessId(ventanaRaizExplorador, &identificadorProceso);
        if (identificadorProceso != procesoActual) return false;

        if (hwndAnfitrionaOut) *hwndAnfitrionaOut = ventanaRaizExplorador;

        PunteroCom<IServiceProvider> proveedorServicios;
        if (FAILED(explorador->QueryInterface(
                IID_PPV_ARGS(proveedorServicios.recibir())))) {
            return false;
        }

        PunteroCom<IShellBrowser> navegadorCarpetas;
        if (FAILED(proveedorServicios->QueryService(
                SID_STopLevelBrowser,
                IID_PPV_ARGS(navegadorCarpetas.recibir())))) {
            return false;
        }

        PunteroCom<IShellView> vistaActiva;
        if (FAILED(navegadorCarpetas->QueryActiveShellView(
                vistaActiva.recibir())) || !vistaActiva) {
            return false;
        }

        PunteroCom<IFolderView2> vistaCarpeta;
        if (FAILED(vistaActiva->QueryInterface(
                IID_PPV_ARGS(vistaCarpeta.recibir()))) || !vistaCarpeta) {
            return false;
        }

        HWND ventanaVistaDef = nullptr;
        if (FAILED(vistaActiva->GetWindow(&ventanaVistaDef)) ||
            !ventanaVistaDef || !IsWindow(ventanaVistaDef)) {
            return false;
        }

        if (hwndDefViewOut) *hwndDefViewOut = ventanaVistaDef;

        std::wstring rutaDirectorioActual;
        PunteroCom<IShellItem> elementoDirectorio;
        if (SUCCEEDED(vistaCarpeta->GetFolder(
                IID_PPV_ARGS(elementoDirectorio.recibir()))) && elementoDirectorio) {
            PWSTR rutaDirectorio = nullptr;
            HRESULT hrDir = elementoDirectorio->GetDisplayName(SIGDN_FILESYSPATH, &rutaDirectorio);
            if (SUCCEEDED(hrDir) && rutaDirectorio) {
                std::wstring candidata = rutaDirectorio;
                CoTaskMemFree(rutaDirectorio);
                if (!candidata.empty() && PathFileExistsW(candidata.c_str())) {
                    rutaDirectorioActual = candidata;
                }
            }
            if (rutaDirectorioActual.empty()) {
                PWSTR rutaParsing = nullptr;
                if (SUCCEEDED(elementoDirectorio->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &rutaParsing)) && rutaParsing) {
                    if (PathFileExistsW(rutaParsing) && PathIsDirectoryW(rutaParsing)) {
                        rutaDirectorioActual = rutaParsing;
                    }
                    CoTaskMemFree(rutaParsing);
                }
            }
            if (rutaDirectorioActual.empty()) {
                PWSTR nombreVisible = nullptr;
                if (SUCCEEDED(elementoDirectorio->GetDisplayName(
                        SIGDN_NORMALDISPLAY, &nombreVisible)) && nombreVisible) {
                    rutaDirectorioActual = L"::SISTEMA::" + std::wstring(nombreVisible);
                    CoTaskMemFree(nombreVisible);
                } else {
                    PWSTR rutaParsing = nullptr;
                    if (SUCCEEDED(elementoDirectorio->GetDisplayName(
                            SIGDN_DESKTOPABSOLUTEPARSING, &rutaParsing)) && rutaParsing) {
                        std::wstring parsingMin = rutaParsing;
                        CoTaskMemFree(rutaParsing);
                        std::transform(parsingMin.begin(), parsingMin.end(), parsingMin.begin(), ::towlower);
                        if (parsingMin.find(L"e88865ea-0e1c-4e20-9aa6-ed353b7c4777") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Galería";
                        } else if (parsingMin.find(L"f874310e-b6b7-47dc-bc84-b9e6b38f5903") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Inicio";
                        } else if (parsingMin.find(L"20d04fe0-3aea-1069-a2d8-08002b30309d") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Este equipo";
                        } else if (parsingMin.find(L"645ff040-5081-101b-9f08-00aa002f954e") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Papelera de reciclaje";
                        } else if (parsingMin.find(L"f02c1a0d-be21-4350-88b0-7367fc96ef3c") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Red";
                        } else if (parsingMin.find(L"26ee0668-a00a-44d7-9371-beb064c98683") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Panel de control";
                        } else {
                            rutaDirectorioActual = L"::SISTEMA::Ubicación del Sistema";
                        }
                    } else {
                        rutaDirectorioActual = L"::SISTEMA::Ubicación del Sistema";
                    }
                }
            }
        }

        *rutaDirOut = rutaDirectorioActual;

        PunteroCom<IShellItemArray> elementosSeleccionados;
        HRESULT resultadoSeleccion =
            vistaCarpeta->GetSelection(TRUE, elementosSeleccionados.recibir());

        if (SUCCEEDED(resultadoSeleccion) && elementosSeleccionados) {
            DWORD cantidadSeleccionados = 0;
            elementosSeleccionados->GetCount(&cantidadSeleccionados);

            for (DWORD posicionElemento = 0;
                 posicionElemento < cantidadSeleccionados;
                 ++posicionElemento) {
                 PunteroCom<IShellItem> elementoSeleccionado;
                 if (SUCCEEDED(elementosSeleccionados->GetItemAt(
                         posicionElemento, elementoSeleccionado.recibir())) &&
                     elementoSeleccionado) {
                     PWSTR rutaElemento = nullptr;
                     HRESULT hrElemento = elementoSeleccionado->GetDisplayName(
                         SIGDN_FILESYSPATH, &rutaElemento);
                     if (FAILED(hrElemento) || !rutaElemento) {
                         hrElemento = elementoSeleccionado->GetDisplayName(
                             SIGDN_DESKTOPABSOLUTEPARSING, &rutaElemento);
                     }
                     if (SUCCEEDED(hrElemento) && rutaElemento) {
                         if (rutaDirectorioActual.empty() ||
                             lstrcmpiW(rutaElemento, rutaDirectorioActual.c_str()) != 0) {
                             if (esArchivoCompatible(rutaElemento)) {
                                 rutasOut->push_back(rutaElemento);
                             }
                         }
                         CoTaskMemFree(rutaElemento);
                     }
                 }
            }
        }

        return true;
    };

    struct VistaCandidataAnfitriona {
        std::vector<std::wstring> rutas;
        std::wstring rutaDirectorio;
        HWND hwndDefView{nullptr};
        bool esVisibleEnPantalla{false};
    };
    std::vector<VistaCandidataAnfitriona> candidatasDeEstaAnfitriona;

    for (long posicionShell = 0; posicionShell < cantidadVentanasShell; ++posicionShell) {
        std::vector<std::wstring> rutasSeleccionadas;
        std::wstring rutaDirectorio;
        HWND hwndDefView = nullptr;
        HWND hwndAnfitrionaItem = nullptr;
        if (extraerDatosElementoShell(posicionShell, &rutasSeleccionadas, &rutaDirectorio, &hwndDefView, &hwndAnfitrionaItem)) {
            // STRICT FILTER: Must belong strictly to THIS active Explorer window
            if (hwndAnfitrionaItem != ventanaExploradorAnfitriona) {
                continue;
            }

            bool esLaVistaActiva = (vistaArchivosVisibleEnPantalla && hwndDefView == vistaArchivosVisibleEnPantalla);
            if (esLaVistaActiva) {
                ventanaVistaArchivosActiva = hwndDefView;
                if (rutaCarpetaActualContenedora) {
                    *rutaCarpetaActualContenedora = rutaDirectorio;
                }
                if (!rutasSeleccionadas.empty()) {
                    std::sort(rutasSeleccionadas.begin(), rutasSeleccionadas.end(),
                              [](const std::wstring& primeraRuta, const std::wstring& segundaRuta) {
                                  return StrCmpLogicalW(primeraRuta.c_str(), segundaRuta.c_str()) < 0;
                              });
                    return rutasSeleccionadas;
                }
                return {};
            }

            RECT rectanguloDefView{};
            GetWindowRect(hwndDefView, &rectanguloDefView);
            bool esVisible = (IsWindowVisible(hwndDefView) && rectanguloDefView.left > -1000);
            candidatasDeEstaAnfitriona.push_back({rutasSeleccionadas, rutaDirectorio, hwndDefView, esVisible});
        }
    }

    for (const auto& candidataVista : candidatasDeEstaAnfitriona) {
        if (candidataVista.esVisibleEnPantalla) {
            ventanaVistaArchivosActiva = candidataVista.hwndDefView;
            if (rutaCarpetaActualContenedora) {
                *rutaCarpetaActualContenedora = candidataVista.rutaDirectorio;
            }
            if (!candidataVista.rutas.empty()) {
                std::vector<std::wstring> resultadoRutas = candidataVista.rutas;
                std::sort(resultadoRutas.begin(), resultadoRutas.end(),
                          [](const std::wstring& primeraRuta, const std::wstring& segundaRuta) {
                              return StrCmpLogicalW(primeraRuta.c_str(), segundaRuta.c_str()) < 0;
                          });
                return resultadoRutas;
            }
            return {};
        }
    }

    if (!candidatasDeEstaAnfitriona.empty()) {
        const auto& primeraCandidata = candidatasDeEstaAnfitriona.front();
        ventanaVistaArchivosActiva = primeraCandidata.hwndDefView;
        if (rutaCarpetaActualContenedora) {
            *rutaCarpetaActualContenedora = primeraCandidata.rutaDirectorio;
        }
        if (!primeraCandidata.rutas.empty()) {
            std::vector<std::wstring> resultadoRutas = primeraCandidata.rutas;
            std::sort(resultadoRutas.begin(), resultadoRutas.end(),
                      [](const std::wstring& primeraRuta, const std::wstring& segundaRuta) {
                          return StrCmpLogicalW(primeraRuta.c_str(), segundaRuta.c_str()) < 0;
                      });
            return resultadoRutas;
        }
        return {};
    }

    return {};
}
/**** ended inlining archivos/seleccion_explorador.cpp ****/
/**** start inlining imagen/controlador_imagen.hpp ****/

#include <windows.h>
#include <gdiplus.h>
#include <wincodec.h>
#include <shlobj.h>
#include <algorithm>
#include <string>
#include <vector>
/**** skipping file: nucleo/puntero_com.hpp ****/

enum class IdAccionImagen {
    Girar90Horario,
    Girar90Antihorario,
    CopiarAlPortapapeles,
    EstablecerFondoPantalla,
};

struct ElementoAccionImagen {
    IdAccionImagen id;
    std::wstring etiqueta;
    std::wstring glifo;
    RECT areaBoton{};
    bool hover{false};
};

struct ElementoMiniaturaGaleria {
    std::wstring rutaArchivo;
    std::wstring nombreArchivo;
    RECT areaTarjeta{};
    bool esActiva{false};
    bool hover{false};
};

struct EstadoVisorImagen {
    std::wstring rutaImagenActual;
    std::vector<std::wstring> imagenesSeleccionadas;
    size_t indiceImagenSeleccionada{0};
    Gdiplus::Image* imagenCargada{nullptr};
    UINT anchoOriginal{0};
    UINT altoOriginal{0};
    size_t posicionEnCarpeta{0};
    size_t totalEnCarpeta{0};
    bool esAnimada{false};
    UINT totalCuadros{1};
    UINT indiceCuadroActual{0};
    std::vector<UINT> retardosCuadros;
    IStream* flujoStreamMemoria{nullptr};

    // Navegación
    RECT botonAnterior{};
    RECT botonSiguiente{};
    RECT botonGaleria{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};
    bool hoverBotonGaleria{false};

    // Modo Galería
    bool modoGaleria{false};
    int desplazamientoScrollGaleria{0};
    int altoTotalGaleria{0};
    std::vector<ElementoMiniaturaGaleria> miniaturasGaleria;

    // Acciones modulares con imágenes
    std::vector<ElementoAccionImagen> accionesDisponibles;
};

inline EstadoVisorImagen estadoImagen;

inline void inicializarAccionesImagen() {
    estadoImagen.accionesDisponibles.clear();
    estadoImagen.accionesDisponibles.push_back({
        IdAccionImagen::Girar90Horario,
        L"Girar 90°",
        L"🔄"
    });
    estadoImagen.accionesDisponibles.push_back({
        IdAccionImagen::CopiarAlPortapapeles,
        L"Copiar",
        L"📋"
    });
    estadoImagen.accionesDisponibles.push_back({
        IdAccionImagen::EstablecerFondoPantalla,
        L"Fondo",
        L"🖼️"
    });
}

inline void liberarRecursosImagen() {
    if (ventanaVideo) {
        KillTimer(ventanaVideo, ID_TEMPORIZADOR_ANIMACION_GIF);
    }
    if (estadoImagen.imagenCargada) {
        delete estadoImagen.imagenCargada;
        estadoImagen.imagenCargada = nullptr;
    }
    if (estadoImagen.flujoStreamMemoria) {
        estadoImagen.flujoStreamMemoria->Release();
        estadoImagen.flujoStreamMemoria = nullptr;
    }
    estadoImagen.rutaImagenActual.clear();
    estadoImagen.imagenesSeleccionadas.clear();
    estadoImagen.indiceImagenSeleccionada = 0;
    estadoImagen.anchoOriginal = 0;
    estadoImagen.altoOriginal = 0;
    estadoImagen.posicionEnCarpeta = 0;
    estadoImagen.totalEnCarpeta = 0;
    estadoImagen.esAnimada = false;
    estadoImagen.totalCuadros = 1;
    estadoImagen.indiceCuadroActual = 0;
    estadoImagen.retardosCuadros.clear();
    estadoImagen.hoverBotonAnterior = false;
    estadoImagen.hoverBotonSiguiente = false;
    estadoImagen.hoverBotonGaleria = false;
    estadoImagen.modoGaleria = false;
    estadoImagen.desplazamientoScrollGaleria = 0;
    estadoImagen.altoTotalGaleria = 0;
    estadoImagen.miniaturasGaleria.clear();
    estadoImagen.accionesDisponibles.clear();
}

inline IStream* crearFlujoEnMemoriaDesdeArchivo(const std::wstring& rutaArchivo) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return nullptr;

    LARGE_INTEGER tamanoArchivo{};
    if (!GetFileSizeEx(manejadorArchivo, &tamanoArchivo) ||
        tamanoArchivo.QuadPart == 0 ||
        tamanoArchivo.QuadPart > 128 * 1024 * 1024) {
        CloseHandle(manejadorArchivo);
        return nullptr;
    }

    HGLOBAL memoriaGlobal = GlobalAlloc(GMEM_MOVEABLE, static_cast<SIZE_T>(tamanoArchivo.QuadPart));
    if (!memoriaGlobal) {
        CloseHandle(manejadorArchivo);
        return nullptr;
    }

    void* punteroMemoria = GlobalLock(memoriaGlobal);
    if (!punteroMemoria) {
        GlobalFree(memoriaGlobal);
        CloseHandle(manejadorArchivo);
        return nullptr;
    }

    DWORD bytesLeidos = 0;
    if (!ReadFile(manejadorArchivo, punteroMemoria, static_cast<DWORD>(tamanoArchivo.QuadPart), &bytesLeidos, nullptr) ||
        bytesLeidos != static_cast<DWORD>(tamanoArchivo.QuadPart)) {
        GlobalUnlock(memoriaGlobal);
        GlobalFree(memoriaGlobal);
        CloseHandle(manejadorArchivo);
        return nullptr;
    }
    GlobalUnlock(memoriaGlobal);
    CloseHandle(manejadorArchivo);

    IStream* flujoStream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(memoriaGlobal, TRUE, &flujoStream))) {
        GlobalFree(memoriaGlobal);
        return nullptr;
    }
    return flujoStream;
}

inline Gdiplus::Bitmap* decodificarImagenConWic(
    const std::wstring& rutaArchivo) {
    IStream* flujoStream = crearFlujoEnMemoriaDesdeArchivo(rutaArchivo);
    if (!flujoStream) return nullptr;

    PunteroCom<IWICImagingFactory> fabricaWic;
    HRESULT resultadoFabrica = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(fabricaWic.recibir()));
    if (FAILED(resultadoFabrica) || !fabricaWic.obtener()) {
        resultadoFabrica = CoCreateInstance(
            CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(fabricaWic.recibir()));
        if (FAILED(resultadoFabrica) || !fabricaWic.obtener()) {
            flujoStream->Release();
            return nullptr;
        }
    }

    PunteroCom<IWICBitmapDecoder> decodificador;
    HRESULT resultadoDecodificador = fabricaWic->CreateDecoderFromStream(
        flujoStream, nullptr, WICDecodeMetadataCacheOnDemand,
        decodificador.recibir());
    flujoStream->Release();
    if (FAILED(resultadoDecodificador) || !decodificador.obtener())
        return nullptr;

    PunteroCom<IWICBitmapFrameDecode> cuadroDecodificado;
    HRESULT resultadoCuadro =
        decodificador->GetFrame(0, cuadroDecodificado.recibir());
    if (FAILED(resultadoCuadro) || !cuadroDecodificado.obtener()) return nullptr;

    PunteroCom<IWICFormatConverter> convertidorFormato;
    HRESULT resultadoConvertidor =
        fabricaWic->CreateFormatConverter(convertidorFormato.recibir());
    if (FAILED(resultadoConvertidor) || !convertidorFormato.obtener())
        return nullptr;

    HRESULT resultadoInicio = convertidorFormato->Initialize(
        cuadroDecodificado.obtener(), GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(resultadoInicio)) return nullptr;

    UINT anchoImagen = 0;
    UINT altoImagen = 0;
    HRESULT resultadoDimensiones =
        convertidorFormato->GetSize(&anchoImagen, &altoImagen);
    if (FAILED(resultadoDimensiones) || anchoImagen == 0 || altoImagen == 0)
        return nullptr;

    UINT pasoBytesFila = anchoImagen * 4;
    UINT tamanoTotalBufer = pasoBytesFila * altoImagen;
    std::vector<BYTE> buferPixeles(tamanoTotalBufer);

    HRESULT resultadoCopia = convertidorFormato->CopyPixels(
        nullptr, pasoBytesFila, tamanoTotalBufer, buferPixeles.data());
    if (FAILED(resultadoCopia)) return nullptr;

    Gdiplus::Bitmap* mapaBitsDestino = new Gdiplus::Bitmap(
        static_cast<INT>(anchoImagen), static_cast<INT>(altoImagen),
        PixelFormat32bppARGB);
    if (!mapaBitsDestino || mapaBitsDestino->GetLastStatus() != Gdiplus::Ok) {
        if (mapaBitsDestino) delete mapaBitsDestino;
        return nullptr;
    }

    Gdiplus::BitmapData datosMapaBits{};
    Gdiplus::Rect rectanguloArea(0, 0, static_cast<INT>(anchoImagen), static_cast<INT>(altoImagen));
    if (mapaBitsDestino->LockBits(&rectanguloArea, Gdiplus::ImageLockModeWrite,
                                  PixelFormat32bppARGB, &datosMapaBits) == Gdiplus::Ok) {
        BYTE* punteroDestino = static_cast<BYTE*>(datosMapaBits.Scan0);
        const BYTE* punteroOrigen = buferPixeles.data();
        for (UINT fila = 0; fila < altoImagen; ++fila) {
            memcpy(punteroDestino + fila * static_cast<UINT>(datosMapaBits.Stride),
                   punteroOrigen + fila * pasoBytesFila,
                   pasoBytesFila);
        }
        mapaBitsDestino->UnlockBits(&datosMapaBits);
    }

    return mapaBitsDestino;
}

inline void inicializarAnimacionGifSiAplica() {
    if (!estadoImagen.imagenCargada) return;

    GUID dimensionTiempo = Gdiplus::FrameDimensionTime;
    UINT cantidadDimensiones =
        estadoImagen.imagenCargada->GetFrameDimensionsCount();
    if (cantidadDimensiones == 0) return;

    std::vector<GUID> listaDimensiones(cantidadDimensiones);
    if (estadoImagen.imagenCargada->GetFrameDimensionsList(
            listaDimensiones.data(), cantidadDimensiones) != Gdiplus::Ok) {
        return;
    }

    bool soporteDimensionTiempo = false;
    for (UINT posicionDimension = 0; posicionDimension < cantidadDimensiones;
         ++posicionDimension) {
        if (listaDimensiones[posicionDimension] == dimensionTiempo) {
            soporteDimensionTiempo = true;
            break;
        }
    }

    if (!soporteDimensionTiempo) return;

    UINT cantidadCuadros =
        estadoImagen.imagenCargada->GetFrameCount(&dimensionTiempo);
    if (cantidadCuadros <= 1) return;

    estadoImagen.esAnimada = true;
    estadoImagen.totalCuadros = cantidadCuadros;
    estadoImagen.indiceCuadroActual = 0;
    estadoImagen.retardosCuadros.clear();

    UINT tamanoPropiedad =
        estadoImagen.imagenCargada->GetPropertyItemSize(PropertyTagFrameDelay);
    if (tamanoPropiedad > 0) {
        std::vector<BYTE> buferPropiedad(tamanoPropiedad);
        Gdiplus::PropertyItem* elementoPropiedad =
            reinterpret_cast<Gdiplus::PropertyItem*>(buferPropiedad.data());
        if (estadoImagen.imagenCargada->GetPropertyItem(
                PropertyTagFrameDelay, tamanoPropiedad, elementoPropiedad) ==
            Gdiplus::Ok) {
            long* arregloRetardos =
                reinterpret_cast<long*>(elementoPropiedad->value);
            UINT totalRetardosLeidos =
                elementoPropiedad->length / static_cast<UINT>(sizeof(long));
            for (UINT posicionCuadro = 0; posicionCuadro < cantidadCuadros;
                 ++posicionCuadro) {
                UINT retardoMilisegundos = 100;
                if (posicionCuadro < totalRetardosLeidos) {
                    long valorRetardoLeido = arregloRetardos[posicionCuadro];
                    if (valorRetardoLeido > 0) {
                        retardoMilisegundos =
                            static_cast<UINT>(valorRetardoLeido * 10);
                    }
                }
                if (retardoMilisegundos < 20) {
                    retardoMilisegundos = 100;
                }
                estadoImagen.retardosCuadros.push_back(retardoMilisegundos);
            }
        }
    }

    if (estadoImagen.retardosCuadros.empty()) {
        estadoImagen.retardosCuadros.assign(cantidadCuadros, 100);
    }

    if (ventanaVideo) {
        UINT retardoInicial = estadoImagen.retardosCuadros.front();
        SetTimer(ventanaVideo, ID_TEMPORIZADOR_ANIMACION_GIF, retardoInicial,
                 nullptr);
    }
}

inline void avanzarCuadroAnimacionGif(HWND ventanaObjetivo) {
    if (!estadoImagen.esAnimada || estadoImagen.totalCuadros <= 1 ||
        !estadoImagen.imagenCargada) {
        return;
    }

    estadoImagen.indiceCuadroActual =
        (estadoImagen.indiceCuadroActual + 1) % estadoImagen.totalCuadros;
    GUID dimensionTiempo = Gdiplus::FrameDimensionTime;
    estadoImagen.imagenCargada->SelectActiveFrame(
        &dimensionTiempo, estadoImagen.indiceCuadroActual);

    UINT siguienteRetardo = 100;
    if (estadoImagen.indiceCuadroActual <
        estadoImagen.retardosCuadros.size()) {
        siguienteRetardo =
            estadoImagen.retardosCuadros[estadoImagen.indiceCuadroActual];
    }
    SetTimer(ventanaObjetivo, ID_TEMPORIZADOR_ANIMACION_GIF, siguienteRetardo,
             nullptr);
    InvalidateRect(ventanaObjetivo, nullptr, FALSE);
}

inline void actualizarListaGaleria() {
    estadoImagen.miniaturasGaleria.clear();
    std::vector<std::wstring> listaRutas;
    if (estadoImagen.imagenesSeleccionadas.size() > 1) {
        listaRutas = estadoImagen.imagenesSeleccionadas;
    } else if (!estadoImagen.rutaImagenActual.empty()) {
        listaRutas = obtenerImagenesEnMismaCarpeta(estadoImagen.rutaImagenActual);
    }
    for (size_t posicionElemento = 0; posicionElemento < listaRutas.size(); ++posicionElemento) {
        ElementoMiniaturaGaleria elemento;
        elemento.rutaArchivo = listaRutas[posicionElemento];
        elemento.nombreArchivo = PathFindFileNameW(listaRutas[posicionElemento].c_str());
        elemento.esActiva = (lstrcmpiW(listaRutas[posicionElemento].c_str(), estadoImagen.rutaImagenActual.c_str()) == 0);
        elemento.hover = false;
        estadoImagen.miniaturasGaleria.push_back(elemento);
    }
}

inline bool cargarImagenPorRuta(const std::wstring& rutaNuevaImagen) {
    if (rutaNuevaImagen.empty()) {
        liberarRecursosImagen();
        return false;
    }

    std::vector<std::wstring> seleccionPrevia = estadoImagen.imagenesSeleccionadas;
    size_t indicePrevio = estadoImagen.indiceImagenSeleccionada;
    bool modoGaleriaPrevio = estadoImagen.modoGaleria;

    liberarRecursosImagen();

    estadoImagen.imagenesSeleccionadas = seleccionPrevia;
    estadoImagen.indiceImagenSeleccionada = indicePrevio;
    estadoImagen.modoGaleria = modoGaleriaPrevio;

    // 1. Decodificación WIC en memoria (rápida, segura y 0 descriptores de archivo abiertos)
    estadoImagen.imagenCargada = decodificarImagenConWic(rutaNuevaImagen);

    // 2. Si WIC no lo decodificó (ej. GIF animado o formato especial), decodificar vía stream en memoria
    if (!estadoImagen.imagenCargada || estadoImagen.imagenCargada->GetLastStatus() != Gdiplus::Ok) {
        if (estadoImagen.imagenCargada) {
            delete estadoImagen.imagenCargada;
            estadoImagen.imagenCargada = nullptr;
        }
        estadoImagen.flujoStreamMemoria = crearFlujoEnMemoriaDesdeArchivo(rutaNuevaImagen);
        if (estadoImagen.flujoStreamMemoria) {
            LARGE_INTEGER liInicio{};
            estadoImagen.flujoStreamMemoria->Seek(liInicio, STREAM_SEEK_SET, nullptr);
            estadoImagen.imagenCargada = Gdiplus::Bitmap::FromStream(estadoImagen.flujoStreamMemoria);
            if (!estadoImagen.imagenCargada || estadoImagen.imagenCargada->GetLastStatus() != Gdiplus::Ok) {
                if (estadoImagen.imagenCargada) {
                    delete estadoImagen.imagenCargada;
                    estadoImagen.imagenCargada = nullptr;
                }
                estadoImagen.flujoStreamMemoria->Release();
                estadoImagen.flujoStreamMemoria = nullptr;
            }
        }
    }

    // 3. Fallback de emergencia
    if (!estadoImagen.imagenCargada ||
        estadoImagen.imagenCargada->GetLastStatus() != Gdiplus::Ok) {
        if (estadoImagen.imagenCargada) {
            delete estadoImagen.imagenCargada;
            estadoImagen.imagenCargada = nullptr;
        }
        estadoImagen.imagenCargada = Gdiplus::Image::FromFile(rutaNuevaImagen.c_str());
    }

    if (!estadoImagen.imagenCargada ||
        estadoImagen.imagenCargada->GetLastStatus() != Gdiplus::Ok) {
        liberarRecursosImagen();
        return false;
    }

    estadoImagen.rutaImagenActual = rutaNuevaImagen;
    estadoImagen.anchoOriginal = estadoImagen.imagenCargada->GetWidth();
    estadoImagen.altoOriginal = estadoImagen.imagenCargada->GetHeight();

    inicializarAnimacionGifSiAplica();
    inicializarAccionesImagen();

    std::vector<std::wstring> listaImagenes =
        obtenerImagenesEnMismaCarpeta(rutaNuevaImagen);
    estadoImagen.totalEnCarpeta = listaImagenes.size();
    auto iterador = std::find(listaImagenes.begin(), listaImagenes.end(),
                              rutaNuevaImagen);
    if (iterador != listaImagenes.end()) {
        estadoImagen.posicionEnCarpeta =
            static_cast<size_t>(
                std::distance(listaImagenes.begin(), iterador)) +
            1;
    } else {
        estadoImagen.posicionEnCarpeta = 1;
    }

    actualizarListaGaleria();

    return true;
}

inline bool girarImagenActual(bool sentidoHorario = true) {
    if (!estadoImagen.imagenCargada) return false;
    Gdiplus::RotateFlipType tipoRotacion = sentidoHorario
        ? Gdiplus::Rotate90FlipNone
        : Gdiplus::Rotate270FlipNone;
    if (estadoImagen.imagenCargada->RotateFlip(tipoRotacion) == Gdiplus::Ok) {
        std::swap(estadoImagen.anchoOriginal, estadoImagen.altoOriginal);
        return true;
    }
    return false;
}

inline bool copiarImagenActualAlPortapapeles(HWND ventana) {
    if (estadoImagen.rutaImagenActual.empty()) return false;
    if (!OpenClipboard(ventana)) return false;
    EmptyClipboard();

    std::wstring ruta = estadoImagen.rutaImagenActual;
    size_t bytes = sizeof(DROPFILES) + (ruta.size() + 2) * sizeof(wchar_t);
    HGLOBAL hGlobal = GlobalAlloc(GHND, bytes);
    if (hGlobal) {
        DROPFILES* estructuraDropFiles = reinterpret_cast<DROPFILES*>(GlobalLock(hGlobal));
        if (estructuraDropFiles) {
            estructuraDropFiles->pFiles = sizeof(DROPFILES);
            estructuraDropFiles->fWide = TRUE;
            wchar_t* buffer = reinterpret_cast<wchar_t*>(reinterpret_cast<BYTE*>(estructuraDropFiles) + sizeof(DROPFILES));
            wcscpy_s(buffer, ruta.size() + 1, ruta.c_str());
            buffer[ruta.size() + 1] = L'\0';
            GlobalUnlock(hGlobal);
            SetClipboardData(CF_HDROP, hGlobal);
        }
    }
    CloseClipboard();
    return true;
}

inline bool establecerComoFondoDePantalla() {
    if (estadoImagen.rutaImagenActual.empty()) return false;
    return SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0,
        const_cast<wchar_t*>(estadoImagen.rutaImagenActual.c_str()),
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) != FALSE;
}

inline bool ejecutarAccionImagen(IdAccionImagen id, HWND ventanaPadre) {
    switch (id) {
        case IdAccionImagen::Girar90Horario:
            return girarImagenActual(true);
        case IdAccionImagen::Girar90Antihorario:
            return girarImagenActual(false);
        case IdAccionImagen::CopiarAlPortapapeles:
            return copiarImagenActualAlPortapapeles(ventanaPadre);
        case IdAccionImagen::EstablecerFondoPantalla:
            return establecerComoFondoDePantalla();
        default:
            return false;
    }
}

inline void dibujarLienzoGaleriaImagenes(HDC contextoDestino, const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTitulo = CreateFontW(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSub = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    RECT areaCabecera{areaDibujo.left + 12, areaDibujo.top + 8, areaDibujo.right - 12, areaDibujo.top + 28};
    SetBkMode(contextoDestino, TRANSPARENT);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDestino, fuenteTitulo));
    std::wstring tituloGaleria = L"Galería de imágenes (" +
        std::to_wstring(estadoImagen.miniaturasGaleria.size()) + L")";
    DrawTextW(contextoDestino, tituloGaleria.c_str(), -1, &areaCabecera, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(contextoDestino, fuenteSub);

    int margenLateral = 12;
    int espacioEntreTarjetas = 8;
    int anchoDisponible = std::max(100, anchoArea - margenLateral * 2);
    size_t cantidadColumnas = static_cast<size_t>(std::max(2, std::min(4, anchoDisponible / 110)));
    int anchoTarjeta = (anchoDisponible - (static_cast<int>(cantidadColumnas) - 1) * espacioEntreTarjetas) / static_cast<int>(cantidadColumnas);
    int altoTarjeta = 95;

    int yInicio = areaDibujo.top + 34 - estadoImagen.desplazamientoScrollGaleria;

    HBRUSH brochaNormal = CreateSolidBrush(RGB(22, 27, 34));
    HBRUSH brochaHover = CreateSolidBrush(RGB(26, 32, 44));
    HBRUSH brochaActiva = CreateSolidBrush(RGB(30, 41, 59));
    HPEN lapizNormal = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizHover = CreatePen(PS_SOLID, 1, RGB(88, 166, 255));
    HPEN lapizActiva = CreatePen(PS_SOLID, 2, RGB(0, 120, 215));

    for (size_t posicionMiniatura = 0; posicionMiniatura < estadoImagen.miniaturasGaleria.size(); ++posicionMiniatura) {
        int columnaActual = static_cast<int>(posicionMiniatura % cantidadColumnas);
        int filaActual = static_cast<int>(posicionMiniatura / cantidadColumnas);
        int posicionTarjetaX = areaDibujo.left + margenLateral + columnaActual * (anchoTarjeta + espacioEntreTarjetas);
        int posicionTarjetaY = yInicio + filaActual * (altoTarjeta + espacioEntreTarjetas);

        RECT areaTarjeta{posicionTarjetaX, posicionTarjetaY, posicionTarjetaX + anchoTarjeta, posicionTarjetaY + altoTarjeta};
        estadoImagen.miniaturasGaleria[posicionMiniatura].areaTarjeta = areaTarjeta;

        if (areaTarjeta.bottom < areaDibujo.top || areaTarjeta.top > areaDibujo.bottom) {
            continue;
        }

        bool esActiva = estadoImagen.miniaturasGaleria[posicionMiniatura].esActiva;
        bool hover = estadoImagen.miniaturasGaleria[posicionMiniatura].hover;

        HBRUSH brochaSeleccionada = esActiva ? brochaActiva : (hover ? brochaHover : brochaNormal);
        HPEN lapizSeleccionado = esActiva ? lapizActiva : (hover ? lapizHover : lapizNormal);

        HBRUSH brochaPreviaCard = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaSeleccionada));
        HPEN lapizPrevioCard = static_cast<HPEN>(SelectObject(contextoDestino, lapizSeleccionado));
        RoundRect(contextoDestino, areaTarjeta.left, areaTarjeta.top, areaTarjeta.right, areaTarjeta.bottom, 6, 6);
        SelectObject(contextoDestino, lapizPrevioCard);
        SelectObject(contextoDestino, brochaPreviaCard);

        RECT areaIcono{areaTarjeta.left + 4, areaTarjeta.top + 6, areaTarjeta.right - 4, areaTarjeta.top + 48};
        SelectObject(contextoDestino, fuenteTitulo);
        SetTextColor(contextoDestino, esActiva ? RGB(88, 166, 255) : RGB(139, 148, 158));
        DrawTextW(contextoDestino, L"🖼️", -1, &areaIcono, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        RECT areaNombre{areaTarjeta.left + 6, areaTarjeta.top + 52, areaTarjeta.right - 6, areaTarjeta.bottom - 4};
        SelectObject(contextoDestino, fuenteSub);
        SetTextColor(contextoDestino, esActiva ? RGB(255, 255, 255) : (hover ? RGB(240, 246, 252) : RGB(201, 209, 217)));
        DrawTextW(contextoDestino, estadoImagen.miniaturasGaleria[posicionMiniatura].nombreArchivo.c_str(), -1,
                  &areaNombre, DT_CENTER | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);
    }

    DeleteObject(lapizActiva);
    DeleteObject(lapizHover);
    DeleteObject(lapizNormal);
    DeleteObject(brochaActiva);
    DeleteObject(brochaHover);
    DeleteObject(brochaNormal);

    size_t totalFilas = (estadoImagen.miniaturasGaleria.size() + cantidadColumnas - 1) / cantidadColumnas;
    estadoImagen.altoTotalGaleria = static_cast<int>(totalFilas) * (altoTarjeta + espacioEntreTarjetas) + 40;

    SelectObject(contextoDestino, fuentePrev);
    DeleteObject(fuenteSub);
    DeleteObject(fuenteTitulo);
}

inline void dibujarLienzoImagen(HDC contextoDestino, const RECT& areaDibujo) {
    if (estadoImagen.modoGaleria) {
        dibujarLienzoGaleriaImagenes(contextoDestino, areaDibujo);
        return;
    }

    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(15, 15, 15));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    if (!estadoImagen.imagenCargada || estadoImagen.anchoOriginal == 0 ||
        estadoImagen.altoOriginal == 0) {
        return;
    }

    float proporcionHorizontal =
        static_cast<float>(anchoArea) /
        static_cast<float>(estadoImagen.anchoOriginal);
    float proporcionVertical =
        static_cast<float>(altoArea) /
        static_cast<float>(estadoImagen.altoOriginal);
    float proporcionEscala =
        std::min(proporcionHorizontal, proporcionVertical);

    int anchoEscalado = static_cast<int>(
        static_cast<float>(estadoImagen.anchoOriginal) * proporcionEscala);
    int altoEscalado = static_cast<int>(
        static_cast<float>(estadoImagen.altoOriginal) * proporcionEscala);

    int coordenadaX =
        areaDibujo.left + (anchoArea - anchoEscalado) / 2;
    int coordenadaY =
        areaDibujo.top + (altoArea - altoEscalado) / 2;

    Gdiplus::Graphics graficosGdi(contextoDestino);
    graficosGdi.SetInterpolationMode(
        Gdiplus::InterpolationModeHighQualityBicubic);
    graficosGdi.DrawImage(estadoImagen.imagenCargada, coordenadaX,
                          coordenadaY, anchoEscalado, altoEscalado);
}

inline bool mostrarImagenAnterior() {
    if (estadoImagen.imagenesSeleccionadas.size() > 1) {
        estadoImagen.indiceImagenSeleccionada =
            (estadoImagen.indiceImagenSeleccionada == 0)
                ? (estadoImagen.imagenesSeleccionadas.size() - 1)
                : (estadoImagen.indiceImagenSeleccionada - 1);
        std::vector<std::wstring> seleccionPrevia = estadoImagen.imagenesSeleccionadas;
        size_t indicePrevio = estadoImagen.indiceImagenSeleccionada;
        bool ok = cargarImagenPorRuta(seleccionPrevia[indicePrevio]);
        estadoImagen.imagenesSeleccionadas = seleccionPrevia;
        estadoImagen.indiceImagenSeleccionada = indicePrevio;
        return ok;
    }
    if (estadoImagen.rutaImagenActual.empty()) return false;
    std::wstring rutaAnterior =
        obtenerImagenAnteriorEnCarpeta(estadoImagen.rutaImagenActual);
    if (rutaAnterior.empty()) return false;
    estadoImagen.imagenesSeleccionadas = { rutaAnterior };
    estadoImagen.indiceImagenSeleccionada = 0;
    return cargarImagenPorRuta(rutaAnterior);
}

inline bool mostrarImagenSiguiente() {
    if (estadoImagen.imagenesSeleccionadas.size() > 1) {
        estadoImagen.indiceImagenSeleccionada =
            (estadoImagen.indiceImagenSeleccionada + 1) %
            estadoImagen.imagenesSeleccionadas.size();
        std::vector<std::wstring> seleccionPrevia = estadoImagen.imagenesSeleccionadas;
        size_t indicePrevio = estadoImagen.indiceImagenSeleccionada;
        bool ok = cargarImagenPorRuta(seleccionPrevia[indicePrevio]);
        estadoImagen.imagenesSeleccionadas = seleccionPrevia;
        estadoImagen.indiceImagenSeleccionada = indicePrevio;
        return ok;
    }
    if (estadoImagen.rutaImagenActual.empty()) return false;
    std::wstring rutaSiguiente =
        obtenerImagenSiguienteEnCarpeta(estadoImagen.rutaImagenActual);
    if (rutaSiguiente.empty()) return false;
    estadoImagen.imagenesSeleccionadas = { rutaSiguiente };
    estadoImagen.indiceImagenSeleccionada = 0;
    return cargarImagenPorRuta(rutaSiguiente);
}

inline void dibujarBarraControlesImagen(HDC contextoDibujo, const RECT& areaCliente) {
    HBRUSH pincelFondo = CreateSolidBrush(RGB(18, 18, 18));
    FillRect(contextoDibujo, &areaCliente, pincelFondo);
    DeleteObject(pincelFondo);

    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(45, 45, 45));
    HPEN lapizPrevio = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    MoveToEx(contextoDibujo, areaCliente.left, areaCliente.top, nullptr);
    LineTo(contextoDibujo, areaCliente.right, areaCliente.top);
    SelectObject(contextoDibujo, lapizPrevio);
    DeleteObject(lapizBorde);

    // Fila 1: Botones de navegación (◀ Anterior, 🖼️ Galería, Siguiente ▶)
    int anchoBotonNav = 80;
    int altoBotonNav = 24;
    int yFilaNav = areaCliente.top + 6;

    estadoImagen.botonAnterior = {
        areaCliente.left + 12,
        yFilaNav,
        areaCliente.left + 12 + anchoBotonNav,
        yFilaNav + altoBotonNav
    };

    estadoImagen.botonSiguiente = {
        areaCliente.right - 12 - anchoBotonNav,
        yFilaNav,
        areaCliente.right - 12,
        yFilaNav + altoBotonNav
    };

    int anchoBotonGaleria = 90;
    int xCentro = (areaCliente.left + areaCliente.right) / 2;
    estadoImagen.botonGaleria = {
        xCentro - anchoBotonGaleria / 2,
        yFilaNav,
        xCentro + anchoBotonGaleria / 2,
        yFilaNav + altoBotonNav
    };

    auto dibujarBoton = [&](const RECT& rect, const wchar_t* texto, bool hover, bool activo = false) {
        COLORREF colorFondo = activo ? RGB(0, 120, 215) : (hover ? RGB(48, 54, 61) : RGB(33, 38, 45));
        COLORREF colorBorde = activo ? RGB(0, 140, 240) : (hover ? RGB(110, 118, 129) : RGB(48, 54, 61));
        HBRUSH brochaBtn = CreateSolidBrush(colorFondo);
        HPEN lapizBtn = CreatePen(PS_SOLID, 1, colorBorde);
        HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoDibujo, brochaBtn));
        HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBtn));
        RoundRect(contextoDibujo, rect.left, rect.top, rect.right, rect.bottom, 6, 6);
        SelectObject(contextoDibujo, lapizPrev);
        SelectObject(contextoDibujo, brochaPrev);
        DeleteObject(lapizBtn);
        DeleteObject(brochaBtn);

        SetBkMode(contextoDibujo, TRANSPARENT);
        SetTextColor(contextoDibujo, (activo || hover) ? RGB(255, 255, 255) : RGB(201, 209, 217));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        RECT rTexto = rect;
        DrawTextW(contextoDibujo, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    };

    dibujarBoton(estadoImagen.botonAnterior, L"◀ Anterior", estadoImagen.hoverBotonAnterior);
    dibujarBoton(estadoImagen.botonGaleria, estadoImagen.modoGaleria ? L"🖼️ Cerrar" : L"🖼️ Galería",
                 estadoImagen.hoverBotonGaleria, estadoImagen.modoGaleria);
    dibujarBoton(estadoImagen.botonSiguiente, L"Siguiente ▶", estadoImagen.hoverBotonSiguiente);

    // Fila 2: Separador y Sección "ACCIONES CON IMÁGENES"
    int yFilaAcciones = yFilaNav + altoBotonNav + 6;
    if (estadoImagen.accionesDisponibles.empty()) {
        inicializarAccionesImagen();
    }

    HPEN lapizSep = CreatePen(PS_SOLID, 1, RGB(38, 44, 52));
    HPEN lapizPrevioSep = static_cast<HPEN>(SelectObject(contextoDibujo, lapizSep));
    MoveToEx(contextoDibujo, areaCliente.left + 12, yFilaAcciones, nullptr);
    LineTo(contextoDibujo, areaCliente.right - 12, yFilaAcciones);
    SelectObject(contextoDibujo, lapizPrevioSep);
    DeleteObject(lapizSep);

    yFilaAcciones += 4;
    int anchoBotonAccion = 96;
    int altoBotonAccion = 22;
    int espacioAcciones = 8;
    int totalAnchoAcciones = static_cast<int>(estadoImagen.accionesDisponibles.size()) * (anchoBotonAccion + espacioAcciones) - espacioAcciones;
    int xInicioAcciones = std::max<int>(areaCliente.left + 12, (areaCliente.left + areaCliente.right - totalAnchoAcciones) / 2);

    for (size_t posicionAccion = 0; posicionAccion < estadoImagen.accionesDisponibles.size(); ++posicionAccion) {
        RECT areaBotonAccion = {
            xInicioAcciones + static_cast<int>(posicionAccion) * (anchoBotonAccion + espacioAcciones),
            yFilaAcciones,
            xInicioAcciones + static_cast<int>(posicionAccion) * (anchoBotonAccion + espacioAcciones) + anchoBotonAccion,
            yFilaAcciones + altoBotonAccion
        };
        estadoImagen.accionesDisponibles[posicionAccion].areaBoton = areaBotonAccion;
        std::wstring etiquetaCompleta = estadoImagen.accionesDisponibles[posicionAccion].glifo + L" " + estadoImagen.accionesDisponibles[posicionAccion].etiqueta;
        dibujarBoton(areaBotonAccion, etiquetaCompleta.c_str(), estadoImagen.accionesDisponibles[posicionAccion].hover);
    }
}


/**** ended inlining imagen/controlador_imagen.hpp ****/
/**** start inlining texto/controlador_texto.hpp ****/

#include <windows.h>
#include <shlwapi.h>
#include <urlmon.h>
#include <gdiplus.h>
#include <algorithm>
#include <string>
#include <vector>
Gdiplus::Bitmap* decodificarImagenConWic(const std::wstring& rutaArchivo);

enum class TipoElementoMarkdown {
    Encabezado1,
    Encabezado2,
    Encabezado3,
    Encabezado4,
    Parrafo,
    BloqueCodigo,
    CitaBloque,
    ListaDesordenada,
    ListaOrdenada,
    CasillaVerificacion,
    ReglaHorizontal,
    SeparadorSeccionPersonalizado,
    Imagen,
    VideoYouTube,
    EnlaceWeb,
    TablaMarkdown
};

enum class EstiloFragmento {
    Normal,
    Negrita,
    Cursiva,
    Subrayado,
    CodigoPildora
};

struct FragmentoTexto {
    std::wstring texto;
    EstiloFragmento estilo{EstiloFragmento::Normal};
};

struct FilaTablaMarkdown {
    std::vector<std::wstring> celdas;
    bool esCabecera{false};
};

struct ElementoMarkdown {
    TipoElementoMarkdown tipo{TipoElementoMarkdown::Parrafo};
    std::wstring textoContenido;
    std::wstring textoSecundario;
    std::vector<FragmentoTexto> fragmentos;
    std::vector<FilaTablaMarkdown> filasTabla;
    int nivelEncabezado{0};
    int sangriaLista{0};
    bool casillaMarcada{false};
    int posicionVerticalY{0};
    int altoCalculado{0};
};

struct ElementoIndiceBarras {
    std::wstring tituloEncabezado;
    int nivelEncabezado{1};
    int posicionDestinoY{0};
    RECT areaVisualBarra{};
    RECT areaBarra{};
};

struct ElementoClicableMarkdown {
    RECT areaClic{};
    std::wstring destinoUrl;
    bool esVideo{false};
    bool esVideoYouTube{false};
    bool esEnlaceWeb{false};
};

struct EntradaCacheBitmap {
    std::wstring rutaUbicacion;
    Gdiplus::Bitmap* mapaBits{nullptr};
};

struct EstadoControladorTexto {
    std::vector<std::wstring> textosSeleccionados;
    std::wstring rutaTextoActual;
    size_t indiceTextoSeleccionado{0};
    std::wstring nombreArchivo;
    bool esFormatoMarkdown{true};
    size_t posicionEnCarpeta{1};
    size_t totalEnCarpeta{1};
    RECT botonAnterior{};
    RECT botonSiguiente{};
    RECT botonEditar{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};
    bool hoverBotonEditar{false};
    std::vector<ElementoMarkdown> elementosMarkdown;
    std::vector<ElementoIndiceBarras> barrasIndice;
    std::vector<ElementoClicableMarkdown> elementosClicables;
    std::vector<EntradaCacheBitmap> cacheBitmaps;
    int desplazamientoScrollY{0};
    int altoTotalContenido{0};
    int posicionBarraHover{-1};
    bool panelIndiceDesplegado{false};
    size_t totalLineasOriginales{0};
};

inline EstadoControladorTexto estadoTexto;

inline void limpiarContenidoTextoCargado() {
    estadoTexto.rutaTextoActual.clear();
    estadoTexto.nombreArchivo.clear();
    estadoTexto.esFormatoMarkdown = true;
    estadoTexto.posicionEnCarpeta = 1;
    estadoTexto.totalEnCarpeta = 1;
    estadoTexto.elementosMarkdown.clear();
    estadoTexto.barrasIndice.clear();
    estadoTexto.elementosClicables.clear();
    for (auto& entrada : estadoTexto.cacheBitmaps) {
        if (entrada.mapaBits) {
            delete entrada.mapaBits;
        }
    }
    estadoTexto.cacheBitmaps.clear();
    estadoTexto.desplazamientoScrollY = 0;
    estadoTexto.altoTotalContenido = 0;
    estadoTexto.posicionBarraHover = -1;
    estadoTexto.panelIndiceDesplegado = false;
    estadoTexto.totalLineasOriginales = 0;
}

inline void liberarRecursosTexto() {
    limpiarContenidoTextoCargado();
    estadoTexto.textosSeleccionados.clear();
    estadoTexto.indiceTextoSeleccionado = 0;
    estadoTexto.botonAnterior = {};
    estadoTexto.botonSiguiente = {};
    estadoTexto.botonEditar = {};
    estadoTexto.hoverBotonAnterior = false;
    estadoTexto.hoverBotonSiguiente = false;
    estadoTexto.hoverBotonEditar = false;
}

inline std::wstring extraerIdVideoYouTube(const std::wstring& urlOTexto) {
    size_t posicionWatch = urlOTexto.find(L"watch?v=");
    if (posicionWatch != std::wstring::npos) {
        std::wstring idCandidato = urlOTexto.substr(posicionWatch + 8);
        size_t posicionAmp = idCandidato.find(L'&');
        if (posicionAmp != std::wstring::npos) {
            idCandidato = idCandidato.substr(0, posicionAmp);
        }
        size_t posicionCierre = idCandidato.find_first_of(L" \t\r\n)\">");
        if (posicionCierre != std::wstring::npos) {
            idCandidato = idCandidato.substr(0, posicionCierre);
        }
        if (!idCandidato.empty()) return idCandidato;
    }

    size_t posicionYoutuBe = urlOTexto.find(L"youtu.be/");
    if (posicionYoutuBe != std::wstring::npos) {
        std::wstring idCandidato = urlOTexto.substr(posicionYoutuBe + 9);
        size_t posicionCierre = idCandidato.find_first_of(L" \t\r\n?&)\">");
        if (posicionCierre != std::wstring::npos) {
            idCandidato = idCandidato.substr(0, posicionCierre);
        }
        if (!idCandidato.empty()) return idCandidato;
    }

    size_t posicionEmbed = urlOTexto.find(L"youtube.com/embed/");
    if (posicionEmbed != std::wstring::npos) {
        std::wstring idCandidato = urlOTexto.substr(posicionEmbed + 18);
        size_t posicionCierre = idCandidato.find_first_of(L" \t\r\n?&)\">");
        if (posicionCierre != std::wstring::npos) {
            idCandidato = idCandidato.substr(0, posicionCierre);
        }
        if (!idCandidato.empty()) return idCandidato;
    }

    return {};
}

inline Gdiplus::Bitmap* obtenerBitmapParaMarkdown(
    const std::wstring& rutaUbicacion,
    const std::wstring& rutaBaseTexto) {
    if (rutaUbicacion.empty()) return nullptr;

    for (const auto& entrada : estadoTexto.cacheBitmaps) {
        if (entrada.rutaUbicacion == rutaUbicacion) {
            return entrada.mapaBits;
        }
    }

    Gdiplus::Bitmap* mapaBitsCargado = nullptr;

    bool esUrlRemota = (rutaUbicacion.rfind(L"http://", 0) == 0 ||
                        rutaUbicacion.rfind(L"https://", 0) == 0);

    if (esUrlRemota) {
        wchar_t rutaCarpetaTemporal[MAX_PATH];
        if (GetTempPathW(MAX_PATH, rutaCarpetaTemporal) > 0) {
            std::wstring carpetaCache =
                std::wstring(rutaCarpetaTemporal) + L"UsefulSideViewMdCache";
            CreateDirectoryW(carpetaCache.c_str(), nullptr);

            DWORD hashValor = 5381;
            for (wchar_t caracter : rutaUbicacion) {
                hashValor = ((hashValor << 5) + hashValor) + static_cast<DWORD>(caracter);
            }
            std::wstring rutaArchivoTemporal =
                carpetaCache + L"\\" + std::to_wstring(hashValor) + L".img";

            if (GetFileAttributesW(rutaArchivoTemporal.c_str()) ==
                INVALID_FILE_ATTRIBUTES) {
                estadoTexto.cacheBitmaps.push_back(
                    {rutaUbicacion, nullptr});
                return nullptr;
            }

            mapaBitsCargado = decodificarImagenConWic(rutaArchivoTemporal.c_str());
        }
    } else {
        std::wstring rutaFinalLocal = rutaUbicacion;
        if (PathIsRelativeW(rutaUbicacion.c_str()) &&
            !rutaBaseTexto.empty()) {
            wchar_t buferDirectorio[MAX_PATH];
            wcsncpy_s(buferDirectorio, rutaBaseTexto.c_str(), MAX_PATH);
            PathRemoveFileSpecW(buferDirectorio);

            wchar_t rutaCombinada[MAX_PATH];
            if (PathCombineW(rutaCombinada, buferDirectorio,
                             rutaUbicacion.c_str())) {
                rutaFinalLocal = rutaCombinada;
            }
        }

        mapaBitsCargado = decodificarImagenConWic(rutaFinalLocal.c_str());
    }

    if (mapaBitsCargado &&
        mapaBitsCargado->GetLastStatus() == Gdiplus::Ok) {
        estadoTexto.cacheBitmaps.push_back(
            {rutaUbicacion, mapaBitsCargado});
        return mapaBitsCargado;
    }

    if (mapaBitsCargado) delete mapaBitsCargado;
    estadoTexto.cacheBitmaps.push_back({rutaUbicacion, nullptr});
    return nullptr;
}

inline std::wstring leerArchivoTextoUtf8(const std::wstring& rutaArchivo) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return {};

    DWORD tamanoArchivoAlto = 0;
    DWORD tamanoArchivoBajo =
        GetFileSize(manejadorArchivo, &tamanoArchivoAlto);
    if (tamanoArchivoBajo == INVALID_FILE_SIZE) {
        CloseHandle(manejadorArchivo);
        return {};
    }
    if (tamanoArchivoBajo == 0) {
        CloseHandle(manejadorArchivo);
        return L"*(Archivo vacío)*";
    }

    constexpr DWORD LIMITE_MAXIMO_LECTURA = 2 * 1024 * 1024;
    DWORD bytesALeer = std::min<DWORD>(tamanoArchivoBajo, LIMITE_MAXIMO_LECTURA);
    std::vector<char> buferBytes(bytesALeer + 2, '\0');
    DWORD bytesLeidos = 0;
    if (!ReadFile(manejadorArchivo, buferBytes.data(), bytesALeer,
                  &bytesLeidos, nullptr) ||
        bytesLeidos == 0) {
        CloseHandle(manejadorArchivo);
        return {};
    }
    CloseHandle(manejadorArchivo);

    const unsigned char* bytesRaw =
        reinterpret_cast<const unsigned char*>(buferBytes.data());
    DWORD longitudEfectiva = bytesLeidos;

    // Detección de BOM UTF-16 LE (0xFF, 0xFE)
    if (longitudEfectiva >= 2 && bytesRaw[0] == 0xFF && bytesRaw[1] == 0xFE) {
        size_t cantidadCaracteres = (longitudEfectiva - 2) / sizeof(wchar_t);
        const wchar_t* punteroW =
            reinterpret_cast<const wchar_t*>(buferBytes.data() + 2);
        return std::wstring(punteroW, cantidadCaracteres);
    }

    // Detección de BOM UTF-8 (0xEF, 0xBB, 0xBF)
    const char* punteroTexto = buferBytes.data();
    if (longitudEfectiva >= 3 &&
        bytesRaw[0] == 0xEF && bytesRaw[1] == 0xBB && bytesRaw[2] == 0xBF) {
        punteroTexto += 3;
        longitudEfectiva -= 3;
    }

    int longitudWide = MultiByteToWideChar(
        CP_UTF8, 0, punteroTexto, static_cast<int>(longitudEfectiva), nullptr,
        0);
    if (longitudWide <= 0) {
        longitudWide = MultiByteToWideChar(
            CP_ACP, 0, punteroTexto, static_cast<int>(longitudEfectiva),
            nullptr, 0);
        if (longitudWide <= 0) return {};
        std::wstring textoWide(static_cast<size_t>(longitudWide), L'\0');
        MultiByteToWideChar(CP_ACP, 0, punteroTexto,
                           static_cast<int>(longitudEfectiva),
                           textoWide.data(), longitudWide);
        return textoWide;
    }

    std::wstring textoWide(static_cast<size_t>(longitudWide), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, punteroTexto,
                       static_cast<int>(longitudEfectiva), textoWide.data(),
                       longitudWide);
    return textoWide;
}

inline std::wstring_view recortarEspaciosVista(std::wstring_view cadenaVista) {
    size_t primero = cadenaVista.find_first_not_of(L" \t\r\n");
    if (primero == std::wstring_view::npos) return {};
    size_t ultimo = cadenaVista.find_last_not_of(L" \t\r\n");
    return cadenaVista.substr(primero, ultimo - primero + 1);
}

inline std::wstring recortarEspacios(const std::wstring& cadena) {
    size_t primero = cadena.find_first_not_of(L" \t\r\n");
    if (primero == std::wstring::npos) return L"";
    size_t ultimo = cadena.find_last_not_of(L" \t\r\n");
    return cadena.substr(primero, ultimo - primero + 1);
}

inline std::vector<FragmentoTexto> parsearFragmentosTexto(const std::wstring& texto) {
    std::vector<FragmentoTexto> fragmentos;
    size_t longitudTexto = texto.size();
    size_t posicionActual = 0;
    std::wstring acumulado;

    while (posicionActual < longitudTexto) {
        if (texto[posicionActual] == L'`') {
            size_t posicionFin = texto.find(L'`', posicionActual + 1);
            if (posicionFin != std::wstring::npos) {
                if (!acumulado.empty()) {
                    fragmentos.push_back({acumulado, EstiloFragmento::Normal});
                    acumulado.clear();
                }
                fragmentos.push_back({texto.substr(posicionActual + 1, posicionFin - posicionActual - 1), EstiloFragmento::CodigoPildora});
                posicionActual = posicionFin + 1;
                continue;
            }
        }
        if (posicionActual + 1 < longitudTexto && texto[posicionActual] == L'*' && texto[posicionActual + 1] == L'*') {
            size_t posicionFin = texto.find(L"**", posicionActual + 2);
            if (posicionFin != std::wstring::npos) {
                if (!acumulado.empty()) {
                    fragmentos.push_back({acumulado, EstiloFragmento::Normal});
                    acumulado.clear();
                }
                fragmentos.push_back({texto.substr(posicionActual + 2, posicionFin - posicionActual - 2), EstiloFragmento::Negrita});
                posicionActual = posicionFin + 2;
                continue;
            }
        }
        if (posicionActual + 1 < longitudTexto && texto[posicionActual] == L'=' && texto[posicionActual + 1] == L'=') {
            size_t posicionFin = texto.find(L"==", posicionActual + 2);
            if (posicionFin != std::wstring::npos) {
                if (!acumulado.empty()) {
                    fragmentos.push_back({acumulado, EstiloFragmento::Normal});
                    acumulado.clear();
                }
                fragmentos.push_back({texto.substr(posicionActual + 2, posicionFin - posicionActual - 2), EstiloFragmento::Subrayado});
                posicionActual = posicionFin + 2;
                continue;
            }
        }
        if (texto[posicionActual] == L'_') {
            size_t posicionFin = texto.find(L'_', posicionActual + 1);
            if (posicionFin != std::wstring::npos && posicionFin > posicionActual + 1) {
                if (!acumulado.empty()) {
                    fragmentos.push_back({acumulado, EstiloFragmento::Normal});
                    acumulado.clear();
                }
                fragmentos.push_back({texto.substr(posicionActual + 1, posicionFin - posicionActual - 1), EstiloFragmento::Cursiva});
                posicionActual = posicionFin + 1;
                continue;
            }
        }
        acumulado += texto[posicionActual];
        posicionActual++;
    }

    if (!acumulado.empty()) {
        fragmentos.push_back({acumulado, EstiloFragmento::Normal});
    }

    return fragmentos;
}

inline void parsearContenidoMarkdown(const std::wstring& contenidoCompleto) {
    estadoTexto.elementosMarkdown.clear();
    estadoTexto.barrasIndice.clear();

    if (contenidoCompleto.empty()) {
        ElementoMarkdown elementoVacio;
        elementoVacio.tipo = TipoElementoMarkdown::Parrafo;
        elementoVacio.textoContenido = L"*(Archivo sin contenido)*";
        estadoTexto.elementosMarkdown.push_back(elementoVacio);
        return;
    }

    bool dentroDeBloqueCodigo = false;
    std::wstring acumuladorBloqueCodigo;
    std::wstring lenguajeCodigo;

    size_t posicionInicio = 0;
    size_t longitudTexto = contenidoCompleto.size();

    while (posicionInicio < longitudTexto) {
        size_t posicionFin = contenidoCompleto.find(L'\n', posicionInicio);
        if (posicionFin == std::wstring::npos) {
            posicionFin = longitudTexto;
        }

        std::wstring lineaActual = contenidoCompleto.substr(
            posicionInicio, posicionFin - posicionInicio);
        if (!lineaActual.empty() && lineaActual.back() == L'\r') {
            lineaActual.pop_back();
        }

        posicionInicio = posicionFin + 1;
        estadoTexto.totalLineasOriginales++;

        if (estadoTexto.elementosMarkdown.size() >= 2500) {
            ElementoMarkdown elementoAviso;
            elementoAviso.tipo = TipoElementoMarkdown::Parrafo;
            elementoAviso.textoContenido = L"... [Vista previa limitada a los primeros 2,500 bloques por rendimiento]";
            estadoTexto.elementosMarkdown.push_back(elementoAviso);
            break;
        }

        // Detección de cercado de código ```
        if (lineaActual.rfind(L"```", 0) == 0) {
            if (dentroDeBloqueCodigo) {
                ElementoMarkdown elementoCodigo;
                elementoCodigo.tipo = TipoElementoMarkdown::BloqueCodigo;
                elementoCodigo.textoContenido = acumuladorBloqueCodigo;
                elementoCodigo.textoSecundario = lenguajeCodigo;
                estadoTexto.elementosMarkdown.push_back(elementoCodigo);

                acumuladorBloqueCodigo.clear();
                lenguajeCodigo.clear();
                dentroDeBloqueCodigo = false;
            } else {
                dentroDeBloqueCodigo = true;
                if (lineaActual.size() > 3) {
                    lenguajeCodigo = lineaActual.substr(3);
                }
            }
            continue;
        }

        if (dentroDeBloqueCodigo) {
            acumuladorBloqueCodigo += lineaActual + L"\n";
            continue;
        }

        // Línea vacía
        if (lineaActual.find_first_not_of(L" \t") == std::wstring::npos) {
            ElementoMarkdown elementoVacio;
            elementoVacio.tipo = TipoElementoMarkdown::Parrafo;
            elementoVacio.textoContenido = L"";
            estadoTexto.elementosMarkdown.push_back(elementoVacio);
            continue;
        }

        // Tablas Markdown
        if (lineaActual.find(L'|') != std::wstring::npos) {
            auto dividirCeldas = [](const std::wstring& linea) -> std::vector<std::wstring> {
                std::vector<std::wstring> celdas;
                size_t inicio = 0;
                if (!linea.empty() && linea.front() == L'|') inicio = 1;
                size_t fin = linea.size();
                if (!linea.empty() && linea.back() == L'|') fin = linea.size() - 1;
                size_t posicionCelda = inicio;
                while (posicionCelda < fin) {
                    size_t posicionProximoSeparador = linea.find(L'|', posicionCelda);
                    if (posicionProximoSeparador == std::wstring::npos || posicionProximoSeparador > fin) {
                        posicionProximoSeparador = fin;
                    }
                    std::wstring_view vistaSub = std::wstring_view(linea).substr(posicionCelda, posicionProximoSeparador - posicionCelda);
                    std::wstring_view vistaRecortada = recortarEspaciosVista(vistaSub);
                    celdas.emplace_back(vistaRecortada);
                    posicionCelda = posicionProximoSeparador + 1;
                }
                return celdas;
            };

            auto esFilaSeparador = [](const std::wstring& linea) -> bool {
                for (wchar_t caracterSeparador : linea) {
                    if (caracterSeparador != L'|' && caracterSeparador != L'-' && caracterSeparador != L':' &&
                        caracterSeparador != L' ' && caracterSeparador != L'\t') {
                        return false;
                    }
                }
                return linea.find(L'-') != std::wstring::npos;
            };

            if (!estadoTexto.elementosMarkdown.empty() &&
                estadoTexto.elementosMarkdown.back().tipo == TipoElementoMarkdown::TablaMarkdown) {
                if (!esFilaSeparador(lineaActual)) {
                    FilaTablaMarkdown fila;
                    fila.celdas = dividirCeldas(lineaActual);
                    fila.esCabecera = false;
                    if (!fila.celdas.empty()) {
                        estadoTexto.elementosMarkdown.back().filasTabla.push_back(fila);
                    }
                }
                continue;
            } else {
                ElementoMarkdown elementoTabla;
                elementoTabla.tipo = TipoElementoMarkdown::TablaMarkdown;
                FilaTablaMarkdown filaCabecera;
                filaCabecera.celdas = dividirCeldas(lineaActual);
                filaCabecera.esCabecera = true;
                if (!filaCabecera.celdas.empty()) {
                    elementoTabla.filasTabla.push_back(filaCabecera);
                    estadoTexto.elementosMarkdown.push_back(elementoTabla);
                }
                continue;
            }
        }

        // Sintaxis especial de separador con título: ---[texto]---
        std::wstring lineaRecortada = recortarEspacios(lineaActual);
        if (lineaRecortada.rfind(L"---[", 0) == 0 &&
            lineaRecortada.size() >= 7 &&
            lineaRecortada.rfind(L"]---") == lineaRecortada.size() - 4) {
            std::wstring tituloSeparador = lineaRecortada.substr(4, lineaRecortada.size() - 8);
            tituloSeparador = recortarEspacios(tituloSeparador);

            ElementoMarkdown elementoSep;
            elementoSep.tipo = TipoElementoMarkdown::SeparadorSeccionPersonalizado;
            elementoSep.textoContenido = tituloSeparador;
            elementoSep.nivelEncabezado = 2;
            estadoTexto.elementosMarkdown.push_back(elementoSep);

            if (!tituloSeparador.empty()) {
                ElementoIndiceBarras barraIndice;
                barraIndice.tituloEncabezado = tituloSeparador;
                barraIndice.nivelEncabezado = 2;
                barraIndice.posicionDestinoY = 0;
                estadoTexto.barrasIndice.push_back(barraIndice);
            }
            continue;
        }

        // Regla horizontal --- o ***
        if (lineaActual == L"---" || lineaActual == L"***" ||
            lineaActual == L"___") {
            ElementoMarkdown elementoRegla;
            elementoRegla.tipo = TipoElementoMarkdown::ReglaHorizontal;
            estadoTexto.elementosMarkdown.push_back(elementoRegla);
            continue;
        }

        // Encabezados #
        if (lineaActual.rfind(L"#### ", 0) == 0) {
            ElementoMarkdown elementoH4;
            elementoH4.tipo = TipoElementoMarkdown::Encabezado4;
            elementoH4.textoContenido = lineaActual.substr(5);
            elementoH4.nivelEncabezado = 4;
            estadoTexto.elementosMarkdown.push_back(elementoH4);
            continue;
        }
        if (lineaActual.rfind(L"### ", 0) == 0) {
            ElementoMarkdown elementoH3;
            elementoH3.tipo = TipoElementoMarkdown::Encabezado3;
            elementoH3.textoContenido = lineaActual.substr(4);
            elementoH3.nivelEncabezado = 3;
            estadoTexto.elementosMarkdown.push_back(elementoH3);
            continue;
        }
        if (lineaActual.rfind(L"## ", 0) == 0) {
            ElementoMarkdown elementoH2;
            elementoH2.tipo = TipoElementoMarkdown::Encabezado2;
            elementoH2.textoContenido = lineaActual.substr(3);
            elementoH2.nivelEncabezado = 2;
            estadoTexto.elementosMarkdown.push_back(elementoH2);
            continue;
        }
        if (lineaActual.rfind(L"# ", 0) == 0) {
            ElementoMarkdown elementoH1;
            elementoH1.tipo = TipoElementoMarkdown::Encabezado1;
            elementoH1.textoContenido = lineaActual.substr(2);
            elementoH1.nivelEncabezado = 1;
            estadoTexto.elementosMarkdown.push_back(elementoH1);
            continue;
        }

        // Citas en bloque >
        if (lineaActual.rfind(L"> ", 0) == 0 || lineaActual == L">") {
            ElementoMarkdown elementoCita;
            elementoCita.tipo = TipoElementoMarkdown::CitaBloque;
            elementoCita.textoContenido =
                (lineaActual.size() > 2) ? lineaActual.substr(2) : L"";
            estadoTexto.elementosMarkdown.push_back(elementoCita);
            continue;
        }

        // Casillas de verificación - [ ] o - [x]
        if (lineaActual.rfind(L"- [ ] ", 0) == 0 ||
            lineaActual.rfind(L"* [ ] ", 0) == 0) {
            ElementoMarkdown elementoCasilla;
            elementoCasilla.tipo = TipoElementoMarkdown::CasillaVerificacion;
            elementoCasilla.casillaMarcada = false;
            elementoCasilla.textoContenido = lineaActual.substr(6);
            estadoTexto.elementosMarkdown.push_back(elementoCasilla);
            continue;
        }
        if (lineaActual.rfind(L"- [x] ", 0) == 0 ||
            lineaActual.rfind(L"- [X] ", 0) == 0 ||
            lineaActual.rfind(L"* [x] ", 0) == 0) {
            ElementoMarkdown elementoCasilla;
            elementoCasilla.tipo = TipoElementoMarkdown::CasillaVerificacion;
            elementoCasilla.casillaMarcada = true;
            elementoCasilla.textoContenido = lineaActual.substr(6);
            estadoTexto.elementosMarkdown.push_back(elementoCasilla);
            continue;
        }

        // Listas ordenadas (1. , 2. , etc.)
        if (lineaActual.size() >= 3 && iswdigit(lineaActual[0])) {
            size_t punto = lineaActual.find(L". ");
            if (punto != std::wstring::npos && punto <= 4) {
                ElementoMarkdown elementoListaOrd;
                elementoListaOrd.tipo = TipoElementoMarkdown::ListaOrdenada;
                elementoListaOrd.textoSecundario = lineaActual.substr(0, punto + 1);
                elementoListaOrd.textoContenido = lineaActual.substr(punto + 2);
                estadoTexto.elementosMarkdown.push_back(elementoListaOrd);
                continue;
            }
        }

        // Listas desordenadas - o *
        if (lineaActual.rfind(L"- ", 0) == 0 ||
            lineaActual.rfind(L"* ", 0) == 0) {
            ElementoMarkdown elementoLista;
            elementoLista.tipo = TipoElementoMarkdown::ListaDesordenada;
            elementoLista.textoContenido = lineaActual.substr(2);
            estadoTexto.elementosMarkdown.push_back(elementoLista);
            continue;
        }

        // Imágenes ![alt](url)
        if (lineaActual.rfind(L"![", 0) == 0) {
            size_t posicionCierreAlt = lineaActual.find(L"](");
            size_t posicionCierreUrl = lineaActual.find(L')', posicionCierreAlt);
            if (posicionCierreAlt != std::wstring::npos &&
                posicionCierreUrl != std::wstring::npos) {
                std::wstring textoAlt =
                    lineaActual.substr(2, posicionCierreAlt - 2);
                std::wstring rutaUrl = lineaActual.substr(
                    posicionCierreAlt + 2,
                    posicionCierreUrl - posicionCierreAlt - 2);

                std::wstring idYouTube = extraerIdVideoYouTube(rutaUrl);
                if (!idYouTube.empty()) {
                    ElementoMarkdown elementoVideo;
                    elementoVideo.tipo = TipoElementoMarkdown::VideoYouTube;
                    elementoVideo.textoContenido =
                        textoAlt.empty() ? L"Video de YouTube" : textoAlt;
                    elementoVideo.textoSecundario = rutaUrl;
                    estadoTexto.elementosMarkdown.push_back(elementoVideo);
                } else {
                    ElementoMarkdown elementoImagen;
                    elementoImagen.tipo = TipoElementoMarkdown::Imagen;
                    elementoImagen.textoContenido = textoAlt;
                    elementoImagen.textoSecundario = rutaUrl;
                    estadoTexto.elementosMarkdown.push_back(elementoImagen);
                }
                continue;
            }
        }

        // Enlaces [texto](url) o video YouTube directo
        std::wstring idDirectoYouTube = extraerIdVideoYouTube(lineaActual);
        if (!idDirectoYouTube.empty()) {
            ElementoMarkdown elementoVideo;
            elementoVideo.tipo = TipoElementoMarkdown::VideoYouTube;
            elementoVideo.textoContenido = L"Reproducir video de YouTube";
            elementoVideo.textoSecundario = lineaActual;
            estadoTexto.elementosMarkdown.push_back(elementoVideo);
            continue;
        }

        if (lineaActual.rfind(L"[", 0) == 0) {
            size_t posicionCierreTexto = lineaActual.find(L"](");
            size_t posicionCierreUrl =
                lineaActual.find(L')', posicionCierreTexto);
            if (posicionCierreTexto != std::wstring::npos &&
                posicionCierreUrl != std::wstring::npos) {
                std::wstring textoEnlace =
                    lineaActual.substr(1, posicionCierreTexto - 1);
                std::wstring rutaUrl = lineaActual.substr(
                    posicionCierreTexto + 2,
                    posicionCierreUrl - posicionCierreTexto - 2);

                std::wstring idYt = extraerIdVideoYouTube(rutaUrl);
                if (!idYt.empty()) {
                    ElementoMarkdown elementoVideo;
                    elementoVideo.tipo = TipoElementoMarkdown::VideoYouTube;
                    elementoVideo.textoContenido =
                        textoEnlace.empty() ? L"Video de YouTube" : textoEnlace;
                    elementoVideo.textoSecundario = rutaUrl;
                    estadoTexto.elementosMarkdown.push_back(elementoVideo);
                } else {
                    ElementoMarkdown elementoEnlace;
                    elementoEnlace.tipo = TipoElementoMarkdown::EnlaceWeb;
                    elementoEnlace.textoContenido = textoEnlace;
                    elementoEnlace.textoSecundario = rutaUrl;
                    estadoTexto.elementosMarkdown.push_back(elementoEnlace);
                }
                continue;
            }
        }

        // Párrafo estándar
        ElementoMarkdown elementoParrafo;
        elementoParrafo.tipo = TipoElementoMarkdown::Parrafo;
        elementoParrafo.textoContenido = lineaActual;
        estadoTexto.elementosMarkdown.push_back(elementoParrafo);
    }

    if (dentroDeBloqueCodigo && !acumuladorBloqueCodigo.empty()) {
        ElementoMarkdown elementoCodigo;
        elementoCodigo.tipo = TipoElementoMarkdown::BloqueCodigo;
        elementoCodigo.textoContenido = acumuladorBloqueCodigo;
        elementoCodigo.textoSecundario = lenguajeCodigo;
        estadoTexto.elementosMarkdown.push_back(elementoCodigo);
    }
}

inline void parsearContenidoTextoPlano(const std::wstring& contenidoCompleto) {
    estadoTexto.elementosMarkdown.clear();
    estadoTexto.barrasIndice.clear();

    if (contenidoCompleto.empty()) {
        ElementoMarkdown elementoVacio;
        elementoVacio.tipo = TipoElementoMarkdown::Parrafo;
        elementoVacio.textoContenido = L"*(Archivo de texto sin contenido)*";
        estadoTexto.elementosMarkdown.push_back(elementoVacio);
        return;
    }

    size_t posicionInicio = 0;
    size_t longitudTexto = contenidoCompleto.size();

    while (posicionInicio < longitudTexto) {
        size_t posicionFin = contenidoCompleto.find(L'\n', posicionInicio);
        if (posicionFin == std::wstring::npos) {
            posicionFin = longitudTexto;
        }

        std::wstring lineaActual = contenidoCompleto.substr(
            posicionInicio, posicionFin - posicionInicio);
        if (!lineaActual.empty() && lineaActual.back() == L'\r') {
            lineaActual.pop_back();
        }

        posicionInicio = posicionFin + 1;
        estadoTexto.totalLineasOriginales++;

        if (estadoTexto.elementosMarkdown.size() >= 2500) {
            ElementoMarkdown elementoAviso;
            elementoAviso.tipo = TipoElementoMarkdown::Parrafo;
            elementoAviso.textoContenido = L"... [Vista previa limitada a las primeras 2,500 líneas por rendimiento]";
            estadoTexto.elementosMarkdown.push_back(elementoAviso);
            break;
        }

        ElementoMarkdown elementoLinea;
        elementoLinea.tipo = TipoElementoMarkdown::Parrafo;
        elementoLinea.textoContenido = lineaActual;
        estadoTexto.elementosMarkdown.push_back(elementoLinea);
    }
}

inline bool cargarTextoPorRuta(const std::wstring& rutaTexto) {
    if (rutaTexto.empty()) {
        limpiarContenidoTextoCargado();
        return false;
    }

    limpiarContenidoTextoCargado();
    estadoTexto.rutaTextoActual = rutaTexto;

    const wchar_t* punteroNombre = PathFindFileNameW(rutaTexto.c_str());
    estadoTexto.nombreArchivo =
        punteroNombre ? punteroNombre : rutaTexto;

    const wchar_t* extension = PathFindExtensionW(rutaTexto.c_str());
    estadoTexto.esFormatoMarkdown = esExtensionMarkdown(extension);

    std::vector<std::wstring> listaArchivosTexto =
        obtenerArchivosTextoEnMismaCarpeta(rutaTexto);
    estadoTexto.totalEnCarpeta =
        std::max<size_t>(1, listaArchivosTexto.size());
    auto iterador = std::find(listaArchivosTexto.begin(), listaArchivosTexto.end(),
                              rutaTexto);
    if (iterador != listaArchivosTexto.end()) {
        estadoTexto.posicionEnCarpeta =
            static_cast<size_t>(
                std::distance(listaArchivosTexto.begin(), iterador)) +
            1;
    } else {
        estadoTexto.posicionEnCarpeta = 1;
    }

    std::wstring contenidoTexto = leerArchivoTextoUtf8(rutaTexto);
    if (estadoTexto.esFormatoMarkdown) {
        parsearContenidoMarkdown(contenidoTexto);
    } else {
        parsearContenidoTextoPlano(contenidoTexto);
    }
    return true;
}

inline bool mostrarTextoAnterior() {
    if (estadoTexto.textosSeleccionados.size() > 1) {
        estadoTexto.indiceTextoSeleccionado =
            (estadoTexto.indiceTextoSeleccionado == 0)
                ? (estadoTexto.textosSeleccionados.size() - 1)
                : (estadoTexto.indiceTextoSeleccionado - 1);
        return cargarTextoPorRuta(
            estadoTexto.textosSeleccionados[estadoTexto.indiceTextoSeleccionado]);
    }
    if (estadoTexto.rutaTextoActual.empty()) return false;
    std::wstring rutaAnterior =
        obtenerArchivoTextoAnteriorEnCarpeta(estadoTexto.rutaTextoActual);
    if (rutaAnterior.empty() ||
        rutaAnterior == estadoTexto.rutaTextoActual) {
        return false;
    }
    estadoTexto.textosSeleccionados = { rutaAnterior };
    estadoTexto.indiceTextoSeleccionado = 0;
    return cargarTextoPorRuta(rutaAnterior);
}

inline bool mostrarTextoSiguiente() {
    if (estadoTexto.textosSeleccionados.size() > 1) {
        estadoTexto.indiceTextoSeleccionado =
            (estadoTexto.indiceTextoSeleccionado + 1) %
            estadoTexto.textosSeleccionados.size();
        return cargarTextoPorRuta(
            estadoTexto.textosSeleccionados[estadoTexto.indiceTextoSeleccionado]);
    }
    if (estadoTexto.rutaTextoActual.empty()) return false;
    std::wstring rutaSiguiente =
        obtenerArchivoTextoSiguienteEnCarpeta(estadoTexto.rutaTextoActual);
    if (rutaSiguiente.empty() ||
        rutaSiguiente == estadoTexto.rutaTextoActual) {
        return false;
    }
    estadoTexto.textosSeleccionados = { rutaSiguiente };
    estadoTexto.indiceTextoSeleccionado = 0;
    return cargarTextoPorRuta(rutaSiguiente);
}

inline void desplazarScrollTexto(int incrementoDelta) {
    int maximoScroll = std::max(0, estadoTexto.altoTotalContenido - 200);
    estadoTexto.desplazamientoScrollY = std::max(
        0, std::min(estadoTexto.desplazamientoScrollY + incrementoDelta,
                    maximoScroll));
}

inline int dibujarFragmentosEnriquecidos(
    HDC hdc, const std::wstring& texto, int xInicio, int yInicio, int anchoMax,
    HFONT fuenteCuerpo, HFONT fuenteNegrita, HFONT fuenteCursiva,
    HFONT fuenteSubrayada, HFONT fuenteCodigoPildora) {
    if (texto.empty()) return 0;

    std::vector<FragmentoTexto> fragmentos = parsearFragmentosTexto(texto);
    if (fragmentos.size() == 1 && fragmentos[0].estilo == EstiloFragmento::Normal) {
        SelectObject(hdc, fuenteCuerpo);
        SetTextColor(hdc, RGB(230, 237, 243));
        RECT r{xInicio, yInicio, xInicio + anchoMax, yInicio + 2000};
        int alto = DrawTextW(hdc, texto.c_str(), -1, &r, DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
        DrawTextW(hdc, texto.c_str(), -1, &r, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
        return alto;
    }

    int xActual = xInicio;
    int yActual = yInicio;
    int altoLinea = 20;

    for (const auto& frag : fragmentos) {
        HFONT fuenteElegida = fuenteCuerpo;
        COLORREF colorTexto = RGB(230, 237, 243);

        if (frag.estilo == EstiloFragmento::Negrita) {
            fuenteElegida = fuenteNegrita;
            colorTexto = RGB(255, 255, 255);
        } else if (frag.estilo == EstiloFragmento::Cursiva) {
            fuenteElegida = fuenteCursiva;
            colorTexto = RGB(230, 237, 243);
        } else if (frag.estilo == EstiloFragmento::Subrayado) {
            fuenteElegida = fuenteSubrayada;
            colorTexto = RGB(88, 166, 255);
        } else if (frag.estilo == EstiloFragmento::CodigoPildora) {
            fuenteElegida = fuenteCodigoPildora;
            colorTexto = RGB(240, 136, 62);
        }

        SelectObject(hdc, fuenteElegida);
        SetTextColor(hdc, colorTexto);

        size_t p = 0;
        while (p < frag.texto.size()) {
            size_t proxEspacio = frag.texto.find_first_of(L" \t", p);
            if (proxEspacio == std::wstring::npos) proxEspacio = frag.texto.size();
            std::wstring palabra = frag.texto.substr(p, proxEspacio - p);
            if (proxEspacio < frag.texto.size()) palabra += frag.texto[proxEspacio];

            SIZE tamPalabra{0, 0};
            GetTextExtentPoint32W(hdc, palabra.c_str(), static_cast<int>(palabra.size()), &tamPalabra);

            if (xActual + tamPalabra.cx > xInicio + anchoMax && xActual > xInicio) {
                xActual = xInicio;
                yActual += altoLinea;
            }

            if (frag.estilo == EstiloFragmento::CodigoPildora) {
                RECT rPildora{xActual - 2, yActual + 1, xActual + tamPalabra.cx + 2, yActual + altoLinea - 1};
                HBRUSH bPildora = CreateSolidBrush(RGB(45, 51, 59));
                HPEN lPildora = CreatePen(PS_SOLID, 1, RGB(68, 76, 86));
                HBRUSH bPrev = static_cast<HBRUSH>(SelectObject(hdc, bPildora));
                HPEN lPrev = static_cast<HPEN>(SelectObject(hdc, lPildora));
                RoundRect(hdc, rPildora.left, rPildora.top, rPildora.right, rPildora.bottom, 4, 4);
                SelectObject(hdc, lPrev);
                SelectObject(hdc, bPrev);
                DeleteObject(lPildora);
                DeleteObject(bPildora);
            }

            RECT rTexto{xActual, yActual, xActual + tamPalabra.cx, yActual + altoLinea};
            DrawTextW(hdc, palabra.c_str(), -1, &rTexto, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            xActual += tamPalabra.cx;
            p = (proxEspacio < frag.texto.size()) ? proxEspacio + 1 : frag.texto.size();
        }
    }

    return (yActual - yInicio) + altoLinea;
}

inline void dibujarLienzoTexto(HDC contextoDestino,
                               const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    // Fondo GitHub Dark: RGB(13, 17, 23)
    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteH1 = CreateFontW(
        -22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteH2 = CreateFontW(
        -18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteH3 = CreateFontW(
        -15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteH4 = CreateFontW(
        -13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteCuerpo = CreateFontW(
        -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteNegrita = CreateFontW(
        -13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteCursiva = CreateFontW(
        -13, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubrayada = CreateFontW(
        -13, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteCodigo = CreateFontW(
        -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
    HFONT fuenteCodigoPildora = CreateFontW(
        -12, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
    HFONT fuentePequena = CreateFontW(
        -11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int margenIzquierdo = areaDibujo.left + 24;
    int margenDerechoTexto = areaDibujo.right - 44;

    int posicionVerticalActual =
        areaDibujo.top + 20 - estadoTexto.desplazamientoScrollY;

    estadoTexto.barrasIndice.clear();

    for (size_t posicionElemento = 0;
         posicionElemento < estadoTexto.elementosMarkdown.size();
         ++posicionElemento) {
        ElementoMarkdown& elemento =
            estadoTexto.elementosMarkdown[posicionElemento];
        elemento.posicionVerticalY = posicionVerticalActual;

        if (elemento.altoCalculado == 0) {
            if (elemento.tipo == TipoElementoMarkdown::Encabezado1) {
                elemento.altoCalculado = 36;
            } else if (elemento.tipo == TipoElementoMarkdown::Encabezado2) {
                elemento.altoCalculado = 32;
            } else if (elemento.tipo == TipoElementoMarkdown::Encabezado3) {
                elemento.altoCalculado = 28;
            } else if (elemento.tipo == TipoElementoMarkdown::Encabezado4) {
                elemento.altoCalculado = 24;
            } else if (elemento.tipo == TipoElementoMarkdown::ReglaHorizontal) {
                elemento.altoCalculado = 14;
            } else if (elemento.tipo == TipoElementoMarkdown::Imagen) {
                elemento.altoCalculado = 180;
            } else if (elemento.tipo == TipoElementoMarkdown::VideoYouTube) {
                elemento.altoCalculado = 240;
            } else {
                elemento.altoCalculado = elemento.textoContenido.empty() ? 8 : 22;
            }
        }

        bool esVisible =
            (posicionVerticalActual + elemento.altoCalculado >=
                 areaDibujo.top - 100 &&
             posicionVerticalActual <= areaDibujo.bottom + 100);

        if (!esVisible) {
            if (estadoTexto.esFormatoMarkdown) {
                if (elemento.tipo == TipoElementoMarkdown::Encabezado1) {
                    ElementoIndiceBarras barraH1;
                    barraH1.tituloEncabezado = elemento.textoContenido;
                    barraH1.nivelEncabezado = 1;
                    barraH1.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH1);
                } else if (elemento.tipo == TipoElementoMarkdown::Encabezado2) {
                    ElementoIndiceBarras barraH2;
                    barraH2.tituloEncabezado = elemento.textoContenido;
                    barraH2.nivelEncabezado = 2;
                    barraH2.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH2);
                } else if (elemento.tipo == TipoElementoMarkdown::Encabezado3) {
                    ElementoIndiceBarras barraH3;
                    barraH3.tituloEncabezado = elemento.textoContenido;
                    barraH3.nivelEncabezado = 3;
                    barraH3.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH3);
                }
            }
            posicionVerticalActual += elemento.altoCalculado;
            continue;
        }

        switch (elemento.tipo) {
            case TipoElementoMarkdown::Encabezado1: {
                SelectObject(contextoDestino, fuenteH1);
                SetTextColor(contextoDestino, RGB(245, 245, 245));
                RECT areaEncabezado{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEncabezado,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEncabezado,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                // Línea divisoria inferior GitHub H1
                HPEN lapizDivisor = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizDivisor));
                MoveToEx(contextoDestino, margenIzquierdo,
                         posicionVerticalActual + altoTexto + 6, nullptr);
                LineTo(contextoDestino, margenDerechoTexto,
                       posicionVerticalActual + altoTexto + 6);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(lapizDivisor);

                if (estadoTexto.esFormatoMarkdown) {
                    ElementoIndiceBarras barraH1;
                    barraH1.tituloEncabezado = elemento.textoContenido;
                    barraH1.nivelEncabezado = 1;
                    barraH1.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH1);
                }

                elemento.altoCalculado = altoTexto + 18;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Encabezado2: {
                SelectObject(contextoDestino, fuenteH2);
                SetTextColor(contextoDestino, RGB(245, 245, 245));
                RECT areaEncabezado{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEncabezado,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEncabezado,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                // Línea divisoria inferior GitHub H2
                HPEN lapizDivisor = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizDivisor));
                MoveToEx(contextoDestino, margenIzquierdo,
                         posicionVerticalActual + altoTexto + 4, nullptr);
                LineTo(contextoDestino, margenDerechoTexto,
                       posicionVerticalActual + altoTexto + 4);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(lapizDivisor);

                if (estadoTexto.esFormatoMarkdown) {
                    ElementoIndiceBarras barraH2;
                    barraH2.tituloEncabezado = elemento.textoContenido;
                    barraH2.nivelEncabezado = 2;
                    barraH2.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH2);
                }

                elemento.altoCalculado = altoTexto + 14;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Encabezado3: {
                SelectObject(contextoDestino, fuenteH3);
                SetTextColor(contextoDestino, RGB(245, 245, 245));
                RECT areaEncabezado{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEncabezado,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEncabezado,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                if (estadoTexto.esFormatoMarkdown) {
                    ElementoIndiceBarras barraH3;
                    barraH3.tituloEncabezado = elemento.textoContenido;
                    barraH3.nivelEncabezado = 3;
                    barraH3.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraH3);
                }

                elemento.altoCalculado = altoTexto + 10;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Encabezado4: {
                SelectObject(contextoDestino, fuenteH4);
                SetTextColor(contextoDestino, RGB(220, 220, 220));
                RECT areaEncabezado{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEncabezado,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEncabezado,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                elemento.altoCalculado = altoTexto + 8;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::BloqueCodigo: {
                SelectObject(contextoDestino, fuenteCodigo);
                RECT areaCalculo{margenIzquierdo + 12,
                                 posicionVerticalActual + 10,
                                 margenDerechoTexto - 12,
                                 posicionVerticalActual + 2000};
                int altoCodigo = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaCalculo,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);

                int altoCaja = altoCodigo + 20;
                RECT areaCajaCodigo{margenIzquierdo, posicionVerticalActual,
                                    margenDerechoTexto,
                                    posicionVerticalActual + altoCaja};

                HBRUSH brochaCaja = CreateSolidBrush(RGB(22, 27, 34));
                HPEN lapizCaja = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HBRUSH brochaPreviaCaja = static_cast<HBRUSH>(
                    SelectObject(contextoDestino, brochaCaja));
                HPEN lapizPrevioCaja = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizCaja));

                RoundRect(contextoDestino, areaCajaCodigo.left,
                          areaCajaCodigo.top, areaCajaCodigo.right,
                          areaCajaCodigo.bottom, 6, 6);

                SelectObject(contextoDestino, brochaPreviaCaja);
                SelectObject(contextoDestino, lapizPrevioCaja);
                DeleteObject(brochaCaja);
                DeleteObject(lapizCaja);

                SetTextColor(contextoDestino, RGB(201, 209, 217));
                RECT areaTextoCodigo{
                    margenIzquierdo + 12, posicionVerticalActual + 10,
                    margenDerechoTexto - 12, posicionVerticalActual + altoCaja - 10};
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaTextoCodigo,
                          DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                elemento.altoCalculado = altoCaja + 12;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::CitaBloque: {
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino, RGB(139, 148, 158));
                RECT areaCita{margenIzquierdo + 16, posicionVerticalActual,
                               margenDerechoTexto,
                               posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaCita,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaCita, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                // Barra vertical izquierda azul Fluent/GitHub
                HBRUSH brochaBarra = CreateSolidBrush(RGB(88, 166, 255));
                RECT areaBarraCita{margenIzquierdo, posicionVerticalActual,
                                   margenIzquierdo + 4,
                                   posicionVerticalActual + altoTexto};
                FillRect(contextoDestino, &areaBarraCita, brochaBarra);
                DeleteObject(brochaBarra);

                elemento.altoCalculado = altoTexto + 8;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::ListaDesordenada: {
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino, RGB(88, 166, 255));
                RECT areaVina{margenIzquierdo + 4, posicionVerticalActual,
                              margenIzquierdo + 18,
                              posicionVerticalActual + 20};
                DrawTextW(contextoDestino, L"\u2022", -1, &areaVina,
                          DT_LEFT | DT_NOPREFIX);

                SetTextColor(contextoDestino, RGB(230, 237, 243));
                RECT areaTexto{margenIzquierdo + 20, posicionVerticalActual,
                               margenDerechoTexto,
                               posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaTexto,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaTexto, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                elemento.altoCalculado = altoTexto + 6;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::CasillaVerificacion: {
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino,
                             elemento.casillaMarcada ? RGB(88, 166, 255)
                                                     : RGB(139, 148, 158));
                RECT areaCasilla{margenIzquierdo + 4, posicionVerticalActual,
                                 margenIzquierdo + 22,
                                 posicionVerticalActual + 20};
                DrawTextW(contextoDestino,
                          elemento.casillaMarcada ? L"\u2611" : L"\u2610", -1,
                          &areaCasilla, DT_LEFT | DT_NOPREFIX);

                SetTextColor(contextoDestino, elemento.casillaMarcada
                                                  ? RGB(139, 148, 158)
                                                  : RGB(230, 237, 243));
                RECT areaTexto{margenIzquierdo + 24, posicionVerticalActual,
                               margenDerechoTexto,
                               posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaTexto,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaTexto, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                elemento.altoCalculado = altoTexto + 6;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::ReglaHorizontal: {
                HPEN lapizRegla = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizRegla));
                MoveToEx(contextoDestino, margenIzquierdo,
                         posicionVerticalActual + 6, nullptr);
                LineTo(contextoDestino, margenDerechoTexto,
                       posicionVerticalActual + 6);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(lapizRegla);

                elemento.altoCalculado = 14;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::SeparadorSeccionPersonalizado: {
                SelectObject(contextoDestino, fuenteH4);
                RECT areaCalculo{0, 0, margenDerechoTexto - margenIzquierdo, 100};
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaCalculo, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
                int anchoTexto = areaCalculo.right - areaCalculo.left;
                int centroY = posicionVerticalActual + 14;
                int centroX = (margenIzquierdo + margenDerechoTexto) / 2;

                HPEN lapizSeparador = CreatePen(PS_SOLID, 2, RGB(48, 54, 61));
                HPEN lapizPrevio = static_cast<HPEN>(SelectObject(contextoDestino, lapizSeparador));

                if (anchoTexto > 0 && !elemento.textoContenido.empty()) {
                    int mitadTexto = anchoTexto / 2 + 10;
                    int xFinIzq = std::max(margenIzquierdo, centroX - mitadTexto);
                    int xIniDer = std::min(margenDerechoTexto, centroX + mitadTexto);

                    MoveToEx(contextoDestino, margenIzquierdo, centroY, nullptr);
                    LineTo(contextoDestino, xFinIzq, centroY);

                    MoveToEx(contextoDestino, xIniDer, centroY, nullptr);
                    LineTo(contextoDestino, margenDerechoTexto, centroY);

                    SelectObject(contextoDestino, lapizPrevio);
                    DeleteObject(lapizSeparador);

                    SetTextColor(contextoDestino, RGB(121, 192, 255));
                    RECT areaTextoSep{xFinIzq, posicionVerticalActual, xIniDer, posicionVerticalActual + 28};
                    DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                              &areaTextoSep, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                } else {
                    MoveToEx(contextoDestino, margenIzquierdo, centroY, nullptr);
                    LineTo(contextoDestino, margenDerechoTexto, centroY);
                    SelectObject(contextoDestino, lapizPrevio);
                    DeleteObject(lapizSeparador);
                }

                if (estadoTexto.esFormatoMarkdown && !elemento.textoContenido.empty()) {
                    ElementoIndiceBarras barraSep;
                    barraSep.tituloEncabezado = elemento.textoContenido;
                    barraSep.nivelEncabezado = 2;
                    barraSep.posicionDestinoY = std::max<int>(
                        0, static_cast<int>(elemento.posicionVerticalY +
                                            estadoTexto.desplazamientoScrollY -
                                            areaDibujo.top - 20));
                    estadoTexto.barrasIndice.push_back(barraSep);
                }

                elemento.altoCalculado = 28;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Imagen: {
                Gdiplus::Bitmap* mapaBits = obtenerBitmapParaMarkdown(
                    elemento.textoSecundario,
                    estadoTexto.rutaTextoActual);

                int anchoDisponible = margenDerechoTexto - margenIzquierdo;
                int altoRenderizado = 160;
                int anchoRenderizado = anchoDisponible;

                if (mapaBits && mapaBits->GetLastStatus() == Gdiplus::Ok) {
                    UINT anchoOriginal = mapaBits->GetWidth();
                    UINT altoOriginal = mapaBits->GetHeight();
                    if (anchoOriginal > 0 && altoOriginal > 0) {
                        double escala = std::min(
                            static_cast<double>(anchoDisponible) /
                                static_cast<double>(anchoOriginal),
                            280.0 / static_cast<double>(altoOriginal));
                        anchoRenderizado = std::max(
                            40, static_cast<int>(static_cast<double>(anchoOriginal) * escala));
                        altoRenderizado = std::max(
                            40, static_cast<int>(static_cast<double>(altoOriginal) * escala));
                    }

                    Gdiplus::Graphics lienzoGdiplus(contextoDestino);
                    lienzoGdiplus.SetInterpolationMode(
                        Gdiplus::InterpolationModeHighQualityBicubic);
                    lienzoGdiplus.DrawImage(mapaBits, margenIzquierdo,
                                           posicionVerticalActual,
                                           anchoRenderizado, altoRenderizado);

                    HPEN lapizBordeImagen =
                        CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                    HPEN lapizPrevioImg = static_cast<HPEN>(
                        SelectObject(contextoDestino, lapizBordeImagen));
                    HBRUSH brochaNula =
                        static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
                    HBRUSH brochaPreviaImg = static_cast<HBRUSH>(
                        SelectObject(contextoDestino, brochaNula));

                    Rectangle(contextoDestino, margenIzquierdo,
                              posicionVerticalActual,
                              margenIzquierdo + anchoRenderizado,
                              posicionVerticalActual + altoRenderizado);

                    SelectObject(contextoDestino, lapizPrevioImg);
                    SelectObject(contextoDestino, brochaPreviaImg);
                    DeleteObject(lapizBordeImagen);
                } else {
                    RECT areaCajaImagen{margenIzquierdo, posicionVerticalActual,
                                        margenDerechoTexto,
                                        posicionVerticalActual + 80};
                    HBRUSH brochaCaja = CreateSolidBrush(RGB(22, 27, 34));
                    HPEN lapizCaja = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                    HBRUSH brochaPrevia = static_cast<HBRUSH>(
                        SelectObject(contextoDestino, brochaCaja));
                    HPEN lapizPrevio = static_cast<HPEN>(
                        SelectObject(contextoDestino, lapizCaja));

                    RoundRect(contextoDestino, areaCajaImagen.left,
                              areaCajaImagen.top, areaCajaImagen.right,
                              areaCajaImagen.bottom, 6, 6);

                    SelectObject(contextoDestino, brochaPrevia);
                    SelectObject(contextoDestino, lapizPrevio);
                    DeleteObject(brochaCaja);
                    DeleteObject(lapizCaja);

                    SelectObject(contextoDestino, fuentePequena);
                    SetTextColor(contextoDestino, RGB(139, 148, 158));
                    std::wstring etiquetaImagen =
                        L"[Imagen: " +
                        (elemento.textoContenido.empty()
                             ? elemento.textoSecundario
                             : elemento.textoContenido) +
                        L"]";
                    DrawTextW(contextoDestino, etiquetaImagen.c_str(), -1,
                              &areaCajaImagen,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE |
                                  DT_NOPREFIX);
                    altoRenderizado = 80;
                }

                if (!elemento.textoContenido.empty()) {
                    SelectObject(contextoDestino, fuentePequena);
                    SetTextColor(contextoDestino, RGB(139, 148, 158));
                    RECT areaPie{margenIzquierdo,
                                 posicionVerticalActual + altoRenderizado + 4,
                                 margenDerechoTexto,
                                 posicionVerticalActual + altoRenderizado + 24};
                    DrawTextW(contextoDestino, elemento.textoContenido.c_str(),
                              -1, &areaPie,
                              DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
                    altoRenderizado += 24;
                }

                ElementoClicableMarkdown clicable;
                clicable.areaClic = {margenIzquierdo, posicionVerticalActual,
                                     margenIzquierdo + anchoRenderizado,
                                     posicionVerticalActual + altoRenderizado};
                clicable.destinoUrl = elemento.textoSecundario;
                clicable.esVideoYouTube = false;
                estadoTexto.elementosClicables.push_back(clicable);

                elemento.altoCalculado = altoRenderizado + 12;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::VideoYouTube: {
                std::wstring idVideo =
                    extraerIdVideoYouTube(elemento.textoSecundario);
                std::wstring urlMiniatura =
                    idVideo.empty()
                        ? L""
                        : (L"https://img.youtube.com/vi/" + idVideo +
                           L"/hqdefault.jpg");

                Gdiplus::Bitmap* miniatura =
                    obtenerBitmapParaMarkdown(urlMiniatura, L"");

                int anchoDisponible = margenDerechoTexto - margenIzquierdo;
                int anchoVideo = std::min(anchoDisponible, 340);
                int altoVideo = (anchoVideo * 9) / 16;

                RECT areaTarjeta{margenIzquierdo, posicionVerticalActual,
                                 margenIzquierdo + anchoVideo,
                                 posicionVerticalActual + altoVideo + 32};

                HBRUSH brochaFondoTarjeta =
                    CreateSolidBrush(RGB(22, 27, 34));
                HPEN lapizBordeTarjeta =
                    CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HBRUSH brochaPrevia = static_cast<HBRUSH>(
                    SelectObject(contextoDestino, brochaFondoTarjeta));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizBordeTarjeta));

                RoundRect(contextoDestino, areaTarjeta.left, areaTarjeta.top,
                          areaTarjeta.right, areaTarjeta.bottom, 8, 8);

                SelectObject(contextoDestino, brochaPrevia);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(brochaFondoTarjeta);
                DeleteObject(lapizBordeTarjeta);

                if (miniatura && miniatura->GetLastStatus() == Gdiplus::Ok) {
                    Gdiplus::Graphics lienzoGdiplus(contextoDestino);
                    lienzoGdiplus.SetInterpolationMode(
                        Gdiplus::InterpolationModeHighQualityBicubic);
                    lienzoGdiplus.DrawImage(miniatura, margenIzquierdo + 2,
                                           posicionVerticalActual + 2,
                                           anchoVideo - 4, altoVideo - 4);
                }

                // Botón central rojo YouTube Play (\u25B6)
                int centroX = margenIzquierdo + (anchoVideo / 2);
                int centroY = posicionVerticalActual + (altoVideo / 2);
                RECT areaBotonPlay{centroX - 22, centroY - 14, centroX + 22,
                                   centroY + 14};

                HBRUSH brochaBotonPlay = CreateSolidBrush(RGB(255, 0, 0));
                HBRUSH brochaPreviaPlay = static_cast<HBRUSH>(
                    SelectObject(contextoDestino, brochaBotonPlay));
                RoundRect(contextoDestino, areaBotonPlay.left,
                          areaBotonPlay.top, areaBotonPlay.right,
                          areaBotonPlay.bottom, 8, 8);
                SelectObject(contextoDestino, brochaPreviaPlay);
                DeleteObject(brochaBotonPlay);

                SelectObject(contextoDestino, fuentePequena);
                SetTextColor(contextoDestino, RGB(255, 255, 255));
                DrawTextW(contextoDestino, L"\u25B6", -1, &areaBotonPlay,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                // Pie de la tarjeta YouTube
                RECT areaPieYouTube{margenIzquierdo + 8,
                                    posicionVerticalActual + altoVideo + 2,
                                    margenIzquierdo + anchoVideo - 8,
                                    areaTarjeta.bottom};
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino, RGB(245, 245, 245));
                std::wstring etiquetaPie =
                    elemento.textoContenido.empty()
                        ? L"YouTube \u2022 Reproducir video"
                        : (elemento.textoContenido + L" \u2022 YouTube");
                DrawTextW(contextoDestino, etiquetaPie.c_str(), -1,
                          &areaPieYouTube,
                          DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                ElementoClicableMarkdown clicable;
                clicable.areaClic = areaTarjeta;
                clicable.destinoUrl = elemento.textoSecundario;
                clicable.esVideoYouTube = true;
                estadoTexto.elementosClicables.push_back(clicable);

                elemento.altoCalculado = altoVideo + 44;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::EnlaceWeb: {
                SelectObject(contextoDestino, fuenteCuerpo);
                SetTextColor(contextoDestino, RGB(88, 166, 255));
                RECT areaEnlace{margenIzquierdo, posicionVerticalActual,
                                 margenDerechoTexto,
                                 posicionVerticalActual + 1000};
                int altoTexto = DrawTextW(
                    contextoDestino, elemento.textoContenido.c_str(), -1,
                    &areaEnlace,
                    DT_LEFT | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
                DrawTextW(contextoDestino, elemento.textoContenido.c_str(), -1,
                          &areaEnlace, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

                HPEN lapizEnlace = CreatePen(PS_SOLID, 1, RGB(88, 166, 255));
                HPEN lapizPrevio = static_cast<HPEN>(
                    SelectObject(contextoDestino, lapizEnlace));
                MoveToEx(contextoDestino, margenIzquierdo,
                         posicionVerticalActual + altoTexto + 2, nullptr);
                LineTo(contextoDestino, areaEnlace.right,
                       posicionVerticalActual + altoTexto + 2);
                SelectObject(contextoDestino, lapizPrevio);
                DeleteObject(lapizEnlace);

                ElementoClicableMarkdown clicable;
                clicable.areaClic = areaEnlace;
                clicable.destinoUrl = elemento.textoSecundario;
                clicable.esVideoYouTube = false;
                estadoTexto.elementosClicables.push_back(clicable);

                elemento.altoCalculado = altoTexto + 8;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::TablaMarkdown: {
                if (elemento.filasTabla.empty()) break;
                size_t numColumnas = 0;
                for (const auto& fila : elemento.filasTabla) {
                    numColumnas = std::max(numColumnas, fila.celdas.size());
                }
                if (numColumnas == 0) numColumnas = 1;

                int anchoTabla = margenDerechoTexto - margenIzquierdo;
                int anchoCol = anchoTabla / static_cast<int>(numColumnas);
                int altoFila = 28;
                int altoTabla = static_cast<int>(elemento.filasTabla.size()) * altoFila;

                RECT areaTabla{margenIzquierdo, posicionVerticalActual,
                               margenIzquierdo + anchoTabla,
                               posicionVerticalActual + altoTabla};

                HBRUSH brochaFondoTabla = CreateSolidBrush(RGB(13, 17, 23));
                HPEN lapizBordeTabla = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                HBRUSH brochaPrevTabla = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaFondoTabla));
                HPEN lapizPrevTabla = static_cast<HPEN>(SelectObject(contextoDestino, lapizBordeTabla));

                RoundRect(contextoDestino, areaTabla.left, areaTabla.top, areaTabla.right, areaTabla.bottom, 8, 8);

                SelectObject(contextoDestino, lapizPrevTabla);
                SelectObject(contextoDestino, brochaPrevTabla);
                DeleteObject(lapizBordeTabla);
                DeleteObject(brochaFondoTabla);

                for (size_t idxFila = 0; idxFila < elemento.filasTabla.size(); ++idxFila) {
                    const auto& fila = elemento.filasTabla[idxFila];
                    int yFila = posicionVerticalActual + static_cast<int>(idxFila) * altoFila;

                    if (fila.esCabecera) {
                        HBRUSH brochaCabecera = CreateSolidBrush(RGB(22, 27, 34));
                        RECT areaCabecera{margenIzquierdo + 1, yFila + 1,
                                          margenIzquierdo + anchoTabla - 1, yFila + altoFila};
                        FillRect(contextoDestino, &areaCabecera, brochaCabecera);
                        DeleteObject(brochaCabecera);
                    } else if (idxFila % 2 == 1) {
                        HBRUSH brochaFilaPar = CreateSolidBrush(RGB(22, 27, 34));
                        RECT areaFilaPar{margenIzquierdo + 1, yFila,
                                         margenIzquierdo + anchoTabla - 1, yFila + altoFila};
                        FillRect(contextoDestino, &areaFilaPar, brochaFilaPar);
                        DeleteObject(brochaFilaPar);
                    }

                    if (idxFila > 0) {
                        HPEN lapizDiv = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                        HPEN lapizPrevDiv = static_cast<HPEN>(SelectObject(contextoDestino, lapizDiv));
                        MoveToEx(contextoDestino, margenIzquierdo, yFila, nullptr);
                        LineTo(contextoDestino, margenIzquierdo + anchoTabla, yFila);
                        SelectObject(contextoDestino, lapizPrevDiv);
                        DeleteObject(lapizDiv);
                    }

                    for (size_t idxCol = 0; idxCol < numColumnas; ++idxCol) {
                        int xCol = margenIzquierdo + static_cast<int>(idxCol) * anchoCol;
                        int xColFin = (idxCol + 1 == numColumnas) ? (margenIzquierdo + anchoTabla) : (xCol + anchoCol);

                        if (idxCol > 0) {
                            HPEN lapizDivV = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
                            HPEN lapizPrevDivV = static_cast<HPEN>(SelectObject(contextoDestino, lapizDivV));
                            MoveToEx(contextoDestino, xCol, yFila, nullptr);
                            LineTo(contextoDestino, xCol, yFila + altoFila);
                            SelectObject(contextoDestino, lapizPrevDivV);
                            DeleteObject(lapizDivV);
                        }

                        std::wstring textoCelda = (idxCol < fila.celdas.size()) ? fila.celdas[idxCol] : L"";
                        RECT areaTextoCelda{xCol + 8, yFila, xColFin - 8, yFila + altoFila};
                        HFONT fuenteCelda = fila.esCabecera ? fuenteNegrita : fuenteCuerpo;
                        HFONT fuentePrevia = static_cast<HFONT>(SelectObject(contextoDestino, fuenteCelda));
                        SetTextColor(contextoDestino, fila.esCabecera ? RGB(240, 246, 252) : RGB(201, 209, 217));
                        DrawTextW(contextoDestino, textoCelda.c_str(), -1, &areaTextoCelda,
                                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
                        SelectObject(contextoDestino, fuentePrevia);
                    }
                }

                elemento.altoCalculado = altoTabla + 14;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::ListaOrdenada: {
                SelectObject(contextoDestino, fuenteNegrita);
                SetTextColor(contextoDestino, RGB(88, 166, 255));
                RECT areaNum{margenIzquierdo + 4, posicionVerticalActual,
                             margenIzquierdo + 24, posicionVerticalActual + 20};
                DrawTextW(contextoDestino, elemento.textoSecundario.c_str(), -1, &areaNum,
                          DT_LEFT | DT_NOPREFIX);

                int altoTexto = dibujarFragmentosEnriquecidos(
                    contextoDestino, elemento.textoContenido, margenIzquierdo + 26,
                    posicionVerticalActual, margenDerechoTexto - (margenIzquierdo + 26),
                    fuenteCuerpo, fuenteNegrita, fuenteCursiva, fuenteSubrayada,
                    fuenteCodigoPildora);

                elemento.altoCalculado = std::max(20, altoTexto) + 6;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
            case TipoElementoMarkdown::Parrafo:
            default: {
                if (elemento.textoContenido.empty()) {
                    elemento.altoCalculado = 8;
                    posicionVerticalActual += elemento.altoCalculado;
                    break;
                }
                int altoTexto = dibujarFragmentosEnriquecidos(
                    contextoDestino, elemento.textoContenido, margenIzquierdo,
                    posicionVerticalActual, margenDerechoTexto - margenIzquierdo,
                    fuenteCuerpo, fuenteNegrita, fuenteCursiva, fuenteSubrayada,
                    fuenteCodigoPildora);

                elemento.altoCalculado = std::max(18, altoTexto) + 8;
                posicionVerticalActual += elemento.altoCalculado;
                break;
            }
        }
    }

    estadoTexto.altoTotalContenido =
        posicionVerticalActual + estadoTexto.desplazamientoScrollY -
        areaDibujo.top + 40;

    // -------------------------------------------------------------
    // RENDERIZADO DEL ÍNDICE VERTICAL CON BARRAS (TOC BARS)
    // EXCLUSIVO PARA ARCHIVOS MARKDOWN (.MD)
    // -------------------------------------------------------------
    if (estadoTexto.esFormatoMarkdown &&
        !estadoTexto.barrasIndice.empty()) {
        int totalBarras =
            static_cast<int>(estadoTexto.barrasIndice.size());
        int alturaEspaciadoBarras = 14;
        int alturaTotalIndice = totalBarras * alturaEspaciadoBarras;
        int topeIndiceY =
            areaDibujo.top +
            std::max<int>(20, (altoArea - alturaTotalIndice) / 2);
        int bordeDerechoIndice = areaDibujo.right - 12;

        for (int posicionBarra = 0; posicionBarra < totalBarras;
             ++posicionBarra) {
            ElementoIndiceBarras& barra =
                estadoTexto.barrasIndice[static_cast<size_t>(posicionBarra)];
            int posicionYBarra = topeIndiceY + posicionBarra * alturaEspaciadoBarras;

            int anchoBarra = 12;
            if (barra.nivelEncabezado == 1) {
                anchoBarra = 18;
            } else if (barra.nivelEncabezado == 2) {
                anchoBarra = 14;
            }

            RECT areaRectBarra{bordeDerechoIndice - anchoBarra, posicionYBarra,
                               bordeDerechoIndice, posicionYBarra + 6};
            barra.areaBarra = areaRectBarra;

            bool esBarraHover = (posicionBarra == estadoTexto.posicionBarraHover);
            COLORREF colorBarra = esBarraHover
                                      ? RGB(88, 166, 255)
                                      : RGB(48, 54, 61);

            HBRUSH brochaBarra = CreateSolidBrush(colorBarra);
            FillRect(contextoDestino, &areaRectBarra, brochaBarra);
            DeleteObject(brochaBarra);
        }

        // Si hay hover sobre una barra, desplegar tarjeta con el título
        if (estadoTexto.posicionBarraHover >= 0 &&
            estadoTexto.posicionBarraHover < totalBarras) {
            const ElementoIndiceBarras& barraActiva =
                estadoTexto.barrasIndice[static_cast<size_t>(
                    estadoTexto.posicionBarraHover)];
            int centroY = (barraActiva.areaBarra.top +
                           barraActiva.areaBarra.bottom) /
                          2;

            SelectObject(contextoDestino, fuentePequena);
            RECT areaCalculoTooltip{0, 0, 300, 100};
            DrawTextW(contextoDestino, barraActiva.tituloEncabezado.c_str(),
                      -1, &areaCalculoTooltip,
                      DT_LEFT | DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);

            int anchoTooltip = areaCalculoTooltip.right - areaCalculoTooltip.left + 16;
            int altoTooltip = 24;

            RECT areaTooltip{
                bordeDerechoIndice - 30 - anchoTooltip, centroY - altoTooltip / 2,
                bordeDerechoIndice - 30, centroY + altoTooltip / 2};

            HBRUSH brochaTooltip = CreateSolidBrush(RGB(38, 38, 38));
            HPEN lapizTooltip = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
            HBRUSH brochaPreviaTooltip = static_cast<HBRUSH>(
                SelectObject(contextoDestino, brochaTooltip));
            HPEN lapizPrevioTooltip = static_cast<HPEN>(
                SelectObject(contextoDestino, lapizTooltip));

            RoundRect(contextoDestino, areaTooltip.left, areaTooltip.top,
                      areaTooltip.right, areaTooltip.bottom, 4, 4);

            SelectObject(contextoDestino, brochaPreviaTooltip);
            SelectObject(contextoDestino, lapizPrevioTooltip);
            DeleteObject(brochaTooltip);
            DeleteObject(lapizTooltip);

            SetTextColor(contextoDestino, RGB(245, 245, 245));
            RECT areaTextoTooltip = areaTooltip;
            areaTextoTooltip.left += 8;
            areaTextoTooltip.right -= 8;
            DrawTextW(contextoDestino, barraActiva.tituloEncabezado.c_str(),
                      -1, &areaTextoTooltip,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
    }

    DeleteObject(fuenteH1);
    DeleteObject(fuenteH2);
    DeleteObject(fuenteH3);
    DeleteObject(fuenteH4);
    DeleteObject(fuenteCuerpo);
    DeleteObject(fuenteNegrita);
    DeleteObject(fuenteCursiva);
    DeleteObject(fuenteSubrayada);
    DeleteObject(fuenteCodigo);
    DeleteObject(fuenteCodigoPildora);
    DeleteObject(fuentePequena);
}

inline void dibujarVistaTexto(HDC contextoDestino,
                              const RECT& areaDibujo) {
    // Barra de estado y navegación superior para archivos de texto y markdown (34px)
    HBRUSH brochaBarra = CreateSolidBrush(RGB(25, 25, 25));
    FillRect(contextoDestino, &areaDibujo, brochaBarra);
    DeleteObject(brochaBarra);

    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevio =
        static_cast<HPEN>(SelectObject(contextoDestino, lapizBorde));
    MoveToEx(contextoDestino, areaDibujo.left, areaDibujo.bottom - 1, nullptr);
    LineTo(contextoDestino, areaDibujo.right, areaDibujo.bottom - 1);
    SelectObject(contextoDestino, lapizPrevio);
    DeleteObject(lapizBorde);

    // Botón Anterior
    RECT areaBotonAnt{areaDibujo.left + 10, areaDibujo.top + 4,
                      areaDibujo.left + 42, areaDibujo.bottom - 4};
    estadoTexto.botonAnterior = areaBotonAnt;

    HBRUSH brochaAnt = CreateSolidBrush(
        estadoTexto.hoverBotonAnterior ? RGB(65, 65, 65) : RGB(40, 40, 40));
    HPEN lapizAnt = CreatePen(
        PS_SOLID, 1,
        estadoTexto.hoverBotonAnterior ? RGB(100, 100, 100) : RGB(55, 55, 55));
    HBRUSH brochaPreviaAnt =
        static_cast<HBRUSH>(SelectObject(contextoDestino, brochaAnt));
    HPEN lapizPrevioAnt =
        static_cast<HPEN>(SelectObject(contextoDestino, lapizAnt));
    RoundRect(contextoDestino, areaBotonAnt.left, areaBotonAnt.top,
              areaBotonAnt.right, areaBotonAnt.bottom, 4, 4);
    SelectObject(contextoDestino, brochaPreviaAnt);
    SelectObject(contextoDestino, lapizPrevioAnt);
    DeleteObject(brochaAnt);
    DeleteObject(lapizAnt);

    // Botón Siguiente
    RECT areaBotonSig{areaDibujo.left + 48, areaDibujo.top + 4,
                      areaDibujo.left + 80, areaDibujo.bottom - 4};
    estadoTexto.botonSiguiente = areaBotonSig;

    HBRUSH brochaSig = CreateSolidBrush(
        estadoTexto.hoverBotonSiguiente ? RGB(65, 65, 65) : RGB(40, 40, 40));
    HPEN lapizSig = CreatePen(
        PS_SOLID, 1,
        estadoTexto.hoverBotonSiguiente ? RGB(100, 100, 100) : RGB(55, 55, 55));
    HBRUSH brochaPreviaSig =
        static_cast<HBRUSH>(SelectObject(contextoDestino, brochaSig));
    HPEN lapizPrevioSig =
        static_cast<HPEN>(SelectObject(contextoDestino, lapizSig));
    RoundRect(contextoDestino, areaBotonSig.left, areaBotonSig.top,
              areaBotonSig.right, areaBotonSig.bottom, 4, 4);
    SelectObject(contextoDestino, brochaPreviaSig);
    SelectObject(contextoDestino, lapizPrevioSig);
    DeleteObject(brochaSig);
    DeleteObject(lapizSig);

    // Botón Editar
    RECT areaBotonEditar{areaDibujo.left + 88, areaDibujo.top + 4,
                         areaDibujo.left + 172, areaDibujo.bottom - 4};
    estadoTexto.botonEditar = areaBotonEditar;

    HBRUSH brochaEdit = CreateSolidBrush(
        estadoTexto.hoverBotonEditar ? RGB(65, 65, 65) : RGB(40, 40, 40));
    HPEN lapizEdit = CreatePen(
        PS_SOLID, 1,
        estadoTexto.hoverBotonEditar ? RGB(100, 100, 100) : RGB(55, 55, 55));
    HBRUSH brochaPreviaEdit =
        static_cast<HBRUSH>(SelectObject(contextoDestino, brochaEdit));
    HPEN lapizPrevioEdit =
        static_cast<HPEN>(SelectObject(contextoDestino, lapizEdit));
    RoundRect(contextoDestino, areaBotonEditar.left, areaBotonEditar.top,
              areaBotonEditar.right, areaBotonEditar.bottom, 4, 4);
    SelectObject(contextoDestino, brochaPreviaEdit);
    SelectObject(contextoDestino, lapizPrevioEdit);
    DeleteObject(brochaEdit);
    DeleteObject(lapizEdit);

    HFONT fuenteBotones = CreateFontW(
        -12, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuentePrevia =
        static_cast<HFONT>(SelectObject(contextoDestino, fuenteBotones));

    SetBkMode(contextoDestino, TRANSPARENT);
    SetTextColor(contextoDestino, RGB(245, 245, 245));
    DrawTextW(contextoDestino, L"◀", -1, &areaBotonAnt,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    DrawTextW(contextoDestino, L"▶", -1, &areaBotonSig,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    DrawTextW(contextoDestino, L"✏️ Editar", -1, &areaBotonEditar,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // Texto de estado
    HFONT fuenteEstado = CreateFontW(
        -11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    SelectObject(contextoDestino, fuenteEstado);
    DeleteObject(fuenteBotones);

    SetTextColor(contextoDestino, RGB(139, 148, 158));
    std::wstring tipoDoc =
        estadoTexto.esFormatoMarkdown ? L"Markdown" : L"Texto Plano";
    std::wstring textoEstado;
    if (estadoTexto.textosSeleccionados.size() > 1) {
        textoEstado =
            tipoDoc + L"  \u2022  " + estadoTexto.nombreArchivo +
            L"  \u2022  " +
            std::to_wstring(estadoTexto.totalLineasOriginales) +
            L" líneas  \u2022  " +
            std::to_wstring(estadoTexto.indiceTextoSeleccionado + 1) + L"/" +
            std::to_wstring(estadoTexto.textosSeleccionados.size());
    } else {
        textoEstado =
            tipoDoc + L"  \u2022  " + estadoTexto.nombreArchivo +
            L"  \u2022  " +
            std::to_wstring(estadoTexto.totalLineasOriginales) +
            L" líneas";
    }

    RECT areaTexto{areaDibujo.left + 182, areaDibujo.top, areaDibujo.right - 10,
                   areaDibujo.bottom};
    DrawTextW(contextoDestino, textoEstado.c_str(), -1, &areaTexto,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    SelectObject(contextoDestino, fuentePrevia);
    DeleteObject(fuenteEstado);
}


/**** ended inlining texto/controlador_texto.hpp ****/
/**** start inlining comprimidos/controlador_comprimidos.hpp ****/

#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06020000
#endif

#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <compressapi.h>
#include <commdlg.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

enum class CategoriaElementoComprimido {
    Carpetas,
    Videos,
    Imagenes,
    Audios,
    Documentos,
    Codigo,
    Texto,
    Comprimidos,
    Otros,
    TotalCategorias
};

struct ElementoComprimido {
    std::wstring rutaArchivo;
    std::wstring nombreArchivo;
    std::wstring extensionFormato;
    std::wstring textoTamano;
    Gdiplus::Image* portadaCargada{nullptr};
    bool esComic{false};
};

struct ElementoArbolComprimido {
    std::wstring nombre;
    std::wstring rutaCompleta;
    bool esDirectorio{false};
    int nivelJerarquia{0};
    uintmax_t tamanoBytes{0};
};

struct ElementoContenidoComprimido {
    std::wstring nombre;
    std::wstring rutaRelativa;
    CategoriaElementoComprimido categoria{CategoriaElementoComprimido::Otros};
    bool esDirectorio{false};
    uintmax_t tamanoBytes{0};
    RECT areaElemento{};
};

struct SeccionContenidoComprimido {
    CategoriaElementoComprimido categoria{CategoriaElementoComprimido::Otros};
    std::wstring tituloSeccion;
    std::wstring iconoSeccion;
    std::vector<ElementoContenidoComprimido> elementos;
    RECT areaCabecera{};
};

struct EstadoControladorComprimidos {
    std::vector<std::wstring> archivosSeleccionados;
    size_t indiceArchivoActual{0};
    size_t totalSeleccionados{0};
    bool tieneComicsSeleccionados{false};
    bool esModoComic{false};

    // Modo Cómic: portadas
    std::vector<ElementoComprimido> elementosSeleccionados;

    // Modo Comprimidos Comunes: Secciones estructuradas por categoría
    std::vector<ElementoArbolComprimido> elementosArbol;
    std::vector<ElementoContenidoComprimido> todosLosElementos;
    std::vector<SeccionContenidoComprimido> seccionesContenido;
    std::wstring rutaComprimidoArbolCargado;
    FILETIME ftUltimaModificacionArbol{};
    int desplazamientoScrollY{0};
    int altoTotalArbol{0};
    RECT areaCajaArbol{};

    // Navegación entre comprimidos múltiples
    RECT botonAnteriorComprimido{};
    RECT botonSiguienteComprimido{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};

    // Programas y Conversión
    std::vector<std::wstring> programasComicsRecordados;
    std::vector<std::wstring> programasComunesRecordados;
    int indiceProgramaSeleccionado{0};
    bool menuProgramasAbierto{false};
    int opcionProgramaHover{-1};
    std::wstring programaTemporal;
    int indiceFormatoDestino{0};
    bool menuDesplegableAbierto{false};
    int opcionFormatoHover{-1};
    bool conversionEnCurso{false};

    // Protección con contraseña para comprimidos
    bool protegidoConContrasena{false};
    bool contrasenaDesbloqueada{false};
    RECT areaBotonDesbloquear{};
    bool hoverBotonDesbloquear{false};
};

inline EstadoControladorComprimidos estadoComprimidos;

inline std::vector<std::wstring>& obtenerListaProgramasActiva() {
    return estadoComprimidos.tieneComicsSeleccionados
               ? estadoComprimidos.programasComicsRecordados
               : estadoComprimidos.programasComunesRecordados;
}

inline const std::vector<std::wstring>& obtenerListaProgramasActivaConst() {
    return estadoComprimidos.tieneComicsSeleccionados
               ? estadoComprimidos.programasComicsRecordados
               : estadoComprimidos.programasComunesRecordados;
}

namespace {

inline std::wstring formatearTamanoArchivo(uintmax_t tamanoEnBytes) {
    if (tamanoEnBytes < 1024) {
        return std::to_wstring(tamanoEnBytes) + L" B";
    }
    if (tamanoEnBytes < 1024 * 1024) {
        wchar_t buferTamano[32];
        swprintf_s(buferTamano, L"%.1f KB",
                   static_cast<double>(tamanoEnBytes) / 1024.0);
        return buferTamano;
    }
    wchar_t buferTamano[32];
    swprintf_s(buferTamano, L"%.1f MB",
               static_cast<double>(tamanoEnBytes) / (1024.0 * 1024.0));
    return buferTamano;
}

#pragma pack(push, 1)
struct CabeceraArchivoZipLocal {
    uint32_t firma;
    uint16_t versionMinima;
    uint16_t banderaGeneral;
    uint16_t metodoCompresion;
    uint16_t horaModificacion;
    uint16_t fechaModificacion;
    uint32_t sumaCrc32;
    uint32_t tamanoComprimido;
    uint32_t tamanoDescomprimido;
    uint16_t longitudNombreArchivo;
    uint16_t longitudCampoExtra;
};
#pragma pack(pop)

inline Gdiplus::Image* extraerPortadaDesdeShell(const std::wstring& rutaArchivo) {
    Gdiplus::Image* imagenExtraida = nullptr;
    IShellItem* elementoShell = nullptr;
    if (SUCCEEDED(SHCreateItemFromParsingName(rutaArchivo.c_str(), nullptr,
                                             IID_PPV_ARGS(&elementoShell)))) {
        IShellItemImageFactory* fabricaImagenes = nullptr;
        if (SUCCEEDED(elementoShell->QueryInterface(
                IID_PPV_ARGS(&fabricaImagenes)))) {
            HBITMAP mapaBitsHBitmap = nullptr;
            SIZE tamanoDeseado{320, 480};
            HRESULT resultadoImagen = fabricaImagenes->GetImage(
                tamanoDeseado, SIIGBF_BIGGERSIZEOK, &mapaBitsHBitmap);
            if (SUCCEEDED(resultadoImagen) && mapaBitsHBitmap) {
                Gdiplus::Bitmap* mapaBitsTemp =
                    Gdiplus::Bitmap::FromHBITMAP(mapaBitsHBitmap, nullptr);
                if (mapaBitsTemp &&
                    mapaBitsTemp->GetLastStatus() == Gdiplus::Ok) {
                    imagenExtraida = mapaBitsTemp;
                } else if (mapaBitsTemp) {
                    delete mapaBitsTemp;
                }
                DeleteObject(mapaBitsHBitmap);
            }
            fabricaImagenes->Release();
        }
        elementoShell->Release();
    }
    return imagenExtraida;
}

inline Gdiplus::Image* extraerPrimeraImagenDeZip(const std::wstring& rutaArchivo) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return nullptr;

    CabeceraArchivoZipLocal cabeceraLocal{};
    DWORD bytesLeidos = 0;
    while (ReadFile(manejadorArchivo, &cabeceraLocal, sizeof(cabeceraLocal),
                    &bytesLeidos, nullptr) &&
           bytesLeidos == sizeof(cabeceraLocal)) {
        if (cabeceraLocal.firma != 0x04034b50) break;

        if (cabeceraLocal.longitudNombreArchivo > 0 &&
            cabeceraLocal.longitudNombreArchivo < 1024) {
            std::vector<char> nombreEntrada(
                cabeceraLocal.longitudNombreArchivo + 1, 0);
            if (!ReadFile(manejadorArchivo, nombreEntrada.data(),
                          cabeceraLocal.longitudNombreArchivo, &bytesLeidos,
                          nullptr) ||
                bytesLeidos != cabeceraLocal.longitudNombreArchivo) {
                break;
            }

            if (cabeceraLocal.longitudCampoExtra > 0) {
                SetFilePointer(manejadorArchivo,
                               cabeceraLocal.longitudCampoExtra, nullptr,
                               FILE_CURRENT);
            }

            std::string nombreArchivoEntrada(nombreEntrada.data());
            std::string extensionEntrada;
            size_t posicionPunto = nombreArchivoEntrada.find_last_of('.');
            if (posicionPunto != std::string::npos) {
                extensionEntrada =
                    nombreArchivoEntrada.substr(posicionPunto);
            }
            for (char& caracter : extensionEntrada) {
                caracter = static_cast<char>(tolower(caracter));
            }

            bool esImagen = (extensionEntrada == ".jpg" ||
                             extensionEntrada == ".jpeg" ||
                             extensionEntrada == ".png" ||
                             extensionEntrada == ".webp" ||
                             extensionEntrada == ".bmp");

            if (esImagen && cabeceraLocal.tamanoDescomprimido > 0 &&
                cabeceraLocal.tamanoDescomprimido < 32 * 1024 * 1024) {
                if (cabeceraLocal.metodoCompresion == 0) {
                    HGLOBAL memoriaGlobal = GlobalAlloc(
                        GMEM_MOVEABLE, cabeceraLocal.tamanoDescomprimido);
                    if (memoriaGlobal) {
                        void* direccionMemoria = GlobalLock(memoriaGlobal);
                        if (direccionMemoria &&
                            ReadFile(manejadorArchivo, direccionMemoria,
                                     cabeceraLocal.tamanoDescomprimido,
                                     &bytesLeidos, nullptr) &&
                            bytesLeidos == cabeceraLocal.tamanoDescomprimido) {
                            GlobalUnlock(memoriaGlobal);
                            IStream* flujoStream = nullptr;
                            if (SUCCEEDED(CreateStreamOnHGlobal(
                                    memoriaGlobal, TRUE, &flujoStream))) {
                                Gdiplus::Bitmap* mapaBitsTemp =
                                    Gdiplus::Bitmap::FromStream(flujoStream);
                                Gdiplus::Image* imagenPortada = nullptr;
                                if (mapaBitsTemp &&
                                    mapaBitsTemp->GetLastStatus() ==
                                        Gdiplus::Ok) {
                                    imagenPortada = mapaBitsTemp->Clone(
                                        0, 0,
                                        static_cast<INT>(
                                            mapaBitsTemp->GetWidth()),
                                        static_cast<INT>(
                                            mapaBitsTemp->GetHeight()),
                                        PixelFormat32bppARGB);
                                }
                                if (mapaBitsTemp) delete mapaBitsTemp;
                                flujoStream->Release();
                                if (imagenPortada &&
                                    imagenPortada->GetLastStatus() ==
                                        Gdiplus::Ok) {
                                    CloseHandle(manejadorArchivo);
                                    return imagenPortada;
                                }
                                if (imagenPortada) delete imagenPortada;
                            }
                        } else {
                            GlobalUnlock(memoriaGlobal);
                            GlobalFree(memoriaGlobal);
                        }
                    }
                } else if (cabeceraLocal.metodoCompresion == 8 &&
                           cabeceraLocal.tamanoComprimido > 0) {
                    std::vector<BYTE> buferComprimido(
                        cabeceraLocal.tamanoComprimido);
                    if (ReadFile(manejadorArchivo, buferComprimido.data(),
                                 cabeceraLocal.tamanoComprimido, &bytesLeidos,
                                 nullptr) &&
                        bytesLeidos == cabeceraLocal.tamanoComprimido) {
                        constexpr DWORD ALGORITMO_DEFLATE_RAW =
                            COMPRESS_ALGORITHM_MSZIP | COMPRESS_RAW;
                        DECOMPRESSOR_HANDLE descompresor = nullptr;
                        if (CreateDecompressor(ALGORITMO_DEFLATE_RAW,
                                               nullptr, &descompresor)) {
                            HGLOBAL memoriaGlobal = GlobalAlloc(
                                GMEM_MOVEABLE,
                                cabeceraLocal.tamanoDescomprimido);
                            if (memoriaGlobal) {
                                void* direccionMemoria =
                                    GlobalLock(memoriaGlobal);
                                SIZE_T tamanoDescomprimidoReal = 0;
                                if (direccionMemoria &&
                                    Decompress(
                                        descompresor, buferComprimido.data(),
                                        cabeceraLocal.tamanoComprimido,
                                        direccionMemoria,
                                        cabeceraLocal.tamanoDescomprimido,
                                        &tamanoDescomprimidoReal)) {
                                    GlobalUnlock(memoriaGlobal);
                                    IStream* flujoStream = nullptr;
                                    if (SUCCEEDED(CreateStreamOnHGlobal(
                                            memoriaGlobal, TRUE,
                                            &flujoStream))) {
                                        Gdiplus::Bitmap* mapaBitsTemp =
                                            Gdiplus::Bitmap::FromStream(
                                                flujoStream);
                                        Gdiplus::Image* imagenPortada =
                                            nullptr;
                                        if (mapaBitsTemp &&
                                            mapaBitsTemp->GetLastStatus() ==
                                                Gdiplus::Ok) {
                                            imagenPortada =
                                                mapaBitsTemp->Clone(
                                                    0, 0,
                                                    static_cast<INT>(
                                                        mapaBitsTemp
                                                            ->GetWidth()),
                                                    static_cast<INT>(
                                                        mapaBitsTemp
                                                            ->GetHeight()),
                                                    PixelFormat32bppARGB);
                                        }
                                        if (mapaBitsTemp) delete mapaBitsTemp;
                                        flujoStream->Release();
                                        if (imagenPortada &&
                                            imagenPortada->GetLastStatus() ==
                                                Gdiplus::Ok) {
                                            CloseDecompressor(descompresor);
                                            CloseHandle(manejadorArchivo);
                                            return imagenPortada;
                                        }
                                        if (imagenPortada) delete imagenPortada;
                                    }
                                } else {
                                    GlobalUnlock(memoriaGlobal);
                                    GlobalFree(memoriaGlobal);
                                }
                            }
                            CloseDecompressor(descompresor);
                        }
                    }
                }
            } else if (cabeceraLocal.tamanoComprimido > 0) {
                SetFilePointer(
                    manejadorArchivo,
                    static_cast<LONG>(cabeceraLocal.tamanoComprimido), nullptr,
                    FILE_CURRENT);
            }
        } else {
            break;
        }
    }

    CloseHandle(manejadorArchivo);
    return nullptr;
}

inline Gdiplus::Image* obtenerPortadaDeComprimido(const std::wstring& rutaActual) {
    Gdiplus::Image* imagenPortada = extraerPortadaDesdeShell(rutaActual);
    if (imagenPortada) return imagenPortada;

    const wchar_t* extension = PathFindExtensionW(rutaActual.c_str());
    if (lstrcmpiW(extension, L".zip") == 0 ||
        lstrcmpiW(extension, L".cbz") == 0) {
        imagenPortada = extraerPrimeraImagenDeZip(rutaActual);
        if (imagenPortada) return imagenPortada;
    }
    return nullptr;
}

}  // namespace

inline void liberarRecursosComprimido() {
    for (ElementoComprimido& elemento :
         estadoComprimidos.elementosSeleccionados) {
        if (elemento.portadaCargada) {
            delete elemento.portadaCargada;
            elemento.portadaCargada = nullptr;
        }
    }
    estadoComprimidos.elementosSeleccionados.clear();
    estadoComprimidos.archivosSeleccionados.clear();
    estadoComprimidos.elementosArbol.clear();
    estadoComprimidos.todosLosElementos.clear();
    estadoComprimidos.seccionesContenido.clear();
    estadoComprimidos.rutaComprimidoArbolCargado.clear();
    estadoComprimidos.ftUltimaModificacionArbol = {};
    estadoComprimidos.desplazamientoScrollY = 0;
    estadoComprimidos.altoTotalArbol = 0;
    estadoComprimidos.totalSeleccionados = 0;
    estadoComprimidos.indiceArchivoActual = 0;
    estadoComprimidos.tieneComicsSeleccionados = false;
    estadoComprimidos.esModoComic = false;
    estadoComprimidos.menuDesplegableAbierto = false;
    estadoComprimidos.opcionFormatoHover = -1;
    estadoComprimidos.menuProgramasAbierto = false;
    estadoComprimidos.opcionProgramaHover = -1;
    estadoComprimidos.hoverBotonAnterior = false;
    estadoComprimidos.hoverBotonSiguiente = false;
    estadoComprimidos.protegidoConContrasena = false;
    estadoComprimidos.contrasenaDesbloqueada = false;
    estadoComprimidos.areaBotonDesbloquear = {};
    estadoComprimidos.hoverBotonDesbloquear = false;
}

inline CategoriaElementoComprimido clasificarCategoriaElementoComprimido(const std::wstring& ruta, bool esDirectorio) {
    if (esDirectorio) {
        return CategoriaElementoComprimido::Carpetas;
    }
    const wchar_t* extensionPuntero = PathFindExtensionW(ruta.c_str());
    if (!extensionPuntero || !*extensionPuntero) {
        return CategoriaElementoComprimido::Otros;
    }
    std::wstring extension = extensionPuntero;
    for (wchar_t& caracter : extension) {
        caracter = static_cast<wchar_t>(towlower(caracter));
    }

    if (extension == L".mp4" || extension == L".mkv" || extension == L".avi" ||
        extension == L".mov" || extension == L".wmv" || extension == L".flv" ||
        extension == L".webm" || extension == L".m4v" || extension == L".ts" ||
        extension == L".3gp" || extension == L".mpeg" || extension == L".mpg") {
        return CategoriaElementoComprimido::Videos;
    }

    if (extension == L".jpg" || extension == L".jpeg" || extension == L".png" ||
        extension == L".webp" || extension == L".bmp" || extension == L".gif" ||
        extension == L".ico" || extension == L".svg" || extension == L".tiff" ||
        extension == L".tif" || extension == L".avif" || extension == L".heic" ||
        extension == L".raw" || extension == L".psd") {
        return CategoriaElementoComprimido::Imagenes;
    }

    if (extension == L".mp3" || extension == L".wav" || extension == L".flac" ||
        extension == L".aac" || extension == L".m4a" || extension == L".ogg" ||
        extension == L".opus" || extension == L".wma" || extension == L".mid" ||
        extension == L".midi" || extension == L".aiff" || extension == L".alac") {
        return CategoriaElementoComprimido::Audios;
    }

    if (extension == L".pdf" || extension == L".doc" || extension == L".docx" ||
        extension == L".xls" || extension == L".xlsx" || extension == L".ppt" ||
        extension == L".pptx" || extension == L".epub" || extension == L".mobi" ||
        extension == L".csv" || extension == L".rtf" || extension == L".odt" ||
        extension == L".ods" || extension == L".odp" || extension == L".pages") {
        return CategoriaElementoComprimido::Documentos;
    }

    if (extension == L".cpp" || extension == L".hpp" || extension == L".c" ||
        extension == L".h" || extension == L".cs" || extension == L".py" ||
        extension == L".js" || extension == L".ts" || extension == L".jsx" ||
        extension == L".tsx" || extension == L".html" || extension == L".htm" ||
        extension == L".css" || extension == L".scss" || extension == L".sass" ||
        extension == L".json" || extension == L".xml" || extension == L".yaml" ||
        extension == L".yml" || extension == L".sql" || extension == L".sh" ||
        extension == L".ps1" || extension == L".bat" || extension == L".cmd" ||
        extension == L".java" || extension == L".rs" || extension == L".go" ||
        extension == L".php" || extension == L".rb" || extension == L".swift" ||
        extension == L".kt" || extension == L".dart" || extension == L".lua" ||
        extension == L".wh.cpp") {
        return CategoriaElementoComprimido::Codigo;
    }

    if (extension == L".txt" || extension == L".log" || extension == L".md" ||
        extension == L".ini" || extension == L".cfg" || extension == L".conf" ||
        extension == L".reg" || extension == L".inf" || extension == L".properties") {
        return CategoriaElementoComprimido::Texto;
    }

    if (extension == L".zip" || extension == L".rar" || extension == L".7z" ||
        extension == L".tar" || extension == L".gz" || extension == L".bz2" ||
        extension == L".xz" || extension == L".cab" || extension == L".iso" ||
        extension == L".cbz" || extension == L".cbr" || extension == L".tgz" ||
        extension == L".z") {
        return CategoriaElementoComprimido::Comprimidos;
    }

    return CategoriaElementoComprimido::Otros;
}

inline std::wstring obtenerTituloCategoriaComprimido(CategoriaElementoComprimido categoria) {
    switch (categoria) {
        case CategoriaElementoComprimido::Carpetas:
            return L"Carpetas y directorios";
        case CategoriaElementoComprimido::Videos:
            return L"Archivos de vídeo";
        case CategoriaElementoComprimido::Imagenes:
            return L"Imágenes y fotografías";
        case CategoriaElementoComprimido::Audios:
            return L"Archivos de audio";
        case CategoriaElementoComprimido::Documentos:
            return L"Documentos y ofimática";
        case CategoriaElementoComprimido::Codigo:
            return L"Código y scripts";
        case CategoriaElementoComprimido::Texto:
            return L"Archivos de texto y notas";
        case CategoriaElementoComprimido::Comprimidos:
            return L"Archivos comprimidos";
        case CategoriaElementoComprimido::Otros:
        default:
            return L"Otros archivos y binarios";
    }
}

inline std::wstring obtenerIconoCategoriaComprimido(CategoriaElementoComprimido categoria) {
    switch (categoria) {
        case CategoriaElementoComprimido::Carpetas:
            return L"📁";
        case CategoriaElementoComprimido::Videos:
            return L"🎬";
        case CategoriaElementoComprimido::Imagenes:
            return L"🖼️";
        case CategoriaElementoComprimido::Audios:
            return L"🎵";
        case CategoriaElementoComprimido::Documentos:
            return L"📄";
        case CategoriaElementoComprimido::Codigo:
            return L"💻";
        case CategoriaElementoComprimido::Texto:
            return L"📝";
        case CategoriaElementoComprimido::Comprimidos:
            return L"📦";
        case CategoriaElementoComprimido::Otros:
        default:
            return L"💾";
    }
}

inline void agruparContenidosEnSecciones(const std::vector<ElementoContenidoComprimido>& listaElementos) {
    estadoComprimidos.todosLosElementos = listaElementos;
    estadoComprimidos.seccionesContenido.clear();

    std::vector<ElementoContenidoComprimido> elementosPorCategoria[static_cast<size_t>(CategoriaElementoComprimido::TotalCategorias)];

    for (const auto& elementoActual : listaElementos) {
        size_t indiceCategoria = static_cast<size_t>(elementoActual.categoria);
        if (indiceCategoria < static_cast<size_t>(CategoriaElementoComprimido::TotalCategorias)) {
            elementosPorCategoria[indiceCategoria].push_back(elementoActual);
        }
    }

    for (size_t posicionCategoria = 0; posicionCategoria < static_cast<size_t>(CategoriaElementoComprimido::TotalCategorias); ++posicionCategoria) {
        std::sort(elementosPorCategoria[posicionCategoria].begin(), elementosPorCategoria[posicionCategoria].end(),
            [](const ElementoContenidoComprimido& primerElemento, const ElementoContenidoComprimido& segundoElemento) {
                return StrCmpLogicalW(primerElemento.rutaRelativa.c_str(), segundoElemento.rutaRelativa.c_str()) < 0;
            });
    }

    for (size_t posicionCategoria = 0; posicionCategoria < static_cast<size_t>(CategoriaElementoComprimido::TotalCategorias); ++posicionCategoria) {
        if (!elementosPorCategoria[posicionCategoria].empty()) {
            SeccionContenidoComprimido seccion;
            seccion.categoria = static_cast<CategoriaElementoComprimido>(posicionCategoria);
            seccion.tituloSeccion = obtenerTituloCategoriaComprimido(seccion.categoria);
            seccion.iconoSeccion = obtenerIconoCategoriaComprimido(seccion.categoria);
            seccion.elementos = elementosPorCategoria[posicionCategoria];
            estadoComprimidos.seccionesContenido.push_back(seccion);
        }
    }
}

inline bool inspeccionarArchivoZipCompleto(const std::wstring& rutaArchivo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return false;

    CabeceraArchivoZipLocal cabeceraLocal{};
    DWORD bytesLeidos = 0;
    std::map<std::wstring, ElementoContenidoComprimido> mapaElementos;
    std::map<std::wstring, bool> mapaCarpetasDetectadas;

    while (ReadFile(manejadorArchivo, &cabeceraLocal, sizeof(cabeceraLocal),
                    &bytesLeidos, nullptr) &&
           bytesLeidos == sizeof(cabeceraLocal)) {
        if (cabeceraLocal.firma != 0x04034b50) break;

        if ((cabeceraLocal.banderaGeneral & 0x0001) != 0) {
            estadoComprimidos.protegidoConContrasena = true;
        }

        if (cabeceraLocal.longitudNombreArchivo > 0 &&
            cabeceraLocal.longitudNombreArchivo < 4096) {
            std::vector<char> nombreEntrada(
                cabeceraLocal.longitudNombreArchivo + 1, 0);
            if (!ReadFile(manejadorArchivo, nombreEntrada.data(),
                          cabeceraLocal.longitudNombreArchivo, &bytesLeidos,
                          nullptr) ||
                bytesLeidos != cabeceraLocal.longitudNombreArchivo) {
                break;
            }

            if (cabeceraLocal.longitudCampoExtra > 0) {
                SetFilePointer(manejadorArchivo,
                               cabeceraLocal.longitudCampoExtra, nullptr,
                               FILE_CURRENT);
            }

            std::string nombreUtf8(nombreEntrada.data());
            for (char& caracter : nombreUtf8) {
                if (caracter == '/') caracter = '\\';
            }

            int longitudAncha = MultiByteToWideChar(CP_UTF8, 0, nombreUtf8.c_str(), -1, nullptr, 0);
            if (longitudAncha <= 0) {
                longitudAncha = MultiByteToWideChar(CP_ACP, 0, nombreUtf8.c_str(), -1, nullptr, 0);
            }
            std::wstring rutaRelativaW;
            if (longitudAncha > 0) {
                std::vector<wchar_t> buferAncho(static_cast<size_t>(longitudAncha));
                MultiByteToWideChar(CP_UTF8, 0, nombreUtf8.c_str(), -1, buferAncho.data(), longitudAncha);
                rutaRelativaW = buferAncho.data();
            }

            if (!rutaRelativaW.empty()) {
                bool esDir = false;
                while (!rutaRelativaW.empty() && rutaRelativaW.back() == L'\\') {
                    esDir = true;
                    rutaRelativaW.pop_back();
                }

                if (esDir) {
                    if (!rutaRelativaW.empty() && !mapaCarpetasDetectadas[rutaRelativaW]) {
                        mapaCarpetasDetectadas[rutaRelativaW] = true;
                        ElementoContenidoComprimido elemDir;
                        elemDir.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                        elemDir.rutaRelativa = rutaRelativaW;
                        elemDir.esDirectorio = true;
                        elemDir.tamanoBytes = 0;
                        elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                        mapaElementos[rutaRelativaW] = elemDir;
                    }
                } else if (!rutaRelativaW.empty()) {
                    size_t posicionBarra = 0;
                    while ((posicionBarra = rutaRelativaW.find(L'\\', posicionBarra)) != std::wstring::npos) {
                        std::wstring subDirectorio = rutaRelativaW.substr(0, posicionBarra);
                        if (!subDirectorio.empty() && !mapaCarpetasDetectadas[subDirectorio]) {
                            mapaCarpetasDetectadas[subDirectorio] = true;
                            ElementoContenidoComprimido elemDir;
                            elemDir.nombre = PathFindFileNameW(subDirectorio.c_str());
                            elemDir.rutaRelativa = subDirectorio;
                            elemDir.esDirectorio = true;
                            elemDir.tamanoBytes = 0;
                            elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                            mapaElementos[subDirectorio] = elemDir;
                        }
                        posicionBarra++;
                    }

                    ElementoContenidoComprimido elem;
                    elem.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                    elem.rutaRelativa = rutaRelativaW;
                    elem.esDirectorio = false;
                    elem.tamanoBytes = cabeceraLocal.tamanoDescomprimido;
                    elem.categoria = clasificarCategoriaElementoComprimido(rutaRelativaW, false);
                    mapaElementos[rutaRelativaW] = elem;
                }
            }

            if (cabeceraLocal.tamanoComprimido > 0) {
                SetFilePointer(
                    manejadorArchivo,
                    static_cast<LONG>(cabeceraLocal.tamanoComprimido), nullptr,
                    FILE_CURRENT);
            }
        } else {
            break;
        }
    }
    CloseHandle(manejadorArchivo);

    for (const auto& [claveRuta, elemento] : mapaElementos) {
        salidaElementos.push_back(elemento);
    }
    return !salidaElementos.empty();
}

inline bool inspeccionarArchivoRarCompleto(const std::wstring& rutaArchivo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return false;

    BYTE firma[8] = {0};
    DWORD bytesLeidos = 0;
    if (!ReadFile(manejadorArchivo, firma, 7, &bytesLeidos, nullptr) || bytesLeidos < 7) {
        CloseHandle(manejadorArchivo);
        return false;
    }

    bool esRar4 = (firma[0] == 0x52 && firma[1] == 0x61 && firma[2] == 0x72 && firma[3] == 0x21 && firma[4] == 0x1a && firma[5] == 0x07 && firma[6] == 0x00);
    bool esRar5 = (firma[0] == 0x52 && firma[1] == 0x61 && firma[2] == 0x72 && firma[3] == 0x21 && firma[4] == 0x1a && firma[5] == 0x07 && firma[6] == 0x01);

    if (!esRar4 && !esRar5) {
        CloseHandle(manejadorArchivo);
        return false;
    }

    std::map<std::wstring, ElementoContenidoComprimido> mapaElementos;
    std::map<std::wstring, bool> mapaCarpetasDetectadas;

    if (esRar4) {
        while (true) {
            BYTE cabeceraBloque[7];
            if (!ReadFile(manejadorArchivo, cabeceraBloque, 7, &bytesLeidos, nullptr) || bytesLeidos < 7) {
                break;
            }
            BYTE tipoBloque = cabeceraBloque[2];
            uint16_t banderas = static_cast<uint16_t>(cabeceraBloque[3] | (cabeceraBloque[4] << 8));
            uint16_t tamanoCabecera = static_cast<uint16_t>(cabeceraBloque[5] | (cabeceraBloque[6] << 8));

            if (tamanoCabecera < 7) break;

            uint32_t tamanoCuerpo = tamanoCabecera - 7;
            std::vector<BYTE> cuerpoCabecera(tamanoCuerpo);
            if (tamanoCuerpo > 0) {
                if (!ReadFile(manejadorArchivo, cuerpoCabecera.data(), tamanoCuerpo, &bytesLeidos, nullptr) || bytesLeidos < tamanoCuerpo) {
                    break;
                }
            }

            uint32_t tamanoEmpaquetado = 0;
            if (tipoBloque == 0x74) {
                if (cuerpoCabecera.size() >= 21) {
                    tamanoEmpaquetado = static_cast<uint32_t>(cuerpoCabecera[0] | (cuerpoCabecera[1] << 8) | (cuerpoCabecera[2] << 16) | (cuerpoCabecera[3] << 24));
                    uint32_t tamanoDesemp = static_cast<uint32_t>(cuerpoCabecera[4] | (cuerpoCabecera[5] << 8) | (cuerpoCabecera[6] << 16) | (cuerpoCabecera[7] << 24));
                    uint16_t tamanoNombre = static_cast<uint16_t>(cuerpoCabecera[19] | (cuerpoCabecera[20] << 8));

                    if ((banderas & 0x0004) != 0) {
                        estadoComprimidos.protegidoConContrasena = true;
                    }
                    bool esDirectorio = (banderas & 0x00E0) == 0x00E0 || (banderas & 0x0020) != 0;

                    size_t offsetNombre = 25;
                    if ((banderas & 0x0100) != 0 && cuerpoCabecera.size() >= 33) {
                        offsetNombre += 8;
                    }
                    if (cuerpoCabecera.size() >= offsetNombre + tamanoNombre && tamanoNombre > 0) {
                        std::string nombreStr(reinterpret_cast<char*>(cuerpoCabecera.data() + offsetNombre), tamanoNombre);
                        for (char& caracter : nombreStr) {
                            if (caracter == '/') caracter = '\\';
                        }
                        int longitudAncha = MultiByteToWideChar(CP_UTF8, 0, nombreStr.c_str(), -1, nullptr, 0);
                        if (longitudAncha <= 0) {
                            longitudAncha = MultiByteToWideChar(CP_ACP, 0, nombreStr.c_str(), -1, nullptr, 0);
                        }
                        if (longitudAncha > 0) {
                            std::vector<wchar_t> buferAncho(static_cast<size_t>(longitudAncha));
                            MultiByteToWideChar(CP_UTF8, 0, nombreStr.c_str(), -1, buferAncho.data(), longitudAncha);
                            std::wstring rutaRelativaW = buferAncho.data();

                            while (!rutaRelativaW.empty() && rutaRelativaW.back() == L'\\') {
                                esDirectorio = true;
                                rutaRelativaW.pop_back();
                            }

                            if (!rutaRelativaW.empty()) {
                                if (esDirectorio) {
                                    if (!mapaCarpetasDetectadas[rutaRelativaW]) {
                                        mapaCarpetasDetectadas[rutaRelativaW] = true;
                                        ElementoContenidoComprimido elemDir;
                                        elemDir.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                                        elemDir.rutaRelativa = rutaRelativaW;
                                        elemDir.esDirectorio = true;
                                        elemDir.tamanoBytes = 0;
                                        elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                                        mapaElementos[rutaRelativaW] = elemDir;
                                    }
                                } else {
                                    size_t posicionBarra = 0;
                                    while ((posicionBarra = rutaRelativaW.find(L'\\', posicionBarra)) != std::wstring::npos) {
                                        std::wstring subDirectorio = rutaRelativaW.substr(0, posicionBarra);
                                        if (!subDirectorio.empty() && !mapaCarpetasDetectadas[subDirectorio]) {
                                            mapaCarpetasDetectadas[subDirectorio] = true;
                                            ElementoContenidoComprimido elemDir;
                                            elemDir.nombre = PathFindFileNameW(subDirectorio.c_str());
                                            elemDir.rutaRelativa = subDirectorio;
                                            elemDir.esDirectorio = true;
                                            elemDir.tamanoBytes = 0;
                                            elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                                            mapaElementos[subDirectorio] = elemDir;
                                        }
                                        posicionBarra++;
                                    }
                                    ElementoContenidoComprimido elem;
                                    elem.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                                    elem.rutaRelativa = rutaRelativaW;
                                    elem.esDirectorio = false;
                                    elem.tamanoBytes = tamanoDesemp;
                                    elem.categoria = clasificarCategoriaElementoComprimido(rutaRelativaW, false);
                                    mapaElementos[rutaRelativaW] = elem;
                                }
                            }
                        }
                    }
                }
            }

            if ((banderas & 0x8000) != 0 && tamanoEmpaquetado > 0) {
                SetFilePointer(manejadorArchivo, static_cast<LONG>(tamanoEmpaquetado), nullptr, FILE_CURRENT);
            }
        }
    }
    CloseHandle(manejadorArchivo);

    for (const auto& [claveRuta, elemento] : mapaElementos) {
        salidaElementos.push_back(elemento);
    }
    return !salidaElementos.empty();
}

inline bool inspeccionarArchivoTarCompleto(const std::wstring& rutaArchivo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    HANDLE manejadorArchivo = CreateFileW(
        rutaArchivo.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (manejadorArchivo == INVALID_HANDLE_VALUE) return false;

    BYTE bloque[512];
    DWORD bytesLeidos = 0;
    std::map<std::wstring, ElementoContenidoComprimido> mapaElementos;
    std::map<std::wstring, bool> mapaCarpetasDetectadas;

    while (ReadFile(manejadorArchivo, bloque, 512, &bytesLeidos, nullptr) && bytesLeidos == 512) {
        if (bloque[0] == 0) break;

        char nombreEntrada[101] = {0};
        memcpy(nombreEntrada, bloque, 100);
        char prefijoEntrada[156] = {0};
        memcpy(prefijoEntrada, bloque + 345, 155);

        std::string rutaCompleta = (prefijoEntrada[0] != '\0')
            ? (std::string(prefijoEntrada) + "/" + std::string(nombreEntrada))
            : std::string(nombreEntrada);

        for (char& caracter : rutaCompleta) {
            if (caracter == '/') caracter = '\\';
        }

        char tamanoOctal[13] = {0};
        memcpy(tamanoOctal, bloque + 124, 12);
        uintmax_t tamanoArchivo = strtoull(tamanoOctal, nullptr, 8);

        char tipoEntrada = static_cast<char>(bloque[156]);
        bool esDirectorio = (tipoEntrada == '5') || (!rutaCompleta.empty() && rutaCompleta.back() == '\\');

        int longitudAncha = MultiByteToWideChar(CP_UTF8, 0, rutaCompleta.c_str(), -1, nullptr, 0);
        if (longitudAncha <= 0) {
            longitudAncha = MultiByteToWideChar(CP_ACP, 0, rutaCompleta.c_str(), -1, nullptr, 0);
        }
        if (longitudAncha > 0) {
            std::vector<wchar_t> buferAncho(static_cast<size_t>(longitudAncha));
            MultiByteToWideChar(CP_UTF8, 0, rutaCompleta.c_str(), -1, buferAncho.data(), longitudAncha);
            std::wstring rutaRelativaW = buferAncho.data();

            while (!rutaRelativaW.empty() && rutaRelativaW.back() == L'\\') {
                esDirectorio = true;
                rutaRelativaW.pop_back();
            }

            if (!rutaRelativaW.empty()) {
                if (esDirectorio) {
                    if (!mapaCarpetasDetectadas[rutaRelativaW]) {
                        mapaCarpetasDetectadas[rutaRelativaW] = true;
                        ElementoContenidoComprimido elemDir;
                        elemDir.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                        elemDir.rutaRelativa = rutaRelativaW;
                        elemDir.esDirectorio = true;
                        elemDir.tamanoBytes = 0;
                        elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                        mapaElementos[rutaRelativaW] = elemDir;
                    }
                } else {
                    size_t posicionBarra = 0;
                    while ((posicionBarra = rutaRelativaW.find(L'\\', posicionBarra)) != std::wstring::npos) {
                        std::wstring subDirectorio = rutaRelativaW.substr(0, posicionBarra);
                        if (!subDirectorio.empty() && !mapaCarpetasDetectadas[subDirectorio]) {
                            mapaCarpetasDetectadas[subDirectorio] = true;
                            ElementoContenidoComprimido elemDir;
                            elemDir.nombre = PathFindFileNameW(subDirectorio.c_str());
                            elemDir.rutaRelativa = subDirectorio;
                            elemDir.esDirectorio = true;
                            elemDir.tamanoBytes = 0;
                            elemDir.categoria = CategoriaElementoComprimido::Carpetas;
                            mapaElementos[subDirectorio] = elemDir;
                        }
                        posicionBarra++;
                    }
                    ElementoContenidoComprimido elem;
                    elem.nombre = PathFindFileNameW(rutaRelativaW.c_str());
                    elem.rutaRelativa = rutaRelativaW;
                    elem.esDirectorio = false;
                    elem.tamanoBytes = tamanoArchivo;
                    elem.categoria = clasificarCategoriaElementoComprimido(rutaRelativaW, false);
                    mapaElementos[rutaRelativaW] = elem;
                }
            }
        }

        uintmax_t bloquesSaltar = (tamanoArchivo + 511) / 512;
        if (bloquesSaltar > 0) {
            LARGE_INTEGER salto{};
            salto.QuadPart = static_cast<LONGLONG>(bloquesSaltar * 512);
            SetFilePointerEx(manejadorArchivo, salto, nullptr, FILE_CURRENT);
        }
    }
    CloseHandle(manejadorArchivo);

    for (const auto& [claveRuta, elemento] : mapaElementos) {
        salidaElementos.push_back(elemento);
    }
    return !salidaElementos.empty();
}

inline void enumerarRecursivoFolderItems(IDispatch* carpetaDispatch, const std::wstring& rutaPrefijo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    if (!carpetaDispatch) return;

    DISPPARAMS paramsVacio{};
    VARIANT resultadoItems{};
    VariantInit(&resultadoItems);
    DISPID dispidItems = 0;
    OLECHAR* nombreItems = const_cast<OLECHAR*>(L"Items");
    if (SUCCEEDED(carpetaDispatch->GetIDsOfNames(IID_NULL, &nombreItems, 1, LOCALE_USER_DEFAULT, &dispidItems)) &&
        SUCCEEDED(carpetaDispatch->Invoke(dispidItems, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsVacio, &resultadoItems, nullptr, nullptr)) &&
        resultadoItems.vt == VT_DISPATCH && resultadoItems.pdispVal) {

        IDispatch* itemsDispatch = resultadoItems.pdispVal;
        VARIANT resultadoCount{};
        VariantInit(&resultadoCount);
        DISPID dispidCount = 0;
        OLECHAR* nombreCount = const_cast<OLECHAR*>(L"Count");
        if (SUCCEEDED(itemsDispatch->GetIDsOfNames(IID_NULL, &nombreCount, 1, LOCALE_USER_DEFAULT, &dispidCount)) &&
            SUCCEEDED(itemsDispatch->Invoke(dispidCount, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resultadoCount, nullptr, nullptr)) &&
            resultadoCount.vt == VT_I4) {

            long totalItems = resultadoCount.lVal;
            DISPID dispidItem = 0;
            OLECHAR* nombreItem = const_cast<OLECHAR*>(L"Item");
            itemsDispatch->GetIDsOfNames(IID_NULL, &nombreItem, 1, LOCALE_USER_DEFAULT, &dispidItem);

            DISPID dispidIsFolder = 0;
            OLECHAR* nombreIsFolder = const_cast<OLECHAR*>(L"IsFolder");
            DISPID dispidName = 0;
            OLECHAR* nombreName = const_cast<OLECHAR*>(L"Name");
            DISPID dispidSize = 0;
            OLECHAR* nombreSize = const_cast<OLECHAR*>(L"Size");
            DISPID dispidGetFolder = 0;
            OLECHAR* nombreGetFolder = const_cast<OLECHAR*>(L"GetFolder");

            for (long posicionItem = 0; posicionItem < totalItems; ++posicionItem) {
                VARIANT argPos{};
                argPos.vt = VT_I4;
                argPos.lVal = posicionItem;
                DISPPARAMS paramsItem{};
                paramsItem.cArgs = 1;
                paramsItem.rgvarg = &argPos;
                VARIANT resultadoItem{};
                VariantInit(&resultadoItem);

                if (SUCCEEDED(itemsDispatch->Invoke(dispidItem, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsItem, &resultadoItem, nullptr, nullptr)) &&
                    resultadoItem.vt == VT_DISPATCH && resultadoItem.pdispVal) {

                    IDispatch* itemDispatch = resultadoItem.pdispVal;

                    if (dispidIsFolder == 0) itemDispatch->GetIDsOfNames(IID_NULL, &nombreIsFolder, 1, LOCALE_USER_DEFAULT, &dispidIsFolder);
                    if (dispidName == 0) itemDispatch->GetIDsOfNames(IID_NULL, &nombreName, 1, LOCALE_USER_DEFAULT, &dispidName);
                    if (dispidSize == 0) itemDispatch->GetIDsOfNames(IID_NULL, &nombreSize, 1, LOCALE_USER_DEFAULT, &dispidSize);
                    if (dispidGetFolder == 0) itemDispatch->GetIDsOfNames(IID_NULL, &nombreGetFolder, 1, LOCALE_USER_DEFAULT, &dispidGetFolder);

                    VARIANT resIsFolder{};
                    VariantInit(&resIsFolder);
                    itemDispatch->Invoke(dispidIsFolder, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resIsFolder, nullptr, nullptr);
                    bool esFolder = (resIsFolder.vt == VT_BOOL && resIsFolder.boolVal == VARIANT_TRUE);

                    VARIANT resName{};
                    VariantInit(&resName);
                    itemDispatch->Invoke(dispidName, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resName, nullptr, nullptr);
                    std::wstring nombreElemento = (resName.vt == VT_BSTR && resName.bstrVal) ? resName.bstrVal : L"";
                    VariantClear(&resName);

                    VARIANT resSize{};
                    VariantInit(&resSize);
                    itemDispatch->Invoke(dispidSize, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resSize, nullptr, nullptr);
                    uintmax_t tamanoElem = (resSize.vt == VT_I4) ? static_cast<uintmax_t>(resSize.lVal) : ((resSize.vt == VT_UI4) ? resSize.ulVal : 0);
                    VariantClear(&resSize);

                    if (!nombreElemento.empty()) {
                        std::wstring rutaRelativa = rutaPrefijo.empty() ? nombreElemento : (rutaPrefijo + L"\\" + nombreElemento);
                        ElementoContenidoComprimido elem;
                        elem.nombre = nombreElemento;
                        elem.rutaRelativa = rutaRelativa;
                        elem.esDirectorio = esFolder;
                        elem.tamanoBytes = tamanoElem;
                        elem.categoria = clasificarCategoriaElementoComprimido(rutaRelativa, esFolder);
                        salidaElementos.push_back(elem);

                        if (esFolder) {
                            VARIANT resSubFolder{};
                            VariantInit(&resSubFolder);
                            if (SUCCEEDED(itemDispatch->Invoke(dispidGetFolder, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsVacio, &resSubFolder, nullptr, nullptr)) &&
                                resSubFolder.vt == VT_DISPATCH && resSubFolder.pdispVal) {
                                enumerarRecursivoFolderItems(resSubFolder.pdispVal, rutaRelativa, salidaElementos);
                                resSubFolder.pdispVal->Release();
                            }
                        }
                    }
                    itemDispatch->Release();
                }
            }
        }
        itemsDispatch->Release();
    }
}

inline bool inspeccionarArchivoViaShellDispatch(const std::wstring& rutaArchivo, std::vector<ElementoContenidoComprimido>& salidaElementos) {
    CLSID clsidShell;
    if (FAILED(CLSIDFromProgID(L"Shell.Application", &clsidShell))) return false;

    IDispatch* shellDispatch = nullptr;
    if (FAILED(CoCreateInstance(clsidShell, nullptr, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IDispatch, reinterpret_cast<void**>(&shellDispatch))) || !shellDispatch) {
        return false;
    }

    DISPID dispidNameSpace = 0;
    OLECHAR* nombreNameSpace = const_cast<OLECHAR*>(L"NameSpace");
    if (FAILED(shellDispatch->GetIDsOfNames(IID_NULL, &nombreNameSpace, 1, LOCALE_USER_DEFAULT, &dispidNameSpace))) {
        shellDispatch->Release();
        return false;
    }

    VARIANT argRuta{};
    argRuta.vt = VT_BSTR;
    argRuta.bstrVal = SysAllocString(rutaArchivo.c_str());
    DISPPARAMS paramsNameSpace{};
    paramsNameSpace.cArgs = 1;
    paramsNameSpace.rgvarg = &argRuta;

    VARIANT resFolder{};
    VariantInit(&resFolder);
    HRESULT hr = shellDispatch->Invoke(dispidNameSpace, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsNameSpace, &resFolder, nullptr, nullptr);
    SysFreeString(argRuta.bstrVal);
    shellDispatch->Release();

    if (SUCCEEDED(hr) && resFolder.vt == VT_DISPATCH && resFolder.pdispVal) {
        enumerarRecursivoFolderItems(resFolder.pdispVal, L"", salidaElementos);
        resFolder.pdispVal->Release();
        return !salidaElementos.empty();
    }
    return false;
}

inline void inspeccionarEstructuraArchivoZip(const std::wstring& rutaArchivo) {
    estadoComprimidos.seccionesContenido.clear();
    estadoComprimidos.todosLosElementos.clear();
    estadoComprimidos.elementosArbol.clear();
    estadoComprimidos.desplazamientoScrollY = 0;
    estadoComprimidos.protegidoConContrasena = false;

    const wchar_t* extension = PathFindExtensionW(rutaArchivo.c_str());

    // El escaneo multinivel solo aplica para archivos comprimidos comunes.
    // Los archivos de comics (.cbz, .cbr, .cb7, .cbt, .cba) quedan excluidos.
    if (esExtensionComic(extension)) {
        return;
    }

    std::vector<ElementoContenidoComprimido> listaElementos;

    bool parseado = false;
    if (lstrcmpiW(extension, L".zip") == 0 || lstrcmpiW(extension, L".jar") == 0 ||
        lstrcmpiW(extension, L".apk") == 0) {
        parseado = inspeccionarArchivoZipCompleto(rutaArchivo, listaElementos);
    } else if (lstrcmpiW(extension, L".rar") == 0) {
        parseado = inspeccionarArchivoRarCompleto(rutaArchivo, listaElementos);
    } else if (lstrcmpiW(extension, L".tar") == 0) {
        parseado = inspeccionarArchivoTarCompleto(rutaArchivo, listaElementos);
    }

    if (!parseado || listaElementos.empty()) {
        inspeccionarArchivoViaShellDispatch(rutaArchivo, listaElementos);
    }

    if (listaElementos.empty()) {
        inspeccionarArchivoZipCompleto(rutaArchivo, listaElementos);
    }

    agruparContenidosEnSecciones(listaElementos);

    for (const auto& elementoActual : estadoComprimidos.todosLosElementos) {
        ElementoArbolComprimido itemArbol;
        itemArbol.nombre = elementoActual.nombre;
        itemArbol.rutaCompleta = elementoActual.rutaRelativa;
        itemArbol.esDirectorio = elementoActual.esDirectorio;
        itemArbol.nivelJerarquia = 0;
        itemArbol.tamanoBytes = elementoActual.tamanoBytes;
        estadoComprimidos.elementosArbol.push_back(itemArbol);
    }
}

inline bool cargarComprimidoPorIndice(size_t indiceComprimido) {
    if (indiceComprimido >= estadoComprimidos.archivosSeleccionados.size()) return false;
    estadoComprimidos.indiceArchivoActual = indiceComprimido;
    const std::wstring& ruta = estadoComprimidos.archivosSeleccionados[indiceComprimido];

    const wchar_t* extension = PathFindExtensionW(ruta.c_str());
    if (esExtensionComic(extension)) {
        // Los comics quedan excluidos del escaneo multinivel y visor de árbol
        return true;
    }

    WIN32_FILE_ATTRIBUTE_DATA datosAttr;
    bool tieneAttr = GetFileAttributesExW(ruta.c_str(), GetFileExInfoStandard, &datosAttr) != FALSE;

    if (tieneAttr && estadoComprimidos.rutaComprimidoArbolCargado == ruta &&
        !estadoComprimidos.elementosArbol.empty() &&
        memcmp(&datosAttr.ftLastWriteTime, &estadoComprimidos.ftUltimaModificacionArbol, sizeof(FILETIME)) == 0) {
        return true;
    }

    estadoComprimidos.rutaComprimidoArbolCargado = ruta;
    if (tieneAttr) {
        estadoComprimidos.ftUltimaModificacionArbol = datosAttr.ftLastWriteTime;
    }
    inspeccionarEstructuraArchivoZip(ruta);
    return true;
}

inline bool mostrarComprimidoAnterior() {
    if (estadoComprimidos.archivosSeleccionados.size() <= 1) return false;
    if (estadoComprimidos.indiceArchivoActual == 0) {
        estadoComprimidos.indiceArchivoActual = estadoComprimidos.archivosSeleccionados.size() - 1;
    } else {
        estadoComprimidos.indiceArchivoActual--;
    }
    return cargarComprimidoPorIndice(estadoComprimidos.indiceArchivoActual);
}

inline bool mostrarComprimidoSiguiente() {
    if (estadoComprimidos.archivosSeleccionados.size() <= 1) return false;
    estadoComprimidos.indiceArchivoActual =
        (estadoComprimidos.indiceArchivoActual + 1) % estadoComprimidos.archivosSeleccionados.size();
    return cargarComprimidoPorIndice(estadoComprimidos.indiceArchivoActual);
}

inline const ElementoArbolComprimido* obtenerElementoArbolComprimidoEnPunto(POINT puntoRaton) {
    if (estadoComprimidos.esModoComic || estadoComprimidos.elementosArbol.empty()) return nullptr;
    if (!PtInRect(&estadoComprimidos.areaCajaArbol, puntoRaton)) return nullptr;

    int yInicioArbol = estadoComprimidos.areaCajaArbol.top + 8 - estadoComprimidos.desplazamientoScrollY;
    int indiceElementoCalculado = (puntoRaton.y - yInicioArbol) / 22;
    if (indiceElementoCalculado >= 0 &&
        static_cast<size_t>(indiceElementoCalculado) < estadoComprimidos.elementosArbol.size()) {
        return &estadoComprimidos.elementosArbol[static_cast<size_t>(indiceElementoCalculado)];
    }
    return nullptr;
}

inline void desplazarScrollComprimido(int incrementoDelta) {
    int maximoScroll = std::max(0, estadoComprimidos.altoTotalArbol - 100);
    estadoComprimidos.desplazamientoScrollY = std::max(
        0, std::min(estadoComprimidos.desplazamientoScrollY + incrementoDelta, maximoScroll));
}

inline void seleccionarYAgregarPrograma(HWND ventanaPadre) {
    wchar_t buferRutaArchivo[MAX_PATH] = L"";
    OPENFILENAMEW estructuraOfn{};
    estructuraOfn.lStructSize = sizeof(estructuraOfn);
    estructuraOfn.hwndOwner = ventanaPadre;
    estructuraOfn.lpstrFilter =
        L"Archivos Ejecutables (*.exe)\0*.exe\0Todos los archivos (*.*)\0*.*\0";
    estructuraOfn.lpstrFile = buferRutaArchivo;
    estructuraOfn.nMaxFile = MAX_PATH;
    estructuraOfn.Flags =
        OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    estructuraOfn.lpstrTitle =
        estadoComprimidos.tieneComicsSeleccionados
            ? L"Seleccionar programa para abrir cómics (.cbz, .cbr)"
            : L"Seleccionar programa para comprimidos comunes (.zip, .rar, .7z, .tar)";

    if (GetOpenFileNameW(&estructuraOfn)) {
        std::wstring rutaSeleccionada(buferRutaArchivo);
        if (!rutaSeleccionada.empty()) {
            std::wstring nombreEjecutable =
                PathFindFileNameW(rutaSeleccionada.c_str());

            std::wstring mensajePregunta =
                estadoComprimidos.tieneComicsSeleccionados
                    ? (L"¿Desea recordar '" + nombreEjecutable +
                       L"' en la lista de opciones para cómics?")
                    : (L"¿Desea recordar '" + nombreEjecutable +
                       L"' en la lista de opciones para comprimidos comunes?");

            int respuesta = MessageBoxW(
                ventanaPadre, mensajePregunta.c_str(),
                L"Recordar programa", MB_YESNO | MB_ICONQUESTION);

            std::vector<std::wstring>& listaActiva =
                obtenerListaProgramasActiva();

            if (respuesta == IDYES) {
                auto iteradorPrograma = std::find(
                    listaActiva.begin(), listaActiva.end(), rutaSeleccionada);
                if (iteradorPrograma == listaActiva.end()) {
                    listaActiva.push_back(rutaSeleccionada);
                    estadoComprimidos.indiceProgramaSeleccionado =
                        static_cast<int>(listaActiva.size() - 1);
                } else {
                    estadoComprimidos.indiceProgramaSeleccionado =
                        static_cast<int>(std::distance(
                            listaActiva.begin(), iteradorPrograma));
                }
                estadoComprimidos.programaTemporal.clear();
            } else {
                estadoComprimidos.programaTemporal = rutaSeleccionada;
            }
        }
    }
}

inline void mostrarMenuDesplegableProgramasComprimidos(HWND ventana, const RECT& botonRect) {
    const std::vector<std::wstring>& listaProgramas = obtenerListaProgramasActivaConst();
    if (listaProgramas.empty() && estadoComprimidos.programaTemporal.empty()) {
        seleccionarYAgregarPrograma(ventana);
        InvalidateRect(ventana, nullptr, FALSE);
        return;
    }

    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    for (size_t posicionPrograma = 0; posicionPrograma < listaProgramas.size(); ++posicionPrograma) {
        UINT banderasMenu = MF_STRING;
        if (static_cast<int>(posicionPrograma) == estadoComprimidos.indiceProgramaSeleccionado) {
            banderasMenu |= MF_CHECKED;
        }
        std::wstring nombreProg = PathFindFileNameW(listaProgramas[posicionPrograma].c_str());
        AppendMenuW(menu, banderasMenu, static_cast<UINT_PTR>(2000 + posicionPrograma), nombreProg.c_str());
    }

    if (!listaProgramas.empty()) {
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    AppendMenuW(menu, MF_STRING, 2999, L"+ Añadir programa...");

    POINT puntoBoton{botonRect.left, botonRect.top};
    ClientToScreen(ventana, &puntoBoton);

    SetForegroundWindow(ventana);
    UINT comandoSeleccionado = static_cast<UINT>(TrackPopupMenuEx(
        menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_VERTICAL,
        puntoBoton.x, puntoBoton.y, ventana, nullptr));
    DestroyMenu(menu);

    if (comandoSeleccionado >= 2000 && comandoSeleccionado < 2000 + listaProgramas.size()) {
        estadoComprimidos.indiceProgramaSeleccionado = static_cast<int>(comandoSeleccionado - 2000);
        estadoComprimidos.programaTemporal.clear();
        InvalidateRect(ventana, nullptr, FALSE);
    } else if (comandoSeleccionado == 2999) {
        seleccionarYAgregarPrograma(ventana);
        InvalidateRect(ventana, nullptr, FALSE);
    }
}

inline void mostrarMenuDesplegableFormatosComprimidos(HWND ventana, const RECT& botonRect) {
    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    for (int posicionFormato = 0; posicionFormato < TOTAL_FORMATOS_DESTINO; ++posicionFormato) {
        UINT banderasMenu = MF_STRING;
        if (posicionFormato == estadoComprimidos.indiceFormatoDestino) {
            banderasMenu |= MF_CHECKED;
        }
        AppendMenuW(menu, banderasMenu, static_cast<UINT_PTR>(1000 + posicionFormato), FORMATOS_DESTINO[posicionFormato]);
    }

    POINT puntoBoton{botonRect.left, botonRect.top};
    ClientToScreen(ventana, &puntoBoton);

    SetForegroundWindow(ventana);
    UINT comandoSeleccionado = static_cast<UINT>(TrackPopupMenuEx(
        menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_VERTICAL,
        puntoBoton.x, puntoBoton.y, ventana, nullptr));
    DestroyMenu(menu);

    if (comandoSeleccionado >= 1000 && comandoSeleccionado < 1000 + TOTAL_FORMATOS_DESTINO) {
        estadoComprimidos.indiceFormatoDestino = static_cast<int>(comandoSeleccionado - 1000);
        InvalidateRect(ventana, nullptr, FALSE);
    }
}

inline std::wstring resolverProgramaParaComprimidos() {
    if (!estadoComprimidos.programaTemporal.empty()) {
        return estadoComprimidos.programaTemporal;
    }
    const std::vector<std::wstring>& listaActiva =
        obtenerListaProgramasActivaConst();
    if (!listaActiva.empty() &&
        estadoComprimidos.indiceProgramaSeleccionado >= 0 &&
        estadoComprimidos.indiceProgramaSeleccionado <
            static_cast<int>(listaActiva.size())) {
        return listaActiva[static_cast<size_t>(
            estadoComprimidos.indiceProgramaSeleccionado)];
    }
    if (estadoComprimidos.tieneComicsSeleccionados) {
        if (!configuracion.programaComicPorDefecto.empty()) {
            return configuracion.programaComicPorDefecto;
        }
        if (estadoComprimidos.programasComicsRecordados.size() == 1) {
            return estadoComprimidos.programasComicsRecordados.front();
        }
    } else {
        if (!configuracion.programaComprimidosPorDefecto.empty()) {
            return configuracion.programaComprimidosPorDefecto;
        }
        if (estadoComprimidos.programasComunesRecordados.size() == 1) {
            return estadoComprimidos.programasComunesRecordados.front();
        }
    }
    if (!listaActiva.empty()) {
        return listaActiva.front();
    }
    return {};
}

inline void ejecutarAbrirArchivosConPrograma(HWND ventanaPadre) {
    std::wstring rutaPrograma = resolverProgramaParaComprimidos();

    if (rutaPrograma.empty()) {
        seleccionarYAgregarPrograma(ventanaPadre);
        rutaPrograma = resolverProgramaParaComprimidos();
        if (rutaPrograma.empty()) return;
    }

    const auto& listaArchivos = estadoComprimidos.archivosSeleccionados;
    for (const auto& ruta : listaArchivos) {
        ShellExecuteW(
            nullptr, L"open", rutaPrograma.c_str(),
            (L"\"" + ruta + L"\"").c_str(), nullptr,
            SW_SHOWNORMAL);
    }
}

inline bool cargarArchivosComprimidos(
    const std::vector<std::wstring>& listaRutas) {
    liberarRecursosComprimido();
    if (listaRutas.empty()) return false;

    estadoComprimidos.archivosSeleccionados = listaRutas;
    estadoComprimidos.totalSeleccionados = listaRutas.size();
    estadoComprimidos.indiceArchivoActual = 0;

    estadoComprimidos.tieneComicsSeleccionados = false;
    for (const std::wstring& ruta : listaRutas) {
        const wchar_t* extension = PathFindExtensionW(ruta.c_str());
        if (esExtensionComic(extension)) {
            estadoComprimidos.tieneComicsSeleccionados = true;
            break;
        }
    }
    estadoComprimidos.esModoComic = estadoComprimidos.tieneComicsSeleccionados;

    if (estadoComprimidos.esModoComic) {
        int limiteGrid = configuracion.limiteCuadriculaComprimidos.load();
        if (limiteGrid != 4 && limiteGrid != 6 && limiteGrid != 8 &&
            limiteGrid != 10) {
            limiteGrid = 4;
        }
        size_t limiteCarga = std::min<size_t>(
            static_cast<size_t>(limiteGrid), listaRutas.size());

        for (size_t posicion = 0; posicion < limiteCarga; ++posicion) {
            const std::wstring& rutaActual = listaRutas[posicion];
            ElementoComprimido elemento;
            elemento.rutaArchivo = rutaActual;
            elemento.nombreArchivo = PathFindFileNameW(rutaActual.c_str());
            const wchar_t* extension = PathFindExtensionW(rutaActual.c_str());
            elemento.extensionFormato = extension ? extension : L"";
            elemento.esComic = esExtensionComic(extension);

            WIN32_FILE_ATTRIBUTE_DATA atributos{};
            if (GetFileAttributesExW(rutaActual.c_str(), GetFileExInfoStandard,
                                     &atributos)) {
                ULARGE_INTEGER tamanoGrande{};
                tamanoGrande.LowPart = atributos.nFileSizeLow;
                tamanoGrande.HighPart = atributos.nFileSizeHigh;
                elemento.textoTamano =
                    formatearTamanoArchivo(tamanoGrande.QuadPart);
            }

            elemento.portadaCargada = obtenerPortadaDeComprimido(rutaActual);
            estadoComprimidos.elementosSeleccionados.push_back(elemento);
        }
    } else {
        cargarComprimidoPorIndice(0);
    }

    return true;
}

inline void ejecutarConversionArchivos() {
    if (estadoComprimidos.totalSeleccionados == 0 ||
        estadoComprimidos.conversionEnCurso) {
        return;
    }

    estadoComprimidos.conversionEnCurso = true;
    const wchar_t* extensionDestino =
        EXTENSIONES_DESTINO[estadoComprimidos.indiceFormatoDestino];

    for (const ElementoComprimido& elemento :
         estadoComprimidos.elementosSeleccionados) {
        wchar_t rutaDestino[MAX_PATH];
        wcsncpy_s(rutaDestino, elemento.rutaArchivo.c_str(), _TRUNCATE);
        PathRenameExtensionW(rutaDestino, extensionDestino);

        if (lstrcmpiW(elemento.rutaArchivo.c_str(), rutaDestino) == 0) {
            continue;
        }

        CopyFileW(elemento.rutaArchivo.c_str(), rutaDestino, FALSE);
    }

    estadoComprimidos.conversionEnCurso = false;
}


/**** ended inlining comprimidos/controlador_comprimidos.hpp ****/
/**** start inlining carpetas/controlador_carpetas.hpp ****/

#include <shlobj.h>

struct ElementoArbolVistaCarpetas {
    std::wstring nombre;
    std::wstring rutaCompleta;
    bool esDirectorio{false};
    uintmax_t tamanoBytes{0};
    int nivelJerarquia{0};
    int cantidadSubElementos{0};
};

struct SubcarpetaPesada {
    std::wstring nombre;
    uintmax_t tamanoBytes{0};
    double porcentaje{0.0};
};

struct ResumenVistaVacia {
    std::wstring rutaCarpeta;
    std::wstring nombreCarpeta;
    uintmax_t tamanoTotalBytes{0};
    int totalSubcarpetas{0};
    int totalArchivos{0};
    int cantidadTiposArchivos{0};
    std::wstring textoExtensionesFantasma;
    std::wstring fechaUltimoAcceso;
    FILETIME ftUltimaModificacion{};
    std::vector<SubcarpetaPesada> top4CarpetasMasPesadas;
    std::vector<std::wstring> carpetasVacias;
    RECT botonEliminarCarpetasVacias{};
    bool hoverBotonEliminarCarpetasVacias{false};
    std::wstring mensajeResultadoEliminacion;
    bool esUbicacionSistema{false};
    std::wstring nombreUbicacionSistema;
    std::wstring mensajeUbicacionSistema;
    bool calculado{false};
    bool calculando{false};
};

struct EstadoControladorVistaCarpetas {
    std::vector<std::wstring> carpetasSeleccionadas;
    size_t indiceCarpetaActual{0};
    std::vector<ElementoArbolVistaCarpetas> elementosArbol;
    std::wstring rutaCarpetaArbolCargada;
    FILETIME ftUltimaModificacionArbol{};
    int desplazamientoScrollY{0};
    int altoTotalArbol{0};
    int nivelExpansionActual{0}; // 0 = Raíz, 1 = Nivel 1, 2 = Nivel 2, ...
    RECT botonAlternarExpansion{};
    bool hoverBotonAlternarExpansion{false};

    // Metadatos de tamano total de carpeta
    std::wstring tamanoTotalCarpetaActualFormateado;
    uintmax_t tamanoTotalCarpetaActualBytes{0};

    // Navegacion entre carpetas seleccionadas
    RECT botonAnteriorCarpeta{};
    RECT botonSiguienteCarpeta{};
    bool hoverBotonAnterior{false};
    bool hoverBotonSiguiente{false};

    // Seccion Abrir con para carpetas
    std::vector<std::wstring> programasCarpetasRecordados;
    int indiceProgramaSeleccionado{0};
    bool menuProgramasAbierto{false};
    int opcionProgramaHover{-1};
    std::wstring programaTemporal;
    RECT botonAbrirCon{};
    RECT botonSelectorPrograma{};
    RECT botonEjecutarAbrir{};
    std::vector<RECT> areasOpcionesPrograma;
    RECT areaCajaArbol{};

    // Seccion Operaciones con carpeta
    RECT botonFusionCarpetas{};
    bool hoverBotonFusion{false};

    // Contenedor de acciones rápidas de carpeta
    RECT areaContenedorAccionesRapidas{};
    RECT botonCrearEnlaceSimbolico{};
    RECT botonCopiarContenidoPortapapeles{};
    bool hoverBotonCrearEnlace{false};
    bool hoverBotonCopiarContenido{false};

    // Vista vacia cuando no hay nada seleccionado en el Explorador
    ResumenVistaVacia resumenVistaVacia;
};

inline EstadoControladorVistaCarpetas estadoVistaCarpetas;

namespace {

inline std::wstring formatearTamanoCarpeta(uintmax_t tamanoEnBytes) {
    if (tamanoEnBytes < 1024) {
        return std::to_wstring(tamanoEnBytes) + L" B";
    }
    if (tamanoEnBytes < 1024 * 1024) {
        wchar_t bufer[32];
        swprintf_s(bufer, L"%.1f KB", static_cast<double>(tamanoEnBytes) / 1024.0);
        return bufer;
    }
    if (tamanoEnBytes < 1024ULL * 1024 * 1024) {
        wchar_t bufer[32];
        swprintf_s(bufer, L"%.1f MB", static_cast<double>(tamanoEnBytes) / (1024.0 * 1024.0));
        return bufer;
    }
    if (tamanoEnBytes < 1024ULL * 1024 * 1024 * 1024) {
        wchar_t bufer[32];
        swprintf_s(bufer, L"%.2f GB", static_cast<double>(tamanoEnBytes) / (1024.0 * 1024.0 * 1024.0));
        return bufer;
    }
    wchar_t bufer[32];
    swprintf_s(bufer, L"%.2f TB", static_cast<double>(tamanoEnBytes) / (1024.0 * 1024.0 * 1024.0 * 1024.0));
    return bufer;
}

inline uintmax_t calcularTamanoDirectorio(const std::wstring& rutaDirectorio, int limiteProfundidad = 64, int* contadorArchivosEscaneados = nullptr) {
    if (limiteProfundidad <= 0) return 0;
    if (contadorArchivosEscaneados && *contadorArchivosEscaneados >= 50000) return 0;

    std::wstring patron = rutaDirectorio + L"\\*";
    WIN32_FIND_DATAW datos;
    HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
    if (busqueda == INVALID_HANDLE_VALUE) return 0;

    uintmax_t tamanoTotal = 0;
    do {
        if (wcscmp(datos.cFileName, L".") == 0 || wcscmp(datos.cFileName, L"..") == 0) {
            continue;
        }
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            continue;
        }
        if (contadorArchivosEscaneados) {
            (*contadorArchivosEscaneados)++;
            if (*contadorArchivosEscaneados >= 50000) break;
        }
        std::wstring rutaHijo = rutaDirectorio + L"\\" + datos.cFileName;
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            tamanoTotal += calcularTamanoDirectorio(rutaHijo, limiteProfundidad - 1, contadorArchivosEscaneados);
        } else {
            uintmax_t tamanoArchivo = (static_cast<uintmax_t>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
            tamanoTotal += tamanoArchivo;
        }
    } while (FindNextFileW(busqueda, &datos));
    FindClose(busqueda);
    return tamanoTotal;
}

}  // namespace

inline void liberarRecursosVistaCarpetas() {
    estadoVistaCarpetas.carpetasSeleccionadas.clear();
    estadoVistaCarpetas.indiceCarpetaActual = 0;
    estadoVistaCarpetas.elementosArbol.clear();
    estadoVistaCarpetas.rutaCarpetaArbolCargada.clear();
    estadoVistaCarpetas.ftUltimaModificacionArbol = {};
    estadoVistaCarpetas.tamanoTotalCarpetaActualBytes = 0;
    estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado.clear();
    estadoVistaCarpetas.desplazamientoScrollY = 0;
    estadoVistaCarpetas.altoTotalArbol = 0;
    estadoVistaCarpetas.nivelExpansionActual = 0;
    estadoVistaCarpetas.hoverBotonAnterior = false;
    estadoVistaCarpetas.hoverBotonSiguiente = false;
    estadoVistaCarpetas.hoverBotonAlternarExpansion = false;
    estadoVistaCarpetas.menuProgramasAbierto = false;
    estadoVistaCarpetas.opcionProgramaHover = -1;
    estadoVistaCarpetas.programaTemporal.clear();
    estadoVistaCarpetas.areaContenedorAccionesRapidas = {};
    estadoVistaCarpetas.botonCrearEnlaceSimbolico = {};
    estadoVistaCarpetas.botonCopiarContenidoPortapapeles = {};
    estadoVistaCarpetas.hoverBotonCrearEnlace = false;
    estadoVistaCarpetas.hoverBotonCopiarContenido = false;
    estadoVistaCarpetas.resumenVistaVacia = {};
}

inline int obtenerMaximoNivelJerarquiaArbol() {
    int maxNivel = 0;
    for (const auto& elementoArbol : estadoVistaCarpetas.elementosArbol) {
        if (elementoArbol.nivelJerarquia > maxNivel) {
            maxNivel = elementoArbol.nivelJerarquia;
        }
    }
    return maxNivel;
}

inline void avanzarNivelExpansionArbol() {
    int maxNivel = obtenerMaximoNivelJerarquiaArbol();
    if (maxNivel <= 0) {
        estadoVistaCarpetas.nivelExpansionActual = 0;
        return;
    }
    if (estadoVistaCarpetas.nivelExpansionActual >= maxNivel) {
        estadoVistaCarpetas.nivelExpansionActual = 0;
    } else {
        estadoVistaCarpetas.nivelExpansionActual++;
    }
}

inline void inspeccionarEstructuraCarpeta(const std::wstring& rutaCarpeta, int nivel = 0, int maxNivel = 3) {
    if (nivel > maxNivel) return;
    if (estadoVistaCarpetas.elementosArbol.size() > 500) return;

    std::wstring patron = rutaCarpeta + L"\\*";
    WIN32_FIND_DATAW datos;
    HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
    if (busqueda == INVALID_HANDLE_VALUE) return;

    std::vector<ElementoArbolVistaCarpetas> subcarpetas;
    std::vector<ElementoArbolVistaCarpetas> archivos;

    do {
        if (wcscmp(datos.cFileName, L".") == 0 || wcscmp(datos.cFileName, L"..") == 0) {
            continue;
        }
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            continue;
        }

        std::wstring rutaCompleta = rutaCarpeta + L"\\" + datos.cFileName;
        bool esDir = (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        ElementoArbolVistaCarpetas elemento;
        elemento.nombre = datos.cFileName;
        elemento.rutaCompleta = rutaCompleta;
        elemento.esDirectorio = esDir;
        elemento.nivelJerarquia = nivel;

        if (esDir) {
            elemento.tamanoBytes = calcularTamanoDirectorio(rutaCompleta, 1);
            subcarpetas.push_back(elemento);
        } else {
            elemento.tamanoBytes = (static_cast<uintmax_t>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
            archivos.push_back(elemento);
        }
    } while (FindNextFileW(busqueda, &datos));
    FindClose(busqueda);

    std::sort(subcarpetas.begin(), subcarpetas.end(), [](const ElementoArbolVistaCarpetas& a, const ElementoArbolVistaCarpetas& b) {
        return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
    });
    std::sort(archivos.begin(), archivos.end(), [](const ElementoArbolVistaCarpetas& a, const ElementoArbolVistaCarpetas& b) {
        return StrCmpLogicalW(a.nombre.c_str(), b.nombre.c_str()) < 0;
    });

    for (const auto& directorioHijo : subcarpetas) {
        estadoVistaCarpetas.elementosArbol.push_back(directorioHijo);
        if (nivel < maxNivel && estadoVistaCarpetas.elementosArbol.size() < 500) {
            inspeccionarEstructuraCarpeta(directorioHijo.rutaCompleta, nivel + 1, maxNivel);
        }
    }
    for (const auto& archivoHijo : archivos) {
        if (estadoVistaCarpetas.elementosArbol.size() < 500) {
            estadoVistaCarpetas.elementosArbol.push_back(archivoHijo);
        }
    }
}

inline bool cargarCarpetaPorIndice(size_t indiceCarpeta) {
    if (indiceCarpeta >= estadoVistaCarpetas.carpetasSeleccionadas.size()) return false;
    estadoVistaCarpetas.indiceCarpetaActual = indiceCarpeta;

    const std::wstring& ruta = estadoVistaCarpetas.carpetasSeleccionadas[indiceCarpeta];
    WIN32_FILE_ATTRIBUTE_DATA datosAttr;
    bool tieneAttr = GetFileAttributesExW(ruta.c_str(), GetFileExInfoStandard, &datosAttr) != FALSE;

    if (tieneAttr && estadoVistaCarpetas.rutaCarpetaArbolCargada == ruta &&
        !estadoVistaCarpetas.elementosArbol.empty() &&
        memcmp(&datosAttr.ftLastWriteTime, &estadoVistaCarpetas.ftUltimaModificacionArbol, sizeof(FILETIME)) == 0) {
        return true;
    }

    estadoVistaCarpetas.elementosArbol.clear();
    estadoVistaCarpetas.desplazamientoScrollY = 0;
    estadoVistaCarpetas.nivelExpansionActual = 0;
    estadoVistaCarpetas.rutaCarpetaArbolCargada = ruta;
    if (tieneAttr) {
        estadoVistaCarpetas.ftUltimaModificacionArbol = datosAttr.ftLastWriteTime;
    }

    estadoVistaCarpetas.tamanoTotalCarpetaActualBytes = calcularTamanoDirectorio(ruta, 4);
    estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado =
        formatearTamanoCarpeta(estadoVistaCarpetas.tamanoTotalCarpetaActualBytes);

    inspeccionarEstructuraCarpeta(ruta, 0, 3);
    return true;
}

inline std::vector<std::wstring> obtenerSubcarpetasEnMismaCarpeta(const std::wstring& rutaCarpetaActual) {
    if (rutaCarpetaActual.empty()) return {};

    wchar_t buferDirectorio[MAX_PATH];
    wcsncpy_s(buferDirectorio, rutaCarpetaActual.c_str(), _TRUNCATE);
    PathRemoveFileSpecW(buferDirectorio);

    std::wstring patronBusqueda = std::wstring(buferDirectorio) + L"\\*";
    WIN32_FIND_DATAW datos;
    HANDLE busqueda = FindFirstFileW(patronBusqueda.c_str(), &datos);
    if (busqueda == INVALID_HANDLE_VALUE) return {};

    std::vector<std::wstring> listaCarpetas;
    do {
        if ((datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            wcscmp(datos.cFileName, L".") != 0 && wcscmp(datos.cFileName, L"..") != 0) {
            std::wstring rutaCompleta = std::wstring(buferDirectorio) + L"\\" + datos.cFileName;
            listaCarpetas.push_back(rutaCompleta);
        }
    } while (FindNextFileW(busqueda, &datos));
    FindClose(busqueda);

    std::sort(listaCarpetas.begin(), listaCarpetas.end(),
              [](const std::wstring& a, const std::wstring& b) {
                  return StrCmpLogicalW(a.c_str(), b.c_str()) < 0;
              });

    return listaCarpetas;
}

inline bool mostrarCarpetaAnterior() {
    if (estadoVistaCarpetas.carpetasSeleccionadas.size() > 1) {
        if (estadoVistaCarpetas.indiceCarpetaActual == 0) {
            estadoVistaCarpetas.indiceCarpetaActual = estadoVistaCarpetas.carpetasSeleccionadas.size() - 1;
        } else {
            estadoVistaCarpetas.indiceCarpetaActual--;
        }
        return cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
    }
    if (estadoVistaCarpetas.rutaCarpetaArbolCargada.empty()) return false;
    std::vector<std::wstring> listaVecinas =
        obtenerSubcarpetasEnMismaCarpeta(estadoVistaCarpetas.rutaCarpetaArbolCargada);
    if (listaVecinas.empty()) return false;
    auto it = std::find(listaVecinas.begin(), listaVecinas.end(),
                        estadoVistaCarpetas.rutaCarpetaArbolCargada);
    std::wstring destino;
    if (it == listaVecinas.end() || it == listaVecinas.begin()) {
        destino = listaVecinas.back();
    } else {
        destino = *(it - 1);
    }
    if (destino.empty() || destino == estadoVistaCarpetas.rutaCarpetaArbolCargada) return false;
    estadoVistaCarpetas.carpetasSeleccionadas = { destino };
    estadoVistaCarpetas.indiceCarpetaActual = 0;
    return cargarCarpetaPorIndice(0);
}

inline bool mostrarCarpetaSiguiente() {
    if (estadoVistaCarpetas.carpetasSeleccionadas.size() > 1) {
        estadoVistaCarpetas.indiceCarpetaActual =
            (estadoVistaCarpetas.indiceCarpetaActual + 1) % estadoVistaCarpetas.carpetasSeleccionadas.size();
        return cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
    }
    if (estadoVistaCarpetas.rutaCarpetaArbolCargada.empty()) return false;
    std::vector<std::wstring> listaVecinas =
        obtenerSubcarpetasEnMismaCarpeta(estadoVistaCarpetas.rutaCarpetaArbolCargada);
    if (listaVecinas.empty()) return false;
    auto it = std::find(listaVecinas.begin(), listaVecinas.end(),
                        estadoVistaCarpetas.rutaCarpetaArbolCargada);
    std::wstring destino;
    if (it == listaVecinas.end() || it + 1 == listaVecinas.end()) {
        destino = listaVecinas.front();
    } else {
        destino = *(it + 1);
    }
    if (destino.empty() || destino == estadoVistaCarpetas.rutaCarpetaArbolCargada) return false;
    estadoVistaCarpetas.carpetasSeleccionadas = { destino };
    estadoVistaCarpetas.indiceCarpetaActual = 0;
    return cargarCarpetaPorIndice(0);
}

inline void calcularResumenVistaVacia(const std::wstring& rutaCarpeta) {
    if (rutaCarpeta.empty()) {
        estadoVistaCarpetas.resumenVistaVacia = {};
        estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema = true;
        estadoVistaCarpetas.resumenVistaVacia.nombreUbicacionSistema = L"Ubicación del Sistema";
        estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
            L"Esta es una sección del sistema de Windows (Inicio, Galería o Este equipo). Las herramientas de vista previa se activarán automáticamente al navegar por carpetas y archivos locales.";
        estadoVistaCarpetas.resumenVistaVacia.calculado = true;
        return;
    }

    if (rutaCarpeta.rfind(L"::SISTEMA::", 0) == 0 || rutaCarpeta.rfind(L"::{", 0) == 0 || rutaCarpeta.rfind(L"shell:::", 0) == 0) {
        std::wstring nombreLimpio;
        if (rutaCarpeta.rfind(L"::SISTEMA::", 0) == 0) {
            nombreLimpio = rutaCarpeta.substr(11);
        } else {
            std::wstring min = rutaCarpeta;
            std::transform(min.begin(), min.end(), min.begin(), ::towlower);
            if (min.find(L"e88865ea-0e1c-4e20-9aa6-ed353b7c4777") != std::wstring::npos) nombreLimpio = L"Galería";
            else if (min.find(L"f874310e-b6b7-47dc-bc84-b9e6b38f5903") != std::wstring::npos) nombreLimpio = L"Inicio";
            else if (min.find(L"20d04fe0-3aea-1069-a2d8-08002b30309d") != std::wstring::npos) nombreLimpio = L"Este equipo";
            else if (min.find(L"645ff040-5081-101b-9f08-00aa002f954e") != std::wstring::npos) nombreLimpio = L"Papelera de reciclaje";
            else if (min.find(L"f02c1a0d-be21-4350-88b0-7367fc96ef3c") != std::wstring::npos) nombreLimpio = L"Red";
            else if (min.find(L"26ee0668-a00a-44d7-9371-beb064c98683") != std::wstring::npos) nombreLimpio = L"Panel de control";
            else nombreLimpio = L"Ubicación del Sistema";
        }

        if (nombreLimpio.find(L'{') != std::wstring::npos || nombreLimpio.find(L"::") != std::wstring::npos) {
            nombreLimpio = L"Ubicación del Sistema";
        }

        estadoVistaCarpetas.resumenVistaVacia = {};
        estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema = true;
        estadoVistaCarpetas.resumenVistaVacia.nombreUbicacionSistema = nombreLimpio;
        if (nombreLimpio == L"Galería" || nombreLimpio == L"Gallery") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"La Galería es una vista agregada de imágenes del sistema y dispositivos vinculados. La vista previa interactiva se activará cuando navegues por carpetas o selecciones imágenes locales.";
        } else if (nombreLimpio == L"Inicio" || nombreLimpio == L"Home") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"La sección Inicio contiene accesos rápidos y archivos recientes. El visor se activará en cuanto selecciones un archivo o ingreses a una carpeta local.";
        } else if (nombreLimpio == L"Este equipo" || nombreLimpio == L"This PC") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"En 'Este equipo' se muestran las unidades y dispositivos del sistema. Ingresa a cualquier unidad o carpeta local para visualizar su estructura y contenidos.";
        } else if (nombreLimpio == L"Papelera de reciclaje" || nombreLimpio == L"Recycle Bin") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"La Papelera de reciclaje almacena temporalmente los archivos eliminados del sistema.";
        } else if (nombreLimpio == L"Red" || nombreLimpio == L"Network") {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"En la sección Red se muestran los equipos y carpetas compartidas disponibles en la red local.";
        } else {
            estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
                L"Esta sección corresponde a una ubicación virtual del sistema. Las vistas previas avanzadas se activarán al navegar o seleccionar archivos locales.";
        }
        estadoVistaCarpetas.resumenVistaVacia.calculado = true;
        return;
    }

    WIN32_FILE_ATTRIBUTE_DATA datosAttr;
    bool tieneAttr = GetFileAttributesExW(rutaCarpeta.c_str(), GetFileExInfoStandard, &datosAttr) != FALSE;

    if (tieneAttr && estadoVistaCarpetas.resumenVistaVacia.calculado &&
        !estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema &&
        estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta == rutaCarpeta &&
        memcmp(&datosAttr.ftLastWriteTime, &estadoVistaCarpetas.resumenVistaVacia.ftUltimaModificacion, sizeof(FILETIME)) == 0) {
        return;
    }

    if (!tieneAttr || !(datosAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        estadoVistaCarpetas.resumenVistaVacia = {};
        estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema = true;
        estadoVistaCarpetas.resumenVistaVacia.nombreUbicacionSistema = L"Ubicación del Sistema";
        estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema =
            L"Ubicación especial o virtual del sistema. El visor se activará automáticamente al explorar carpetas y archivos locales.";
        estadoVistaCarpetas.resumenVistaVacia.calculado = true;
        return;
    }

    estadoVistaCarpetas.resumenVistaVacia = {};
    estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema = false;
    estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta = rutaCarpeta;
    estadoVistaCarpetas.resumenVistaVacia.nombreCarpeta = PathFindFileNameW(rutaCarpeta.c_str());
    estadoVistaCarpetas.resumenVistaVacia.calculando = true;

    if (tieneAttr) {
        estadoVistaCarpetas.resumenVistaVacia.ftUltimaModificacion = datosAttr.ftLastWriteTime;
        FILETIME ftLocal;
        FileTimeToLocalFileTime(&datosAttr.ftLastAccessTime, &ftLocal);
        SYSTEMTIME st;
        FileTimeToSystemTime(&ftLocal, &st);
        wchar_t buferFecha[64];
        swprintf_s(buferFecha, L"%02u/%02u/%04u %02u:%02u",
                   st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
        estadoVistaCarpetas.resumenVistaVacia.fechaUltimoAcceso = buferFecha;
    }

    std::wstring patron = rutaCarpeta + L"\\*";
    WIN32_FIND_DATAW datos;
    HANDLE busqueda = FindFirstFileW(patron.c_str(), &datos);
    if (busqueda == INVALID_HANDLE_VALUE) {
        estadoVistaCarpetas.resumenVistaVacia.calculando = false;
        estadoVistaCarpetas.resumenVistaVacia.calculado = true;
        return;
    }

    uintmax_t tamanoTotal = 0;
    int totalCarpetas = 0;
    int totalArchivos = 0;
    std::map<std::wstring, int> extensiones;
    std::vector<SubcarpetaPesada> subcarpetas;
    std::vector<std::wstring> carpetasVaciasDetectadas;

    do {
        if (wcscmp(datos.cFileName, L".") == 0 || wcscmp(datos.cFileName, L"..") == 0) {
            continue;
        }
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            continue;
        }
        std::wstring rutaHijo = rutaCarpeta + L"\\" + datos.cFileName;
        if (datos.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            totalCarpetas++;
            if (PathIsDirectoryEmptyW(rutaHijo.c_str())) {
                carpetasVaciasDetectadas.push_back(datos.cFileName);
            }
            int contadorEscaneados = 0;
            uintmax_t tamSub = calcularTamanoDirectorio(rutaHijo, 4, &contadorEscaneados);
            tamanoTotal += tamSub;
            SubcarpetaPesada subcarpetaItem;
            subcarpetaItem.nombre = datos.cFileName;
            subcarpetaItem.tamanoBytes = tamSub;
            subcarpetas.push_back(subcarpetaItem);
        } else {
            totalArchivos++;
            uintmax_t tamanoArchivo = (static_cast<uintmax_t>(datos.nFileSizeHigh) << 32) | datos.nFileSizeLow;
            tamanoTotal += tamanoArchivo;
            const wchar_t* extensionArchivo = PathFindExtensionW(datos.cFileName);
            if (extensionArchivo && *extensionArchivo) {
                std::wstring extensionMinusculas = extensionArchivo;
                std::transform(extensionMinusculas.begin(), extensionMinusculas.end(), extensionMinusculas.begin(), ::towlower);
                extensiones[extensionMinusculas]++;
            } else {
                extensiones[L"(sin extension)"]++;
            }
        }
    } while (FindNextFileW(busqueda, &datos));
    FindClose(busqueda);

    std::vector<SubcarpetaPesada> subcarpetasOrdenadas = subcarpetas;
    std::sort(subcarpetasOrdenadas.begin(), subcarpetasOrdenadas.end(), [](const SubcarpetaPesada& primera, const SubcarpetaPesada& segunda) {
        return primera.tamanoBytes > segunda.tamanoBytes;
    });

    std::vector<SubcarpetaPesada> top4;
    for (size_t posicionSubcarpeta = 0; posicionSubcarpeta < std::min<size_t>(4, subcarpetasOrdenadas.size()); ++posicionSubcarpeta) {
        SubcarpetaPesada subcarpetaItem = subcarpetasOrdenadas[posicionSubcarpeta];
        subcarpetaItem.porcentaje = (tamanoTotal > 0) ? (static_cast<double>(subcarpetaItem.tamanoBytes) / static_cast<double>(tamanoTotal)) : 0.0;
        top4.push_back(subcarpetaItem);
    }

    std::vector<std::pair<int, std::wstring>> listaOrdenadaExtensiones;
    for (const auto& [extensionClave, cantidadArchivos] : extensiones) {
        listaOrdenadaExtensiones.push_back({cantidadArchivos, extensionClave});
    }
    std::sort(listaOrdenadaExtensiones.begin(), listaOrdenadaExtensiones.end(),
              [](const std::pair<int, std::wstring>& primera, const std::pair<int, std::wstring>& segunda) {
                  if (primera.first != segunda.first) {
                      return primera.first > segunda.first;
                  }
                  return StrCmpLogicalW(primera.second.c_str(), segunda.second.c_str()) < 0;
              });

    std::wstring textoExtensionesAcumulado;
    for (size_t posicionExtension = 0; posicionExtension < listaOrdenadaExtensiones.size(); ++posicionExtension) {
        if (posicionExtension > 0) {
            textoExtensionesAcumulado += L", ";
        }
        textoExtensionesAcumulado += listaOrdenadaExtensiones[posicionExtension].second;
    }

    estadoVistaCarpetas.resumenVistaVacia.tamanoTotalBytes = tamanoTotal;
    estadoVistaCarpetas.resumenVistaVacia.totalSubcarpetas = totalCarpetas;
    estadoVistaCarpetas.resumenVistaVacia.totalArchivos = totalArchivos;
    estadoVistaCarpetas.resumenVistaVacia.cantidadTiposArchivos = static_cast<int>(extensiones.size());
    estadoVistaCarpetas.resumenVistaVacia.textoExtensionesFantasma = textoExtensionesAcumulado;
    estadoVistaCarpetas.resumenVistaVacia.top4CarpetasMasPesadas = top4;
    estadoVistaCarpetas.resumenVistaVacia.carpetasVacias = carpetasVaciasDetectadas;
    estadoVistaCarpetas.resumenVistaVacia.calculando = false;
    estadoVistaCarpetas.resumenVistaVacia.calculado = true;
}

inline bool eliminarCarpetasVacias() {
    if (estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta.empty() ||
        estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.empty()) {
        return false;
    }

    int eliminadas = 0;
    const std::wstring& rutaBase = estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta;
    for (const auto& nombreCarpeta : estadoVistaCarpetas.resumenVistaVacia.carpetasVacias) {
        std::wstring rutaCompleta = rutaBase + L"\\" + nombreCarpeta;
        if (PathIsDirectoryEmptyW(rutaCompleta.c_str())) {
            if (RemoveDirectoryW(rutaCompleta.c_str())) {
                eliminadas++;
            }
        }
    }

    std::wstring msgResultado;
    if (eliminadas > 0) {
        msgResultado = L"✓ Se eliminaron " + std::to_wstring(eliminadas) + L" carpetas vacías exitosamente.";
    }

    std::wstring rutaActual = rutaBase;
    estadoVistaCarpetas.resumenVistaVacia.calculado = false;
    calcularResumenVistaVacia(rutaActual);
    estadoVistaCarpetas.resumenVistaVacia.mensajeResultadoEliminacion = msgResultado;
    return true;
}

inline void seleccionarYAgregarProgramaCarpeta(HWND ventanaPadre) {
    wchar_t buferRutaArchivo[MAX_PATH] = L"";
    OPENFILENAMEW estructuraOfn{};
    estructuraOfn.lStructSize = sizeof(estructuraOfn);
    estructuraOfn.hwndOwner = ventanaPadre;
    estructuraOfn.lpstrFilter = L"Archivos Ejecutables (*.exe)\0*.exe\0Todos los archivos (*.*)\0*.*\0";
    estructuraOfn.lpstrFile = buferRutaArchivo;
    estructuraOfn.nMaxFile = MAX_PATH;
    estructuraOfn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    estructuraOfn.lpstrTitle = L"Seleccionar programa para abrir carpetas";

    if (GetOpenFileNameW(&estructuraOfn)) {
        std::wstring rutaSeleccionada(buferRutaArchivo);
        if (!rutaSeleccionada.empty()) {
            std::wstring nombreEjecutable = PathFindFileNameW(rutaSeleccionada.c_str());
            std::wstring mensaje = L"¿Desea recordar '" + nombreEjecutable + L"' en la lista de opciones para carpetas?";
            int res = MessageBoxW(ventanaPadre, mensaje.c_str(), L"Recordar programa", MB_YESNO | MB_ICONQUESTION);
            if (res == IDYES) {
                auto it = std::find(estadoVistaCarpetas.programasCarpetasRecordados.begin(),
                                    estadoVistaCarpetas.programasCarpetasRecordados.end(), rutaSeleccionada);
                if (it == estadoVistaCarpetas.programasCarpetasRecordados.end()) {
                    estadoVistaCarpetas.programasCarpetasRecordados.push_back(rutaSeleccionada);
                    estadoVistaCarpetas.indiceProgramaSeleccionado =
                        static_cast<int>(estadoVistaCarpetas.programasCarpetasRecordados.size() - 1);
                } else {
                    estadoVistaCarpetas.indiceProgramaSeleccionado =
                        static_cast<int>(std::distance(estadoVistaCarpetas.programasCarpetasRecordados.begin(), it));
                }
                estadoVistaCarpetas.programaTemporal.clear();
            } else {
                estadoVistaCarpetas.programaTemporal = rutaSeleccionada;
            }
        }
    }
}
inline std::wstring resolverProgramaParaCarpetas() {
    if (!estadoVistaCarpetas.programaTemporal.empty()) {
        return estadoVistaCarpetas.programaTemporal;
    }
    if (!estadoVistaCarpetas.programasCarpetasRecordados.empty() &&
        estadoVistaCarpetas.indiceProgramaSeleccionado >= 0 &&
        estadoVistaCarpetas.indiceProgramaSeleccionado <
            static_cast<int>(estadoVistaCarpetas.programasCarpetasRecordados.size())) {
        return estadoVistaCarpetas.programasCarpetasRecordados[static_cast<size_t>(
            estadoVistaCarpetas.indiceProgramaSeleccionado)];
    }
    if (!configuracion.programaCarpetasPorDefecto.empty()) {
        return configuracion.programaCarpetasPorDefecto;
    }
    if (estadoVistaCarpetas.programasCarpetasRecordados.size() == 1) {
        return estadoVistaCarpetas.programasCarpetasRecordados.front();
    }
    if (!estadoVistaCarpetas.programasCarpetasRecordados.empty()) {
        return estadoVistaCarpetas.programasCarpetasRecordados.front();
    }
    return {};
}

inline void mostrarMenuDesplegableProgramasCarpetas(HWND ventana, const RECT& botonRect) {
    if (estadoVistaCarpetas.programasCarpetasRecordados.empty() && estadoVistaCarpetas.programaTemporal.empty()) {
        seleccionarYAgregarProgramaCarpeta(ventana);
        InvalidateRect(ventana, nullptr, FALSE);
        return;
    }

    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    for (size_t posicionPrograma = 0; posicionPrograma < estadoVistaCarpetas.programasCarpetasRecordados.size(); ++posicionPrograma) {
        UINT banderasMenu = MF_STRING;
        if (static_cast<int>(posicionPrograma) == estadoVistaCarpetas.indiceProgramaSeleccionado) {
            banderasMenu |= MF_CHECKED;
        }
        std::wstring nombreProg = PathFindFileNameW(estadoVistaCarpetas.programasCarpetasRecordados[posicionPrograma].c_str());
        AppendMenuW(menu, banderasMenu, static_cast<UINT_PTR>(3000 + posicionPrograma), nombreProg.c_str());
    }

    if (!estadoVistaCarpetas.programasCarpetasRecordados.empty()) {
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    AppendMenuW(menu, MF_STRING, 3999, L"+ Añadir programa...");

    POINT puntoBoton{botonRect.left, botonRect.top};
    ClientToScreen(ventana, &puntoBoton);

    SetForegroundWindow(ventana);
    UINT comandoSeleccionado = static_cast<UINT>(TrackPopupMenuEx(
        menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_VERTICAL,
        puntoBoton.x, puntoBoton.y, ventana, nullptr));
    DestroyMenu(menu);

    if (comandoSeleccionado >= 3000 && comandoSeleccionado < 3000 + estadoVistaCarpetas.programasCarpetasRecordados.size()) {
        estadoVistaCarpetas.indiceProgramaSeleccionado = static_cast<int>(comandoSeleccionado - 3000);
        estadoVistaCarpetas.programaTemporal.clear();
        InvalidateRect(ventana, nullptr, FALSE);
    } else if (comandoSeleccionado == 3999) {
        seleccionarYAgregarProgramaCarpeta(ventana);
        InvalidateRect(ventana, nullptr, FALSE);
    }
}

inline void ejecutarAbrirCarpetasConPrograma(HWND ventanaPadre) {
    std::wstring rutaProg = resolverProgramaParaCarpetas();

    if (rutaProg.empty()) {
        seleccionarYAgregarProgramaCarpeta(ventanaPadre);
        rutaProg = resolverProgramaParaCarpetas();
        if (rutaProg.empty()) return;
    }

    for (const auto& rutaCarpeta : estadoVistaCarpetas.carpetasSeleccionadas) {
        ShellExecuteW(nullptr, L"open", rutaProg.c_str(),
                      (L"\"" + rutaCarpeta + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
    }
}

inline bool ejecutarCrearEnlaceSimbolico(HWND ventanaPadre) {
    if (estadoVistaCarpetas.carpetasSeleccionadas.empty() ||
        estadoVistaCarpetas.indiceCarpetaActual >= estadoVistaCarpetas.carpetasSeleccionadas.size()) {
        return false;
    }
    const std::wstring& rutaOrigen = estadoVistaCarpetas.carpetasSeleccionadas[estadoVistaCarpetas.indiceCarpetaActual];
    if (rutaOrigen.empty() || !PathFileExistsW(rutaOrigen.c_str())) return false;

    std::wstring nombreCarpetaOrigen = PathFindFileNameW(rutaOrigen.c_str());

    PunteroCom<IFileSaveDialog> dialogoGuardar;
    HRESULT resultadoCom = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER,
                                            IID_PPV_ARGS(dialogoGuardar.recibir()));
    if (FAILED(resultadoCom) || !dialogoGuardar.obtener()) {
        return false;
    }

    dialogoGuardar->SetTitle(L"Seleccionar ubicación para el enlace simbólico");
    dialogoGuardar->SetFileName(nombreCarpetaOrigen.c_str());
    COMDLG_FILTERSPEC filtroCom[] = {
        { L"Todos los archivos (*.*)", L"*.*" }
    };
    dialogoGuardar->SetFileTypes(1, filtroCom);

    if (SUCCEEDED(dialogoGuardar->Show(ventanaPadre))) {
        PunteroCom<IShellItem> elementoResultado;
        if (SUCCEEDED(dialogoGuardar->GetResult(elementoResultado.recibir())) && elementoResultado.obtener()) {
            PWSTR rutaElegida = nullptr;
            if (SUCCEEDED(elementoResultado->GetDisplayName(SIGDN_FILESYSPATH, &rutaElegida)) && rutaElegida) {
                std::wstring rutaDestinoEnlace(rutaElegida);
                CoTaskMemFree(rutaElegida);

                if (!rutaDestinoEnlace.empty()) {
                    BOOL enlaceCreado = CreateSymbolicLinkW(rutaDestinoEnlace.c_str(), rutaOrigen.c_str(),
                                                            SYMBOLIC_LINK_FLAG_DIRECTORY);
                    if (!enlaceCreado) {
                        enlaceCreado = CreateSymbolicLinkW(rutaDestinoEnlace.c_str(), rutaOrigen.c_str(),
                                                            SYMBOLIC_LINK_FLAG_DIRECTORY | 0x1);
                    }
                    if (!enlaceCreado) {
                        std::wstring rutaAccesoDirecto = rutaDestinoEnlace;
                        if (PathFindExtensionW(rutaAccesoDirecto.c_str()) == nullptr ||
                            lstrcmpiW(PathFindExtensionW(rutaAccesoDirecto.c_str()), L".lnk") != 0) {
                            rutaAccesoDirecto += L".lnk";
                        }
                        PunteroCom<IShellLinkW> accesoDirecto;
                        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(accesoDirecto.recibir())))) {
                            accesoDirecto->SetPath(rutaOrigen.c_str());
                            PunteroCom<IPersistFile> archivoPersistente;
                            if (SUCCEEDED(accesoDirecto->QueryInterface(IID_PPV_ARGS(archivoPersistente.recibir())))) {
                                if (SUCCEEDED(archivoPersistente->Save(rutaAccesoDirecto.c_str(), TRUE))) {
                                    enlaceCreado = TRUE;
                                    rutaDestinoEnlace = rutaAccesoDirecto;
                                }
                            }
                        }
                    }

                    if (enlaceCreado) {
                        std::wstring mensajeExito = L"✓ Enlace creado exitosamente en:\n" + rutaDestinoEnlace;
                        MessageBoxW(ventanaPadre, mensajeExito.c_str(), L"Crear Enlace", MB_OK | MB_ICONINFORMATION);
                        return true;
                    } else {
                        DWORD errorCodigo = GetLastError();
                        std::wstring mensajeError = L"No fue posible crear el enlace (error " + std::to_wstring(errorCodigo) + L").";
                        MessageBoxW(ventanaPadre, mensajeError.c_str(), L"Crear Enlace", MB_OK | MB_ICONERROR);
                    }
                }
            }
        }
    }
    return false;
}

inline bool ejecutarCopiarContenidoPortapapeles(HWND ventanaPadre) {
    if (estadoVistaCarpetas.carpetasSeleccionadas.empty() ||
        estadoVistaCarpetas.indiceCarpetaActual >= estadoVistaCarpetas.carpetasSeleccionadas.size()) {
        return false;
    }
    const std::wstring& rutaCarpetaOrigen = estadoVistaCarpetas.carpetasSeleccionadas[estadoVistaCarpetas.indiceCarpetaActual];
    if (rutaCarpetaOrigen.empty() || !PathFileExistsW(rutaCarpetaOrigen.c_str())) return false;

    std::wstring patronBusqueda = rutaCarpetaOrigen + L"\\*";
    WIN32_FIND_DATAW datosElemento{};
    HANDLE busqueda = FindFirstFileW(patronBusqueda.c_str(), &datosElemento);
    if (busqueda == INVALID_HANDLE_VALUE) return false;

    std::vector<std::wstring> subcarpetasRaiz;
    std::vector<std::pair<std::wstring, uintmax_t>> archivosRaiz;

    do {
        if (wcscmp(datosElemento.cFileName, L".") == 0 || wcscmp(datosElemento.cFileName, L"..") == 0) {
            continue;
        }
        if (datosElemento.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            subcarpetasRaiz.push_back(datosElemento.cFileName);
        } else {
            uintmax_t tamanoArchivo = (static_cast<uintmax_t>(datosElemento.nFileSizeHigh) << 32) | datosElemento.nFileSizeLow;
            archivosRaiz.push_back({ datosElemento.cFileName, tamanoArchivo });
        }
    } while (FindNextFileW(busqueda, &datosElemento));
    FindClose(busqueda);

    std::sort(subcarpetasRaiz.begin(), subcarpetasRaiz.end(), [](const std::wstring& primerDirectorio, const std::wstring& segundoDirectorio) {
        return StrCmpLogicalW(primerDirectorio.c_str(), segundoDirectorio.c_str()) < 0;
    });

    std::sort(archivosRaiz.begin(), archivosRaiz.end(), [](const std::pair<std::wstring, uintmax_t>& primerArchivo, const std::pair<std::wstring, uintmax_t>& segundoArchivo) {
        return StrCmpLogicalW(primerArchivo.first.c_str(), segundoArchivo.first.c_str()) < 0;
    });

    std::wstring textoAcumulado;
    textoAcumulado += L"Contenido de: ";
    textoAcumulado += PathFindFileNameW(rutaCarpetaOrigen.c_str());
    textoAcumulado += L"\r\n";
    textoAcumulado += L"Ruta: " + rutaCarpetaOrigen + L"\r\n";
    textoAcumulado += L"--------------------------------------------------\r\n";

    for (const auto& nombreSubcarpeta : subcarpetasRaiz) {
        textoAcumulado += L"📁 [Carpeta] " + nombreSubcarpeta + L"\r\n";
    }
    for (const auto& [nombreArchivo, tamanoBytes] : archivosRaiz) {
        textoAcumulado += L"📄 [Archivo] " + nombreArchivo + L" (" + formatearTamanoCarpeta(tamanoBytes) + L")\r\n";
    }

    textoAcumulado += L"--------------------------------------------------\r\n";
    textoAcumulado += L"Total: " + std::to_wstring(subcarpetasRaiz.size()) + L" carpetas, " +
                      std::to_wstring(archivosRaiz.size()) + L" archivos (" +
                      std::to_wstring(subcarpetasRaiz.size() + archivosRaiz.size()) + L" elementos)\r\n";

    if (!OpenClipboard(ventanaPadre)) {
        return false;
    }
    EmptyClipboard();

    size_t tamanoBufer = (textoAcumulado.size() + 1) * sizeof(wchar_t);
    HGLOBAL memoriaGlobal = GlobalAlloc(GMEM_MOVEABLE, tamanoBufer);
    if (memoriaGlobal) {
        wchar_t* punteroDestino = static_cast<wchar_t*>(GlobalLock(memoriaGlobal));
        if (punteroDestino) {
            memcpy(punteroDestino, textoAcumulado.c_str(), tamanoBufer);
            GlobalUnlock(memoriaGlobal);
            SetClipboardData(CF_UNICODETEXT, memoriaGlobal);
        }
    }
    CloseClipboard();

    MessageBoxW(ventanaPadre, L"✓ El contenido del directorio raíz ha sido copiado al portapapeles.",
                L"Copiar Contenido", MB_OK | MB_ICONINFORMATION);
    return true;
}

inline bool fusionarDirectorioRecursivo(const std::wstring& rutaOrigen, const std::wstring& rutaDestino) {
    CreateDirectoryW(rutaDestino.c_str(), nullptr);
    std::wstring patronBusqueda = rutaOrigen + L"\\*";
    WIN32_FIND_DATAW datosElemento{};
    HANDLE busquedaArchivos = FindFirstFileW(patronBusqueda.c_str(), &datosElemento);
    if (busquedaArchivos == INVALID_HANDLE_VALUE) return false;

    do {
        if (wcscmp(datosElemento.cFileName, L".") == 0 || wcscmp(datosElemento.cFileName, L"..") == 0) {
            continue;
        }
        std::wstring rutaElementoOrigen = rutaOrigen + L"\\" + datosElemento.cFileName;
        std::wstring rutaElementoDestino = rutaDestino + L"\\" + datosElemento.cFileName;

        if (datosElemento.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            fusionarDirectorioRecursivo(rutaElementoOrigen, rutaElementoDestino);
            RemoveDirectoryW(rutaElementoOrigen.c_str());
        } else {
            if (PathFileExistsW(rutaElementoDestino.c_str())) {
                const wchar_t* extensionPuntero = PathFindExtensionW(datosElemento.cFileName);
                std::wstring nombreSinExtension = datosElemento.cFileName;
                if (extensionPuntero && *extensionPuntero) {
                    nombreSinExtension = nombreSinExtension.substr(0, nombreSinExtension.length() - wcslen(extensionPuntero));
                }
                int sufijoIncremental = 2;
                std::wstring destinoAlternativo;
                do {
                    destinoAlternativo = rutaDestino + L"\\" + nombreSinExtension + L" (" +
                                         std::to_wstring(sufijoIncremental) + L")" +
                                         (extensionPuntero ? extensionPuntero : L"");
                    sufijoIncremental++;
                } while (PathFileExistsW(destinoAlternativo.c_str()));
                rutaElementoDestino = destinoAlternativo;
            }
            MoveFileExW(rutaElementoOrigen.c_str(), rutaElementoDestino.c_str(),
                       MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
        }
    } while (FindNextFileW(busquedaArchivos, &datosElemento));
    FindClose(busquedaArchivos);

    RemoveDirectoryW(rutaOrigen.c_str());
    return true;
}

struct ContextoDialogoNombreCarpeta {
    std::wstring nombreSugerido;
    std::wstring nombreResultado;
    bool aceptado{false};
    bool terminado{false};
    HWND controlTextoEdicion{nullptr};
    HWND controlBotonAceptar{nullptr};
    HWND controlBotonCancelar{nullptr};
    HFONT fuenteDialogo{nullptr};
};

inline LRESULT CALLBACK procesarVentanaDialogoNombreCarpeta(HWND ventana, UINT mensaje, WPARAM parametroW, LPARAM parametroL) {
    ContextoDialogoNombreCarpeta* contexto = reinterpret_cast<ContextoDialogoNombreCarpeta*>(
        GetWindowLongPtrW(ventana, GWLP_USERDATA));

    switch (mensaje) {
        case WM_CREATE: {
            CREATESTRUCTW* estructuraCreacion = reinterpret_cast<CREATESTRUCTW*>(parametroL);
            contexto = reinterpret_cast<ContextoDialogoNombreCarpeta*>(estructuraCreacion->lpCreateParams);
            SetWindowLongPtrW(ventana, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(contexto));

            contexto->fuenteDialogo = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                                  DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

            HWND etiquetaTexto = CreateWindowExW(0, L"STATIC", L"Introduzca el nombre de la carpeta final:",
                                                 WS_CHILD | WS_VISIBLE, 20, 16, 360, 20,
                                                 ventana, nullptr, nullptr, nullptr);
            SendMessageW(etiquetaTexto, WM_SETFONT, reinterpret_cast<WPARAM>(contexto->fuenteDialogo), TRUE);

            contexto->controlTextoEdicion = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", contexto->nombreSugerido.c_str(),
                                                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                                            20, 42, 360, 26,
                                                            ventana, reinterpret_cast<HMENU>(101), nullptr, nullptr);
            SendMessageW(contexto->controlTextoEdicion, WM_SETFONT, reinterpret_cast<WPARAM>(contexto->fuenteDialogo), TRUE);
            SendMessageW(contexto->controlTextoEdicion, EM_SETSEL, 0, -1);

            contexto->controlBotonAceptar = CreateWindowExW(0, L"BUTTON", L"Aceptar",
                                                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                                            175, 84, 95, 28,
                                                            ventana, reinterpret_cast<HMENU>(IDOK), nullptr, nullptr);
            SendMessageW(contexto->controlBotonAceptar, WM_SETFONT, reinterpret_cast<WPARAM>(contexto->fuenteDialogo), TRUE);

            contexto->controlBotonCancelar = CreateWindowExW(0, L"BUTTON", L"Cancelar",
                                                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                                             285, 84, 95, 28,
                                                             ventana, reinterpret_cast<HMENU>(IDCANCEL), nullptr, nullptr);
            SendMessageW(contexto->controlBotonCancelar, WM_SETFONT, reinterpret_cast<WPARAM>(contexto->fuenteDialogo), TRUE);

            SetFocus(contexto->controlTextoEdicion);
            return 0;
        }

        case WM_COMMAND: {
            WORD identificadorControl = LOWORD(parametroW);
            if (identificadorControl == IDOK) {
                if (contexto && contexto->controlTextoEdicion) {
                    wchar_t buferTexto[MAX_PATH];
                    GetWindowTextW(contexto->controlTextoEdicion, buferTexto, MAX_PATH);
                    std::wstring textoIngresado = buferTexto;

                    size_t primerNoEspacio = textoIngresado.find_first_not_of(L" \t\r\n");
                    if (primerNoEspacio != std::wstring::npos) {
                        size_t ultimoNoEspacio = textoIngresado.find_last_not_of(L" \t\r\n");
                        textoIngresado = textoIngresado.substr(primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);
                    } else {
                        textoIngresado.clear();
                    }

                    if (textoIngresado.empty()) {
                        MessageBoxW(ventana, L"El nombre de la carpeta no puede estar vacío.",
                                    L"Useful Sideview - Nombre Inválido", MB_ICONWARNING | MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
                        SetFocus(contexto->controlTextoEdicion);
                        return 0;
                    }

                    if (textoIngresado.find_first_of(L"\\/:*?\"<>|") != std::wstring::npos) {
                        MessageBoxW(ventana,
                                    L"El nombre no puede contener ninguno de los siguientes caracteres:\n\\ / : * ? \" < > |",
                                    L"Useful Sideview - Nombre Inválido", MB_ICONWARNING | MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
                        SetFocus(contexto->controlTextoEdicion);
                        return 0;
                    }

                    contexto->nombreResultado = textoIngresado;
                    contexto->aceptado = true;
                }
                DestroyWindow(ventana);
                return 0;
            }
            if (identificadorControl == IDCANCEL) {
                DestroyWindow(ventana);
                return 0;
            }
            break;
        }

        case WM_CLOSE: {
            DestroyWindow(ventana);
            return 0;
        }

        case WM_DESTROY: {
            if (contexto) {
                if (contexto->fuenteDialogo) {
                    DeleteObject(contexto->fuenteDialogo);
                    contexto->fuenteDialogo = nullptr;
                }
                contexto->terminado = true;
            }
            return 0;
        }

        default:
            break;
    }
    return DefWindowProcW(ventana, mensaje, parametroW, parametroL);
}

inline bool solicitarNombreCarpetaFinal(HWND ventanaPadre, const std::wstring& nombreSugerido, std::wstring& nombreElegido) {
    WNDCLASSEXW claseDialogo{};
    claseDialogo.cbSize = sizeof(WNDCLASSEXW);
    claseDialogo.lpfnWndProc = procesarVentanaDialogoNombreCarpeta;
    claseDialogo.hInstance = GetModuleHandleW(nullptr);
    claseDialogo.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    claseDialogo.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    claseDialogo.lpszClassName = L"UsefulSideviewNombreCarpetaDlg";
    RegisterClassExW(&claseDialogo);

    ContextoDialogoNombreCarpeta contexto;
    contexto.nombreSugerido = nombreSugerido;

    int anchoDialogo = 415;
    int altoDialogo = 160;
    RECT rectanguloPadre{};
    HWND ventanaFoco = GetForegroundWindow();
    if (ventanaFoco && IsWindow(ventanaFoco)) {
        GetWindowRect(ventanaFoco, &rectanguloPadre);
    } else if (ventanaPadre && IsWindow(ventanaPadre)) {
        GetWindowRect(ventanaPadre, &rectanguloPadre);
    } else {
        rectanguloPadre = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    }
    int posicionX = rectanguloPadre.left + (rectanguloPadre.right - rectanguloPadre.left - anchoDialogo) / 2;
    int posicionY = rectanguloPadre.top + (rectanguloPadre.bottom - rectanguloPadre.top - altoDialogo) / 2;

    HWND ventanaDialogo = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_DLGMODALFRAME,
        L"UsefulSideviewNombreCarpetaDlg",
        L"Useful Sideview - Elegir Nombre de Carpeta Final",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        posicionX, posicionY, anchoDialogo, altoDialogo,
        nullptr, nullptr, GetModuleHandleW(nullptr), &contexto
    );

    if (!ventanaDialogo) return false;

    SetForegroundWindow(ventanaDialogo);
    if (contexto.controlTextoEdicion) {
        SetFocus(contexto.controlTextoEdicion);
    }

    MSG mensajeBucle{};
    while (!contexto.terminado && IsWindow(ventanaDialogo)) {
        while (PeekMessageW(&mensajeBucle, nullptr, 0, 0, PM_REMOVE)) {
            if (mensajeBucle.message == WM_QUIT) {
                contexto.terminado = true;
                break;
            }
            if (!IsDialogMessageW(ventanaDialogo, &mensajeBucle)) {
                TranslateMessage(&mensajeBucle);
                DispatchMessageW(&mensajeBucle);
            }
        }
        if (!contexto.terminado && IsWindow(ventanaDialogo)) {
            MsgWaitForMultipleObjects(0, nullptr, FALSE, 10, QS_ALLINPUT);
        }
    }

    if (contexto.aceptado) {
        nombreElegido = contexto.nombreResultado;
        return true;
    }
    return false;
}

inline bool ejecutarFusionCarpetas(HWND ventanaPadre) {
    if (estadoVistaCarpetas.carpetasSeleccionadas.size() < 2) {
        return false;
    }

    wchar_t buferPadre[MAX_PATH];
    wcsncpy_s(buferPadre, estadoVistaCarpetas.carpetasSeleccionadas[0].c_str(), _TRUNCATE);
    PathRemoveFileSpecW(buferPadre);
    std::wstring rutaPadre = buferPadre;

    std::wstring nombreSugerido = PathFindFileNameW(estadoVistaCarpetas.carpetasSeleccionadas[0].c_str());
    std::wstring nombreElegido;

    if (!solicitarNombreCarpetaFinal(ventanaPadre, nombreSugerido, nombreElegido)) {
        return false;
    }

    std::wstring rutaDestino = rutaPadre + L"\\" + nombreElegido;
    size_t totalCarpetas = estadoVistaCarpetas.carpetasSeleccionadas.size();

    std::wstring mensajeConfirmacion =
        L"¿Desea fusionar las " + std::to_wstring(totalCarpetas) +
        L" carpetas seleccionadas en la carpeta de destino:\n📁 " +
        nombreElegido + L"\n(" + rutaDestino + L")?\n\n"
        L"Todos los archivos y subcarpetas se consolidarán en esta carpeta final.\n"
        L"Las carpetas de origen fusionadas se eliminarán tras mover su contenido.";

    int respuestaUsuario = MessageBoxW(
        nullptr,
        mensajeConfirmacion.c_str(),
        L"Useful Sideview - Confirmar Fusión de Carpetas",
        MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2 | MB_TOPMOST | MB_SETFOREGROUND
    );

    if (respuestaUsuario != IDYES) {
        return false;
    }

    CreateDirectoryW(rutaDestino.c_str(), nullptr);

    int carpetasFusionadasExitosamente = 0;
    for (size_t posicionCarpeta = 0; posicionCarpeta < estadoVistaCarpetas.carpetasSeleccionadas.size(); ++posicionCarpeta) {
        const std::wstring& rutaOrigen = estadoVistaCarpetas.carpetasSeleccionadas[posicionCarpeta];
        if (rutaOrigen != rutaDestino && PathFileExistsW(rutaOrigen.c_str())) {
            if (fusionarDirectorioRecursivo(rutaOrigen, rutaDestino)) {
                carpetasFusionadasExitosamente++;
            }
        }
    }

    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATHW, rutaPadre.c_str(), nullptr);
    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATHW, rutaDestino.c_str(), nullptr);

    estadoVistaCarpetas.carpetasSeleccionadas = { rutaDestino };
    estadoVistaCarpetas.rutaCarpetaArbolCargada.clear();
    estadoVistaCarpetas.indiceCarpetaActual = 0;
    cargarCarpetaPorIndice(0);

    std::wstring mensajeExito =
        L"✓ Fusión completada exitosamente. Se fusionaron " +
        std::to_wstring(carpetasFusionadasExitosamente) +
        L" carpetas en la carpeta final:\n📁 " + nombreElegido;

    MessageBoxW(
        nullptr,
        mensajeExito.c_str(),
        L"Useful Sideview - Fusión de Carpetas Exitosa",
        MB_ICONINFORMATION | MB_OK | MB_TOPMOST | MB_SETFOREGROUND
    );

    return true;
}

inline const ElementoArbolVistaCarpetas* obtenerElementoArbolEnPunto(POINT puntoRaton) {
    if (estadoVistaCarpetas.elementosArbol.empty()) return nullptr;
    if (!PtInRect(&estadoVistaCarpetas.areaCajaArbol, puntoRaton)) return nullptr;

    int yInicioArbol = estadoVistaCarpetas.areaCajaArbol.top + 8 - estadoVistaCarpetas.desplazamientoScrollY;
    constexpr int ALTO_FILA_ARBOL = 22;

    if (puntoRaton.y < yInicioArbol) return nullptr;

    int indiceElementoCalculado = (puntoRaton.y - yInicioArbol) / ALTO_FILA_ARBOL;
    if (indiceElementoCalculado >= 0 &&
        static_cast<size_t>(indiceElementoCalculado) < estadoVistaCarpetas.elementosArbol.size()) {
        return &estadoVistaCarpetas.elementosArbol[static_cast<size_t>(indiceElementoCalculado)];
    }
    return nullptr;
}

inline void ejecutarAbrirElementoArbol(const ElementoArbolVistaCarpetas& elemento) {
    if (elemento.rutaCompleta.empty()) return;
    HINSTANCE resultado = ShellExecuteW(nullptr, nullptr, elemento.rutaCompleta.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(resultado) <= 32) {
        ShellExecuteW(nullptr, L"open", elemento.rutaCompleta.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
}

inline void ejecutarRenombrarElementoArbol(HWND ventanaPadre, const ElementoArbolVistaCarpetas& elemento) {
    std::wstring rutaDirectorioPadre;
    wchar_t buferDirectorio[MAX_PATH] = L"";
    wcscpy_s(buferDirectorio, elemento.rutaCompleta.c_str());
    PathRemoveFileSpecW(buferDirectorio);
    rutaDirectorioPadre = buferDirectorio;

    wchar_t buferNuevoNombre[MAX_PATH] = L"";
    OPENFILENAMEW estructuraOfn{};
    estructuraOfn.lStructSize = sizeof(estructuraOfn);
    estructuraOfn.hwndOwner = ventanaPadre;
    estructuraOfn.lpstrInitialDir = rutaDirectorioPadre.c_str();
    wcscpy_s(buferNuevoNombre, elemento.nombre.c_str());
    estructuraOfn.lpstrFile = buferNuevoNombre;
    estructuraOfn.nMaxFile = MAX_PATH;
    estructuraOfn.lpstrTitle = L"Escriba o elija el nuevo nombre para el archivo";
    estructuraOfn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&estructuraOfn)) {
        std::wstring rutaNueva(buferNuevoNombre);
        if (!rutaNueva.empty() && rutaNueva != elemento.rutaCompleta) {
            if (MoveFileExW(elemento.rutaCompleta.c_str(), rutaNueva.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) {
                cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
                InvalidateRect(ventanaPadre, nullptr, FALSE);
            } else {
                DWORD errorRenombrado = GetLastError();
                MessageBoxW(ventanaPadre, (L"No se pudo renombrar el elemento (código de error: " + std::to_wstring(errorRenombrado) + L").").c_str(),
                            L"Error al renombrar", MB_OK | MB_ICONERROR);
            }
        }
    }
}

inline void ejecutarEliminarElementoArbol(HWND ventanaPadre, const ElementoArbolVistaCarpetas& elemento) {
    std::wstring mensajeConfirmacion = L"¿Desea enviar '" + elemento.nombre + L"' a la Papelera de reciclaje?";
    int respuestaUsuario = MessageBoxW(ventanaPadre, mensajeConfirmacion.c_str(), L"Confirmar eliminación",
                                       MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
    if (respuestaUsuario != IDYES) return;

    wchar_t buferDobleNulo[MAX_PATH + 2] = L"";
    wcsncpy_s(buferDobleNulo, elemento.rutaCompleta.c_str(), MAX_PATH);
    buferDobleNulo[wcslen(elemento.rutaCompleta.c_str()) + 1] = L'\0';

    SHFILEOPSTRUCTW operacionArchivo{};
    operacionArchivo.hwnd = ventanaPadre;
    operacionArchivo.wFunc = FO_DELETE;
    operacionArchivo.pFrom = buferDobleNulo;
    operacionArchivo.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

    if (SHFileOperationW(&operacionArchivo) == 0 && !operacionArchivo.fAnyOperationsAborted) {
        cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
        InvalidateRect(ventanaPadre, nullptr, FALSE);
    }
}

constexpr UINT ID_MENU_ARBOL_ABRIR = 5101;
constexpr UINT ID_MENU_ARBOL_RENOMBRAR = 5102;
constexpr UINT ID_MENU_ARBOL_ELIMINAR = 5103;

inline void mostrarMenuContextualElementoArbol(HWND ventanaPadre, const ElementoArbolVistaCarpetas& elemento, POINT puntoPantalla) {
    HMENU menuEmergente = CreatePopupMenu();
    if (!menuEmergente) return;

    AppendMenuW(menuEmergente, MF_STRING, ID_MENU_ARBOL_ABRIR, L"Abrir");
    AppendMenuW(menuEmergente, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menuEmergente, MF_STRING, ID_MENU_ARBOL_RENOMBRAR, L"Renombrar...");
    AppendMenuW(menuEmergente, MF_STRING, ID_MENU_ARBOL_ELIMINAR, L"Eliminar");

    SetForegroundWindow(ventanaPadre);
    UINT comandoSeleccionado = static_cast<UINT>(
        TrackPopupMenuEx(menuEmergente,
                         TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                         puntoPantalla.x, puntoPantalla.y,
                         ventanaPadre, nullptr));
    DestroyMenu(menuEmergente);

    if (comandoSeleccionado == ID_MENU_ARBOL_ABRIR) {
        ejecutarAbrirElementoArbol(elemento);
    } else if (comandoSeleccionado == ID_MENU_ARBOL_RENOMBRAR) {
        ejecutarRenombrarElementoArbol(ventanaPadre, elemento);
    } else if (comandoSeleccionado == ID_MENU_ARBOL_ELIMINAR) {
        ejecutarEliminarElementoArbol(ventanaPadre, elemento);
    }
}

inline void dibujarLienzoVistaCarpetas(HDC contextoDestino, const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTitulo = CreateFontW(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteItemArbol = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuentePequena = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int yActual = areaDibujo.top + 16;
    bool hayMultiplesCarpetas = estadoVistaCarpetas.carpetasSeleccionadas.size() > 1;

    std::wstring nombreCarpetaActual;
    std::wstring rutaCompletaActual;
    if (estadoVistaCarpetas.indiceCarpetaActual < estadoVistaCarpetas.carpetasSeleccionadas.size()) {
        rutaCompletaActual = estadoVistaCarpetas.carpetasSeleccionadas[estadoVistaCarpetas.indiceCarpetaActual];
        nombreCarpetaActual = PathFindFileNameW(rutaCompletaActual.c_str());
        if (nombreCarpetaActual.empty()) {
            nombreCarpetaActual = rutaCompletaActual;
        }
    }

    int anchoBoton = 32;
    int altoBoton = 26;
    estadoVistaCarpetas.botonAnteriorCarpeta = {
        areaDibujo.right - 88, yActual, areaDibujo.right - 88 + anchoBoton, yActual + altoBoton};
    estadoVistaCarpetas.botonSiguienteCarpeta = {
        areaDibujo.right - 48, yActual, areaDibujo.right - 48 + anchoBoton, yActual + altoBoton};

    auto dibujarBotonNav = [&](const RECT& r, const wchar_t* texto, bool hover, bool habilitado) {
        COLORREF colorFondo = !habilitado ? RGB(22, 27, 34) : (hover ? RGB(48, 54, 61) : RGB(33, 38, 45));
        COLORREF colorBorde = !habilitado ? RGB(33, 38, 45) : (hover ? RGB(110, 118, 129) : RGB(48, 54, 61));
        HBRUSH bBtn = CreateSolidBrush(colorFondo);
        HPEN lBtn = CreatePen(PS_SOLID, 1, colorBorde);
        HBRUSH bPrev = static_cast<HBRUSH>(SelectObject(contextoDestino, bBtn));
        HPEN lPrev = static_cast<HPEN>(SelectObject(contextoDestino, lBtn));
        RoundRect(contextoDestino, r.left, r.top, r.right, r.bottom, 6, 6);
        SelectObject(contextoDestino, lPrev);
        SelectObject(contextoDestino, bPrev);
        DeleteObject(lBtn);
        DeleteObject(bBtn);

        SetTextColor(contextoDestino, !habilitado ? RGB(75, 85, 99) : (hover ? RGB(255, 255, 255) : RGB(201, 209, 217)));
        HFONT fPrev = static_cast<HFONT>(SelectObject(contextoDestino, fuentePequena));
        RECT rTexto = r;
        DrawTextW(contextoDestino, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDestino, fPrev);
    };

    dibujarBotonNav(estadoVistaCarpetas.botonAnteriorCarpeta, L"◀", estadoVistaCarpetas.hoverBotonAnterior, true);
    dibujarBotonNav(estadoVistaCarpetas.botonSiguienteCarpeta, L"▶", estadoVistaCarpetas.hoverBotonSiguiente, true);

    SelectObject(contextoDestino, fuenteTitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaTitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 100, yActual + 22};
    std::wstring tituloVisual = L"📁 " + nombreCarpetaActual;
    if (hayMultiplesCarpetas) {
        tituloVisual += L" (" + std::to_wstring(estadoVistaCarpetas.indiceCarpetaActual + 1) +
                        L"/" + std::to_wstring(estadoVistaCarpetas.carpetasSeleccionadas.size()) + L")";
    }

    SIZE tamanoTextoTitulo{};
    GetTextExtentPoint32W(contextoDestino, tituloVisual.c_str(), static_cast<int>(tituloVisual.size()), &tamanoTextoTitulo);

    RECT areaTextoTitulo = areaTitulo;
    areaTextoTitulo.right = std::min<LONG>(areaTitulo.right, areaTitulo.left + tamanoTextoTitulo.cx);
    DrawTextW(contextoDestino, tituloVisual.c_str(), -1, &areaTextoTitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    if (!estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado.empty()) {
        SelectObject(contextoDestino, fuenteItemArbol);
        SetTextColor(contextoDestino, RGB(110, 118, 129));
        RECT areaTextoFantasma{areaTextoTitulo.right + 10, yActual + 2, areaDibujo.right - 100, yActual + 22};
        if (areaTextoFantasma.left < areaTextoFantasma.right) {
            DrawTextW(contextoDestino, estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado.c_str(), -1,
                      &areaTextoFantasma, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
    }

    yActual += 20;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaRutaCompleta{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, rutaCompletaActual.c_str(), -1, &areaRutaCompleta,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    yActual += 16;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaSub{areaDibujo.left + 24, yActual, areaDibujo.right - 130, yActual + 18};
    DrawTextW(contextoDestino, L"ESTRUCTURA DE ARCHIVOS (VISTA DE CARPETAS)", -1, &areaSub,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    int maxNivelJerarquia = obtenerMaximoNivelJerarquiaArbol();
    std::wstring textoBtnExpansion;
    if (estadoVistaCarpetas.nivelExpansionActual == 0) {
        textoBtnExpansion = L"▶ Raíz";
    } else if (estadoVistaCarpetas.nivelExpansionActual >= maxNivelJerarquia && maxNivelJerarquia > 0) {
        textoBtnExpansion = L"🔽 Todo";
    } else {
        textoBtnExpansion = L"▶ Nivel " + std::to_wstring(estadoVistaCarpetas.nivelExpansionActual);
    }

    estadoVistaCarpetas.botonAlternarExpansion = {areaDibujo.right - 120, yActual - 2, areaDibujo.right - 24, yActual + 18};
    dibujarBotonNav(estadoVistaCarpetas.botonAlternarExpansion, textoBtnExpansion.c_str(), estadoVistaCarpetas.hoverBotonAlternarExpansion, true);

    yActual += 20;

    int altoCajaArbol = std::max(80, std::min<int>(150, altoArea - (hayMultiplesCarpetas ? 410 : 300)));
    RECT areaCajaArbol{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + altoCajaArbol};
    estadoVistaCarpetas.areaCajaArbol = areaCajaArbol;

    HBRUSH brochaCaja = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizCaja = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPreviaCaja = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaCaja));
    HPEN lapizPrevioCaja = static_cast<HPEN>(SelectObject(contextoDestino, lapizCaja));
    RoundRect(contextoDestino, areaCajaArbol.left, areaCajaArbol.top, areaCajaArbol.right, areaCajaArbol.bottom, 8, 8);
    SelectObject(contextoDestino, lapizPrevioCaja);
    SelectObject(contextoDestino, brochaPreviaCaja);
    DeleteObject(lapizCaja);
    DeleteObject(brochaCaja);

    int yFilaArbol = areaCajaArbol.top + 8 - estadoVistaCarpetas.desplazamientoScrollY;
    int altoFila = 22;
    int elementosVisiblesContador = 0;

    HRGN regionRecorte = CreateRectRgn(areaCajaArbol.left + 2, areaCajaArbol.top + 2,
                                       areaCajaArbol.right - 2, areaCajaArbol.bottom - 2);
    SelectClipRgn(contextoDestino, regionRecorte);

    if (estadoVistaCarpetas.elementosArbol.empty()) {
        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(139, 148, 158));
        RECT areaVacio = areaCajaArbol;
        DrawTextW(contextoDestino, L"(Carpeta vacia o sin subelementos)", -1, &areaVacio,
                   DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    } else {
        for (const auto& elementoArbol : estadoVistaCarpetas.elementosArbol) {
            if (elementoArbol.nivelJerarquia > estadoVistaCarpetas.nivelExpansionActual) {
                continue;
            }
            elementosVisiblesContador++;
            if (yFilaArbol + altoFila >= areaCajaArbol.top && yFilaArbol <= areaCajaArbol.bottom) {
                int sangria = areaCajaArbol.left + 12 + elementoArbol.nivelJerarquia * 16;
                std::wstring prefijo = elementoArbol.esDirectorio ? L"📁 " : L"📄 ";
                std::wstring linea = prefijo + elementoArbol.nombre;

                SelectObject(contextoDestino, fuenteItemArbol);
                SetTextColor(contextoDestino, elementoArbol.esDirectorio ? RGB(88, 166, 255) : RGB(201, 209, 217));
                RECT areaElem{sangria, yFilaArbol, areaCajaArbol.right - 90, yFilaArbol + altoFila};
                DrawTextW(contextoDestino, linea.c_str(), -1, &areaElem,
                          DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                SelectObject(contextoDestino, fuentePequena);
                SetTextColor(contextoDestino, elementoArbol.esDirectorio ? RGB(88, 166, 255) : RGB(110, 118, 129));
                std::wstring tamanoTexto = formatearTamanoCarpeta(elementoArbol.tamanoBytes);
                RECT areaTamanoElem{areaCajaArbol.right - 88, yFilaArbol, areaCajaArbol.right - 12, yFilaArbol + altoFila};
                DrawTextW(contextoDestino, tamanoTexto.c_str(), -1, &areaTamanoElem,
                          DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }
            yFilaArbol += altoFila;
        }
    }
    SelectClipRgn(contextoDestino, nullptr);
    DeleteObject(regionRecorte);

    estadoVistaCarpetas.altoTotalArbol = elementosVisiblesContador * altoFila + 16;

    yActual += altoCajaArbol + 16;

    HPEN lapizSep = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSep = static_cast<HPEN>(SelectObject(contextoDestino, lapizSep));
    MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
    LineTo(contextoDestino, areaDibujo.right - 24, yActual);
    SelectObject(contextoDestino, lapizPrevSep);
    DeleteObject(lapizSep);

    yActual += 14;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaSubAbrir{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, L"ABRIR CARPETA CON", -1, &areaSubAbrir,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 20;

    std::wstring textoProgramaActual = L"Seleccionar programa...";
    std::wstring rutaResuelta = resolverProgramaParaCarpetas();
    if (!rutaResuelta.empty()) {
        textoProgramaActual = PathFindFileNameW(rutaResuelta.c_str());
    }

    int anchoSelector = areaDibujo.right - areaDibujo.left - 48 - 100;
    estadoVistaCarpetas.botonSelectorPrograma = {areaDibujo.left + 24, yActual, areaDibujo.left + 24 + anchoSelector, yActual + 30};
    estadoVistaCarpetas.botonEjecutarAbrir = {areaDibujo.right - 24 - 90, yActual, areaDibujo.right - 24, yActual + 30};

    bool selectorHover = (estadoVistaCarpetas.opcionProgramaHover == 9999);
    HBRUSH brochaSelector = CreateSolidBrush(selectorHover ? RGB(33, 38, 45) : RGB(22, 27, 34));
    HPEN lapizSelector = CreatePen(PS_SOLID, 1, selectorHover ? RGB(88, 166, 255) : RGB(48, 54, 61));
    HBRUSH brochaPreviaSelector = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaSelector));
    HPEN lapizPrevioSelector = static_cast<HPEN>(SelectObject(contextoDestino, lapizSelector));
    RoundRect(contextoDestino, estadoVistaCarpetas.botonSelectorPrograma.left,
              estadoVistaCarpetas.botonSelectorPrograma.top,
              estadoVistaCarpetas.botonSelectorPrograma.right,
              estadoVistaCarpetas.botonSelectorPrograma.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioSelector);
    SelectObject(contextoDestino, brochaPreviaSelector);
    DeleteObject(lapizSelector);
    DeleteObject(brochaSelector);

    SelectObject(contextoDestino, fuenteItemArbol);
    SetTextColor(contextoDestino, rutaResuelta.empty() ? RGB(110, 118, 129) : RGB(240, 246, 252));
    RECT areaTextoPrograma = estadoVistaCarpetas.botonSelectorPrograma;
    areaTextoPrograma.left += 10;
    areaTextoPrograma.right -= 24;
    DrawTextW(contextoDestino, textoProgramaActual.c_str(), -1, &areaTextoPrograma,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaFlecha{estadoVistaCarpetas.botonSelectorPrograma.right - 20,
                    estadoVistaCarpetas.botonSelectorPrograma.top,
                    estadoVistaCarpetas.botonSelectorPrograma.right - 6,
                    estadoVistaCarpetas.botonSelectorPrograma.bottom};
    DrawTextW(contextoDestino, estadoVistaCarpetas.menuProgramasAbierto ? L"▲" : L"▼", -1,
              &areaFlecha, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    bool ejecutarHover = (estadoVistaCarpetas.opcionProgramaHover == 9998);
    HBRUSH brochaEjecutar = CreateSolidBrush(ejecutarHover ? RGB(46, 160, 67) : RGB(35, 134, 54));
    HPEN lapizEjecutar = CreatePen(PS_SOLID, 1, RGB(46, 160, 67));
    HBRUSH brochaPreviaEjecutar = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaEjecutar));
    HPEN lapizPrevioEjecutar = static_cast<HPEN>(SelectObject(contextoDestino, lapizEjecutar));
    RoundRect(contextoDestino, estadoVistaCarpetas.botonEjecutarAbrir.left,
              estadoVistaCarpetas.botonEjecutarAbrir.top,
              estadoVistaCarpetas.botonEjecutarAbrir.right,
              estadoVistaCarpetas.botonEjecutarAbrir.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioEjecutar);
    SelectObject(contextoDestino, brochaPreviaEjecutar);
    DeleteObject(lapizEjecutar);
    DeleteObject(brochaEjecutar);

    SelectObject(contextoDestino, fuenteItemArbol);
    SetTextColor(contextoDestino, RGB(255, 255, 255));
    RECT areaTextoEjecutar = estadoVistaCarpetas.botonEjecutarAbrir;
    DrawTextW(contextoDestino, L"Abrir", -1, &areaTextoEjecutar,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 42;

    HPEN lapizSepAcciones = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSepAcciones = static_cast<HPEN>(SelectObject(contextoDestino, lapizSepAcciones));
    MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
    LineTo(contextoDestino, areaDibujo.right - 24, yActual);
    SelectObject(contextoDestino, lapizPrevSepAcciones);
    DeleteObject(lapizSepAcciones);

    yActual += 12;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaSubAcciones{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, L"ACCIONES RÁPIDAS", -1, &areaSubAcciones,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 20;

    int anchoContenedor = areaDibujo.right - areaDibujo.left - 48;
    int altoContenedorAcciones = 42;
    estadoVistaCarpetas.areaContenedorAccionesRapidas = {
        areaDibujo.left + 24, yActual,
        areaDibujo.left + 24 + anchoContenedor,
        yActual + altoContenedorAcciones
    };

    HBRUSH brochaContenedor = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizContenedor = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPreviaContenedor = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaContenedor));
    HPEN lapizPrevioContenedor = static_cast<HPEN>(SelectObject(contextoDestino, lapizContenedor));
    RoundRect(contextoDestino, estadoVistaCarpetas.areaContenedorAccionesRapidas.left,
              estadoVistaCarpetas.areaContenedorAccionesRapidas.top,
              estadoVistaCarpetas.areaContenedorAccionesRapidas.right,
              estadoVistaCarpetas.areaContenedorAccionesRapidas.bottom, 8, 8);
    SelectObject(contextoDestino, lapizPrevioContenedor);
    SelectObject(contextoDestino, brochaPreviaContenedor);
    DeleteObject(lapizContenedor);
    DeleteObject(brochaContenedor);

    int paddingInterno = 5;
    int anchoBotonAccion = (anchoContenedor - (paddingInterno * 3)) / 2;
    int yBotonesAccion = yActual + paddingInterno;
    int altoBotonAccion = altoContenedorAcciones - (paddingInterno * 2);

    estadoVistaCarpetas.botonCrearEnlaceSimbolico = {
        estadoVistaCarpetas.areaContenedorAccionesRapidas.left + paddingInterno,
        yBotonesAccion,
        estadoVistaCarpetas.areaContenedorAccionesRapidas.left + paddingInterno + anchoBotonAccion,
        yBotonesAccion + altoBotonAccion
    };

    estadoVistaCarpetas.botonCopiarContenidoPortapapeles = {
        estadoVistaCarpetas.botonCrearEnlaceSimbolico.right + paddingInterno,
        yBotonesAccion,
        estadoVistaCarpetas.botonCrearEnlaceSimbolico.right + paddingInterno + anchoBotonAccion,
        yBotonesAccion + altoBotonAccion
    };

    // 1. Botón Enlace Simbólico
    bool hoverEnlace = estadoVistaCarpetas.hoverBotonCrearEnlace;
    HBRUSH brochaEnlace = CreateSolidBrush(hoverEnlace ? RGB(33, 38, 45) : RGB(13, 17, 23));
    HPEN lapizEnlace = CreatePen(PS_SOLID, 1, hoverEnlace ? RGB(88, 166, 255) : RGB(48, 54, 61));
    HBRUSH brochaPreviaEnlace = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaEnlace));
    HPEN lapizPrevioEnlace = static_cast<HPEN>(SelectObject(contextoDestino, lapizEnlace));
    RoundRect(contextoDestino, estadoVistaCarpetas.botonCrearEnlaceSimbolico.left,
              estadoVistaCarpetas.botonCrearEnlaceSimbolico.top,
              estadoVistaCarpetas.botonCrearEnlaceSimbolico.right,
              estadoVistaCarpetas.botonCrearEnlaceSimbolico.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioEnlace);
    SelectObject(contextoDestino, brochaPreviaEnlace);
    DeleteObject(lapizEnlace);
    DeleteObject(brochaEnlace);

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, hoverEnlace ? RGB(255, 255, 255) : RGB(201, 209, 217));
    RECT areaTextoEnlace = estadoVistaCarpetas.botonCrearEnlaceSimbolico;
    DrawTextW(contextoDestino, L"🔗 Crear enlace", -1, &areaTextoEnlace,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // 2. Botón Copiar Contenido
    bool hoverCopiar = estadoVistaCarpetas.hoverBotonCopiarContenido;
    HBRUSH brochaCopiar = CreateSolidBrush(hoverCopiar ? RGB(33, 38, 45) : RGB(13, 17, 23));
    HPEN lapizCopiar = CreatePen(PS_SOLID, 1, hoverCopiar ? RGB(88, 166, 255) : RGB(48, 54, 61));
    HBRUSH brochaPreviaCopiar = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaCopiar));
    HPEN lapizPrevioCopiar = static_cast<HPEN>(SelectObject(contextoDestino, lapizCopiar));
    RoundRect(contextoDestino, estadoVistaCarpetas.botonCopiarContenidoPortapapeles.left,
              estadoVistaCarpetas.botonCopiarContenidoPortapapeles.top,
              estadoVistaCarpetas.botonCopiarContenidoPortapapeles.right,
              estadoVistaCarpetas.botonCopiarContenidoPortapapeles.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioCopiar);
    SelectObject(contextoDestino, brochaPreviaCopiar);
    DeleteObject(lapizCopiar);
    DeleteObject(brochaCopiar);

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, hoverCopiar ? RGB(255, 255, 255) : RGB(201, 209, 217));
    RECT areaTextoCopiar = estadoVistaCarpetas.botonCopiarContenidoPortapapeles;
    DrawTextW(contextoDestino, L"📋 Copiar contenido", -1, &areaTextoCopiar,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    if (hayMultiplesCarpetas) {
        yActual += 42;

        HPEN lapizSepOp = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
        HPEN lapizPrevSepOp = static_cast<HPEN>(SelectObject(contextoDestino, lapizSepOp));
        MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
        LineTo(contextoDestino, areaDibujo.right - 24, yActual);
        SelectObject(contextoDestino, lapizPrevSepOp);
        DeleteObject(lapizSepOp);

        yActual += 12;

        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(139, 148, 158));
        RECT areaSubOperaciones{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
        DrawTextW(contextoDestino, L"OPERACIONES CON CARPETA", -1, &areaSubOperaciones,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yActual += 20;

        estadoVistaCarpetas.botonFusionCarpetas = {areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 32};
        bool hoverFusion = estadoVistaCarpetas.hoverBotonFusion;
        HBRUSH brochaFusion = CreateSolidBrush(hoverFusion ? RGB(48, 54, 61) : RGB(33, 38, 45));
        HPEN lapizFusion = CreatePen(PS_SOLID, 1, hoverFusion ? RGB(88, 166, 255) : RGB(48, 54, 61));
        HBRUSH brochaPreviaFusion = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaFusion));
        HPEN lapizPrevioFusion = static_cast<HPEN>(SelectObject(contextoDestino, lapizFusion));
        RoundRect(contextoDestino, estadoVistaCarpetas.botonFusionCarpetas.left,
                  estadoVistaCarpetas.botonFusionCarpetas.top,
                  estadoVistaCarpetas.botonFusionCarpetas.right,
                  estadoVistaCarpetas.botonFusionCarpetas.bottom, 6, 6);
        SelectObject(contextoDestino, lapizPrevioFusion);
        SelectObject(contextoDestino, brochaPreviaFusion);
        DeleteObject(lapizFusion);
        DeleteObject(brochaFusion);

        SelectObject(contextoDestino, fuenteItemArbol);
        SetTextColor(contextoDestino, hoverFusion ? RGB(255, 255, 255) : RGB(230, 237, 243));
        RECT areaTextoFusion = estadoVistaCarpetas.botonFusionCarpetas;
        DrawTextW(contextoDestino, L"🔀 Fusión de carpetas", -1, &areaTextoFusion,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    } else {
        estadoVistaCarpetas.botonFusionCarpetas = {};
    }

    DeleteObject(fuenteTitulo);
    DeleteObject(fuenteItemArbol);
    DeleteObject(fuentePequena);
}

inline void dibujarLienzoVistaVacia(HDC contextoDestino, const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTituloGrande = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubtitulo = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteItemVistaVacia = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuentePequena = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int yActual = areaDibujo.top + 20;

    if (estadoVistaCarpetas.resumenVistaVacia.esUbicacionSistema) {
        SelectObject(contextoDestino, fuenteTituloGrande);
        SetTextColor(contextoDestino, RGB(255, 255, 255));
        RECT areaTituloSistema{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 28};
        DrawTextW(contextoDestino, L"Useful Sideview", -1, &areaTituloSistema,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yActual += 32;

        SelectObject(contextoDestino, fuenteSubtitulo);
        SetTextColor(contextoDestino, RGB(88, 166, 255));
        RECT areaNombreSeccion{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 22};
        std::wstring etiquetaSeccion = L"📁 " + estadoVistaCarpetas.resumenVistaVacia.nombreUbicacionSistema;
        DrawTextW(contextoDestino, etiquetaSeccion.c_str(), -1, &areaNombreSeccion,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        yActual += 30;

        int altoTarjetaInfo = 110;
        RECT areaTarjetaInfo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + altoTarjetaInfo};
        HBRUSH brochaInfo = CreateSolidBrush(RGB(22, 27, 34));
        HPEN lapizInfo = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
        HBRUSH brochaPrevInfo = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaInfo));
        HPEN lapizPrevInfo = static_cast<HPEN>(SelectObject(contextoDestino, lapizInfo));
        RoundRect(contextoDestino, areaTarjetaInfo.left, areaTarjetaInfo.top,
                  areaTarjetaInfo.right, areaTarjetaInfo.bottom, 8, 8);
        SelectObject(contextoDestino, lapizPrevInfo);
        SelectObject(contextoDestino, brochaPrevInfo);
        DeleteObject(lapizInfo);
        DeleteObject(brochaInfo);

        SelectObject(contextoDestino, fuenteItemVistaVacia);
        SetTextColor(contextoDestino, RGB(201, 209, 217));
        RECT areaTextoInfo{areaTarjetaInfo.left + 16, areaTarjetaInfo.top + 14,
                           areaTarjetaInfo.right - 16, areaTarjetaInfo.bottom - 14};
        DrawTextW(contextoDestino,
                  estadoVistaCarpetas.resumenVistaVacia.mensajeUbicacionSistema.c_str(),
                  -1, &areaTextoInfo,
                  DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

        DeleteObject(fuenteTituloGrande);
        DeleteObject(fuenteSubtitulo);
        DeleteObject(fuenteItemVistaVacia);
        DeleteObject(fuentePequena);
        return;
    }

    SelectObject(contextoDestino, fuenteTituloGrande);
    SetTextColor(contextoDestino, RGB(88, 166, 255));
    RECT areaTitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 28};
    DrawTextW(contextoDestino, L"Useful Sideview", -1, &areaTitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 30;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaDesc{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, L"VISTA VACÍA: RESUMEN DE LA CARPETA ACTUAL", -1, &areaDesc,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 22;

    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaNombreCarpeta{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 20};
    std::wstring txtNombre = L"📁 " + (estadoVistaCarpetas.resumenVistaVacia.nombreCarpeta.empty()
                                         ? estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta
                                         : estadoVistaCarpetas.resumenVistaVacia.nombreCarpeta);
    DrawTextW(contextoDestino, txtNombre.c_str(), -1, &areaNombreCarpeta,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    yActual += 20;

    SelectObject(contextoDestino, fuentePequena);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaRutaVistaVacia{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 16};
    DrawTextW(contextoDestino, estadoVistaCarpetas.resumenVistaVacia.rutaCarpeta.c_str(), -1, &areaRutaVistaVacia,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    yActual += 24;

    int anchoTarjeta = (areaDibujo.right - areaDibujo.left - 48 - 12) / 2;
    int altoTarjeta = 66;

    auto dibujarTarjetaMetrica = [&](int posicionHorizontal, int posicionVertical, const wchar_t* tituloMetrica,
                                     const std::wstring& textoValor, COLORREF colorValor,
                                     const std::wstring& textoFantasma = L"") {
        RECT rectanguloTarjeta{posicionHorizontal, posicionVertical, posicionHorizontal + anchoTarjeta, posicionVertical + altoTarjeta};
        HBRUSH brochaTarjeta = CreateSolidBrush(RGB(22, 27, 34));
        HPEN lapizTarjeta = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
        HBRUSH brochaPrevia = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaTarjeta));
        HPEN lapizPrevio = static_cast<HPEN>(SelectObject(contextoDestino, lapizTarjeta));
        RoundRect(contextoDestino, rectanguloTarjeta.left, rectanguloTarjeta.top, rectanguloTarjeta.right, rectanguloTarjeta.bottom, 6, 6);
        SelectObject(contextoDestino, lapizPrevio);
        SelectObject(contextoDestino, brochaPrevia);
        DeleteObject(lapizTarjeta);
        DeleteObject(brochaTarjeta);

        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(139, 148, 158));
        RECT rectanguloTitulo{posicionHorizontal + 10, posicionVertical + 6, posicionHorizontal + anchoTarjeta - 10, posicionVertical + 19};
        DrawTextW(contextoDestino, tituloMetrica, -1, &rectanguloTitulo, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(contextoDestino, fuenteSubtitulo);
        SetTextColor(contextoDestino, colorValor);
        RECT rectanguloValor{posicionHorizontal + 10, posicionVertical + 21, posicionHorizontal + anchoTarjeta - 10, posicionVertical + 41};
        DrawTextW(contextoDestino, textoValor.c_str(), -1, &rectanguloValor, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        if (!textoFantasma.empty()) {
            SelectObject(contextoDestino, fuentePequena);
            SetTextColor(contextoDestino, RGB(110, 118, 129));
            RECT rectanguloFantasma{posicionHorizontal + 10, posicionVertical + 43, posicionHorizontal + anchoTarjeta - 10, posicionVertical + 59};
            DrawTextW(contextoDestino, textoFantasma.c_str(), -1, &rectanguloFantasma,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
    };

    // 1. Peso total
    dibujarTarjetaMetrica(areaDibujo.left + 24, yActual, L"Peso total",
                          formatearTamanoCarpeta(estadoVistaCarpetas.resumenVistaVacia.tamanoTotalBytes),
                          RGB(88, 166, 255));
    // 2. Cantidad de carpetas
    dibujarTarjetaMetrica(areaDibujo.left + 24 + anchoTarjeta + 12, yActual, L"Subcarpetas",
                          std::to_wstring(estadoVistaCarpetas.resumenVistaVacia.totalSubcarpetas) + L" carpetas",
                          RGB(63, 185, 80));

    yActual += altoTarjeta + 10;

    // 3. Tipos de archivo (con texto fantasma discreto de las extensiones)
    dibujarTarjetaMetrica(areaDibujo.left + 24, yActual, L"Tipos de archivos",
                          std::to_wstring(estadoVistaCarpetas.resumenVistaVacia.cantidadTiposArchivos) + L" extensiones",
                          RGB(210, 153, 34),
                          estadoVistaCarpetas.resumenVistaVacia.textoExtensionesFantasma);
    // 4. Ultimo acceso
    dibujarTarjetaMetrica(areaDibujo.left + 24 + anchoTarjeta + 12, yActual, L"Último acceso",
                          estadoVistaCarpetas.resumenVistaVacia.fechaUltimoAcceso.empty() ? L"N/D" : estadoVistaCarpetas.resumenVistaVacia.fechaUltimoAcceso,
                          RGB(163, 113, 247));

    yActual += altoTarjeta + 22;

    HPEN lapizSep = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSep = static_cast<HPEN>(SelectObject(contextoDestino, lapizSep));
    MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
    LineTo(contextoDestino, areaDibujo.right - 24, yActual);
    SelectObject(contextoDestino, lapizPrevSep);
    DeleteObject(lapizSep);

    yActual += 16;

    // 5. Top 4 subcarpetas mas pesadas
    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaTopTit{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 20};
    DrawTextW(contextoDestino, L"Top 4 Subcarpetas más pesadas", -1, &areaTopTit,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 26;

    if (estadoVistaCarpetas.resumenVistaVacia.top4CarpetasMasPesadas.empty()) {
        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(139, 148, 158));
        RECT areaSinSubcarpetas{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 30};
        DrawTextW(contextoDestino, L"(No se encontraron subcarpetas en el directorio actual)", -1, &areaSinSubcarpetas,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    } else {
        int umbralBajo = configuracion.umbralPorcentajeBajoMedioVistaVacia.load();
        int umbralAlto = configuracion.umbralPorcentajeMedioAltoVistaVacia.load();
        COLORREF colorBajo = configuracion.colorRangoBajoVistaVacia.load();
        COLORREF colorMedio = configuracion.colorRangoMedioVistaVacia.load();
        COLORREF colorAlto = configuracion.colorRangoAltoVistaVacia.load();

        for (const auto& subcarpeta : estadoVistaCarpetas.resumenVistaVacia.top4CarpetasMasPesadas) {
            SelectObject(contextoDestino, fuenteItemVistaVacia);
            SetTextColor(contextoDestino, RGB(201, 209, 217));
            RECT areaNombreSubcarpeta{areaDibujo.left + 24, yActual, areaDibujo.right - 140, yActual + 18};
            std::wstring nombreLimpio = L"📁 " + subcarpeta.nombre;
            DrawTextW(contextoDestino, nombreLimpio.c_str(), -1, &areaNombreSubcarpeta,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

            SelectObject(contextoDestino, fuentePequena);
            SetTextColor(contextoDestino, RGB(139, 148, 158));
            double porcentajeSubcarpeta = subcarpeta.porcentaje * 100.0;
            wchar_t buferTamanoPorcentaje[64];
            swprintf_s(buferTamanoPorcentaje, L"%ls (%.1f%%)",
                       formatearTamanoCarpeta(subcarpeta.tamanoBytes).c_str(),
                       porcentajeSubcarpeta);
            RECT areaTamanoSubcarpeta{areaDibujo.right - 136, yActual, areaDibujo.right - 24, yActual + 18};
            DrawTextW(contextoDestino, buferTamanoPorcentaje, -1, &areaTamanoSubcarpeta,
                      DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            yActual += 20;

            int anchoBarraTotal = areaDibujo.right - areaDibujo.left - 48;
            int anchoBarraProgreso = std::max(4, static_cast<int>(static_cast<double>(anchoBarraTotal) * subcarpeta.porcentaje));

            RECT areaFondoBarra{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 6};
            HBRUSH brochaFondoBarra = CreateSolidBrush(RGB(33, 38, 45));
            FillRect(contextoDestino, &areaFondoBarra, brochaFondoBarra);
            DeleteObject(brochaFondoBarra);

            COLORREF colorBarra;
            if (porcentajeSubcarpeta < static_cast<double>(umbralBajo)) {
                colorBarra = colorBajo;
            } else if (porcentajeSubcarpeta < static_cast<double>(umbralAlto)) {
                colorBarra = colorMedio;
            } else {
                colorBarra = colorAlto;
            }

            RECT areaRellenoBarra{areaDibujo.left + 24, yActual, areaDibujo.left + 24 + anchoBarraProgreso, yActual + 6};
            HBRUSH brochaRellenoBarra = CreateSolidBrush(colorBarra);
            FillRect(contextoDestino, &areaRellenoBarra, brochaRellenoBarra);
            DeleteObject(brochaRellenoBarra);

            yActual += 14;
        }
    }

    yActual += 8;
    HPEN lapizSep2 = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSep2 = static_cast<HPEN>(SelectObject(contextoDestino, lapizSep2));
    MoveToEx(contextoDestino, areaDibujo.left + 24, yActual, nullptr);
    LineTo(contextoDestino, areaDibujo.right - 24, yActual);
    SelectObject(contextoDestino, lapizPrevSep2);
    DeleteObject(lapizSep2);

    yActual += 14;

    // 6. Sección de Carpetas Vacías
    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaVaciasTit{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 20};
    int totalVacias = static_cast<int>(estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.size());
    std::wstring tituloVacias = L"📁 Carpetas Vacías (" + std::to_wstring(totalVacias) + L")";
    DrawTextW(contextoDestino, tituloVacias.c_str(), -1, &areaVaciasTit,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 24;

    if (totalVacias == 0) {
        SelectObject(contextoDestino, fuentePequena);
        SetTextColor(contextoDestino, RGB(63, 185, 80));
        RECT areaSinVacias{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 22};
        std::wstring mensajeTextoVacias = estadoVistaCarpetas.resumenVistaVacia.mensajeResultadoEliminacion.empty()
            ? L"✓ No se detectaron carpetas vacías en este directorio."
            : estadoVistaCarpetas.resumenVistaVacia.mensajeResultadoEliminacion;
        DrawTextW(contextoDestino, mensajeTextoVacias.c_str(), -1, &areaSinVacias,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias = {};
    } else {
        size_t limiteCarpetasMostrar = std::min<size_t>(3, estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.size());
        for (size_t posicionCarpetaVacia = 0; posicionCarpetaVacia < limiteCarpetasMostrar; ++posicionCarpetaVacia) {
            SelectObject(contextoDestino, fuenteItemVistaVacia);
            SetTextColor(contextoDestino, RGB(201, 209, 217));
            RECT areaItemVacia{areaDibujo.left + 28, yActual, areaDibujo.right - 24, yActual + 18};
            std::wstring textoCarpetaVacia = L"• 📁 " + estadoVistaCarpetas.resumenVistaVacia.carpetasVacias[posicionCarpetaVacia];
            DrawTextW(contextoDestino, textoCarpetaVacia.c_str(), -1, &areaItemVacia,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
            yActual += 20;
        }
        if (estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.size() > 3) {
            SelectObject(contextoDestino, fuentePequena);
            SetTextColor(contextoDestino, RGB(139, 148, 158));
            RECT areaMasVacias{areaDibujo.left + 28, yActual, areaDibujo.right - 24, yActual + 16};
            std::wstring textoMasCarpetas = L"+ " + std::to_wstring(estadoVistaCarpetas.resumenVistaVacia.carpetasVacias.size() - 3) + L" carpetas vacías más...";
            DrawTextW(contextoDestino, textoMasCarpetas.c_str(), -1, &areaMasVacias,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            yActual += 18;
        }

        yActual += 6;

        int anchoBoton = std::min<int>(220, static_cast<int>(areaDibujo.right - areaDibujo.left - 48));
        int altoBoton = 30;
        RECT areaBoton{areaDibujo.left + 24, yActual, areaDibujo.left + 24 + anchoBoton, yActual + altoBoton};
        estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias = areaBoton;

        bool hoverBotonEliminar = estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias;
        HBRUSH brochaBotonEliminar = CreateSolidBrush(hoverBotonEliminar ? RGB(218, 54, 51) : RGB(33, 38, 45));
        HPEN lapizBotonEliminar = CreatePen(PS_SOLID, 1, hoverBotonEliminar ? RGB(248, 81, 73) : RGB(248, 81, 73));
        HBRUSH brochaPreviaBoton = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaBotonEliminar));
        HPEN lapizPrevioBoton = static_cast<HPEN>(SelectObject(contextoDestino, lapizBotonEliminar));
        RoundRect(contextoDestino, areaBoton.left, areaBoton.top, areaBoton.right, areaBoton.bottom, 6, 6);
        SelectObject(contextoDestino, lapizPrevioBoton);
        SelectObject(contextoDestino, brochaPreviaBoton);
        DeleteObject(lapizBotonEliminar);
        DeleteObject(brochaBotonEliminar);

        SelectObject(contextoDestino, fuenteSubtitulo);
        SetTextColor(contextoDestino, hoverBotonEliminar ? RGB(255, 255, 255) : RGB(248, 81, 73));
        DrawTextW(contextoDestino, L"🗑️ Eliminar carpetas vacías", -1, &areaBoton,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    DeleteObject(fuenteTituloGrande);
    DeleteObject(fuenteSubtitulo);
    DeleteObject(fuenteItemVistaVacia);
    DeleteObject(fuentePequena);
}
/**** ended inlining carpetas/controlador_carpetas.hpp ****/
/**** start inlining interfaz/diseno_barra_progreso.hpp ****/

struct DisenoBarraProgreso {
    RECT barraControles{};
    RECT areaInteractiva{};
    RECT botonAnterior{};
    RECT botonReproduccion{};
    RECT botonSiguiente{};
    RECT botonSonido{};
    RECT botonRepeticion{};
    RECT botonVelocidad{};
    RECT panelVelocidad{};
    RECT opcionesVelocidad[TOTAL_OPCIONES_VELOCIDAD]{};
    int limiteIzquierdo = 0;
    int limiteDerecho = 0;
    int centroVertical = 0;
};

inline int calcularInicioGrupoBotones(int anchoCliente) {
    return std::max(0, (anchoCliente - ANCHO_GRUPO_BOTONES) / 2);
}

inline double limitarProporcionProgreso(double proporcion) {
    return std::max(0.0, std::min(proporcion, 1.0));
}

inline double calcularProporcionProgreso(
    const DisenoBarraProgreso& disenoBarraProgreso,
    int posicionHorizontal) {
    if (disenoBarraProgreso.limiteDerecho <=
        disenoBarraProgreso.limiteIzquierdo) {
        return 0.0;
    }
    int posicionLimitada = std::max(
        disenoBarraProgreso.limiteIzquierdo,
        std::min(posicionHorizontal, disenoBarraProgreso.limiteDerecho));
    return static_cast<double>(posicionLimitada -
                               disenoBarraProgreso.limiteIzquierdo) /
           static_cast<double>(disenoBarraProgreso.limiteDerecho -
                               disenoBarraProgreso.limiteIzquierdo);
}

inline int calcularPosicionProgreso(
    const DisenoBarraProgreso& disenoBarraProgreso, double proporcion) {
    return disenoBarraProgreso.limiteIzquierdo + static_cast<int>(
        (disenoBarraProgreso.limiteDerecho -
         disenoBarraProgreso.limiteIzquierdo) *
        limitarProporcionProgreso(proporcion));
}

inline bool contienePunto(const RECT& rectangulo, int posicionHorizontal,
                          int posicionVertical) {
    POINT punto{posicionHorizontal, posicionVertical};
    return PtInRect(&rectangulo, punto) != FALSE;
}

inline bool esClicEnBarraProgreso(
    const DisenoBarraProgreso& disenoBarraProgreso, int posicionHorizontal,
    int posicionVertical) {
    if (!contienePunto(disenoBarraProgreso.barraControles, posicionHorizontal,
                       posicionVertical)) {
        return false;
    }
    if (contienePunto(disenoBarraProgreso.botonAnterior, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonReproduccion, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonSiguiente, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonSonido, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonRepeticion, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonVelocidad, posicionHorizontal,
                      posicionVertical)) {
        return false;
    }
    return posicionHorizontal >= disenoBarraProgreso.limiteIzquierdo &&
           posicionHorizontal <= disenoBarraProgreso.limiteDerecho &&
           posicionVertical < disenoBarraProgreso.botonReproduccion.top;
}

inline DisenoBarraProgreso calcularDisenoBarraProgreso(HWND ventana) {
    RECT areaCliente;
    GetClientRect(ventana, &areaCliente);
    DisenoBarraProgreso disenoBarraProgreso;
    disenoBarraProgreso.barraControles = areaCliente;
    disenoBarraProgreso.limiteIzquierdo = MARGEN_BARRA_PROGRESO;
    disenoBarraProgreso.limiteDerecho = static_cast<int>(std::max<LONG>(
        disenoBarraProgreso.limiteIzquierdo,
        areaCliente.right - MARGEN_BARRA_PROGRESO));
    disenoBarraProgreso.centroVertical = 34;
    int bordeSuperiorBotones = 54;
    disenoBarraProgreso.areaInteractiva = {
        disenoBarraProgreso.limiteIzquierdo, 5,
        disenoBarraProgreso.limiteDerecho, bordeSuperiorBotones - 4};

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
        int anchoGrupoBotonesImagen =
            TAMANO_BOTON_CONTROL * 2 + SEPARACION_BOTONES;
        int inicioGrupoBotones = std::max<int>(
            0, (static_cast<int>(areaCliente.right) - anchoGrupoBotonesImagen) / 2);
        disenoBarraProgreso.botonAnterior = {
            inicioGrupoBotones, bordeSuperiorBotones,
            inicioGrupoBotones + TAMANO_BOTON_CONTROL,
            bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
        disenoBarraProgreso.botonSiguiente = {
            disenoBarraProgreso.botonAnterior.right + SEPARACION_BOTONES,
            bordeSuperiorBotones,
            disenoBarraProgreso.botonAnterior.right + SEPARACION_BOTONES +
                TAMANO_BOTON_CONTROL,
            bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
        return disenoBarraProgreso;
    }

    int inicioGrupoBotones = calcularInicioGrupoBotones(areaCliente.right);
    disenoBarraProgreso.botonAnterior = {
        inicioGrupoBotones, bordeSuperiorBotones,
        inicioGrupoBotones + TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonReproduccion = {
        disenoBarraProgreso.botonAnterior.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonAnterior.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonSiguiente = {
        disenoBarraProgreso.botonReproduccion.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonReproduccion.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonSonido = {
        disenoBarraProgreso.botonSiguiente.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonSiguiente.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonRepeticion = {
        disenoBarraProgreso.botonSonido.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonSonido.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};

    int centroVerticalBotones =
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL / 2;
    int izquierdaBotonVelocidad = std::max<int>(
        static_cast<int>(disenoBarraProgreso.botonRepeticion.right) + 8,
        disenoBarraProgreso.limiteDerecho - ANCHO_BOTON_VELOCIDAD);
    disenoBarraProgreso.botonVelocidad = {
        izquierdaBotonVelocidad,
        centroVerticalBotones - ALTO_BOTON_VELOCIDAD / 2,
        izquierdaBotonVelocidad + ANCHO_BOTON_VELOCIDAD,
        centroVerticalBotones + ALTO_BOTON_VELOCIDAD / 2};

    int izquierdaPanelVelocidad =
        disenoBarraProgreso.botonVelocidad.right - ANCHO_PANEL_VELOCIDAD;
    int fondoPanelVelocidad = disenoBarraProgreso.botonVelocidad.top - 2;
    int topePanelVelocidad = fondoPanelVelocidad - ALTO_PANEL_VELOCIDAD;
    disenoBarraProgreso.panelVelocidad = {
        izquierdaPanelVelocidad, topePanelVelocidad,
        disenoBarraProgreso.botonVelocidad.right, fondoPanelVelocidad};

    for (int posicionOpcion = 0; posicionOpcion < TOTAL_OPCIONES_VELOCIDAD;
         ++posicionOpcion) {
        int topeOpcion = topePanelVelocidad + 4 +
                         posicionOpcion * ALTO_OPCION_VELOCIDAD;
        disenoBarraProgreso.opcionesVelocidad[posicionOpcion] = {
            izquierdaPanelVelocidad + 4, topeOpcion,
            disenoBarraProgreso.botonVelocidad.right - 4,
            topeOpcion + ALTO_OPCION_VELOCIDAD};
    }

    return disenoBarraProgreso;
}

struct DisenoComprimidos {
    RECT areaControles{};
    bool tieneComics{false};

    // Cabecera y separador de operaciones
    RECT areaTituloOperaciones{};
    RECT areaSeparadorOperaciones{};

    // Fila Abrir con
    RECT areaEtiquetaAbrirCon{};
    RECT botonDesplegablePrograma{};
    RECT panelOpcionesPrograma{};
    std::vector<RECT> opcionesPrograma{};
    RECT botonEjecutarAbrir{};

    // Fila Convertir a
    RECT areaEtiquetaConversion{};
    RECT botonDesplegableFormato{};
    RECT panelOpcionesFormato{};
    RECT opcionesFormato[TOTAL_FORMATOS_DESTINO]{};
    RECT botonEjecutarConversion{};
};

inline DisenoComprimidos calcularDisenoComprimidos(HWND ventana) {
    RECT areaCliente;
    GetClientRect(ventana, &areaCliente);
    DisenoComprimidos diseno;
    diseno.areaControles = areaCliente;
    diseno.tieneComics = estadoComprimidos.tieneComicsSeleccionados;

    int margenLateral = 14;
    int anchoDisponible = std::max<int>(
        100, static_cast<int>(areaCliente.right) - margenLateral * 2);

    int altoFila = 26;
    int anchoBotonAccion = 82;

    // Sección de operaciones
    int topeTitulo = 6;
    diseno.areaTituloOperaciones = {
        margenLateral, topeTitulo,
        static_cast<LONG>(areaCliente.right) - margenLateral,
        topeTitulo + 18
    };

    int topeSeparador = topeTitulo + 20;
    diseno.areaSeparadorOperaciones = {
        margenLateral, topeSeparador,
        static_cast<LONG>(areaCliente.right) - margenLateral,
        topeSeparador + 1
    };

    // Fila 1: Abrir con
    int topeFilaAbrir = topeSeparador + 8;
    int anchoDesplegableProg = std::min<int>(
        170, std::max<int>(110, (anchoDisponible - anchoBotonAccion - 12) / 2 + 30));
    int anchoEtiquetaAbrir = std::max<int>(
        68, anchoDisponible - anchoDesplegableProg - anchoBotonAccion - 16);

    diseno.areaEtiquetaAbrirCon = {
        margenLateral, topeFilaAbrir,
        margenLateral + anchoEtiquetaAbrir,
        topeFilaAbrir + altoFila};

    diseno.botonDesplegablePrograma = {
        diseno.areaEtiquetaAbrirCon.right + 8, topeFilaAbrir,
        diseno.areaEtiquetaAbrirCon.right + 8 + anchoDesplegableProg,
        topeFilaAbrir + altoFila};

    diseno.botonEjecutarAbrir = {
        diseno.botonDesplegablePrograma.right + 8, topeFilaAbrir,
        static_cast<LONG>(areaCliente.right) - margenLateral,
        topeFilaAbrir + altoFila};

    // Fila 2: Convertir a
    int topeFilaConv = topeFilaAbrir + altoFila + 6;
    int anchoDesplegableForm = std::min<int>(
        130, std::max<int>(90, (anchoDisponible - anchoBotonAccion - 12) / 2));
    int anchoEtiquetaConv = std::max<int>(
        68, anchoDisponible - anchoDesplegableForm - anchoBotonAccion - 16);

    diseno.areaEtiquetaConversion = {
        margenLateral, topeFilaConv,
        margenLateral + anchoEtiquetaConv,
        topeFilaConv + altoFila};

    diseno.botonDesplegableFormato = {
        diseno.areaEtiquetaConversion.right + 8, topeFilaConv,
        diseno.areaEtiquetaConversion.right + 8 + anchoDesplegableForm,
        topeFilaConv + altoFila};

    diseno.botonEjecutarConversion = {
        diseno.botonDesplegableFormato.right + 8, topeFilaConv,
        static_cast<LONG>(areaCliente.right) - margenLateral,
        topeFilaConv + altoFila};

    return diseno;
}
/**** ended inlining interfaz/diseno_barra_progreso.hpp ****/
/**** start inlining reproductor/calculos_tiempo.hpp ****/

inline LONGLONG calcularPosicionTemporal(LONGLONG duracionTotal,
                                         double proporcionObjetivo) {
    if (duracionTotal <= 0) return 0;
    return static_cast<LONGLONG>(
        static_cast<double>(duracionTotal) *
        limitarProporcionProgreso(proporcionObjetivo));
}
/**** ended inlining reproductor/calculos_tiempo.hpp ****/
/**** start inlining reproductor/conversion_tiempo_media_foundation.hpp ****/

#include <propidl.h>
#include <limits>

inline bool extraerTiempoNoNegativoMediaFoundation(
    const PROPVARIANT& propiedadTiempo, LONGLONG* tiempoExtraido) {
    if (!tiempoExtraido) return false;

    ULONGLONG tiempoSinSigno = 0;
    if (propiedadTiempo.vt == VT_UI8) {
        tiempoSinSigno = propiedadTiempo.uhVal.QuadPart;
    } else if (propiedadTiempo.vt == VT_I8 &&
               propiedadTiempo.hVal.QuadPart >= 0) {
        tiempoSinSigno =
            static_cast<ULONGLONG>(propiedadTiempo.hVal.QuadPart);
    } else {
        return false;
    }

    constexpr ULONGLONG MAYOR_TIEMPO_REPRESENTABLE =
        static_cast<ULONGLONG>(std::numeric_limits<LONGLONG>::max());
    if (tiempoSinSigno > MAYOR_TIEMPO_REPRESENTABLE) return false;

    *tiempoExtraido = static_cast<LONGLONG>(tiempoSinSigno);
    return true;
}

inline bool extraerDuracionMediaFoundation(
    const PROPVARIANT& propiedadDuracion, LONGLONG* duracionExtraida) {
    return extraerTiempoNoNegativoMediaFoundation(propiedadDuracion,
                                                   duracionExtraida);
}

inline bool extraerPosicionMediaFoundation(
    const PROPVARIANT& propiedadPosicion, LONGLONG* posicionExtraida) {
    return extraerTiempoNoNegativoMediaFoundation(propiedadPosicion,
                                                   posicionExtraida);
}
/**** ended inlining reproductor/conversion_tiempo_media_foundation.hpp ****/
/**** start inlining reproductor/controles.cpp ****/
void reiniciarCacheTiempoReproduccion() {
    posicionReproduccionCache.store(0);
    duracionReproduccionCache.store(0);
    tiempoReproduccionDisponible.store(false);
    cambioPosicionPendiente.store(false);
    posicionamientoDisponible.store(false);
}

void invalidarInterfazControles() {
    if (ventanaControles) {
        InvalidateRect(ventanaControles, nullptr, FALSE);
    }
}

// Solo válido en el hilo reproductor: ahí vive IMFPMediaPlayer y COM responde
// correctamente a GetPosition y GetDuration.
bool leerTiempoDesdeReproductor(LONGLONG* posicionActual,
                                LONGLONG* duracionTotal) {
    if (!reproductor || !posicionActual || !duracionTotal) return false;

    PROPVARIANT propiedadPosicion;
    PROPVARIANT propiedadDuracion;
    PropVariantInit(&propiedadPosicion);
    PropVariantInit(&propiedadDuracion);
    HRESULT resultadoPosicion = reproductor->GetPosition(
        MFP_POSITIONTYPE_100NS, &propiedadPosicion);
    HRESULT resultadoDuracion = reproductor->GetDuration(
        MFP_POSITIONTYPE_100NS, &propiedadDuracion);
    LONGLONG posicionExtraida = 0;
    LONGLONG duracionExtraida = 0;
    bool tiempoDisponible =
        SUCCEEDED(resultadoPosicion) && SUCCEEDED(resultadoDuracion) &&
        extraerPosicionMediaFoundation(propiedadPosicion,
                                        &posicionExtraida) &&
        extraerDuracionMediaFoundation(propiedadDuracion,
                                        &duracionExtraida) &&
        duracionExtraida > 0;
    if (tiempoDisponible) {
        *posicionActual = posicionExtraida;
        *duracionTotal = duracionExtraida;
    }
    PropVariantClear(&propiedadPosicion);
    PropVariantClear(&propiedadDuracion);
    return tiempoDisponible;
}

void actualizarCacheTiempoReproduccion() {
    if (!reproductor) {
        reiniciarCacheTiempoReproduccion();
        return;
    }

    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    if (!leerTiempoDesdeReproductor(&posicionActual, &duracionTotal)) {
        return;
    }

    bool teniaTiempo = tiempoReproduccionDisponible.load();
    long long posicionAnterior = posicionReproduccionCache.load();
    posicionReproduccionCache.store(posicionActual);
    duracionReproduccionCache.store(duracionTotal);
    tiempoReproduccionDisponible.store(true);

    long long deltaPosicion = posicionActual - posicionAnterior;
    if (deltaPosicion < 0) {
        deltaPosicion = -deltaPosicion;
    }
    constexpr long long umbralActualizacion = 500000LL;
    if (ventanaControles &&
        (!teniaTiempo || deltaPosicion >= umbralActualizacion)) {
        PostMessageW(ventanaControles, MENSAJE_ACTUALIZAR_INTERFAZ, 0, 0);
    }
}

bool obtenerTiempoReproduccionCache(LONGLONG* posicionActual,
                                    LONGLONG* duracionTotal) {
    if (!tiempoReproduccionDisponible.load() || !posicionActual ||
        !duracionTotal) {
        return false;
    }
    *posicionActual = posicionReproduccionCache.load();
    *duracionTotal = duracionReproduccionCache.load();
    return *duracionTotal > 0;
}

bool aplicarPosicionAlReproductor(LONGLONG posicionObjetivo,
                                  bool reproducirAlCompletar) {
    if (!reproductor) return false;

    PROPVARIANT propiedadPosicion;
    PropVariantInit(&propiedadPosicion);
    propiedadPosicion.vt = VT_I8;
    propiedadPosicion.hVal.QuadPart = posicionObjetivo;

    reproducirTrasCambioPosicion.store(reproducirAlCompletar);
    HRESULT resultadoPosicion = reproductor->SetPosition(
        MFP_POSITIONTYPE_100NS, &propiedadPosicion);
    PropVariantClear(&propiedadPosicion);

    if (FAILED(resultadoPosicion)) {
        reproducirTrasCambioPosicion.store(false);
        Wh_Log(L"No fue posible cambiar la posición (HRESULT 0x%08X)",
               static_cast<unsigned int>(resultadoPosicion));
        return false;
    }

    invalidarInterfazControles();
    return true;
}

// Respaldo en el hilo reproductor cuando la caché todavía no tiene duración.
bool buscarEnReproductorDesdeHilo(double proporcionObjetivo,
                                  bool reproducirAlCompletar = false) {
    if (!reproductor) return false;

    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    if (!leerTiempoDesdeReproductor(&posicionActual, &duracionTotal)) {
        return false;
    }

    double proporcionLimitada =
        limitarProporcionProgreso(proporcionObjetivo);
    LONGLONG posicionObjetivo = calcularPosicionTemporal(
        duracionTotal, proporcionLimitada);
    if (!aplicarPosicionAlReproductor(posicionObjetivo, reproducirAlCompletar)) {
        return false;
    }

    duracionReproduccionCache.store(duracionTotal);
    tiempoReproduccionDisponible.store(true);
    return true;
}

// La caché evita consultar la duración durante cada interacción. Si todavía
// no está preparada, el delegado ejecuta una única lectura en el hilo dueño.
bool buscarEnReproduccion(double proporcionObjetivo,
                          bool reproducirAlCompletar = false) {
    if (!reproductor || !posicionamientoDisponible.load()) return false;

    double proporcionLimitada =
        limitarProporcionProgreso(proporcionObjetivo);
    proporcionPosicionSolicitada.store(proporcionLimitada);
    cambioPosicionPendiente.store(true);
    LONGLONG duracionTotal = duracionReproduccionCache.load();
    if (duracionTotal <= 0) {
        reproducirTrasCambioPosicion.store(reproducirAlCompletar);
        if (ventanaDelegadoReproductor &&
            IsWindow(ventanaDelegadoReproductor)) {
            bool busquedaAplicada =
                SendMessageW(ventanaDelegadoReproductor,
                             MENSAJE_EJECUTAR_BUSQUEDA, 0, 0) != 0;
            if (!busquedaAplicada) cambioPosicionPendiente.store(false);
            return busquedaAplicada;
        }
        cambioPosicionPendiente.store(false);
        return false;
    }

    LONGLONG posicionObjetivo =
        calcularPosicionTemporal(duracionTotal, proporcionLimitada);
    bool busquedaAplicada =
        aplicarPosicionAlReproductor(posicionObjetivo,
                                     reproducirAlCompletar);
    if (!busquedaAplicada) cambioPosicionPendiente.store(false);
    return busquedaAplicada;
}

void alternarReproduccion() {
    if (!reproductor) return;
    MFP_MEDIAPLAYER_STATE estadoReproduccion;
    if (SUCCEEDED(reproductor->GetState(&estadoReproduccion)) &&
        estadoReproduccion == MFP_MEDIAPLAYER_STATE_PLAYING) {
        reproductor->Pause();
    } else {
        reproductor->Play();
    }
    invalidarInterfazControles();
}

void alternarSonido() {
    if (!reproductor) return;
    BOOL sonidoSilenciado = TRUE;
    reproductor->GetMute(&sonidoSilenciado);
    reproductor->SetMute(!sonidoSilenciado);
    invalidarInterfazControles();
}

void alternarRepeticion() {
    repeticionHabilitada.store(!repeticionHabilitada.load());
    invalidarInterfazControles();
}

inline void actualizarMetadatosVideo(const std::wstring& rutaVideo) {
    if (rutaVideo.empty()) {
        liberarRecursosVideo();
        return;
    }

    estadoVideoMetadatos.rutaVideo = rutaVideo;
    estadoVideoMetadatos.nombreArchivo = PathFindFileNameW(rutaVideo.c_str());
    const wchar_t* extension = PathFindExtensionW(rutaVideo.c_str());
    estadoVideoMetadatos.extensionFormato = extension ? extension : L"";

    WIN32_FILE_ATTRIBUTE_DATA datosArchivo{};
    if (GetFileAttributesExW(rutaVideo.c_str(), GetFileExInfoStandard, &datosArchivo)) {
        ULARGE_INTEGER tamanoGrande{};
        tamanoGrande.LowPart = datosArchivo.nFileSizeLow;
        tamanoGrande.HighPart = datosArchivo.nFileSizeHigh;
        wchar_t buferTamano[64]{};
        StrFormatByteSizeW(static_cast<LONGLONG>(tamanoGrande.QuadPart), buferTamano, ARRAYSIZE(buferTamano));
        estadoVideoMetadatos.textoTamano = buferTamano;

        FILETIME ftLocal{};
        FileTimeToLocalFileTime(&datosArchivo.ftLastWriteTime, &ftLocal);
        SYSTEMTIME st{};
        FileTimeToSystemTime(&ftLocal, &st);
        wchar_t buferFecha[64]{};
        swprintf_s(buferFecha, L"%02u/%02u/%04u %02u:%02u",
                   st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
        estadoVideoMetadatos.textoFechaModificacion = buferFecha;
    } else {
        estadoVideoMetadatos.textoTamano = L"";
        estadoVideoMetadatos.textoFechaModificacion = L"";
    }

    std::vector<std::wstring> videosCarpeta = obtenerVideosEnMismaCarpeta(rutaVideo);
    estadoVideoMetadatos.totalEnCarpeta = static_cast<int>(videosCarpeta.size());
    auto iteradorEnCarpeta = std::find(videosCarpeta.begin(), videosCarpeta.end(), rutaVideo);
    if (iteradorEnCarpeta != videosCarpeta.end()) {
        estadoVideoMetadatos.indiceEnCarpeta = static_cast<int>(std::distance(videosCarpeta.begin(), iteradorEnCarpeta));
    } else {
        estadoVideoMetadatos.indiceEnCarpeta = 0;
    }

    if (listaVideosSeleccionados.size() > 1) {
        estadoVideoMetadatos.esRangoSeleccionado = true;
        estadoVideoMetadatos.totalEnRango = static_cast<int>(listaVideosSeleccionados.size());
        estadoVideoMetadatos.indiceEnRango = static_cast<int>(indiceVideoSeleccionado);
    } else {
        estadoVideoMetadatos.esRangoSeleccionado = false;
        estadoVideoMetadatos.totalEnRango = 1;
        estadoVideoMetadatos.indiceEnRango = 0;
    }
}

inline void activarParpadeoBotonesNavVideo() {
    estadoVideoMetadatos.tiempoInicioParpadeoNav = GetTickCount64();
    if (ventanaControles && IsWindow(ventanaControles)) {
        SetTimer(ventanaControles, ID_TEMPORIZADOR_PARPADEO_NAV, 420, nullptr);
        InvalidateRect(ventanaControles, nullptr, FALSE);
    }
}

bool reproducirVideoPorRuta(const std::wstring& rutaNuevoVideo) {
    if (!reproductor || rutaNuevoVideo.empty()) return false;
    navegacionManualEnCurso.store(true);
    reproductor->Stop();
    reiniciarCacheTiempoReproduccion();
    rutaVideoActiva = rutaNuevoVideo;
    actualizarMetadatosVideo(rutaNuevoVideo);
    HRESULT resultadoCarga = reproductor->CreateMediaItemFromURL(
        rutaNuevoVideo.c_str(), FALSE, 0, nullptr);
    if (FAILED(resultadoCarga)) {
        navegacionManualEnCurso.store(false);
        Wh_Log(L"No fue posible cargar el video: %s (HRESULT 0x%08X)",
               rutaNuevoVideo.c_str(),
               static_cast<unsigned int>(resultadoCarga));
        return false;
    }
    if (ventanaReproductor && IsWindow(ventanaReproductor)) {
        InvalidateRect(ventanaReproductor, nullptr, FALSE);
    }
    invalidarInterfazControles();
    return true;
}

void reproducirVideoAnterior() {
    if (listaVideosSeleccionados.size() > 1) {
        indiceVideoSeleccionado =
            (indiceVideoSeleccionado == 0)
                ? (listaVideosSeleccionados.size() - 1)
                : (indiceVideoSeleccionado - 1);
        reproducirVideoPorRuta(listaVideosSeleccionados[indiceVideoSeleccionado]);
        return;
    }
    if (rutaVideoActiva.empty()) return;
    std::vector<std::wstring> videosCarpeta = obtenerVideosEnMismaCarpeta(rutaVideoActiva);
    if (videosCarpeta.size() <= 1) {
        activarParpadeoBotonesNavVideo();
        return;
    }
    std::wstring rutaAnterior =
        obtenerVideoAnteriorEnCarpeta(rutaVideoActiva);
    if (!rutaAnterior.empty() && rutaAnterior != rutaVideoActiva) {
        reproducirVideoPorRuta(rutaAnterior);
    } else {
        activarParpadeoBotonesNavVideo();
    }
}

void reproducirVideoSiguiente() {
    if (listaVideosSeleccionados.size() > 1) {
        indiceVideoSeleccionado =
            (indiceVideoSeleccionado + 1) % listaVideosSeleccionados.size();
        reproducirVideoPorRuta(listaVideosSeleccionados[indiceVideoSeleccionado]);
        return;
    }
    if (rutaVideoActiva.empty()) return;
    std::vector<std::wstring> videosCarpeta = obtenerVideosEnMismaCarpeta(rutaVideoActiva);
    if (videosCarpeta.size() <= 1) {
        activarParpadeoBotonesNavVideo();
        return;
    }
    std::wstring rutaSiguiente =
        obtenerVideoSiguienteEnCarpeta(rutaVideoActiva);
    if (!rutaSiguiente.empty() && rutaSiguiente != rutaVideoActiva) {
        reproducirVideoPorRuta(rutaSiguiente);
    } else {
        activarParpadeoBotonesNavVideo();
    }
}

void cambiarVelocidadReproduccion(float nuevaVelocidad) {
    if (!reproductor) return;
    if (SUCCEEDED(reproductor->SetRate(nuevaVelocidad))) {
        velocidadReproduccionActual.store(nuevaVelocidad);
    }
    invalidarInterfazControles();
}


void iniciarArrastreBarraProgreso(HWND ventana, int posicionHorizontal) {
    if (!reproductor || !posicionamientoDisponible.load()) return;

    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);
    proporcionProgreso = calcularProporcionProgreso(
        disenoBarraProgreso, posicionHorizontal);
    arrastrandoBarraProgreso = true;

    SetFocus(ventana);
    SetCapture(ventana);
    invalidarInterfazControles();
}

void actualizarArrastreBarraProgreso(HWND ventana, int posicionHorizontal) {
    if (!arrastrandoBarraProgreso) return;
    if (GetCapture() != ventana) {
        arrastrandoBarraProgreso = false;
        invalidarInterfazControles();
        return;
    }

    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);
    proporcionProgreso = calcularProporcionProgreso(
        disenoBarraProgreso, posicionHorizontal);

    invalidarInterfazControles();
}

void finalizarArrastreBarraProgreso(HWND ventana, bool aplicarCambio) {
    if (!arrastrandoBarraProgreso) return;
    if (aplicarCambio) {
        buscarEnReproduccion(proporcionProgreso);
    }
    arrastrandoBarraProgreso = false;
    if (GetCapture() == ventana) ReleaseCapture();
    invalidarInterfazControles();
}

void desplazarBarraProgresoConTeclado(double incrementoProporcion) {
    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    if (!obtenerTiempoReproduccionCache(&posicionActual, &duracionTotal)) {
        return;
    }
    buscarEnReproduccion(
        static_cast<double>(posicionActual) /
            static_cast<double>(duracionTotal) +
        incrementoProporcion);
}
/**** ended inlining reproductor/controles.cpp ****/
/**** start inlining reproductor/eventos_media_foundation.cpp ****/
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
/**** ended inlining reproductor/eventos_media_foundation.cpp ****/
/**** start inlining interfaz/renderizado.cpp ****/
std::wstring formatearTiempo(LONGLONG unidadesCienNanosegundos) {
    long long segundosTotales = unidadesCienNanosegundos / 10000000LL;
    long long horas = segundosTotales / 3600;
    int minutos = static_cast<int>((segundosTotales % 3600) / 60);
    int segundos = static_cast<int>(segundosTotales % 60);
    auto formatearDosDigitos = [](int numero) {
        std::wstring numeroFormateado = std::to_wstring(numero);
        return numeroFormateado.length() == 1
                   ? L"0" + numeroFormateado
                   : numeroFormateado;
    };
    if (horas > 0) {
        return std::to_wstring(horas) + L":" +
               formatearDosDigitos(minutos) + L":" +
               formatearDosDigitos(segundos);
    }
    return formatearDosDigitos(minutos) + L":" +
           formatearDosDigitos(segundos);
}

std::wstring formatearEtiquetaVelocidad(float velocidad) {
    for (int posicionOpcion = 0; posicionOpcion < TOTAL_OPCIONES_VELOCIDAD;
         ++posicionOpcion) {
        if (std::abs(VALORES_VELOCIDAD[posicionOpcion] - velocidad) < 0.01f) {
            return ETIQUETAS_VELOCIDAD[posicionOpcion];
        }
    }
    wchar_t buferVelocidad[16];
    swprintf_s(buferVelocidad, L"%.2fx", static_cast<double>(velocidad));
    return buferVelocidad;
}

bool crearRecursosInterfaz() {
    fuenteIconos = CreateFontW(
        -18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe Fluent Icons");
    fuenteTexto = CreateFontW(
        -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    fuenteTextoPequeno = CreateFontW(
        -11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    if (fuenteIconos && fuenteTexto && fuenteTextoPequeno) return true;

    if (fuenteIconos) DeleteObject(fuenteIconos);
    if (fuenteTexto) DeleteObject(fuenteTexto);
    if (fuenteTextoPequeno) DeleteObject(fuenteTextoPequeno);
    fuenteIconos = nullptr;
    fuenteTexto = nullptr;
    fuenteTextoPequeno = nullptr;
    return false;
}

void liberarRecursosInterfaz() {
    if (fuenteIconos) DeleteObject(fuenteIconos);
    if (fuenteTexto) DeleteObject(fuenteTexto);
    if (fuenteTextoPequeno) DeleteObject(fuenteTextoPequeno);
    fuenteIconos = nullptr;
    fuenteTexto = nullptr;
    fuenteTextoPequeno = nullptr;
}

void dibujarLinea(HDC contextoDibujo, int limiteIzquierdo,
                  int limiteDerecho, int posicionVertical, int grosor,
                  COLORREF color) {
    HPEN lapiz = CreatePen(PS_SOLID, grosor, color);
    HPEN lapizPreviamenteSeleccionado =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapiz));
    MoveToEx(contextoDibujo, limiteIzquierdo, posicionVertical, nullptr);
    LineTo(contextoDibujo, limiteDerecho, posicionVertical);
    SelectObject(contextoDibujo, lapizPreviamenteSeleccionado);
    DeleteObject(lapiz);
}

void dibujarBotonControl(HDC contextoDibujo, const RECT& areaBoton,
                         const wchar_t* glifo, bool resaltado,
                         bool botonPrincipal, bool parpadeoAlerta = false) {
    COLORREF colorFondo = parpadeoAlerta
                              ? RGB(85, 45, 25)
                              : (botonPrincipal
                                     ? RGB(0, 120, 212)
                                     : (resaltado ? RGB(35, 91, 138)
                                                  : RGB(43, 43, 43)));
    COLORREF colorBorde = parpadeoAlerta
                              ? RGB(255, 170, 0)
                              : (resaltado || botonPrincipal
                                     ? RGB(94, 181, 255)
                                     : RGB(72, 72, 72));
    COLORREF colorTexto = parpadeoAlerta
                              ? RGB(255, 210, 100)
                              : RGB(250, 250, 250);
    HBRUSH pincelFondo = CreateSolidBrush(colorFondo);
    HPEN lapizBorde = CreatePen(PS_SOLID, 1, colorBorde);
    HBRUSH pincelAnterior =
        static_cast<HBRUSH>(SelectObject(contextoDibujo, pincelFondo));
    HPEN lapizAnterior =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    RoundRect(contextoDibujo, areaBoton.left, areaBoton.top,
              areaBoton.right, areaBoton.bottom, 14, 14);
    SelectObject(contextoDibujo, lapizAnterior);
    SelectObject(contextoDibujo, pincelAnterior);
    DeleteObject(lapizBorde);
    DeleteObject(pincelFondo);

    HFONT fuenteAnterior =
        static_cast<HFONT>(SelectObject(contextoDibujo, fuenteIconos));
    SetTextColor(contextoDibujo, colorTexto);
    RECT areaIcono = areaBoton;
    DrawTextW(contextoDibujo, glifo, -1, &areaIcono,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(contextoDibujo, fuenteAnterior);
}

void dibujarBotonTexto(HDC contextoDibujo, const RECT& areaBoton,
                       const wchar_t* texto, bool resaltado) {
    COLORREF colorFondo = resaltado ? RGB(0, 120, 212) : RGB(43, 43, 43);
    COLORREF colorBorde = resaltado ? RGB(94, 181, 255) : RGB(72, 72, 72);
    HBRUSH pincelFondo = CreateSolidBrush(colorFondo);
    HPEN lapizBorde = CreatePen(PS_SOLID, 1, colorBorde);
    HBRUSH pincelAnterior =
        static_cast<HBRUSH>(SelectObject(contextoDibujo, pincelFondo));
    HPEN lapizAnterior =
        static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    RoundRect(contextoDibujo, areaBoton.left, areaBoton.top,
              areaBoton.right, areaBoton.bottom, 10, 10);
    SelectObject(contextoDibujo, lapizAnterior);
    SelectObject(contextoDibujo, pincelAnterior);
    DeleteObject(lapizBorde);
    DeleteObject(pincelFondo);

    HFONT fuenteAnterior =
        static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
    SetTextColor(contextoDibujo, RGB(245, 245, 245));
    RECT areaTexto = areaBoton;
    DrawTextW(contextoDibujo, texto, -1, &areaTexto,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(contextoDibujo, fuenteAnterior);
}

void dibujarMenuVelocidadFlotante(HWND ventana) {
    PAINTSTRUCT pintura;
    HDC contextoPantalla = BeginPaint(ventana, &pintura);
    RECT areaCliente;
    GetClientRect(ventana, &areaCliente);
    int anchoCliente = areaCliente.right - areaCliente.left;
    int altoCliente = areaCliente.bottom - areaCliente.top;

    if (anchoCliente > 0 && altoCliente > 0) {
        HDC contextoMemoria = CreateCompatibleDC(contextoPantalla);
        HBITMAP mapaBitsMemoria =
            CreateCompatibleBitmap(contextoPantalla, anchoCliente, altoCliente);
        HBITMAP mapaBitsAnterior = static_cast<HBITMAP>(
            SelectObject(contextoMemoria, mapaBitsMemoria));

        HBRUSH pincelFondoPanel = CreateSolidBrush(RGB(28, 28, 28));
        HPEN lapizBordePanel = CreatePen(PS_SOLID, 1, RGB(70, 70, 70));
        HBRUSH pincelAnterior =
            static_cast<HBRUSH>(SelectObject(contextoMemoria, pincelFondoPanel));
        HPEN lapizAnterior =
            static_cast<HPEN>(SelectObject(contextoMemoria, lapizBordePanel));

        RoundRect(contextoMemoria, areaCliente.left, areaCliente.top,
                  areaCliente.right, areaCliente.bottom, 10, 10);

        SelectObject(contextoMemoria, lapizAnterior);
        SelectObject(contextoMemoria, pincelAnterior);
        DeleteObject(lapizBordePanel);
        DeleteObject(pincelFondoPanel);

        HFONT fuenteAnterior =
            static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTextoPequeno));
        SetBkMode(contextoMemoria, TRANSPARENT);
        float velocidadActual = velocidadReproduccionActual.load();

        for (int posicionOpcion = 0; posicionOpcion < TOTAL_OPCIONES_VELOCIDAD;
             ++posicionOpcion) {
            int topeOpcion = 4 + posicionOpcion * ALTO_OPCION_VELOCIDAD;
            RECT areaOpcion = {4, topeOpcion, anchoCliente - 4,
                               topeOpcion + ALTO_OPCION_VELOCIDAD};
            bool esVelocidadSeleccionada =
                std::abs(VALORES_VELOCIDAD[posicionOpcion] - velocidadActual) <
                0.01f;
            bool esOpcionHover =
                posicionOpcion == opcionVelocidadResaltada;

            if (esVelocidadSeleccionada || esOpcionHover) {
                COLORREF colorOpcion = esVelocidadSeleccionada
                                           ? RGB(0, 120, 212)
                                           : RGB(55, 55, 55);
                HBRUSH pincelOpcion = CreateSolidBrush(colorOpcion);
                HPEN lapizOpcion = CreatePen(PS_SOLID, 1, colorOpcion);
                HBRUSH pincelOpAnterior = static_cast<HBRUSH>(
                    SelectObject(contextoMemoria, pincelOpcion));
                HPEN lapizOpAnterior = static_cast<HPEN>(
                    SelectObject(contextoMemoria, lapizOpcion));
                RoundRect(contextoMemoria, areaOpcion.left, areaOpcion.top,
                          areaOpcion.right, areaOpcion.bottom, 6, 6);
                SelectObject(contextoMemoria, lapizOpAnterior);
                SelectObject(contextoMemoria, pincelOpAnterior);
                DeleteObject(lapizOpcion);
                DeleteObject(pincelOpcion);
            }

            SetTextColor(contextoMemoria, esVelocidadSeleccionada
                                             ? RGB(255, 255, 255)
                                             : (esOpcionHover ? RGB(245, 245, 245)
                                                              : RGB(210, 210, 210)));
            RECT areaTextoOpcion = areaOpcion;
            DrawTextW(contextoMemoria, ETIQUETAS_VELOCIDAD[posicionOpcion], -1,
                      &areaTextoOpcion,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }

        SelectObject(contextoMemoria, fuenteAnterior);

        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente, contextoMemoria,
               0, 0, SRCCOPY);

        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
    }
    EndPaint(ventana, &pintura);
}

void dibujarLienzoComprimidos(HDC contextoMemoria, const RECT& areaCliente) {
    int anchoCliente = areaCliente.right - areaCliente.left;
    int altoCliente = areaCliente.bottom - areaCliente.top;
    if (anchoCliente <= 0 || altoCliente <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(18, 18, 18));
    FillRect(contextoMemoria, &areaCliente, brochaFondo);
    DeleteObject(brochaFondo);

    Gdiplus::Graphics graficos(contextoMemoria);
    graficos.SetInterpolationMode(
        Gdiplus::InterpolationModeHighQualityBicubic);
    graficos.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    if (estadoComprimidos.esModoComic) {
        size_t totalElementos = estadoComprimidos.elementosSeleccionados.size();
        if (totalElementos == 1) {
            const ElementoComprimido& elemento =
                estadoComprimidos.elementosSeleccionados.front();
            if (elemento.portadaCargada) {
                int anchoImagen = static_cast<int>(elemento.portadaCargada->GetWidth());
                int altoImagen = static_cast<int>(elemento.portadaCargada->GetHeight());
                if (anchoImagen > 0 && altoImagen > 0) {
                    float escalaHorizontal =
                        static_cast<float>(anchoCliente - 24) /
                        static_cast<float>(anchoImagen);
                    float escalaVertical =
                        static_cast<float>(altoCliente - 24) /
                        static_cast<float>(altoImagen);
                    float escalaOptima =
                        std::min(escalaHorizontal, escalaVertical);

                    int anchoDestino =
                        static_cast<int>(static_cast<float>(anchoImagen) * escalaOptima);
                    int altoDestino =
                        static_cast<int>(static_cast<float>(altoImagen) * escalaOptima);
                    int posicionX = (anchoCliente - anchoDestino) / 2;
                    int posicionY = (altoCliente - altoDestino) / 2;

                    graficos.DrawImage(elemento.portadaCargada, posicionX,
                                       posicionY, anchoDestino, altoDestino);
                }
            } else {
                RECT areaIcono{0, altoCliente / 2 - 40, anchoCliente,
                               altoCliente / 2};
                RECT areaTexto{10, altoCliente / 2 + 5, anchoCliente - 10,
                               altoCliente / 2 + 30};
                RECT areaTamano{10, altoCliente / 2 + 30, anchoCliente - 10,
                                altoCliente / 2 + 50};

                HFONT fuenteIconoGrande = CreateFontW(
                    -42, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                    DEFAULT_PITCH | FF_DONTCARE, L"Segoe Fluent Icons");
                HFONT fuentePrevia = static_cast<HFONT>(
                    SelectObject(contextoMemoria, fuenteIconoGrande));
                SetBkMode(contextoMemoria, TRANSPARENT);
                SetTextColor(contextoMemoria, RGB(255, 140, 0));
                DrawTextW(contextoMemoria, GLIFO_COMPRIMIDO, -1, &areaIcono,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                SelectObject(contextoMemoria, fuenteTexto);
                SetTextColor(contextoMemoria, RGB(230, 230, 230));
                DrawTextW(contextoMemoria, elemento.nombreArchivo.c_str(), -1,
                          &areaTexto,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS |
                              DT_NOPREFIX);

                SetTextColor(contextoMemoria, RGB(160, 160, 160));
                DrawTextW(contextoMemoria, elemento.textoTamano.c_str(), -1,
                          &areaTamano,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                SelectObject(contextoMemoria, fuentePrevia);
                DeleteObject(fuenteIconoGrande);
            }
        } else if (totalElementos >= 2) {
            int limite = configuracion.limiteCuadriculaComprimidos.load();
            if (limite != 4 && limite != 6 && limite != 8 && limite != 10) {
                limite = 4;
            }
            int totalFilas = (limite + 1) / 2;

            int margen = 12;
            int separacion = 8;
            int anchoDisponible = anchoCliente - margen * 2 - separacion;
            int altoDisponible =
                altoCliente - margen * 2 - (totalFilas - 1) * separacion;
            int anchoCelda = std::max(40, anchoDisponible / 2);
            int altoCelda = std::max(40, altoDisponible / totalFilas);

            size_t cantidadADibujar = std::min<size_t>(
                static_cast<size_t>(limite), totalElementos);

            for (size_t posicion = 0; posicion < cantidadADibujar; ++posicion) {
                int fila = static_cast<int>(posicion / 2);
                int columna = static_cast<int>(posicion % 2);
                int izquierdaCelda = margen + columna * (anchoCelda + separacion);
                int topeCelda = margen + fila * (altoCelda + separacion);

                RECT areaCelda{izquierdaCelda, topeCelda,
                               izquierdaCelda + anchoCelda,
                               topeCelda + altoCelda};
                HBRUSH brochaCelda = CreateSolidBrush(RGB(28, 28, 28));
                FillRect(contextoMemoria, &areaCelda, brochaCelda);
                DeleteObject(brochaCelda);

                const ElementoComprimido& elemento =
                    estadoComprimidos.elementosSeleccionados[posicion];
                if (elemento.portadaCargada) {
                    int anchoImg = static_cast<int>(elemento.portadaCargada->GetWidth());
                    int altoImg = static_cast<int>(elemento.portadaCargada->GetHeight());
                    float escH = static_cast<float>(anchoCelda - 8) /
                                 static_cast<float>(anchoImg);
                    float escV = static_cast<float>(altoCelda - 8) /
                                 static_cast<float>(altoImg);
                    float esc = std::min(escH, escV);
                    int anchoD = static_cast<int>(static_cast<float>(anchoImg) * esc);
                    int altoD = static_cast<int>(static_cast<float>(altoImg) * esc);
                    int posX = izquierdaCelda + (anchoCelda - anchoD) / 2;
                    int posY = topeCelda + (altoCelda - altoD) / 2;
                    graficos.DrawImage(elemento.portadaCargada, posX, posY,
                                       anchoD, altoD);
                } else {
                    HFONT fuenteIconoPrevia = static_cast<HFONT>(
                        SelectObject(contextoMemoria, fuenteIconos));
                    SetBkMode(contextoMemoria, TRANSPARENT);
                    SetTextColor(contextoMemoria, RGB(255, 140, 0));
                    RECT areaIconoCelda{izquierdaCelda, topeCelda + 6,
                                        izquierdaCelda + anchoCelda,
                                        topeCelda + altoCelda / 2};
                    DrawTextW(contextoMemoria, GLIFO_COMPRIMIDO, -1,
                              &areaIconoCelda,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                    SelectObject(contextoMemoria, fuenteTextoPequeno);
                    SetTextColor(contextoMemoria, RGB(220, 220, 220));
                    RECT areaTextoCelda{izquierdaCelda + 4,
                                        topeCelda + altoCelda / 2,
                                        izquierdaCelda + anchoCelda - 4,
                                        topeCelda + altoCelda - 4};
                    DrawTextW(contextoMemoria, elemento.nombreArchivo.c_str(), -1,
                              &areaTextoCelda,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE |
                                  DT_END_ELLIPSIS | DT_NOPREFIX);
                    SelectObject(contextoMemoria, fuenteIconoPrevia);
                }

                HPEN lapizCelda = CreatePen(PS_SOLID, 1, RGB(48, 48, 48));
                HPEN lapizPrevio =
                    static_cast<HPEN>(SelectObject(contextoMemoria, lapizCelda));
                HBRUSH brochaNula =
                    static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
                HBRUSH brochaPrevia =
                    static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaNula));
                Rectangle(contextoMemoria, areaCelda.left, areaCelda.top,
                          areaCelda.right, areaCelda.bottom);
                SelectObject(contextoMemoria, brochaPrevia);
                SelectObject(contextoMemoria, lapizPrevio);
                DeleteObject(lapizCelda);
            }

            if (estadoComprimidos.totalSeleccionados > static_cast<size_t>(limite)) {
                std::wstring textoContador =
                    L"+" + std::to_wstring(
                               estadoComprimidos.totalSeleccionados -
                               static_cast<size_t>(limite));
                int anchoBadge = 46;
                int altoBadge = 26;
                int izquierdaBadge = anchoCliente - margen - anchoBadge;
                int topeBadge = margen;
                RECT areaBadge{izquierdaBadge, topeBadge,
                               izquierdaBadge + anchoBadge,
                               topeBadge + altoBadge};

                HBRUSH brochaBadge = CreateSolidBrush(RGB(0, 120, 215));
                HPEN lapizBadge = CreatePen(PS_SOLID, 1, RGB(0, 140, 240));
                HBRUSH brochaBadgePrevia = static_cast<HBRUSH>(
                    SelectObject(contextoMemoria, brochaBadge));
                HPEN lapizBadgePrevio =
                    static_cast<HPEN>(SelectObject(contextoMemoria, lapizBadge));
                RoundRect(contextoMemoria, areaBadge.left, areaBadge.top,
                          areaBadge.right, areaBadge.bottom, 12, 12);
                SelectObject(contextoMemoria, brochaBadgePrevia);
                SelectObject(contextoMemoria, lapizBadgePrevio);
                DeleteObject(brochaBadge);
                DeleteObject(lapizBadge);

                HFONT fuentePrevia = static_cast<HFONT>(
                    SelectObject(contextoMemoria, fuenteTextoPequeno));
                SetBkMode(contextoMemoria, TRANSPARENT);
                SetTextColor(contextoMemoria, RGB(255, 255, 255));
                DrawTextW(contextoMemoria, textoContador.c_str(), -1, &areaBadge,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                SelectObject(contextoMemoria, fuentePrevia);
            }
        }
        return;
    }

    // --- MODO COMPRIMIDOS COMUNES (TREEMAP) ---
    int margenSuperior = 10;
    int altoCabecera = 36;
    if (estadoComprimidos.archivosSeleccionados.size() > 1) {
        int anchoBoton = 72;
        int altoBoton = 24;
        estadoComprimidos.botonAnteriorComprimido = {
            12, margenSuperior + 4, 12 + anchoBoton, margenSuperior + 4 + altoBoton
        };
        estadoComprimidos.botonSiguienteComprimido = {
            anchoCliente - 12 - anchoBoton, margenSuperior + 4, anchoCliente - 12, margenSuperior + 4 + altoBoton
        };

        auto dibujarBotonNav = [&](const RECT& rect, const wchar_t* texto, bool hover) {
            COLORREF colorFondo = hover ? RGB(48, 54, 61) : RGB(33, 38, 45);
            COLORREF colorBorde = hover ? RGB(110, 118, 129) : RGB(48, 54, 61);
            HBRUSH brochaBtn = CreateSolidBrush(colorFondo);
            HPEN lapizBtn = CreatePen(PS_SOLID, 1, colorBorde);
            HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaBtn));
            HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoMemoria, lapizBtn));
            RoundRect(contextoMemoria, rect.left, rect.top, rect.right, rect.bottom, 6, 6);
            SelectObject(contextoMemoria, lapizPrev);
            SelectObject(contextoMemoria, brochaPrev);
            DeleteObject(lapizBtn);
            DeleteObject(brochaBtn);

            SetBkMode(contextoMemoria, TRANSPARENT);
            SetTextColor(contextoMemoria, hover ? RGB(255, 255, 255) : RGB(201, 209, 217));
            HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTextoPequeno));
            RECT rTexto = rect;
            DrawTextW(contextoMemoria, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(contextoMemoria, fuentePrev);
        };

        dibujarBotonNav(estadoComprimidos.botonAnteriorComprimido, L"◀ Ant", estadoComprimidos.hoverBotonAnterior);
        dibujarBotonNav(estadoComprimidos.botonSiguienteComprimido, L"Sig ▶", estadoComprimidos.hoverBotonSiguiente);

        std::wstring nombreArchivoActual = L"";
        if (estadoComprimidos.indiceArchivoActual < estadoComprimidos.archivosSeleccionados.size()) {
            nombreArchivoActual = PathFindFileNameW(
                estadoComprimidos.archivosSeleccionados[estadoComprimidos.indiceArchivoActual].c_str());
        }

        std::wstring textoCentro = nombreArchivoActual + L" (" +
                                   std::to_wstring(estadoComprimidos.indiceArchivoActual + 1) +
                                   L"/" + std::to_wstring(estadoComprimidos.archivosSeleccionados.size()) + L")";
        RECT areaCentro{
            estadoComprimidos.botonAnteriorComprimido.right + 6,
            margenSuperior,
            estadoComprimidos.botonSiguienteComprimido.left - 6,
            margenSuperior + altoCabecera
        };
        SetBkMode(contextoMemoria, TRANSPARENT);
        SetTextColor(contextoMemoria, RGB(240, 240, 240));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTexto));
        DrawTextW(contextoMemoria, textoCentro.c_str(), -1, &areaCentro,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(contextoMemoria, fuentePrev);
    } else {
        std::wstring nombreArchivoActual = L"";
        if (!estadoComprimidos.archivosSeleccionados.empty()) {
            nombreArchivoActual = PathFindFileNameW(
                estadoComprimidos.archivosSeleccionados.front().c_str());
        }
        RECT areaTitulo{14, margenSuperior, anchoCliente - 14, margenSuperior + altoCabecera};
        SetBkMode(contextoMemoria, TRANSPARENT);
        SetTextColor(contextoMemoria, RGB(240, 240, 240));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTexto));
        DrawTextW(contextoMemoria, (L"📦 " + nombreArchivoActual).c_str(), -1, &areaTitulo,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(contextoMemoria, fuentePrev);
    }

    // Área del TreeMap Box
    int margenArbol = 12;
    int topeArbol = margenSuperior + altoCabecera + 4;
    int altoArbolBox = altoCliente - topeArbol - 10;
    if (altoArbolBox < 40) return;

    estadoComprimidos.areaCajaArbol = {
        margenArbol, topeArbol, anchoCliente - margenArbol, topeArbol + altoArbolBox
    };

    HBRUSH brochaCaja = CreateSolidBrush(RGB(13, 17, 23));
    HPEN lapizCaja = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPrevCaja = static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaCaja));
    HPEN lapizPrevCaja = static_cast<HPEN>(SelectObject(contextoMemoria, lapizCaja));
    RoundRect(contextoMemoria, estadoComprimidos.areaCajaArbol.left,
              estadoComprimidos.areaCajaArbol.top, estadoComprimidos.areaCajaArbol.right,
              estadoComprimidos.areaCajaArbol.bottom, 8, 8);
    SelectObject(contextoMemoria, lapizPrevCaja);
    SelectObject(contextoMemoria, brochaPrevCaja);
    DeleteObject(lapizCaja);
    DeleteObject(brochaCaja);

    if (estadoComprimidos.protegidoConContrasena && !estadoComprimidos.contrasenaDesbloqueada) {
        SelectObject(contextoMemoria, fuenteTexto);
        SetTextColor(contextoMemoria, RGB(255, 180, 50));
        RECT areaIconoPass{estadoComprimidos.areaCajaArbol.left + 16,
                           estadoComprimidos.areaCajaArbol.top + 30,
                           estadoComprimidos.areaCajaArbol.right - 16,
                           estadoComprimidos.areaCajaArbol.top + 55};
        DrawTextW(contextoMemoria, L"🔒 Archivo protegido con contraseña", -1,
                  &areaIconoPass, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        SelectObject(contextoMemoria, fuenteTextoPequeno);
        SetTextColor(contextoMemoria, RGB(139, 148, 158));
        RECT areaDescPass{estadoComprimidos.areaCajaArbol.left + 20,
                          estadoComprimidos.areaCajaArbol.top + 62,
                          estadoComprimidos.areaCajaArbol.right - 20,
                          estadoComprimidos.areaCajaArbol.top + 105};
        DrawTextW(contextoMemoria,
                  L"Este archivo comprimido está cifrado. Para visualizar los archivos contenidos en el primer nivel, introduzca su contraseña.",
                  -1, &areaDescPass, DT_CENTER | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

        int anchoBotonDesbloquear = 200;
        int altoBotonDesbloquear = 32;
        int centroX = (estadoComprimidos.areaCajaArbol.left + estadoComprimidos.areaCajaArbol.right) / 2;
        int topBoton = estadoComprimidos.areaCajaArbol.top + 115;
        estadoComprimidos.areaBotonDesbloquear = {
            centroX - anchoBotonDesbloquear / 2, topBoton,
            centroX + anchoBotonDesbloquear / 2, topBoton + altoBotonDesbloquear
        };

        bool hoverDesbloquear = estadoComprimidos.hoverBotonDesbloquear;
        HBRUSH brochaDesb = CreateSolidBrush(hoverDesbloquear ? RGB(48, 54, 61) : RGB(33, 38, 45));
        HPEN lapizDesb = CreatePen(PS_SOLID, 1, hoverDesbloquear ? RGB(88, 166, 255) : RGB(48, 54, 61));
        HBRUSH brochaPrevDesb = static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaDesb));
        HPEN lapizPrevDesb = static_cast<HPEN>(SelectObject(contextoMemoria, lapizDesb));
        RoundRect(contextoMemoria, estadoComprimidos.areaBotonDesbloquear.left,
                  estadoComprimidos.areaBotonDesbloquear.top,
                  estadoComprimidos.areaBotonDesbloquear.right,
                  estadoComprimidos.areaBotonDesbloquear.bottom, 6, 6);
        SelectObject(contextoMemoria, lapizPrevDesb);
        SelectObject(contextoMemoria, brochaPrevDesb);
        DeleteObject(lapizDesb);
        DeleteObject(brochaDesb);

        SetTextColor(contextoMemoria, hoverDesbloquear ? RGB(255, 255, 255) : RGB(230, 237, 243));
        RECT rTextoBtn = estadoComprimidos.areaBotonDesbloquear;
        DrawTextW(contextoMemoria, L"🔑 Desbloquear contenido", -1, &rTextoBtn,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        return;
    }

    int altoCabeceraSeccion = 28;
    int altoFilaElemento = 22;

    int altoCalculadoTotal = 16;
    for (const auto& seccionActual : estadoComprimidos.seccionesContenido) {
        altoCalculadoTotal += altoCabeceraSeccion + static_cast<int>(seccionActual.elementos.size()) * altoFilaElemento + 8;
    }
    estadoComprimidos.altoTotalArbol = altoCalculadoTotal;

    int posicionVerticalInicio = estadoComprimidos.areaCajaArbol.top + 8 - estadoComprimidos.desplazamientoScrollY;

    HRGN regionRecorte = CreateRectRgn(
        estadoComprimidos.areaCajaArbol.left + 2,
        estadoComprimidos.areaCajaArbol.top + 2,
        estadoComprimidos.areaCajaArbol.right - 2,
        estadoComprimidos.areaCajaArbol.bottom - 2);
    SelectClipRgn(contextoMemoria, regionRecorte);

    HFONT fuenteTituloSeccion = CreateFontW(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteItemArbol = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    int posicionVerticalCursor = posicionVerticalInicio;

    for (auto& seccionActual : estadoComprimidos.seccionesContenido) {
        int topeSeccion = posicionVerticalCursor;
        int fondoSeccion = posicionVerticalCursor + altoCabeceraSeccion;
        posicionVerticalCursor += altoCabeceraSeccion;

        RECT areaSeccionHeader{
            estadoComprimidos.areaCajaArbol.left + 8,
            topeSeccion,
            estadoComprimidos.areaCajaArbol.right - 8,
            fondoSeccion
        };
        seccionActual.areaCabecera = areaSeccionHeader;

        if (fondoSeccion >= estadoComprimidos.areaCajaArbol.top &&
            topeSeccion <= estadoComprimidos.areaCajaArbol.bottom) {

            std::wstring textoBadge = seccionActual.iconoSeccion + L" " + seccionActual.tituloSeccion +
                                      L" (" + std::to_wstring(seccionActual.elementos.size()) + L")";

            SelectObject(contextoMemoria, fuenteTituloSeccion);
            SIZE tamanoTextoBadge{};
            GetTextExtentPoint32W(contextoMemoria, textoBadge.c_str(), static_cast<int>(textoBadge.size()), &tamanoTextoBadge);

            RECT areaPill = {
                areaSeccionHeader.left + 4,
                areaSeccionHeader.top + 3,
                areaSeccionHeader.left + 4 + tamanoTextoBadge.cx + 16,
                areaSeccionHeader.top + 3 + 22
            };

            HBRUSH brochaPill = CreateSolidBrush(RGB(30, 41, 59));
            HPEN lapizPill = CreatePen(PS_SOLID, 1, RGB(56, 139, 253));
            HBRUSH brochaPreviaPill = static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaPill));
            HPEN lapizPrevioPill = static_cast<HPEN>(SelectObject(contextoMemoria, lapizPill));
            RoundRect(contextoMemoria, areaPill.left, areaPill.top, areaPill.right, areaPill.bottom, 6, 6);
            SelectObject(contextoMemoria, lapizPrevioPill);
            SelectObject(contextoMemoria, brochaPreviaPill);
            DeleteObject(lapizPill);
            DeleteObject(brochaPill);

            SetBkMode(contextoMemoria, TRANSPARENT);
            SetTextColor(contextoMemoria, RGB(121, 192, 255));
            DrawTextW(contextoMemoria, textoBadge.c_str(), -1, &areaPill,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            int yLinea = areaSeccionHeader.top + 14;
            HPEN lapizDiv = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
            HPEN lapizPrevDiv = static_cast<HPEN>(SelectObject(contextoMemoria, lapizDiv));
            MoveToEx(contextoMemoria, areaPill.right + 10, yLinea, nullptr);
            LineTo(contextoMemoria, areaSeccionHeader.right - 4, yLinea);
            SelectObject(contextoMemoria, lapizPrevDiv);
            DeleteObject(lapizDiv);
        }

        SelectObject(contextoMemoria, fuenteItemArbol);
        for (auto& elementoActual : seccionActual.elementos) {
            int topeElem = posicionVerticalCursor;
            int fondoElem = posicionVerticalCursor + altoFilaElemento;
            posicionVerticalCursor += altoFilaElemento;

            RECT areaItem = {
                estadoComprimidos.areaCajaArbol.left + 16,
                topeElem,
                estadoComprimidos.areaCajaArbol.right - 80,
                fondoElem
            };
            elementoActual.areaElemento = areaItem;

            if (fondoElem >= estadoComprimidos.areaCajaArbol.top &&
                topeElem <= estadoComprimidos.areaCajaArbol.bottom) {

                std::wstring prefijo = elementoActual.esDirectorio ? L"📁 " : (seccionActual.iconoSeccion + L" ");
                std::wstring textoNombre = prefijo + elementoActual.rutaRelativa;

                SetBkMode(contextoMemoria, TRANSPARENT);
                SetTextColor(contextoMemoria, elementoActual.esDirectorio ? RGB(121, 192, 255) : RGB(225, 232, 240));
                DrawTextW(contextoMemoria, textoNombre.c_str(), -1, &areaItem,
                          DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                if (!elementoActual.esDirectorio && elementoActual.tamanoBytes > 0) {
                    std::wstring txtTamano = formatearTamanoArchivo(elementoActual.tamanoBytes);
                    RECT areaTam{
                        estadoComprimidos.areaCajaArbol.right - 78,
                        topeElem,
                        estadoComprimidos.areaCajaArbol.right - 12,
                        fondoElem
                    };
                    SetTextColor(contextoMemoria, RGB(139, 148, 158));
                    DrawTextW(contextoMemoria, txtTamano.c_str(), -1, &areaTam,
                              DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                }
            }
        }
        posicionVerticalCursor += 6;
    }

    SelectClipRgn(contextoMemoria, nullptr);
    DeleteObject(regionRecorte);
    DeleteObject(fuenteItemArbol);
    DeleteObject(fuenteTituloSeccion);
}

void dibujarControlesComprimidos(HDC contextoMemoria, HWND ventana,
                                 const RECT& areaCliente) {
    DisenoComprimidos diseno = calcularDisenoComprimidos(ventana);

    HFONT fuentePrevia =
        static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTexto));
    SetBkMode(contextoMemoria, TRANSPARENT);

    // --- CABECERA Y SEPARADOR DE OPERACIONES ---
    HPEN lapizSepOp = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevSepOp = static_cast<HPEN>(SelectObject(contextoMemoria, lapizSepOp));
    MoveToEx(contextoMemoria, diseno.areaSeparadorOperaciones.left, diseno.areaSeparadorOperaciones.top, nullptr);
    LineTo(contextoMemoria, diseno.areaSeparadorOperaciones.right, diseno.areaSeparadorOperaciones.top);
    SelectObject(contextoMemoria, lapizPrevSepOp);
    DeleteObject(lapizSepOp);

    SelectObject(contextoMemoria, fuenteTextoPequeno);
    SetTextColor(contextoMemoria, RGB(139, 148, 158));
    DrawTextW(contextoMemoria, L"⚡ OPERACIONES CON ARCHIVOS COMPRIMIDOS", -1,
              &diseno.areaTituloOperaciones,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // --- 1. FILA ABRIR CON ---
    SelectObject(contextoMemoria, fuenteTexto);
    SetTextColor(contextoMemoria, RGB(220, 220, 220));
    DrawTextW(contextoMemoria, L"Abrir con:", -1,
              &diseno.areaEtiquetaAbrirCon,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    HBRUSH brochaProg = CreateSolidBrush(
        estadoComprimidos.menuProgramasAbierto ? RGB(45, 45, 45)
                                               : RGB(35, 35, 35));
    HPEN lapizProg = CreatePen(PS_SOLID, 1, RGB(70, 70, 70));
    HBRUSH brochaPreviaProg =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaProg));
    HPEN lapizPrevioProg =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizProg));
    RoundRect(contextoMemoria, diseno.botonDesplegablePrograma.left,
              diseno.botonDesplegablePrograma.top,
              diseno.botonDesplegablePrograma.right,
              diseno.botonDesplegablePrograma.bottom, 6, 6);
    SelectObject(contextoMemoria, brochaPreviaProg);
    SelectObject(contextoMemoria, lapizPrevioProg);
    DeleteObject(brochaProg);
    DeleteObject(lapizProg);

    RECT areaTextoProg = diseno.botonDesplegablePrograma;
    areaTextoProg.left += 8;
    areaTextoProg.right -= 22;
    SetTextColor(contextoMemoria, RGB(240, 240, 240));

    std::wstring textoProgramaActual = L"Añade un programa";
    std::wstring rutaResuelta = resolverProgramaParaComprimidos();
    if (!rutaResuelta.empty()) {
        textoProgramaActual = PathFindFileNameW(rutaResuelta.c_str());
    }

    DrawTextW(contextoMemoria, textoProgramaActual.c_str(), -1,
              &areaTextoProg,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS |
                  DT_NOPREFIX);

    RECT areaFlechaProg = diseno.botonDesplegablePrograma;
    areaFlechaProg.left = areaFlechaProg.right - 20;
    SelectObject(contextoMemoria, fuenteIconos);
    SetTextColor(contextoMemoria, RGB(180, 180, 180));
    DrawTextW(contextoMemoria, GLIFO_CHEVRON_ARRIBA, -1, &areaFlechaProg,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    HBRUSH brochaAbrir = CreateSolidBrush(RGB(0, 120, 215));
    HPEN lapizAbrir = CreatePen(PS_SOLID, 1, RGB(0, 140, 240));
    HBRUSH brochaPreviaAbrir =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaAbrir));
    HPEN lapizPrevioAbrir =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizAbrir));
    RoundRect(contextoMemoria, diseno.botonEjecutarAbrir.left,
              diseno.botonEjecutarAbrir.top,
              diseno.botonEjecutarAbrir.right,
              diseno.botonEjecutarAbrir.bottom, 6, 6);
    SelectObject(contextoMemoria, brochaPreviaAbrir);
    SelectObject(contextoMemoria, lapizPrevioAbrir);
    DeleteObject(brochaAbrir);
    DeleteObject(lapizAbrir);

    SelectObject(contextoMemoria, fuenteTextoPequeno);
    SetTextColor(contextoMemoria, RGB(255, 255, 255));
    DrawTextW(contextoMemoria, L"Abrir", -1,
              &diseno.botonEjecutarAbrir,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // --- 2. FILA CONVERTIR A ---
    SelectObject(contextoMemoria, fuenteTexto);
    SetTextColor(contextoMemoria, RGB(220, 220, 220));
    DrawTextW(contextoMemoria, L"Convertir a:", -1,
              &diseno.areaEtiquetaConversion,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    HBRUSH brochaCombo = CreateSolidBrush(
        estadoComprimidos.menuDesplegableAbierto ? RGB(45, 45, 45)
                                                 : RGB(35, 35, 35));
    HPEN lapizCombo = CreatePen(PS_SOLID, 1, RGB(70, 70, 70));
    HBRUSH brochaPreviaCombo =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaCombo));
    HPEN lapizPrevioCombo =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizCombo));
    RoundRect(contextoMemoria, diseno.botonDesplegableFormato.left,
              diseno.botonDesplegableFormato.top,
              diseno.botonDesplegableFormato.right,
              diseno.botonDesplegableFormato.bottom, 6, 6);
    SelectObject(contextoMemoria, brochaPreviaCombo);
    SelectObject(contextoMemoria, lapizPrevioCombo);
    DeleteObject(brochaCombo);
    DeleteObject(lapizCombo);

    RECT areaTextoCombo = diseno.botonDesplegableFormato;
    areaTextoCombo.left += 8;
    areaTextoCombo.right -= 22;
    SetTextColor(contextoMemoria, RGB(240, 240, 240));
    DrawTextW(contextoMemoria,
              FORMATOS_DESTINO[estadoComprimidos.indiceFormatoDestino], -1,
              &areaTextoCombo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS |
                  DT_NOPREFIX);

    RECT areaFlechaCombo = diseno.botonDesplegableFormato;
    areaFlechaCombo.left = areaFlechaCombo.right - 20;
    SelectObject(contextoMemoria, fuenteIconos);
    SetTextColor(contextoMemoria, RGB(180, 180, 180));
    DrawTextW(contextoMemoria, GLIFO_CHEVRON_ARRIBA, -1, &areaFlechaCombo,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    HBRUSH brochaBoton = CreateSolidBrush(
        estadoComprimidos.conversionEnCurso ? RGB(0, 90, 160)
                                            : RGB(0, 120, 215));
    HPEN lapizBoton = CreatePen(
        PS_SOLID, 1,
        estadoComprimidos.conversionEnCurso ? RGB(0, 110, 180)
                                            : RGB(0, 140, 240));
    HBRUSH brochaPreviaBoton =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, brochaBoton));
    HPEN lapizPrevioBoton =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizBoton));
    RoundRect(contextoMemoria, diseno.botonEjecutarConversion.left,
              diseno.botonEjecutarConversion.top,
              diseno.botonEjecutarConversion.right,
              diseno.botonEjecutarConversion.bottom, 6, 6);
    SelectObject(contextoMemoria, brochaPreviaBoton);
    SelectObject(contextoMemoria, lapizPrevioBoton);
    DeleteObject(brochaBoton);
    DeleteObject(lapizBoton);

    SelectObject(contextoMemoria, fuenteTextoPequeno);
    SetTextColor(contextoMemoria, RGB(255, 255, 255));
    std::wstring textoBoton =
        estadoComprimidos.conversionEnCurso ? L"Convirtiendo..." : L"Convertir";
    DrawTextW(contextoMemoria, textoBoton.c_str(), -1,
              &diseno.botonEjecutarConversion,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // --- 3. PANELES FLOTANTES DESPLEGABLES ---

    SelectObject(contextoMemoria, fuentePrevia);
}

inline void dibujarBarraControlesPendiente(HDC contextoDibujo,
                                           const RECT& areaCliente,
                                           TipoContenidoArchivo tipo) {
    HBRUSH pincelFondo = CreateSolidBrush(RGB(18, 18, 18));
    FillRect(contextoDibujo, &areaCliente, pincelFondo);
    DeleteObject(pincelFondo);

    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(45, 45, 45));
    HPEN lapizPrevio = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    MoveToEx(contextoDibujo, areaCliente.left, areaCliente.top, nullptr);
    LineTo(contextoDibujo, areaCliente.right, areaCliente.top);
    SelectObject(contextoDibujo, lapizPrevio);
    DeleteObject(lapizBorde);

    EstadoPendienteImplementacion& estado =
        (tipo == TipoContenidoArchivo::Documento) ? estadoDocumentos : estadoCodigo;

    int anchoBoton = 84;
    int altoBoton = 24;
    int margenVertical = (areaCliente.bottom - areaCliente.top - altoBoton) / 2;

    estado.botonAnterior = {
        areaCliente.left + 12,
        areaCliente.top + margenVertical,
        areaCliente.left + 12 + anchoBoton,
        areaCliente.top + margenVertical + altoBoton
    };

    estado.botonSiguiente = {
        areaCliente.right - 12 - anchoBoton,
        areaCliente.top + margenVertical,
        areaCliente.right - 12,
        areaCliente.top + margenVertical + altoBoton
    };

    auto dibujarBotonNav = [&](const RECT& rect, const wchar_t* texto, bool hover) {
        COLORREF colorFondo = hover ? RGB(48, 54, 61) : RGB(33, 38, 45);
        COLORREF colorBorde = hover ? RGB(110, 118, 129) : RGB(48, 54, 61);
        HBRUSH brochaBtn = CreateSolidBrush(colorFondo);
        HPEN lapizBtn = CreatePen(PS_SOLID, 1, colorBorde);
        HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoDibujo, brochaBtn));
        HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBtn));
        RoundRect(contextoDibujo, rect.left, rect.top, rect.right, rect.bottom, 6, 6);
        SelectObject(contextoDibujo, lapizPrev);
        SelectObject(contextoDibujo, brochaPrev);
        DeleteObject(lapizBtn);
        DeleteObject(brochaBtn);

        SetBkMode(contextoDibujo, TRANSPARENT);
        SetTextColor(contextoDibujo, hover ? RGB(255, 255, 255) : RGB(201, 209, 217));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        RECT rTexto = rect;
        DrawTextW(contextoDibujo, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    };

    dibujarBotonNav(estado.botonAnterior, L"◀ Anterior", estado.hoverBotonAnterior);
    dibujarBotonNav(estado.botonSiguiente, L"Siguiente ▶", estado.hoverBotonSiguiente);

    std::wstring textoCentro;
    if (estado.archivosSeleccionados.size() > 1) {
        textoCentro = estado.nombreArchivo + L"  \u2022  " +
                      std::to_wstring(estado.indiceActual + 1) + L"/" +
                      std::to_wstring(estado.archivosSeleccionados.size());
    } else {
        textoCentro = estado.nombreArchivo;
    }
    if (!textoCentro.empty()) {
        RECT areaCentro = {
            estado.botonAnterior.right + 8,
            areaCliente.top,
            estado.botonSiguiente.left - 8,
            areaCliente.bottom
        };
        SetBkMode(contextoDibujo, TRANSPARENT);
        SetTextColor(contextoDibujo, RGB(139, 148, 158));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        DrawTextW(contextoDibujo, textoCentro.c_str(), -1, &areaCentro,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    }
}

inline void dibujarLienzoPendienteImplementacion(HDC contextoDestino,
                                                const RECT& areaDibujo,
                                                TipoContenidoArchivo tipo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTitulo = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubtitulo = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteTextoNormal = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteBadge = CreateFontW(-11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    HFONT fuenteOriginal = static_cast<HFONT>(GetCurrentObject(contextoDestino, OBJ_FONT));
    SetBkMode(contextoDestino, TRANSPARENT);

    int yActual = areaDibujo.top + 24;

    SelectObject(contextoDestino, fuenteTitulo);
    SetTextColor(contextoDestino, RGB(255, 255, 255));
    RECT areaTitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 28};
    DrawTextW(contextoDestino, L"Useful Sideview", -1, &areaTitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 32;

    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(88, 166, 255));
    RECT areaSubtitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 22};
    std::wstring etiquetaCategoria =
        (tipo == TipoContenidoArchivo::Documento)
            ? L"\U0001F4C4 Documentos (PDF / Word)"
            : L"\U0001F4BB Códigos de Programación";
    DrawTextW(contextoDestino, etiquetaCategoria.c_str(), -1, &areaSubtitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 34;

    EstadoPendienteImplementacion& estado =
        (tipo == TipoContenidoArchivo::Documento) ? estadoDocumentos : estadoCodigo;

    int altoTarjeta = 146;
    RECT areaTarjeta{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + altoTarjeta};
    HBRUSH brochaTarjeta = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizTarjeta = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaTarjeta));
    HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoDestino, lapizTarjeta));
    RoundRect(contextoDestino, areaTarjeta.left, areaTarjeta.top,
              areaTarjeta.right, areaTarjeta.bottom, 8, 8);
    SelectObject(contextoDestino, lapizPrev);
    SelectObject(contextoDestino, brochaPrev);
    DeleteObject(lapizTarjeta);
    DeleteObject(brochaTarjeta);

    int yCard = areaTarjeta.top + 14;

    // Nombre de archivo
    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaNombreArchivo{areaTarjeta.left + 16, yCard, areaTarjeta.right - 16, yCard + 20};
    DrawTextW(contextoDestino, estado.nombreArchivo.c_str(), -1, &areaNombreArchivo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    yCard += 26;

    // Badge "Pendiente de implementación"
    RECT areaBadge{areaTarjeta.left + 16, yCard, areaTarjeta.left + 215, yCard + 22};
    HBRUSH brochaBadge = CreateSolidBrush(RGB(56, 35, 12));
    HPEN lapizBadge = CreatePen(PS_SOLID, 1, RGB(187, 128, 9));
    HBRUSH brochaPrevB = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaBadge));
    HPEN lapizPrevB = static_cast<HPEN>(SelectObject(contextoDestino, lapizBadge));
    RoundRect(contextoDestino, areaBadge.left, areaBadge.top, areaBadge.right, areaBadge.bottom, 4, 4);
    SelectObject(contextoDestino, lapizPrevB);
    SelectObject(contextoDestino, brochaPrevB);
    DeleteObject(lapizBadge);
    DeleteObject(brochaBadge);

    SelectObject(contextoDestino, fuenteBadge);
    SetTextColor(contextoDestino, RGB(227, 179, 65));
    DrawTextW(contextoDestino, L"\u23F3 Pendiente de implementación", -1, &areaBadge,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yCard += 28;

    // Mensaje descriptivo
    SelectObject(contextoDestino, fuenteTextoNormal);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    std::wstring mensaje =
        (tipo == TipoContenidoArchivo::Documento)
            ? L"El visor para documentos PDF y paquetería de Microsoft Word está en desarrollo y se integrará en una próxima versión."
            : L"El visor con resaltado de sintaxis para lenguajes de programación está en desarrollo y se integrará en una próxima versión.";

    RECT areaMensaje{areaTarjeta.left + 16, yCard, areaTarjeta.right - 16, areaTarjeta.bottom - 12};
    DrawTextW(contextoDestino, mensaje.c_str(), -1, &areaMensaje,
              DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

    SelectObject(contextoDestino, fuenteOriginal);
    DeleteObject(fuenteTitulo);
    DeleteObject(fuenteSubtitulo);
    DeleteObject(fuenteTextoNormal);
    DeleteObject(fuenteBadge);
}

inline void dibujarBarraControlesDesconocido(HDC contextoDibujo,
                                             const RECT& areaCliente) {
    HBRUSH pincelFondo = CreateSolidBrush(RGB(18, 18, 18));
    FillRect(contextoDibujo, &areaCliente, pincelFondo);
    DeleteObject(pincelFondo);

    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(45, 45, 45));
    HPEN lapizPrevio = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBorde));
    MoveToEx(contextoDibujo, areaCliente.left, areaCliente.top, nullptr);
    LineTo(contextoDibujo, areaCliente.right, areaCliente.top);
    SelectObject(contextoDibujo, lapizPrevio);
    DeleteObject(lapizBorde);

    int anchoBoton = 84;
    int altoBoton = 24;
    int margenVertical = (areaCliente.bottom - areaCliente.top - altoBoton) / 2;

    estadoDesconocido.botonAnterior = {
        areaCliente.left + 12,
        areaCliente.top + margenVertical,
        areaCliente.left + 12 + anchoBoton,
        areaCliente.top + margenVertical + altoBoton
    };

    estadoDesconocido.botonSiguiente = {
        areaCliente.right - 12 - anchoBoton,
        areaCliente.top + margenVertical,
        areaCliente.right - 12,
        areaCliente.top + margenVertical + altoBoton
    };

    auto dibujarBotonNav = [&](const RECT& rect, const wchar_t* texto, bool hover) {
        COLORREF colorFondo = hover ? RGB(48, 54, 61) : RGB(33, 38, 45);
        COLORREF colorBorde = hover ? RGB(110, 118, 129) : RGB(48, 54, 61);
        HBRUSH brochaBtn = CreateSolidBrush(colorFondo);
        HPEN lapizBtn = CreatePen(PS_SOLID, 1, colorBorde);
        HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoDibujo, brochaBtn));
        HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoDibujo, lapizBtn));
        RoundRect(contextoDibujo, rect.left, rect.top, rect.right, rect.bottom, 6, 6);
        SelectObject(contextoDibujo, lapizPrev);
        SelectObject(contextoDibujo, brochaPrev);
        DeleteObject(lapizBtn);
        DeleteObject(brochaBtn);

        SetBkMode(contextoDibujo, TRANSPARENT);
        SetTextColor(contextoDibujo, hover ? RGB(255, 255, 255) : RGB(201, 209, 217));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        RECT rTexto = rect;
        DrawTextW(contextoDibujo, texto, -1, &rTexto, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    };

    dibujarBotonNav(estadoDesconocido.botonAnterior, L"◀ Anterior", estadoDesconocido.hoverBotonAnterior);
    dibujarBotonNav(estadoDesconocido.botonSiguiente, L"Siguiente ▶", estadoDesconocido.hoverBotonSiguiente);

    std::wstring textoCentro;
    if (estadoDesconocido.archivosSeleccionados.size() > 1) {
        textoCentro = estadoDesconocido.nombreArchivo + L"  \u2022  " +
                      std::to_wstring(estadoDesconocido.indiceActual + 1) + L"/" +
                      std::to_wstring(estadoDesconocido.archivosSeleccionados.size());
    } else {
        textoCentro = estadoDesconocido.nombreArchivo;
    }
    if (!textoCentro.empty()) {
        RECT areaCentro = {
            estadoDesconocido.botonAnterior.right + 8,
            areaCliente.top,
            estadoDesconocido.botonSiguiente.left - 8,
            areaCliente.bottom
        };
        SetBkMode(contextoDibujo, TRANSPARENT);
        SetTextColor(contextoDibujo, RGB(139, 148, 158));
        HFONT fuentePrev = static_cast<HFONT>(SelectObject(contextoDibujo, fuenteTextoPequeno));
        DrawTextW(contextoDibujo, textoCentro.c_str(), -1, &areaCentro,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        SelectObject(contextoDibujo, fuentePrev);
    }
}

inline void dibujarLienzoArchivoDesconocido(HDC contextoDestino,
                                           const RECT& areaDibujo) {
    int anchoArea = areaDibujo.right - areaDibujo.left;
    int altoArea = areaDibujo.bottom - areaDibujo.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaDibujo, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteTitulo = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubtitulo = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteTextoNormal = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteBadge = CreateFontW(-11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    HFONT fuenteOriginal = static_cast<HFONT>(GetCurrentObject(contextoDestino, OBJ_FONT));
    SetBkMode(contextoDestino, TRANSPARENT);

    int yActual = areaDibujo.top + 24;

    SelectObject(contextoDestino, fuenteTitulo);
    SetTextColor(contextoDestino, RGB(255, 255, 255));
    RECT areaTitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 28};
    DrawTextW(contextoDestino, L"Useful Sideview", -1, &areaTitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 32;

    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    RECT areaSubtitulo{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + 22};
    DrawTextW(contextoDestino, L"Archivo no compatible / desconocido", -1, &areaSubtitulo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yActual += 34;

    int altoTarjeta = 146;
    RECT areaTarjeta{areaDibujo.left + 24, yActual, areaDibujo.right - 24, yActual + altoTarjeta};
    HBRUSH brochaTarjeta = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizTarjeta = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPrev = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaTarjeta));
    HPEN lapizPrev = static_cast<HPEN>(SelectObject(contextoDestino, lapizTarjeta));
    RoundRect(contextoDestino, areaTarjeta.left, areaTarjeta.top,
              areaTarjeta.right, areaTarjeta.bottom, 8, 8);
    SelectObject(contextoDestino, lapizPrev);
    SelectObject(contextoDestino, brochaPrev);
    DeleteObject(lapizTarjeta);
    DeleteObject(brochaTarjeta);

    int yCard = areaTarjeta.top + 14;

    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaNombreArchivo{areaTarjeta.left + 16, yCard, areaTarjeta.right - 16, yCard + 20};
    DrawTextW(contextoDestino, estadoDesconocido.nombreArchivo.c_str(), -1, &areaNombreArchivo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    yCard += 26;

    // Badge "Archivo desconocido"
    RECT areaBadge{areaTarjeta.left + 16, yCard, areaTarjeta.left + 190, yCard + 22};
    HBRUSH brochaBadge = CreateSolidBrush(RGB(33, 38, 45));
    HPEN lapizBadge = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPrevB = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaBadge));
    HPEN lapizPrevB = static_cast<HPEN>(SelectObject(contextoDestino, lapizBadge));
    RoundRect(contextoDestino, areaBadge.left, areaBadge.top, areaBadge.right, areaBadge.bottom, 4, 4);
    SelectObject(contextoDestino, lapizPrevB);
    SelectObject(contextoDestino, brochaPrevB);
    DeleteObject(lapizBadge);
    DeleteObject(brochaBadge);

    SelectObject(contextoDestino, fuenteBadge);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    DrawTextW(contextoDestino, L"Archivo desconocido", -1, &areaBadge,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    yCard += 28;

    SelectObject(contextoDestino, fuenteTextoNormal);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    std::wstring mensaje = L"No hay una previsualización disponible para este tipo de archivo";
    if (!estadoDesconocido.tamanoFormateado.empty()) {
        mensaje += L" (" + estadoDesconocido.tamanoFormateado + L")";
    }
    mensaje += L".";

    RECT areaMensaje{areaTarjeta.left + 16, yCard, areaTarjeta.right - 16, areaTarjeta.bottom - 12};
    DrawTextW(contextoDestino, mensaje.c_str(), -1, &areaMensaje,
              DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

    SelectObject(contextoDestino, fuenteOriginal);
    DeleteObject(fuenteTitulo);
    DeleteObject(fuenteSubtitulo);
    DeleteObject(fuenteTextoNormal);
    DeleteObject(fuenteBadge);
}

void dibujarInterfaz(HWND ventana) {
    PAINTSTRUCT pintura;
    HDC contextoPantalla = BeginPaint(ventana, &pintura);
    DisenoBarraProgreso disenoBarraProgreso =
        calcularDisenoBarraProgreso(ventana);

    RECT areaCliente = disenoBarraProgreso.barraControles;
    int anchoCliente = areaCliente.right - areaCliente.left;
    int altoCliente = areaCliente.bottom - areaCliente.top;

    if (anchoCliente <= 0 || altoCliente <= 0) {
        EndPaint(ventana, &pintura);
        return;
    }

    HDC contextoMemoria = CreateCompatibleDC(contextoPantalla);
    HBITMAP mapaBitsMemoria =
        CreateCompatibleBitmap(contextoPantalla, anchoCliente, altoCliente);
    HBITMAP mapaBitsAnterior = static_cast<HBITMAP>(
        SelectObject(contextoMemoria, mapaBitsMemoria));

    HBRUSH pincelFondo = CreateSolidBrush(RGB(25, 25, 25));
    FillRect(contextoMemoria, &areaCliente, pincelFondo);
    DeleteObject(pincelFondo);
    SetBkMode(contextoMemoria, TRANSPARENT);
    SetTextColor(contextoMemoria, RGB(245, 245, 245));

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
        dibujarVistaTexto(contextoMemoria, areaCliente);
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);
        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento ||
        tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
        dibujarBarraControlesPendiente(contextoMemoria, areaCliente, tipoContenidoActivo.load());
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);
        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
        dibujarBarraControlesDesconocido(contextoMemoria, areaCliente);
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);
        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
        dibujarControlesComprimidos(contextoMemoria, ventana, areaCliente);
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);
        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
        dibujarBarraControlesImagen(contextoMemoria, areaCliente);
        BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
               contextoMemoria, 0, 0, SRCCOPY);

        SelectObject(contextoMemoria, mapaBitsAnterior);
        DeleteObject(mapaBitsMemoria);
        DeleteDC(contextoMemoria);
        EndPaint(ventana, &pintura);
        return;
    }

    MFP_MEDIAPLAYER_STATE estadoReproduccion = MFP_MEDIAPLAYER_STATE_EMPTY;
    BOOL sonidoSilenciado = TRUE;
    if (reproductor) {
        reproductor->GetState(&estadoReproduccion);
        reproductor->GetMute(&sonidoSilenciado);
    }
    LONGLONG posicionActual = 0;
    LONGLONG duracionTotal = 0;
    bool tiempoDisponible =
        obtenerTiempoReproduccionCache(&posicionActual, &duracionTotal);
    double progresoNormalizado =
        tiempoDisponible
            ? static_cast<double>(posicionActual) /
                  static_cast<double>(duracionTotal)
            : 0.0;
    if (arrastrandoBarraProgreso) {
        progresoNormalizado = proporcionProgreso;
        if (tiempoDisponible) {
            posicionActual = calcularPosicionTemporal(
                duracionTotal, proporcionProgreso);
        }
    } else if (cambioPosicionPendiente.load()) {
        progresoNormalizado = proporcionPosicionSolicitada.load();
        if (tiempoDisponible) {
            posicionActual = calcularPosicionTemporal(
                duracionTotal, progresoNormalizado);
        }
    }

    if (tiempoDisponible) {
        LONGLONG tiempoRestante =
            std::max(0LL, duracionTotal - posicionActual);
        std::wstring textoTranscurrido = formatearTiempo(posicionActual);
        std::wstring textoRestante =
            L"-" + formatearTiempo(tiempoRestante);
        RECT areaTranscurrido{disenoBarraProgreso.limiteIzquierdo, 5,
                              disenoBarraProgreso.limiteDerecho, 22};
        RECT areaRestante = areaTranscurrido;
        HFONT fuenteAnterior =
            static_cast<HFONT>(SelectObject(contextoMemoria, fuenteTexto));
        SetTextColor(contextoMemoria, RGB(210, 210, 210));
        DrawTextW(contextoMemoria, textoTranscurrido.c_str(), -1,
                  &areaTranscurrido,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        DrawTextW(contextoMemoria, textoRestante.c_str(), -1, &areaRestante,
                  DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(contextoMemoria, fuenteAnterior);
    }

    dibujarLinea(contextoMemoria, disenoBarraProgreso.limiteIzquierdo,
                 disenoBarraProgreso.limiteDerecho,
                 disenoBarraProgreso.centroVertical, 3,
                 tiempoDisponible && posicionamientoDisponible.load()
                     ? RGB(92, 92, 92)
                     : RGB(58, 58, 58));
    int posicionPerilla =
        calcularPosicionProgreso(disenoBarraProgreso, progresoNormalizado);
    dibujarLinea(contextoMemoria, disenoBarraProgreso.limiteIzquierdo,
                 posicionPerilla, disenoBarraProgreso.centroVertical, 4,
                 RGB(0, 120, 215));

    HBRUSH pincelPerilla = CreateSolidBrush(RGB(245, 245, 245));
    HBRUSH pincelPreviamenteSeleccionado =
        static_cast<HBRUSH>(SelectObject(contextoMemoria, pincelPerilla));
    HPEN lapizBorde = CreatePen(PS_SOLID, 1, RGB(0, 120, 215));
    HPEN lapizPreviamenteSeleccionado =
        static_cast<HPEN>(SelectObject(contextoMemoria, lapizBorde));
    Ellipse(contextoMemoria, posicionPerilla - RADIO_PERILLA_PROGRESO,
            disenoBarraProgreso.centroVertical - RADIO_PERILLA_PROGRESO,
            posicionPerilla + RADIO_PERILLA_PROGRESO + 1,
            disenoBarraProgreso.centroVertical + RADIO_PERILLA_PROGRESO + 1);
    SelectObject(contextoMemoria, lapizPreviamenteSeleccionado);
    SelectObject(contextoMemoria, pincelPreviamenteSeleccionado);
    DeleteObject(lapizBorde);
    DeleteObject(pincelPerilla);

    bool reproduciendo =
        estadoReproduccion == MFP_MEDIAPLAYER_STATE_PLAYING;
    bool parpadeoActivo = (GetTickCount64() - estadoVideoMetadatos.tiempoInicioParpadeoNav < 400);

    dibujarBotonControl(contextoMemoria,
                         disenoBarraProgreso.botonAnterior,
                         GLIFO_ANTERIOR, parpadeoActivo, false, parpadeoActivo);
    dibujarBotonControl(contextoMemoria,
                         disenoBarraProgreso.botonReproduccion,
                         reproduciendo ? GLIFO_PAUSAR : GLIFO_REPRODUCIR,
                         reproduciendo, true);
    dibujarBotonControl(contextoMemoria,
                         disenoBarraProgreso.botonSiguiente,
                         GLIFO_SIGUIENTE, parpadeoActivo, false, parpadeoActivo);
    dibujarBotonControl(contextoMemoria, disenoBarraProgreso.botonSonido,
                         sonidoSilenciado ? GLIFO_SILENCIO : GLIFO_VOLUMEN,
                         !sonidoSilenciado, false);
    dibujarBotonControl(contextoMemoria,
                         disenoBarraProgreso.botonRepeticion,
                         GLIFO_REPETICION, repeticionHabilitada.load(),
                         false);

    std::wstring etiquetaVelocidad =
        formatearEtiquetaVelocidad(velocidadReproduccionActual.load());
    dibujarBotonTexto(contextoMemoria, disenoBarraProgreso.botonVelocidad,
                      etiquetaVelocidad.c_str(), menuVelocidadDesplegado);

    BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente, contextoMemoria,
           0, 0, SRCCOPY);

    SelectObject(contextoMemoria, mapaBitsAnterior);
    DeleteObject(mapaBitsMemoria);
    DeleteDC(contextoMemoria);

    EndPaint(ventana, &pintura);
}

void dibujarCabeceraMetadatosVideo(HDC contextoDestino, const RECT& areaMetadatos) {
    int anchoArea = areaMetadatos.right - areaMetadatos.left;
    int altoArea = areaMetadatos.bottom - areaMetadatos.top;
    if (anchoArea <= 0 || altoArea <= 0) return;

    HBRUSH brochaFondo = CreateSolidBrush(RGB(13, 17, 23));
    FillRect(contextoDestino, &areaMetadatos, brochaFondo);
    DeleteObject(brochaFondo);

    HFONT fuenteBadge = CreateFontW(-11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteTitulo = CreateFontW(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT fuenteSubtitulo = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SetBkMode(contextoDestino, TRANSPARENT);

    int posicionVertical = areaMetadatos.top + 8;

    // 1. Badge "🎬 VIDEO" y contador de posición / rango
    SelectObject(contextoDestino, fuenteBadge);

    // Pill "🎬 VIDEO"
    std::wstring etiquetaTipo = L"🎬 VIDEO";
    SIZE tamanoTipo{};
    GetTextExtentPoint32W(contextoDestino, etiquetaTipo.c_str(), static_cast<int>(etiquetaTipo.size()), &tamanoTipo);
    RECT areaPillTipo = {
        areaMetadatos.left + 14,
        posicionVertical,
        areaMetadatos.left + 14 + tamanoTipo.cx + 14,
        posicionVertical + 20
    };
    HBRUSH brochaPillTipo = CreateSolidBrush(RGB(30, 41, 59));
    HPEN lapizPillTipo = CreatePen(PS_SOLID, 1, RGB(56, 139, 253));
    HBRUSH brochaPreviaPill = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaPillTipo));
    HPEN lapizPrevioPill = static_cast<HPEN>(SelectObject(contextoDestino, lapizPillTipo));
    RoundRect(contextoDestino, areaPillTipo.left, areaPillTipo.top, areaPillTipo.right, areaPillTipo.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioPill);
    SelectObject(contextoDestino, brochaPreviaPill);
    DeleteObject(lapizPillTipo);
    DeleteObject(brochaPillTipo);

    SetTextColor(contextoDestino, RGB(88, 166, 255));
    DrawTextW(contextoDestino, etiquetaTipo.c_str(), -1, &areaPillTipo,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // Pill contador (Rango vs Carpeta vs Único)
    std::wstring etiquetaContador;
    if (estadoVideoMetadatos.esRangoSeleccionado) {
        etiquetaContador = std::to_wstring(estadoVideoMetadatos.indiceEnRango + 1) + L"/" +
                           std::to_wstring(estadoVideoMetadatos.totalEnRango) + L" (Rango)";
    } else if (estadoVideoMetadatos.totalEnCarpeta > 1) {
        etiquetaContador = std::to_wstring(estadoVideoMetadatos.indiceEnCarpeta + 1) + L"/" +
                           std::to_wstring(estadoVideoMetadatos.totalEnCarpeta) + L" (Carpeta)";
    } else {
        etiquetaContador = L"1/1 (Único)";
    }

    SIZE tamanoContador{};
    GetTextExtentPoint32W(contextoDestino, etiquetaContador.c_str(), static_cast<int>(etiquetaContador.size()), &tamanoContador);
    RECT areaPillContador = {
        areaPillTipo.right + 8,
        posicionVertical,
        areaPillTipo.right + 8 + tamanoContador.cx + 14,
        posicionVertical + 20
    };
    HBRUSH brochaPillCont = CreateSolidBrush(RGB(22, 27, 34));
    HPEN lapizPillCont = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HBRUSH brochaPreviaCont = static_cast<HBRUSH>(SelectObject(contextoDestino, brochaPillCont));
    HPEN lapizPrevioCont = static_cast<HPEN>(SelectObject(contextoDestino, lapizPillCont));
    RoundRect(contextoDestino, areaPillContador.left, areaPillContador.top, areaPillContador.right, areaPillContador.bottom, 6, 6);
    SelectObject(contextoDestino, lapizPrevioCont);
    SelectObject(contextoDestino, brochaPreviaCont);
    DeleteObject(lapizPillCont);
    DeleteObject(brochaPillCont);

    SetTextColor(contextoDestino, RGB(139, 148, 158));
    DrawTextW(contextoDestino, etiquetaContador.c_str(), -1, &areaPillContador,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    posicionVertical += 25;

    // 2. Nombre del video
    SelectObject(contextoDestino, fuenteTitulo);
    SetTextColor(contextoDestino, RGB(240, 246, 252));
    RECT areaNombreVideo = {
        areaMetadatos.left + 14,
        posicionVertical,
        areaMetadatos.right - 14,
        posicionVertical + 22
    };
    std::wstring nombreMostrar = estadoVideoMetadatos.nombreArchivo.empty()
                                     ? PathFindFileNameW(rutaVideoActiva.c_str())
                                     : estadoVideoMetadatos.nombreArchivo;
    DrawTextW(contextoDestino, nombreMostrar.c_str(), -1, &areaNombreVideo,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    posicionVertical += 24;

    // 3. Metadatos secundarios (Tamaño, formato, fecha)
    SelectObject(contextoDestino, fuenteSubtitulo);
    SetTextColor(contextoDestino, RGB(139, 148, 158));
    std::wstring infoSecundaria;
    if (!estadoVideoMetadatos.textoTamano.empty()) {
        infoSecundaria += L"📦 " + estadoVideoMetadatos.textoTamano;
    }
    if (!estadoVideoMetadatos.extensionFormato.empty()) {
        if (!infoSecundaria.empty()) infoSecundaria += L"   •   ";
        infoSecundaria += L"🏷️ " + estadoVideoMetadatos.extensionFormato;
    }
    if (!estadoVideoMetadatos.textoFechaModificacion.empty()) {
        if (!infoSecundaria.empty()) infoSecundaria += L"   •   ";
        infoSecundaria += L"📅 " + estadoVideoMetadatos.textoFechaModificacion;
    }
    RECT areaInfoSecundaria = {
        areaMetadatos.left + 14,
        posicionVertical,
        areaMetadatos.right - 14,
        posicionVertical + 18
    };
    DrawTextW(contextoDestino, infoSecundaria.c_str(), -1, &areaInfoSecundaria,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    // Línea divisoria inferior
    int ySeparador = areaMetadatos.bottom - 1;
    HPEN lapizSeparador = CreatePen(PS_SOLID, 1, RGB(48, 54, 61));
    HPEN lapizPrevioSep = static_cast<HPEN>(SelectObject(contextoDestino, lapizSeparador));
    MoveToEx(contextoDestino, areaMetadatos.left + 14, ySeparador, nullptr);
    LineTo(contextoDestino, areaMetadatos.right - 14, ySeparador);
    SelectObject(contextoDestino, lapizPrevioSep);
    DeleteObject(lapizSeparador);

    DeleteObject(fuenteBadge);
    DeleteObject(fuenteTitulo);
    DeleteObject(fuenteSubtitulo);
}

/**** ended inlining interfaz/renderizado.cpp ****/
/**** start inlining interfaz/ventanas.cpp ****/
void ocultarMenuVelocidadInmediato() {
    if (ventanaMenuVelocidad && IsWindow(ventanaMenuVelocidad)) {
        ShowWindow(ventanaMenuVelocidad, SW_HIDE);
    }
    menuVelocidadDesplegado = false;
    animandoCierreMenuVelocidad = false;
    opacidadMenuVelocidad = 255;
    opcionVelocidadResaltada = -1;
    if (ventanaControles && IsWindow(ventanaControles)) {
        InvalidateRect(ventanaControles, nullptr, FALSE);
    }
}

void mostrarMenuVelocidadFlotante(HWND ventanaOrigen) {
    if (!ventanaMenuVelocidad || !IsWindow(ventanaMenuVelocidad)) return;

    DisenoBarraProgreso diseno = calcularDisenoBarraProgreso(ventanaOrigen);
    POINT puntoOrigen{diseno.botonVelocidad.right - ANCHO_PANEL_VELOCIDAD,
                      diseno.botonVelocidad.top - ALTO_PANEL_VELOCIDAD - 6};
    ClientToScreen(ventanaOrigen, &puntoOrigen);

    menuVelocidadDesplegado = true;
    opacidadMenuVelocidad = 255;
    SetLayeredWindowAttributes(ventanaMenuVelocidad, 0, 255, LWA_ALPHA);

    SetWindowPos(ventanaMenuVelocidad, HWND_TOPMOST, puntoOrigen.x,
                 puntoOrigen.y, ANCHO_PANEL_VELOCIDAD, ALTO_PANEL_VELOCIDAD,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);
    InvalidateRect(ventanaMenuVelocidad, nullptr, TRUE);
    InvalidateRect(ventanaOrigen, nullptr, FALSE);
}

void ajustarLayoutVentanasReproductor(HWND ventanaContenedora) {
    if (tipoContenidoActivo.load() != TipoContenidoArchivo::Video ||
        !IsWindowVisible(ventanaContenedora)) {
        ocultarMenuVelocidadInmediato();
    } else if (menuVelocidadDesplegado && ventanaMenuVelocidad && IsWindow(ventanaMenuVelocidad)) {
        if (ventanaControles && IsWindow(ventanaControles)) {
            DisenoBarraProgreso diseno = calcularDisenoBarraProgreso(ventanaControles);
            POINT puntoOrigen{diseno.botonVelocidad.right - ANCHO_PANEL_VELOCIDAD,
                              diseno.botonVelocidad.top - ALTO_PANEL_VELOCIDAD - 6};
            ClientToScreen(ventanaControles, &puntoOrigen);
            SetWindowPos(ventanaMenuVelocidad, HWND_TOPMOST, puntoOrigen.x, puntoOrigen.y,
                         ANCHO_PANEL_VELOCIDAD, ALTO_PANEL_VELOCIDAD,
                         SWP_NOACTIVATE | SWP_NOSIZE);
        }
    }
    RECT areaCliente;
    GetClientRect(ventanaContenedora, &areaCliente);
    int altoControlesEfectivo = ALTO_CONTROLES;
    bool controlesArriba = false;
    int altoMetadatosVideo = 0;

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
        altoMetadatosVideo = ALTO_METADATOS_VIDEO;
    } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
        altoControlesEfectivo = ALTO_CONTROLES_COMPRIMIDOS;
    } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
        altoControlesEfectivo = 34;
        controlesArriba = true;
    } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
        altoControlesEfectivo = 66;
    } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento ||
               tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo ||
               tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
        altoControlesEfectivo = 34;
    } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas ||
               tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
        altoControlesEfectivo = 0;
    }
    int altoAreaVideo =
        std::max<LONG>(0, areaCliente.bottom - altoMetadatosVideo - altoControlesEfectivo);

    int yControles = 0;
    int yVideo = 0;
    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
        yVideo = altoMetadatosVideo;
        yControles = areaCliente.bottom - altoControlesEfectivo;
    } else if (controlesArriba && altoControlesEfectivo > 0) {
        yControles = 0;
        yVideo = altoControlesEfectivo;
    } else {
        yVideo = 0;
        yControles = altoAreaVideo;
    }

    HDWP hdwp = BeginDeferWindowPos(2);
    if (hdwp && ventanaVideo && IsWindow(ventanaVideo)) {
        hdwp = DeferWindowPos(hdwp, ventanaVideo, nullptr, 0, yVideo, areaCliente.right,
                              altoAreaVideo,
                              SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_SHOWWINDOW);
    }
    if (hdwp && ventanaControles && IsWindow(ventanaControles)) {
        if (altoControlesEfectivo > 0) {
            hdwp = DeferWindowPos(hdwp, ventanaControles, HWND_TOP, 0, yControles,
                                  areaCliente.right, altoControlesEfectivo,
                                  SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS);
        }
    }
    if (hdwp) {
        EndDeferWindowPos(hdwp);
    }

    if (ventanaControles && IsWindow(ventanaControles) && altoControlesEfectivo <= 0) {
        ShowWindow(ventanaControles, SW_HIDE);
    }
    if (ventanaVideo && IsWindow(ventanaVideo)) {
        InvalidateRect(ventanaVideo, nullptr, FALSE);
    }
    if (ventanaControles && IsWindow(ventanaControles) && altoControlesEfectivo > 0) {
        InvalidateRect(ventanaControles, nullptr, FALSE);
    }
    if (reproductor) {
        reproductor->UpdateVideo();
    }
}

inline bool mostrarDocumentoAnterior() {
    if (estadoDocumentos.archivosSeleccionados.size() > 1) {
        estadoDocumentos.indiceActual =
            (estadoDocumentos.indiceActual == 0)
                ? (estadoDocumentos.archivosSeleccionados.size() - 1)
                : (estadoDocumentos.indiceActual - 1);
        estadoDocumentos.rutaActual =
            estadoDocumentos.archivosSeleccionados[estadoDocumentos.indiceActual];
        estadoDocumentos.nombreArchivo =
            PathFindFileNameW(estadoDocumentos.rutaActual.c_str());
        estadoDocumentos.extension =
            PathFindExtensionW(estadoDocumentos.rutaActual.c_str());
        return true;
    }
    if (estadoDocumentos.rutaActual.empty()) return false;
    std::wstring rutaAnterior = obtenerDocumentoAnteriorEnCarpeta(estadoDocumentos.rutaActual);
    if (rutaAnterior.empty() || rutaAnterior == estadoDocumentos.rutaActual) return false;
    estadoDocumentos.rutaActual = rutaAnterior;
    estadoDocumentos.nombreArchivo = PathFindFileNameW(estadoDocumentos.rutaActual.c_str());
    estadoDocumentos.extension = PathFindExtensionW(estadoDocumentos.rutaActual.c_str());
    estadoDocumentos.archivosSeleccionados = { rutaAnterior };
    estadoDocumentos.indiceActual = 0;
    return true;
}

inline bool mostrarDocumentoSiguiente() {
    if (estadoDocumentos.archivosSeleccionados.size() > 1) {
        estadoDocumentos.indiceActual =
            (estadoDocumentos.indiceActual + 1) %
            estadoDocumentos.archivosSeleccionados.size();
        estadoDocumentos.rutaActual =
            estadoDocumentos.archivosSeleccionados[estadoDocumentos.indiceActual];
        estadoDocumentos.nombreArchivo =
            PathFindFileNameW(estadoDocumentos.rutaActual.c_str());
        estadoDocumentos.extension =
            PathFindExtensionW(estadoDocumentos.rutaActual.c_str());
        return true;
    }
    if (estadoDocumentos.rutaActual.empty()) return false;
    std::wstring rutaSiguiente = obtenerDocumentoSiguienteEnCarpeta(estadoDocumentos.rutaActual);
    if (rutaSiguiente.empty() || rutaSiguiente == estadoDocumentos.rutaActual) return false;
    estadoDocumentos.rutaActual = rutaSiguiente;
    estadoDocumentos.nombreArchivo = PathFindFileNameW(estadoDocumentos.rutaActual.c_str());
    estadoDocumentos.extension = PathFindExtensionW(estadoDocumentos.rutaActual.c_str());
    estadoDocumentos.archivosSeleccionados = { rutaSiguiente };
    estadoDocumentos.indiceActual = 0;
    return true;
}

inline bool mostrarCodigoAnterior() {
    if (estadoCodigo.archivosSeleccionados.size() > 1) {
        estadoCodigo.indiceActual =
            (estadoCodigo.indiceActual == 0)
                ? (estadoCodigo.archivosSeleccionados.size() - 1)
                : (estadoCodigo.indiceActual - 1);
        estadoCodigo.rutaActual =
            estadoCodigo.archivosSeleccionados[estadoCodigo.indiceActual];
        estadoCodigo.nombreArchivo =
            PathFindFileNameW(estadoCodigo.rutaActual.c_str());
        estadoCodigo.extension =
            PathFindExtensionW(estadoCodigo.rutaActual.c_str());
        return true;
    }
    if (estadoCodigo.rutaActual.empty()) return false;
    std::wstring rutaAnterior = obtenerCodigoAnteriorEnCarpeta(estadoCodigo.rutaActual);
    if (rutaAnterior.empty() || rutaAnterior == estadoCodigo.rutaActual) return false;
    estadoCodigo.rutaActual = rutaAnterior;
    estadoCodigo.nombreArchivo = PathFindFileNameW(estadoCodigo.rutaActual.c_str());
    estadoCodigo.extension = PathFindExtensionW(estadoCodigo.rutaActual.c_str());
    estadoCodigo.archivosSeleccionados = { rutaAnterior };
    estadoCodigo.indiceActual = 0;
    return true;
}

inline bool mostrarCodigoSiguiente() {
    if (estadoCodigo.archivosSeleccionados.size() > 1) {
        estadoCodigo.indiceActual =
            (estadoCodigo.indiceActual + 1) %
            estadoCodigo.archivosSeleccionados.size();
        estadoCodigo.rutaActual =
            estadoCodigo.archivosSeleccionados[estadoCodigo.indiceActual];
        estadoCodigo.nombreArchivo =
            PathFindFileNameW(estadoCodigo.rutaActual.c_str());
        estadoCodigo.extension =
            PathFindExtensionW(estadoCodigo.rutaActual.c_str());
        return true;
    }
    if (estadoCodigo.rutaActual.empty()) return false;
    std::wstring rutaSiguiente = obtenerCodigoSiguienteEnCarpeta(estadoCodigo.rutaActual);
    if (rutaSiguiente.empty() || rutaSiguiente == estadoCodigo.rutaActual) return false;
    estadoCodigo.rutaActual = rutaSiguiente;
    estadoCodigo.nombreArchivo = PathFindFileNameW(estadoCodigo.rutaActual.c_str());
    estadoCodigo.extension = PathFindExtensionW(estadoCodigo.rutaActual.c_str());
    estadoCodigo.archivosSeleccionados = { rutaSiguiente };
    estadoCodigo.indiceActual = 0;
    return true;
}

inline bool mostrarDesconocidoAnterior() {
    if (estadoDesconocido.archivosSeleccionados.size() > 1) {
        estadoDesconocido.indiceActual =
            (estadoDesconocido.indiceActual == 0)
                ? (estadoDesconocido.archivosSeleccionados.size() - 1)
                : (estadoDesconocido.indiceActual - 1);
        estadoDesconocido.rutaActual =
            estadoDesconocido.archivosSeleccionados[estadoDesconocido.indiceActual];
        estadoDesconocido.nombreArchivo =
            PathFindFileNameW(estadoDesconocido.rutaActual.c_str());
        estadoDesconocido.extension =
            PathFindExtensionW(estadoDesconocido.rutaActual.c_str());
        WIN32_FILE_ATTRIBUTE_DATA datosArchivo{};
        if (GetFileAttributesExW(estadoDesconocido.rutaActual.c_str(), GetFileExInfoStandard, &datosArchivo)) {
            ULARGE_INTEGER tamano;
            tamano.LowPart = datosArchivo.nFileSizeLow;
            tamano.HighPart = datosArchivo.nFileSizeHigh;
            wchar_t bufTamano[64]{};
            StrFormatByteSizeW(static_cast<LONGLONG>(tamano.QuadPart), bufTamano, ARRAYSIZE(bufTamano));
            estadoDesconocido.tamanoFormateado = bufTamano;
        } else {
            estadoDesconocido.tamanoFormateado = L"";
        }
        return true;
    }
    return false;
}

inline bool mostrarDesconocidoSiguiente() {
    if (estadoDesconocido.archivosSeleccionados.size() > 1) {
        estadoDesconocido.indiceActual =
            (estadoDesconocido.indiceActual + 1) %
            estadoDesconocido.archivosSeleccionados.size();
        estadoDesconocido.rutaActual =
            estadoDesconocido.archivosSeleccionados[estadoDesconocido.indiceActual];
        estadoDesconocido.nombreArchivo =
            PathFindFileNameW(estadoDesconocido.rutaActual.c_str());
        estadoDesconocido.extension =
            PathFindExtensionW(estadoDesconocido.rutaActual.c_str());
        WIN32_FILE_ATTRIBUTE_DATA datosArchivo{};
        if (GetFileAttributesExW(estadoDesconocido.rutaActual.c_str(), GetFileExInfoStandard, &datosArchivo)) {
            ULARGE_INTEGER tamano;
            tamano.LowPart = datosArchivo.nFileSizeLow;
            tamano.HighPart = datosArchivo.nFileSizeHigh;
            wchar_t bufTamano[64]{};
            StrFormatByteSizeW(static_cast<LONGLONG>(tamano.QuadPart), bufTamano, ARRAYSIZE(bufTamano));
            estadoDesconocido.tamanoFormateado = bufTamano;
        } else {
            estadoDesconocido.tamanoFormateado = L"";
        }
        return true;
    }
    return false;
}

LRESULT CALLBACK procesarVentanaDelegadoReproductor(HWND ventana, UINT mensaje,
                                                    WPARAM parametroMensaje,
                                                    LPARAM datosMensaje) {
    if (mensaje == MENSAJE_EJECUTAR_BUSQUEDA) {
        return buscarEnReproductorDesdeHilo(
                   proporcionPosicionSolicitada.load(),
                   reproducirTrasCambioPosicion.load())
                   ? 1
                   : 0;
    }
    if (mensaje == MENSAJE_ACTUALIZAR_CACHE_TIEMPO) {
        actualizarCacheTiempoReproduccion();
        return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

inline void invalidarInterfaz() {
    if (ventanaControles && IsWindow(ventanaControles)) {
        InvalidateRect(ventanaControles, nullptr, FALSE);
        UpdateWindow(ventanaControles);
    }
    if (ventanaVideo && IsWindow(ventanaVideo)) {
        InvalidateRect(ventanaVideo, nullptr, FALSE);
        UpdateWindow(ventanaVideo);
    }
}

LRESULT CALLBACK procesarVentanaControles(HWND ventana, UINT mensaje,
                                          WPARAM parametroMensaje,
                                          LPARAM datosMensaje) {
    switch (mensaje) {
        case WM_LBUTTONDOWN: {
            int posicionHorizontal = GET_X_LPARAM(datosMensaje);
            int posicionVertical = GET_Y_LPARAM(datosMensaje);

            if (tipoContenidoActivo.load() ==
                TipoContenidoArchivo::Comprimido) {
                DisenoComprimidos disenoComprimidos =
                    calcularDisenoComprimidos(ventana);

                if (contienePunto(
                        disenoComprimidos.botonDesplegablePrograma,
                        posicionHorizontal, posicionVertical)) {
                    mostrarMenuDesplegableProgramasComprimidos(
                        ventana, disenoComprimidos.botonDesplegablePrograma);
                    return 0;
                }

                if (contienePunto(
                        disenoComprimidos.botonEjecutarAbrir,
                        posicionHorizontal, posicionVertical)) {
                    ejecutarAbrirArchivosConPrograma(ventana);
                    invalidarInterfaz();
                    return 0;
                }

                if (contienePunto(
                        disenoComprimidos.botonDesplegableFormato,
                        posicionHorizontal, posicionVertical)) {
                    mostrarMenuDesplegableFormatosComprimidos(
                        ventana, disenoComprimidos.botonDesplegableFormato);
                    return 0;
                }

                if (contienePunto(
                        disenoComprimidos.botonEjecutarConversion,
                        posicionHorizontal, posicionVertical)) {
                    ejecutarConversionArchivos();
                    invalidarInterfaz();
                    return 0;
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoTexto.botonAnterior, puntoRaton)) {
                    if (mostrarTextoAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoTexto.botonSiguiente, puntoRaton)) {
                    if (mostrarTextoSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoTexto.botonEditar, puntoRaton)) {
                    if (!estadoTexto.rutaTextoActual.empty()) {
                        HINSTANCE res = ShellExecuteW(nullptr, L"edit", estadoTexto.rutaTextoActual.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                        if (reinterpret_cast<INT_PTR>(res) <= 32) {
                            ShellExecuteW(nullptr, L"open", estadoTexto.rutaTextoActual.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                        }
                    }
                    return 0;
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoDocumentos.botonAnterior, puntoRaton)) {
                    if (mostrarDocumentoAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoDocumentos.botonSiguiente, puntoRaton)) {
                    if (mostrarDocumentoSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoCodigo.botonAnterior, puntoRaton)) {
                    if (mostrarCodigoAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoCodigo.botonSiguiente, puntoRaton)) {
                    if (mostrarCodigoSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoDesconocido.botonAnterior, puntoRaton)) {
                    if (mostrarDesconocidoAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoDesconocido.botonSiguiente, puntoRaton)) {
                    if (mostrarDesconocidoSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                return 0;
            }

            DisenoBarraProgreso disenoBarraProgreso =
                calcularDisenoBarraProgreso(ventana);

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                if (PtInRect(&estadoImagen.botonAnterior, puntoRaton)) {
                    if (mostrarImagenAnterior()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoImagen.botonSiguiente, puntoRaton)) {
                    if (mostrarImagenSiguiente()) {
                        invalidarInterfaz();
                    }
                    return 0;
                }
                if (PtInRect(&estadoImagen.botonGaleria, puntoRaton)) {
                    estadoImagen.modoGaleria = !estadoImagen.modoGaleria;
                    if (estadoImagen.modoGaleria) {
                        actualizarListaGaleria();
                    }
                    invalidarInterfaz();
                    return 0;
                }
                for (size_t posicionAccion = 0; posicionAccion < estadoImagen.accionesDisponibles.size(); ++posicionAccion) {
                    if (PtInRect(&estadoImagen.accionesDisponibles[posicionAccion].areaBoton, puntoRaton)) {
                        if (ejecutarAccionImagen(estadoImagen.accionesDisponibles[posicionAccion].id, ventana)) {
                            invalidarInterfaz();
                        }
                        return 0;
                    }
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
                if (contienePunto(disenoBarraProgreso.botonVelocidad,
                                  posicionHorizontal, posicionVertical)) {
                    if (menuVelocidadDesplegado) {
                        ocultarMenuVelocidadInmediato();
                    } else {
                        mostrarMenuVelocidadFlotante(ventana);
                    }
                    return 0;
                }
                if (menuVelocidadDesplegado) {
                    ocultarMenuVelocidadInmediato();
                }
            }

            if (contienePunto(disenoBarraProgreso.botonAnterior,
                              posicionHorizontal, posicionVertical)) {
                reproducirVideoAnterior();
                return 0;
            }
            if (contienePunto(disenoBarraProgreso.botonReproduccion,
                              posicionHorizontal, posicionVertical)) {
                alternarReproduccion();
                return 0;
            }
            if (contienePunto(disenoBarraProgreso.botonSiguiente,
                              posicionHorizontal, posicionVertical)) {
                reproducirVideoSiguiente();
                return 0;
            }
            if (contienePunto(disenoBarraProgreso.botonSonido,
                              posicionHorizontal, posicionVertical)) {
                alternarSonido();
                return 0;
            }
            if (contienePunto(disenoBarraProgreso.botonRepeticion,
                              posicionHorizontal, posicionVertical)) {
                alternarRepeticion();
                return 0;
            }

            if (esClicEnBarraProgreso(disenoBarraProgreso, posicionHorizontal,
                                      posicionVertical)) {
                iniciarArrastreBarraProgreso(ventana, posicionHorizontal);
                return 0;
            }
            return 0;
        }
        case WM_LBUTTONDBLCLK: {
            int posicionHorizontal = GET_X_LPARAM(datosMensaje);
            int posicionVertical = GET_Y_LPARAM(datosMensaje);

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
                DisenoBarraProgreso disenoBarraProgreso =
                    calcularDisenoBarraProgreso(ventana);
                if (esClicEnBarraProgreso(disenoBarraProgreso, posicionHorizontal,
                                          posicionVertical)) {
                    proporcionProgreso = calcularProporcionProgreso(
                        disenoBarraProgreso, posicionHorizontal);
                    buscarEnReproduccion(proporcionProgreso);
                    arrastrandoBarraProgreso = false;
                    if (GetCapture() == ventana) ReleaseCapture();
                    invalidarInterfazControles();
                    return 0;
                }
            }

            return procesarVentanaControles(ventana, WM_LBUTTONDOWN, parametroMensaje, datosMensaje);
        }
        case WM_MOUSEMOVE: {
            int posicionHorizontal = GET_X_LPARAM(datosMensaje);
            int posicionVertical = GET_Y_LPARAM(datosMensaje);

            if (arrastrandoBarraProgreso) {
                if (!(parametroMensaje & MK_LBUTTON) || GetCapture() != ventana) {
                    finalizarArrastreBarraProgreso(ventana, true);
                } else {
                    actualizarArrastreBarraProgreso(ventana, posicionHorizontal);
                    return 0;
                }
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT),
                                                  TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);

                POINT puntoRaton{posicionHorizontal, posicionVertical};
                bool hoverAnt =
                    PtInRect(&estadoTexto.botonAnterior, puntoRaton);
                bool hoverSig =
                    PtInRect(&estadoTexto.botonSiguiente, puntoRaton);
                if (hoverAnt != estadoTexto.hoverBotonAnterior ||
                    hoverSig != estadoTexto.hoverBotonSiguiente) {
                    estadoTexto.hoverBotonAnterior = hoverAnt;
                    estadoTexto.hoverBotonSiguiente = hoverSig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT pt{posicionHorizontal, posicionVertical};
                bool ant = PtInRect(&estadoDocumentos.botonAnterior, pt) != FALSE;
                bool sig = PtInRect(&estadoDocumentos.botonSiguiente, pt) != FALSE;
                if (ant != estadoDocumentos.hoverBotonAnterior || sig != estadoDocumentos.hoverBotonSiguiente) {
                    estadoDocumentos.hoverBotonAnterior = ant;
                    estadoDocumentos.hoverBotonSiguiente = sig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT pt{posicionHorizontal, posicionVertical};
                bool ant = PtInRect(&estadoCodigo.botonAnterior, pt) != FALSE;
                bool sig = PtInRect(&estadoCodigo.botonSiguiente, pt) != FALSE;
                if (ant != estadoCodigo.hoverBotonAnterior || sig != estadoCodigo.hoverBotonSiguiente) {
                    estadoCodigo.hoverBotonAnterior = ant;
                    estadoCodigo.hoverBotonSiguiente = sig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                bool hoverBotonAnteriorDetectado = PtInRect(&estadoImagen.botonAnterior, puntoRaton) != FALSE;
                bool hoverBotonSiguienteDetectado = PtInRect(&estadoImagen.botonSiguiente, puntoRaton) != FALSE;
                bool hoverBotonGaleriaDetectado = PtInRect(&estadoImagen.botonGaleria, puntoRaton) != FALSE;
                bool cambioHover = (hoverBotonAnteriorDetectado != estadoImagen.hoverBotonAnterior ||
                                    hoverBotonSiguienteDetectado != estadoImagen.hoverBotonSiguiente ||
                                    hoverBotonGaleriaDetectado != estadoImagen.hoverBotonGaleria);
                estadoImagen.hoverBotonAnterior = hoverBotonAnteriorDetectado;
                estadoImagen.hoverBotonSiguiente = hoverBotonSiguienteDetectado;
                estadoImagen.hoverBotonGaleria = hoverBotonGaleriaDetectado;

                for (size_t posicionAccion = 0; posicionAccion < estadoImagen.accionesDisponibles.size(); ++posicionAccion) {
                    bool estaSobreBotonAccion = PtInRect(&estadoImagen.accionesDisponibles[posicionAccion].areaBoton, puntoRaton) != FALSE;
                    if (estaSobreBotonAccion != estadoImagen.accionesDisponibles[posicionAccion].hover) {
                        estadoImagen.accionesDisponibles[posicionAccion].hover = estaSobreBotonAccion;
                        cambioHover = true;
                    }
                }
                if (cambioHover) {
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT puntoRaton{posicionHorizontal, posicionVertical};
                bool ant = PtInRect(&estadoDesconocido.botonAnterior, puntoRaton) != FALSE;
                bool sig = PtInRect(&estadoDesconocido.botonSiguiente, puntoRaton) != FALSE;
                if (ant != estadoDesconocido.hoverBotonAnterior || sig != estadoDesconocido.hoverBotonSiguiente) {
                    estadoDesconocido.hoverBotonAnterior = ant;
                    estadoDesconocido.hoverBotonSiguiente = sig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }

            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT)};
            eventoSeguimiento.dwFlags = TME_LEAVE;
            eventoSeguimiento.hwndTrack = ventana;
            TrackMouseEvent(&eventoSeguimiento);
            return 0;
        }
        case WM_MOUSELEAVE:
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                if (estadoTexto.hoverBotonAnterior ||
                    estadoTexto.hoverBotonSiguiente) {
                    estadoTexto.hoverBotonAnterior = false;
                    estadoTexto.hoverBotonSiguiente = false;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                if (estadoDocumentos.hoverBotonAnterior ||
                    estadoDocumentos.hoverBotonSiguiente) {
                    estadoDocumentos.hoverBotonAnterior = false;
                    estadoDocumentos.hoverBotonSiguiente = false;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                if (estadoCodigo.hoverBotonAnterior ||
                    estadoCodigo.hoverBotonSiguiente) {
                    estadoCodigo.hoverBotonAnterior = false;
                    estadoCodigo.hoverBotonSiguiente = false;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                bool cambioHover = estadoImagen.hoverBotonAnterior ||
                                   estadoImagen.hoverBotonSiguiente ||
                                   estadoImagen.hoverBotonGaleria;
                estadoImagen.hoverBotonAnterior = false;
                estadoImagen.hoverBotonSiguiente = false;
                estadoImagen.hoverBotonGaleria = false;
                for (size_t posicionAccion = 0; posicionAccion < estadoImagen.accionesDisponibles.size(); ++posicionAccion) {
                    if (estadoImagen.accionesDisponibles[posicionAccion].hover) {
                        estadoImagen.accionesDisponibles[posicionAccion].hover = false;
                        cambioHover = true;
                    }
                }
                if (cambioHover) {
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
                if (estadoDesconocido.hoverBotonAnterior ||
                    estadoDesconocido.hoverBotonSiguiente) {
                    estadoDesconocido.hoverBotonAnterior = false;
                    estadoDesconocido.hoverBotonSiguiente = false;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
            return 0;
        case WM_SETCURSOR: {
            POINT puntoCursor;
            GetCursorPos(&puntoCursor);
            RECT rectControles;
            GetWindowRect(ventana, &rectControles);
            if (puntoCursor.x >= rectControles.left && puntoCursor.x <= rectControles.left + 8) {
                SetCursor(LoadCursorW(nullptr, IDC_SIZEWE));
                return TRUE;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                DisenoBarraProgreso diseno = calcularDisenoBarraProgreso(ventana);
                if (esClicEnBarraProgreso(diseno, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonReproduccion, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonAnterior, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonSiguiente, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonSonido, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonRepeticion, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonVelocidad, puntoRaton.x, puntoRaton.y)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoTexto.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoTexto.botonSiguiente, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoDocumentos.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoDocumentos.botonSiguiente, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoCodigo.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoCodigo.botonSiguiente, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoImagen.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoImagen.botonSiguiente, puntoRaton) ||
                    PtInRect(&estadoImagen.botonGaleria, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
                for (size_t posicionAccion = 0; posicionAccion < estadoImagen.accionesDisponibles.size(); ++posicionAccion) {
                    if (PtInRect(&estadoImagen.accionesDisponibles[posicionAccion].areaBoton, puntoRaton)) {
                        SetCursor(LoadCursorW(nullptr, IDC_HAND));
                        return TRUE;
                    }
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                DisenoComprimidos diseno = calcularDisenoComprimidos(ventana);
                if (contienePunto(diseno.botonDesplegablePrograma, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonEjecutarAbrir, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonDesplegableFormato, puntoRaton.x, puntoRaton.y) ||
                    contienePunto(diseno.botonEjecutarConversion, puntoRaton.x, puntoRaton.y)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Desconocido) {
                POINT puntoRaton;
                GetCursorPos(&puntoRaton);
                ScreenToClient(ventana, &puntoRaton);
                if (PtInRect(&estadoDesconocido.botonAnterior, puntoRaton) ||
                    PtInRect(&estadoDesconocido.botonSiguiente, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
            break;
        }
        case WM_LBUTTONUP:
            if (arrastrandoBarraProgreso) {
                finalizarArrastreBarraProgreso(ventana, true);
            }
            return 0;
        case WM_CAPTURECHANGED:
        case WM_CANCELMODE:
        case WM_KILLFOCUS:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            if (arrastrandoBarraProgreso) {
                finalizarArrastreBarraProgreso(ventana, false);
            }
            return 0;
        case WM_MOUSEWHEEL:
            break;
        case WM_KEYDOWN:
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                if (parametroMensaje == VK_LEFT || parametroMensaje == L'P') {
                    if (mostrarTextoAnterior()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_RIGHT || parametroMensaje == L'N') {
                    if (mostrarTextoSiguiente()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_UP) {
                    desplazarScrollTexto(-40);
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_DOWN ||
                    parametroMensaje == VK_SPACE) {
                    desplazarScrollTexto(40);
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_PRIOR) {
                    desplazarScrollTexto(-240);
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_NEXT) {
                    desplazarScrollTexto(240);
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_HOME) {
                    estadoTexto.desplazamientoScrollY = 0;
                    invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_END) {
                    estadoTexto.desplazamientoScrollY =
                        std::max(0, estadoTexto.altoTotalContenido - 200);
                    invalidarInterfaz();
                    return 0;
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Documento) {
                if (parametroMensaje == VK_LEFT ||
                    parametroMensaje == VK_UP ||
                    parametroMensaje == L'P') {
                    if (mostrarDocumentoAnterior()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_RIGHT ||
                    parametroMensaje == VK_DOWN ||
                    parametroMensaje == VK_SPACE ||
                    parametroMensaje == L'N') {
                    if (mostrarDocumentoSiguiente()) invalidarInterfaz();
                    return 0;
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Codigo) {
                if (parametroMensaje == VK_LEFT ||
                    parametroMensaje == VK_UP ||
                    parametroMensaje == L'P') {
                    if (mostrarCodigoAnterior()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_RIGHT ||
                    parametroMensaje == VK_DOWN ||
                    parametroMensaje == VK_SPACE ||
                    parametroMensaje == L'N') {
                    if (mostrarCodigoSiguiente()) invalidarInterfaz();
                    return 0;
                }
                return 0;
            }
            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
                if (parametroMensaje == VK_LEFT ||
                    parametroMensaje == VK_UP ||
                    parametroMensaje == L'P') {
                    if (mostrarImagenAnterior()) invalidarInterfaz();
                    return 0;
                }
                if (parametroMensaje == VK_RIGHT ||
                    parametroMensaje == VK_DOWN ||
                    parametroMensaje == VK_SPACE ||
                    parametroMensaje == L'N') {
                    if (mostrarImagenSiguiente()) invalidarInterfaz();
                    return 0;
                }
                return 0;
            }

            if (parametroMensaje == VK_LEFT || parametroMensaje == VK_DOWN) {
                desplazarBarraProgresoConTeclado(
                    -PASO_BARRA_PROGRESO_TECLADO);
                return 0;
            }
            if (parametroMensaje == VK_RIGHT || parametroMensaje == VK_UP) {
                desplazarBarraProgresoConTeclado(
                    PASO_BARRA_PROGRESO_TECLADO);
                return 0;
            }
            if (parametroMensaje == VK_HOME) {
                buscarEnReproduccion(0.0);
                return 0;
            }
            if (parametroMensaje == VK_END) {
                buscarEnReproduccion(1.0);
                return 0;
            }
            if (parametroMensaje == VK_SPACE) {
                alternarReproduccion();
                return 0;
            }
            if (parametroMensaje == L'M') {
                alternarSonido();
                return 0;
            }
            if (parametroMensaje == L'L') {
                alternarRepeticion();
                return 0;
            }
            if (parametroMensaje == L'P') {
                reproducirVideoAnterior();
                return 0;
            }
            if (parametroMensaje == L'N') {
                reproducirVideoSiguiente();
                return 0;
            }
        case WM_NCHITTEST: {
            POINT puntoPantalla{GET_X_LPARAM(datosMensaje),
                                GET_Y_LPARAM(datosMensaje)};
            RECT rectVentana;
            GetWindowRect(ventana, &rectVentana);
            if (puntoPantalla.x >= rectVentana.left &&
                puntoPantalla.x <= rectVentana.left + 6) {
                return HTTRANSPARENT;
            }
            break;
        }
        case WM_GETDLGCODE:
            return DLGC_WANTARROWS | DLGC_WANTCHARS;
        case WM_TIMER:
            if (parametroMensaje == ID_TEMPORIZADOR_PARPADEO_NAV) {
                KillTimer(ventana, ID_TEMPORIZADOR_PARPADEO_NAV);
            }
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case WM_SIZE:
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case MENSAJE_ACTUALIZAR_INTERFAZ:
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            dibujarInterfaz(ventana);
            return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaReproductor(HWND ventana, UINT mensaje,
                                             WPARAM parametroMensaje,
                                             LPARAM datosMensaje) {
    if (mensaje == WM_ERASEBKGND) {
        return 1;
    }
    if (mensaje == WM_NCHITTEST) {
        POINT puntoPantalla{GET_X_LPARAM(datosMensaje),
                            GET_Y_LPARAM(datosMensaje)};
        RECT rectVentana;
        GetWindowRect(ventana, &rectVentana);
        if (puntoPantalla.x >= rectVentana.left &&
            puntoPantalla.x <= rectVentana.left + 6) {
            return HTTRANSPARENT;
        }
    }
    if (mensaje == WM_SIZE) {
        ajustarLayoutVentanasReproductor(ventana);
        return 0;
    }
    if (mensaje == WM_PAINT) {
        PAINTSTRUCT pintura;
        HDC contextoPantalla = BeginPaint(ventana, &pintura);
        RECT areaCliente;
        GetClientRect(ventana, &areaCliente);
        int anchoCliente = areaCliente.right - areaCliente.left;
        int altoCliente = areaCliente.bottom - areaCliente.top;
        if (anchoCliente > 0 && altoCliente > 0) {
            HDC contextoMemoria = CreateCompatibleDC(contextoPantalla);
            HBITMAP mapaBitsMemoria = CreateCompatibleBitmap(contextoPantalla, anchoCliente, altoCliente);
            HBITMAP mapaBitsAnterior = static_cast<HBITMAP>(SelectObject(contextoMemoria, mapaBitsMemoria));

            HBRUSH brochaFondo = CreateSolidBrush(COLOR_FONDO_REPRODUCTOR);
            FillRect(contextoMemoria, &areaCliente, brochaFondo);
            DeleteObject(brochaFondo);

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Video) {
                RECT areaMetadatos{0, 0, anchoCliente, std::min<int>(altoCliente, ALTO_METADATOS_VIDEO)};
                dibujarCabeceraMetadatosVideo(contextoMemoria, areaMetadatos);
            }

            BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente, contextoMemoria, 0, 0, SRCCOPY);
            SelectObject(contextoMemoria, mapaBitsAnterior);
            DeleteObject(mapaBitsMemoria);
            DeleteDC(contextoMemoria);
        }
        EndPaint(ventana, &pintura);
        return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaVideo(HWND ventana, UINT mensaje,
                                      WPARAM parametroMensaje,
                                      LPARAM datosMensaje) {
    if (mensaje == WM_ERASEBKGND) {
        return 1;
    }
    if (mensaje == WM_NCHITTEST) {
        POINT puntoPantalla{GET_X_LPARAM(datosMensaje),
                            GET_Y_LPARAM(datosMensaje)};
        RECT rectVentana;
        GetWindowRect(ventana, &rectVentana);
        if (puntoPantalla.x >= rectVentana.left &&
            puntoPantalla.x <= rectVentana.left + 6) {
            return HTTRANSPARENT;
        }
    }
    if (mensaje == WM_SIZE) {
        InvalidateRect(ventana, nullptr, FALSE);
        return 0;
    }
    if (mensaje == WM_MOUSEWHEEL) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            short deltaRueda = GET_WHEEL_DELTA_WPARAM(parametroMensaje);
            desplazarScrollTexto(deltaRueda > 0 ? -60 : 60);
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            short deltaRueda = GET_WHEEL_DELTA_WPARAM(parametroMensaje);
            int incremento = (deltaRueda > 0 ? -44 : 44);
            int maxScroll = std::max(0, estadoVistaCarpetas.altoTotalArbol - 100);
            estadoVistaCarpetas.desplazamientoScrollY = std::max(
                0, std::min(estadoVistaCarpetas.desplazamientoScrollY + incremento, maxScroll));
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen && estadoImagen.modoGaleria) {
            short deltaRueda = GET_WHEEL_DELTA_WPARAM(parametroMensaje);
            RECT rc;
            GetClientRect(ventana, &rc);
            int altoMax = std::max(0, estadoImagen.altoTotalGaleria - static_cast<int>(rc.bottom - rc.top));
            estadoImagen.desplazamientoScrollGaleria =
                std::max(0, std::min(estadoImagen.desplazamientoScrollGaleria - (deltaRueda > 0 ? 50 : -50), altoMax));
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
            if (!estadoComprimidos.esModoComic) {
                short deltaRueda = GET_WHEEL_DELTA_WPARAM(parametroMensaje);
                int incremento = (deltaRueda > 0 ? -44 : 44);
                desplazarScrollComprimido(incremento);
                InvalidateRect(ventana, nullptr, FALSE);
                return 0;
            }
        }
    }
    if (mensaje == WM_MOUSEMOVE) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
            TrackMouseEvent(&eventoSeguimiento);
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            bool hoverAnterior = PtInRect(&estadoVistaCarpetas.botonAnteriorCarpeta, pt) != FALSE;
            bool hoverSiguiente = PtInRect(&estadoVistaCarpetas.botonSiguienteCarpeta, pt) != FALSE;
            bool hoverExpansion = PtInRect(&estadoVistaCarpetas.botonAlternarExpansion, pt) != FALSE;
            bool hoverFusion = PtInRect(&estadoVistaCarpetas.botonFusionCarpetas, pt) != FALSE;
            bool hoverEnlace = PtInRect(&estadoVistaCarpetas.botonCrearEnlaceSimbolico, pt) != FALSE;
            bool hoverCopiar = PtInRect(&estadoVistaCarpetas.botonCopiarContenidoPortapapeles, pt) != FALSE;
            if (hoverAnterior != estadoVistaCarpetas.hoverBotonAnterior ||
                hoverSiguiente != estadoVistaCarpetas.hoverBotonSiguiente ||
                hoverExpansion != estadoVistaCarpetas.hoverBotonAlternarExpansion ||
                hoverFusion != estadoVistaCarpetas.hoverBotonFusion ||
                hoverEnlace != estadoVistaCarpetas.hoverBotonCrearEnlace ||
                hoverCopiar != estadoVistaCarpetas.hoverBotonCopiarContenido) {
                estadoVistaCarpetas.hoverBotonAnterior = hoverAnterior;
                estadoVistaCarpetas.hoverBotonSiguiente = hoverSiguiente;
                estadoVistaCarpetas.hoverBotonAlternarExpansion = hoverExpansion;
                estadoVistaCarpetas.hoverBotonFusion = hoverFusion;
                estadoVistaCarpetas.hoverBotonCrearEnlace = hoverEnlace;
                estadoVistaCarpetas.hoverBotonCopiarContenido = hoverCopiar;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
            TrackMouseEvent(&eventoSeguimiento);
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            bool hover = PtInRect(&estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias, pt) != FALSE;
            if (hover != estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias) {
                estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias = hover;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
            if (!estadoComprimidos.esModoComic && estadoComprimidos.archivosSeleccionados.size() > 1) {
                TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
                TrackMouseEvent(&eventoSeguimiento);
                POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
                bool ant = PtInRect(&estadoComprimidos.botonAnteriorComprimido, pt) != FALSE;
                bool sig = PtInRect(&estadoComprimidos.botonSiguienteComprimido, pt) != FALSE;
                if (ant != estadoComprimidos.hoverBotonAnterior || sig != estadoComprimidos.hoverBotonSiguiente) {
                    estadoComprimidos.hoverBotonAnterior = ant;
                    estadoComprimidos.hoverBotonSiguiente = sig;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
                return 0;
            }
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT),
                                              TME_LEAVE, ventana, 0};
            TrackMouseEvent(&eventoSeguimiento);

            if (estadoTexto.esFormatoMarkdown) {
                POINT puntoRaton{GET_X_LPARAM(datosMensaje),
                                 GET_Y_LPARAM(datosMensaje)};
                int barraHoverDetectada = -1;
                for (size_t posicionBarra = 0;
                     posicionBarra < estadoTexto.barrasIndice.size();
                     ++posicionBarra) {
                    if (PtInRect(
                            &estadoTexto.barrasIndice[posicionBarra].areaVisualBarra,
                            puntoRaton)) {
                        barraHoverDetectada = static_cast<int>(posicionBarra);
                        break;
                    }
                }
                if (barraHoverDetectada != estadoTexto.posicionBarraHover) {
                    estadoTexto.posicionBarraHover = barraHoverDetectada;
                    InvalidateRect(ventana, nullptr, FALSE);
                }
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen && estadoImagen.modoGaleria) {
            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ventana, 0};
            TrackMouseEvent(&eventoSeguimiento);
            POINT puntoRaton{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            bool cambioHover = false;
            for (size_t posicionMiniatura = 0; posicionMiniatura < estadoImagen.miniaturasGaleria.size(); ++posicionMiniatura) {
                bool estaSobreTarjeta = PtInRect(&estadoImagen.miniaturasGaleria[posicionMiniatura].areaTarjeta, puntoRaton) != FALSE;
                if (estaSobreTarjeta != estadoImagen.miniaturasGaleria[posicionMiniatura].hover) {
                    estadoImagen.miniaturasGaleria[posicionMiniatura].hover = estaSobreTarjeta;
                    cambioHover = true;
                }
            }
            if (cambioHover) InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        }
    }
    if (mensaje == WM_MOUSELEAVE || mensaje == WM_KILLFOCUS) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            if (estadoVistaCarpetas.hoverBotonAnterior ||
                estadoVistaCarpetas.hoverBotonSiguiente ||
                estadoVistaCarpetas.hoverBotonAlternarExpansion ||
                estadoVistaCarpetas.hoverBotonFusion ||
                estadoVistaCarpetas.hoverBotonCrearEnlace ||
                estadoVistaCarpetas.hoverBotonCopiarContenido) {
                estadoVistaCarpetas.hoverBotonAnterior = false;
                estadoVistaCarpetas.hoverBotonSiguiente = false;
                estadoVistaCarpetas.hoverBotonAlternarExpansion = false;
                estadoVistaCarpetas.hoverBotonFusion = false;
                estadoVistaCarpetas.hoverBotonCrearEnlace = false;
                estadoVistaCarpetas.hoverBotonCopiarContenido = false;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
            if (estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias) {
                estadoVistaCarpetas.resumenVistaVacia.hoverBotonEliminarCarpetasVacias = false;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
            if (estadoComprimidos.hoverBotonAnterior || estadoComprimidos.hoverBotonSiguiente) {
                estadoComprimidos.hoverBotonAnterior = false;
                estadoComprimidos.hoverBotonSiguiente = false;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            if (estadoTexto.posicionBarraHover != -1) {
                estadoTexto.posicionBarraHover = -1;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen && estadoImagen.modoGaleria) {
            bool cambioHover = false;
            for (size_t posicionMiniatura = 0; posicionMiniatura < estadoImagen.miniaturasGaleria.size(); ++posicionMiniatura) {
                if (estadoImagen.miniaturasGaleria[posicionMiniatura].hover) {
                    estadoImagen.miniaturasGaleria[posicionMiniatura].hover = false;
                    cambioHover = true;
                }
            }
            if (cambioHover) InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        }
    }
    if (mensaje == WM_SETCURSOR) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(ventana, &pt);
            if (PtInRect(&estadoVistaCarpetas.botonAnteriorCarpeta, pt) ||
                PtInRect(&estadoVistaCarpetas.botonSiguienteCarpeta, pt) ||
                PtInRect(&estadoVistaCarpetas.botonAlternarExpansion, pt) ||
                PtInRect(&estadoVistaCarpetas.botonSelectorPrograma, pt) ||
                PtInRect(&estadoVistaCarpetas.botonEjecutarAbrir, pt) ||
                PtInRect(&estadoVistaCarpetas.botonCrearEnlaceSimbolico, pt) ||
                PtInRect(&estadoVistaCarpetas.botonCopiarContenidoPortapapeles, pt) ||
                PtInRect(&estadoVistaCarpetas.botonFusionCarpetas, pt)) {
                SetCursor(LoadCursorW(nullptr, IDC_HAND));
                return TRUE;
            }
            if (configuracion.interaccionArchivosVistaCarpetas.load() &&
                obtenerElementoArbolEnPunto(pt) != nullptr) {
                SetCursor(LoadCursorW(nullptr, IDC_HAND));
                return TRUE;
            }
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(ventana, &pt);
            if (PtInRect(&estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias, pt)) {
                SetCursor(LoadCursorW(nullptr, IDC_HAND));
                return TRUE;
            }
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Comprimido) {
            if (!estadoComprimidos.esModoComic) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(ventana, &pt);
                if (PtInRect(&estadoComprimidos.botonAnteriorComprimido, pt) ||
                    PtInRect(&estadoComprimidos.botonSiguienteComprimido, pt) ||
                    PtInRect(&estadoComprimidos.areaBotonDesbloquear, pt) ||
                    obtenerElementoArbolComprimidoEnPunto(pt) != nullptr) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            POINT puntoRaton;
            GetCursorPos(&puntoRaton);
            ScreenToClient(ventana, &puntoRaton);

            for (size_t posicionClicable = 0;
                 posicionClicable < estadoTexto.elementosClicables.size();
                 ++posicionClicable) {
                if (PtInRect(
                        &estadoTexto.elementosClicables[posicionClicable]
                             .areaClic,
                        puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }

            if (estadoTexto.esFormatoMarkdown) {
                for (size_t posicionBarra = 0;
                     posicionBarra < estadoTexto.barrasIndice.size();
                     ++posicionBarra) {
                    if (PtInRect(&estadoTexto.barrasIndice[posicionBarra]
                                      .areaVisualBarra,
                                 puntoRaton)) {
                        SetCursor(LoadCursorW(nullptr, IDC_HAND));
                        return TRUE;
                    }
                }
            }
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen && estadoImagen.modoGaleria) {
            POINT puntoRaton;
            GetCursorPos(&puntoRaton);
            ScreenToClient(ventana, &puntoRaton);
            for (size_t posicionMiniatura = 0; posicionMiniatura < estadoImagen.miniaturasGaleria.size(); ++posicionMiniatura) {
                if (PtInRect(&estadoImagen.miniaturasGaleria[posicionMiniatura].areaTarjeta, puntoRaton)) {
                    SetCursor(LoadCursorW(nullptr, IDC_HAND));
                    return TRUE;
                }
            }
        }
    }
    if (mensaje == WM_LBUTTONDOWN) {
        if (arrastrandoBarraProgreso) {
            arrastrandoBarraProgreso = false;
            if (ventanaControles && GetCapture() == ventanaControles) {
                ReleaseCapture();
            }
        }
        if (menuVelocidadDesplegado) {
            ocultarMenuVelocidadInmediato();
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            if (PtInRect(&estadoVistaCarpetas.botonAnteriorCarpeta, pt)) {
                if (mostrarCarpetaAnterior()) invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonSiguienteCarpeta, pt)) {
                if (mostrarCarpetaSiguiente()) invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonAlternarExpansion, pt)) {
                avanzarNivelExpansionArbol();
                estadoVistaCarpetas.desplazamientoScrollY = 0;
                invalidarInterfaz();
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonSelectorPrograma, pt)) {
                mostrarMenuDesplegableProgramasCarpetas(ventana, estadoVistaCarpetas.botonSelectorPrograma);
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonEjecutarAbrir, pt)) {
                ejecutarAbrirCarpetasConPrograma(ventana);
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonCrearEnlaceSimbolico, pt)) {
                if (ejecutarCrearEnlaceSimbolico(ventana)) {
                    invalidarInterfaz();
                }
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonCopiarContenidoPortapapeles, pt)) {
                if (ejecutarCopiarContenidoPortapapeles(ventana)) {
                    invalidarInterfaz();
                }
                return 0;
            }
            if (PtInRect(&estadoVistaCarpetas.botonFusionCarpetas, pt)) {
                if (ejecutarFusionCarpetas(ventana)) {
                    invalidarInterfaz();
                }
                return 0;
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            if (PtInRect(&estadoVistaCarpetas.resumenVistaVacia.botonEliminarCarpetasVacias, pt)) {
                if (eliminarCarpetasVacias()) {
                    invalidarInterfaz();
                }
                return 0;
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
            POINT puntoRaton{GET_X_LPARAM(datosMensaje),
                             GET_Y_LPARAM(datosMensaje)};

            for (size_t posicionClicable = 0;
                 posicionClicable < estadoTexto.elementosClicables.size();
                 ++posicionClicable) {
                if (PtInRect(
                        &estadoTexto.elementosClicables[posicionClicable]
                             .areaClic,
                        puntoRaton)) {
                    ShellExecuteW(
                        nullptr, L"open",
                        estadoTexto.elementosClicables[posicionClicable]
                            .destinoUrl.c_str(),
                        nullptr, nullptr, SW_SHOWNORMAL);
                    return 0;
                }
            }

            if (estadoTexto.esFormatoMarkdown) {
                for (size_t posicionBarra = 0;
                     posicionBarra < estadoTexto.barrasIndice.size();
                     ++posicionBarra) {
                    if (PtInRect(&estadoTexto.barrasIndice[posicionBarra]
                                      .areaVisualBarra,
                                 puntoRaton)) {
                        estadoTexto.desplazamientoScrollY =
                            estadoTexto.barrasIndice[posicionBarra]
                                .posicionDestinoY;
                        InvalidateRect(ventana, nullptr, FALSE);
                        return 0;
                    }
                }
            }
            return 0;
        }
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
            POINT puntoRaton{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            if (estadoImagen.modoGaleria) {
                for (size_t posicionMiniatura = 0; posicionMiniatura < estadoImagen.miniaturasGaleria.size(); ++posicionMiniatura) {
                    if (PtInRect(&estadoImagen.miniaturasGaleria[posicionMiniatura].areaTarjeta, puntoRaton)) {
                        cargarImagenPorRuta(estadoImagen.miniaturasGaleria[posicionMiniatura].rutaArchivo);
                        estadoImagen.indiceImagenSeleccionada = posicionMiniatura;
                        estadoImagen.modoGaleria = false;
                        invalidarInterfaz();
                        return 0;
                    }
                }
                return 0;
            }
            RECT rectanguloCliente;
            GetClientRect(ventana, &rectanguloCliente);
            if (puntoRaton.x < (rectanguloCliente.right - rectanguloCliente.left) / 2) {
                if (mostrarImagenAnterior()) invalidarInterfaz();
            } else {
                if (mostrarImagenSiguiente()) invalidarInterfaz();
            }
            return 0;
        }
        if (tipoContenidoActivo.load() ==
            TipoContenidoArchivo::Comprimido) {
            POINT pt{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
            if (!estadoComprimidos.esModoComic) {
                if (estadoComprimidos.archivosSeleccionados.size() > 1) {
                    if (PtInRect(&estadoComprimidos.botonAnteriorComprimido, pt)) {
                        if (mostrarComprimidoAnterior()) invalidarInterfaz();
                        return 0;
                    }
                    if (PtInRect(&estadoComprimidos.botonSiguienteComprimido, pt)) {
                        if (mostrarComprimidoSiguiente()) invalidarInterfaz();
                        return 0;
                    }
                }
                if (estadoComprimidos.protegidoConContrasena && !estadoComprimidos.contrasenaDesbloqueada) {
                    if (PtInRect(&estadoComprimidos.areaBotonDesbloquear, pt)) {
                        estadoComprimidos.contrasenaDesbloqueada = true;
                        invalidarInterfaz();
                        return 0;
                    }
                }
            }
            return 0;
        }
        alternarReproduccion();
        return 0;
    }
    if (mensaje == WM_LBUTTONDBLCLK) {
        if (menuVelocidadDesplegado) {
            ocultarMenuVelocidadInmediato();
        }
        return procesarVentanaVideo(ventana, WM_LBUTTONDOWN, parametroMensaje, datosMensaje);
    }
    if (mensaje == WM_RBUTTONUP) {
        if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
            if (configuracion.interaccionArchivosVistaCarpetas.load()) {
                POINT puntoRaton{GET_X_LPARAM(datosMensaje), GET_Y_LPARAM(datosMensaje)};
                const ElementoArbolVistaCarpetas* elementoArbol = obtenerElementoArbolEnPunto(puntoRaton);
                if (elementoArbol) {
                    POINT puntoPantalla = puntoRaton;
                    ClientToScreen(ventana, &puntoPantalla);
                    mostrarMenuContextualElementoArbol(ventana, *elementoArbol, puntoPantalla);
                    return 0;
                }
            }
            return 0;
        }
        return 0;
    }
    if (mensaje == WM_PAINT) {
        PAINTSTRUCT pintura;
        HDC contextoPantalla = BeginPaint(ventana, &pintura);
        RECT areaCliente;
        GetClientRect(ventana, &areaCliente);
        int anchoCliente = areaCliente.right - areaCliente.left;
        int altoCliente = areaCliente.bottom - areaCliente.top;

        if (anchoCliente > 0 && altoCliente > 0) {
            HDC contextoMemoria = CreateCompatibleDC(contextoPantalla);
            HBITMAP mapaBitsMemoria = CreateCompatibleBitmap(
                contextoPantalla, anchoCliente, altoCliente);
            HBITMAP mapaBitsAnterior = static_cast<HBITMAP>(
                SelectObject(contextoMemoria, mapaBitsMemoria));

            if (tipoContenidoActivo.load() == TipoContenidoArchivo::Texto) {
                dibujarLienzoTexto(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Documento ||
                       tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Codigo) {
                dibujarLienzoPendienteImplementacion(
                    contextoMemoria, areaCliente, tipoContenidoActivo.load());
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Imagen) {
                dibujarLienzoImagen(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Comprimido) {
                dibujarLienzoComprimidos(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::VistaCarpetas) {
                dibujarLienzoVistaCarpetas(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::VistaVacia) {
                dibujarLienzoVistaVacia(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Desconocido) {
                dibujarLienzoArchivoDesconocido(contextoMemoria, areaCliente);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            } else if (tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Video ||
                       tipoContenidoActivo.load() ==
                       TipoContenidoArchivo::Ninguno) {
                HBRUSH brochaFondo = CreateSolidBrush(COLOR_FONDO_REPRODUCTOR);
                FillRect(contextoMemoria, &areaCliente, brochaFondo);
                DeleteObject(brochaFondo);
                BitBlt(contextoPantalla, 0, 0, anchoCliente, altoCliente,
                       contextoMemoria, 0, 0, SRCCOPY);
            }

            SelectObject(contextoMemoria, mapaBitsAnterior);
            DeleteObject(mapaBitsMemoria);
            DeleteDC(contextoMemoria);
        }
        EndPaint(ventana, &pintura);
        return 0;
    }
    if (mensaje == WM_TIMER) {
        if (parametroMensaje == ID_TEMPORIZADOR_ANIMACION_GIF) {
            avanzarCuadroAnimacionGif(ventana);
            return 0;
        }
    }
    if (mensaje == WM_DESTROY) {
        KillTimer(ventana, ID_TEMPORIZADOR_ANIMACION_GIF);
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

LRESULT CALLBACK procesarVentanaMenuVelocidad(HWND ventana, UINT mensaje,
                                              WPARAM parametroMensaje,
                                              LPARAM datosMensaje) {
    switch (mensaje) {
        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT eventoSeguimiento{sizeof(TRACKMOUSEEVENT),
                                              TME_LEAVE, ventana, 0};
            TrackMouseEvent(&eventoSeguimiento);

            int posicionVertical = GET_Y_LPARAM(datosMensaje);
            int nuevaOpcion = (posicionVertical - 4) / ALTO_OPCION_VELOCIDAD;
            if (nuevaOpcion < 0 || nuevaOpcion >= TOTAL_OPCIONES_VELOCIDAD) {
                nuevaOpcion = -1;
            }
            if (nuevaOpcion != opcionVelocidadResaltada) {
                opcionVelocidadResaltada = nuevaOpcion;
                InvalidateRect(ventana, nullptr, FALSE);
            }
            return 0;
        }
        case WM_MOUSELEAVE: {
            opcionVelocidadResaltada = -1;
            InvalidateRect(ventana, nullptr, FALSE);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int posicionVertical = GET_Y_LPARAM(datosMensaje);
            int opcionElegida = (posicionVertical - 4) / ALTO_OPCION_VELOCIDAD;
            if (opcionElegida >= 0 && opcionElegida < TOTAL_OPCIONES_VELOCIDAD) {
                cambiarVelocidadReproduccion(VALORES_VELOCIDAD[opcionElegida]);
                opcionVelocidadResaltada = opcionElegida;
                // Contraer el panel de opciones de velocidad inmediatamente
                ocultarMenuVelocidadInmediato();
            }
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            dibujarMenuVelocidadFlotante(ventana);
            return 0;
    }
    return DefWindowProcW(ventana, mensaje, parametroMensaje, datosMensaje);
}

bool registrarClasesDeVentana() {
    WNDCLASSEXW descripcionClase{sizeof(descripcionClase)};
    descripcionClase.hInstance = GetModuleHandleW(nullptr);
    descripcionClase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    descripcionClase.style = CS_DBLCLKS;

    descripcionClase.lpfnWndProc = procesarVentanaReproductor;
    descripcionClase.lpszClassName = CLASE_VENTANA_REPRODUCTOR;
    ATOM atomoClaseReproductor = RegisterClassExW(&descripcionClase);
    if (!atomoClaseReproductor &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    descripcionClase.style = CS_DBLCLKS;
    descripcionClase.lpfnWndProc = procesarVentanaVideo;
    descripcionClase.hbrBackground =
        static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    descripcionClase.lpszClassName = CLASE_VENTANA_VIDEO;
    ATOM atomoClaseVideo = RegisterClassExW(&descripcionClase);
    if (!atomoClaseVideo && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        if (atomoClaseReproductor) {
            UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                             descripcionClase.hInstance);
        }
        return false;
    }

    descripcionClase.style = CS_DBLCLKS;
    descripcionClase.lpfnWndProc = procesarVentanaControles;
    descripcionClase.hbrBackground = nullptr;
    descripcionClase.lpszClassName = CLASE_VENTANA_CONTROLES;
    ATOM atomoClaseControles = RegisterClassExW(&descripcionClase);
    if (!atomoClaseControles &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        UnregisterClassW(CLASE_VENTANA_VIDEO, descripcionClase.hInstance);
        if (atomoClaseReproductor) {
            UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                             descripcionClase.hInstance);
        }
        return false;
    }

    descripcionClase.style = 0;
    descripcionClase.lpfnWndProc = procesarVentanaMenuVelocidad;
    descripcionClase.hbrBackground = nullptr;
    descripcionClase.lpszClassName = CLASE_VENTANA_MENU_VELOCIDAD;
    ATOM atomoClaseMenuVelocidad = RegisterClassExW(&descripcionClase);
    if (!atomoClaseMenuVelocidad &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        UnregisterClassW(CLASE_VENTANA_CONTROLES, descripcionClase.hInstance);
        UnregisterClassW(CLASE_VENTANA_VIDEO, descripcionClase.hInstance);
        if (atomoClaseReproductor) {
            UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                             descripcionClase.hInstance);
        }
        return false;
    }

    descripcionClase.lpfnWndProc = procesarVentanaDelegadoReproductor;
    descripcionClase.lpszClassName = CLASE_VENTANA_DELEGADO;
    if (RegisterClassExW(&descripcionClase) ||
        GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
        return true;
    }

    UnregisterClassW(CLASE_VENTANA_MENU_VELOCIDAD, descripcionClase.hInstance);
    UnregisterClassW(CLASE_VENTANA_CONTROLES, descripcionClase.hInstance);
    UnregisterClassW(CLASE_VENTANA_VIDEO, descripcionClase.hInstance);
    if (atomoClaseReproductor) {
        UnregisterClassW(CLASE_VENTANA_REPRODUCTOR,
                         descripcionClase.hInstance);
    }
    return false;
}

void eliminarClasesDeVentana() {
    HINSTANCE instancia = GetModuleHandleW(nullptr);
    UnregisterClassW(CLASE_VENTANA_REPRODUCTOR, instancia);
    UnregisterClassW(CLASE_VENTANA_VIDEO, instancia);
    UnregisterClassW(CLASE_VENTANA_CONTROLES, instancia);
    UnregisterClassW(CLASE_VENTANA_MENU_VELOCIDAD, instancia);
    UnregisterClassW(CLASE_VENTANA_DELEGADO, instancia);
}
/**** ended inlining interfaz/ventanas.cpp ****/
/**** start inlining integracion/panel_previsualizacion.cpp ****/
struct DatosBusquedaVistaArchivos {
    HWND mejorVistaArchivos{nullptr};
    int mayorAreaPixeles{0};
};

BOOL CALLBACK encontrarVistaDeArchivos(HWND ventanaCandidata,
                                       LPARAM direccionResultado) {
    auto* datosBusqueda = reinterpret_cast<DatosBusquedaVistaArchivos*>(direccionResultado);
    wchar_t nombreClase[64];
    if (GetClassNameW(ventanaCandidata, nombreClase, ARRAYSIZE(nombreClase)) &&
        lstrcmpW(nombreClase, L"SHELLDLL_DefView") == 0 &&
        IsWindowVisible(ventanaCandidata)) {
        RECT rectanguloCandidata;
        GetWindowRect(ventanaCandidata, &rectanguloCandidata);
        int anchoCandidata = rectanguloCandidata.right - rectanguloCandidata.left;
        int altoCandidata = rectanguloCandidata.bottom - rectanguloCandidata.top;
        int areaCandidata = anchoCandidata * altoCandidata;
        if (anchoCandidata > 50 && altoCandidata > 50 && areaCandidata > datosBusqueda->mayorAreaPixeles) {
            datosBusqueda->mayorAreaPixeles = areaCandidata;
            datosBusqueda->mejorVistaArchivos = ventanaCandidata;
        }
    }
    return TRUE;
}

bool obtenerAreaPanel(RECT* areaPanel);
bool ajustarReproductorAlPanel(const RECT* areaPanelCalculada = nullptr);

inline void bloquearYSuprimirPanelNativoWindows(HWND ventanaExplorador) {
    if (!ventanaExplorador || !IsWindow(ventanaExplorador)) return;

    EnumChildWindows(
        ventanaExplorador,
        [](HWND ventanaHija, LPARAM) -> BOOL {
            if (ventanaHija == ventanaReproductor || ventanaHija == ventanaVideo ||
                ventanaHija == ventanaControles || ventanaHija == ventanaMenuVelocidad) {
                return TRUE;
            }
            wchar_t nombreClase[128]{};
            if (GetClassNameW(ventanaHija, nombreClase, ARRAYSIZE(nombreClase))) {
                if (wcsstr(nombreClase, L"ShellPreview") ||
                    wcsstr(nombreClase, L"PreviewHandler") ||
                    wcsstr(nombreClase, L"ATL:") ||
                    wcsstr(nombreClase, L"UniversalSearchBand") ||
                    wcsstr(nombreClase, L"ActiveX") ||
                    wcsstr(nombreClase, L"RichEdit20W_Preview")) {
                    if (IsWindowVisible(ventanaHija)) {
                        ShowWindow(ventanaHija, SW_HIDE);
                        SetWindowPos(ventanaHija, nullptr, 0, 0, 0, 0,
                                     SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE |
                                     SWP_NOZORDER | SWP_NOACTIVATE);
                    }
                }
            }
            return TRUE;
        },
        0);
}

LRESULT CALLBACK procesarSubclaseVentanaExplorador(HWND hwnd, UINT uMsg,
                                                   WPARAM wParam, LPARAM lParam,
                                                   UINT_PTR uIdSubclass,
                                                   DWORD_PTR dwRefData) {
    if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) {
        if (wParam == 'P' || wParam == 'p') {
            bool ctrlPresionado = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            bool altPresionado = (GetKeyState(VK_MENU) & 0x8000) != 0;
            if (ctrlPresionado || altPresionado) {
                SendMessageW(hwnd, WM_COMMAND, 41489, 0);
                return 0;
            }
        }
    }
    LRESULT resultado = DefSubclassProc(hwnd, uMsg, wParam, lParam);
    if (uMsg == WM_COMMAND) {
        WORD idComando = LOWORD(wParam);
        if (idComando == 41489 || idComando == 0xA211 ||
            idComando == 41490 || idComando == 0xA212) {
            RECT areaPanelCalculada{};
            if (obtenerAreaPanel(&areaPanelCalculada)) {
                ajustarReproductorAlPanel(&areaPanelCalculada);
                invalidarInterfaz();
            } else if (ventanaReproductor && IsWindow(ventanaReproductor)) {
                ShowWindow(ventanaReproductor, SW_HIDE);
            }
        }
    } else if (uMsg == WM_WINDOWPOSCHANGED || uMsg == WM_SIZE) {
        if (ventanaReproductor && IsWindow(ventanaReproductor) && IsWindowVisible(ventanaReproductor)) {
            RECT areaPanelCalculada{};
            if (obtenerAreaPanel(&areaPanelCalculada)) {
                ajustarReproductorAlPanel(&areaPanelCalculada);
            }
        }
    } else if (uMsg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, procesarSubclaseVentanaExplorador, uIdSubclass);
    }
    return resultado;
}

inline void asegurarSubclaseVentanaExplorador(HWND ventanaExplorador) {
    if (ventanaExplorador && IsWindow(ventanaExplorador)) {
        SetWindowSubclass(ventanaExplorador, procesarSubclaseVentanaExplorador,
                          ID_SUBCLASE_VENTANA_EXPLORADOR, 0);
    }
}

LRESULT CALLBACK procesarSubclaseVistaArchivos(HWND hwnd, UINT uMsg,
                                              WPARAM wParam, LPARAM lParam,
                                              UINT_PTR uIdSubclass,
                                              DWORD_PTR dwRefData) {
    if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) {
        if (wParam == VK_DELETE || wParam == VK_F2) {
            if (reproductor) {
                reproductor->Stop();
            }
        }
        if (wParam == 'P' || wParam == 'p') {
            bool ctrlPresionado = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            bool altPresionado = (GetKeyState(VK_MENU) & 0x8000) != 0;
            if (ctrlPresionado || altPresionado) {
                HWND ventanaRaiz = normalizarVentanaExploradorAnfitriona(hwnd);
                if (ventanaRaiz) {
                    SendMessageW(ventanaRaiz, WM_COMMAND, 41489, 0);
                    return 0;
                }
            }
        }
    }
    LRESULT resultado = DefSubclassProc(hwnd, uMsg, wParam, lParam);
    if (uMsg == WM_WINDOWPOSCHANGED || uMsg == WM_SIZE || uMsg == WM_SHOWWINDOW) {
        if (ventanaReproductor && IsWindow(ventanaReproductor)) {
            RECT areaPanelCalculada{};
            if (obtenerAreaPanel(&areaPanelCalculada)) {
                ajustarReproductorAlPanel(&areaPanelCalculada);
            }
        }
    } else if (uMsg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, procesarSubclaseVistaArchivos, uIdSubclass);
    }
    return resultado;
}

inline void asegurarSubclaseVistaArchivos(HWND vistaArchivos) {
    if (vistaArchivos && IsWindow(vistaArchivos)) {
        SetWindowSubclass(vistaArchivos, procesarSubclaseVistaArchivos,
                          ID_SUBCLASE_VISTA_ARCHIVOS, 0);
    }
}

bool obtenerAreaPanel(RECT* areaPanel) {
    if (!areaPanel) return false;

    DWORD procesoActual = GetCurrentProcessId();
    HWND ventanaFoco = GetForegroundWindow();
    if (ventanaFoco) {
        DWORD identificadorProcesoFoco = 0;
        GetWindowThreadProcessId(ventanaFoco, &identificadorProcesoFoco);
        if (identificadorProcesoFoco == procesoActual) {
            HWND ventanaNormalizada = normalizarVentanaExploradorAnfitriona(ventanaFoco);
            if (ventanaNormalizada && IsWindowVisible(ventanaNormalizada)) {
                ventanaExploradorAnfitriona = ventanaNormalizada;
            }
        }
    }

    if (!ventanaExploradorAnfitriona || !IsWindow(ventanaExploradorAnfitriona) ||
        !IsWindowVisible(ventanaExploradorAnfitriona)) {
        HWND ventanaCandidataExplorador = FindWindowExW(nullptr, nullptr, L"CabinetWClass", nullptr);
        while (ventanaCandidataExplorador) {
            DWORD identificadorProcesoCandidata = 0;
            GetWindowThreadProcessId(ventanaCandidataExplorador, &identificadorProcesoCandidata);
            if (identificadorProcesoCandidata == procesoActual && IsWindowVisible(ventanaCandidataExplorador)) {
                ventanaExploradorAnfitriona = normalizarVentanaExploradorAnfitriona(ventanaCandidataExplorador);
                break;
            }
            ventanaCandidataExplorador = FindWindowExW(nullptr, ventanaCandidataExplorador, L"CabinetWClass", nullptr);
        }
    }

    if (!ventanaExploradorAnfitriona || !IsWindow(ventanaExploradorAnfitriona) ||
        !IsWindowVisible(ventanaExploradorAnfitriona)) {
        return false;
    }

    asegurarSubclaseVentanaExplorador(ventanaExploradorAnfitriona);

    HWND vistaArchivos = ventanaVistaArchivosActiva;
    if (!vistaArchivos || !IsWindow(vistaArchivos) || !IsWindowVisible(vistaArchivos) ||
        !IsChild(ventanaExploradorAnfitriona, vistaArchivos)) {
        DatosBusquedaVistaArchivos datosBusqueda;
        EnumChildWindows(ventanaExploradorAnfitriona, encontrarVistaDeArchivos,
                         reinterpret_cast<LPARAM>(&datosBusqueda));
        vistaArchivos = datosBusqueda.mejorVistaArchivos;
        if (vistaArchivos) {
            ventanaVistaArchivosActiva = vistaArchivos;
        }
    }
    if (!vistaArchivos) return false;

    asegurarSubclaseVistaArchivos(vistaArchivos);

    RECT areaContenedorPantalla;
    RECT areaVistaPantalla;
    GetWindowRect(ventanaExploradorAnfitriona, &areaContenedorPantalla);
    GetWindowRect(vistaArchivos, &areaVistaPantalla);

    constexpr int ANCHO_MINIMO_PANEL = 100;
    constexpr int MARGEN_SEPARADOR_EXPLORER = 4;
    int anchoPanelDetectado = areaContenedorPantalla.right - (areaVistaPantalla.right + MARGEN_SEPARADOR_EXPLORER);

    if (anchoPanelDetectado < ANCHO_MINIMO_PANEL) {
        return false;
    }

    RECT rectPanelPantalla = {
        areaVistaPantalla.right + MARGEN_SEPARADOR_EXPLORER,
        areaVistaPantalla.top,
        areaContenedorPantalla.right,
        areaVistaPantalla.bottom
    };

    MapWindowPoints(HWND_DESKTOP, ventanaExploradorAnfitriona,
                    reinterpret_cast<POINT*>(&rectPanelPantalla), 2);

    *areaPanel = rectPanelPantalla;
    return true;
}

inline bool debeBloquearTamanoParaTipo(TipoContenidoArchivo tipoContenido) {
    switch (tipoContenido) {
        case TipoContenidoArchivo::Video:
            return configuracion.bloquearTamanoVideo.load();
        case TipoContenidoArchivo::Imagen:
            return configuracion.bloquearTamanoImagen.load();
        case TipoContenidoArchivo::Comprimido:
            return configuracion.bloquearTamanoComprimidos.load();
        case TipoContenidoArchivo::Texto:
            return configuracion.bloquearTamanoTexto.load();
        case TipoContenidoArchivo::Documento:
            return configuracion.bloquearTamanoDocumentos.load();
        case TipoContenidoArchivo::Codigo:
            return configuracion.bloquearTamanoCodigo.load();
        case TipoContenidoArchivo::VistaCarpetas:
            return configuracion.bloquearTamanoVistaCarpetas.load();
        case TipoContenidoArchivo::VistaVacia:
            return configuracion.bloquearTamanoVistaVacia.load();
        case TipoContenidoArchivo::Desconocido:
            return configuracion.bloquearTamanoDesconocido.load();
        default:
            return false;
    }
}

bool ajustarReproductorAlPanel(const RECT* areaPanelCalculada) {
    if (!ventanaReproductor) return false;
    RECT areaPanel;
    if (areaPanelCalculada) {
        areaPanel = *areaPanelCalculada;
    } else if (!obtenerAreaPanel(&areaPanel)) {
        ShowWindow(ventanaReproductor, SW_HIDE);
        return false;
    }

    if (ventanaExploradorAnfitriona && IsWindow(ventanaExploradorAnfitriona)) {
        bloquearYSuprimirPanelNativoWindows(ventanaExploradorAnfitriona);
    }

    if (GetParent(ventanaReproductor) != ventanaExploradorAnfitriona) {
        SetParent(ventanaReproductor, ventanaExploradorAnfitriona);
    }

    RECT areaActual;
    GetWindowRect(ventanaReproductor, &areaActual);
    MapWindowPoints(HWND_DESKTOP, ventanaExploradorAnfitriona,
                    reinterpret_cast<POINT*>(&areaActual), 2);

    int nuevoAncho = areaPanel.right - areaPanel.left;
    int nuevoAlto = areaPanel.bottom - areaPanel.top;
    int anchoActual = areaActual.right - areaActual.left;
    int altoActual = areaActual.bottom - areaActual.top;

    bool dimensionesCambiaron = (areaActual.left != areaPanel.left ||
                                 areaActual.top != areaPanel.top ||
                                 anchoActual != nuevoAncho ||
                                 altoActual != nuevoAlto);

    if (!dimensionesCambiaron && IsWindowVisible(ventanaReproductor)) {
        return true;
    }

    bool bloqueoActivo = debeBloquearTamanoParaTipo(tipoContenidoActivo.load());
    if (bloqueoActivo && anchoActual > 0 && altoActual > 0) {
        SetWindowPos(ventanaReproductor, HWND_TOP, areaPanel.left, areaPanel.top,
                     anchoActual, altoActual,
                     SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS);
        ajustarLayoutVentanasReproductor(ventanaReproductor);
        return true;
    }

    SetWindowPos(ventanaReproductor, HWND_TOP, areaPanel.left, areaPanel.top,
                 nuevoAncho, nuevoAlto,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS);

    ajustarLayoutVentanasReproductor(ventanaReproductor);
    return true;
}

void liberarRecursosDelReproductor() {
    if (reproductor) {
        reproductor->Shutdown();
        reproductor->Release();
        reproductor = nullptr;
    }
    if (receptorEventos) {
        receptorEventos->Release();
        receptorEventos = nullptr;
    }
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    tipoContenidoActivo.store(TipoContenidoArchivo::Ninguno);

    if (ventanaMenuVelocidad) {
        DestroyWindow(ventanaMenuVelocidad);
        ventanaMenuVelocidad = nullptr;
    }
    menuVelocidadDesplegado = false;
    animandoCierreMenuVelocidad = false;
    opacidadMenuVelocidad = 255;
    opcionVelocidadResaltada = -1;

    if (ventanaVistaArchivosActiva && IsWindow(ventanaVistaArchivosActiva)) {
        RemoveWindowSubclass(ventanaVistaArchivosActiva, procesarSubclaseVistaArchivos,
                             ID_SUBCLASE_VISTA_ARCHIVOS);
    }
    ventanaVistaArchivosActiva = nullptr;

    if (ventanaExploradorAnfitriona && IsWindow(ventanaExploradorAnfitriona)) {
        RemoveWindowSubclass(ventanaExploradorAnfitriona, procesarSubclaseVentanaExplorador,
                             ID_SUBCLASE_VENTANA_EXPLORADOR);
    }

    if (ventanaReproductor) DestroyWindow(ventanaReproductor);

    ventanaReproductor = nullptr;
    ventanaVideo = nullptr;
    ventanaControles = nullptr;
    arrastrandoBarraProgreso = false;
    proporcionProgreso = 0.0;
    reproducirTrasCambioPosicion.store(false);
    reiniciarCacheTiempoReproduccion();
}

bool asegurarVentanasContenedoras() {
    RECT areaPanel;
    if (!obtenerAreaPanel(&areaPanel)) return false;

    if (ventanaExploradorAnfitriona && IsWindow(ventanaExploradorAnfitriona)) {
        bloquearYSuprimirPanelNativoWindows(ventanaExploradorAnfitriona);
    }

    if (ventanaReproductor && IsWindow(ventanaReproductor)) {
        if (GetParent(ventanaReproductor) != ventanaExploradorAnfitriona) {
            SetParent(ventanaReproductor, ventanaExploradorAnfitriona);
        }
        return true;
    }

    if (!ventanaReproductor) {
        ventanaReproductor = CreateWindowExW(
            0, CLASE_VENTANA_REPRODUCTOR, L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS |
                WS_CLIPCHILDREN,
            areaPanel.left, areaPanel.top,
            areaPanel.right - areaPanel.left,
            areaPanel.bottom - areaPanel.top,
            ventanaExploradorAnfitriona, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (!ventanaReproductor) return false;

        ventanaVideo = CreateWindowExW(
            0, CLASE_VENTANA_VIDEO, L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 0, 0, ventanaReproductor, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (!ventanaVideo) {
            liberarRecursosDelReproductor();
            return false;
        }

        ventanaControles = CreateWindowExW(
            0, CLASE_VENTANA_CONTROLES, L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 0, 0, ventanaReproductor,
            nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!ventanaControles) {
            liberarRecursosDelReproductor();
            return false;
        }

        ventanaMenuVelocidad = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            CLASE_VENTANA_MENU_VELOCIDAD, L"",
            WS_POPUP | WS_CLIPSIBLINGS,
            0, 0, ANCHO_PANEL_VELOCIDAD, ALTO_PANEL_VELOCIDAD,
            ventanaReproductor, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (ventanaMenuVelocidad) {
            SetLayeredWindowAttributes(ventanaMenuVelocidad, 0, 255, LWA_ALPHA);
        }

        if (!SetTimer(ventanaControles, ID_TEMPORIZADOR_INTERFAZ,
                      INTERVALO_REFRESCO_MS, nullptr)) {
            liberarRecursosDelReproductor();
            return false;
        }
    }
    return true;
}

inline void detenerYLiberarReproductorMfSiExiste() {
    bool reproductorEstabaActivo = (reproductor != nullptr);
    if (reproductor) {
        reproductor->Shutdown();
        reproductor->Release();
        reproductor = nullptr;
    }
    if (receptorEventos) {
        receptorEventos->Release();
        receptorEventos = nullptr;
    }
    reiniciarCacheTiempoReproduccion();
    if (reproductorEstabaActivo && ventanaReproductor && IsWindow(ventanaReproductor)) {
        if (ventanaVideo && IsWindow(ventanaVideo)) {
            DestroyWindow(ventanaVideo);
            ventanaVideo = nullptr;
        }
        RECT areaCliente;
        GetClientRect(ventanaReproductor, &areaCliente);
        ventanaVideo = CreateWindowExW(
            0, CLASE_VENTANA_VIDEO, L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, areaCliente.right, areaCliente.bottom,
            ventanaReproductor, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (ventanaVideo) {
            InvalidateRect(ventanaVideo, nullptr, FALSE);
        }
    }
}

inline void liberarRecursosDeTodasCategorias() {
    detenerYLiberarReproductorMfSiExiste();
    liberarRecursosVideo();
    liberarRecursosImagen();
    liberarRecursosTexto();
    liberarRecursosComprimido();
    liberarRecursosVistaCarpetas();
    liberarRecursosDocumentos();
    liberarRecursosCodigo();
    liberarRecursosDesconocido();
    tipoContenidoActivo.store(TipoContenidoArchivo::Ninguno);
}

bool asegurarReproductor(const std::vector<std::wstring>& listaVideos) {
    if (listaVideos.empty()) return false;
    liberarRecursosDeTodasCategorias();
    tipoContenidoActivo.store(TipoContenidoArchivo::Video);

    if (!asegurarVentanasContenedoras()) return false;

    if (!reproductor) {
        receptorEventos = new (std::nothrow) ReceptorEventosMediaFoundation();
        if (!receptorEventos) {
            liberarRecursosDelReproductor();
            return false;
        }
        if (FAILED(MFPCreateMediaPlayer(nullptr, FALSE, 0, receptorEventos,
                                        ventanaVideo, &reproductor))) {
            liberarRecursosDelReproductor();
            return false;
        }
    }

    if (!reproductor || !ajustarReproductorAlPanel()) return false;
    listaVideosSeleccionados = listaVideos;
    std::sort(listaVideosSeleccionados.begin(), listaVideosSeleccionados.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    indiceVideoSeleccionado = 0;
    actualizarMetadatosVideo(listaVideosSeleccionados.front());
    reproductor->Stop();
    reproductor->SetMute(!configuracion.sonidoHabilitado.load());
    reiniciarCacheTiempoReproduccion();
    HRESULT resultadoCarga = reproductor->CreateMediaItemFromURL(
        listaVideosSeleccionados.front().c_str(), FALSE, 0, nullptr);
    if (ventanaReproductor && IsWindow(ventanaReproductor)) {
        InvalidateRect(ventanaReproductor, nullptr, TRUE);
    }
    invalidarInterfaz();
    return SUCCEEDED(resultadoCarga);
}

bool asegurarVisorImagen(const std::vector<std::wstring>& listaImagenes) {
    if (listaImagenes.empty()) return false;
    liberarRecursosDeTodasCategorias();
    tipoContenidoActivo.store(TipoContenidoArchivo::Imagen);

    if (!asegurarVentanasContenedoras()) return false;
    estadoImagen.imagenesSeleccionadas = listaImagenes;
    std::sort(estadoImagen.imagenesSeleccionadas.begin(), estadoImagen.imagenesSeleccionadas.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    estadoImagen.indiceImagenSeleccionada = 0;
    if (!cargarImagenPorRuta(estadoImagen.imagenesSeleccionadas.front())) return false;
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorTexto(const std::vector<std::wstring>& listaTextos) {
    if (listaTextos.empty()) return false;
    liberarRecursosDeTodasCategorias();
    tipoContenidoActivo.store(TipoContenidoArchivo::Texto);

    if (!asegurarVentanasContenedoras()) return false;
    estadoTexto.textosSeleccionados = listaTextos;
    std::sort(estadoTexto.textosSeleccionados.begin(), estadoTexto.textosSeleccionados.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    estadoTexto.indiceTextoSeleccionado = 0;
    if (!cargarTextoPorRuta(estadoTexto.textosSeleccionados.front())) return false;
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorPendiente(const std::vector<std::wstring>& listaArchivos,
                            TipoContenidoArchivo tipo) {
    if (listaArchivos.empty()) return false;
    liberarRecursosDeTodasCategorias();
    tipoContenidoActivo.store(tipo);

    if (!asegurarVentanasContenedoras()) return false;

    EstadoPendienteImplementacion& estado =
        (tipo == TipoContenidoArchivo::Documento) ? estadoDocumentos : estadoCodigo;
    estado.archivosSeleccionados = listaArchivos;
    std::sort(estado.archivosSeleccionados.begin(), estado.archivosSeleccionados.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    estado.indiceActual = 0;
    estado.rutaActual = estado.archivosSeleccionados.front();
    estado.nombreArchivo = PathFindFileNameW(estado.rutaActual.c_str());
    estado.extension = PathFindExtensionW(estado.rutaActual.c_str());

    if (!ajustarReproductorAlPanel()) return false;
    invalidarInterfaz();
    return true;
}

bool asegurarVisorComprimidos(
    const std::vector<std::wstring>& listaArchivos) {
    liberarRecursosDeTodasCategorias();
    tipoContenidoActivo.store(TipoContenidoArchivo::Comprimido);

    if (!asegurarVentanasContenedoras()) return false;
    std::vector<std::wstring> listaOrdenada = listaArchivos;
    std::sort(listaOrdenada.begin(), listaOrdenada.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    if (!cargarArchivosComprimidos(listaOrdenada)) return false;
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorVistaCarpetas(
    const std::vector<std::wstring>& listaCarpetas) {
    liberarRecursosDeTodasCategorias();
    tipoContenidoActivo.store(TipoContenidoArchivo::VistaCarpetas);

    if (!asegurarVentanasContenedoras()) return false;
    estadoVistaCarpetas.carpetasSeleccionadas = listaCarpetas;
    std::sort(estadoVistaCarpetas.carpetasSeleccionadas.begin(), estadoVistaCarpetas.carpetasSeleccionadas.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    if (!cargarCarpetaPorIndice(0)) return false;
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorVistaVacia(const std::wstring& rutaCarpeta) {
    liberarRecursosDeTodasCategorias();
    tipoContenidoActivo.store(TipoContenidoArchivo::VistaVacia);

    if (!asegurarVentanasContenedoras()) return false;
    calcularResumenVistaVacia(rutaCarpeta);
    if (!ajustarReproductorAlPanel()) return false;

    invalidarInterfaz();
    return true;
}

bool asegurarVisorDesconocido(const std::vector<std::wstring>& listaArchivos) {
    if (listaArchivos.empty()) return false;
    liberarRecursosDeTodasCategorias();
    tipoContenidoActivo.store(TipoContenidoArchivo::Desconocido);

    if (!asegurarVentanasContenedoras()) return false;

    estadoDesconocido.archivosSeleccionados = listaArchivos;
    std::sort(estadoDesconocido.archivosSeleccionados.begin(), estadoDesconocido.archivosSeleccionados.end(),
              [](const std::wstring& rutaPrimera, const std::wstring& rutaSegunda) {
                  return StrCmpLogicalW(rutaPrimera.c_str(), rutaSegunda.c_str()) < 0;
              });
    estadoDesconocido.indiceActual = 0;
    estadoDesconocido.rutaActual = estadoDesconocido.archivosSeleccionados.front();
    estadoDesconocido.nombreArchivo = PathFindFileNameW(estadoDesconocido.rutaActual.c_str());
    estadoDesconocido.extension = PathFindExtensionW(estadoDesconocido.rutaActual.c_str());

    WIN32_FILE_ATTRIBUTE_DATA datosArchivo{};
    if (GetFileAttributesExW(estadoDesconocido.rutaActual.c_str(), GetFileExInfoStandard, &datosArchivo)) {
        ULARGE_INTEGER tamano;
        tamano.LowPart = datosArchivo.nFileSizeLow;
        tamano.HighPart = datosArchivo.nFileSizeHigh;
        wchar_t bufTamano[64]{};
        StrFormatByteSizeW(static_cast<LONGLONG>(tamano.QuadPart), bufTamano, ARRAYSIZE(bufTamano));
        estadoDesconocido.tamanoFormateado = bufTamano;
    } else {
        estadoDesconocido.tamanoFormateado = L"";
    }

    if (!ajustarReproductorAlPanel()) return false;
    invalidarInterfaz();
    return true;
}

bool asegurarVisualizadorParaArchivos(
    const std::vector<std::wstring>& listaArchivos) {
    if (listaArchivos.empty()) return false;

    TipoContenidoArchivo tipoSeleccion = clasificarSeleccionArchivos(listaArchivos);

    if (tipoSeleccion == TipoContenidoArchivo::VistaCarpetas) {
        std::vector<std::wstring> listaCarpetas;
        for (const auto& ruta : listaArchivos) {
            if (esRutaCarpeta(ruta.c_str())) listaCarpetas.push_back(ruta);
        }
        return asegurarVisorVistaCarpetas(!listaCarpetas.empty() ? listaCarpetas : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::VistaVacia) {
        return asegurarVisorVistaVacia(listaArchivos.front());
    }
    if (tipoSeleccion == TipoContenidoArchivo::Comprimido) {
        std::vector<std::wstring> listaComprimidos;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionComprimido(ext)) listaComprimidos.push_back(ruta);
        }
        return asegurarVisorComprimidos(!listaComprimidos.empty() ? listaComprimidos : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Video) {
        std::vector<std::wstring> listaVideos;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionVideo(ext)) listaVideos.push_back(ruta);
        }
        return asegurarReproductor(!listaVideos.empty() ? listaVideos : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Imagen) {
        std::vector<std::wstring> listaImagenes;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionImagen(ext)) listaImagenes.push_back(ruta);
        }
        return asegurarVisorImagen(!listaImagenes.empty() ? listaImagenes : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Texto) {
        std::vector<std::wstring> listaTextos;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionTexto(ext)) listaTextos.push_back(ruta);
        }
        return asegurarVisorTexto(!listaTextos.empty() ? listaTextos : listaArchivos);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Documento) {
        std::vector<std::wstring> listaDocumentos;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionDocumento(ext)) listaDocumentos.push_back(ruta);
        }
        return asegurarVisorPendiente(!listaDocumentos.empty() ? listaDocumentos : listaArchivos,
                                      TipoContenidoArchivo::Documento);
    }
    if (tipoSeleccion == TipoContenidoArchivo::Codigo) {
        std::vector<std::wstring> listaCodigo;
        for (const auto& ruta : listaArchivos) {
            const wchar_t* ext = PathFindExtensionW(ruta.c_str());
            if (esExtensionCodigo(ext)) listaCodigo.push_back(ruta);
        }
        return asegurarVisorPendiente(!listaCodigo.empty() ? listaCodigo : listaArchivos,
                                      TipoContenidoArchivo::Codigo);
    }
    return asegurarVisorDesconocido(listaArchivos);
}

/**** ended inlining integracion/panel_previsualizacion.cpp ****/
/**** start inlining ejecucion/hilo_reproductor.cpp ****/
void procesarMensajesDelHilo() {
    MSG mensajePendiente;
    while (PeekMessageW(&mensajePendiente, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&mensajePendiente);
        DispatchMessageW(&mensajePendiente);
    }
}

bool asegurarDelegadoReproductor() {
    if (ventanaDelegadoReproductor) return true;
    ventanaDelegadoReproductor = CreateWindowExW(
        0, CLASE_VENTANA_DELEGADO, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
        GetModuleHandleW(nullptr), nullptr);
    if (!ventanaDelegadoReproductor) {
        Wh_Log(L"No fue posible crear el delegado del reproductor (error %u)",
               static_cast<unsigned int>(GetLastError()));
        return false;
    }
    return true;
}

DWORD WINAPI ejecutarHiloReproductor(void*) {
    HRESULT resultadoInicializacionCom =
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(resultadoInicializacionCom)) {
        Wh_Log(L"No fue posible inicializar COM (HRESULT 0x%08X)",
               static_cast<unsigned int>(resultadoInicializacionCom));
        return 1;
    }
    bool mfIniciado = SUCCEEDED(MFStartup(MF_VERSION));
    if (!mfIniciado) {
        Wh_Log(L"Media Foundation no está disponible; se omitirá la reproducción de video");
    }

    if (!asegurarDelegadoReproductor()) {
        if (mfIniciado) MFShutdown();
        CoUninitialize();
        return 1;
    }

    PunteroCom<IShellWindows> ventanasShell;

    while (!detenerHilo.load()) {
        if (!ventanasShell.obtener()) {
            CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_ALL,
                             IID_PPV_ARGS(ventanasShell.recibir()));
        }

        // La configuración se aplica una sola vez por cambio.
        if (configuracionPendiente.exchange(false)) {
            if (reproductor) {
                reproductor->SetMute(!configuracion.sonidoHabilitado.load());
            }
            if (ventanaReproductor && IsWindowVisible(ventanaReproductor)) {
                ajustarReproductorAlPanel();
                invalidarInterfaz();
            }
        }

        if (ventanasShell.obtener()) {
            long totalVentanas = 0;
            if (FAILED(ventanasShell->get_Count(&totalVentanas))) {
                // Desconexión RPC o instancia inválida: liberar para reconectar en el siguiente ciclo
                ventanasShell.liberar();
            }
        }

        std::wstring rutaCarpetaContenedora;
        std::vector<std::wstring> rutasSeleccionadas =
            obtenerArchivosSeleccionados(ventanasShell.obtener(),
                                         &rutaCarpetaContenedora);

        RECT areaPanel{};
        bool panelPrevisualizacionAbierto = obtenerAreaPanel(&areaPanel);
        if (!panelPrevisualizacionAbierto) {
            if (!rutaArchivoActivo.empty() ||
                (ventanaReproductor && IsWindowVisible(ventanaReproductor))) {
                liberarRecursosDeTodasCategorias();
                rutaArchivoActivo.clear();
                rutaVideoActiva.clear();
                if (ventanaReproductor) {
                    ShowWindow(ventanaReproductor, SW_HIDE);
                }
            }
        } else {
            if (!rutasSeleccionadas.empty()) {
                std::wstring claveSeleccion;
                for (const auto& rutaElemento : rutasSeleccionadas) {
                    claveSeleccion += rutaElemento + L"|";
                }
                if (claveSeleccion != rutaArchivoActivo) {
                    if (asegurarVisualizadorParaArchivos(rutasSeleccionadas)) {
                        rutaArchivoActivo = claveSeleccion;
                        rutaVideoActiva = rutasSeleccionadas.front();
                    }
                } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaCarpetas) {
                    if (estadoVistaCarpetas.indiceCarpetaActual < estadoVistaCarpetas.carpetasSeleccionadas.size()) {
                        const std::wstring& rutaCarpetaInspeccionada =
                            estadoVistaCarpetas.carpetasSeleccionadas[estadoVistaCarpetas.indiceCarpetaActual];
                        WIN32_FILE_ATTRIBUTE_DATA datosAtributosCarpeta;
                        if (GetFileAttributesExW(rutaCarpetaInspeccionada.c_str(), GetFileExInfoStandard, &datosAtributosCarpeta) != FALSE) {
                            if (memcmp(&datosAtributosCarpeta.ftLastWriteTime,
                                       &estadoVistaCarpetas.ftUltimaModificacionArbol,
                                       sizeof(FILETIME)) != 0) {
                                cargarCarpetaPorIndice(estadoVistaCarpetas.indiceCarpetaActual);
                                invalidarInterfaz();
                            }
                        }
                    }
                }
            } else if (!rutaCarpetaContenedora.empty()) {
                std::wstring claveVistaVacia =
                    L"VISTA_VACIA|" + rutaCarpetaContenedora;
                if (claveVistaVacia != rutaArchivoActivo) {
                    if (asegurarVisorVistaVacia(rutaCarpetaContenedora)) {
                        rutaArchivoActivo = claveVistaVacia;
                        rutaVideoActiva.clear();
                    }
                } else if (tipoContenidoActivo.load() == TipoContenidoArchivo::VistaVacia) {
                    WIN32_FILE_ATTRIBUTE_DATA datosAttrCarpeta;
                    if (GetFileAttributesExW(rutaCarpetaContenedora.c_str(), GetFileExInfoStandard, &datosAttrCarpeta) != FALSE) {
                        if (memcmp(&datosAttrCarpeta.ftLastWriteTime,
                                   &estadoVistaCarpetas.resumenVistaVacia.ftUltimaModificacion,
                                   sizeof(FILETIME)) != 0) {
                            calcularResumenVistaVacia(rutaCarpetaContenedora);
                            invalidarInterfaz();
                        }
                    }
                }
            } else {
                rutaArchivoActivo.clear();
                rutaVideoActiva.clear();
                if (ventanaReproductor) {
                    ShowWindow(ventanaReproductor, SW_HIDE);
                }
            }
            if (ventanaReproductor && IsWindowVisible(ventanaReproductor)) {
                ajustarReproductorAlPanel(&areaPanel);
            }
        }

        DWORD intervaloEspera =
            ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) ? 16 : 100;
        DWORD resultadoEspera = MsgWaitForMultipleObjects(
            1, &eventoDetener, FALSE, intervaloEspera, QS_ALLINPUT);
        if (resultadoEspera == WAIT_OBJECT_0) break;
        if (resultadoEspera == WAIT_FAILED) {
            Wh_Log(L"Falló la espera del hilo reproductor (error %u)",
                   static_cast<unsigned int>(GetLastError()));
            break;
        }
        procesarMensajesDelHilo();
        if (reproductor && ventanaControles &&
            IsWindowVisible(ventanaControles)) {
            actualizarCacheTiempoReproduccion();
        }
    }

    liberarRecursosDelReproductor();
    if (ventanaDelegadoReproductor) {
        DestroyWindow(ventanaDelegadoReproductor);
        ventanaDelegadoReproductor = nullptr;
    }
    // Las referencias COM deben liberarse antes de cerrar el apartamento.
    ventanasShell.liberar();
    if (mfIniciado) {
        MFShutdown();
    }
    CoUninitialize();
    return 0;
}
/**** ended inlining ejecucion/hilo_reproductor.cpp ****/

}  // namespace

/**** start inlining windhawk/ciclo_vida.cpp ****/
namespace {

inline COLORREF parsearColorConfigurado(PCWSTR textoEntrada, COLORREF colorPorDefecto) {
    if (!textoEntrada || !*textoEntrada) return colorPorDefecto;
    std::wstring textoLimpio(textoEntrada);
    size_t primerNoEspacio = textoLimpio.find_first_not_of(L" \t\r\n");
    if (primerNoEspacio == std::wstring::npos) return colorPorDefecto;
    size_t ultimoNoEspacio = textoLimpio.find_last_not_of(L" \t\r\n");
    textoLimpio = textoLimpio.substr(primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);

    if (textoLimpio[0] == L'#') {
        std::wstring textoHexadecimal = textoLimpio.substr(1);
        if (textoHexadecimal.length() == 6) {
            wchar_t* caracterFinal = nullptr;
            unsigned long valorNumerico = wcstoul(textoHexadecimal.c_str(), &caracterFinal, 16);
            if (caracterFinal != textoHexadecimal.c_str()) {
                BYTE componenteRojo = static_cast<BYTE>((valorNumerico >> 16) & 0xFF);
                BYTE componenteVerde = static_cast<BYTE>((valorNumerico >> 8) & 0xFF);
                BYTE componenteAzul = static_cast<BYTE>(valorNumerico & 0xFF);
                return RGB(componenteRojo, componenteVerde, componenteAzul);
            }
        } else if (textoHexadecimal.length() == 3) {
            wchar_t textoRojo[] = { textoHexadecimal[0], textoHexadecimal[0], 0 };
            wchar_t textoVerde[] = { textoHexadecimal[1], textoHexadecimal[1], 0 };
            wchar_t textoAzul[] = { textoHexadecimal[2], textoHexadecimal[2], 0 };
            BYTE componenteRojo = static_cast<BYTE>(wcstoul(textoRojo, nullptr, 16));
            BYTE componenteVerde = static_cast<BYTE>(wcstoul(textoVerde, nullptr, 16));
            BYTE componenteAzul = static_cast<BYTE>(wcstoul(textoAzul, nullptr, 16));
            return RGB(componenteRojo, componenteVerde, componenteAzul);
        }
    }

    int rojoEscaneado = -1;
    int verdeEscaneado = -1;
    int azulEscaneado = -1;
    if (swscanf_s(textoLimpio.c_str(), L"RGB(%d,%d,%d)", &rojoEscaneado, &verdeEscaneado, &azulEscaneado) == 3 ||
        swscanf_s(textoLimpio.c_str(), L"rgb(%d,%d,%d)", &rojoEscaneado, &verdeEscaneado, &azulEscaneado) == 3 ||
        swscanf_s(textoLimpio.c_str(), L"%d,%d,%d", &rojoEscaneado, &verdeEscaneado, &azulEscaneado) == 3 ||
        swscanf_s(textoLimpio.c_str(), L"%d %d %d", &rojoEscaneado, &verdeEscaneado, &azulEscaneado) == 3) {
        if (rojoEscaneado >= 0 && rojoEscaneado <= 255 &&
            verdeEscaneado >= 0 && verdeEscaneado <= 255 &&
            azulEscaneado >= 0 && azulEscaneado <= 255) {
            return RGB(static_cast<BYTE>(rojoEscaneado),
                       static_cast<BYTE>(verdeEscaneado),
                       static_cast<BYTE>(azulEscaneado));
        }
    }

    return colorPorDefecto;
}

void cargarConfiguracion() {
    configuracion.reproduccionAutomatica.store(
        Wh_GetIntSetting(L"autoplay") != 0);
    configuracion.sonidoHabilitado.store(Wh_GetIntSetting(L"sound") != 0);
    configuracion.bloquearTamanoVideo.store(
        Wh_GetIntSetting(L"lock_preview_size_video") != 0);
    configuracion.bloquearTamanoImagen.store(
        Wh_GetIntSetting(L"lock_preview_size_image") != 0);
    configuracion.bloquearTamanoComprimidos.store(
        Wh_GetIntSetting(L"lock_preview_size_compressed") != 0);
    configuracion.bloquearTamanoTexto.store(
        Wh_GetIntSetting(L"lock_preview_size_text") != 0);
    configuracion.bloquearTamanoDocumentos.store(
        Wh_GetIntSetting(L"lock_preview_size_documents") != 0);
    configuracion.bloquearTamanoCodigo.store(
        Wh_GetIntSetting(L"lock_preview_size_code") != 0);
    configuracion.bloquearTamanoVistaCarpetas.store(
        Wh_GetIntSetting(L"lock_preview_size_folder") != 0);
    configuracion.bloquearTamanoVistaVacia.store(
        Wh_GetIntSetting(L"lock_preview_size_empty_view") != 0);
    configuracion.bloquearTamanoDesconocido.store(
        Wh_GetIntSetting(L"lock_preview_size_unknown") != 0);
    configuracion.interaccionArchivosVistaCarpetas.store(
        Wh_GetIntSetting(L"folder_tree_interactive_files") != 0);
    configuracion.arbolCarpetasExpandidoPorDefecto.store(
        Wh_GetIntSetting(L"folder_tree_expanded_by_default") != 0);

    int umbralBajo = Wh_GetIntSetting(L"empty_view_threshold_low_medium");
    if (umbralBajo <= 0 || umbralBajo > 95) umbralBajo = 40;

    int umbralAlto = Wh_GetIntSetting(L"empty_view_threshold_medium_high");
    if (umbralAlto <= 0 || umbralAlto > 100) umbralAlto = 70;

    if (umbralBajo >= umbralAlto) {
        umbralAlto = std::min<int>(100, umbralBajo + 1);
    }

    configuracion.umbralPorcentajeBajoMedioVistaVacia.store(umbralBajo);
    configuracion.umbralPorcentajeMedioAltoVistaVacia.store(umbralAlto);

    PCWSTR textoColorBajo = Wh_GetStringSetting(L"empty_view_color_low");
    COLORREF colorRangoBajo = parsearColorConfigurado(textoColorBajo, RGB(63, 185, 80));
    if (textoColorBajo) Wh_FreeStringSetting(textoColorBajo);

    PCWSTR textoColorMedio = Wh_GetStringSetting(L"empty_view_color_medium");
    COLORREF colorRangoMedio = parsearColorConfigurado(textoColorMedio, RGB(210, 153, 34));
    if (textoColorMedio) Wh_FreeStringSetting(textoColorMedio);

    PCWSTR textoColorAlto = Wh_GetStringSetting(L"empty_view_color_high");
    COLORREF colorRangoAlto = parsearColorConfigurado(textoColorAlto, RGB(248, 81, 73));
    if (textoColorAlto) Wh_FreeStringSetting(textoColorAlto);

    if (colorRangoBajo == colorRangoMedio) {
        colorRangoMedio = RGB(210, 153, 34);
    }
    if (colorRangoMedio == colorRangoAlto || colorRangoBajo == colorRangoAlto) {
        colorRangoAlto = RGB(248, 81, 73);
    }

    configuracion.colorRangoBajoVistaVacia.store(colorRangoBajo);
    configuracion.colorRangoMedioVistaVacia.store(colorRangoMedio);
    configuracion.colorRangoAltoVistaVacia.store(colorRangoAlto);

    int limiteComprimidos = Wh_GetIntSetting(L"compressed_grid_limit");
    if (limiteComprimidos != 4 && limiteComprimidos != 6 &&
        limiteComprimidos != 8 && limiteComprimidos != 10) {
        limiteComprimidos = 4;
    }
    configuracion.limiteCuadriculaComprimidos.store(limiteComprimidos);

    PCWSTR programaComicPorDefecto = Wh_GetStringSetting(L"comic_default_program");
    if (programaComicPorDefecto) {
        configuracion.programaComicPorDefecto = programaComicPorDefecto;
        Wh_FreeStringSetting(programaComicPorDefecto);
    } else {
        configuracion.programaComicPorDefecto.clear();
    }

    configuracion.programasComicsRecordados.clear();
    if (!configuracion.programaComicPorDefecto.empty()) {
        configuracion.programasComicsRecordados.push_back(
            configuracion.programaComicPorDefecto);
    }

    PCWSTR programasComicsRecordadosRaw =
        Wh_GetStringSetting(L"comic_remembered_programs");
    if (programasComicsRecordadosRaw) {
        std::wstring textoLista(programasComicsRecordadosRaw);
        Wh_FreeStringSetting(programasComicsRecordadosRaw);

        size_t posicionInicio = 0;
        while (posicionInicio < textoLista.size()) {
            size_t posicionFin = textoLista.find(L';', posicionInicio);
            if (posicionFin == std::wstring::npos) posicionFin = textoLista.size();
            std::wstring rutaItem =
                textoLista.substr(posicionInicio, posicionFin - posicionInicio);
            size_t primerNoEspacio = rutaItem.find_first_not_of(L" \t\r\n");
            if (primerNoEspacio != std::wstring::npos) {
                size_t ultimoNoEspacio = rutaItem.find_last_not_of(L" \t\r\n");
                rutaItem = rutaItem.substr(
                    primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);
                if (!rutaItem.empty() &&
                    std::find(configuracion.programasComicsRecordados.begin(),
                              configuracion.programasComicsRecordados.end(),
                              rutaItem) ==
                        configuracion.programasComicsRecordados.end()) {
                    configuracion.programasComicsRecordados.push_back(rutaItem);
                }
            }
            posicionInicio = posicionFin + 1;
        }
    }

    PCWSTR programaComprimidosPorDefecto =
        Wh_GetStringSetting(L"compressed_default_program");
    if (programaComprimidosPorDefecto) {
        configuracion.programaComprimidosPorDefecto =
            programaComprimidosPorDefecto;
        Wh_FreeStringSetting(programaComprimidosPorDefecto);
    } else {
        configuracion.programaComprimidosPorDefecto.clear();
    }

    configuracion.programasComprimidosRecordados.clear();
    if (!configuracion.programaComprimidosPorDefecto.empty()) {
        configuracion.programasComprimidosRecordados.push_back(
            configuracion.programaComprimidosPorDefecto);
    }

    PCWSTR programasComprimidosRecordadosRaw =
        Wh_GetStringSetting(L"compressed_remembered_programs");
    if (programasComprimidosRecordadosRaw) {
        std::wstring textoLista(programasComprimidosRecordadosRaw);
        Wh_FreeStringSetting(programasComprimidosRecordadosRaw);

        size_t posicionInicio = 0;
        while (posicionInicio < textoLista.size()) {
            size_t posicionFin = textoLista.find(L';', posicionInicio);
            if (posicionFin == std::wstring::npos) posicionFin = textoLista.size();
            std::wstring rutaItem =
                textoLista.substr(posicionInicio, posicionFin - posicionInicio);
            size_t primerNoEspacio = rutaItem.find_first_not_of(L" \t\r\n");
            if (primerNoEspacio != std::wstring::npos) {
                size_t ultimoNoEspacio = rutaItem.find_last_not_of(L" \t\r\n");
                rutaItem = rutaItem.substr(
                    primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);
                if (!rutaItem.empty() &&
                    std::find(configuracion.programasComprimidosRecordados.begin(),
                              configuracion.programasComprimidosRecordados.end(),
                              rutaItem) ==
                        configuracion.programasComprimidosRecordados.end()) {
                    configuracion.programasComprimidosRecordados.push_back(
                        rutaItem);
                }
            }
            posicionInicio = posicionFin + 1;
        }
    }

    PCWSTR programaCarpetasPorDefecto =
        Wh_GetStringSetting(L"folder_default_program");
    if (programaCarpetasPorDefecto) {
        configuracion.programaCarpetasPorDefecto = programaCarpetasPorDefecto;
        Wh_FreeStringSetting(programaCarpetasPorDefecto);
    } else {
        configuracion.programaCarpetasPorDefecto.clear();
    }

    configuracion.programasCarpetasRecordados.clear();
    if (!configuracion.programaCarpetasPorDefecto.empty()) {
        configuracion.programasCarpetasRecordados.push_back(
            configuracion.programaCarpetasPorDefecto);
    }

    PCWSTR programasCarpetasRecordadosRaw =
        Wh_GetStringSetting(L"folder_remembered_programs");
    if (programasCarpetasRecordadosRaw) {
        std::wstring textoLista(programasCarpetasRecordadosRaw);
        Wh_FreeStringSetting(programasCarpetasRecordadosRaw);

        size_t posicionInicio = 0;
        while (posicionInicio < textoLista.size()) {
            size_t posicionFin = textoLista.find(L';', posicionInicio);
            if (posicionFin == std::wstring::npos) posicionFin = textoLista.size();
            std::wstring rutaItem =
                textoLista.substr(posicionInicio, posicionFin - posicionInicio);
            size_t primerNoEspacio = rutaItem.find_first_not_of(L" \t\r\n");
            if (primerNoEspacio != std::wstring::npos) {
                size_t ultimoNoEspacio = rutaItem.find_last_not_of(L" \t\r\n");
                rutaItem = rutaItem.substr(
                    primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);
                if (!rutaItem.empty() &&
                    std::find(configuracion.programasCarpetasRecordados.begin(),
                              configuracion.programasCarpetasRecordados.end(),
                              rutaItem) ==
                        configuracion.programasCarpetasRecordados.end()) {
                    configuracion.programasCarpetasRecordados.push_back(
                        rutaItem);
                }
            }
            posicionInicio = posicionFin + 1;
        }
    }

    estadoComprimidos.programasComicsRecordados =
        configuracion.programasComicsRecordados;
    estadoComprimidos.programasComunesRecordados =
        configuracion.programasComprimidosRecordados;
    estadoComprimidos.indiceProgramaSeleccionado = 0;

    estadoVistaCarpetas.programasCarpetasRecordados =
        configuracion.programasCarpetasRecordados;
    estadoVistaCarpetas.indiceProgramaSeleccionado = 0;

    configuracionPendiente.store(true);
}

using TipoCreateProcessW = BOOL (WINAPI *)(
    LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
    BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
inline TipoCreateProcessW originalCreateProcessW = nullptr;

inline BOOL WINAPI GanchoCreateProcessW(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation) {
    if ((lpApplicationName && wcsstr(lpApplicationName, L"prevhost.exe") != nullptr) ||
        (lpCommandLine && wcsstr(lpCommandLine, L"prevhost.exe") != nullptr)) {
        SetLastError(ERROR_CANCELLED);
        return FALSE;
    }
    if (originalCreateProcessW) {
        return originalCreateProcessW(
            lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
            bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
            lpStartupInfo, lpProcessInformation);
    }
    return FALSE;
}

using TipoCoCreateInstance = HRESULT (WINAPI *)(
    REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);
inline TipoCoCreateInstance originalCoCreateInstance = nullptr;

inline HRESULT WINAPI GanchoCoCreateInstance(
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD dwClsContext,
    REFIID riid,
    LPVOID* ppv) {
    // IID_IPreviewHandler: {8895b1c6-b41f-4c1c-a562-0d564250836f}
    static const IID IID_IPreviewHandler_Local =
        {0x8895b1c6, 0xb41f, 0x4c1c, {0xa5, 0x62, 0x0d, 0x56, 0x42, 0x50, 0x83, 0x6f}};

    if (IsEqualIID(riid, IID_IPreviewHandler_Local)) {
        return E_NOINTERFACE;
    }

    if (originalCoCreateInstance) {
        return originalCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }
    return E_FAIL;
}

inline bool esProcesoExplorerValido() {
    wchar_t rutaModuloEjecutable[MAX_PATH];
    if (GetModuleFileNameW(nullptr, rutaModuloEjecutable, MAX_PATH) == 0) return true;
    const wchar_t* nombreEjecutable = PathFindFileNameW(rutaModuloEjecutable);
    if (!nombreEjecutable || !*nombreEjecutable) return true;
    return (_wcsicmp(nombreEjecutable, L"explorer.exe") == 0);
}

}  // namespace

BOOL Wh_ModInit() {
    if (!esProcesoExplorerValido()) {
        Wh_Log(L"Módulo omitido por ejecutarse en proceso distinto de explorer.exe");
        return FALSE;
    }

    Wh_SetFunctionHook((void*)CreateProcessW, (void*)GanchoCreateProcessW, (void**)&originalCreateProcessW);
    Wh_SetFunctionHook((void*)CoCreateInstance, (void*)GanchoCoCreateInstance, (void**)&originalCoCreateInstance);

    cargarConfiguracion();
    Gdiplus::GdiplusStartupInput entradaGdiplus;
    if (Gdiplus::GdiplusStartup(&tokenGdiplus, &entradaGdiplus, nullptr) !=
        Gdiplus::Ok) {
        Wh_Log(L"No fue posible inicializar GDI+");
        return FALSE;
    }
    if (!crearRecursosInterfaz()) {
        Wh_Log(L"No fue posible crear las fuentes de la interfaz");
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
        return FALSE;
    }
    if (!registrarClasesDeVentana()) {
        Wh_Log(L"No fue posible registrar las clases de ventana");
        liberarRecursosInterfaz();
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
        return FALSE;
    }
    detenerHilo.store(false);
    eventoDetener = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!eventoDetener) {
        Wh_Log(L"No fue posible crear el evento de detención (error %u)",
               static_cast<unsigned int>(GetLastError()));
        eliminarClasesDeVentana();
        liberarRecursosInterfaz();
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
        return FALSE;
    }
    hiloReproductor =
        CreateThread(nullptr, 0, ejecutarHiloReproductor, nullptr, 0, nullptr);
    if (!hiloReproductor) {
        Wh_Log(L"No fue posible crear el hilo reproductor (error %u)",
               static_cast<unsigned int>(GetLastError()));
        CloseHandle(eventoDetener);
        eventoDetener = nullptr;
        eliminarClasesDeVentana();
        liberarRecursosInterfaz();
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
        return FALSE;
    }
    return TRUE;
}

void Wh_ModSettingsChanged() {
    cargarConfiguracion();
    configuracionPendiente.store(true);
}

void Wh_ModUninit() {
    detenerHilo.store(true);
    if (eventoDetener) SetEvent(eventoDetener);
    if (hiloReproductor) {
        DWORD identificadorHiloReproductor = GetThreadId(hiloReproductor);
        if (identificadorHiloReproductor != 0) {
            PostThreadMessageW(identificadorHiloReproductor, WM_NULL, 0, 0);
        }
        if (ventanaDelegadoReproductor && IsWindow(ventanaDelegadoReproductor)) {
            PostMessageW(ventanaDelegadoReproductor, WM_NULL, 0, 0);
        }
        WaitForSingleObject(hiloReproductor, 3000);
        CloseHandle(hiloReproductor);
        hiloReproductor = nullptr;
    }
    if (eventoDetener) {
        CloseHandle(eventoDetener);
        eventoDetener = nullptr;
    }
    eliminarClasesDeVentana();
    liberarRecursosInterfaz();
    if (tokenGdiplus) {
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
    }
}
/**** ended inlining windhawk/ciclo_vida.cpp ****/
/**** ended inlining useful-sideview.cpp ****/
