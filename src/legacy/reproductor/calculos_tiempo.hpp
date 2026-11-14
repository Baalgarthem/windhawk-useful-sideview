#pragma once

inline LONGLONG calcularPosicionTemporal(LONGLONG duracionTotal,
                                         double proporcionObjetivo) {
    if (duracionTotal <= 0) return 0;
    return static_cast<LONGLONG>(
        static_cast<double>(duracionTotal) *
        limitarProporcionProgreso(proporcionObjetivo));
}
