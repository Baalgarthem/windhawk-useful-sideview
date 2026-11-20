#pragma once

#include <windows.h>
#include <bcrypt.h>
#include <shlwapi.h>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace SideViewV2 {

struct DatosArchivoComparacionV2 {
    std::wstring ruta;
    std::wstring nombre;
    std::wstring extension;
    std::wstring fps{L"Pendiente"};
    std::uintmax_t bytes{0};
    std::wstring sha256{L"No disponible"};
};

struct EstadoComparadorArchivosV2 {
    DatosArchivoComparacionV2 izquierda;
    DatosArchivoComparacionV2 derecha;
    std::wstring mensaje{L"Comparación general de archivos."};
};

class ServicioComparadorArchivosV2 {
public:
    virtual ~ServicioComparadorArchivosV2() = default;

    virtual EstadoComparadorArchivosV2 CrearComparacion(
        const std::vector<std::wstring>& seleccion) const {
        EstadoComparadorArchivosV2 estado;
        if (seleccion.size() != 2) {
            estado.mensaje = L"Se requieren exactamente dos archivos del mismo tipo.";
            return estado;
        }
        estado.izquierda = LeerDatos(seleccion[0]);
        estado.derecha = LeerDatos(seleccion[1]);
        estado.mensaje = L"Comparación general lista.";
        return estado;
    }

    std::wstring FormatearTamano(std::uintmax_t bytes) const {
        static constexpr const wchar_t* unidades[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
        double valor = static_cast<double>(bytes);
        std::size_t unidad = 0;
        constexpr std::size_t cantidad = sizeof(unidades) / sizeof(unidades[0]);
        while (valor >= 1024.0 && unidad + 1 < cantidad) {
            valor /= 1024.0;
            ++unidad;
        }
        std::wostringstream salida;
        salida << std::fixed << std::setprecision(
            unidad == 0 ? 0 : valor < 10.0 ? 2 : 1)
               << valor << L" " << unidades[unidad];
        return salida.str();
    }

    std::wstring FormatearDiferencia(std::uintmax_t izquierda,
                                     std::uintmax_t derecha) const {
        const bool negativa = derecha < izquierda;
        const std::uintmax_t magnitud = negativa ? izquierda - derecha : derecha - izquierda;
        return std::wstring(negativa ? L"-" : L"+") + FormatearTamano(magnitud);
    }

protected:
    static DatosArchivoComparacionV2 LeerDatos(const std::wstring& ruta) {
        DatosArchivoComparacionV2 datos;
        datos.ruta = ruta;
        const wchar_t* nombre = PathFindFileNameW(ruta.c_str());
        datos.nombre = (nombre && *nombre) ? nombre : L"Archivo";
        const wchar_t* extension = PathFindExtensionW(ruta.c_str());
        datos.extension = extension ? extension : L"";

        WIN32_FILE_ATTRIBUTE_DATA atributos{};
        if (GetFileAttributesExW(ruta.c_str(), GetFileExInfoStandard, &atributos)) {
            ULARGE_INTEGER tamano{};
            tamano.HighPart = atributos.nFileSizeHigh;
            tamano.LowPart = atributos.nFileSizeLow;
            datos.bytes = static_cast<std::uintmax_t>(tamano.QuadPart);
        }
        datos.sha256 = CalcularSha256(ruta);
        return datos;
    }

private:
    static std::wstring CalcularSha256(const std::wstring& ruta) {
        HANDLE archivo = CreateFileW(ruta.c_str(), GENERIC_READ,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                     nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (archivo == INVALID_HANDLE_VALUE) return L"No disponible";

        BCRYPT_ALG_HANDLE algoritmo = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        PUCHAR objeto = nullptr;
        std::wstring resultado = L"No disponible";
        do {
            if (BCryptOpenAlgorithmProvider(&algoritmo, BCRYPT_SHA256_ALGORITHM,
                                            nullptr, 0) != 0) break;
            DWORD tamanoObjeto = 0;
            DWORD bytesResultado = 0;
            if (BCryptGetProperty(algoritmo, BCRYPT_OBJECT_LENGTH,
                                  reinterpret_cast<PUCHAR>(&tamanoObjeto),
                                  sizeof(tamanoObjeto), &bytesResultado, 0) != 0 ||
                tamanoObjeto == 0) break;
            objeto = static_cast<PUCHAR>(HeapAlloc(GetProcessHeap(), 0, tamanoObjeto));
            if (!objeto || BCryptCreateHash(algoritmo, &hash, objeto, tamanoObjeto,
                                            nullptr, 0, 0) != 0) break;

            std::vector<BYTE> buffer(1024 * 1024);
            bool lecturaValida = true;
            for (;;) {
                DWORD leidos = 0;
                if (!ReadFile(archivo, buffer.data(), static_cast<DWORD>(buffer.size()),
                              &leidos, nullptr)) {
                    lecturaValida = false;
                    break;
                }
                if (leidos == 0) break;
                if (BCryptHashData(hash, buffer.data(), leidos, 0) != 0) {
                    lecturaValida = false;
                    break;
                }
            }
            if (!lecturaValida) break;

            std::vector<BYTE> digest(32);
            if (BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0) != 0) break;
            std::wostringstream hex;
            hex << std::hex << std::setfill(L'0');
            for (BYTE byte : digest) hex << std::setw(2) << static_cast<unsigned>(byte);
            resultado = hex.str();
        } while (false);

        if (hash) BCryptDestroyHash(hash);
        if (objeto) HeapFree(GetProcessHeap(), 0, objeto);
        if (algoritmo) BCryptCloseAlgorithmProvider(algoritmo, 0);
        CloseHandle(archivo);
        return resultado;
    }
};

} // namespace SideViewV2
