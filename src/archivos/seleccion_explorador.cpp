bool esRutaDeVideo(const wchar_t* ruta) {
    if (!ruta || !*ruta || PathIsDirectoryW(ruta)) return false;
    const wchar_t* extension = PathFindExtensionW(ruta);
    static constexpr const wchar_t* EXTENSIONES_VIDEO[] = {
        L".mp4", L".m4v", L".mov", L".wmv", L".avi", L".mkv",
        L".webm", L".mpg", L".mpeg", L".mts", L".m2ts", L".3gp"};
    for (const wchar_t* extensionVideo : EXTENSIONES_VIDEO) {
        if (_wcsicmp(extension, extensionVideo) == 0) return true;
    }
    return false;
}

std::wstring obtenerVideoSeleccionado() {
    IShellWindows* ventanasShell = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&ventanasShell)))) {
        return {};
    }

    std::wstring rutaSeleccionada;
    long cantidad = 0;
    ventanasShell->get_Count(&cantidad);
    for (long indice = 0; indice < cantidad && rutaSeleccionada.empty(); ++indice) {
        VARIANT posicion;
        VariantInit(&posicion);
        posicion.vt = VT_I4;
        posicion.lVal = indice;
        IDispatch* despacho = nullptr;
        if (FAILED(ventanasShell->Item(posicion, &despacho)) || !despacho) continue;

        IWebBrowserApp* navegador = nullptr;
        if (SUCCEEDED(despacho->QueryInterface(IID_PPV_ARGS(&navegador)))) {
            SHANDLE_PTR identificadorVentana = 0;
            navegador->get_HWND(&identificadorVentana);
            HWND candidata = reinterpret_cast<HWND>(identificadorVentana);
            DWORD idProceso = 0;
            GetWindowThreadProcessId(candidata, &idProceso);

            // Cada instancia inyectada atiende únicamente sus propias ventanas
            // de Explorer, incluso cuando ShellWindows enumera todas globalmente.
            if (idProceso == GetCurrentProcessId() && IsWindowVisible(candidata)) {
                IServiceProvider* proveedor = nullptr;
                if (SUCCEEDED(navegador->QueryInterface(IID_PPV_ARGS(&proveedor)))) {
                    IShellBrowser* navegadorShell = nullptr;
                    if (SUCCEEDED(proveedor->QueryService(
                            SID_STopLevelBrowser, IID_PPV_ARGS(&navegadorShell)))) {
                        IShellView* vistaShell = nullptr;
                        if (SUCCEEDED(navegadorShell->QueryActiveShellView(&vistaShell))) {
                            IFolderView2* vistaCarpeta = nullptr;
                            if (SUCCEEDED(vistaShell->QueryInterface(
                                    IID_PPV_ARGS(&vistaCarpeta)))) {
                                IShellItemArray* seleccion = nullptr;
                                if (SUCCEEDED(vistaCarpeta->GetSelection(
                                        TRUE, &seleccion)) && seleccion) {
                                    DWORD elementos = 0;
                                    seleccion->GetCount(&elementos);
                                    if (elementos == 1) {
                                        IShellItem* elemento = nullptr;
                                        if (SUCCEEDED(seleccion->GetItemAt(0, &elemento))) {
                                            PWSTR ruta = nullptr;
                                            if (SUCCEEDED(elemento->GetDisplayName(
                                                    SIGDN_FILESYSPATH, &ruta)) &&
                                                esRutaDeVideo(ruta)) {
                                                rutaSeleccionada = ruta;
                                                ventanaHost = candidata;
                                            }
                                            CoTaskMemFree(ruta);
                                            elemento->Release();
                                        }
                                    }
                                    seleccion->Release();
                                }
                                vistaCarpeta->Release();
                            }
                            vistaShell->Release();
                        }
                        navegadorShell->Release();
                    }
                    proveedor->Release();
                }
            }
            navegador->Release();
        }
        despacho->Release();
    }
    ventanasShell->Release();
    return rutaSeleccionada;
}
