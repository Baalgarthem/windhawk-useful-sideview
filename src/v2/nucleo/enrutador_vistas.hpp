#pragma once

#include <shlwapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cwctype>

namespace SideViewV2 {

enum class TipoVistaContenido {
    Tutorial,      // Walkthrough Screen (cuando no hay selección)
    Comprimidos,   // Vista funcional de ZIP, RAR, CBZ, CBR, 7Z, TAR, etc.
    Texto,         // Vista de TXT, Markdown (en desarrollo)
    Imagen,        // Vista de PNG, JPG, WebP, GIF, etc. (en desarrollo)
    Video,         // Vista de MP4, MKV, AVI, etc. (en desarrollo)
    Documentos,    // Vista de PDF, DOCX, XLSX, etc. (en desarrollo)
    Codigo,        // Vista de C++, Python, JS, etc. (en desarrollo)
    Carpetas,      // Vista de inspección de árbol de carpetas (en desarrollo)
    Desconocido    // Archivo genérico / visor pendiente (en desarrollo)
};

namespace {

inline void NormalizarExtension(const wchar_t* origen, wchar_t* destino, size_t capacidad) {
    if (!origen || !destino || capacidad == 0) return;
    size_t i = 0;
    while (origen[i] != L'\0' && i + 1 < capacidad) {
        destino[i] = towlower(origen[i]);
        ++i;
    }
    destino[i] = L'\0';
}

inline bool CoincideExtension(const wchar_t* extensionMin, const wchar_t* const* lista, size_t cantidad) {
    if (!extensionMin || *extensionMin == L'\0' || cantidad == 0) return false;
    return std::binary_search(lista, lista + cantidad, extensionMin,
                              [](const wchar_t* a, const wchar_t* b) {
                                  return wcscmp(a, b) < 0;
                              });
}

} // namespace

inline bool EsExtensionComprimido(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_COMPRIMIDOS[] = {
        L".7z", L".bz2", L".bzip2", L".cbr", L".cbz", L".gz", L".gzip",
        L".iso", L".rar", L".tar", L".tbz2", L".tgz", L".txz", L".xz",
        L".z", L".zip", L".zipx"
    };
    return CoincideExtension(ext, EXT_COMPRIMIDOS, ARRAYSIZE(EXT_COMPRIMIDOS));
}

inline bool EsExtensionImagen(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_IMAGENES[] = {
        L".avif", L".bmp", L".dib", L".gif", L".heic", L".heif", L".ico",
        L".jfif", L".jpe", L".jpeg", L".jpg", L".png", L".tif", L".tiff", L".webp"
    };
    return CoincideExtension(ext, EXT_IMAGENES, ARRAYSIZE(EXT_IMAGENES));
}

inline bool EsExtensionVideo(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_VIDEO[] = {
        L".3g2", L".3gp", L".avi", L".flv", L".m2ts", L".m4v", L".mkv",
        L".mov", L".mp4", L".mpeg", L".mpg", L".mts", L".ts", L".webm", L".wmv"
    };
    return CoincideExtension(ext, EXT_VIDEO, ARRAYSIZE(EXT_VIDEO));
}

inline bool EsExtensionTexto(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_TEXTO[] = {
        L".csv", L".ini", L".log", L".markdown", L".md", L".nfo", L".rst", L".tsv", L".txt"
    };
    return CoincideExtension(ext, EXT_TEXTO, ARRAYSIZE(EXT_TEXTO));
}

inline bool EsExtensionCodigo(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_CODIGO[] = {
        L".c", L".cc", L".cpp", L".cs", L".css", L".cxx", L".go", L".h",
        L".hh", L".hpp", L".htm", L".html", L".hxx", L".java", L".js",
        L".json", L".m", L".php", L".ps1", L".py", L".rs", L".sh", L".sql",
        L".swift", L".ts", L".xml", L".yaml", L".yml"
    };
    return CoincideExtension(ext, EXT_CODIGO, ARRAYSIZE(EXT_CODIGO));
}

inline bool EsExtensionDocumento(const wchar_t* extension) {
    if (!extension || *extension == L'\0') return false;
    wchar_t ext[32]{};
    NormalizarExtension(extension, ext, ARRAYSIZE(ext));
    static const wchar_t* const EXT_DOCS[] = {
        L".doc", L".docx", L".epub", L".mobi", L".odp", L".ods", L".odt",
        L".pdf", L".pps", L".ppsx", L".ppt", L".pptx", L".rtf", L".xls", L".xlsx"
    };
    return CoincideExtension(ext, EXT_DOCS, ARRAYSIZE(EXT_DOCS));
}

inline bool EsRutaCarpeta(const wchar_t* ruta) {
    if (!ruta || *ruta == L'\0') return false;
    DWORD attr = GetFileAttributesW(ruta);
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

inline TipoVistaContenido ClasificarRuta(const wchar_t* ruta) {
    if (!ruta || *ruta == L'\0') return TipoVistaContenido::Tutorial;
    if (EsRutaCarpeta(ruta)) return TipoVistaContenido::Carpetas;

    const wchar_t* ext = PathFindExtensionW(ruta);
    if (EsExtensionComprimido(ext)) return TipoVistaContenido::Comprimidos;
    if (EsExtensionImagen(ext)) return TipoVistaContenido::Imagen;
    if (EsExtensionVideo(ext)) return TipoVistaContenido::Video;
    if (EsExtensionTexto(ext)) return TipoVistaContenido::Texto;
    if (EsExtensionCodigo(ext)) return TipoVistaContenido::Codigo;
    if (EsExtensionDocumento(ext)) return TipoVistaContenido::Documentos;

    return TipoVistaContenido::Desconocido;
}

inline TipoVistaContenido ClasificarSeleccion(const std::vector<std::wstring>& seleccion,
                                              const std::wstring& carpeta = L"") {
    if (seleccion.empty()) {
        // Sin archivos seleccionados seguimos dentro de una carpeta: la vista
        // general de carpetas es la misma vista inyectada que la de una carpeta
        // seleccionada, no el walkthrough.
        return carpeta.empty() ? TipoVistaContenido::Tutorial : TipoVistaContenido::Carpetas;
    }
    if (seleccion.size() == 1) {
        return ClasificarRuta(seleccion.front().c_str());
    }

    size_t cComprimidos = 0;
    size_t cImagenes = 0;
    size_t cVideos = 0;
    size_t cTexto = 0;
    size_t cCodigo = 0;
    size_t cDocs = 0;
    size_t cCarpetas = 0;
    size_t cOtros = 0;

    for (const auto& r : seleccion) {
        TipoVistaContenido t = ClasificarRuta(r.c_str());
        switch (t) {
            case TipoVistaContenido::Comprimidos: cComprimidos++; break;
            case TipoVistaContenido::Imagen: cImagenes++; break;
            case TipoVistaContenido::Video: cVideos++; break;
            case TipoVistaContenido::Texto: cTexto++; break;
            case TipoVistaContenido::Codigo: cCodigo++; break;
            case TipoVistaContenido::Documentos: cDocs++; break;
            case TipoVistaContenido::Carpetas: cCarpetas++; break;
            default: cOtros++; break;
        }
    }

    size_t total = seleccion.size();
    if (cComprimidos == total) return TipoVistaContenido::Comprimidos;
    if (cImagenes == total) return TipoVistaContenido::Imagen;
    if (cVideos == total) return TipoVistaContenido::Video;
    if (cTexto == total) return TipoVistaContenido::Texto;
    if (cCarpetas == total) return TipoVistaContenido::Carpetas;
    if (cDocs == total) return TipoVistaContenido::Documentos;
    if (cCodigo == total) return TipoVistaContenido::Codigo;

    // Preferencias prioritarias en selecciones mixtas
    if (cComprimidos > 0) return TipoVistaContenido::Comprimidos;
    if (cImagenes > 0) return TipoVistaContenido::Imagen;
    if (cVideos > 0) return TipoVistaContenido::Video;
    if (cTexto > 0) return TipoVistaContenido::Texto;
    if (cDocs > 0) return TipoVistaContenido::Documentos;
    if (cCodigo > 0) return TipoVistaContenido::Codigo;
    if (cCarpetas > 0) return TipoVistaContenido::Carpetas;

    return TipoVistaContenido::Desconocido;
}

} // namespace SideViewV2
