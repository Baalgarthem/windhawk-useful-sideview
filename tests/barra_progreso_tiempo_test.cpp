#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>
#include <atomic>
#include <cassert>

#include "nucleo/constantes.hpp"
#include "nucleo/configuracion.hpp"

Configuracion configuracion;

#include "interfaz/diseno_barra_progreso.hpp"
#include "reproductor/calculos_tiempo.hpp"
#include "reproductor/conversion_tiempo_media_foundation.hpp"

int main() {
    assert(!configuracion.reproduccionAutomatica.load());
    assert(!configuracion.sonidoHabilitado.load());

    assert(limitarProporcionProgreso(-0.5) == 0.0);
    assert(limitarProporcionProgreso(0.5) == 0.5);
    assert(limitarProporcionProgreso(1.5) == 1.0);

    DisenoBarraProgreso disenoBarraProgreso;
    disenoBarraProgreso.limiteIzquierdo = 10;
    disenoBarraProgreso.limiteDerecho = 790;
    assert(calcularProporcionProgreso(disenoBarraProgreso, -20) == 0.0);
    assert(calcularProporcionProgreso(disenoBarraProgreso, 790) == 1.0);
    double proporcionCentral =
        calcularProporcionProgreso(disenoBarraProgreso, 400);
    assert(proporcionCentral > 0.49 && proporcionCentral < 0.51);
    assert(calcularPosicionProgreso(disenoBarraProgreso, 0.5) == 400);
    assert(calcularInicioGrupoBotones(300) == 84);
    assert(calcularPosicionTemporal(1000000000LL, 0.25) == 250000000LL);
    assert(calcularPosicionTemporal(1000000000LL, -1.0) == 0);
    assert(calcularPosicionTemporal(1000000000LL, 2.0) == 1000000000LL);

    // MFPlay entrega normalmente la duración como VT_UI8. Esta regresión
    // reproduce el contrato que impedía que la búsqueda alcanzara SetPosition.
    PROPVARIANT propiedadDuracion{};
    propiedadDuracion.vt = VT_UI8;
    propiedadDuracion.uhVal.QuadPart = 1000000000ULL;
    LONGLONG duracionExtraida = 0;
    assert(extraerDuracionMediaFoundation(propiedadDuracion,
                                           &duracionExtraida));
    assert(duracionExtraida == 1000000000LL);

    PROPVARIANT propiedadPosicion{};
    propiedadPosicion.vt = VT_I8;
    propiedadPosicion.hVal.QuadPart = 250000000LL;
    LONGLONG posicionExtraida = 0;
    assert(extraerPosicionMediaFoundation(propiedadPosicion,
                                           &posicionExtraida));
    assert(posicionExtraida == 250000000LL);

    PROPVARIANT propiedadInvalida{};
    propiedadInvalida.vt = VT_LPWSTR;
    assert(!extraerDuracionMediaFoundation(propiedadInvalida,
                                            &duracionExtraida));

    PROPVARIANT propiedadDesbordada{};
    propiedadDesbordada.vt = VT_UI8;
    propiedadDesbordada.uhVal.QuadPart =
        std::numeric_limits<ULONGLONG>::max();
    assert(!extraerDuracionMediaFoundation(propiedadDesbordada,
                                            &duracionExtraida));

    RECT areaInteractiva{0, 10, 800, 34};
    assert(contienePunto(areaInteractiva, 400, 22));
    assert(!contienePunto(areaInteractiva, 400, 40));

    DisenoBarraProgreso disenoClic;
    disenoClic.barraControles = {0, 0, 800, 96};
    disenoClic.limiteIzquierdo = 16;
    disenoClic.limiteDerecho = 784;
    disenoClic.botonReproduccion = {332, 54, 368, 90};
    disenoClic.botonSonido = {380, 54, 416, 90};
    disenoClic.botonRepeticion = {428, 54, 464, 90};
    assert(esClicEnBarraProgreso(disenoClic, 400, 34));
    assert(!esClicEnBarraProgreso(disenoClic, 350, 70));
    assert(!esClicEnBarraProgreso(disenoClic, 10, 34));
    return 0;
}
