#pragma once

struct DisenoBarraProgreso {
    RECT barraControles{};
    RECT areaInteractiva{};
    RECT botonReproduccion{};
    RECT botonSonido{};
    RECT botonRepeticion{};
    int limiteIzquierdo = 0;
    int limiteDerecho = 0;
    int centroVertical = 0;
};

inline int calcularInicioGrupoBotones(int anchoCliente) {
    return std::max(0, (anchoCliente - ANCHO_GRUPO_BOTONES) / 2);
}

inline double limitarProporcionProgreso(double proporcion) {
    return std::max(0.0, std::min(proporcion, 1.0));
}

inline double calcularProporcionProgreso(
    const DisenoBarraProgreso& disenoBarraProgreso,
    int posicionHorizontal) {
    if (disenoBarraProgreso.limiteDerecho <=
        disenoBarraProgreso.limiteIzquierdo) {
        return 0.0;
    }
    int posicionLimitada = std::max(
        disenoBarraProgreso.limiteIzquierdo,
        std::min(posicionHorizontal, disenoBarraProgreso.limiteDerecho));
    return static_cast<double>(posicionLimitada -
                               disenoBarraProgreso.limiteIzquierdo) /
           static_cast<double>(disenoBarraProgreso.limiteDerecho -
                               disenoBarraProgreso.limiteIzquierdo);
}

inline int calcularPosicionProgreso(
    const DisenoBarraProgreso& disenoBarraProgreso, double proporcion) {
    return disenoBarraProgreso.limiteIzquierdo + static_cast<int>(
        (disenoBarraProgreso.limiteDerecho -
         disenoBarraProgreso.limiteIzquierdo) *
        limitarProporcionProgreso(proporcion));
}

inline bool contienePunto(const RECT& rectangulo, int posicionHorizontal,
                          int posicionVertical) {
    POINT punto{posicionHorizontal, posicionVertical};
    return PtInRect(&rectangulo, punto) != FALSE;
}

inline bool esClicEnBarraProgreso(
    const DisenoBarraProgreso& disenoBarraProgreso, int posicionHorizontal,
    int posicionVertical) {
    if (!contienePunto(disenoBarraProgreso.barraControles, posicionHorizontal,
                       posicionVertical)) {
        return false;
    }
    if (contienePunto(disenoBarraProgreso.botonReproduccion, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonSonido, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonRepeticion, posicionHorizontal,
                      posicionVertical)) {
        return false;
    }
    return posicionHorizontal >= disenoBarraProgreso.limiteIzquierdo &&
           posicionHorizontal <= disenoBarraProgreso.limiteDerecho &&
           posicionVertical < disenoBarraProgreso.botonReproduccion.top;
}

inline DisenoBarraProgreso calcularDisenoBarraProgreso(HWND ventana) {
    RECT areaCliente;
    GetClientRect(ventana, &areaCliente);
    DisenoBarraProgreso disenoBarraProgreso;
    disenoBarraProgreso.barraControles = areaCliente;
    disenoBarraProgreso.limiteIzquierdo = MARGEN_BARRA_PROGRESO;
    disenoBarraProgreso.limiteDerecho = static_cast<int>(std::max<LONG>(
        disenoBarraProgreso.limiteIzquierdo,
        areaCliente.right - MARGEN_BARRA_PROGRESO));
    disenoBarraProgreso.centroVertical = 34;
    int inicioGrupoBotones = calcularInicioGrupoBotones(areaCliente.right);
    int bordeSuperiorBotones = 54;
    disenoBarraProgreso.areaInteractiva = {
        disenoBarraProgreso.limiteIzquierdo, 5,
        disenoBarraProgreso.limiteDerecho, bordeSuperiorBotones - 4};
    disenoBarraProgreso.botonReproduccion = {
        inicioGrupoBotones, bordeSuperiorBotones,
        inicioGrupoBotones + TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonSonido = {
        disenoBarraProgreso.botonReproduccion.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonReproduccion.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonRepeticion = {
        disenoBarraProgreso.botonSonido.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonSonido.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    return disenoBarraProgreso;
}
