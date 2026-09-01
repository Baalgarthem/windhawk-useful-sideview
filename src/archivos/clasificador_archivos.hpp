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

bool esExtensionDeLista(const wchar_t* extension,
                        const wchar_t* const* listaExtensiones,
                        size_t cantidadExtensiones) {
    if (!extension || *extension == L'\0') {
        return false;
    }
    for (size_t posicion = 0; posicion < cantidadExtensiones; ++posicion) {
        if (lstrcmpiW(extension, listaExtensiones[posicion]) == 0) {
            return true;
        }
    }
    return false;
}

}  // namespace

inline bool esExtensionVideo(const wchar_t* extension) {
    static const wchar_t* const EXTENSIONES_VIDEO[] = {
        L".mp4",  L".m4v", L".mov", L".wmv", L".avi",
        L".mkv",  L".webm", L".mpg", L".mpeg", L".mts",
        L".m2ts", L".3gp", L".flv", L".ts",  L".vob"};
    return esExtensionDeLista(extension, EXTENSIONES_VIDEO,
                              ARRAYSIZE(EXTENSIONES_VIDEO));
}

inline bool esExtensionImagen(const wchar_t* extension) {
    static const wchar_t* const EXTENSIONES_IMAGEN[] = {
        L".jpg",  L".jpeg", L".png",  L".bmp",  L".gif",
        L".webp", L".ico",  L".tiff", L".tif",  L".svg",
        L".avif", L".jfif", L".heic", L".emf",  L".wmf"};
    return esExtensionDeLista(extension, EXTENSIONES_IMAGEN,
                              ARRAYSIZE(EXTENSIONES_IMAGEN));
}

inline bool esExtensionMarkdown(const wchar_t* extension) {
    static const wchar_t* const EXTENSIONES_MARKDOWN[] = {
        L".md", L".markdown", L".mdown", L".mkd", L".mkdn"};
    return esExtensionDeLista(extension, EXTENSIONES_MARKDOWN,
                              ARRAYSIZE(EXTENSIONES_MARKDOWN));
}

inline bool esExtensionTexto(const wchar_t* extension) {
    static const wchar_t* const EXTENSIONES_TEXTO[] = {
        L".txt", L".md", L".markdown", L".mdown", L".mkd", L".mkdn", L".log"};
    return esExtensionDeLista(extension, EXTENSIONES_TEXTO,
                              ARRAYSIZE(EXTENSIONES_TEXTO));
}

inline bool esExtensionDocumento(const wchar_t* extension) {
    static const wchar_t* const EXTENSIONES_DOCUMENTO[] = {
        L".pdf", L".doc", L".docx", L".docm", L".dot",
        L".dotx", L".dotm", L".rtf", L".odt", L".wps",
        L".xps", L".oxps", L".epub"};
    return esExtensionDeLista(extension, EXTENSIONES_DOCUMENTO,
                              ARRAYSIZE(EXTENSIONES_DOCUMENTO));
}

inline bool esExtensionCodigo(const wchar_t* extension) {
    static const wchar_t* const EXTENSIONES_CODIGO[] = {
        L".c", L".cpp", L".cxx", L".cc", L".h", L".hpp", L".hxx", L".inl",
        L".cs", L".vb", L".fs", L".java", L".kt", L".kts", L".scala",
        L".py", L".pyw", L".ipynb", L".js", L".mjs", L".cjs", L".jsx",
        L".ts", L".tsx", L".html", L".htm", L".css", L".scss", L".sass",
        L".less", L".vue", L".svelte", L".php", L".sh", L".bash", L".zsh",
        L".bat", L".cmd", L".ps1", L".psm1", L".psd1", L".vbs", L".lua",
        L".json", L".yaml", L".yml", L".toml", L".xml", L".ini", L".cfg",
        L".conf", L".sql", L".env", L".rs", L".go", L".swift", L".rb",
        L".pl", L".pm", L".hs", L".dart", L".zig", L".r", L".asm", L".s"};
    return esExtensionDeLista(extension, EXTENSIONES_CODIGO,
                              ARRAYSIZE(EXTENSIONES_CODIGO));
}

inline bool esExtensionComic(const wchar_t* extension) {
    static const wchar_t* const EXTENSIONES_COMIC[] = {
        L".cbz", L".cbr", L".cb7", L".cbt", L".cba"};
    return esExtensionDeLista(extension, EXTENSIONES_COMIC,
                              ARRAYSIZE(EXTENSIONES_COMIC));
}

inline bool esExtensionComprimido(const wchar_t* extension) {
    static const wchar_t* const EXTENSIONES_COMPRIMIDO[] = {
        L".zip", L".rar", L".7z",  L".tar", L".gz",
        L".bz2", L".xz",  L".cab", L".iso", L".tgz",
        L".cbz", L".cbr", L".cb7", L".cbt", L".cba"};
    return esExtensionDeLista(extension, EXTENSIONES_COMPRIMIDO,
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
