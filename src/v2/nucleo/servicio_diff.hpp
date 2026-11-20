#pragma once

#include "v2/nucleo/servicio_comparador_archivos.hpp"
#include "v2/nucleo/servicio_texto.hpp"
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

// Contrato general de diff. Cada tipo de archivo puede aportar otra estrategia
// sin modificar la vista ni el comparador padre.
class ServicioDiffViewV2 : public ServicioComparadorArchivosV2 {
public:
    ~ServicioDiffViewV2() override = default;
    virtual EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const = 0;
};

class ServicioComparadorTextoV2 final : public ServicioDiffViewV2 {
public:
    explicit ServicioComparadorTextoV2(std::shared_ptr<ServicioTextoV2> texto)
        : m_texto(std::move(texto)) {}

    EstadoDiffV2 CrearDiff(const std::vector<std::wstring>& seleccion) const override {
        EstadoDiffV2 resultado;
        if (seleccion.size() != 2 || !m_texto) {
            resultado.mensaje = L"Selecciona exactamente dos archivos de texto.";
            return resultado;
        }
        DocumentoTextoV2 izquierda = m_texto->Cargar(seleccion[0]);
        DocumentoTextoV2 derecha = m_texto->Cargar(seleccion[1]);
        resultado = CompararLineas(izquierda.lineas, derecha.lineas);
        resultado.mensaje = L"Diff de texto listo.";
        return resultado;
    }

private:
    static EstadoDiffV2 CompararLineas(const std::vector<std::wstring>& izquierda,
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

    std::shared_ptr<ServicioTextoV2> m_texto;
};

class ServicioComparadorDocumentosV2 final : public ServicioComparadorArchivosV2 {
public:
    EstadoComparadorArchivosV2 CrearComparacion(
        const std::vector<std::wstring>& seleccion) const override {
        EstadoComparadorArchivosV2 estado =
            ServicioComparadorArchivosV2::CrearComparacion(seleccion);
        estado.mensaje = L"Comparación documental preparada; el contenido se mostrará por tipo de documento.";
        return estado;
    }
};

} // namespace SideViewV2
