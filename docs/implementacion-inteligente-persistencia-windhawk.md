# Implementación inteligente de persistencia de datos en mods de Windhawk

## Objetivo

Este documento reúne el patrón de persistencia observado en los mods de
referencia del directorio `refs` y lo adapta a Useful Side View V2. La meta es
que los ajustes administrados por Windhawk y los datos propios del mod tengan
responsabilidades separadas, claves estables, migración segura y un costo
mínimo dentro de `explorer.exe`.

## Evidencia revisada

Se revisaron tres implementaciones con estrategias complementarias:

| Referencia | Patrón observado | Aplicación útil para V2 |
|---|---|---|
| `refs/bluetooth-auto-toggle.wh.cpp` | Carga todos los ajustes en una estructura local y publica el nuevo snapshot bajo un candado; usa `Wh_Get/SetIntValue` y `Wh_Get/SetBinaryValue` para estado de ejecución y marcadores por sesión. | Separar configuración de Windhawk del estado interno y usar un snapshot coherente para cada recarga. |
| `refs/quick-look.wh.cpp` | Lee `Wh_Get*Setting` en `LoadSettings`, conserva valores de usuario con `Wh_Get/SetIntValue` y restablece los valores persistidos cuando la opción de recordar está activa. El ciclo de vida detiene trabajadores antes de liberar el mod. | Leer persistencia una vez al inicializar, volver a cargarla ante cambios y nunca hacer I/O desde el renderizador. |
| `refs/explorer-column-view.wh.cpp` | Usa claves con nombres versionados, valida rangos, comprueba el resultado de `Wh_SetIntValue` y elimina valores con `Wh_DeleteValue` cuando la opción deja de aplicar. | Usar namespaces estables, validación estricta y manejo explícito de fallos. |

La API nativa de Windhawk ofrece tres familias distintas:

1. `Wh_GetIntSetting`, `Wh_GetStringSetting` y sus equivalentes son para los
   campos declarados en `WindhawkModSettings`.
2. `Wh_Get/SetIntValue`, `Wh_Get/SetStringValue` y
   `Wh_Get/SetBinaryValue` son para datos internos persistentes del mod.
3. `Wh_DeleteValue` elimina datos internos obsoletos o desactivados.

No se debe usar el registro de Windows directamente para sustituir esta API:
los valores de Windhawk pertenecen al mod, sobreviven al reinicio de
`explorer.exe` y permiten que el host administre su ciclo de vida.

## Patrón unificado para Useful Side View

### 1. Configuración declarada

Los switches visibles en ajustes se leen con `Wh_Get*Setting` dentro de una
función de carga única. Esa función publica los resultados en variables
atómicas o en un snapshot protegido. `Wh_ModSettingsChanged` vuelve a ejecutar
la carga y notifica a las ventanas activas mediante mensajes, sin que la UI
consulte Windhawk durante `WM_PAINT`.

### 2. Estado propio del mod

Los datos que no son ajustes editables —por ejemplo, ejecutables seleccionados
en "Abrir con"— se almacenan con `Wh_*Value`. Cada dominio recibe un namespace
estable y versionado:

```text
useful-sideview.v3.abrirCon.<dominio>.schema
useful-sideview.v3.abrirCon.<dominio>.count
useful-sideview.v3.abrirCon.<dominio>.program.0 ... program.N
useful-sideview.v3.abrirCon.<dominio>.last
```

Los dominios actuales son `carpetas`, `comprimidos`, `imagenes` y `videos`.
Separarlos evita que un ejecutable válido para una categoría se convierta por
accidente en el predeterminado de otra, pero todos usan el mismo servicio de
persistencia y el mismo formato.

### 3. Escritura resistente

La escritura se hace fuera del renderizado y bajo el mutex del servicio:

1. Normalizar y deduplicar la ruta del ejecutable sin distinguir mayúsculas.
2. Escribir todos los elementos de la lista.
3. Escribir `last`.
4. Escribir `count` al final como marcador de commit lógico.

Si una operación falla, se conserva el snapshot anterior y se registra el
fallo. Al cargar, un `count` inválido, excesivo o ausente hace que se ignoren
los datos incompletos y se usen los valores por defecto. Esto no ofrece una
transacción de disco completa —Windhawk no expone una operación transaccional—,
pero evita publicar un contador nuevo antes de que el contenido esté listo.

### 4. Migración

La versión nueva intenta primero `v3`. Si no existe, lee las claves históricas
`abrirCon.v2.<dominio>`, fusiona sus valores con los predeterminados y los
reescribe en el formato nuevo. La migración es idempotente y no borra las
claves antiguas hasta que la carga nueva se haya confirmado correctamente.

### 5. Reglas de rendimiento y estabilidad

- Leer persistencia sólo al crear el servicio, no en `Renderizar`, timers ni
  callbacks de dibujo.
- Mantener en memoria la lista y el último programa; la UI sólo recibe copias.
- Proteger la lista con un mutex pequeño y no sostenerlo durante
  `ShellExecuteW`, diálogos o llamadas COM.
- Limitar tamaños y cantidades antes de reservar memoria.
- No almacenar handles, punteros, HWND ni rutas temporales de una ventana.
- Comprobar el resultado de cada API de escritura y no lanzar excepciones hacia
  Explorer.
- Mantener la relectura de ajustes separada de la persistencia de uso.

## Decisión aplicada a V2

Useful Side View usa un `ServicioPersistenciaWindhawkV2` inyectado en cada
servicio `Abrir con`. `ServicioAbrirConV2` conserva únicamente la política de
dominio —lista, último ejecutable, deduplicación y migración—, mientras el
servicio compartido encapsula las llamadas nativas de Windhawk. Así la UI no
conoce claves ni almacenamiento y la función puede retirarse o sustituirse sin
modificar las vistas.

La versión del mod sigue una fuente de verdad en V2. El compilador verifica que
la versión del metadato `@version`, la constante visible en la UI y el bundle
coincidan antes de generar `dist/useful-sideview.wh.cpp`. Los cambios de
versión actualizan esas referencias y el menú del compilador permite mensajes
personalizados o mensajes genéricos por defecto.
