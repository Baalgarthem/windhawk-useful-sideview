#pragma once
#include <windows.h>
#include <shlwapi.h>
#include <algorithm>
#include <string>
#include <vector>
namespace SideViewV2 {
class ServicioExtensionV2 {
public:
    size_t Cambiar(const std::vector<std::wstring>& archivos, const std::wstring& extension) const {
        std::wstring ext = extension;
        if (ext.empty()) return 0;
        if (ext.front() != L'.') ext.insert(ext.begin(), L'.');
        if (ext.size() > 32 || ext.find_first_of(L"\\/:*?\"<>|\r\n") != std::wstring::npos) return 0;

        size_t total = 0;
        for (const auto& origen : archivos) {
            DWORD atributos = GetFileAttributesW(origen.c_str());
            if (atributos == INVALID_FILE_ATTRIBUTES ||
                (atributos & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) {
                continue;
            }

            // PathRenameExtensionW escribe sobre el buffer recibido. Reservar
            // espacio adicional evita desbordamientos si la nueva extensión
            // es más larga que la original.
            std::vector<wchar_t> buffer(origen.size() + ext.size() + 2, L'\0');
            std::copy(origen.begin(), origen.end(), buffer.begin());
            if (!PathRenameExtensionW(buffer.data(), ext.c_str())) continue;
            std::wstring destino(buffer.data());
            if (_wcsicmp(origen.c_str(), destino.c_str()) == 0 ||
                GetFileAttributesW(destino.c_str()) != INVALID_FILE_ATTRIBUTES) {
                continue;
            }
            if (MoveFileExW(origen.c_str(), destino.c_str(), MOVEFILE_COPY_ALLOWED)) ++total;
        }
        return total;
    }
};
}
