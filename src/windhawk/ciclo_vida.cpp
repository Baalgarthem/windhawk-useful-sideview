namespace {

void cargarConfiguracion() {
    configuracion.reproduccionAutomatica.store(
        Wh_GetIntSetting(L"autoplay") != 0);
    configuracion.sonidoHabilitado.store(Wh_GetIntSetting(L"sound") != 0);
}

}  // namespace

BOOL Wh_ModInit() {
    cargarConfiguracion();
    if (!registrarClasesDeVentana()) return FALSE;
    detenerHilo.store(false);
    hiloReproductor =
        CreateThread(nullptr, 0, ejecutarHiloReproductor, nullptr, 0, nullptr);
    if (!hiloReproductor) {
        eliminarClasesDeVentana();
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
    if (hiloReproductor) {
        WaitForSingleObject(hiloReproductor, 3000);
        CloseHandle(hiloReproductor);
        hiloReproductor = nullptr;
    }
    eliminarClasesDeVentana();
}
