#pragma once

#include <windows.h>
#include <shobjidl.h>
#include <exdisp.h>
#include <shlobj.h>
#include <shlguid.h>
#include <string>
#include <vector>
#include "v2/nucleo/servicio_com.hpp"

namespace SideViewV2 {

inline IShellView* ObtenerShellViewActivo(HWND top, HWND* outTab) {
    if (outTab) *outTab = nullptr;
    if (!top) return nullptr;

    IShellWindows* shellWindows = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_ALL,
                                  IID_IShellWindows, reinterpret_cast<void**>(&shellWindows));
    if (FAILED(hr) || !shellWindows) return nullptr;

    HWND tabActiva = FindWindowExW(top, nullptr, L"ShellTabWindowClass", nullptr);
    long cantidad = 0;
    shellWindows->get_Count(&cantidad);

    IShellView* resultado = nullptr;

    for (long i = 0; i < cantidad && !resultado; ++i) {
        VARIANT indice;
        VariantInit(&indice);
        indice.vt = VT_I4;
        indice.lVal = i;

        IDispatch* despacho = nullptr;
        if (FAILED(shellWindows->Item(indice, &despacho)) || !despacho) {
            VariantClear(&indice);
            continue;
        }
        VariantClear(&indice);

        IWebBrowserApp* app = nullptr;
        if (FAILED(despacho->QueryInterface(IID_IWebBrowserApp, reinterpret_cast<void**>(&app))) || !app) {
            despacho->Release();
            continue;
        }

        SHANDLE_PTR hwndItem = 0;
        if (SUCCEEDED(app->get_HWND(&hwndItem)) && hwndItem) {
            HWND hwndVentana = reinterpret_cast<HWND>(hwndItem);
            bool coincide = (hwndVentana == top);
            if (!coincide) {
                HWND ancestro = GetAncestor(hwndVentana, GA_ROOT);
                if (ancestro == top) coincide = true;
            }

            if (coincide) {
                IServiceProvider* sp = nullptr;
                if (SUCCEEDED(app->QueryInterface(IID_IServiceProvider, reinterpret_cast<void**>(&sp))) && sp) {
                    IShellBrowser* sb = nullptr;
                    if (SUCCEEDED(sp->QueryService(SID_STopLevelBrowser, IID_IShellBrowser,
                                                   reinterpret_cast<void**>(&sb))) && sb) {
                        IShellView* sv = nullptr;
                        if (SUCCEEDED(sb->QueryActiveShellView(&sv)) && sv) {
                            HWND hwndVista = nullptr;
                            sv->GetWindow(&hwndVista);
                            if (hwndVista) {
                                bool vistaCoincide = true;
                                if (tabActiva && IsWindow(tabActiva)) {
                                    vistaCoincide = IsChild(tabActiva, hwndVista);
                                }
                                if (vistaCoincide) {
                                    resultado = sv;
                                    if (outTab) {
                                        HWND p = hwndVista;
                                        while (p && p != top) {
                                            wchar_t clase[64]{};
                                            GetClassNameW(p, clase, ARRAYSIZE(clase));
                                            if (_wcsicmp(clase, L"ShellTabWindowClass") == 0) {
                                                *outTab = p;
                                                break;
                                            }
                                            p = GetParent(p);
                                        }
                                        if (!*outTab) *outTab = tabActiva ? tabActiva : top;
                                    }
                                }
                            }
                            if (!resultado) sv->Release();
                        }
                        sb->Release();
                    }
                    sp->Release();
                }
            }
        }
        app->Release();
        despacho->Release();
    }

    shellWindows->Release();
    return resultado;
}

inline std::vector<std::wstring> ObtenerRutasSeleccionadasDeVista(IShellView* vista, std::wstring* outCarpeta = nullptr) {
    if (outCarpeta) outCarpeta->clear();
    if (!vista) return {};
    AmbitoComV2 com;
    if (!com.Disponible()) return {};

    IFolderView2* fv = nullptr;
    if (FAILED(vista->QueryInterface(IID_IFolderView2, reinterpret_cast<void**>(&fv))) || !fv) {
        return {};
    }

    if (outCarpeta) {
        IShellItem* carpetaItem = nullptr;
        if (SUCCEEDED(fv->GetFolder(IID_IShellItem, reinterpret_cast<void**>(&carpetaItem))) && carpetaItem) {
            PWSTR rutaDirectorio = nullptr;
            if (SUCCEEDED(carpetaItem->GetDisplayName(SIGDN_FILESYSPATH, &rutaDirectorio)) && rutaDirectorio) {
                *outCarpeta = rutaDirectorio;
                CoTaskMemFree(rutaDirectorio);
            }
            carpetaItem->Release();
        }
    }

    std::vector<std::wstring> rutas;
    IShellItemArray* items = nullptr;
    if (SUCCEEDED(fv->GetSelection(TRUE, &items)) && items) {
        DWORD conteo = 0;
        items->GetCount(&conteo);
        for (DWORD i = 0; i < conteo; ++i) {
            IShellItem* item = nullptr;
            if (SUCCEEDED(items->GetItemAt(i, &item)) && item) {
                PWSTR ruta = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &ruta)) && ruta) {
                    rutas.push_back(ruta);
                    CoTaskMemFree(ruta);
                }
                item->Release();
            }
        }
        items->Release();
    }

    fv->Release();
    return rutas;
}

} // namespace SideViewV2
