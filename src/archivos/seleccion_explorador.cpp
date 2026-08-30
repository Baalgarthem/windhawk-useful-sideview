bool esRutaDeVideo(const wchar_t* rutaArchivo) {
    if (!rutaArchivo || !*rutaArchivo || PathIsDirectoryW(rutaArchivo)) {
        return false;
    }

    const wchar_t* extensionArchivo = PathFindExtensionW(rutaArchivo);
    static constexpr const wchar_t* EXTENSIONES_VIDEO[] = {
        L".mp4", L".m4v", L".mov", L".wmv", L".avi", L".mkv",
        L".webm", L".mpg", L".mpeg", L".mts", L".m2ts", L".3gp"};
    for (const wchar_t* extensionVideo : EXTENSIONES_VIDEO) {
        if (_wcsicmp(extensionArchivo, extensionVideo) == 0) return true;
    }
    return false;
}

std::wstring obtenerRutaVideoDeSeleccion(IWebBrowserApp* explorador) {
    PunteroCom<IServiceProvider> proveedorServicios;
    if (FAILED(explorador->QueryInterface(
            IID_PPV_ARGS(proveedorServicios.recibir())))) {
        return {};
    }

    PunteroCom<IShellBrowser> navegadorCarpetas;
    if (FAILED(proveedorServicios->QueryService(
            SID_STopLevelBrowser,
            IID_PPV_ARGS(navegadorCarpetas.recibir())))) {
        return {};
    }

    PunteroCom<IShellView> vistaActiva;
    if (FAILED(navegadorCarpetas->QueryActiveShellView(
            vistaActiva.recibir()))) {
        return {};
    }

    PunteroCom<IFolderView2> vistaCarpeta;
    if (FAILED(vistaActiva->QueryInterface(
            IID_PPV_ARGS(vistaCarpeta.recibir())))) {
        return {};
    }

    PunteroCom<IShellItemArray> elementosSeleccionados;
    if (FAILED(vistaCarpeta->GetSelection(
            TRUE, elementosSeleccionados.recibir()))) {
        return {};
    }

    DWORD cantidadElementosSeleccionados = 0;
    if (FAILED(elementosSeleccionados->GetCount(
            &cantidadElementosSeleccionados)) ||
        cantidadElementosSeleccionados != 1) {
        return {};
    }

    PunteroCom<IShellItem> elementoSeleccionado;
    if (FAILED(elementosSeleccionados->GetItemAt(
            0, elementoSeleccionado.recibir()))) {
        return {};
    }

    PWSTR rutaElementoSeleccionado = nullptr;
    if (FAILED(elementoSeleccionado->GetDisplayName(
            SIGDN_FILESYSPATH, &rutaElementoSeleccionado))) {
        return {};
    }

    std::wstring rutaVideo;
    if (esRutaDeVideo(rutaElementoSeleccionado)) {
        rutaVideo = rutaElementoSeleccionado;
    }
    CoTaskMemFree(rutaElementoSeleccionado);
    return rutaVideo;
}

std::wstring obtenerVideoSeleccionado(IShellWindows* ventanasShell) {
    if (!ventanasShell) return {};

    long cantidadVentanasShell = 0;
    if (FAILED(ventanasShell->get_Count(&cantidadVentanasShell))) return {};

    // IShellWindows solo expone Item mediante una posición numérica. El índice
    // queda confinado a este recorrido y nombra explícitamente qué representa.
    for (long posicionVentanaShell = 0;
         posicionVentanaShell < cantidadVentanasShell;
         ++posicionVentanaShell) {
        VARIANT identificadorVentanaShell;
        VariantInit(&identificadorVentanaShell);
        identificadorVentanaShell.vt = VT_I4;
        identificadorVentanaShell.lVal = posicionVentanaShell;

        PunteroCom<IDispatch> despachoVentana;
        if (FAILED(ventanasShell->Item(
                identificadorVentanaShell, despachoVentana.recibir()))) {
            continue;
        }

        PunteroCom<IWebBrowserApp> explorador;
        if (FAILED(despachoVentana->QueryInterface(
                IID_PPV_ARGS(explorador.recibir())))) {
            continue;
        }

        SHANDLE_PTR identificadorVentana = 0;
        if (FAILED(explorador->get_HWND(&identificadorVentana))) continue;

        HWND ventanaExplorador =
            reinterpret_cast<HWND>(identificadorVentana);
        DWORD identificadorProceso = 0;
        GetWindowThreadProcessId(ventanaExplorador, &identificadorProceso);

        // Cada instancia inyectada atiende únicamente sus propias ventanas,
        // aunque ShellWindows enumere ventanas de otros procesos Explorer.
        if (identificadorProceso != GetCurrentProcessId() ||
            !IsWindowVisible(ventanaExplorador)) {
            continue;
        }

        std::wstring rutaVideo =
            obtenerRutaVideoDeSeleccion(explorador.obtener());
        if (!rutaVideo.empty()) {
            ventanaExploradorAnfitriona = ventanaExplorador;
            return rutaVideo;
        }
    }
    return {};
}
