#pragma once

#include <string>
#include <vector>
#include <atomic>

enum class TipoContenidoArchivo;

struct Configuracion {
    // Windhawk notifica cambios desde otro hilo. Los atómicos evitan bloquear
    // el camino crítico del reproductor y eliminan carreras de datos.
    std::atomic_bool reproduccionAutomatica{false};
    std::atomic_bool sonidoHabilitado{false};
    std::atomic_bool bloquearTamanoVideo{false};
    std::atomic_bool bloquearTamanoImagen{false};
    std::atomic_bool bloquearTamanoComprimidos{false};
    std::atomic_bool bloquearTamanoTexto{false};
    std::atomic_bool bloquearTamanoDocumentos{false};
    std::atomic_bool bloquearTamanoCodigo{false};
    std::atomic_bool bloquearTamanoVistaCarpetas{false};
    std::atomic_bool bloquearTamanoVistaVacia{false};
    std::atomic_bool bloquearTamanoDesconocido{false};
    std::atomic<int> limiteCuadriculaComprimidos{4};
    std::wstring programaComicPorDefecto;
    std::vector<std::wstring> programasComicsRecordados;
    std::wstring programaComprimidosPorDefecto;
    std::vector<std::wstring> programasComprimidosRecordados;
    std::wstring programaCarpetasPorDefecto;
    std::vector<std::wstring> programasCarpetasRecordados;
    std::atomic_bool interaccionArchivosVistaCarpetas{false};
    std::atomic_bool arbolCarpetasExpandidoPorDefecto{false};
    std::atomic<int> umbralPorcentajeBajoMedioVistaVacia{40};
    std::atomic<int> umbralPorcentajeMedioAltoVistaVacia{70};
    std::atomic<COLORREF> colorRangoBajoVistaVacia{RGB(63, 185, 80)};
    std::atomic<COLORREF> colorRangoMedioVistaVacia{RGB(210, 153, 34)};
    std::atomic<COLORREF> colorRangoAltoVistaVacia{RGB(248, 81, 73)};
};

