#pragma once
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <atomic>

namespace SideViewV2 {
inline std::atomic_bool g_mostrarNumerosLineaV2{true};

struct DocumentoTextoV2 { std::wstring ruta; std::vector<std::wstring> lineas; std::vector<size_t> indice; };

class ServicioTextoV2 {
public:
    DocumentoTextoV2 Cargar(const std::wstring& ruta) const {
        DocumentoTextoV2 documento; documento.ruta = ruta;
        HANDLE archivo = CreateFileW(ruta.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (archivo == INVALID_HANDLE_VALUE) return documento;
        LARGE_INTEGER tam{}; if (!GetFileSizeEx(archivo, &tam) || tam.QuadPart > 16 * 1024 * 1024) { CloseHandle(archivo); return documento; }
        std::string bytes(static_cast<size_t>(tam.QuadPart), '\0'); DWORD leidos = 0; size_t totalLeido = 0;
        while (totalLeido < bytes.size()) { DWORD bloque = 0; if (!ReadFile(archivo, bytes.data() + totalLeido, static_cast<DWORD>(bytes.size() - totalLeido), &bloque, nullptr) || bloque == 0) break; totalLeido += bloque; }
        leidos = static_cast<DWORD>(totalLeido);
        CloseHandle(archivo); bytes.resize(leidos);
        int caracteres = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
        UINT pagina = CP_UTF8;
        if (caracteres <= 0) { caracteres = MultiByteToWideChar(CP_ACP, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0); pagina = CP_ACP; }
        std::wstring texto(static_cast<size_t>(std::max(0, caracteres)), L'\0');
        if (caracteres > 0) MultiByteToWideChar(pagina, 0, bytes.data(), static_cast<int>(bytes.size()), texto.data(), caracteres);
        if (!texto.empty() && texto.front() == L'\ufeff') texto.erase(texto.begin());
        size_t inicio = 0;
        while (inicio <= texto.size()) { size_t fin = texto.find_first_of(L"\r\n", inicio); documento.lineas.push_back(texto.substr(inicio, fin == std::wstring::npos ? fin : fin - inicio)); if (fin == std::wstring::npos) break; inicio = fin + 1; if (texto[fin] == L'\r' && inicio < texto.size() && texto[inicio] == L'\n') ++inicio; }
        for (size_t i=0;i<documento.lineas.size();++i) if (documento.lineas[i].find_first_not_of(L" \t") != std::wstring::npos && (documento.lineas[i][0] == L'#' || documento.lineas[i].find(L"TODO") != std::wstring::npos)) documento.indice.push_back(i);
        return documento;
    }
    bool Copiar(const std::wstring& texto) const { if(!OpenClipboard(nullptr)) return false; if(!EmptyClipboard()){CloseClipboard();return false;} HGLOBAL h=GlobalAlloc(GMEM_MOVEABLE,(texto.size()+1)*sizeof(wchar_t)); if(!h){CloseClipboard();return false;} void* p=GlobalLock(h); if(!p){GlobalFree(h);CloseClipboard();return false;} memcpy(p,texto.c_str(),(texto.size()+1)*sizeof(wchar_t)); GlobalUnlock(h); if(!SetClipboardData(CF_UNICODETEXT,h)){GlobalFree(h);CloseClipboard();return false;} CloseClipboard(); return true; }
    bool Renombrar(HWND owner, const std::wstring& actual) const {
        if (actual.empty()) return false;
        size_t separador = actual.find_last_of(L"\\/");
        std::wstring carpeta = separador == std::wstring::npos ? L"" : actual.substr(0, separador);
        const wchar_t* nombreActual = separador == std::wstring::npos ? actual.c_str() : actual.c_str() + separador + 1;

        std::vector<wchar_t> destino(32768, L'\0');
        wcsncpy_s(destino.data(), destino.size(), nombreActual, _TRUNCATE);
        OPENFILENAMEW dialogo{};
        dialogo.lStructSize = sizeof(dialogo);
        dialogo.hwndOwner = owner;
        dialogo.lpstrFile = destino.data();
        dialogo.nMaxFile = static_cast<DWORD>(destino.size());
        dialogo.lpstrTitle = L"Renombrar archivo";
        dialogo.lpstrFilter = L"Todos los archivos (*.*)\0*.*\0";
        dialogo.Flags = OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN | OFN_NODEREFERENCELINKS;
        if (!GetSaveFileNameW(&dialogo)) return false;

        std::wstring nuevoNombre = destino.data();
        size_t ultimoSeparador = nuevoNombre.find_last_of(L"\\/");
        if (ultimoSeparador != std::wstring::npos) nuevoNombre = nuevoNombre.substr(ultimoSeparador + 1);
        if (nuevoNombre.empty() || nuevoNombre == L"." || nuevoNombre == L".." ||
            nuevoNombre.find_first_of(L"\\/:*?\"<>|\r\n") != std::wstring::npos) {
            return false;
        }

        std::wstring nueva = carpeta.empty() ? nuevoNombre : carpeta + L"\\" + nuevoNombre;
        if (_wcsicmp(actual.c_str(), nueva.c_str()) == 0 ||
            GetFileAttributesW(nueva.c_str()) != INVALID_FILE_ATTRIBUTES) return false;
        return MoveFileW(actual.c_str(), nueva.c_str()) != FALSE;
    }
};
}
