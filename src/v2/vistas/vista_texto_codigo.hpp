#pragma once
#include "v2/ui/panel_interfaz.hpp"
#include "v2/nucleo/servicio_texto.hpp"
#include "v2/nucleo/servicio_navegacion.hpp"
#include <windowsx.h>
#include <algorithm>
#include <memory>

namespace SideViewV2 {
class VistaTextoCodigo : public IPanelContenido {
public:
    VistaTextoCodigo(std::shared_ptr<ServicioTextoV2> texto, std::shared_ptr<ServicioNavegacionV2> nav, bool codigo)
        : m_texto(std::move(texto)), m_nav(std::move(nav)), m_codigo(codigo) {}
    void AlCambiarSeleccion(const std::vector<std::wstring>& archivos, const std::wstring&) override {
        m_archivos = archivos; m_indiceArchivo = 0; Cargar();
    }
    bool ProcesarMensajeRaton(UINT msg, WPARAM, LPARAM lParam, HWND hwnd) override {
        if (msg != WM_LBUTTONDOWN) return false; POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (PtInRect(&m_rcAnterior,p)) { Cambiar(-1,hwnd); return true; }
        if (PtInRect(&m_rcSiguiente,p)) { Cambiar(1,hwnd); return true; }
        if (PtInRect(&m_rcCopiar,p)) { m_texto->Copiar(Unir()); return true; }
        if (PtInRect(&m_rcRenombrar,p) && !m_documento.ruta.empty()) { if(m_texto->Renombrar(hwnd,m_documento.ruta)) Cargar(); return true; }
        for (size_t i=0;i<m_documento.indice.size();++i) { RECT r{m_rcIndice.left,m_rcIndice.top+static_cast<int>(i*22),m_rcIndice.right,m_rcIndice.top+static_cast<int>((i+1)*22)}; if(PtInRect(&r,p)){m_lineaInicial=m_documento.indice[i];InvalidateRect(hwnd,nullptr,FALSE);return true;} }
        return PtInRect(&m_rcIndice,p);
    }
    bool ProcesarMensajeTeclado(UINT msg, WPARAM wParam, LPARAM, HWND hwnd) override {
        if(msg != WM_KEYDOWN) return false; if(wParam==VK_LEFT){Cambiar(-1,hwnd);return true;} if(wParam==VK_RIGHT){Cambiar(1,hwnd);return true;} if(wParam==VK_HOME){m_lineaInicial=0;InvalidateRect(hwnd,nullptr,FALSE);return true;} return false;
    }
    void Renderizar(HDC hdc, const RECT& rect, const TemaVisual& tema) override {
        HBRUSH b=CreateSolidBrush(tema.fondo); FillRect(hdc,&rect,b); DeleteObject(b); SetBkMode(hdc,TRANSPARENT);
        int top=8; m_rcAnterior={rect.left+8,rect.top+top,rect.left+72,rect.top+top+28}; m_rcSiguiente={rect.left+78,rect.top+top,rect.left+142,rect.top+top+28}; m_rcCopiar={rect.right-170,rect.top+top,rect.right-88,rect.top+top+28}; m_rcRenombrar={rect.right-82,rect.top+top,rect.right-8,rect.top+top+28};
        DibujarBoton(hdc,m_rcAnterior,L"◀",tema); DibujarBoton(hdc,m_rcSiguiente,L"▶",tema); DibujarBoton(hdc,m_rcCopiar,L"Copiar",tema); DibujarBoton(hdc,m_rcRenombrar,L"Renombrar",tema);
        SetTextColor(hdc,tema.textoPrincipal); RECT titulo{rect.left+154,rect.top+top,rect.right-180,rect.top+top+28}; std::wstring nombre=m_documento.ruta; size_t p=nombre.find_last_of(L"\\/"); if(p!=std::wstring::npos) nombre=nombre.substr(p+1); DrawTextW(hdc,nombre.c_str(),-1,&titulo,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
        m_rcIndice={rect.left+8,rect.top+48,rect.left+190,rect.bottom-8}; HBRUSH bi=CreateSolidBrush(tema.fondoCabecera); FillRect(hdc,&m_rcIndice,bi); DeleteObject(bi); SetTextColor(hdc,tema.textoSecundario); size_t filasIndice = m_rcIndice.bottom > m_rcIndice.top ? static_cast<size_t>((m_rcIndice.bottom-m_rcIndice.top)/22) : 0; size_t entradasIndice = std::min(filasIndice,m_documento.indice.size()); for(size_t i=0;i<entradasIndice;++i){size_t n=m_documento.indice[i];RECT r{m_rcIndice.left+8,m_rcIndice.top+static_cast<int>(i*22),m_rcIndice.right-4,m_rcIndice.top+static_cast<int>((i+1)*22)}; DrawTextW(hdc,m_documento.lineas[n].c_str(),-1,&r,DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);}
        bool mostrarNumeros=g_mostrarNumerosLineaV2.load(); int x=rect.left+202,y=rect.top+54; SetTextColor(hdc,tema.textoPrincipal); for(size_t i=m_lineaInicial;i<m_documento.lineas.size() && y<rect.bottom-8;++i,y+=20){if(mostrarNumeros){SetTextColor(hdc,tema.textoSecundario); wchar_t no[32]; swprintf_s(no,L"%5zu",i+1); TextOutW(hdc,x,y,no,static_cast<int>(wcslen(no)));} const std::wstring& linea=m_documento.lineas[i]; bool encabezado=!linea.empty() && linea[0]==L'#'; bool lista=linea.size()>1 && (linea[0]==L'-'||linea[0]==L'*'||linea[0]==L'>'); COLORREF color=m_codigo&&EsClave(linea)?RGB(180,100,220):(encabezado?RGB(70,150,230):(lista?RGB(90,180,120):tema.textoPrincipal)); SetTextColor(hdc,color); RECT r{x+ (mostrarNumeros?52:4),y,rect.right-8,y+20}; DrawTextW(hdc,linea.c_str(),-1,&r,DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);}
    }
private:
    void Cargar(){m_documento=m_archivos.empty()?DocumentoTextoV2{}:m_texto->Cargar(m_archivos[m_indiceArchivo]);m_lineaInicial=0;}
    void Cambiar(int delta,HWND hwnd){if(m_archivos.size()<2)return;m_indiceArchivo=delta<0?m_nav->Anterior(m_indiceArchivo,m_archivos.size()):m_nav->Siguiente(m_indiceArchivo,m_archivos.size());Cargar();InvalidateRect(hwnd,nullptr,FALSE);}
    std::wstring Unir()const{size_t capacidad=0;for(const auto& l:m_documento.lineas) capacidad+=l.size()+2;std::wstring r;r.reserve(capacidad);for(const auto& l:m_documento.lineas){r+=l;r+=L"\r\n";}return r;}
    static bool EsClave(const std::wstring& l){static const wchar_t* k[]={L"class ",L"struct ",L"void ",L"int ",L"return ",L"#include",L"function ",L"const ",L"public:",L"private:"};for(auto s:k)if(l.find(s)!=std::wstring::npos)return true;return false;}
    void DibujarBoton(HDC hdc,const RECT& r,const wchar_t* t,const TemaVisual& tema){HBRUSH b=CreateSolidBrush(tema.botonFondo);FillRect(hdc,&r,b);DeleteObject(b);SetTextColor(hdc,tema.textoPrincipal);DrawTextW(hdc,t,-1,const_cast<RECT*>(&r),DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);}
    std::shared_ptr<ServicioTextoV2> m_texto; std::shared_ptr<ServicioNavegacionV2> m_nav; bool m_codigo{}; std::vector<std::wstring> m_archivos; size_t m_indiceArchivo{}; size_t m_lineaInicial{}; DocumentoTextoV2 m_documento; RECT m_rcAnterior{},m_rcSiguiente{},m_rcCopiar{},m_rcRenombrar{},m_rcIndice{};
};
}
