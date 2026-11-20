#pragma once

#include "v2/nucleo/servicio_comparador_archivos.hpp"
#include "v2/nucleo/servicio_texto.hpp"
#include <propkey.h>
#include <shobjidl.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace SideViewV2 {

enum class TipoLineaDiffV2 { Igual, Agregada, Eliminada, Modificada };

struct LineaDiffV2 {
    TipoLineaDiffV2 tipo{TipoLineaDiffV2::Igual};
    std::size_t lineaIzquierda{0};
    std::size_t lineaDerecha{0};
    std::wstring izquierda;
    std::wstring derecha;
};

struct EstadoDiffV2 {
    std::vector<LineaDiffV2> lineas;
    std::wstring mensaje;
};

// Contrato inyectable para cualquier diff. El algoritmo de alineación común
// vive aquí para que texto, documentos y futuras variantes compartan reglas.
class ServicioDiffViewV2 : public ServicioComparadorArchivosV2 {
public:
    ~ServicioDiffViewV2() override = default;
    virtual EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const = 0;

protected:
    static EstadoDiffV2 CompararLineasGenerico(
        const std::vector<std::wstring>& izquierda,
        const std::vector<std::wstring>& derecha) {
        EstadoDiffV2 resultado;
        std::size_t i = 0;
        std::size_t j = 0;
        constexpr std::size_t ventana = 24;
        while (i < izquierda.size() || j < derecha.size()) {
            if (i < izquierda.size() && j < derecha.size() && izquierda[i] == derecha[j]) {
                resultado.lineas.push_back({TipoLineaDiffV2::Igual, i + 1, j + 1,
                                            izquierda[i], derecha[j]});
                ++i;
                ++j;
                continue;
            }

            std::size_t siguienteDerecha = derecha.size();
            for (std::size_t k = j + 1; k < std::min(derecha.size(), j + ventana); ++k) {
                if (i < izquierda.size() && izquierda[i] == derecha[k]) {
                    siguienteDerecha = k;
                    break;
                }
            }
            std::size_t siguienteIzquierda = izquierda.size();
            for (std::size_t k = i + 1; k < std::min(izquierda.size(), i + ventana); ++k) {
                if (j < derecha.size() && izquierda[k] == derecha[j]) {
                    siguienteIzquierda = k;
                    break;
                }
            }

            if (siguienteDerecha != derecha.size() &&
                (siguienteIzquierda == izquierda.size() ||
                 siguienteDerecha - j <= siguienteIzquierda - i)) {
                resultado.lineas.push_back({TipoLineaDiffV2::Agregada, 0, j + 1,
                                            L"", derecha[j]});
                ++j;
            } else if (siguienteIzquierda != izquierda.size()) {
                resultado.lineas.push_back({TipoLineaDiffV2::Eliminada, i + 1, 0,
                                            izquierda[i], L""});
                ++i;
            } else {
                resultado.lineas.push_back({TipoLineaDiffV2::Modificada,
                                            i < izquierda.size() ? i + 1 : 0,
                                            j < derecha.size() ? j + 1 : 0,
                                            i < izquierda.size() ? izquierda[i] : L"",
                                            j < derecha.size() ? derecha[j] : L""});
                if (i < izquierda.size()) ++i;
                if (j < derecha.size()) ++j;
            }
        }
        return resultado;
    }

    static std::vector<std::wstring> DatosComoLineas(
        const DatosArchivoComparacionV2& datos) {
        std::vector<std::wstring> lineas;
        lineas.push_back(L"Nombre: " + datos.nombre);
        lineas.push_back(L"Formato: " + datos.extension);
        lineas.push_back(L"SHA-256: " + datos.sha256);
        lineas.push_back(L"Peso en bytes: " + std::to_wstring(datos.bytes));
        lineas.push_back(datos.paginas ? L"Páginas: " + std::to_wstring(datos.paginas)
                                       : L"Páginas: No disponible");
        lineas.push_back(L"Contenido: el adaptador documental compara metadatos hasta habilitar extracción segura.");
        return lineas;
    }
};

class ServicioComparadorTextoV2 final : public ServicioDiffViewV2 {
public:
    explicit ServicioComparadorTextoV2(std::shared_ptr<ServicioTextoV2> texto)
        : m_texto(std::move(texto)) {}

    EstadoComparadorArchivosV2 CrearComparacion(
        const std::vector<std::wstring>& seleccion) const override {
        EstadoComparadorArchivosV2 estado =
            ServicioComparadorArchivosV2::CrearComparacion(seleccion);
        if (seleccion.size() != 2 || !m_texto) return estado;
        const DocumentoTextoV2 izquierda = m_texto->Cargar(seleccion[0]);
        const DocumentoTextoV2 derecha = m_texto->Cargar(seleccion[1]);
        estado.izquierda.lineas = izquierda.lineas.size();
        estado.derecha.lineas = derecha.lineas.size();
        estado.mensaje = L"Comparación de texto lista.";
        return estado;
    }

    EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const override {
        EstadoDiffV2 resultado;
        if (seleccion.size() != 2 || !m_texto) {
            resultado.mensaje = L"Selecciona exactamente dos archivos de texto.";
            return resultado;
        }
        const DocumentoTextoV2 izquierda = m_texto->Cargar(seleccion[0]);
        const DocumentoTextoV2 derecha = m_texto->Cargar(seleccion[1]);
        resultado = CompararLineasGenerico(izquierda.lineas, derecha.lineas);
        resultado.mensaje = L"Diff de texto listo.";
        return resultado;
    }

private:
    std::shared_ptr<ServicioTextoV2> m_texto;
};

class ServicioComparadorDocumentosV2 final : public ServicioDiffViewV2 {
public:
    ServicioComparadorDocumentosV2() = default;

    EstadoComparadorArchivosV2 CrearComparacion(
        const std::vector<std::wstring>& seleccion) const override {
        EstadoComparadorArchivosV2 estado =
            ServicioComparadorArchivosV2::CrearComparacion(seleccion);
        if (seleccion.size() != 2) return estado;
        estado.izquierda.paginas = LeerPaginas(estado.izquierda.ruta);
        estado.derecha.paginas = LeerPaginas(estado.derecha.ruta);
        estado.mensaje = L"Comparación documental lista para Diff View.";
        return estado;
    }

    EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const override {
        EstadoDiffV2 resultado;
        if (seleccion.size() != 2) {
            resultado.mensaje = L"Selecciona exactamente dos documentos del mismo formato.";
            return resultado;
        }
        const EstadoComparadorArchivosV2 estado = CrearComparacion(seleccion);
        resultado = CompararLineasGenerico(
            DatosComoLineas(estado.izquierda), DatosComoLineas(estado.derecha));
        resultado.mensaje = L"Diff documental preparado con un resumen semántico.";
        return resultado;
    }

private:
    static std::uintmax_t LeerPaginas(const std::wstring& ruta) {
        IShellItem2* elemento = nullptr;
        if (FAILED(SHCreateItemFromParsingName(
                ruta.c_str(), nullptr, IID_IShellItem2,
                reinterpret_cast<void**>(&elemento))) || !elemento) {
            return 0;
        }
        ULONG paginas = 0;
        const HRESULT resultado = elemento->GetUInt32(PKEY_Document_PageCount, &paginas);
        elemento->Release();
        return SUCCEEDED(resultado) ? static_cast<std::uintmax_t>(paginas) : 0;
    }
};

} // namespace SideViewV2
