#pragma once

#include <propidl.h>
#include <limits>

inline bool extraerTiempoNoNegativoMediaFoundation(
    const PROPVARIANT& propiedadTiempo, LONGLONG* tiempoExtraido) {
    if (!tiempoExtraido) return false;

    ULONGLONG tiempoSinSigno = 0;
    if (propiedadTiempo.vt == VT_UI8) {
        tiempoSinSigno = propiedadTiempo.uhVal.QuadPart;
    } else if (propiedadTiempo.vt == VT_I8 &&
               propiedadTiempo.hVal.QuadPart >= 0) {
        tiempoSinSigno =
            static_cast<ULONGLONG>(propiedadTiempo.hVal.QuadPart);
    } else {
        return false;
    }

    constexpr ULONGLONG MAYOR_TIEMPO_REPRESENTABLE =
        static_cast<ULONGLONG>(std::numeric_limits<LONGLONG>::max());
    if (tiempoSinSigno > MAYOR_TIEMPO_REPRESENTABLE) return false;

    *tiempoExtraido = static_cast<LONGLONG>(tiempoSinSigno);
    return true;
}

inline bool extraerDuracionMediaFoundation(
    const PROPVARIANT& propiedadDuracion, LONGLONG* duracionExtraida) {
    return extraerTiempoNoNegativoMediaFoundation(propiedadDuracion,
                                                   duracionExtraida);
}

inline bool extraerPosicionMediaFoundation(
    const PROPVARIANT& propiedadPosicion, LONGLONG* posicionExtraida) {
    return extraerTiempoNoNegativoMediaFoundation(propiedadPosicion,
                                                   posicionExtraida);
}
