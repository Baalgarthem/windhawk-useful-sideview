#pragma once

struct Configuracion {
    // Windhawk notifica cambios desde otro hilo. Los atómicos evitan bloquear
    // el camino crítico del reproductor y eliminan carreras de datos.
    std::atomic_bool reproduccionAutomatica{false};
    std::atomic_bool sonidoHabilitado{false};
};
