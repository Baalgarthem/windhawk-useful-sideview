# Auditoría de estabilidad del ciclo de vida V2 — 4.1.7

## Alcance

Se revisaron los cinco mods de referencia disponibles en `refs/` para
identificar prácticas que eviten que Explorer salte a código de una DLL ya
descargada, que queden ventanas con `WNDPROC` inválido o que sobrevivan hilos,
callbacks y objetos COM al desactivar el mod:

- `refs/explorer-column-view.wh.cpp`
- `refs/gpu-accelerated-explorer-view.wh.cpp`
- `refs/quick-look.wh.cpp`
- `refs/bluetooth-auto-toggle.wh.cpp`
- `refs/winklok.wh.cpp`

La auditoría se concentró en inicialización, creación de múltiples ventanas,
subclases, clases Win32, hooks, workers, Media Foundation, COM y descarga.

## Patrones seguros encontrados

| Referencia | Implementación relevante | Aplicación en Useful Side View |
| --- | --- | --- |
| `explorer-column-view` | Bandera atómica de descarga, contador de callbacks activos, drenaje posterior a la retirada de detours, parada y espera del hilo coordinador, limpieza de clases y objetos COM. | `ciclo_vida_v2.hpp`, `AmbitoCallbackV2`, `DrenarCallbacksV2` y retirada explícita de subclases superiores. |
| `gpu-accelerated-explorer-view` | Separación entre `Wh_ModBeforeUninit` y `Wh_ModUninit`; primero quiesce observers/overlays y después, cuando Windhawk ya retiró detours, drena callbacks y libera registries, workers y clases. | `Wh_ModBeforeUninit` bloquea trabajo nuevo; `Wh_ModUninit` desmonta ventanas en su hilo, drena callbacks y libera clases V2. |
| `quick-look` | Envía una orden de limpieza a cada Explorer en el hilo propietario, espera a que todas las ventanas respondan y sólo después desregistra clases. Workers cancelables y esperados. | `WM_SIDEVIEW_UNLOAD_V2` se envía síncronamente al panel o contenedor propietario; los diálogos modales se cierran antes de desregistrar clases. |
| `bluetooth-auto-toggle` | Señal de parada, wake-up y espera del worker antes de cerrar handles. | Se conserva el cierre bloqueante del hilo del hook de teclado antes de descargar. |
| `winklok` | Todos los hilos de inicialización, lectura, sincronización y trabajos se detienen y se esperan antes de retornar de `Wh_ModUninit`. | Se verificó que el único worker V2 (`ServicioFusionCarpetasV2`) se une en su destructor y se evita crear trabajo durante la descarga. |

## Causas detectadas en V2

1. `ProcedimientoSubclaseTop` se instalaba en cada `CabinetWClass` y
   `ExploreWClass`, tanto existentes como creadas después, pero
   `DesinicializarModV2` nunca las retiraba. Al recompilar o desactivar, el
   siguiente mensaje de Explorer podía invocar la dirección de una DLL que ya
   no estaba cargada.
2. La descarga destruía `EstadoPanelV2` desde el hilo de configuración de
   Windhawk. Las ventanas del panel pertenecen al hilo UI de Explorer y
   `DestroyWindow` no debe cruzar ese límite. Si fallaba, el estado podía
   borrarse mientras la ventana conservaba `GWLP_USERDATA`.
3. Algunas clases V2 se registraban con `GetModuleHandleW(nullptr)`, que es el
   módulo del ejecutable `explorer.exe`, y no se desregistraban todas. Esto
   podía dejar una clase con un `WNDPROC` de una generación anterior tras una
   recarga del mod.
4. El hook de teclado también usaba el módulo del ejecutable para registrar un
   `WH_KEYBOARD_LL`; ahora usa el módulo real que contiene el código V2.
5. El hook `CreateWindowExW` no validaba `lpClassName` antes de `_wcsicmp` y
   su guardia de reentrada no tenía limpieza RAII.
6. Los callbacks de ventana y Media Foundation podían publicar mensajes
   mientras empezaba la descarga, aumentando la ventana de carrera entre
   `Uninit` y la liberación de estados.

## Correcciones aplicadas

- Se creó `src/v2/nucleo/ciclo_vida_v2.hpp` con:
  - `g_descargandoMod` como puerta de quiescencia común.
  - contador de callbacks activos y `AmbitoCallbackV2`.
  - drenaje acotado antes de desregistrar clases.
  - resolución del `HINSTANCE` real de la DLL V2 mediante
    `GetModuleHandleExW(...FROM_ADDRESS...)`.
  - cierre síncrono de ventanas modales por clase.
- Se agregó `Wh_ModBeforeUninit` para cerrar la admisión de trabajo y detener
  el hook de teclado antes de que Windhawk retire los detours.
- Se retiran las subclases top de todas las ventanas Explorer y se ignoran
  nuevas entradas durante la descarga.
- Se agregó `WM_SIDEVIEW_UNLOAD_V2`; el panel recibe la orden con `SendMessageW`
  y destruye sus recursos en su propio hilo. Si el panel ya no existe, el
  contenedor recibe la orden de respaldo.
- `DestruirEstado` desinstala primero las subclases, corta `GWLP_USERDATA` antes
  de `DestroyWindow`, libera la vista y sólo entonces borra el estado.
- El reproductor Media Foundation desconecta eventos durante la quiescencia,
  evita publicar nuevos mensajes, usa el módulo real y desregistra su clase.
- Los prompts de carpetas y notas usan el módulo real, validan la clase
  existente y se cierran/desregistran durante la descarga.
- `CreateWindowExW_Hook` valida punteros, comprueba el estado de descarga y usa
  una guardia RAII para la reentrada.

## Resultado esperado

La ruta de activación/desactivación queda ordenada así:

```text
BeforeUninit
  -> bloquear entradas nuevas
  -> detener hook de teclado
  -> Windhawk retira detours
Uninit
  -> cerrar prompts
  -> retirar subclases top
  -> pedir limpieza al hilo UI de cada panel
  -> drenar callbacks Media Foundation/ventana
  -> desregistrar clases del módulo
  -> retornar con estados y workers liberados
```

La integración GPU no modifica su overlay ni su layout DirectComposition. Sólo
se aplican las mismas reglas de quiescencia y limpieza a las superficies que
Useful Side View crea encima de él.

Esta revisión es estática: no se ejecutaron pruebas funcionales ni se forzó el
ciclo de Explorer, de acuerdo con la instrucción del proyecto de no hacer
tests salvo solicitud explícita. Sí se verificó la sintaxis estricta del
entrypoint V2 y la sincronización del bundle final.
