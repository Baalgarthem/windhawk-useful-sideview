namespace {

inline COLORREF parsearColorConfigurado(PCWSTR textoEntrada, COLORREF colorPorDefecto) {
    if (!textoEntrada || !*textoEntrada) return colorPorDefecto;
    std::wstring textoLimpio(textoEntrada);
    size_t primerNoEspacio = textoLimpio.find_first_not_of(L" \t\r\n");
    if (primerNoEspacio == std::wstring::npos) return colorPorDefecto;
    size_t ultimoNoEspacio = textoLimpio.find_last_not_of(L" \t\r\n");
    textoLimpio = textoLimpio.substr(primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);

    if (textoLimpio[0] == L'#') {
        std::wstring textoHexadecimal = textoLimpio.substr(1);
        if (textoHexadecimal.length() == 6) {
            wchar_t* caracterFinal = nullptr;
            unsigned long valorNumerico = wcstoul(textoHexadecimal.c_str(), &caracterFinal, 16);
            if (caracterFinal != textoHexadecimal.c_str()) {
                BYTE componenteRojo = static_cast<BYTE>((valorNumerico >> 16) & 0xFF);
                BYTE componenteVerde = static_cast<BYTE>((valorNumerico >> 8) & 0xFF);
                BYTE componenteAzul = static_cast<BYTE>(valorNumerico & 0xFF);
                return RGB(componenteRojo, componenteVerde, componenteAzul);
            }
        } else if (textoHexadecimal.length() == 3) {
            wchar_t textoRojo[] = { textoHexadecimal[0], textoHexadecimal[0], 0 };
            wchar_t textoVerde[] = { textoHexadecimal[1], textoHexadecimal[1], 0 };
            wchar_t textoAzul[] = { textoHexadecimal[2], textoHexadecimal[2], 0 };
            BYTE componenteRojo = static_cast<BYTE>(wcstoul(textoRojo, nullptr, 16));
            BYTE componenteVerde = static_cast<BYTE>(wcstoul(textoVerde, nullptr, 16));
            BYTE componenteAzul = static_cast<BYTE>(wcstoul(textoAzul, nullptr, 16));
            return RGB(componenteRojo, componenteVerde, componenteAzul);
        }
    }

    int rojoEscaneado = -1;
    int verdeEscaneado = -1;
    int azulEscaneado = -1;
    if (swscanf_s(textoLimpio.c_str(), L"RGB(%d,%d,%d)", &rojoEscaneado, &verdeEscaneado, &azulEscaneado) == 3 ||
        swscanf_s(textoLimpio.c_str(), L"rgb(%d,%d,%d)", &rojoEscaneado, &verdeEscaneado, &azulEscaneado) == 3 ||
        swscanf_s(textoLimpio.c_str(), L"%d,%d,%d", &rojoEscaneado, &verdeEscaneado, &azulEscaneado) == 3 ||
        swscanf_s(textoLimpio.c_str(), L"%d %d %d", &rojoEscaneado, &verdeEscaneado, &azulEscaneado) == 3) {
        if (rojoEscaneado >= 0 && rojoEscaneado <= 255 &&
            verdeEscaneado >= 0 && verdeEscaneado <= 255 &&
            azulEscaneado >= 0 && azulEscaneado <= 255) {
            return RGB(static_cast<BYTE>(rojoEscaneado),
                       static_cast<BYTE>(verdeEscaneado),
                       static_cast<BYTE>(azulEscaneado));
        }
    }

    return colorPorDefecto;
}

void cargarConfiguracion() {
    configuracion.reproduccionAutomatica.store(
        Wh_GetIntSetting(L"autoplay") != 0);
    configuracion.sonidoHabilitado.store(Wh_GetIntSetting(L"sound") != 0);
    configuracion.bloquearTamanoVideo.store(
        Wh_GetIntSetting(L"lock_preview_size_video") != 0);
    configuracion.bloquearTamanoImagen.store(
        Wh_GetIntSetting(L"lock_preview_size_image") != 0);
    configuracion.bloquearTamanoComprimidos.store(
        Wh_GetIntSetting(L"lock_preview_size_compressed") != 0);
    configuracion.bloquearTamanoTexto.store(
        Wh_GetIntSetting(L"lock_preview_size_text") != 0);
    configuracion.bloquearTamanoDocumentos.store(
        Wh_GetIntSetting(L"lock_preview_size_documents") != 0);
    configuracion.bloquearTamanoCodigo.store(
        Wh_GetIntSetting(L"lock_preview_size_code") != 0);
    configuracion.bloquearTamanoVistaCarpetas.store(
        Wh_GetIntSetting(L"lock_preview_size_folder") != 0);
    configuracion.bloquearTamanoVistaVacia.store(
        Wh_GetIntSetting(L"lock_preview_size_empty_view") != 0);
    configuracion.bloquearTamanoDesconocido.store(
        Wh_GetIntSetting(L"lock_preview_size_unknown") != 0);
    configuracion.interaccionArchivosVistaCarpetas.store(
        Wh_GetIntSetting(L"folder_tree_interactive_files") != 0);
    configuracion.arbolCarpetasExpandidoPorDefecto.store(
        Wh_GetIntSetting(L"folder_tree_expanded_by_default") != 0);

    int umbralBajo = Wh_GetIntSetting(L"empty_view_threshold_low_medium");
    if (umbralBajo <= 0 || umbralBajo > 95) umbralBajo = 40;

    int umbralAlto = Wh_GetIntSetting(L"empty_view_threshold_medium_high");
    if (umbralAlto <= 0 || umbralAlto > 100) umbralAlto = 70;

    if (umbralBajo >= umbralAlto) {
        umbralAlto = std::min<int>(100, umbralBajo + 1);
    }

    configuracion.umbralPorcentajeBajoMedioVistaVacia.store(umbralBajo);
    configuracion.umbralPorcentajeMedioAltoVistaVacia.store(umbralAlto);

    PCWSTR textoColorBajo = Wh_GetStringSetting(L"empty_view_color_low");
    COLORREF colorRangoBajo = parsearColorConfigurado(textoColorBajo, RGB(63, 185, 80));
    if (textoColorBajo) Wh_FreeStringSetting(textoColorBajo);

    PCWSTR textoColorMedio = Wh_GetStringSetting(L"empty_view_color_medium");
    COLORREF colorRangoMedio = parsearColorConfigurado(textoColorMedio, RGB(210, 153, 34));
    if (textoColorMedio) Wh_FreeStringSetting(textoColorMedio);

    PCWSTR textoColorAlto = Wh_GetStringSetting(L"empty_view_color_high");
    COLORREF colorRangoAlto = parsearColorConfigurado(textoColorAlto, RGB(248, 81, 73));
    if (textoColorAlto) Wh_FreeStringSetting(textoColorAlto);

    if (colorRangoBajo == colorRangoMedio) {
        colorRangoMedio = RGB(210, 153, 34);
    }
    if (colorRangoMedio == colorRangoAlto || colorRangoBajo == colorRangoAlto) {
        colorRangoAlto = RGB(248, 81, 73);
    }

    configuracion.colorRangoBajoVistaVacia.store(colorRangoBajo);
    configuracion.colorRangoMedioVistaVacia.store(colorRangoMedio);
    configuracion.colorRangoAltoVistaVacia.store(colorRangoAlto);

    int limiteComprimidos = Wh_GetIntSetting(L"compressed_grid_limit");
    if (limiteComprimidos != 4 && limiteComprimidos != 6 &&
        limiteComprimidos != 8 && limiteComprimidos != 10) {
        limiteComprimidos = 4;
    }
    configuracion.limiteCuadriculaComprimidos.store(limiteComprimidos);

    PCWSTR programaComicPorDefecto = Wh_GetStringSetting(L"comic_default_program");
    if (programaComicPorDefecto) {
        configuracion.programaComicPorDefecto = programaComicPorDefecto;
        Wh_FreeStringSetting(programaComicPorDefecto);
    } else {
        configuracion.programaComicPorDefecto.clear();
    }

    configuracion.programasComicsRecordados.clear();
    if (!configuracion.programaComicPorDefecto.empty()) {
        configuracion.programasComicsRecordados.push_back(
            configuracion.programaComicPorDefecto);
    }

    PCWSTR programasComicsRecordadosRaw =
        Wh_GetStringSetting(L"comic_remembered_programs");
    if (programasComicsRecordadosRaw) {
        std::wstring textoLista(programasComicsRecordadosRaw);
        Wh_FreeStringSetting(programasComicsRecordadosRaw);

        size_t posicionInicio = 0;
        while (posicionInicio < textoLista.size()) {
            size_t posicionFin = textoLista.find(L';', posicionInicio);
            if (posicionFin == std::wstring::npos) posicionFin = textoLista.size();
            std::wstring rutaItem =
                textoLista.substr(posicionInicio, posicionFin - posicionInicio);
            size_t primerNoEspacio = rutaItem.find_first_not_of(L" \t\r\n");
            if (primerNoEspacio != std::wstring::npos) {
                size_t ultimoNoEspacio = rutaItem.find_last_not_of(L" \t\r\n");
                rutaItem = rutaItem.substr(
                    primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);
                if (!rutaItem.empty() &&
                    std::find(configuracion.programasComicsRecordados.begin(),
                              configuracion.programasComicsRecordados.end(),
                              rutaItem) ==
                        configuracion.programasComicsRecordados.end()) {
                    configuracion.programasComicsRecordados.push_back(rutaItem);
                }
            }
            posicionInicio = posicionFin + 1;
        }
    }

    PCWSTR programaComprimidosPorDefecto =
        Wh_GetStringSetting(L"compressed_default_program");
    if (programaComprimidosPorDefecto) {
        configuracion.programaComprimidosPorDefecto =
            programaComprimidosPorDefecto;
        Wh_FreeStringSetting(programaComprimidosPorDefecto);
    } else {
        configuracion.programaComprimidosPorDefecto.clear();
    }

    configuracion.programasComprimidosRecordados.clear();
    if (!configuracion.programaComprimidosPorDefecto.empty()) {
        configuracion.programasComprimidosRecordados.push_back(
            configuracion.programaComprimidosPorDefecto);
    }

    PCWSTR programasComprimidosRecordadosRaw =
        Wh_GetStringSetting(L"compressed_remembered_programs");
    if (programasComprimidosRecordadosRaw) {
        std::wstring textoLista(programasComprimidosRecordadosRaw);
        Wh_FreeStringSetting(programasComprimidosRecordadosRaw);

        size_t posicionInicio = 0;
        while (posicionInicio < textoLista.size()) {
            size_t posicionFin = textoLista.find(L';', posicionInicio);
            if (posicionFin == std::wstring::npos) posicionFin = textoLista.size();
            std::wstring rutaItem =
                textoLista.substr(posicionInicio, posicionFin - posicionInicio);
            size_t primerNoEspacio = rutaItem.find_first_not_of(L" \t\r\n");
            if (primerNoEspacio != std::wstring::npos) {
                size_t ultimoNoEspacio = rutaItem.find_last_not_of(L" \t\r\n");
                rutaItem = rutaItem.substr(
                    primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);
                if (!rutaItem.empty() &&
                    std::find(configuracion.programasComprimidosRecordados.begin(),
                              configuracion.programasComprimidosRecordados.end(),
                              rutaItem) ==
                        configuracion.programasComprimidosRecordados.end()) {
                    configuracion.programasComprimidosRecordados.push_back(
                        rutaItem);
                }
            }
            posicionInicio = posicionFin + 1;
        }
    }

    PCWSTR programaCarpetasPorDefecto =
        Wh_GetStringSetting(L"folder_default_program");
    if (programaCarpetasPorDefecto) {
        configuracion.programaCarpetasPorDefecto = programaCarpetasPorDefecto;
        Wh_FreeStringSetting(programaCarpetasPorDefecto);
    } else {
        configuracion.programaCarpetasPorDefecto.clear();
    }

    configuracion.programasCarpetasRecordados.clear();
    if (!configuracion.programaCarpetasPorDefecto.empty()) {
        configuracion.programasCarpetasRecordados.push_back(
            configuracion.programaCarpetasPorDefecto);
    }

    PCWSTR programasCarpetasRecordadosRaw =
        Wh_GetStringSetting(L"folder_remembered_programs");
    if (programasCarpetasRecordadosRaw) {
        std::wstring textoLista(programasCarpetasRecordadosRaw);
        Wh_FreeStringSetting(programasCarpetasRecordadosRaw);

        size_t posicionInicio = 0;
        while (posicionInicio < textoLista.size()) {
            size_t posicionFin = textoLista.find(L';', posicionInicio);
            if (posicionFin == std::wstring::npos) posicionFin = textoLista.size();
            std::wstring rutaItem =
                textoLista.substr(posicionInicio, posicionFin - posicionInicio);
            size_t primerNoEspacio = rutaItem.find_first_not_of(L" \t\r\n");
            if (primerNoEspacio != std::wstring::npos) {
                size_t ultimoNoEspacio = rutaItem.find_last_not_of(L" \t\r\n");
                rutaItem = rutaItem.substr(
                    primerNoEspacio, ultimoNoEspacio - primerNoEspacio + 1);
                if (!rutaItem.empty() &&
                    std::find(configuracion.programasCarpetasRecordados.begin(),
                              configuracion.programasCarpetasRecordados.end(),
                              rutaItem) ==
                        configuracion.programasCarpetasRecordados.end()) {
                    configuracion.programasCarpetasRecordados.push_back(
                        rutaItem);
                }
            }
            posicionInicio = posicionFin + 1;
        }
    }

    estadoComprimidos.programasComicsRecordados =
        configuracion.programasComicsRecordados;
    estadoComprimidos.programasComunesRecordados =
        configuracion.programasComprimidosRecordados;
    estadoComprimidos.indiceProgramaSeleccionado = 0;

    estadoVistaCarpetas.programasCarpetasRecordados =
        configuracion.programasCarpetasRecordados;
    estadoVistaCarpetas.indiceProgramaSeleccionado = 0;

    configuracionPendiente.store(true);
}

using TipoCreateProcessW = BOOL (WINAPI *)(
    LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
    BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
inline TipoCreateProcessW originalCreateProcessW = nullptr;

inline BOOL WINAPI GanchoCreateProcessW(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation) {
    if ((lpApplicationName && wcsstr(lpApplicationName, L"prevhost.exe") != nullptr) ||
        (lpCommandLine && wcsstr(lpCommandLine, L"prevhost.exe") != nullptr)) {
        SetLastError(ERROR_CANCELLED);
        return FALSE;
    }
    if (originalCreateProcessW) {
        return originalCreateProcessW(
            lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
            bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
            lpStartupInfo, lpProcessInformation);
    }
    return FALSE;
}

using TipoCoCreateInstance = HRESULT (WINAPI *)(
    REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);
inline TipoCoCreateInstance originalCoCreateInstance = nullptr;

inline HRESULT WINAPI GanchoCoCreateInstance(
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD dwClsContext,
    REFIID riid,
    LPVOID* ppv) {
    // IID_IPreviewHandler: {8895b1c6-b41f-4c1c-a562-0d564250836f}
    static const IID IID_IPreviewHandler_Local =
        {0x8895b1c6, 0xb41f, 0x4c1c, {0xa5, 0x62, 0x0d, 0x56, 0x42, 0x50, 0x83, 0x6f}};

    if (IsEqualIID(riid, IID_IPreviewHandler_Local)) {
        return E_NOINTERFACE;
    }

    if (originalCoCreateInstance) {
        return originalCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }
    return E_FAIL;
}

inline bool esProcesoExplorerValido() {
    wchar_t rutaModuloEjecutable[MAX_PATH];
    if (GetModuleFileNameW(nullptr, rutaModuloEjecutable, MAX_PATH) == 0) return true;
    const wchar_t* nombreEjecutable = PathFindFileNameW(rutaModuloEjecutable);
    if (!nombreEjecutable || !*nombreEjecutable) return true;
    return (_wcsicmp(nombreEjecutable, L"explorer.exe") == 0);
}

}  // namespace

BOOL Wh_ModInit() {
    if (!esProcesoExplorerValido()) {
        Wh_Log(L"Módulo omitido por ejecutarse en proceso distinto de explorer.exe");
        return FALSE;
    }

    Wh_SetFunctionHook((void*)CreateProcessW, (void*)GanchoCreateProcessW, (void**)&originalCreateProcessW);
    Wh_SetFunctionHook((void*)CoCreateInstance, (void*)GanchoCoCreateInstance, (void**)&originalCoCreateInstance);

    cargarConfiguracion();
    Gdiplus::GdiplusStartupInput entradaGdiplus;
    if (Gdiplus::GdiplusStartup(&tokenGdiplus, &entradaGdiplus, nullptr) !=
        Gdiplus::Ok) {
        Wh_Log(L"No fue posible inicializar GDI+");
        return FALSE;
    }
    if (!crearRecursosInterfaz()) {
        Wh_Log(L"No fue posible crear las fuentes de la interfaz");
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
        return FALSE;
    }
    if (!registrarClasesDeVentana()) {
        Wh_Log(L"No fue posible registrar las clases de ventana");
        liberarRecursosInterfaz();
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
        return FALSE;
    }
    detenerHilo.store(false);
    eventoDetener = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!eventoDetener) {
        Wh_Log(L"No fue posible crear el evento de detención (error %u)",
               static_cast<unsigned int>(GetLastError()));
        eliminarClasesDeVentana();
        liberarRecursosInterfaz();
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
        return FALSE;
    }
    hiloReproductor =
        CreateThread(nullptr, 0, ejecutarHiloReproductor, nullptr, 0, nullptr);
    if (!hiloReproductor) {
        Wh_Log(L"No fue posible crear el hilo reproductor (error %u)",
               static_cast<unsigned int>(GetLastError()));
        CloseHandle(eventoDetener);
        eventoDetener = nullptr;
        eliminarClasesDeVentana();
        liberarRecursosInterfaz();
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
        return FALSE;
    }
    return TRUE;
}

void Wh_ModSettingsChanged() {
    cargarConfiguracion();
    configuracionPendiente.store(true);
}

void Wh_ModUninit() {
    detenerHilo.store(true);
    if (eventoDetener) SetEvent(eventoDetener);
    if (hiloReproductor) {
        DWORD identificadorHiloReproductor = GetThreadId(hiloReproductor);
        if (identificadorHiloReproductor != 0) {
            PostThreadMessageW(identificadorHiloReproductor, WM_NULL, 0, 0);
        }
        if (ventanaDelegadoReproductor && IsWindow(ventanaDelegadoReproductor)) {
            PostMessageW(ventanaDelegadoReproductor, WM_NULL, 0, 0);
        }
        WaitForSingleObject(hiloReproductor, 3000);
        CloseHandle(hiloReproductor);
        hiloReproductor = nullptr;
    }
    if (eventoDetener) {
        CloseHandle(eventoDetener);
        eventoDetener = nullptr;
    }
    eliminarClasesDeVentana();
    liberarRecursosInterfaz();
    if (tokenGdiplus) {
        Gdiplus::GdiplusShutdown(tokenGdiplus);
        tokenGdiplus = 0;
    }
}
