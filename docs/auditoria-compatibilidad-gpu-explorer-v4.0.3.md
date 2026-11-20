# Auditoría de compatibilidad con GPU-accelerated Explorer View — V4.0.3

## Orden de la integración

1. Auditar el mod de referencia y localizar su contrato de interoperabilidad.
2. Separar la superficie Shell lógica de la superficie que realmente dibuja.
3. Inyectar un servicio de compatibilidad en el banco de dependencias V2.
4. Mantener la UI limitada al dibujado y al manejo visual de mensajes.
5. Validar estáticamente el bundle y sincronizar la versión.

## Hallazgos del mod de referencia

El archivo `refs/gpu-accelerated-explorer-view.wh.cpp` mantiene viva la vista
Shell (`SHELLDLL_DefView`) porque la usa para COM, selección, navegación y
comandos nativos, pero en los modos GPU:

- crea un overlay DirectComposition como hijo directo de `CabinetWClass`;
- lo registra mediante la propiedad
  `WindhawkExplorerGpuOverlayOwnsShellView` en la vista Shell;
- oculta o colapsa la superficie nativa para que no se dupliquen filas;
- conserva la geometría y el ciclo de vida por pestaña;
- vuelve a la presentación nativa cuando el usuario desactiva el modo GPU.

Por tanto, `IShellView::GetWindow()` no es necesariamente la superficie
visible. El código anterior de Useful Side View usaba esa ventana y su padre
como si fueran la superficie visual, lo que dejaba el panel dentro de un host
oculto o fuera del orden Z del renderer GPU.

## Adaptador V2

`ServicioCompatibilidadGpuV2` es una dependencia inyectable y sin estado que:

- consume únicamente la propiedad HWND del contrato observado;
- valida que el overlay sea una ventana visible, hija directa del mismo frame y
  perteneciente al mismo árbol de Explorer;
- devuelve dos identidades: `vistaShell` para COM/selección y
  `superficieVisual` para el layout del panel;
- usa la ruta nativa cuando el overlay no está activo;
- actualiza los límites leyendo la geometría real del overlay.

La integración no enlaza contra símbolos, clases ni memoria del otro mod. Si el
mod GPU no está instalado, cambia de versión o está en modo nativo, el
adaptador falla cerrado y se conserva el comportamiento original.

## Política de layout cooperativo

El panel se convierte en un hijo del frame sólo durante la compatibilidad GPU y
se superpone en el sector derecho de la superficie visual. El overlay GPU no se
redimensiona. Esto es deliberado:

- evita competir con los hooks `SetWindowPos`/`DeferWindowPos` del mod GPU;
- evita invalidar su árbol DirectComposition;
- evita que el mod de referencia vuelva a ocupar el área completa después de
  cada timer o evento de navegación;
- mantiene intacta la superficie Shell que el otro mod utiliza como modelo.

En modo comparador, el panel cubre toda la superficie GPU como ya lo hace la
experiencia de comparación en la ruta nativa. Al cerrar, sólo se oculta el
panel; no se intenta “restaurar” ni mover el overlay del mod externo.

## Separación UI / lógica

La UI sigue dibujando exclusivamente sobre el buffer del panel. La detección
del overlay, validación de HWND, captura de límites y política de cooperación
viven en `src/v2/nucleo/servicio_compatibilidad_gpu.hpp`. El gestor de ventanas
únicamente coordina el ciclo de vida y aplica el resultado del servicio.

La selección y la carpeta continúan obteniéndose del `IShellView` inyectado;
Useful Side View no duplica el enumerador GPU ni intenta leer su estado
interno. Así la compatibilidad es removible sin dejar dependencias en las
vistas.

## Riesgos auditados

| Riesgo | Tratamiento |
|---|---|
| Overlay perteneciente a otra ventana | Se valida `GetAncestor` y `GetParent` contra el frame actual. |
| Overlay oculto por modo nativo | Se usa la ruta Shell normal. |
| Renderer GPU redimensionado desde el panel | Se prohíbe redimensionar la superficie GPU; sólo se superpone el panel. |
| Cambio de tamaño del frame | Se recapturan límites en `WM_SIZE` y en el timer existente. |
| Destrucción del overlay o de Explorer | Todas las operaciones pasan por `IsWindow` y el estado existente se libera antes de destruir el panel. |
| Reentrada de hooks externos | No se agregan hooks globales ni llamadas al código del mod GPU. |
| Fallos de identificación | El adaptador falla cerrado y no sustituye la vista nativa. |

## Validación

Se validan el bundle amalgamado y la sintaxis C++ con Clang, además del parseo
del compilador PowerShell. No se ejecutan pruebas funcionales dentro de
Explorer porque no fueron solicitadas.
