# AGENTS.md

Este documento es la política principal del proyecto **Useful Side View**. Al consolidarse como política activa debe publicarse con el nombre `AGENTS.md`; mientras coexistan ambos archivos, `Agents (main).md` tiene precedencia y sirve como fuente de verdad para la integración solicitada por el usuario.

Este archivo establece las reglas generales y obligatorias para desarrollar, adaptar y mantener el mod. Todas las instrucciones aquí definidas deben cumplirse de forma puntual y completa, sin sustituir los requerimientos expresos del usuario ni las restricciones de seguridad de la plataforma.

## Contexto general del proyecto

Useful Side View es un mod nativo para Windhawk que se ejecuta dentro de `explorer.exe` y convierte el panel de vista previa del Explorador de archivos de Windows 11 en un reproductor de video ligero, adaptativo e integrado. El mod detecta el archivo seleccionado, inserta una interfaz de reproducción en el panel lateral y utiliza Win32, COM y Media Foundation/MFPlay para reproducir el contenido sin iniciar un reproductor externo.

Las funciones esenciales actuales son reproducir y pausar, adelantar o retroceder mediante una línea de tiempo interactiva, cambiar el estado del audio, activar la repetición, adaptar la interfaz al tamaño del panel y respetar las opciones de autorreproducción y audio. La autorreproducción y el audio deben permanecer desactivados de forma predeterminada. El artefacto que Windhawk consume es el archivo único generado en `dist/useful-sideview.wh.cpp`; su fuente editable se mantiene modularizada en `src`.

## Objetivo general

Desarrollar y mantener un reproductor de video funcional, rápido, estable y de bajo consumo para el panel de vista previa de Windows 11, con controles modernos y centrados, comportamiento adaptativo, configuración segura y una arquitectura mantenible que pueda distribuirse como un único archivo compatible con Windhawk.

## Objetivos específicos

* Mantener una detección confiable del video seleccionado y una integración segura con las ventanas del Explorador.
* Reproducir video y audio mediante la infraestructura multimedia nativa de Windows, liberando oportunamente ventanas, interfaces COM, reproductores, pinceles, fuentes y demás recursos.
* Ofrecer controles semánticos para reproducir/pausar, silenciar/activar audio, repetir y navegar por la línea de tiempo mediante clic, arrastre y teclado.
* Conservar la autorreproducción y el audio desactivados por defecto, permitiendo que el usuario cambie ambas preferencias desde la configuración de Windhawk.
* Mantener el código modular en español, documentado pedagógicamente y verificable mediante pruebas automatizadas, análisis estático, compilación y enlace.
* Generar de forma reproducible `dist/useful-sideview.wh.cpp` sin editar manualmente el artefacto.
* Registrar cambios, bugs, hallazgos, causas raíz, pruebas y soluciones con trazabilidad acumulativa.

## Alcances y limitaciones

* El alcance se limita al panel de vista previa del Explorador de archivos de Windows 11 y al proceso `explorer.exe`; no pretende reemplazar un reproductor multimedia completo.
* Los formatos reproducibles dependen de los códecs y controladores de Media Foundation disponibles en el sistema.
* La integración se apoya en ventanas y comportamientos internos del Explorador que pueden cambiar entre versiones de Windows; toda compatibilidad nueva debe verificarse en versiones concretas.
* La prioridad es la eficiencia dentro de Explorer. No deben añadirse dependencias pesadas, decodificadores duplicados ni trabajo permanente cuando no exista un video activo.
* La interfaz debe seguir siendo utilizable al redimensionar el panel, pero está condicionada por el espacio mínimo que ofrezca el Explorador.
* La validación automatizada demuestra propiedades del código y del enlace; la experiencia final dentro de Windhawk y Explorer requiere también una prueba manual del usuario.
* No se afirmará certificación formal con normas ISO ni compatibilidad universal sin evidencia documentada.

# Reglas generales

* La arquitectura se define según las necesidades reales de Useful Side View y debe garantizar modularidad, separación de responsabilidades, legibilidad, mantenibilidad, rendimiento y uso correcto de memoria, identificadores, ventanas y recursos multimedia.
* La programación orientada a objetos, los patrones de diseño, la programación orientada a eventos y los procesos asíncronos se utilizan únicamente cuando corresponden técnicamente. En este mod se privilegian componentes nativos pequeños, eventos de Media Foundation y mensajes de ventana por encima de abstracciones innecesarias.
* El proyecto debe mantener una estructura semántica. Los nombres de archivos, módulos, carpetas y componentes reflejan con claridad su función dentro del dominio del panel de vista previa y del reproductor.
* Todo el código propio debe seguir **Clean Code en español**:
  * Las variables, funciones, métodos, clases, módulos y constantes utilizan nombres representativos, comprensibles y coherentes con el dominio.
  * Las funciones y métodos se nombran con verbos que expresan la acción, por ejemplo `obtenerVideoSeleccionado`, `ajustarReproductorAlPanel` o `liberarRecursosDelReproductor`.
  * Se evitan abreviaturas ambiguas, letras aisladas y nombres genéricos como `data`, `temp`, `obj`, `item` o `valor` cuando exista un término más preciso.
  * El código se escribe conforme a la función realizada para que el lector pueda seguirlo semánticamente sin depender de comentarios para entender el flujo básico.
  * Se evita trabajar con índices, posiciones numéricas y accesos por subíndice siempre que sea posible. Se prefieren recorridos por elemento, iteradores, algoritmos estándar o abstracciones que nombren la entidad procesada.
  * Cuando una API externa exige un índice, este se limita al bloque mínimo y recibe un nombre que indique qué recorre; no se utilizan `i`, `j` o `k` como nombres opacos.
  * Los identificadores y firmas impuestos por Win32, COM, Media Foundation, MFPlay y Windhawk conservan su nomenclatura oficial; el código propio que los rodea permanece semántico y en español.
  * Siempre que se escriba código se evalúa qué decisiones, restricciones o riesgos necesitan un comentario pedagógico. Los comentarios explican el motivo técnico y no repiten literalmente una instrucción evidente.
* La documentación explica decisiones, comportamientos, pruebas y límites de forma clara, didáctica y útil. Para cualquier asunto relacionado con bugs, documentación o antecedentes se consulta primero `docs`, especialmente `docs/bugs-trace.md`, `docs/registro-cambios.md`, `docs/arquitectura.md` y `docs/estado-proyecto.md`.
* Si se inicia un proyecto nuevo, su estructura debe desplegarse estrictamente. Como Useful Side View ya contiene archivos, lógica y una arquitectura funcional, toda normalización, mejora, combinación o reestructuración debe integrarse gradualmente con el estado actual, conservar el comportamiento válido y evitar código duplicado o basura.
* La estructura de carpetas debe ser modular y proporcional al tamaño y naturaleza del mod. Actualmente `src` se divide en `archivos`, `ejecucion`, `integracion`, `interfaz`, `nucleo`, `reproductor` y `windhawk`.
* Los alcances y limitaciones son obligatorios y se mantienen tanto en este documento como en la documentación especializada.
* Este documento siempre comienza con el contexto general del programa, sus funciones esenciales, su objetivo y sus límites.
* La política del proyecto mantiene un enfoque explícito en ingeniería de software, diseño, arquitectura, calidad interna y evidencia verificable.
* Siempre se adaptan y personalizan las siguientes normas al tamaño, alcance y riesgos reales del mod:
  * **ISO/IEC/IEEE 42010**: se utiliza para describir interesados, inquietudes, puntos de vista, vistas, decisiones y relaciones arquitectónicas del sistema.
  * **ISO/IEC 25010**: guía los atributos de calidad de rendimiento, compatibilidad, confiabilidad, seguridad, usabilidad y mantenibilidad que la arquitectura y las pruebas deben satisfacer.
  * **ISO/IEC 5055**: guía la medición automatizada de seguridad, confiabilidad, eficiencia y mantenibilidad mediante compilación estricta, análisis estático y pruebas proporcionales al proyecto.
  * Su aplicación se documenta en `docs/arquitectura.md` y `docs/auditoria-iso.md`; no se afirma certificación o conformidad formal sin una evaluación independiente y evidencia suficiente.
* El diseño objetivo utiliza `dist`, `src`, `tools`, `docs`, `tests` y `assets`. Las cinco primeras ya forman parte del proyecto; `assets` y su inventario visual deben añadirse cuando se normalicen los recursos de interfaz, sin simular que ya existen.
* **Normalización de UI y Paleta de Colores**: Todos los módulos de interfaz gráfica y renderizado visual (reproductor de video, visor de imágenes, interfaz de comprimidos y lector de Markdown) deben utilizar estrictamente la paleta de colores definida en `docs/paleta-colores.md`. Este archivo es la fuente canónica de verdad para normalización de color, tipografía, bordes, estados y comportamiento de interfaz en todo el proyecto.
* El sistema agéntico debe utilizar QA-Agent y Architect-Agent siempre que la plataforma permita su ejecución y exista una tarea concreta, acotada y delegable. Las restricciones superiores del sistema, seguridad y disponibilidad prevalecen.
* La carpeta raíz se mantiene limpia y contiene solo archivos generales o puntos de entrada necesarios, actualmente `.gitignore`, `README.md`, `AGENTS.md`, `Agents (main).md`, `build.cmd` y `build.ps1`, además de las carpetas de primer nivel.
* El proceso definido de generación utiliza `tools/build.ps1` y `tools/combiner/combine.py`; produce `dist/useful-sideview.wh.cpp`. Los lanzadores de la raíz solo delegan al flujo central y el artefacto de `dist` nunca se edita manualmente.
  * `esbuild` solo es la preferencia para proyectos web compatibles y no corresponde a este mod C++.
  * En Useful Side View el combinador de fuentes de Python es el bundler adecuado y el enlazador de Windhawk/LLVM valida el archivo final.
* Toda función nueva o corrección debe validarse mediante pruebas acordes a su alcance.
  * Antes de aplicar una solución se ejecuta una prueba que establezca la línea base o reproduzca el fallo.
  * Después de aplicarla se repite la prueba y se ejecutan verificaciones de regresión proporcionales al riesgo.
  * Una función solo se considera técnicamente terminada cuando pasa sus pruebas, el análisis aplicable, el bundling y, si corresponde, la compilación y el enlace completos.
  * Todo bug reproducible se registra en `docs/bugs-trace.md` bajo un folio único y, cuando sea viable, se acompaña de una prueba que demuestre su corrección y detecte regresiones.
* Todos los bugs, hallazgos, causas raíz, hipótesis descartadas y soluciones aplicadas deben documentarse. Una causa no demostrada se registra como `Por confirmar`; no se presenta una conjetura como causa raíz.
* No se repiten soluciones afines o ya aplicadas salvo que una diferencia verificable del contexto lo haga lógicamente necesario. La justificación y la evidencia previa deben registrarse antes de repetir el intento.
* Cada módulo dentro de `src` posee un nombre representativo y agrupa responsabilidades relacionadas de forma coherente. No se incorpora código generado, duplicado, inalcanzable o especulativo sin una necesidad demostrada.
* La ejecución de las normas ISO se reparte entre los agentes cuando hayan sido creados: Architect-Agent cubre arquitectura y atributos de calidad; QA-Agent cubre evidencia, métricas, pruebas y regresiones. La responsabilidad final de integrar y verificar permanece en el agente principal.
* Este documento se actualiza cuando el usuario establezca un comportamiento permanente. Las nuevas reglas se integran sin borrar secciones prioritarias ni contradecir instrucciones superiores.
* Cuando los tokens estén próximos a agotarse se crea o actualiza `docs/resume.md` con el punto exacto de continuación. Las acciones pausadas se marcan de manera segura en la documentación o, solo cuando aporte claridad y no rompa el código, con una referencia temporal localizada en el código.

## Normas ISO obligatorias adaptadas

Las normas se aplican como marcos de ingeniería proporcionales y no como una afirmación automática de certificación:

* **ISO/IEC/IEEE 42010 — arquitectura**:
  * Interesados: usuario del mod, mantenedor, responsable de calidad y responsable de distribución cuando estos roles existan.
  * Sistemas externos y entorno: Windhawk, Explorer, Windows, COM y Media Foundation. Se documentan como dependencias o contexto, no como personas interesadas.
  * Inquietudes: integración no invasiva, rendimiento, compatibilidad, liberación de recursos, mantenibilidad y continuidad del panel.
  * Vistas mínimas: módulos de `src`, integración con ventanas de Explorer, ciclo de vida multimedia, flujo de configuración y generación del bundle.
  * Cada vista declara su punto de vista: interesados e inquietudes atendidas, convenciones o modelos utilizados y correspondencias con otras vistas. Como mínimo se mantienen puntos de vista modular, de integración, de ejecución y de construcción/distribución.
  * Evidencia: `docs/arquitectura.md`, decisiones registradas y relaciones verificables entre módulos.
* **ISO/IEC 25010 — calidad del producto**:
  * Adecuación funcional: reproducción, controles, línea de tiempo, audio, repetición y configuración.
  * Eficiencia: trabajo bajo demanda, ausencia de dependencias pesadas, respuesta inmediata y liberación determinista de recursos.
  * Compatibilidad y confiabilidad: convivencia con Explorer, manejo de errores de COM/Media Foundation y recuperación ante cambios de selección.
  * Usabilidad y mantenibilidad: interfaz adaptativa, valores seguros por defecto, Clean Code en español y módulos cohesionados.
  * Evidencia: pruebas, mediciones proporcionales, `docs/auditoria-iso.md` y validación manual documentada.
* **ISO/IEC 5055 — calidad interna automatizada**:
  * Seguridad: límites de recursos, manejo de punteros, validación de handles y ausencia de operaciones destructivas implícitas.
  * Confiabilidad: estados consistentes, manejo de resultados `HRESULT`, liberación RAII y pruebas de regresión.
  * Eficiencia y mantenibilidad: compilación estricta, análisis estático, baja duplicación, complejidad controlada y dependencias comprensibles.
  * Evidencia: `tests/run-quality.ps1`, resultados de compilación/enlace y auditorías registradas.

Toda desviación conocida, evidencia faltante o validación que dependa del entorno real se documenta como limitación o insumo pendiente. No se emplea el nombre de una norma para ocultar una prueba inexistente.

## Subagentes programados

Siempre que el sistema agéntico permita esta funcionalidad y haya trabajo concreto que pueda ejecutarse de forma acotada deben utilizarse:

* QA-Agent
* Architect-Agent

El **QA-Agent** debe reproducir, documentar y validar bugs mediante pruebas. Debe consultar y mantener actualizado `docs/bugs-trace.md`, separar síntomas, hallazgos, hipótesis y causas raíz, ejecutar evidencia antes y después de cada solución y comprobar que la corrección no introduzca regresiones conocidas. Puede declarar una corrección como validada técnicamente, pero no cerrar definitivamente un bug sin confirmación del usuario.

El **Architect-Agent** debe revisar requerimientos, decisiones, dependencias y límites; diseñar o depurar módulos; validar su integración; y comprobar que `src` conserve coherencia sintáctica, semántica y arquitectónica. Debe relacionar cada requerimiento con su funcionalidad, módulo, riesgo y resultado esperado conforme a ISO/IEC/IEEE 42010 e ISO/IEC 25010. Si una validación falla por un defecto reproducible, el caso se traslada al QA-Agent para su seguimiento.

Los subagentes reciben tareas independientes y delimitadas, no editan simultáneamente las mismas líneas y comunican sus hallazgos al agente principal. El agente principal conserva la responsabilidad de integrar cambios, resolver contradicciones, aplicar la precedencia de este documento y efectuar la verificación final. Si la plataforma no permite subagentes o una tarea no puede delegarse de forma segura, se documenta esa condición y el agente principal ejecuta el flujo completo.

## Flujo típico de trabajo con agentes

1. **Continuidad**: se consulta `docs/resume.md` si existe y se revisan `docs/estado-proyecto.md`, `docs/registro-cambios.md` y la documentación relacionada con la solicitud.
2. **Clasificación**: se determina si la petición es una función, refactorización, documentación o bug. Si es un bug, se consulta o crea su folio en `docs/bugs-trace.md` antes de aplicar soluciones.
3. **Arquitectura**: Architect-Agent analiza requerimientos, módulos, interfaces, riesgos y atributos de calidad cuando la tarea es delegable; el agente principal valida la propuesta contra el código actual.
4. **Línea base**: QA-Agent o el agente principal ejecuta las pruebas previas pertinentes y registra los hallazgos. Para bugs se investiga primero la documentación técnica vigente aplicable.
5. **Implementación**: se modifica la fuente canónica en `src` con Clean Code en español. Se preservan cambios ajenos, se evita repetir soluciones fallidas y no se edita `dist`.
6. **Verificación**: se repiten las pruebas, se comprueban regresiones, se ejecuta `tests/run-quality.ps1` cuando el alcance lo amerita y se inspecciona el resultado del bundle y enlace.
7. **Distribución**: `tools/build.ps1` y `tools/combiner/combine.py` generan `dist/useful-sideview.wh.cpp`; el artefacto se entrega para su prueba manual en Windhawk.
8. **Trazabilidad**: se actualizan `docs/registro-cambios.md`, el folio de bug cuando corresponda, la arquitectura y cualquier manual afectado.
9. **Confirmación**: el agente informa el resultado técnico y las limitaciones. Solo el usuario declara definitivamente solucionado o cerrado un bug después de validarlo en su entorno.

## Diseño del root (carpeta raíz)

La estructura definida en esta sección es obligatoria. Debe desplegarse de forma proporcional y verificable, integrándose con el estado existente sin crear carpetas vacías, documentos duplicados ni herramientas simuladas. El estado actual y el diseño objetivo son:

```text
windhawk-useful-sideview/
|-- .gitignore
|-- README.md
|-- AGENTS.md
|-- Agents (main).md
|-- build.cmd
|-- build.ps1
|-- assets/                         # objetivo pendiente de normalización visual
|   `-- design-list.md
|-- dist/
|   `-- useful-sideview.wh.cpp      # artefacto generado, solo lectura
|-- docs/                           # documentación y trazabilidad local
|-- src/
|   |-- archivos/
|   |-- ejecucion/
|   |-- integracion/
|   |-- interfaz/
|   |-- nucleo/
|   |-- reproductor/
|   `-- windhawk/
|-- tests/
`-- tools/
    |-- build.ps1
    |-- combiner/combine.py
    `-- deployment-manager         # obligación pendiente; coordinará el flujo existente
```

### assets

La carpeta `assets` debe contener los recursos estáticos utilizados por el proyecto, como íconos, imágenes, multimedia y sonidos. Actualmente los controles se dibujan mediante código nativo y la carpeta todavía no existe; debe crearse cuando se incorpore o normalice el inventario visual, sin mover allí archivos fuente ni generar recursos artificiales.

Como el mod tiene interfaz gráfica, debe incluir `design-list.md`.

`design-list.md` debe documentar de forma exacta las paletas de colores, estados visuales, iconografía, dimensiones, espaciado, contraste, adaptación al redimensionado y criterios de estilo utilizados en el reproductor. Será la especificación canónica del diseño vigente; `docs/cambios_ui.md` conservará únicamente el historial, las razones y la transición entre diseños. Ambos se relacionan con la implementación sin duplicar la misma fuente de verdad. La explicación debe ser pedagógica y distinguir el diseño esperado de las limitaciones de Win32 y del panel anfitrión.

### dist

La carpeta `dist` contiene exclusivamente el artefacto generado para distribución en Windhawk: `useful-sideview.wh.cpp`.

Todo proceso de compilación o bundling genera sus artefactos finales dentro de esta carpeta. `dist` se considera de solo lectura: nunca se corrige manualmente; cualquier cambio debe realizarse en `src` o en el combinador y volver a generarse.

### src

La carpeta `src` debe contener todo el código fuente y la estructura modular necesaria para el proyecto.

La implementación se organiza mediante módulos con responsabilidades claramente identificables:

* `archivos`: detección y validación del elemento seleccionado en Explorer.
* `ejecucion`: hilo y ciclo operativo del reproductor.
* `integracion`: descubrimiento y acoplamiento seguro con el panel de vista previa.
* `interfaz`: ventanas, distribución adaptativa, controles y renderizado.
* `nucleo`: configuración, constantes, estado compartido y utilidades de recursos.
* `reproductor`: Media Foundation/MFPlay, eventos, tiempo y acciones multimedia.
* `windhawk`: ciclo de vida, configuración y puntos de entrada exigidos por Windhawk.

`src/main.wh.cpp` y `src/useful-sideview.cpp` solo pueden actuar como composición o entrada cuando el diseño lo requiera; no deben volver a concentrar responsabilidades ya modularizadas. Las dependencias entre módulos deben ser explícitas, acíclicas cuando sea posible y coherentes con `docs/arquitectura.md`.

### tools

La carpeta `tools` contiene los scripts auxiliares de desarrollo, mantenimiento, automatización y despliegue. El flujo vigente se apoya en `tools/build.ps1` y `tools/combiner/combine.py`; `tests/run-quality.ps1` coordina pruebas, advertencias estrictas, análisis estático, bundling y validación completa de enlace.

Debe incluir obligatoriamente `deployment-manager`, que será el script central para administrar los procesos de pruebas, compilación, bundling, generación de `dist/useful-sideview.wh.cpp` y manejo de versiones del mod. Esta herramienta aún no está implementada y se registra como normalización pendiente; hasta entonces debe reutilizar y coordinar los scripts existentes, no duplicar su lógica.

`deployment-manager` debe funcionar mediante una interfaz de consola totalmente interactiva, pedagógica y simple, compatible con el entorno PowerShell actual. Cada operación debe indicar qué acción se realizará, qué archivos o etapas serán afectados y cuál fue el resultado. Los mensajes utilizan lenguaje directo y comprensible, sin exigir que el usuario conozca los parámetros internos de Python, Clang/LLVM, el SDK de Windows o Windhawk.

Debe contar obligatoriamente con un menú interactivo desde el cual puedan ejecutarse las operaciones disponibles. El menú debe organizar las acciones de forma clara y permitir regresar, cancelar o salir sin ejecutar operaciones innecesarias.

`deployment-manager` debe:

* Detectar y administrar Python, el combinador de fuentes, Clang/LLVM, el SDK de Windows y las herramientas de Windhawk utilizadas por el proyecto.
* Ejecutar el flujo completo de pruebas, análisis, bundling, compilación y enlace correspondiente.
* Generar y administrar exclusivamente `dist/useful-sideview.wh.cpp` y cualquier artefacto futuro documentado.
* Consultar, mostrar y actualizar la versión actual del proyecto.
* Mantener coherencia entre la versión declarada y los artefactos generados.
* Mostrar de forma pedagógica cada etapa relevante del proceso y su resultado.
* Validar requisitos, dependencias, fuentes modulares, orden de combinación, archivos y condiciones necesarias antes de iniciar una operación.
* Detener inmediatamente un flujo cuando una validación obligatoria falle.
* Incorporar candados de seguridad para impedir operaciones destructivas, inconsistentes, accidentales o ejecutadas sobre condiciones inválidas.
* Solicitar confirmación explícita antes de realizar operaciones destructivas, sobrescribir fuentes o documentación, eliminar datos, cambiar versiones, generar releases o alterar elementos no regenerables. La sustitución atómica de `dist/useful-sideview.wh.cpp` por el bundle temporal validado forma parte normal del build automatizado y no requiere una confirmación adicional; si la validación falla, el artefacto válido anterior debe conservarse.
* Evitar incrementos de versión, generación de releases o artefactos finales cuando el proceso de empaquetado o sus validaciones hayan fallado.
* Informar claramente cualquier error y conservar la información necesaria para diagnosticarlo.
* Finalizar cada flujo indicando de forma inequívoca si la operación fue completada, cancelada o fallida.

La interacción debe priorizar **simplicidad, pedagogía y seguridad**. El usuario debe poder controlar el flujo completo de calidad, bundling y versiones desde `deployment-manager` sin ejecutar manualmente los comandos internos. La selección de una acción nunca debe modificar `src`, borrar documentación ni sobrescribir un artefacto válido después de un fallo.

La regla central es: **una interfaz simple controla el proceso; las validaciones y candados protegen el proyecto; `deployment-manager` coordina las pruebas, el combinador, el compilador, el enlace y las versiones**.


### docs

La carpeta `docs` contiene la documentación de trabajo y es la primera fuente local para bugs, decisiones, arquitectura, compilación y continuidad. Tanto `docs/` como `AGENTS.md` permanecen ignorados por `.gitignore` por instrucción del usuario; esta condición no reduce su obligatoriedad ni autoriza omitir actualizaciones.

La cobertura documental obligatoria del main es el diseño objetivo. El proyecto presenta una **cobertura parcial y una no conformidad pendiente** mientras falten los documentos y herramientas señalados en la tabla. Los archivos actuales aportan cobertura provisional, pero no convierten automáticamente un requisito pendiente en cumplido. No se crean documentos duplicados para el mismo propósito: cuando se normalice un nombre, se migra su contenido y se actualizan todas las referencias en una sola operación trazable.

| Documento requerido por el main | Archivo canónico actual o estado | Cobertura obligatoria |
| --- | --- | --- |
| `manual-desarrollo.md` | `arquitectura.md` + `comandos-build.md` | Técnicas, decisiones, procedimientos y compilación. Puede consolidarse en el futuro sin perder información. |
| `manual-usuario.md` | Pendiente | Instalación en Windhawk, configuración, uso del panel y solución básica de problemas. |
| `requerimientos.md` | Pendiente | Requerimientos funcionales, no funcionales e interfaces externas. |
| `diseño-proyecto.md` | `arquitectura.md` + `cambios_ui.md` | Diseño técnico, funcional y visual. |
| `bug-trace.md` | `bugs-trace.md` | Registro histórico único de bugs, hallazgos, causas y soluciones. El nombre plural es el canónico del proyecto. |
| `legal.md` | Pendiente | Licencias, distribución, dependencias y marco jurídico aplicable. |
| `insumos-pendientes.md` | Pendiente | Recursos, decisiones o evidencia externa faltante. |
| `stack.md` | `tecnologias.md` | Tecnologías, APIs, herramientas, instalación y recomendaciones. |
| `general-log.md` | `registro-cambios.md` | Registro cronológico numerado de cambios y decisiones. |
| `factibilidad.md` | Pendiente | Factibilidad técnica, operativa y económica proporcional al mod. |
| `alcances.md` | Pendiente; cobertura inicial en este documento y `estado-proyecto.md` | Contexto, actores, límites, alcance y trazabilidad de requerimientos. |
| `resume.md` | Pendiente; se crea cuando exista una pausa real que transferir | Punto exacto de continuación y peticiones pendientes del usuario. |

Además, el proyecto conserva documentos especializados que no sustituyen la cobertura anterior:

* `auditoria-iso.md`: aplicación proporcionada y evidencia de ISO/IEC/IEEE 42010, ISO/IEC 25010 e ISO/IEC 5055.
* `estado-proyecto.md`: fotografía verificable del estado actual, riesgos y pendientes.
* `cambios_ui.md`: historial de decisiones y cambios visibles del reproductor.

En `alcances.md` debe analizarse el contexto de Useful Side View, el problema, los actores, las actividades principales, los límites y el alcance funcional. La especificación mantiene trazabilidad entre necesidades, alcance y requerimientos, utilizando IEEE 830 o un criterio moderno equivalente como referencia. Como `src` ya existe, este archivo debe validar y orientar su evolución, no justificar una reescritura automática.

La cobertura de `manual-desarrollo.md`, hoy repartida entre `arquitectura.md` y `comandos-build.md`, documenta técnicas, decisiones, estrategias y procedimientos relevantes del desarrollo nativo, el bundling y las verificaciones.

En `manual-usuario.md` debe explicarse cómo instalar `dist/useful-sideview.wh.cpp` como mod local de Windhawk, abrir el panel con `Alt+P`, configurar autorreproducción y audio, utilizar los controles y reconocer limitaciones de códecs desde la perspectiva del usuario final.

En `requerimientos.md` deben documentarse los requerimientos funcionales, no funcionales y de interfaces externas de Windhawk, Explorer, Win32, COM y Media Foundation, con identificadores trazables a módulos y pruebas.

La cobertura de `diseño-proyecto.md`, hoy en `arquitectura.md` y `cambios_ui.md`, documenta la estructura, módulos, responsabilidades, relaciones, flujo general, estados visuales y decisiones de diseño.

En `factibilidad.md` debe analizarse de forma proporcional la factibilidad técnica, operativa y económica del mod, incluyendo compatibilidad con Windows 11, Windhawk y los códecs disponibles.

En `legal.md` deben analizarse las implicaciones legales relevantes del producto, incluyendo licencias del código y herramientas, distribución, monetización, cobros, suscripciones y sistemas premium/pro si llegaran a existir. El análisis se enfoca en la aplicación jurídica y fiscal correspondiente, con atención al derecho mexicano cuando sea el marco aplicable; no sustituye asesoría profesional.

En `insumos-pendientes.md` deben registrarse recursos, datos, decisiones, credenciales, archivos de prueba, versiones de Windows, trazas o elementos externos que todavía sean necesarios. Si el usuario no aporta un insumo indispensable, la función afectada permanece como diseño o trabajo pendiente y no se simula como terminada.

La cobertura de `stack.md` corresponde a `tecnologias.md`, donde se documentan C++, Win32, COM, Media Foundation/MFPlay, Windhawk, PowerShell, Python, Clang/LLVM, Windows SDK, patrones y herramientas realmente utilizados. Debe incluir instalación, comandos, requisitos de ejecución, despliegue y recomendaciones del entorno.

La cobertura de `general-log.md` corresponde a `registro-cambios.md`, que mantiene en tabla Markdown un registro cronológico, numerado y pedagógico de cambios, decisiones y acontecimientos relevantes, incluyendo un ejemplo de lo realizado. Su esquema actual de cuatro columnas es legado y debe migrarse de forma acumulativa para añadir `Verificación` y `Resultado`; hasta esa migración, ambos datos se escriben explícitamente dentro de `Explicación pedagógica` o `Ejemplo`.

En `resume.md` se documenta lo que estaba en curso antes de una pausa por agotamiento de contexto o transferencia de trabajo. Cuando exista, siempre se lee antes de continuar. Utiliza una tabla Markdown con folio, fecha y hora de pausa, fecha y hora de reanudación o resumen, acciones pausadas, ubicación exacta para continuar y peticiones del usuario. La columna de peticiones conserva el placeholder `(ingresa tu petición aquí)`. No se crea ni modifica `resume.md` si no existe una pausa real; antes de pausar se deja una referencia segura y localizable, preferentemente documental.

El archivo canónico `docs/bugs-trace.md` funciona como registro histórico obligatorio de todos los bugs detectados durante el desarrollo. Se mantiene en una **tabla Markdown**. Cada defecto se registra una sola vez con un **folio único y consecutivo**; si persiste después de una solución, las nuevas hipótesis, pruebas, cambios y resultados se acumulan en ese mismo folio.

La tabla debe contener como mínimo las siguientes columnas:

| Campo | Contenido |
| --- | --- |
| Folio | Identificador único y consecutivo del bug. |
| Estado | `Abierto`, `En diagnóstico`, `Causa raíz identificada`, `En validación`, `Reabierto`, `Resuelto` o `Cerrado`, respetando la autoridad del usuario para los dos últimos. |
| Área | Módulo, componente o integración afectada. |
| Título | Nombre breve, representativo y distinto del síntoma genérico. |
| Descripción / reproducción | Contexto, precondiciones y pasos mínimos para observar el bug; puede incluir mensajes de error o fragmentos breves. |
| Sintomatología | Comportamientos, errores o efectos observables, sin presentarlos como causa. |
| Hallazgos | Evidencia observada o medida: resultados de pruebas, trazas, estados, documentación oficial o comportamiento reproducible. |
| Causa raíz | Mecanismo causal demostrado. Mientras no exista evidencia suficiente se escribe `Por confirmar`. |
| Prueba previa | Línea base ejecutada antes de la solución, con resultado y evidencia. |
| Soluciones aplicadas | Historial cronológico acumulativo de cada solución o prueba, decisión técnica y resultado. |
| Prueba posterior / verificación | Evidencia ejecutada después de cada cambio y comprobaciones de regresión. |
| Solución definitiva | Única solución reconocida como definitiva después de la confirmación expresa del usuario. |

El esquema actual de `docs/bugs-trace.md` es legado y todavía no contiene todas estas columnas. Debe migrarse de forma acumulativa, conservando literalmente cada folio, hallazgo e intento existente. Los estados `Resuelto` registrados antes de esta política se consideran históricos; cuando falte confirmación expresa del usuario o exista validación visual pendiente, se interpretan y migran a `En validación` en lugar de usarse como evidencia de cierre definitivo.

* Cada bug conserva el mismo folio mientras corresponda al mismo defecto y mecanismo causal investigado.
* Hallazgos y causa raíz se documentan por separado. Un hallazgo es evidencia; una causa raíz explica de forma verificable por qué ocurre el defecto.
* Si la causa raíz aún no está demostrada, se registra `Por confirmar`, el estado permanece abierto o en diagnóstico y se conservan las hipótesis descartadas.
* Las soluciones aplicadas forman un historial acumulativo. Nunca se borra un intento fallido ni se reescribe el registro para que parezca exitoso.
* Una solución descartada o afín a otra ya probada no se repite bajo las mismas condiciones sin una diferencia lógica verificable y una justificación técnica documentada antes del reintento.
* Si un bug reaparece por la misma causa se reabre el folio. Si la evidencia demuestra una causa distinta, se crea un folio nuevo y se relacionan ambos registros.
* Si una solución falla, su resultado se registra antes de intentar una alternativa.
* Cada intento aporta una hipótesis, prueba o solución distinta, o explica técnicamente por qué es necesario repetir una prueba anterior.
* Antes de cada solución se ejecuta y registra una prueba previa; después se repite la validación y se comprueban regresiones según el riesgo.
* El proceso continúa hasta identificar y corregir la causa raíz o documentar explícitamente la evidencia disponible, las limitaciones y por qué no pudo determinarse.
* El QA-Agent mantiene y actualiza `docs/bugs-trace.md` cuando esté disponible; el agente principal verifica la integración y asegura la trazabilidad cuando no lo esté.
* Antes de aplicar una solución se consulta el historial del folio y se investiga documentación vigente. Para este mod se priorizan fuentes primarias y oficiales como Microsoft Learn, Windows SDK, Media Foundation, Win32 y la documentación o código de Windhawk; foros y reportes comunitarios solo aportan evidencia secundaria. La búsqueda debe ser pertinente al fallo y quedar citada cuando influya en la decisión.
* Las hipótesis locales se basan en el estado actual del proyecto, el código fuente, el artefacto generado, las pruebas y el contexto vigente. Si la red no está disponible o el defecto es exclusivamente interno, esa limitación se documenta y se continúa con evidencia local reproducible.
* El agente puede registrar `Causa raíz identificada` o `En validación` cuando exista evidencia técnica, pero no puede marcar por sí solo el bug como finalizado, cerrado, arreglado o definitivamente resuelto.
* La solución definitiva se establece únicamente cuando el usuario confirma que el bug quedó solucionado y solicita cerrarlo. Entonces se registra qué parche, acción o código resolvió el caso, dónde se aplicó y con qué prueba se verificó. Solo existe una solución definitiva vigente; si cambia, la anterior pasa al final de `Soluciones aplicadas` y la columna conserva únicamente la nueva decisión confirmada.

La regla central es: **un defecto identificado = un folio = un historial acumulativo**.

### tests

La carpeta `tests` contiene las pruebas necesarias para validar funcionalidades, correcciones y riesgos relevantes. Actualmente incluye pruebas del cálculo y navegación temporal en `barra_progreso_tiempo_test.cpp`, el ejecutor rápido `run-tests.ps1` y la validación integral `run-quality.ps1`.

El QA-Agent trabaja principalmente sobre esta carpeta y relaciona cada prueba de regresión con el folio correspondiente de `docs/bugs-trace.md`.

Toda funcionalidad nueva debe contar con pruebas correspondientes antes de considerarse técnicamente terminada. Toda corrección de un bug reproducible incorpora, cuando sea viable, una prueba capaz de demostrar el defecto antes del cambio, comprobar la corrección después y detectar su reaparición. Los aspectos que solo puedan validarse dentro de Explorer se acompañan de una guía manual precisa y de las comprobaciones automatizadas posibles sobre su lógica aislada.

La validación proporcional completa puede incluir:

1. Prueba previa o reproducción controlada.
2. Pruebas unitarias mediante `tests/run-tests.ps1`.
3. Compilación con advertencias estrictas y análisis estático.
4. Generación reproducible del bundle.
5. Compilación y enlace del archivo final con las bibliotecas de Windows requeridas.
6. Prueba manual en Windhawk y Explorer cuando el comportamiento dependa de ventanas reales, selección de archivos, audio, video o interacción del usuario.

Esta obligación de trazabilidad no admite excepciones por el tamaño o la aparente importancia del cambio; solo cambia la profundidad de la prueba según el riesgo.

### Trazabilidad obligatoria del desarrollo

Todo cambio realizado en el proyecto debe quedar documentado obligatoriamente.

Cada nueva función, modificación, corrección, refactorización, cambio de comportamiento o actualización permanente de estas reglas se registra en `docs/registro-cambios.md`, equivalente canónico actual de `general-log.md`. El registro utiliza una tabla Markdown, folio consecutivo, explicación pedagógica y ejemplo; indica qué se modificó, por qué, cómo se verificó y cuál fue el resultado. Ningún cambio implementado puede omitirse.

Todo bug detectado se registra obligatoriamente en `docs/bugs-trace.md`. Identificar la causa raíz no termina su seguimiento: el folio continúa activo hasta que el usuario confirme la resolución. Si el defecto no puede corregirse, permanece abierto con la limitación técnica y la evidencia documentadas explícitamente.

El registro y seguimiento en `docs/bugs-trace.md` es obligatorio, sin excepciones ni sustitución por comentarios, mensajes de commit, conversaciones, notas temporales u otros medios. Estos elementos pueden enlazarse como evidencia, pero no reemplazan el folio.

Cuando el QA-Agent esté disponible, asume la reproducción, atención y seguimiento del bug. Registra sintomatología, hallazgos, causa raíz o estado `Por confirmar`, cada solución aplicada y su resultado; además valida las correcciones y comprueba posibles regresiones. El agente principal revisa e integra su trabajo.

Cada intento sobre un bug actualiza su mismo folio antes de ejecutar una alternativa. Un bug no puede abandonarse ni ignorarse. El agente tampoco puede declararlo definitivamente resuelto o cerrado: puede dejar evidencia técnica de validación y debe esperar la confirmación del usuario.

La trazabilidad mínima obligatoria del proyecto queda definida de esta forma:

* **Nueva función o cambio → `docs/registro-cambios.md`**
* **Bug detectado → nuevo folio en `docs/bugs-trace.md`**
* **Hallazgo o causa raíz → actualización separada dentro del mismo folio**
* **Bug con QA-Agent disponible → seguimiento obligatorio por QA-Agent y revisión del agente principal**
* **Corrección aplicada → prueba previa + actualización de `bugs-trace.md` + cambio en código fuente**
* **Corrección verificada → prueba posterior/regresión + actualización de `bugs-trace.md` y `registro-cambios.md`**
* **Bundle generado → comprobación de que `dist/useful-sideview.wh.cpp` proviene de `src` y del combinador**
* **Cambio validado → registro del resultado técnico; el cierre definitivo del bug corresponde al usuario**

Esta trazabilidad vincula requerimientos, arquitectura, código fuente, pruebas, artefacto y documentación. Cualquier discrepancia se corrige en la fuente canónica correspondiente, evitando mantener dos versiones contradictorias de una misma decisión.
