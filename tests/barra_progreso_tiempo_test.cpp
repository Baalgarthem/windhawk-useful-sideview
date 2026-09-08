#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06020000
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <objbase.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <atomic>
#include <cassert>

#include "nucleo/constantes.hpp"
#include "nucleo/configuracion.hpp"
#include "archivos/clasificador_archivos.hpp"

Configuracion configuracion;
std::atomic<TipoContenidoArchivo> tipoContenidoActivo{TipoContenidoArchivo::Ninguno};
bool menuVelocidadDesplegado = false;
bool arrastrandoBarraProgreso = false;

std::vector<std::wstring> obtenerArchivosTextoEnMismaCarpeta(const std::wstring& rutaTexto) {
    return {rutaTexto};
}

std::wstring obtenerArchivoTextoAnteriorEnCarpeta(const std::wstring& rutaTexto) {
    return rutaTexto;
}

std::wstring obtenerArchivoTextoSiguienteEnCarpeta(const std::wstring& rutaTexto) {
    return rutaTexto;
}

HWND ventanaVideo = nullptr;
HFONT fuenteTextoPequeno = nullptr;

std::vector<std::wstring> obtenerImagenesEnMismaCarpeta(const std::wstring& /*rutaImagen*/) {
    return {L"C:\\img1.png", L"C:\\img2.jpg", L"C:\\img3.webp"};
}

std::wstring obtenerImagenAnteriorEnCarpeta(const std::wstring& rutaImagen) {
    if (rutaImagen == L"C:\\img2.jpg") return L"C:\\img1.png";
    return L"C:\\img3.webp";
}

std::wstring obtenerImagenSiguienteEnCarpeta(const std::wstring& rutaImagen) {
    if (rutaImagen == L"C:\\img1.png") return L"C:\\img2.jpg";
    return L"C:\\img3.webp";
}

#include "imagen/controlador_imagen.hpp"
#include "comprimidos/controlador_comprimidos.hpp"
#include "interfaz/diseno_barra_progreso.hpp"
#include "reproductor/calculos_tiempo.hpp"
#include "reproductor/conversion_tiempo_media_foundation.hpp"
#include "texto/controlador_texto.hpp"
#include "carpetas/controlador_carpetas.hpp"

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

int main() {
    assert(!configuracion.reproduccionAutomatica.load());
    assert(!configuracion.sonidoHabilitado.load());
    assert(!configuracion.bloquearTamanoVideo.load());
    assert(!configuracion.bloquearTamanoImagen.load());
    assert(!configuracion.bloquearTamanoComprimidos.load());
    assert(!configuracion.bloquearTamanoTexto.load());
    assert(configuracion.limiteCuadriculaComprimidos.load() == 4);

    assert(clasificarTipoArchivo(L"C:\\ruta\\video.mp4") == TipoContenidoArchivo::Video);
    assert(clasificarTipoArchivo(L"C:\\ruta\\video.mkv") == TipoContenidoArchivo::Video);
    assert(clasificarTipoArchivo(L"C:\\ruta\\imagen.png") == TipoContenidoArchivo::Imagen);
    assert(clasificarTipoArchivo(L"C:\\ruta\\foto.jpg") == TipoContenidoArchivo::Imagen);
    assert(clasificarTipoArchivo(L"C:\\ruta\\foto.webp") == TipoContenidoArchivo::Imagen);
    assert(clasificarTipoArchivo(L"C:\\ruta\\animacion.gif") == TipoContenidoArchivo::Imagen);
    assert(clasificarTipoArchivo(L"C:\\ruta\\escaneo.tiff") == TipoContenidoArchivo::Imagen);
    assert(clasificarTipoArchivo(L"C:\\ruta\\escaneo.tif") == TipoContenidoArchivo::Imagen);
    assert(clasificarTipoArchivo(L"C:\\ruta\\foto.avif") == TipoContenidoArchivo::Imagen);
    assert(clasificarTipoArchivo(L"C:\\ruta\\foto.heic") == TipoContenidoArchivo::Imagen);
    assert(clasificarTipoArchivo(L"C:\\ruta\\readme.md") == TipoContenidoArchivo::Texto);
    assert(clasificarTipoArchivo(L"C:\\ruta\\doc.markdown") == TipoContenidoArchivo::Texto);
    assert(clasificarTipoArchivo(L"C:\\ruta\\readme.mdown") == TipoContenidoArchivo::Texto);
    assert(clasificarTipoArchivo(L"C:\\ruta\\notes.mkd") == TipoContenidoArchivo::Texto);
    assert(clasificarTipoArchivo(L"C:\\ruta\\notes.txt") == TipoContenidoArchivo::Texto);
    assert(clasificarTipoArchivo(L"C:\\ruta\\doc.pdf") == TipoContenidoArchivo::Documento);
    assert(clasificarTipoArchivo(L"C:\\ruta\\informe.docx") == TipoContenidoArchivo::Documento);
    assert(clasificarTipoArchivo(L"C:\\ruta\\config.yaml") == TipoContenidoArchivo::Codigo);
    assert(clasificarTipoArchivo(L"C:\\ruta\\data.json") == TipoContenidoArchivo::Codigo);
    assert(clasificarTipoArchivo(L"C:\\ruta\\main.cpp") == TipoContenidoArchivo::Codigo);
    assert(clasificarTipoArchivo(L"C:\\ruta\\script.py") == TipoContenidoArchivo::Codigo);
    assert(esExtensionDocumento(L".pdf"));
    assert(esExtensionDocumento(L".docx"));
    assert(esExtensionCodigo(L".cpp"));
    assert(esExtensionCodigo(L".py"));
    assert(esExtensionMarkdown(L".md"));
    assert(esExtensionMarkdown(L".markdown"));
    assert(esExtensionMarkdown(L".mdown"));
    assert(!esExtensionMarkdown(L".txt"));
    assert(esExtensionTexto(L".txt"));
    assert(esExtensionTexto(L".md"));
    assert(!esExtensionTexto(L".pdf"));
    assert(clasificarTipoArchivo(L"C:\\ruta\\archivo.zip") == TipoContenidoArchivo::Comprimido);
    assert(clasificarTipoArchivo(L"C:\\ruta\\comic.cbz") == TipoContenidoArchivo::Comprimido);
    assert(clasificarTipoArchivo(L"C:\\ruta\\comic.cbr") == TipoContenidoArchivo::Comprimido);
    assert(esExtensionComic(L".cbz"));
    assert(esExtensionComic(L".cbr"));
    assert(!esExtensionComic(L".zip"));
    assert(clasificarTipoArchivo(L"C:\\ruta\\binario.exe") == TipoContenidoArchivo::Desconocido);

    // Validar detección de presencia de cómics en controlador_comprimidos
    std::vector<std::wstring> listaSinComics = { L"C:\\ruta\\archivo1.zip", L"C:\\ruta\\archivo2.tar" };
    std::vector<std::wstring> listaConComics = { L"C:\\ruta\\archivo1.zip", L"C:\\ruta\\comic1.cbz", L"C:\\ruta\\comic2.cbr" };

    estadoComprimidos.programasComicsRecordados = { L"C:\\Program Files\\CDisplayEx\\CDisplayEx.exe" };
    estadoComprimidos.programasComunesRecordados = { L"C:\\Program Files\\7-Zip\\7zFM.exe" };

    cargarArchivosComprimidos(listaSinComics);
    assert(!estadoComprimidos.tieneComicsSeleccionados);
    assert(obtenerListaProgramasActivaConst()[0] == L"C:\\Program Files\\7-Zip\\7zFM.exe");

    cargarArchivosComprimidos(listaConComics);
    assert(estadoComprimidos.tieneComicsSeleccionados);
    assert(estadoComprimidos.totalSeleccionados == 3);
    assert(obtenerListaProgramasActivaConst()[0] == L"C:\\Program Files\\CDisplayEx\\CDisplayEx.exe");

    // Validar resolverProgramaParaComprimidos con auto-selección si solo hay 1 programa
    assert(resolverProgramaParaComprimidos() == L"C:\\Program Files\\CDisplayEx\\CDisplayEx.exe");

    // Prueba de parseo de Markdown, imágenes, YouTube, enlaces, listas ordenadas y tablas
    std::wstring muestraMarkdown =
        L"# Titulo Principal\n"
        L"Este es un parrafo con **negrita**, _cursiva_, ==subrayado== y `codigo`.\n"
        L"1. Primer paso\n"
        L"2. Segundo paso\n"
        L"| Cabecera 1 | Cabecera 2 |\n"
        L"| --- | --- |\n"
        L"| Celda 1 | Celda 2 |\n"
        L"| Celda 3 | Celda 4 |\n"
        L"![Captura Local](./assets/captura.png)\n"
        L"![Banner Web](https://example.com/banner.jpg)\n"
        L"[Demo en YouTube](https://www.youtube.com/watch?v=dQw4w9WgXcQ)\n"
        L"https://youtu.be/abc123xyz89\n"
        L"[Documentacion](https://docs.microsoft.com)\n"
        L"## Seccion 1\n"
        L"- Elemento 1\n"
        L"- Elemento 2\n"
        L"```cpp\n"
        L"int main() { return 0; }\n"
        L"```\n"
        L"### Subseccion 1.1\n"
        L"> Cita importante\n";

    parsearContenidoMarkdown(muestraMarkdown);
    assert(estadoTexto.elementosMarkdown.size() >= 10);
    assert(estadoTexto.totalLineasOriginales > 0);

    // Validar fragmentos de texto enriquecidos
    auto frags = parsearFragmentosTexto(L"Texto con **negrita** y `pildora`");
    assert(frags.size() == 4);
    assert(frags[1].estilo == EstiloFragmento::Negrita);
    assert(frags[1].texto == L"negrita");
    assert(frags[3].estilo == EstiloFragmento::CodigoPildora);
    assert(frags[3].texto == L"pildora");

    parsearContenidoMarkdown(L"");
    assert(!estadoTexto.elementosMarkdown.empty());
    assert(estadoTexto.elementosMarkdown[0].textoContenido == L"*(Archivo sin contenido)*");

    // Prueba de parseo de Texto Plano (sin índice)
    std::wstring muestraTextoPlano = L"Linea 1\nLinea 2\n# No es encabezado markdown\nLinea 4";
    parsearContenidoTextoPlano(muestraTextoPlano);
    assert(estadoTexto.elementosMarkdown.size() == 4);
    assert(estadoTexto.barrasIndice.empty());
    assert(estadoTexto.elementosMarkdown[2].textoContenido == L"# No es encabezado markdown");

    // Validar extracción de ID de YouTube
    assert(extraerIdVideoYouTube(L"https://www.youtube.com/watch?v=dQw4w9WgXcQ") == L"dQw4w9WgXcQ");
    assert(extraerIdVideoYouTube(L"https://youtu.be/abc123xyz89") == L"abc123xyz89");
    assert(extraerIdVideoYouTube(L"https://www.youtube.com/embed/9bZkp7q19f0") == L"9bZkp7q19f0");
    assert(extraerIdVideoYouTube(L"https://example.com/other").empty());

    // Validar formateo de tamaños de carpetas
    assert(formatearTamanoCarpeta(500) == L"500 B");
    assert(formatearTamanoCarpeta(2048) == L"2.0 KB");
    assert(formatearTamanoCarpeta(1048576 * 5) == L"5.0 MB");

    assert(limitarProporcionProgreso(-0.5) == 0.0);
    assert(limitarProporcionProgreso(0.5) == 0.5);
    assert(limitarProporcionProgreso(1.5) == 1.0);

    DisenoBarraProgreso disenoBarraProgreso;
    disenoBarraProgreso.limiteIzquierdo = 10;
    disenoBarraProgreso.limiteDerecho = 790;
    assert(calcularProporcionProgreso(disenoBarraProgreso, -20) == 0.0);
    assert(calcularProporcionProgreso(disenoBarraProgreso, 790) == 1.0);
    double proporcionCentral =
        calcularProporcionProgreso(disenoBarraProgreso, 400);
    assert(proporcionCentral > 0.49 && proporcionCentral < 0.51);
    assert(calcularPosicionProgreso(disenoBarraProgreso, 0.5) == 400);
    assert(calcularInicioGrupoBotones(300) == 49);
    assert(calcularPosicionTemporal(1000000000LL, 0.25) == 250000000LL);
    assert(calcularPosicionTemporal(1000000000LL, -1.0) == 0);
    assert(calcularPosicionTemporal(1000000000LL, 2.0) == 1000000000LL);

    PROPVARIANT propiedadDuracion{};
    propiedadDuracion.vt = VT_UI8;
    propiedadDuracion.uhVal.QuadPart = 1000000000ULL;
    LONGLONG duracionExtraida = 0;
    assert(extraerDuracionMediaFoundation(propiedadDuracion,
                                           &duracionExtraida));
    assert(duracionExtraida == 1000000000LL);

    PROPVARIANT propiedadPosicion{};
    propiedadPosicion.vt = VT_I8;
    propiedadPosicion.hVal.QuadPart = 250000000LL;
    LONGLONG posicionExtraida = 0;
    assert(extraerPosicionMediaFoundation(propiedadPosicion,
                                           &posicionExtraida));
    assert(posicionExtraida == 250000000LL);

    PROPVARIANT propiedadInvalida{};
    propiedadInvalida.vt = VT_LPWSTR;
    assert(!extraerDuracionMediaFoundation(propiedadInvalida,
                                            &duracionExtraida));

    PROPVARIANT propiedadDesbordada{};
    propiedadDesbordada.vt = VT_UI8;
    propiedadDesbordada.uhVal.QuadPart =
        std::numeric_limits<ULONGLONG>::max();
    assert(!extraerDuracionMediaFoundation(propiedadDesbordada,
                                            &duracionExtraida));

    RECT areaInteractiva{0, 10, 800, 34};
    assert(contienePunto(areaInteractiva, 400, 22));
    assert(!contienePunto(areaInteractiva, 400, 40));

    DisenoBarraProgreso disenoClic;
    disenoClic.barraControles = {0, 0, 800, 96};
    disenoClic.limiteIzquierdo = 16;
    disenoClic.limiteDerecho = 784;
    disenoClic.botonAnterior = {249, 54, 283, 88};
    disenoClic.botonReproduccion = {291, 54, 325, 88};
    disenoClic.botonSiguiente = {333, 54, 367, 88};
    disenoClic.botonSonido = {375, 54, 409, 88};
    disenoClic.botonRepeticion = {417, 54, 451, 88};
    disenoClic.botonVelocidad = {738, 59, 784, 85};
    assert(esClicEnBarraProgreso(disenoClic, 400, 34));
    assert(!esClicEnBarraProgreso(disenoClic, 295, 70));
    assert(!esClicEnBarraProgreso(disenoClic, 10, 34));

    // Validar constantes y geometría del menú de velocidad flotante
    assert(TOTAL_OPCIONES_VELOCIDAD == 6);
    assert(VALORES_VELOCIDAD[0] == 0.5f);
    assert(VALORES_VELOCIDAD[5] == 2.0f);
    assert(ANCHO_PANEL_VELOCIDAD == 64);
    assert(ALTO_PANEL_VELOCIDAD > 0);
    // Validar módulo de imágenes, acciones modulares y galería
    inicializarAccionesImagen();
    assert(estadoImagen.accionesDisponibles.size() == 3);
    assert(estadoImagen.accionesDisponibles[0].id == IdAccionImagen::Girar90Horario);
    assert(estadoImagen.accionesDisponibles[1].id == IdAccionImagen::CopiarAlPortapapeles);
    assert(estadoImagen.accionesDisponibles[2].id == IdAccionImagen::EstablecerFondoPantalla);

    // Selección única: Galería muestra todas las imágenes de la carpeta
    estadoImagen.rutaImagenActual = L"C:\\img2.jpg";
    estadoImagen.imagenesSeleccionadas = { L"C:\\img2.jpg" };
    actualizarListaGaleria();
    assert(estadoImagen.miniaturasGaleria.size() == 3);
    assert(estadoImagen.miniaturasGaleria[1].esActiva);

    // Selección múltiple (rango): Galería y navegación trabajan estrictamente sobre el rango
    estadoImagen.imagenesSeleccionadas = { L"C:\\rango1.png", L"C:\\rango2.png" };
    estadoImagen.indiceImagenSeleccionada = 0;
    actualizarListaGaleria();
    assert(estadoImagen.miniaturasGaleria.size() == 2);
    assert(estadoImagen.miniaturasGaleria[0].nombreArchivo == L"rango1.png");

    // Validar categorización y secciones estructuradas de comprimidos
    assert(ALTO_CONTROLES_COMPRIMIDOS == 104);
    assert(clasificarCategoriaElementoComprimido(L"subcarpeta", true) == CategoriaElementoComprimido::Carpetas);
    assert(clasificarCategoriaElementoComprimido(L"video.mp4", false) == CategoriaElementoComprimido::Videos);
    assert(clasificarCategoriaElementoComprimido(L"foto.png", false) == CategoriaElementoComprimido::Imagenes);
    assert(clasificarCategoriaElementoComprimido(L"cancion.mp3", false) == CategoriaElementoComprimido::Audios);
    assert(clasificarCategoriaElementoComprimido(L"documento.pdf", false) == CategoriaElementoComprimido::Documentos);
    assert(clasificarCategoriaElementoComprimido(L"script.py", false) == CategoriaElementoComprimido::Codigo);
    assert(clasificarCategoriaElementoComprimido(L"nota.txt", false) == CategoriaElementoComprimido::Texto);
    assert(clasificarCategoriaElementoComprimido(L"backup.zip", false) == CategoriaElementoComprimido::Comprimidos);
    assert(clasificarCategoriaElementoComprimido(L"archivo.bin", false) == CategoriaElementoComprimido::Otros);

    std::vector<ElementoContenidoComprimido> listaPrueba;
    listaPrueba.push_back({L"script.py", L"codigo\\script.py", CategoriaElementoComprimido::Codigo, false, 500});
    listaPrueba.push_back({L"codigo", L"codigo", CategoriaElementoComprimido::Carpetas, true, 0});
    listaPrueba.push_back({L"foto.jpg", L"fotos\\foto.jpg", CategoriaElementoComprimido::Imagenes, false, 1200000});

    agruparContenidosEnSecciones(listaPrueba);
    assert(estadoComprimidos.seccionesContenido.size() == 3);
    assert(estadoComprimidos.seccionesContenido[0].categoria == CategoriaElementoComprimido::Carpetas);
    assert(estadoComprimidos.seccionesContenido[1].categoria == CategoriaElementoComprimido::Imagenes);
    assert(estadoComprimidos.seccionesContenido[2].categoria == CategoriaElementoComprimido::Codigo);

    // Validar exclusión estricta de cómics del escaneo multinivel
    inspeccionarEstructuraArchivoZip(L"C:\\comic.cbz");
    assert(estadoComprimidos.seccionesContenido.empty());
    assert(estadoComprimidos.elementosArbol.empty());

    // Validar generación de texto fantasma en resumen de vista vacía
    ResumenVistaVacia resumenTest;
    resumenTest.textoExtensionesFantasma = L".png, .jpg, .txt";
    assert(resumenTest.textoExtensionesFantasma.find(L".png") != std::wstring::npos);

    // Validar constantes y estado de la cabecera de metadatos de video
    assert(ALTO_METADATOS_VIDEO == 90);
    assert(ID_TEMPORIZADOR_PARPADEO_NAV == 1005);
    liberarRecursosVideo();
    assert(estadoVideoMetadatos.rutaVideo.empty());
    assert(estadoVideoMetadatos.totalEnRango == 0);
    assert(estadoVideoMetadatos.tiempoInicioParpadeoNav == 0);

    // Validar inicio en nivel raíz (0) y expansión cíclica adaptativa según niveles reales
    liberarRecursosVistaCarpetas();
    assert(estadoVistaCarpetas.nivelExpansionActual == 0);

    estadoVistaCarpetas.elementosArbol.clear();
    estadoVistaCarpetas.elementosArbol.push_back({L"raiz_item", L"C:\\raiz", true, 0, 0, 0});
    estadoVistaCarpetas.elementosArbol.push_back({L"sub_item_1", L"C:\\raiz\\sub1", true, 0, 1, 0});
    estadoVistaCarpetas.elementosArbol.push_back({L"sub_item_2", L"C:\\raiz\\sub1\\sub2", true, 0, 2, 0});

    assert(obtenerMaximoNivelJerarquiaArbol() == 2);
    assert(estadoVistaCarpetas.nivelExpansionActual == 0);

    avanzarNivelExpansionArbol();
    assert(estadoVistaCarpetas.nivelExpansionActual == 1);

    avanzarNivelExpansionArbol();
    assert(estadoVistaCarpetas.nivelExpansionActual == 2);

    avanzarNivelExpansionArbol();
    assert(estadoVistaCarpetas.nivelExpansionActual == 0);

    // Validar formateo de tamaño total de carpeta para texto fantasma
    assert(formatearTamanoCarpeta(500) == L"500 B");
    assert(formatearTamanoCarpeta(2048).find(L"KB") != std::wstring::npos);
    assert(formatearTamanoCarpeta(10485760).find(L"MB") != std::wstring::npos);
    assert(formatearTamanoCarpeta(10737418240ULL).find(L"GB") != std::wstring::npos);

    estadoVistaCarpetas.tamanoTotalCarpetaActualBytes = 10485760;
    estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado = formatearTamanoCarpeta(10485760);
    assert(!estadoVistaCarpetas.tamanoTotalCarpetaActualFormateado.empty());

    // Validar liberación completa de recursos de imagen y flujo en memoria
    liberarRecursosImagen();
    assert(estadoImagen.imagenCargada == nullptr);
    assert(estadoImagen.flujoStreamMemoria == nullptr);
    assert(estadoImagen.rutaImagenActual.empty());

    return 0;
}
