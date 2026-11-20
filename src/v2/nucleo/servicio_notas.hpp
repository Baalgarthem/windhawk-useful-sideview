#pragma once

#include <windows.h>
#include "v2/nucleo/servicio_persistencia_windhawk.hpp"

#include <algorithm>
#include <cstddef>
#include <cwchar>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace SideViewV2 {

enum class FuenteNotaV2 {
    Nativa,
    WinKlok,
};

struct NotaV2 {
    int id{0};
    std::wstring titulo;
    std::wstring cuerpo;
    long long modificado{0};
    FuenteNotaV2 fuente{FuenteNotaV2::Nativa};
    int idWinKlok{0};
};

struct ResultadoSincronizacionNotasV2 {
    bool disponible{false};
    bool realizada{false};
    int importadas{0};
    int exportadas{0};
    std::wstring mensaje;
};

struct NotaWinKlokV2 {
    int id{0};
    std::wstring titulo;
    std::wstring cuerpo;
    long long modificado{0};
};

// Adaptador de compatibilidad estrictamente encapsulado. No usa el almacén de
// Useful Side View para fingir que es el de otro mod: resuelve únicamente el
// almacén LocalStorage de WinKlok a través del engine.ini de Windhawk.
class AlmacenNotasWinKlokV2 final {
public:
    bool Leer(std::vector<NotaWinKlokV2>& notas, int& siguienteId, bool crear = false) const {
        Backend backend{};
        if (!Resolver(backend, crear)) return false;

        std::wstring ids;
        if (!LeerCadena(backend, L"noteIds", ids)) ids.clear();
        notas.clear();
        int maxId = 0;
        for (const std::wstring& token : Separar(ids, L',')) {
            const int id = AEntero(token, 0);
            if (id <= 0 || std::any_of(notas.begin(), notas.end(), [id](const auto& n) { return n.id == id; })) {
                continue;
            }

            std::wstring valor;
            if (!LeerGrande(backend, L"note" + std::to_wstring(id), valor)) continue;
            NotaWinKlokV2 nota{};
            if (!DeserializarNota(valor, nota)) continue;
            nota.id = id;
            notas.push_back(std::move(nota));
            maxId = std::max(maxId, id);
        }

        int guardado = 1;
        LeerEntero(backend, L"noteNext", guardado);
        siguienteId = std::max(maxId + 1, std::max(1, guardado));
        std::stable_sort(notas.begin(), notas.end(), [](const auto& a, const auto& b) {
            return a.modificado > b.modificado;
        });
        return true;
    }

    bool Guardar(const std::vector<NotaWinKlokV2>& notas, int siguienteId,
                 const std::vector<int>& eliminar) const {
        Backend backend{};
        if (!Resolver(backend, true)) return false;

        std::vector<int> ids;
        ids.reserve(notas.size());
        for (const auto& nota : notas) {
            if (nota.id <= 0 || std::find(eliminar.begin(), eliminar.end(), nota.id) != eliminar.end()) continue;
            if (std::find(ids.begin(), ids.end(), nota.id) == ids.end()) ids.push_back(nota.id);
            if (!GuardarGrande(backend, L"note" + std::to_wstring(nota.id), SerializarNota(nota))) return false;
        }

        for (int id : eliminar) {
            EliminarGrande(backend, L"note" + std::to_wstring(id));
        }

        std::wstring indice;
        for (int id : ids) {
            if (!indice.empty()) indice += L',';
            indice += std::to_wstring(id);
        }
        if (!GuardarCadena(backend, L"noteIds", indice) || !GuardarEntero(backend, L"noteNext", std::max(1, siguienteId))) {
            return false;
        }
        VaciarCacheIni(backend);
        return true;
    }

    static constexpr std::size_t TamanoFragmento() { return kTamanoFragmento; }
    static constexpr int MaximoFragmentos() { return kMaximoFragmentos; }

private:
    enum class TipoBackend {
        Ninguno,
        Registro,
        Ini,
    };

    struct Backend {
        TipoBackend tipo{TipoBackend::Ninguno};
        HKEY raiz{nullptr};
        std::wstring subclave;
        std::wstring archivoIni;
    };

    static constexpr std::size_t kTamanoFragmento = 30000;
    static constexpr int kMaximoFragmentos = 512;

    static std::wstring Recortar(const std::wstring& valor) {
        const std::size_t primero = valor.find_first_not_of(L" \t\r\n");
        if (primero == std::wstring::npos) return {};
        const std::size_t ultimo = valor.find_last_not_of(L" \t\r\n");
        return valor.substr(primero, ultimo - primero + 1);
    }

    static std::wstring ExpandirVariables(const std::wstring& valor) {
        if (valor.empty()) return {};
        DWORD longitud = ExpandEnvironmentStringsW(valor.c_str(), nullptr, 0);
        if (longitud == 0) return {};
        std::vector<wchar_t> buffer(longitud, L'\0');
        if (!ExpandEnvironmentStringsW(valor.c_str(), buffer.data(), longitud)) return {};
        return std::wstring(buffer.data());
    }

    static std::wstring RutaEngineIni() {
        HMODULE modulo = GetModuleHandleW(L"windhawk.dll");
        if (!modulo) return {};
        std::vector<wchar_t> buffer(32768, L'\0');
        DWORD longitud = GetModuleFileNameW(modulo, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (longitud == 0 || longitud >= buffer.size()) return {};
        std::wstring ruta(buffer.data(), longitud);
        const std::size_t ultima = ruta.find_last_of(L"\\/");
        if (ultima == std::wstring::npos) return {};
        ruta.resize(ultima);
        const std::size_t penultima = ruta.find_last_of(L"\\/");
        if (penultima == std::wstring::npos) return {};
        ruta.resize(penultima + 1);
        return ruta + L"engine.ini";
    }

    static bool ClaveExiste(HKEY raiz, const std::wstring& subclave) {
        HKEY clave = nullptr;
        const LSTATUS estado = RegOpenKeyExW(raiz, subclave.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &clave);
        if (clave) RegCloseKey(clave);
        return estado == ERROR_SUCCESS;
    }

    static HKEY ResolverRaiz(const std::wstring& nombre) {
        if (_wcsicmp(nombre.c_str(), L"HKCU") == 0 || _wcsicmp(nombre.c_str(), L"HKEY_CURRENT_USER") == 0) {
            return HKEY_CURRENT_USER;
        }
        if (_wcsicmp(nombre.c_str(), L"HKU") == 0 || _wcsicmp(nombre.c_str(), L"HKEY_USERS") == 0) {
            return HKEY_USERS;
        }
        if (_wcsicmp(nombre.c_str(), L"HKLM") == 0 || _wcsicmp(nombre.c_str(), L"HKEY_LOCAL_MACHINE") == 0) {
            return HKEY_LOCAL_MACHINE;
        }
        return nullptr;
    }

    static bool Resolver(Backend& backend, bool crear) {
        const std::wstring engineIni = RutaEngineIni();
        if (engineIni.empty()) return false;

        wchar_t portable[16]{};
        GetPrivateProfileStringW(L"Storage", L"Portable", L"0", portable, ARRAYSIZE(portable), engineIni.c_str());
        wchar_t appDataBuffer[32768]{};
        GetPrivateProfileStringW(L"Storage", L"AppDataPath", L"", appDataBuffer, ARRAYSIZE(appDataBuffer), engineIni.c_str());
        const std::wstring appData = ExpandirVariables(Recortar(appDataBuffer));
        if (appData.empty()) return false;

        const bool esPortable = _wtoi(portable) != 0;
        if (esPortable) {
            backend.tipo = TipoBackend::Ini;
            backend.archivoIni = appData + L"\\ModsWritable\\winklok.ini";
            const std::wstring mods = appData + L"\\Mods\\winklok.ini";
            const std::wstring writable = appData + L"\\ModsWritable\\winklok.ini";
            const bool instalado = GetFileAttributesW(mods.c_str()) != INVALID_FILE_ATTRIBUTES ||
                                   GetFileAttributesW(writable.c_str()) != INVALID_FILE_ATTRIBUTES;
            if (!instalado &&
                GetFileAttributesW(backend.archivoIni.c_str()) == INVALID_FILE_ATTRIBUTES) {
                return false;
            }
            if (crear) {
                WritePrivateProfileStringW(L"LocalStorage", L"__useful_sideview_probe", L"1", backend.archivoIni.c_str());
                WritePrivateProfileStringW(L"LocalStorage", L"__useful_sideview_probe", nullptr, backend.archivoIni.c_str());
            }
            return true;
        }

        wchar_t registryBuffer[32768]{};
        GetPrivateProfileStringW(L"Storage", L"RegistryKey", L"", registryBuffer, ARRAYSIZE(registryBuffer), engineIni.c_str());
        const std::wstring registryKey = Recortar(registryBuffer);
        const std::size_t separador = registryKey.find(L'\\');
        if (separador == std::wstring::npos) return false;
        const HKEY raiz = ResolverRaiz(registryKey.substr(0, separador));
        if (!raiz) return false;
        const std::wstring base = registryKey.substr(separador + 1);
        const std::wstring instalado = base + L"\\Mods\\winklok";
        const std::wstring localStorage = base + L"\\ModsWritable\\winklok\\LocalStorage";
        if (!ClaveExiste(raiz, localStorage) && !ClaveExiste(raiz, instalado)) return false;
        if (crear) {
            HKEY clave = nullptr;
            const LSTATUS estado = RegCreateKeyExW(raiz, localStorage.c_str(), 0, nullptr, 0,
                                                   KEY_READ | KEY_SET_VALUE | KEY_WOW64_64KEY,
                                                   nullptr, &clave, nullptr);
            if (estado != ERROR_SUCCESS) return false;
            RegCloseKey(clave);
        }
        backend.tipo = TipoBackend::Registro;
        backend.raiz = raiz;
        backend.subclave = localStorage;
        return true;
    }

    static bool LeerCadena(const Backend& backend, const std::wstring& clave, std::wstring& salida) {
        salida.clear();
        if (backend.tipo == TipoBackend::Ini) {
            std::vector<wchar_t> buffer(kTamanoFragmento + 2, L'\0');
            const DWORD longitud = GetPrivateProfileStringW(L"LocalStorage", clave.c_str(), L"", buffer.data(),
                                                             static_cast<DWORD>(buffer.size()), backend.archivoIni.c_str());
            if (longitud == 0 || longitud >= buffer.size() - 1) return false;
            salida.assign(buffer.data(), longitud);
            return true;
        }

        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &registro) != ERROR_SUCCESS) {
            return false;
        }
        DWORD tipo = 0;
        DWORD bytes = 0;
        LSTATUS estado = RegQueryValueExW(registro, clave.c_str(), nullptr, &tipo, nullptr, &bytes);
        if (estado != ERROR_SUCCESS || (tipo != REG_SZ && tipo != REG_EXPAND_SZ) || bytes < sizeof(wchar_t)) {
            RegCloseKey(registro);
            return false;
        }
        std::vector<wchar_t> buffer(bytes / sizeof(wchar_t) + 1, L'\0');
        estado = RegQueryValueExW(registro, clave.c_str(), nullptr, &tipo,
                                  reinterpret_cast<LPBYTE>(buffer.data()), &bytes);
        RegCloseKey(registro);
        if (estado != ERROR_SUCCESS) return false;
        salida.assign(buffer.data());
        return true;
    }

    static bool LeerEntero(const Backend& backend, const std::wstring& clave, int& salida) {
        std::wstring texto;
        if (LeerCadena(backend, clave, texto)) {
            salida = AEntero(texto, salida);
            return true;
        }
        if (backend.tipo != TipoBackend::Registro) return false;
        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &registro) != ERROR_SUCCESS) return false;
        DWORD tipo = 0;
        DWORD valor = 0;
        DWORD bytes = sizeof(valor);
        const LSTATUS estado = RegQueryValueExW(registro, clave.c_str(), nullptr, &tipo,
                                                 reinterpret_cast<LPBYTE>(&valor), &bytes);
        RegCloseKey(registro);
        if (estado != ERROR_SUCCESS || tipo != REG_DWORD || bytes != sizeof(valor)) return false;
        salida = static_cast<int>(valor);
        return true;
    }

    static bool GuardarCadena(const Backend& backend, const std::wstring& clave, const std::wstring& valor) {
        if (backend.tipo == TipoBackend::Ini) {
            return WritePrivateProfileStringW(L"LocalStorage", clave.c_str(), valor.c_str(), backend.archivoIni.c_str()) != FALSE;
        }
        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &registro) != ERROR_SUCCESS) return false;
        const DWORD bytes = static_cast<DWORD>((valor.size() + 1) * sizeof(wchar_t));
        const LSTATUS estado = RegSetValueExW(registro, clave.c_str(), 0, REG_SZ,
                                               reinterpret_cast<const BYTE*>(valor.c_str()), bytes);
        RegCloseKey(registro);
        return estado == ERROR_SUCCESS;
    }

    static bool GuardarEntero(const Backend& backend, const std::wstring& clave, int valor) {
        if (backend.tipo == TipoBackend::Ini) {
            return GuardarCadena(backend, clave, std::to_wstring(valor));
        }
        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &registro) != ERROR_SUCCESS) return false;
        const DWORD valorDword = static_cast<DWORD>(std::max(0, valor));
        const LSTATUS estado = RegSetValueExW(registro, clave.c_str(), 0, REG_DWORD,
                                               reinterpret_cast<const BYTE*>(&valorDword), sizeof(valorDword));
        RegCloseKey(registro);
        return estado == ERROR_SUCCESS;
    }

    static void Eliminar(const Backend& backend, const std::wstring& clave) {
        if (backend.tipo == TipoBackend::Ini) {
            WritePrivateProfileStringW(L"LocalStorage", clave.c_str(), nullptr, backend.archivoIni.c_str());
            return;
        }
        HKEY registro = nullptr;
        if (RegOpenKeyExW(backend.raiz, backend.subclave.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &registro) == ERROR_SUCCESS) {
            RegDeleteValueW(registro, clave.c_str());
            RegCloseKey(registro);
        }
    }

    static void VaciarCacheIni(const Backend& backend) {
        if (backend.tipo == TipoBackend::Ini) {
            WritePrivateProfileStringW(nullptr, nullptr, nullptr, backend.archivoIni.c_str());
        }
    }

    static bool LeerGrande(const Backend& backend, const std::wstring& clave, std::wstring& salida) {
        salida.clear();
        int cantidad = 0;
        LeerEntero(backend, clave + L"_n", cantidad);
        if (cantidad < 0 || cantidad > kMaximoFragmentos) return false;
        if (cantidad == 0) return LeerCadena(backend, clave, salida);
        for (int i = 0; i < cantidad; ++i) {
            std::wstring parte;
            if (!LeerCadena(backend, clave + L"_" + std::to_wstring(i), parte)) return false;
            salida += parte;
        }
        return true;
    }

    static bool GuardarGrande(const Backend& backend, const std::wstring& clave, const std::wstring& valor) {
        int anterior = 0;
        LeerEntero(backend, clave + L"_n", anterior);
        int cantidad = 0;
        for (std::size_t inicio = 0; inicio < valor.size(); inicio += kTamanoFragmento) {
            const std::wstring parte = valor.substr(inicio, kTamanoFragmento);
            if (!GuardarCadena(backend, clave + L"_" + std::to_wstring(cantidad), parte)) return false;
            ++cantidad;
        }
        if (cantidad == 0) {
            if (!GuardarCadena(backend, clave + L"_0", L"")) return false;
            cantidad = 1;
        }
        for (int i = cantidad; i < anterior; ++i) Eliminar(backend, clave + L"_" + std::to_wstring(i));
        if (!GuardarEntero(backend, clave + L"_n", cantidad)) return false;
        Eliminar(backend, clave);
        return true;
    }

    static void EliminarGrande(const Backend& backend, const std::wstring& clave) {
        int cantidad = 0;
        LeerEntero(backend, clave + L"_n", cantidad);
        if (cantidad >= 0 && cantidad <= kMaximoFragmentos) {
            for (int i = 0; i < cantidad; ++i) Eliminar(backend, clave + L"_" + std::to_wstring(i));
        }
        Eliminar(backend, clave + L"_n");
        Eliminar(backend, clave);
    }

    static std::vector<std::wstring> Separar(const std::wstring& valor, wchar_t separador) {
        std::vector<std::wstring> partes;
        std::size_t inicio = 0;
        while (inicio <= valor.size()) {
            const std::size_t fin = valor.find(separador, inicio);
            partes.push_back(valor.substr(inicio, fin == std::wstring::npos ? fin : fin - inicio));
            if (fin == std::wstring::npos) break;
            inicio = fin + 1;
        }
        return partes;
    }

    static int AEntero(const std::wstring& valor, int predeterminado) {
        if (valor.empty()) return predeterminado;
        wchar_t* fin = nullptr;
        const long resultado = wcstol(valor.c_str(), &fin, 10);
        if (!fin || *fin != L'\0' || resultado < 0 || resultado > 2147483647L) return predeterminado;
        return static_cast<int>(resultado);
    }

    static std::wstring Escapar(const std::wstring& valor) {
        std::wstring salida;
        salida.reserve(valor.size() + 16);
        for (wchar_t caracter : valor) {
            switch (caracter) {
                case L'\\': salida += L"\\\\"; break;
                case L'|': salida += L"\\p"; break;
                case L';': salida += L"\\s"; break;
                case L'\n': salida += L"\\n"; break;
                case L'\r': break;
                default: salida += caracter; break;
            }
        }
        return salida;
    }

    static std::wstring Desescapar(const std::wstring& valor) {
        std::wstring salida;
        salida.reserve(valor.size());
        for (std::size_t i = 0; i < valor.size(); ++i) {
            if (valor[i] != L'\\' || i + 1 >= valor.size()) {
                salida += valor[i];
                continue;
            }
            const wchar_t codigo = valor[++i];
            if (codigo == L'n') salida += L'\n';
            else if (codigo == L'p') salida += L'|';
            else if (codigo == L's') salida += L';';
            else if (codigo == L'\\') salida += L'\\';
            else { salida += L'\\'; salida += codigo; }
        }
        return salida;
    }

    static bool DeserializarNota(const std::wstring& valor, NotaWinKlokV2& nota) {
        const std::size_t primero = valor.find(L'|');
        if (primero == std::wstring::npos) return false;
        const std::size_t segundo = valor.find(L'|', primero + 1);
        if (segundo == std::wstring::npos) return false;
        nota.modificado = static_cast<long long>(AEntero(valor.substr(0, primero), 0));
        nota.titulo = Desescapar(valor.substr(primero + 1, segundo - primero - 1));
        nota.cuerpo = Desescapar(valor.substr(segundo + 1));
        return nota.modificado >= 0;
    }

    static std::wstring SerializarNota(const NotaWinKlokV2& nota) {
        return std::to_wstring(std::max(0LL, nota.modificado)) + L"|" +
               Escapar(nota.titulo) + L"|" + Escapar(nota.cuerpo);
    }
};

class ServicioNotasV2 final {
public:
    explicit ServicioNotasV2(
        std::shared_ptr<ServicioPersistenciaWindhawkV2> persistencia,
        std::function<bool()> politicaSincronizacion = {})
        : m_persistencia(std::move(persistencia)),
          m_politicaSincronizacion(std::move(politicaSincronizacion)) {}

    void Cargar() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_cargado) return;
        CargarNativas();
        m_cargado = true;
        ActualizarPuenteWinKlokBloqueado();
    }

    void Recargar() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_cargado) {
            CargarNativas();
            m_cargado = true;
        } else {
            CargarNativas();
        }
        ActualizarPuenteWinKlokBloqueado();
    }

    const std::vector<NotaV2>& Notas() const { return m_notas; }

    const NotaV2* Buscar(int id) const {
        const auto it = std::find_if(m_notas.begin(), m_notas.end(), [id](const auto& nota) { return nota.id == id; });
        return it == m_notas.end() ? nullptr : &*it;
    }

    int Crear(const std::wstring& titulo, const std::wstring& cuerpo) {
        std::lock_guard<std::mutex> lock(m_mutex);
        CargarSiHaceFaltaBloqueado();
        NotaV2 nota{};
        nota.id = m_siguienteId++;
        nota.titulo = titulo.empty() ? L"Nueva nota" : titulo;
        nota.cuerpo = cuerpo;
        nota.modificado = Ahora();
        m_notas.insert(m_notas.begin(), nota);
        GuardarNativasBloqueado();
        if (SincronizacionActiva()) SincronizarBloqueado(false);
        return nota.id;
    }

    bool Actualizar(int id, const std::wstring& titulo, const std::wstring& cuerpo) {
        std::lock_guard<std::mutex> lock(m_mutex);
        CargarSiHaceFaltaBloqueado();
        NotaV2* nota = BuscarMutable(id);
        if (!nota) return false;
        nota->titulo = titulo.empty() ? L"Sin título" : titulo;
        nota->cuerpo = cuerpo;
        nota->modificado = Ahora();
        OrdenarBloqueado();
        GuardarNativasBloqueado();
        if (SincronizacionActiva()) SincronizarBloqueado(false);
        return true;
    }

    bool Eliminar(int id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        CargarSiHaceFaltaBloqueado();
        const auto it = std::find_if(m_notas.begin(), m_notas.end(), [id](const auto& nota) { return nota.id == id; });
        if (it == m_notas.end()) return false;
        if (it->idWinKlok > 0) m_eliminadosWinKlok.push_back(it->idWinKlok);
        m_notas.erase(it);
        GuardarNativasBloqueado();
        if (SincronizacionActiva()) SincronizarBloqueado(false);
        return true;
    }

    bool Copiar(HWND hwnd, int id) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        const NotaV2* nota = Buscar(id);
        if (!nota || !OpenClipboard(hwnd)) return false;
        const std::wstring contenido = nota->cuerpo;
        EmptyClipboard();
        const SIZE_T bytes = (contenido.size() + 1) * sizeof(wchar_t);
        HGLOBAL memoria = GlobalAlloc(GMEM_MOVEABLE, bytes);
        if (!memoria) { CloseClipboard(); return false; }
        void* destino = GlobalLock(memoria);
        if (!destino) { GlobalFree(memoria); CloseClipboard(); return false; }
        memcpy(destino, contenido.c_str(), bytes);
        GlobalUnlock(memoria);
        if (!SetClipboardData(CF_UNICODETEXT, memoria)) {
            GlobalFree(memoria);
            CloseClipboard();
            return false;
        }
        CloseClipboard();
        return true;
    }

    ResultadoSincronizacionNotasV2 ForzarSincronizacion() {
        std::lock_guard<std::mutex> lock(m_mutex);
        CargarSiHaceFaltaBloqueado();
        if (!SincronizacionActiva()) {
            return {false, false, 0, 0,
                    L"Activa \"Sincronización de notas con WinKlok (experimental)\" en Ajustes para usar esta función."};
        }
        return SincronizarBloqueado(true);
    }

    bool WinKlokDisponible() const { return m_winKlokDisponible; }

    bool EditarConPrompt(HWND propietario, int id, bool esNueva) {
        NotaV2 inicial{};
        if (!esNueva) {
            std::lock_guard<std::mutex> lock(m_mutex);
            const NotaV2* existente = Buscar(id);
            if (!existente) return false;
            inicial = *existente;
        }

        PromptNotaV2 prompt{propietario, inicial.titulo, inicial.cuerpo};
        if (!MostrarPromptNota(prompt)) return false;
        if (esNueva) {
            Crear(prompt.titulo, prompt.cuerpo);
        } else {
            Actualizar(id, prompt.titulo, prompt.cuerpo);
        }
        return true;
    }

private:
    struct PromptNotaV2 {
        HWND propietario{nullptr};
        std::wstring titulo;
        std::wstring cuerpo;
        bool aceptado{false};
        HWND campoTitulo{nullptr};
        HWND campoCuerpo{nullptr};
    };

    static constexpr int kControlTitulo = 101;
    static constexpr int kControlCuerpo = 102;

    static LRESULT CALLBACK ProcedimientoPromptNota(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam) {
        auto* estado = reinterpret_cast<PromptNotaV2*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (mensaje == WM_NCCREATE) {
            auto* crear = reinterpret_cast<CREATESTRUCTW*>(lParam);
            estado = reinterpret_cast<PromptNotaV2*>(crear->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(estado));
        }
        if (!estado) return DefWindowProcW(hwnd, mensaje, wParam, lParam);
        if (mensaje == WM_CREATE) {
            CreateWindowW(L"STATIC", L"Título", WS_CHILD | WS_VISIBLE, 14, 12, 70, 20, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            estado->campoTitulo = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", estado->titulo.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                                  82, 10, 390, 24, hwnd, reinterpret_cast<HMENU>(kControlTitulo), GetModuleHandleW(nullptr), nullptr);
            CreateWindowW(L"STATIC", L"Contenido (Markdown opcional)", WS_CHILD | WS_VISIBLE, 14, 44, 240, 20, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            estado->campoCuerpo = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", estado->cuerpo.c_str(), WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL,
                                                  14, 66, 458, 250, hwnd, reinterpret_cast<HMENU>(kControlCuerpo), GetModuleHandleW(nullptr), nullptr);
            CreateWindowW(L"BUTTON", L"Guardar", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 296, 328, 84, 28, hwnd, reinterpret_cast<HMENU>(IDOK), GetModuleHandleW(nullptr), nullptr);
            CreateWindowW(L"BUTTON", L"Cancelar", WS_CHILD | WS_VISIBLE, 388, 328, 84, 28, hwnd, reinterpret_cast<HMENU>(IDCANCEL), GetModuleHandleW(nullptr), nullptr);
            SendMessageW(estado->campoTitulo, EM_SETSEL, 0, -1);
            SetFocus(estado->campoTitulo);
            return 0;
        }
        if (mensaje == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)) {
            if (LOWORD(wParam) == IDOK) {
                const int tituloLargo = GetWindowTextLengthW(estado->campoTitulo);
                const int cuerpoLargo = GetWindowTextLengthW(estado->campoCuerpo);
                std::vector<wchar_t> titulo(static_cast<std::size_t>(tituloLargo) + 1, L'\0');
                std::vector<wchar_t> cuerpo(static_cast<std::size_t>(cuerpoLargo) + 1, L'\0');
                GetWindowTextW(estado->campoTitulo, titulo.data(), static_cast<int>(titulo.size()));
                GetWindowTextW(estado->campoCuerpo, cuerpo.data(), static_cast<int>(cuerpo.size()));
                estado->titulo.assign(titulo.data());
                estado->cuerpo.assign(cuerpo.data());
                if (estado->titulo.empty()) estado->titulo = L"Sin título";
                estado->aceptado = true;
            }
            EnableWindow(estado->propietario, TRUE);
            DestroyWindow(hwnd);
            return 0;
        }
        if (mensaje == WM_CLOSE) {
            EnableWindow(estado->propietario, TRUE);
            DestroyWindow(hwnd);
            return 0;
        }
        return DefWindowProcW(hwnd, mensaje, wParam, lParam);
    }

    static bool MostrarPromptNota(PromptNotaV2& estado) {
        static std::once_flag registroClase;
        std::call_once(registroClase, [] {
            WNDCLASSW clase{};
            clase.lpfnWndProc = ProcedimientoPromptNota;
            clase.hInstance = GetModuleHandleW(nullptr);
            clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            clase.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            clase.lpszClassName = L"UsefulSideView_PromptNota_V2";
            RegisterClassW(&clase);
        });

        EnableWindow(estado.propietario, FALSE);
        HWND ventana = CreateWindowExW(WS_EX_DLGMODALFRAME, L"UsefulSideView_PromptNota_V2", L"Nota",
                                       WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 500, 395,
                                       estado.propietario, nullptr, GetModuleHandleW(nullptr), &estado);
        if (!ventana) { EnableWindow(estado.propietario, TRUE); return false; }
        RECT padre{}, dialogo{};
        GetWindowRect(estado.propietario, &padre);
        GetWindowRect(ventana, &dialogo);
        SetWindowPos(ventana, nullptr,
                     padre.left + ((padre.right - padre.left) - (dialogo.right - dialogo.left)) / 2,
                     padre.top + ((padre.bottom - padre.top) - (dialogo.bottom - dialogo.top)) / 2,
                     0, 0, SWP_NOSIZE | SWP_NOZORDER);
        ShowWindow(ventana, SW_SHOW);
        UpdateWindow(ventana);
        MSG mensaje{};
        while (IsWindow(ventana) && GetMessageW(&mensaje, nullptr, 0, 0) > 0) {
            if (!IsDialogMessageW(ventana, &mensaje)) {
                TranslateMessage(&mensaje);
                DispatchMessageW(&mensaje);
            }
        }
        EnableWindow(estado.propietario, TRUE);
        SetForegroundWindow(estado.propietario);
        return estado.aceptado;
    }

    static long long Ahora() {
        FILETIME ahora{};
        GetSystemTimeAsFileTime(&ahora);
        ULARGE_INTEGER ticks{};
        ticks.LowPart = ahora.dwLowDateTime;
        ticks.HighPart = ahora.dwHighDateTime;
        constexpr ULONGLONG kEpochUnix = 116444736000000000ULL;
        return static_cast<long long>((ticks.QuadPart > kEpochUnix ? ticks.QuadPart - kEpochUnix : 0) / 10000000ULL);
    }

    static std::wstring Escapar(const std::wstring& valor) {
        std::wstring salida;
        salida.reserve(valor.size() + 16);
        for (wchar_t caracter : valor) {
            switch (caracter) {
                case L'\\': salida += L"\\\\"; break;
                case L'|': salida += L"\\p"; break;
                case L';': salida += L"\\s"; break;
                case L'\n': salida += L"\\n"; break;
                case L'\r': break;
                default: salida += caracter; break;
            }
        }
        return salida;
    }

    static std::wstring Desescapar(const std::wstring& valor) {
        std::wstring salida;
        for (std::size_t i = 0; i < valor.size(); ++i) {
            if (valor[i] != L'\\' || i + 1 >= valor.size()) { salida += valor[i]; continue; }
            const wchar_t codigo = valor[++i];
            if (codigo == L'n') salida += L'\n';
            else if (codigo == L'p') salida += L'|';
            else if (codigo == L's') salida += L';';
            else if (codigo == L'\\') salida += L'\\';
            else { salida += L'\\'; salida += codigo; }
        }
        return salida;
    }

    static std::vector<std::wstring> Separar(const std::wstring& valor, wchar_t separador) {
        std::vector<std::wstring> partes;
        std::size_t inicio = 0;
        while (inicio <= valor.size()) {
            const std::size_t fin = valor.find(separador, inicio);
            partes.push_back(valor.substr(inicio, fin == std::wstring::npos ? fin : fin - inicio));
            if (fin == std::wstring::npos) break;
            inicio = fin + 1;
        }
        return partes;
    }

    static int AEntero(const std::wstring& valor, int predeterminado) {
        if (valor.empty()) return predeterminado;
        wchar_t* fin = nullptr;
        const long resultado = wcstol(valor.c_str(), &fin, 10);
        if (!fin || *fin != L'\0' || resultado < 0 || resultado > 2147483647L) return predeterminado;
        return static_cast<int>(resultado);
    }

    static std::wstring ClaveNota(int id) { return L"nota." + std::to_wstring(id); }

    static std::wstring Serializar(const NotaV2& nota) {
        return std::to_wstring(std::max(0LL, nota.modificado)) + L"|" +
               (nota.fuente == FuenteNotaV2::WinKlok ? L"w" : L"n") + L"|" +
               std::to_wstring(std::max(0, nota.idWinKlok)) + L"|" + Escapar(nota.titulo) + L"|" + Escapar(nota.cuerpo);
    }

    static bool Deserializar(const std::wstring& valor, int id, NotaV2& nota) {
        const auto campos = Separar(valor, L'|');
        if (campos.size() < 3) return false;
        nota = {};
        nota.id = id;
        nota.modificado = static_cast<long long>(AEntero(campos[0], 0));
        if (campos.size() >= 5) {
            nota.fuente = campos[1] == L"w" ? FuenteNotaV2::WinKlok : FuenteNotaV2::Nativa;
            nota.idWinKlok = AEntero(campos[2], 0);
            nota.titulo = Desescapar(campos[3]);
            nota.cuerpo = Desescapar(campos[4]);
        } else {
            nota.fuente = FuenteNotaV2::Nativa;
            nota.titulo = Desescapar(campos[1]);
            nota.cuerpo = Desescapar(campos[2]);
        }
        return nota.modificado >= 0;
    }

    NotaV2* BuscarMutable(int id) {
        const auto it = std::find_if(m_notas.begin(), m_notas.end(), [id](const auto& nota) { return nota.id == id; });
        return it == m_notas.end() ? nullptr : &*it;
    }

    void OrdenarBloqueado() {
        std::stable_sort(m_notas.begin(), m_notas.end(), [](const auto& a, const auto& b) {
            return a.modificado > b.modificado;
        });
    }

    void CargarSiHaceFaltaBloqueado() {
        if (m_cargado) return;
        CargarNativas();
        m_cargado = true;
        ActualizarPuenteWinKlokBloqueado();
    }

    bool SincronizacionActiva() const {
        return m_politicaSincronizacion && m_politicaSincronizacion();
    }

    void ActualizarPuenteWinKlokBloqueado() {
        if (SincronizacionActiva()) {
            SincronizarBloqueado(false);
            return;
        }

        // Al desactivar la opción, las notas importadas dejan de formar parte
        // del modelo local y las nativas pierden el vínculo externo. Así el
        // modo predeterminado queda completamente atómico y reversible sin
        // escribir ni borrar datos del otro mod.
        bool cambio = !m_eliminadosWinKlok.empty();
        for (auto it = m_notas.begin(); it != m_notas.end();) {
            if (it->fuente == FuenteNotaV2::WinKlok) {
                BorrarCadenaFragmentada(ClaveNota(it->id));
                it = m_notas.erase(it);
                cambio = true;
                continue;
            }
            if (it->idWinKlok != 0) {
                it->idWinKlok = 0;
                cambio = true;
            }
            ++it;
        }
        m_eliminadosWinKlok.clear();
        m_winKlokDisponible = false;
        if (cambio) {
            OrdenarBloqueado();
            GuardarNativasBloqueado();
        }
    }

    void CargarNativas() {
        m_notas.clear();
        m_eliminadosWinKlok.clear();
        if (!m_persistencia) return;

        const std::wstring indice = m_persistencia->LeerCadena(m_persistencia->CrearClave(L"notas", L"index"));
        int maxId = 0;
        for (const std::wstring& token : Separar(indice, L',')) {
            const int id = AEntero(token, 0);
            if (id <= 0) continue;
            const std::wstring valor = LeerCadenaFragmentada(ClaveNota(id));
            NotaV2 nota{};
            if (!Deserializar(valor, id, nota)) continue;
            m_notas.push_back(std::move(nota));
            maxId = std::max(maxId, id);
        }
        m_siguienteId = std::max(maxId + 1, m_persistencia->LeerEntero(m_persistencia->CrearClave(L"notas", L"next"), 1));
        const std::wstring eliminados = m_persistencia->LeerCadena(m_persistencia->CrearClave(L"notas", L"deleted-winklok"));
        for (const std::wstring& token : Separar(eliminados, L',')) {
            const int id = AEntero(token, 0);
            if (id > 0) m_eliminadosWinKlok.push_back(id);
        }
        OrdenarBloqueado();
    }

    void GuardarNativasBloqueado() const {
        if (!m_persistencia) return;
        std::wstring indice;
        for (const auto& nota : m_notas) {
            if (!indice.empty()) indice += L',';
            indice += std::to_wstring(nota.id);
            GuardarCadenaFragmentada(ClaveNota(nota.id), Serializar(nota));
        }
        m_persistencia->GuardarCadena(m_persistencia->CrearClave(L"notas", L"index"), indice);
        m_persistencia->GuardarEntero(m_persistencia->CrearClave(L"notas", L"next"), std::max(1, m_siguienteId));
        std::wstring eliminados;
        for (int id : m_eliminadosWinKlok) {
            if (!eliminados.empty()) eliminados += L',';
            eliminados += std::to_wstring(id);
        }
        m_persistencia->GuardarCadena(m_persistencia->CrearClave(L"notas", L"deleted-winklok"), eliminados);
    }

    void BorrarCadenaFragmentada(const std::wstring& clave) const {
        if (!m_persistencia) return;
        const int cantidad = m_persistencia->LeerEntero(
            m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()), 0);
        if (cantidad > 0 && cantidad <= AlmacenNotasWinKlokV2::MaximoFragmentos()) {
            for (int i = 0; i < cantidad; ++i) {
                m_persistencia->Eliminar(m_persistencia->CrearClave(
                    L"notas", (clave + L"." + std::to_wstring(i)).c_str()));
            }
        }
        m_persistencia->Eliminar(m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()));
        m_persistencia->Eliminar(m_persistencia->CrearClave(L"notas", clave.c_str()));
    }

    std::wstring LeerCadenaFragmentada(const std::wstring& clave) const {
        if (!m_persistencia) return {};
        const int cantidad = m_persistencia->LeerEntero(m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()), 0);
        if (cantidad <= 0 || cantidad > AlmacenNotasWinKlokV2::MaximoFragmentos()) {
            return m_persistencia->LeerCadena(m_persistencia->CrearClave(L"notas", clave.c_str()));
        }
        std::wstring salida;
        for (int i = 0; i < cantidad; ++i) {
            salida += m_persistencia->LeerCadena(m_persistencia->CrearClave(L"notas", (clave + L"." + std::to_wstring(i)).c_str()));
        }
        return salida;
    }

    void GuardarCadenaFragmentada(const std::wstring& clave, const std::wstring& valor) const {
        if (!m_persistencia) return;
        const int anterior = m_persistencia->LeerEntero(m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()), 0);
        int cantidad = 0;
        if (valor.empty()) {
            m_persistencia->GuardarCadena(m_persistencia->CrearClave(L"notas", (clave + L".0").c_str()), L"");
        }
        for (std::size_t inicio = 0; inicio < valor.size(); inicio += AlmacenNotasWinKlokV2::TamanoFragmento()) {
            m_persistencia->GuardarCadena(m_persistencia->CrearClave(L"notas", (clave + L"." + std::to_wstring(cantidad)).c_str()),
                                           valor.substr(inicio, AlmacenNotasWinKlokV2::TamanoFragmento()));
            ++cantidad;
        }
        if (cantidad == 0) cantidad = 1;
        for (int i = cantidad; i < anterior; ++i) {
            m_persistencia->Eliminar(m_persistencia->CrearClave(L"notas", (clave + L"." + std::to_wstring(i)).c_str()));
        }
        m_persistencia->GuardarEntero(m_persistencia->CrearClave(L"notas", (clave + L".n").c_str()), cantidad);
        m_persistencia->Eliminar(m_persistencia->CrearClave(L"notas", clave.c_str()));
    }

    ResultadoSincronizacionNotasV2 SincronizarBloqueado(bool forzar) {
        ResultadoSincronizacionNotasV2 resultado{};
        std::vector<NotaWinKlokV2> externas;
        int siguienteExterno = 1;
        if (!m_almacenWinKlok.Leer(externas, siguienteExterno, forzar)) {
            m_winKlokDisponible = false;
            if (!forzar) {
                resultado.mensaje = L"WinKlok no está disponible; las notas nativas quedaron guardadas.";
                return resultado;
            }
            resultado.mensaje = L"WinKlok no está instalado o su almacén no está disponible; las notas nativas quedaron guardadas.";
            return resultado;
        }
        resultado.disponible = true;
        m_winKlokDisponible = true;

        std::vector<int> idsEliminar = m_eliminadosWinKlok;
        auto externa = [&externas](int id) -> NotaWinKlokV2* {
            const auto it = std::find_if(externas.begin(), externas.end(), [id](const auto& nota) { return nota.id == id; });
            return it == externas.end() ? nullptr : &*it;
        };

        for (auto it = m_notas.begin(); it != m_notas.end();) {
            auto& nota = *it;
            if (nota.idWinKlok <= 0) {
                nota.idWinKlok = siguienteExterno++;
            }
            NotaWinKlokV2* espejo = externa(nota.idWinKlok);
            if (!espejo) {
                if (nota.fuente == FuenteNotaV2::WinKlok) {
                    BorrarCadenaFragmentada(ClaveNota(nota.id));
                    it = m_notas.erase(it);
                    continue;
                }
                externas.push_back({nota.idWinKlok, nota.titulo, nota.cuerpo, nota.modificado});
                ++resultado.exportadas;
            } else if (espejo->modificado > nota.modificado) {
                nota.titulo = espejo->titulo;
                nota.cuerpo = espejo->cuerpo;
                nota.modificado = espejo->modificado;
                ++resultado.importadas;
            } else if (nota.modificado > espejo->modificado || nota.fuente == FuenteNotaV2::Nativa) {
                espejo->titulo = nota.titulo;
                espejo->cuerpo = nota.cuerpo;
                espejo->modificado = nota.modificado;
                ++resultado.exportadas;
            }
            ++it;
        }

        for (const auto& externaNota : externas) {
            if (std::find(idsEliminar.begin(), idsEliminar.end(), externaNota.id) != idsEliminar.end()) continue;
            const bool conocida = std::any_of(m_notas.begin(), m_notas.end(), [&](const auto& nota) {
                return nota.idWinKlok == externaNota.id;
            });
            if (conocida) continue;
            NotaV2 importada{};
            importada.id = m_siguienteId++;
            importada.titulo = externaNota.titulo;
            importada.cuerpo = externaNota.cuerpo;
            importada.modificado = externaNota.modificado;
            importada.fuente = FuenteNotaV2::WinKlok;
            importada.idWinKlok = externaNota.id;
            m_notas.push_back(std::move(importada));
            ++resultado.importadas;
        }

        const bool guardadoExterno = m_almacenWinKlok.Guardar(externas, siguienteExterno, idsEliminar);
        if (!guardadoExterno) {
            resultado.mensaje = L"No se pudo escribir el almacén de WinKlok; las notas nativas quedaron guardadas.";
            GuardarNativasBloqueado();
            return resultado;
        }
        m_eliminadosWinKlok.clear();
        OrdenarBloqueado();
        GuardarNativasBloqueado();
        resultado.realizada = true;
        resultado.mensaje = resultado.importadas || resultado.exportadas
            ? L"Sincronización completada con WinKlok."
            : L"Las notas ya estaban sincronizadas con WinKlok.";
        return resultado;
    }

    std::shared_ptr<ServicioPersistenciaWindhawkV2> m_persistencia;
    std::function<bool()> m_politicaSincronizacion;
    mutable std::mutex m_mutex;
    mutable AlmacenNotasWinKlokV2 m_almacenWinKlok;
    std::vector<NotaV2> m_notas;
    std::vector<int> m_eliminadosWinKlok;
    int m_siguienteId{1};
    bool m_cargado{false};
    bool m_winKlokDisponible{false};
};

} // namespace SideViewV2
