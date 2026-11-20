#pragma once

#include <windows.h>
#include <shlwapi.h>
#include <cstdint>
#include <string>
#include <vector>

namespace SideViewV2 {
#pragma pack(push, 1)
struct CabeceraZipLocalV2 {
    uint32_t firma; uint16_t versionMinima; uint16_t banderaGeneral; uint16_t metodoCompresion;
    uint16_t horaModificacion; uint16_t fechaModificacion; uint32_t sumaCrc32;
    uint32_t tamanoComprimido; uint32_t tamanoDescomprimido; uint16_t longitudNombreArchivo;
    uint16_t longitudCampoExtra;
};
#pragma pack(pop)

struct EntradaComprimidoV2 {
    std::wstring nombre;
    bool esDirectorio{false};
    uintmax_t tamanoBytes{0};
};

class ServicioContenidoComprimidoV2 {
public:
    std::vector<EntradaComprimidoV2> Cargar(const std::wstring& ruta) const {
        std::vector<EntradaComprimidoV2> entradas;
        const wchar_t* ext = PathFindExtensionW(ruta.c_str());
        if (_wcsicmp(ext, L".zip") != 0 && _wcsicmp(ext, L".jar") != 0 && _wcsicmp(ext, L".apk") != 0) return entradas;
        HANDLE archivo = CreateFileW(ruta.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (archivo == INVALID_HANDLE_VALUE) return entradas;
        CabeceraZipLocalV2 cabecera{};
        while (entradas.size() < 2000 && Leer(archivo,&cabecera,sizeof(cabecera))) {
            if (cabecera.firma != 0x04034b50 || cabecera.longitudNombreArchivo == 0 || cabecera.longitudNombreArchivo >= 4096) break;
            std::vector<char> nombre(cabecera.longitudNombreArchivo + 1, 0);
            if (!Leer(archivo,nombre.data(),cabecera.longitudNombreArchivo)) break;
            if (cabecera.longitudCampoExtra > 0 && !Saltar(archivo,cabecera.longitudCampoExtra)) break;
            for (char& caracter : nombre) if (caracter == '/') caracter = '\\';
            int longitud = MultiByteToWideChar(CP_UTF8, 0, nombre.data(), -1, nullptr, 0);
            if (longitud <= 0) longitud = MultiByteToWideChar(CP_ACP, 0, nombre.data(), -1, nullptr, 0);
            if (longitud <= 0) break;
            std::vector<wchar_t> nombreW(static_cast<size_t>(longitud));
            UINT pagina = MultiByteToWideChar(CP_UTF8, 0, nombre.data(), -1, nombreW.data(), longitud) > 0 ? CP_UTF8 : CP_ACP;
            if (pagina == CP_ACP) MultiByteToWideChar(CP_ACP, 0, nombre.data(), -1, nombreW.data(), longitud);
            std::wstring relativa=nombreW.data(); bool directorio=!relativa.empty() && relativa.back()==L'\\';
            const wchar_t* solo=PathFindFileNameW(relativa.c_str());
            entradas.push_back({(solo&&*solo)?solo:relativa,directorio,cabecera.tamanoDescomprimido});
            if (cabecera.banderaGeneral & 0x0008) break; // descriptor posterior: no es seguro saltar sin leerlo
            if (!Saltar(archivo,cabecera.tamanoComprimido)) break;
        }
        CloseHandle(archivo); return entradas;
    }
private:
    static bool Leer(HANDLE archivo, void* destino, DWORD bytes) {
        BYTE* salida=static_cast<BYTE*>(destino); DWORD total=0;
        while (total < bytes) { DWORD leidos=0; if (!ReadFile(archivo,salida+total,bytes-total,&leidos,nullptr) || leidos==0) return false; total+=leidos; }
        return true;
    }

    static bool Saltar(HANDLE archivo, DWORD bytes) {
        LARGE_INTEGER desplazamiento{}; desplazamiento.QuadPart=bytes;
        return SetFilePointerEx(archivo, desplazamiento, nullptr, FILE_CURRENT) != FALSE;
    }
};
}
