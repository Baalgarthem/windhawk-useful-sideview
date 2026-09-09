#pragma once

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
