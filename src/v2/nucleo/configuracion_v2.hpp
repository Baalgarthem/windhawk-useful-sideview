#pragma once

#include <algorithm>
#include <atomic>

namespace SideViewV2 {

// Interruptor de seguridad para la funcionalidad experimental. Mantenerlo
// aislado permite desactivar el flujo completo sin acoplarlo a las vistas.
inline std::atomic_bool g_comparadorVideosExperimentalV2{true};
inline std::atomic_bool g_comparadorVideosPantallaCompletaV2{true};
inline std::atomic_bool g_mostrarVersionModV2{true};
inline std::atomic_bool g_modoSemanticoDiffViewV2{true};
inline std::atomic_bool g_sincronizacionNotasWinKlokV2{false};
inline std::atomic<int> g_tamanoFuenteTextoV2{10};
inline std::atomic<int> g_tamanoFuenteCodigoV2{10};
inline std::atomic_bool g_ajusteLineaTextoCodigoV2{true};
inline std::atomic_bool g_reproductorVideoEnVistaV2{false};

inline int NormalizarTamanoFuenteV2(int valor) {
    // Un valor 0 puede aparecer si se instala una versión que todavía no
    // tenía esta opción; conservar el valor inicial evita una fuente
    // accidentalmente diminuta en esa migración.
    return valor <= 0 ? 10 : std::clamp(valor, 8, 18);
}

inline bool ComparadorVideosExperimentalDisponibleV2() {
    return g_comparadorVideosExperimentalV2.load(std::memory_order_acquire);
}

inline bool ComparadorVideosPantallaCompletaDisponibleV2() {
    return g_comparadorVideosPantallaCompletaV2.load(std::memory_order_acquire);
}

inline bool MostrarVersionModDisponibleV2() {
    return g_mostrarVersionModV2.load(std::memory_order_acquire);
}

inline bool ModoSemanticoDiffViewDisponibleV2() {
    return g_modoSemanticoDiffViewV2.load(std::memory_order_acquire);
}

inline bool SincronizacionNotasWinKlokDisponibleV2() {
    return g_sincronizacionNotasWinKlokV2.load(std::memory_order_acquire);
}

inline int TamanoFuenteTextoV2() {
    return NormalizarTamanoFuenteV2(g_tamanoFuenteTextoV2.load(std::memory_order_acquire));
}

inline int TamanoFuenteCodigoV2() {
    return NormalizarTamanoFuenteV2(g_tamanoFuenteCodigoV2.load(std::memory_order_acquire));
}

inline bool AjusteLineaTextoCodigoDisponibleV2() {
    return g_ajusteLineaTextoCodigoV2.load(std::memory_order_acquire);
}

inline bool ReproductorVideoEnVistaDisponibleV2() {
    return g_reproductorVideoEnVistaV2.load(std::memory_order_acquire);
}

} // namespace SideViewV2
