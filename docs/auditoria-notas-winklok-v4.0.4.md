# Auditoría e implementación segura de notas con WinKlok — V4.0.4

## Alcance

Se auditó `refs/winklok.wh.cpp` como referencia de comportamiento y persistencia.
El archivo de referencia no se modifica ni se compila como parte de V2. La función
implementada vive en `src/v2/nucleo/servicio_notas.hpp` y se inyecta en la vista
general mediante `DependenciasVistaV2`.

## Hallazgos relevantes

WinKlok persiste el índice en `noteIds`, el siguiente identificador en `noteNext`
y cada nota en `note<ID>`. Los cuerpos que pueden superar el límite de un valor se
dividen en `note<ID>_0`, `note<ID>_1`, etc., con el contador `note<ID>_n`. Su
serialización utiliza `modified|escaped-title|escaped-body`; `\\`, `|`, `;` y los
saltos de línea se escapan antes de guardar.

Las llamadas `Wh_Get*Value` están aisladas por mod. Por ello, llamar esas funciones
desde Useful Side View no permite leer WinKlok: sólo devolvería el `LocalStorage`
propio de Useful Side View. La compatibilidad V4.0.4 resuelve el `engine.ini` del
Windhawk cargado, detecta si la instalación usa registro o INI y accede únicamente a
`ModsWritable\\winklok\\LocalStorage`. Si no puede resolverlo, la función se limita
a las notas nativas y no altera Explorer.

## Diseño aplicado

1. `ServicioNotasV2` es la única autoridad de datos. La vista sólo dibuja tarjetas,
   botones y el detalle; las operaciones de crear, editar, eliminar, copiar,
   persistir y sincronizar se delegan al servicio inyectado.
2. Las notas nativas usan claves propias `useful-sideview.v3.notas.*` y un formato
   versionable con fuente e identificador espejo de WinKlok. Los cuerpos se fragmentan
   a 30 000 caracteres y se limpian los fragmentos obsoletos.
3. Las notas importadas desde WinKlok se guardan localmente con fuente `WinKlok`.
   Las notas creadas en Useful Side View conservan fuente `Nativa` aunque se reflejen
   en WinKlok, evitando duplicados visuales y permitiendo distinguir su origen.
4. La sincronización compara `modified` en segundos Unix. El contenido más reciente
   gana; si no existe un espejo se asigna un identificador WinKlok libre. Las
   eliminaciones se mantienen como tombstones hasta que el almacén externo puede
   actualizarse.
5. El botón **Forzar sincronización** puede crear el almacén writable de WinKlok sólo
   cuando Windhawk muestra que el mod existe. No escribe en una ruta arbitraria,
   no toca la configuración global y tolera que WinKlok esté desactivado.

## Blindajes

- Sólo se aceptan raíces HKCU, HKU o HKLM declaradas por el `engine.ini` de
  Windhawk y se usa `KEY_WOW64_64KEY` para mantener el mismo ámbito que el engine
  x64. Si el ACL del almacén no permite la operación, el puente falla cerrado.
- Se acota el número de fragmentos externos a 512, se deduplican IDs y se evita
  reanimar notas WinKlok que el usuario eliminó.
- El fallo del backend externo nunca impide guardar notas nativas.
- El editor modal tiene cancelación explícita, buffers dimensionados con la longitud
  real y no se ejecutan operaciones de sincronización durante el renderizado GDI.
- La vista no accede a `refs/`, `legacy/` ni a APIs internas no documentadas de
  Windhawk; sólo usa las APIs Win32 de registro/INI necesarias para el puente de
  compatibilidad y el contrato de almacenamiento descrito por el engine.

## Resultado

La función queda preparada para retirar el adaptador WinKlok sin alterar el modelo
de notas nativas: basta con eliminar la dependencia `AlmacenNotasWinKlokV2` y dejar
`ServicioNotasV2` en modo local. El bundle final conserva un único artefacto en
`dist/useful-sideview.wh.cpp`.
