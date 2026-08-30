namespace {

void cargarConfiguracion() {
    configuracion.reproduccionAutomatica.store(
        Wh_GetIntSetting(L"autoplay") != 0);
    configuracion.sonidoHabilitado.store(Wh_GetIntSetting(L"sound") != 0);
    configuracionPendiente.store(true);
}

}  // namespace

BOOL Wh_ModInit() {
    cargarConfiguracion();
    if (!crearRecursosInterfaz()) {
        Wh_Log(L"No fue posible crear las fuentes de la interfaz");
        return FALSE;
    }
    if (!registrarClasesDeVentana()) {
        Wh_Log(L"No fue posible registrar las clases de ventana");
        liberarRecursosInterfaz();
        return FALSE;
    }
    detenerHilo.store(false);
    eventoDetener = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!eventoDetener) {
        Wh_Log(L"No fue posible crear el evento de detención (error %u)",
               static_cast<unsigned int>(GetLastError()));
        eliminarClasesDeVentana();
        liberarRecursosInterfaz();
        return FALSE;
    }
    hiloReproductor =
        CreateThread(nullptr, 0, ejecutarHiloReproductor, nullptr, 0, nullptr);
    if (!hiloReproductor) {
        Wh_Log(L"No fue posible crear el hilo reproductor (error %u)",
               static_cast<unsigned int>(GetLastError()));
        CloseHandle(eventoDetener);
        eventoDetener = nullptr;
        eliminarClasesDeVentana();
        liberarRecursosInterfaz();
        return FALSE;
    }
    return TRUE;
}

void Wh_ModSettingsChanged() {
    // No se toca IMFPMediaPlayer aquí: el hilo propietario observará los
    // valores atómicos durante su siguiente iteración.
    cargarConfiguracion();
}

void Wh_ModUninit() {
    detenerHilo.store(true);
    if (eventoDetener) SetEvent(eventoDetener);
    if (hiloReproductor) {
        // No se descarga código mientras el hilo todavía pueda ejecutarlo.
        WaitForSingleObject(hiloReproductor, INFINITE);
        CloseHandle(hiloReproductor);
        hiloReproductor = nullptr;
    }
    if (eventoDetener) {
        CloseHandle(eventoDetener);
        eventoDetener = nullptr;
    }
    eliminarClasesDeVentana();
    liberarRecursosInterfaz();
}
