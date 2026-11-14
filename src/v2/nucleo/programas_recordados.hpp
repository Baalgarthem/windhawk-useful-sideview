#pragma once
#include <string>
#include <vector>
#include <algorithm>

namespace SideViewV2 {
struct ProgramasRecordadosV2 {
    std::vector<std::wstring> carpetas{L"code.exe", L"explorer.exe", L"cmd.exe", L"powershell.exe"};
    std::vector<std::wstring> comprimidos;
    std::wstring ultimoCarpetas;
    std::wstring ultimoComprimidos;
};
inline ProgramasRecordadosV2 g_programasRecordadosV2;
inline void RecordarProgramaV2(std::vector<std::wstring>& lista, std::wstring& ultimo, const std::wstring& ruta) {
    if (ruta.empty()) return;
    if (std::find(lista.begin(), lista.end(), ruta) == lista.end()) lista.push_back(ruta);
    ultimo = ruta;
}
inline std::wstring ProgramaSeleccionadoV2(const std::vector<std::wstring>& lista, const std::wstring& ultimo) {
    if (!ultimo.empty()) return ultimo;
    return lista.size() == 1 ? lista.front() : std::wstring();
}
}
