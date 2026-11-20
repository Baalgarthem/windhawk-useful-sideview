#pragma once

#include <atomic>

namespace SideViewV2 {

// Interruptor de seguridad para la funcionalidad experimental. Mantenerlo
// aislado permite desactivar el flujo completo sin acoplarlo a las vistas.
inline std::atomic_bool g_comparadorVideosExperimentalV2{true};
inline std::atomic_bool g_comparadorVideosPantallaCompletaV2{true};
inline std::atomic_bool g_mostrarVersionModV2{true};

inline bool ComparadorVideosExperimentalDisponibleV2() {
    return g_comparadorVideosExperimentalV2.load(std::memory_order_acquire);
}

inline bool ComparadorVideosPantallaCompletaDisponibleV2() {
    return g_comparadorVideosPantallaCompletaV2.load(std::memory_order_acquire);
}

inline bool MostrarVersionModDisponibleV2() {
    return g_mostrarVersionModV2.load(std::memory_order_acquire);
}

} // namespace SideViewV2
