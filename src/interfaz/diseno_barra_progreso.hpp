#pragma once

struct DisenoBarraProgreso {
    RECT barraControles{};
    RECT areaInteractiva{};
    RECT botonAnterior{};
    RECT botonReproduccion{};
    RECT botonSiguiente{};
    RECT botonSonido{};
    RECT botonRepeticion{};
    RECT botonVelocidad{};
    RECT panelVelocidad{};
    RECT opcionesVelocidad[TOTAL_OPCIONES_VELOCIDAD]{};
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
    if (contienePunto(disenoBarraProgreso.botonAnterior, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonReproduccion, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonSiguiente, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonSonido, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonRepeticion, posicionHorizontal,
                      posicionVertical) ||
        contienePunto(disenoBarraProgreso.botonVelocidad, posicionHorizontal,
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
    int bordeSuperiorBotones = 54;
    disenoBarraProgreso.areaInteractiva = {
        disenoBarraProgreso.limiteIzquierdo, 5,
        disenoBarraProgreso.limiteDerecho, bordeSuperiorBotones - 4};

    if (tipoContenidoActivo.load() == TipoContenidoArchivo::Imagen) {
        int anchoGrupoBotonesImagen =
            TAMANO_BOTON_CONTROL * 2 + SEPARACION_BOTONES;
        int inicioGrupoBotones = std::max<int>(
            0, (static_cast<int>(areaCliente.right) - anchoGrupoBotonesImagen) / 2);
        disenoBarraProgreso.botonAnterior = {
            inicioGrupoBotones, bordeSuperiorBotones,
            inicioGrupoBotones + TAMANO_BOTON_CONTROL,
            bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
        disenoBarraProgreso.botonSiguiente = {
            disenoBarraProgreso.botonAnterior.right + SEPARACION_BOTONES,
            bordeSuperiorBotones,
            disenoBarraProgreso.botonAnterior.right + SEPARACION_BOTONES +
                TAMANO_BOTON_CONTROL,
            bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
        return disenoBarraProgreso;
    }

    int inicioGrupoBotones = calcularInicioGrupoBotones(areaCliente.right);
    disenoBarraProgreso.botonAnterior = {
        inicioGrupoBotones, bordeSuperiorBotones,
        inicioGrupoBotones + TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonReproduccion = {
        disenoBarraProgreso.botonAnterior.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonAnterior.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonSiguiente = {
        disenoBarraProgreso.botonReproduccion.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonReproduccion.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonSonido = {
        disenoBarraProgreso.botonSiguiente.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonSiguiente.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};
    disenoBarraProgreso.botonRepeticion = {
        disenoBarraProgreso.botonSonido.right + SEPARACION_BOTONES,
        bordeSuperiorBotones,
        disenoBarraProgreso.botonSonido.right + SEPARACION_BOTONES +
            TAMANO_BOTON_CONTROL,
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL};

    int centroVerticalBotones =
        bordeSuperiorBotones + TAMANO_BOTON_CONTROL / 2;
    int izquierdaBotonVelocidad = std::max<int>(
        static_cast<int>(disenoBarraProgreso.botonRepeticion.right) + 8,
        disenoBarraProgreso.limiteDerecho - ANCHO_BOTON_VELOCIDAD);
    disenoBarraProgreso.botonVelocidad = {
        izquierdaBotonVelocidad,
        centroVerticalBotones - ALTO_BOTON_VELOCIDAD / 2,
        izquierdaBotonVelocidad + ANCHO_BOTON_VELOCIDAD,
        centroVerticalBotones + ALTO_BOTON_VELOCIDAD / 2};

    int izquierdaPanelVelocidad =
        disenoBarraProgreso.botonVelocidad.right - ANCHO_PANEL_VELOCIDAD;
    int fondoPanelVelocidad = disenoBarraProgreso.botonVelocidad.top - 2;
    int topePanelVelocidad = fondoPanelVelocidad - ALTO_PANEL_VELOCIDAD;
    disenoBarraProgreso.panelVelocidad = {
        izquierdaPanelVelocidad, topePanelVelocidad,
        disenoBarraProgreso.botonVelocidad.right, fondoPanelVelocidad};

    for (int posicionOpcion = 0; posicionOpcion < TOTAL_OPCIONES_VELOCIDAD;
         ++posicionOpcion) {
        int topeOpcion = topePanelVelocidad + 4 +
                         posicionOpcion * ALTO_OPCION_VELOCIDAD;
        disenoBarraProgreso.opcionesVelocidad[posicionOpcion] = {
            izquierdaPanelVelocidad + 4, topeOpcion,
            disenoBarraProgreso.botonVelocidad.right - 4,
            topeOpcion + ALTO_OPCION_VELOCIDAD};
    }

    return disenoBarraProgreso;
}

struct DisenoComprimidos {
    RECT areaControles{};
    bool tieneComics{false};

    // Fila Abrir con (exclusiva para cómics)
    RECT areaEtiquetaAbrirCon{};
    RECT botonDesplegablePrograma{};
    RECT panelOpcionesPrograma{};
    std::vector<RECT> opcionesPrograma{};
    RECT botonEjecutarAbrir{};

    // Fila Convertir a
    RECT areaEtiquetaConversion{};
    RECT botonDesplegableFormato{};
    RECT panelOpcionesFormato{};
    RECT opcionesFormato[TOTAL_FORMATOS_DESTINO]{};
    RECT botonEjecutarConversion{};

    // Recuadro Log
    RECT areaRecuadroLog{};
};

inline DisenoComprimidos calcularDisenoComprimidos(HWND ventana) {
    RECT areaCliente;
    GetClientRect(ventana, &areaCliente);
    DisenoComprimidos diseno;
    diseno.areaControles = areaCliente;
    diseno.tieneComics = estadoComprimidos.tieneComicsSeleccionados;

    int margenLateral = 14;
    int anchoDisponible = std::max<int>(
        100, static_cast<int>(areaCliente.right) - margenLateral * 2);

    int altoOpcion = 24;

    int altoFila = 28;
    int anchoBotonAccion = 82;

    // Fila 1: Abrir con
    int topeFilaAbrir = 8;
    int anchoDesplegableProg = std::min<int>(
        170, std::max<int>(110, (anchoDisponible - anchoBotonAccion - 12) / 2 + 30));
    int anchoEtiquetaAbrir = std::max<int>(
        68, anchoDisponible - anchoDesplegableProg - anchoBotonAccion - 16);

    diseno.areaEtiquetaAbrirCon = {
        margenLateral, topeFilaAbrir,
        margenLateral + anchoEtiquetaAbrir,
        topeFilaAbrir + altoFila};

    diseno.botonDesplegablePrograma = {
        diseno.areaEtiquetaAbrirCon.right + 8, topeFilaAbrir,
        diseno.areaEtiquetaAbrirCon.right + 8 + anchoDesplegableProg,
        topeFilaAbrir + altoFila};

    diseno.botonEjecutarAbrir = {
        diseno.botonDesplegablePrograma.right + 8, topeFilaAbrir,
        static_cast<LONG>(areaCliente.right) - margenLateral,
        topeFilaAbrir + altoFila};

    // Menú desplegable de programas
    size_t totalOpcionesPrograma =
        obtenerListaProgramasActivaConst().size() + 1;
    int altoPanelProg =
        static_cast<int>(totalOpcionesPrograma) * altoOpcion + 6;
    int topePanelProg = diseno.botonDesplegablePrograma.bottom + 2;
    diseno.panelOpcionesPrograma = {
        diseno.botonDesplegablePrograma.left, topePanelProg,
        diseno.botonDesplegablePrograma.right + 40,
        topePanelProg + altoPanelProg};

    diseno.opcionesPrograma.resize(totalOpcionesPrograma);
    for (size_t posicionOpcion = 0; posicionOpcion < totalOpcionesPrograma;
         ++posicionOpcion) {
        diseno.opcionesPrograma[posicionOpcion] = {
            diseno.panelOpcionesPrograma.left + 2,
            topePanelProg + 3 + static_cast<int>(posicionOpcion) * altoOpcion,
            diseno.panelOpcionesPrograma.right - 2,
            topePanelProg + 3 +
                static_cast<int>(posicionOpcion + 1) * altoOpcion};
    }

    // Fila 2: Convertir a
    int topeFilaConv = topeFilaAbrir + altoFila + 6;
    int anchoDesplegableForm = std::min<int>(
        130, std::max<int>(90, (anchoDisponible - anchoBotonAccion - 12) / 2));
    int anchoEtiquetaConv = std::max<int>(
        68, anchoDisponible - anchoDesplegableForm - anchoBotonAccion - 16);

    diseno.areaEtiquetaConversion = {
        margenLateral, topeFilaConv,
        margenLateral + anchoEtiquetaConv,
        topeFilaConv + altoFila};

    diseno.botonDesplegableFormato = {
        diseno.areaEtiquetaConversion.right + 8, topeFilaConv,
        diseno.areaEtiquetaConversion.right + 8 + anchoDesplegableForm,
        topeFilaConv + altoFila};

    diseno.botonEjecutarConversion = {
        diseno.botonDesplegableFormato.right + 8, topeFilaConv,
        static_cast<LONG>(areaCliente.right) - margenLateral,
        topeFilaConv + altoFila};

    int altoPanelFormato = altoOpcion * TOTAL_FORMATOS_DESTINO + 6;
    int topePanelFormato = diseno.botonDesplegableFormato.bottom + 2;
    diseno.panelOpcionesFormato = {
        diseno.botonDesplegableFormato.left, topePanelFormato,
        diseno.botonDesplegableFormato.right,
        topePanelFormato + altoPanelFormato};

    for (int posicion = 0; posicion < TOTAL_FORMATOS_DESTINO; ++posicion) {
        diseno.opcionesFormato[posicion] = {
            diseno.panelOpcionesFormato.left + 2,
            topePanelFormato + 3 + posicion * altoOpcion,
            diseno.panelOpcionesFormato.right - 2,
            topePanelFormato + 3 + (posicion + 1) * altoOpcion};
    }

    // Fila 3: Log
    int topeSeccionLog = topeFilaConv + altoFila + 6;
    int fondoSeccionLog = std::max<int>(
        topeSeccionLog + 30, static_cast<int>(areaCliente.bottom) - 6);
    diseno.areaRecuadroLog = {
        margenLateral, topeSeccionLog,
        static_cast<LONG>(areaCliente.right) - margenLateral,
        fondoSeccionLog};

    return diseno;
}


