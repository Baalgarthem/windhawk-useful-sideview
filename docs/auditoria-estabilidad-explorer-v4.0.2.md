# Auditoría de estabilidad de Explorer — V2

## Alcance

La auditoría se centró en las rutas que pueden ejecutarse dentro de
`explorer.exe`: creación y destrucción de ventanas, múltiples ventanas o
pestañas, callbacks de Media Foundation, timers, cambios de ajustes, estado
compartido y trabajos de fusión de carpetas.

## Riesgos encontrados y correcciones

| Área | Riesgo | Corrección aplicada |
|---|---|---|
| Estado por ventana | Un estado podía seguir visible para el mapa global mientras comenzaba su liberación. | `DestruirEstado` marca el estado como cerrado, detiene el timer y lo retira del mapa antes de liberar vistas y HWNDs. |
| Hook de creación | La instalación de una subclase podía crear ventanas auxiliares y reentrar al hook. | Se añadió una guarda `thread_local` alrededor de la instalación de subclases. |
| Recarga del mod | El indicador de descarga podía permanecer activo tras una reinicialización en el mismo proceso. | `InicializarModV2` reinicia explícitamente el estado de descarga. |
| Ajustes en caliente | Una vista destruida o no creada podía recibir `ActualizarConfiguracion`. | La notificación ahora comprueba la vista antes de invocarla. |
| Reproductor | Un formato inválido no debe propagar errores a Explorer. | Media Foundation queda aislado en el servicio inyectable; los errores se convierten en mensajes dentro del área de video y el callback se desconecta antes de liberar el backend. |
| Fusión de carpetas | El trabajo de archivos no debe bloquear el hilo visual ni dejar excepciones escapar. | El trabajo continúa en segundo plano, captura excepciones, admite cancelación y publica sólo una notificación si el HWND sigue siendo válido. |
| Persistencia | Una escritura parcial podía publicar un contador nuevo antes que sus elementos. | El formato V3 escribe el contador al final y valida cantidad, esquema y cadenas al cargar; además migra V2 sin borrar la fuente histórica. |

## Reglas conservadas

- No se ejecutan operaciones de persistencia desde `WM_PAINT`.
- Cada ventana conserva su propio `EstadoPanelV2`; los servicios stateless o
  protegidos se comparten sólo cuando es seguro.
- Los callbacks multimedia no poseen el panel: sólo publican mensajes y usan
  un destino atómico desconectable.
- La liberación detiene timers, desconecta callbacks, libera COM/Media
  Foundation y destruye las ventanas hijas en orden inverso.
- Los tamaños de previews, documentos y buffers tienen límites defensivos.

## Evidencia de revisión

El bundle V2 se amalgama desde `src/` y se valida con Clang usando
`-fsyntax-only`. La compilación no sustituye una prueba funcional dentro de
Explorer; esa comprobación queda fuera de esta auditoría porque no fue
solicitada como prueba de ejecución.
