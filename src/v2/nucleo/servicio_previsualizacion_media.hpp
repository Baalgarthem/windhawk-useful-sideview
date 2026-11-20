#pragma once

#include <windows.h>
#include <wincodec.h>
#include <shobjidl.h>
#include <string>
#include "v2/nucleo/servicio_com.hpp"

namespace SideViewV2 {
class ServicioPrevisualizacionMediaV2 {
public:
    HBITMAP CargarImagen(const std::wstring& ruta) const {
        AmbitoComV2 com;
        if (!com.Disponible()) return nullptr;
        IWICImagingFactory* fabrica = nullptr; IWICBitmapDecoder* decoder = nullptr;
        IWICBitmapFrameDecode* frame = nullptr; IWICFormatConverter* converter = nullptr;
        HBITMAP resultado = nullptr;
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fabrica))) ||
            FAILED(fabrica->CreateDecoderFromFilename(ruta.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder)) ||
            FAILED(decoder->GetFrame(0, &frame)) || FAILED(fabrica->CreateFormatConverter(&converter)) ||
            FAILED(converter->Initialize(frame, GUID_WICPixelFormat24bppBGR, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
            if (converter) converter->Release(); if (frame) frame->Release(); if (decoder) decoder->Release(); if (fabrica) fabrica->Release(); return nullptr;
        }
        UINT ancho=0, alto=0;
        if (FAILED(converter->GetSize(&ancho,&alto)) || ancho == 0 || alto == 0 || ancho > 8192 || alto > 8192 || static_cast<ULONGLONG>(ancho) * alto > 16ULL * 1024 * 1024) { converter->Release(); frame->Release(); decoder->Release(); fabrica->Release(); return nullptr; }
        BITMAPINFO info{}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER); info.bmiHeader.biWidth=static_cast<LONG>(ancho); info.bmiHeader.biHeight=-static_cast<LONG>(alto); info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=24; info.bmiHeader.biCompression=BI_RGB;
        void* bits=nullptr; HDC dc=GetDC(nullptr);
        if (!dc) { converter->Release(); frame->Release(); decoder->Release(); fabrica->Release(); return nullptr; }
        resultado=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0); ReleaseDC(nullptr,dc);
        UINT stride=((ancho*3+3)/4)*4; if(resultado && bits && FAILED(converter->CopyPixels(nullptr,stride,stride*alto,static_cast<BYTE*>(bits)))) { DeleteObject(resultado); resultado=nullptr; }
        converter->Release(); frame->Release(); decoder->Release(); fabrica->Release(); return resultado;
    }

    HBITMAP CargarMiniatura(const std::wstring& ruta, SIZE size = SIZE{900, 600}) const {
        AmbitoComV2 com;
        if (!com.Disponible()) return nullptr;
        IShellItem* item=nullptr; IShellItemImageFactory* factory=nullptr; HBITMAP bitmap=nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(ruta.c_str(), nullptr, IID_PPV_ARGS(&item))) &&
            item && SUCCEEDED(item->QueryInterface(IID_PPV_ARGS(&factory))) && factory) {
            if (FAILED(factory->GetImage(size, SIIGBF_RESIZETOFIT | SIIGBF_THUMBNAILONLY, &bitmap)) && bitmap) {
                DeleteObject(bitmap);
                bitmap = nullptr;
            }
        }
        if(factory) factory->Release(); if(item) item->Release(); return bitmap;
    }
};
}
