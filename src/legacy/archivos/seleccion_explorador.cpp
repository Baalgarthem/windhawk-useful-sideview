#include "archivos/clasificador_archivos.hpp"

inline bool esRutaDeVideo(const wchar_t* rutaArchivo) {
    if (!rutaArchivo || !*rutaArchivo || PathIsDirectoryW(rutaArchivo)) {
        return false;
    }
    return clasificarTipoArchivo(rutaArchivo) == TipoContenidoArchivo::Video;
}

inline bool esRutaDeImagen(const wchar_t* rutaArchivo) {
    if (!rutaArchivo || !*rutaArchivo || PathIsDirectoryW(rutaArchivo)) {
        return false;
    }
    return clasificarTipoArchivo(rutaArchivo) == TipoContenidoArchivo::Imagen;
}

inline std::vector<std::wstring> obtenerArchivosEnMismaCarpetaPorTipo(
    const std::wstring& rutaArchivoActual,
    TipoContenidoArchivo tipoRequerido) {
    if (rutaArchivoActual.empty()) return {};

    wchar_t buferDirectorio[MAX_PATH];
    wcsncpy_s(buferDirectorio, rutaArchivoActual.c_str(), _TRUNCATE);
    PathRemoveFileSpecW(buferDirectorio);

    std::wstring patronBusqueda = std::wstring(buferDirectorio) + L"\\*";
    WIN32_FIND_DATAW datosArchivo;
    HANDLE manejadorBusqueda =
        FindFirstFileW(patronBusqueda.c_str(), &datosArchivo);
    if (manejadorBusqueda == INVALID_HANDLE_VALUE) return {};

    std::vector<std::wstring> listaArchivos;
    do {
        if ((datosArchivo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            std::wstring rutaCompleta = std::wstring(buferDirectorio) + L"\\" +
                                        datosArchivo.cFileName;
            bool coincideTipo = false;
            if (tipoRequerido == TipoContenidoArchivo::Video) {
                coincideTipo = esRutaDeVideo(rutaCompleta.c_str());
            } else if (tipoRequerido == TipoContenidoArchivo::Imagen) {
                coincideTipo = esRutaDeImagen(rutaCompleta.c_str());
            } else {
                coincideTipo =
                    clasificarTipoArchivo(rutaCompleta.c_str()) == tipoRequerido;
            }
            if (coincideTipo) {
                listaArchivos.push_back(rutaCompleta);
            }
        }
    } while (FindNextFileW(manejadorBusqueda, &datosArchivo));
    FindClose(manejadorBusqueda);

    std::sort(listaArchivos.begin(), listaArchivos.end(),
              [](const std::wstring& primerRuta,
                 const std::wstring& segundaRuta) {
                  return StrCmpLogicalW(primerRuta.c_str(),
                                        segundaRuta.c_str()) < 0;
              });

    return listaArchivos;
}

inline std::vector<std::wstring> obtenerVideosEnMismaCarpeta(
    const std::wstring& rutaVideoActual) {
    return obtenerArchivosEnMismaCarpetaPorTipo(
        rutaVideoActual, TipoContenidoArchivo::Video);
}

inline std::vector<std::wstring> obtenerImagenesEnMismaCarpeta(
    const std::wstring& rutaImagenActual) {
    return obtenerArchivosEnMismaCarpetaPorTipo(
        rutaImagenActual, TipoContenidoArchivo::Imagen);
}

inline std::wstring obtenerArchivoAnteriorEnCarpeta(
    const std::wstring& rutaArchivoActual,
    TipoContenidoArchivo tipoRequerido) {
    std::vector<std::wstring> listaArchivos =
        obtenerArchivosEnMismaCarpetaPorTipo(rutaArchivoActual, tipoRequerido);
    if (listaArchivos.empty()) return {};

    auto iteradorActual =
        std::find(listaArchivos.begin(), listaArchivos.end(), rutaArchivoActual);
    if (iteradorActual == listaArchivos.end()) {
        return listaArchivos.front();
    }
    if (iteradorActual == listaArchivos.begin()) {
        return listaArchivos.back();
    }
    return *(iteradorActual - 1);
}

inline std::wstring obtenerArchivoSiguienteEnCarpeta(
    const std::wstring& rutaArchivoActual,
    TipoContenidoArchivo tipoRequerido) {
    std::vector<std::wstring> listaArchivos =
        obtenerArchivosEnMismaCarpetaPorTipo(rutaArchivoActual, tipoRequerido);
    if (listaArchivos.empty()) return {};

    auto iteradorActual =
        std::find(listaArchivos.begin(), listaArchivos.end(), rutaArchivoActual);
    if (iteradorActual == listaArchivos.end() ||
        iteradorActual + 1 == listaArchivos.end()) {
        return listaArchivos.front();
    }
    return *(iteradorActual + 1);
}

inline std::wstring obtenerVideoAnteriorEnCarpeta(
    const std::wstring& rutaVideoActual) {
    std::vector<std::wstring> listaVideos =
        obtenerVideosEnMismaCarpeta(rutaVideoActual);
    if (listaVideos.empty()) return {};

    auto iteradorActual =
        std::find(listaVideos.begin(), listaVideos.end(), rutaVideoActual);
    if (iteradorActual == listaVideos.end()) {
        return listaVideos.front();
    }
    if (iteradorActual == listaVideos.begin()) {
        return listaVideos.back();
    }
    return *(iteradorActual - 1);
}

inline std::wstring obtenerVideoSiguienteEnCarpeta(
    const std::wstring& rutaVideoActual) {
    std::vector<std::wstring> listaVideos =
        obtenerVideosEnMismaCarpeta(rutaVideoActual);
    if (listaVideos.empty()) return {};

    auto iteradorActual =
        std::find(listaVideos.begin(), listaVideos.end(), rutaVideoActual);
    if (iteradorActual == listaVideos.end() ||
        iteradorActual + 1 == listaVideos.end()) {
        return listaVideos.front();
    }
    return *(iteradorActual + 1);
}

inline std::wstring obtenerImagenAnteriorEnCarpeta(
    const std::wstring& rutaImagenActual) {
    return obtenerArchivoAnteriorEnCarpeta(
        rutaImagenActual, TipoContenidoArchivo::Imagen);
}

inline std::wstring obtenerImagenSiguienteEnCarpeta(
    const std::wstring& rutaImagenActual) {
    return obtenerArchivoSiguienteEnCarpeta(
        rutaImagenActual, TipoContenidoArchivo::Imagen);
}

inline std::vector<std::wstring> obtenerArchivosTextoEnMismaCarpeta(
    const std::wstring& rutaTextoActual) {
    return obtenerArchivosEnMismaCarpetaPorTipo(
        rutaTextoActual, TipoContenidoArchivo::Texto);
}

inline std::wstring obtenerArchivoTextoAnteriorEnCarpeta(
    const std::wstring& rutaTextoActual) {
    return obtenerArchivoAnteriorEnCarpeta(
        rutaTextoActual, TipoContenidoArchivo::Texto);
}

inline std::wstring obtenerArchivoTextoSiguienteEnCarpeta(
    const std::wstring& rutaTextoActual) {
    return obtenerArchivoSiguienteEnCarpeta(
        rutaTextoActual, TipoContenidoArchivo::Texto);
}

[[maybe_unused]] inline std::wstring obtenerDocumentoAnteriorEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoAnteriorEnCarpeta(rutaActual, TipoContenidoArchivo::Documento);
}

[[maybe_unused]] inline std::wstring obtenerDocumentoSiguienteEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoSiguienteEnCarpeta(rutaActual, TipoContenidoArchivo::Documento);
}

[[maybe_unused]] inline std::wstring obtenerCodigoAnteriorEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoAnteriorEnCarpeta(rutaActual, TipoContenidoArchivo::Codigo);
}

[[maybe_unused]] inline std::wstring obtenerCodigoSiguienteEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoSiguienteEnCarpeta(rutaActual, TipoContenidoArchivo::Codigo);
}

[[maybe_unused]] inline std::wstring obtenerComprimidoAnteriorEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoAnteriorEnCarpeta(rutaActual, TipoContenidoArchivo::Comprimido);
}

[[maybe_unused]] inline std::wstring obtenerComprimidoSiguienteEnCarpeta(const std::wstring& rutaActual) {
    return obtenerArchivoSiguienteEnCarpeta(rutaActual, TipoContenidoArchivo::Comprimido);
}

inline HWND normalizarVentanaExploradorAnfitriona(HWND ventanaCandidata) {
    if (!ventanaCandidata || !IsWindow(ventanaCandidata)) return nullptr;
    HWND ventanaRaiz = GetAncestor(ventanaCandidata, GA_ROOT);
    if (ventanaRaiz && IsWindow(ventanaRaiz)) {
        wchar_t claseRaiz[64];
        if (GetClassNameW(ventanaRaiz, claseRaiz, ARRAYSIZE(claseRaiz)) &&
            lstrcmpW(claseRaiz, L"CabinetWClass") == 0) {
            return ventanaRaiz;
        }
    }
    wchar_t claseCandidata[64];
    if (GetClassNameW(ventanaCandidata, claseCandidata, ARRAYSIZE(claseCandidata)) &&
        lstrcmpW(claseCandidata, L"CabinetWClass") == 0) {
        return ventanaCandidata;
    }
    return nullptr;
}

[[maybe_unused]] inline HWND encontrarContenedorPestanaOExplorador(HWND ventanaVistaDef, HWND ventanaExploradorFallback) {
    if (ventanaVistaDef && IsWindow(ventanaVistaDef)) {
        HWND ventanaRaiz = normalizarVentanaExploradorAnfitriona(ventanaVistaDef);
        if (ventanaRaiz) return ventanaRaiz;
    }
    return normalizarVentanaExploradorAnfitriona(ventanaExploradorFallback);
}

inline HWND obtenerVistaArchivosVisible(HWND ventanaAnfitriona) {
    if (!ventanaAnfitriona || !IsWindow(ventanaAnfitriona)) return nullptr;

    HWND vistaConFoco = nullptr;
    HWND ventanaFoco = GetFocus();
    if (ventanaFoco && IsWindow(ventanaFoco) &&
        (IsChild(ventanaAnfitriona, ventanaFoco) || ventanaFoco == ventanaAnfitriona)) {
        HWND cursorVentana = ventanaFoco;
        while (cursorVentana && cursorVentana != ventanaAnfitriona) {
            wchar_t claseCursor[64];
            if (GetClassNameW(cursorVentana, claseCursor, ARRAYSIZE(claseCursor)) &&
                lstrcmpW(claseCursor, L"SHELLDLL_DefView") == 0 &&
                IsWindowVisible(cursorVentana)) {
                vistaConFoco = cursorVentana;
                break;
            }
            cursorVentana = GetParent(cursorVentana);
        }
    }
    if (vistaConFoco) {
        ventanaVistaArchivosActiva = vistaConFoco;
        return vistaConFoco;
    }

    struct DatosBusquedaVista {
        HWND mejorVista{nullptr};
        int mayorArea{0};
    } datosBusqueda;

    EnumChildWindows(
        ventanaAnfitriona,
        [](HWND hwndHijo, LPARAM lParam) -> BOOL {
            wchar_t claseHijo[64];
            if (GetClassNameW(hwndHijo, claseHijo, ARRAYSIZE(claseHijo)) &&
                lstrcmpW(claseHijo, L"SHELLDLL_DefView") == 0 &&
                IsWindowVisible(hwndHijo)) {
                
                HWND padreHijo = GetParent(hwndHijo);
                if (padreHijo && !IsWindowVisible(padreHijo)) return TRUE;

                RECT rectanguloHijo{};
                GetWindowRect(hwndHijo, &rectanguloHijo);
                int anchoVentana = rectanguloHijo.right - rectanguloHijo.left;
                int altoVentana = rectanguloHijo.bottom - rectanguloHijo.top;
                int areaVentana = anchoVentana * altoVentana;
                if (rectanguloHijo.left > -1000 && anchoVentana > 50 && altoVentana > 50) {
                    auto* d = reinterpret_cast<DatosBusquedaVista*>(lParam);
                    if (areaVentana > d->mayorArea) {
                        d->mayorArea = areaVentana;
                        d->mejorVista = hwndHijo;
                    }
                }
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&datosBusqueda));

    if (datosBusqueda.mejorVista) {
        ventanaVistaArchivosActiva = datosBusqueda.mejorVista;
    }
    return datosBusqueda.mejorVista;
}

inline std::vector<std::wstring> obtenerArchivosSeleccionados(
    IShellWindows* ventanasShell,
    std::wstring* rutaCarpetaActualContenedora = nullptr) {
    DWORD procesoActual = GetCurrentProcessId();

    HWND ventanaPrimerPlano = GetForegroundWindow();
    if (ventanaPrimerPlano) {
        DWORD identificadorProcesoFoco = 0;
        GetWindowThreadProcessId(ventanaPrimerPlano, &identificadorProcesoFoco);
        if (identificadorProcesoFoco == procesoActual) {
            HWND ventanaNormalizada = normalizarVentanaExploradorAnfitriona(ventanaPrimerPlano);
            if (ventanaNormalizada && IsWindowVisible(ventanaNormalizada)) {
                ventanaExploradorAnfitriona = ventanaNormalizada;
            }
        }
    }

    if (!ventanaExploradorAnfitriona || !IsWindow(ventanaExploradorAnfitriona) || !IsWindowVisible(ventanaExploradorAnfitriona)) {
        HWND ventanaCandidataExplorador = FindWindowExW(nullptr, nullptr, L"CabinetWClass", nullptr);
        while (ventanaCandidataExplorador) {
            DWORD identificadorProcesoCandidata = 0;
            GetWindowThreadProcessId(ventanaCandidataExplorador, &identificadorProcesoCandidata);
            if (identificadorProcesoCandidata == procesoActual && IsWindowVisible(ventanaCandidataExplorador)) {
                ventanaExploradorAnfitriona = normalizarVentanaExploradorAnfitriona(ventanaCandidataExplorador);
                break;
            }
            ventanaCandidataExplorador = FindWindowExW(nullptr, ventanaCandidataExplorador, L"CabinetWClass", nullptr);
        }
    }

    if (!ventanasShell || !ventanaExploradorAnfitriona) return {};

    long cantidadVentanasShell = 0;
    if (FAILED(ventanasShell->get_Count(&cantidadVentanasShell))) return {};

    HWND vistaArchivosVisibleEnPantalla = obtenerVistaArchivosVisible(ventanaExploradorAnfitriona);

    auto extraerDatosElementoShell = [&](long indiceItem,
                                         std::vector<std::wstring>* rutasOut,
                                         std::wstring* rutaDirOut,
                                         HWND* hwndDefViewOut,
                                         HWND* hwndAnfitrionaOut) -> bool {
        rutasOut->clear();
        rutaDirOut->clear();
        if (hwndDefViewOut) *hwndDefViewOut = nullptr;
        if (hwndAnfitrionaOut) *hwndAnfitrionaOut = nullptr;

        VARIANT identificadorVentanaShell;
        VariantInit(&identificadorVentanaShell);
        identificadorVentanaShell.vt = VT_I4;
        identificadorVentanaShell.lVal = indiceItem;

        PunteroCom<IDispatch> despachoVentana;
        HRESULT resultadoElemento = ventanasShell->Item(
            identificadorVentanaShell, despachoVentana.recibir());
        VariantClear(&identificadorVentanaShell);
        if (FAILED(resultadoElemento) || !despachoVentana) return false;

        PunteroCom<IWebBrowserApp> explorador;
        if (FAILED(despachoVentana->QueryInterface(
                IID_PPV_ARGS(explorador.recibir()))) || !explorador) {
            return false;
        }

        SHANDLE_PTR identificadorVentana = 0;
        if (FAILED(explorador->get_HWND(&identificadorVentana))) return false;

        HWND ventanaExplorador = reinterpret_cast<HWND>(identificadorVentana);
        HWND ventanaRaizExplorador = normalizarVentanaExploradorAnfitriona(ventanaExplorador);
        if (!ventanaRaizExplorador) return false;

        DWORD identificadorProceso = 0;
        GetWindowThreadProcessId(ventanaRaizExplorador, &identificadorProceso);
        if (identificadorProceso != procesoActual) return false;

        if (hwndAnfitrionaOut) *hwndAnfitrionaOut = ventanaRaizExplorador;

        PunteroCom<IServiceProvider> proveedorServicios;
        if (FAILED(explorador->QueryInterface(
                IID_PPV_ARGS(proveedorServicios.recibir())))) {
            return false;
        }

        PunteroCom<IShellBrowser> navegadorCarpetas;
        if (FAILED(proveedorServicios->QueryService(
                SID_STopLevelBrowser,
                IID_PPV_ARGS(navegadorCarpetas.recibir())))) {
            return false;
        }

        PunteroCom<IShellView> vistaActiva;
        if (FAILED(navegadorCarpetas->QueryActiveShellView(
                vistaActiva.recibir())) || !vistaActiva) {
            return false;
        }

        PunteroCom<IFolderView2> vistaCarpeta;
        if (FAILED(vistaActiva->QueryInterface(
                IID_PPV_ARGS(vistaCarpeta.recibir()))) || !vistaCarpeta) {
            return false;
        }

        HWND ventanaVistaDef = nullptr;
        if (FAILED(vistaActiva->GetWindow(&ventanaVistaDef)) ||
            !ventanaVistaDef || !IsWindow(ventanaVistaDef)) {
            return false;
        }

        if (hwndDefViewOut) *hwndDefViewOut = ventanaVistaDef;

        std::wstring rutaDirectorioActual;
        PunteroCom<IShellItem> elementoDirectorio;
        if (SUCCEEDED(vistaCarpeta->GetFolder(
                IID_PPV_ARGS(elementoDirectorio.recibir()))) && elementoDirectorio) {
            PWSTR rutaDirectorio = nullptr;
            HRESULT hrDir = elementoDirectorio->GetDisplayName(SIGDN_FILESYSPATH, &rutaDirectorio);
            if (SUCCEEDED(hrDir) && rutaDirectorio) {
                std::wstring candidata = rutaDirectorio;
                CoTaskMemFree(rutaDirectorio);
                if (!candidata.empty() && PathFileExistsW(candidata.c_str())) {
                    rutaDirectorioActual = candidata;
                }
            }
            if (rutaDirectorioActual.empty()) {
                PWSTR rutaParsing = nullptr;
                if (SUCCEEDED(elementoDirectorio->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &rutaParsing)) && rutaParsing) {
                    if (PathFileExistsW(rutaParsing) && PathIsDirectoryW(rutaParsing)) {
                        rutaDirectorioActual = rutaParsing;
                    }
                    CoTaskMemFree(rutaParsing);
                }
            }
            if (rutaDirectorioActual.empty()) {
                PWSTR nombreVisible = nullptr;
                if (SUCCEEDED(elementoDirectorio->GetDisplayName(
                        SIGDN_NORMALDISPLAY, &nombreVisible)) && nombreVisible) {
                    rutaDirectorioActual = L"::SISTEMA::" + std::wstring(nombreVisible);
                    CoTaskMemFree(nombreVisible);
                } else {
                    PWSTR rutaParsing = nullptr;
                    if (SUCCEEDED(elementoDirectorio->GetDisplayName(
                            SIGDN_DESKTOPABSOLUTEPARSING, &rutaParsing)) && rutaParsing) {
                        std::wstring parsingMin = rutaParsing;
                        CoTaskMemFree(rutaParsing);
                        std::transform(parsingMin.begin(), parsingMin.end(), parsingMin.begin(), ::towlower);
                        if (parsingMin.find(L"e88865ea-0e1c-4e20-9aa6-ed353b7c4777") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Galería";
                        } else if (parsingMin.find(L"f874310e-b6b7-47dc-bc84-b9e6b38f5903") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Inicio";
                        } else if (parsingMin.find(L"20d04fe0-3aea-1069-a2d8-08002b30309d") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Este equipo";
                        } else if (parsingMin.find(L"645ff040-5081-101b-9f08-00aa002f954e") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Papelera de reciclaje";
                        } else if (parsingMin.find(L"f02c1a0d-be21-4350-88b0-7367fc96ef3c") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Red";
                        } else if (parsingMin.find(L"26ee0668-a00a-44d7-9371-beb064c98683") != std::wstring::npos) {
                            rutaDirectorioActual = L"::SISTEMA::Panel de control";
                        } else {
                            rutaDirectorioActual = L"::SISTEMA::Ubicación del Sistema";
                        }
                    } else {
                        rutaDirectorioActual = L"::SISTEMA::Ubicación del Sistema";
                    }
                }
            }
        }

        *rutaDirOut = rutaDirectorioActual;

        PunteroCom<IShellItemArray> elementosSeleccionados;
        HRESULT resultadoSeleccion =
            vistaCarpeta->GetSelection(TRUE, elementosSeleccionados.recibir());

        if (SUCCEEDED(resultadoSeleccion) && elementosSeleccionados) {
            DWORD cantidadSeleccionados = 0;
            elementosSeleccionados->GetCount(&cantidadSeleccionados);

            for (DWORD posicionElemento = 0;
                 posicionElemento < cantidadSeleccionados;
                 ++posicionElemento) {
                 PunteroCom<IShellItem> elementoSeleccionado;
                 if (SUCCEEDED(elementosSeleccionados->GetItemAt(
                         posicionElemento, elementoSeleccionado.recibir())) &&
                     elementoSeleccionado) {
                     PWSTR rutaElemento = nullptr;
                     HRESULT hrElemento = elementoSeleccionado->GetDisplayName(
                         SIGDN_FILESYSPATH, &rutaElemento);
                     if (FAILED(hrElemento) || !rutaElemento) {
                         hrElemento = elementoSeleccionado->GetDisplayName(
                             SIGDN_DESKTOPABSOLUTEPARSING, &rutaElemento);
                     }
                     if (SUCCEEDED(hrElemento) && rutaElemento) {
                         if (rutaDirectorioActual.empty() ||
                             lstrcmpiW(rutaElemento, rutaDirectorioActual.c_str()) != 0) {
                             if (esArchivoCompatible(rutaElemento)) {
                                 rutasOut->push_back(rutaElemento);
                             }
                         }
                         CoTaskMemFree(rutaElemento);
                     }
                 }
            }
        }

        return true;
    };

    struct VistaCandidataAnfitriona {
        std::vector<std::wstring> rutas;
        std::wstring rutaDirectorio;
        HWND hwndDefView{nullptr};
        bool esVisibleEnPantalla{false};
    };
    std::vector<VistaCandidataAnfitriona> candidatasDeEstaAnfitriona;

    for (long posicionShell = 0; posicionShell < cantidadVentanasShell; ++posicionShell) {
        std::vector<std::wstring> rutasSeleccionadas;
        std::wstring rutaDirectorio;
        HWND hwndDefView = nullptr;
        HWND hwndAnfitrionaItem = nullptr;
        if (extraerDatosElementoShell(posicionShell, &rutasSeleccionadas, &rutaDirectorio, &hwndDefView, &hwndAnfitrionaItem)) {
            // STRICT FILTER: Must belong strictly to THIS active Explorer window
            if (hwndAnfitrionaItem != ventanaExploradorAnfitriona) {
                continue;
            }

            bool esLaVistaActiva = (vistaArchivosVisibleEnPantalla && hwndDefView == vistaArchivosVisibleEnPantalla);
            if (esLaVistaActiva) {
                ventanaVistaArchivosActiva = hwndDefView;
                if (rutaCarpetaActualContenedora) {
                    *rutaCarpetaActualContenedora = rutaDirectorio;
                }
                if (!rutasSeleccionadas.empty()) {
                    std::sort(rutasSeleccionadas.begin(), rutasSeleccionadas.end(),
                              [](const std::wstring& primeraRuta, const std::wstring& segundaRuta) {
                                  return StrCmpLogicalW(primeraRuta.c_str(), segundaRuta.c_str()) < 0;
                              });
                    return rutasSeleccionadas;
                }
                return {};
            }

            RECT rectanguloDefView{};
            GetWindowRect(hwndDefView, &rectanguloDefView);
            bool esVisible = (IsWindowVisible(hwndDefView) && rectanguloDefView.left > -1000);
            candidatasDeEstaAnfitriona.push_back({rutasSeleccionadas, rutaDirectorio, hwndDefView, esVisible});
        }
    }

    for (const auto& candidataVista : candidatasDeEstaAnfitriona) {
        if (candidataVista.esVisibleEnPantalla) {
            ventanaVistaArchivosActiva = candidataVista.hwndDefView;
            if (rutaCarpetaActualContenedora) {
                *rutaCarpetaActualContenedora = candidataVista.rutaDirectorio;
            }
            if (!candidataVista.rutas.empty()) {
                std::vector<std::wstring> resultadoRutas = candidataVista.rutas;
                std::sort(resultadoRutas.begin(), resultadoRutas.end(),
                          [](const std::wstring& primeraRuta, const std::wstring& segundaRuta) {
                              return StrCmpLogicalW(primeraRuta.c_str(), segundaRuta.c_str()) < 0;
                          });
                return resultadoRutas;
            }
            return {};
        }
    }

    if (!candidatasDeEstaAnfitriona.empty()) {
        const auto& primeraCandidata = candidatasDeEstaAnfitriona.front();
        ventanaVistaArchivosActiva = primeraCandidata.hwndDefView;
        if (rutaCarpetaActualContenedora) {
            *rutaCarpetaActualContenedora = primeraCandidata.rutaDirectorio;
        }
        if (!primeraCandidata.rutas.empty()) {
            std::vector<std::wstring> resultadoRutas = primeraCandidata.rutas;
            std::sort(resultadoRutas.begin(), resultadoRutas.end(),
                      [](const std::wstring& primeraRuta, const std::wstring& segundaRuta) {
                          return StrCmpLogicalW(primeraRuta.c_str(), segundaRuta.c_str()) < 0;
                      });
            return resultadoRutas;
        }
        return {};
    }

    return {};
}
