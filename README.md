# Useful Side View 4.1.3

**Useful Side View** es un mod para [Windhawk](https://windhawk.net/) que transforma el panel de vista previa de Windows 11 (`Alt + P`) en una suite multimedia integral, ultrarrápida y responsiva para videos, imágenes, cómics, archivos comprimidos y documentos Markdown.

---

## ✨ Principales Virtudes y Capacidades

### 🎬 1. Reproductor de Video Fluido e Interactivo
- **Compatibilidad Extensa**: Soporte nativo para `.mp4`, `.mkv`, `.avi`, `.mov`, `.wmv`, `.webm`, `.flv`, `.m4v`, `.ts`, `.mts` mediante Media Foundation.
- **Controles Fluent Design**: Play/Pausa (clic en video o botón), activación/desactivación de audio (mute), repetición en bucle (*loop*) y selector de velocidad (0.5x a 2.0x).
- **Línea de Tiempo con Miniatura**: Barra de progreso interactiva con búsqueda por arrastre, marcas de tiempo precisas y atajos de teclado completos (Espacio, Flechas, Inicio, Fin, M, L, P, N).
- **Navegación de Carpeta**: Salto inmediato al video anterior o siguiente.

### 🖼️ 2. Visor de Imágenes de Amplio Espectro y GIFs en Loop
- **Formatos Modernos y Tradicionales**: Soporta `.png`, `.jpg`, `.jpeg`, `.webp`, `.gif`, `.bmp`, `.tiff`, `.tif`, `.avif`, `.heic`, `.jfif` e `.ico`.
- **Reproducción Infinita de GIFs**: Animación automática en bucle continuo respetando los retardos de cuadro nativos (`PropertyTagFrameDelay`).
- **Navegación Intuitiva**: Botones de imagen anterior/siguiente integrados y atajos de teclado.

### 📚 3. Suite Integral para Cómics y Archivos Comprimidos
- **Formatos Compatibles**: Cómics (`.cbz`, `.cbr`) y archivos comprimidos (`.zip`, `.rar`, `.7z`, `.tar`, `.iso`).
- **Extracción Inteligente de Portadas**: Obtención de miniaturas de Explorer y descompresión interna Deflate para mostrar portadas en alta definición.
- **Cuadrícula Adaptativa con Contador `+N`**: Visualización de selecciones múltiples en cuadrículas configurables de 4, 6, 8 o 10 archivos con insignia flotante para elementos excedentes.
- **Selector 'Abrir con' Inteligente y Apertura Secuencial**:
  - Discriminación inteligente: activa lectores de cómics si hay cómics presentes o descompresores comunes si solo hay comprimidos normales.
  - Apertura en orden cronológico de toda la selección con la aplicación elegida.
- **Conversor y Registro en Tiempo Real**: Menú de conversión entre formatos y recuadro de historial (*log*) scrolleable.

### 📝 4. Visor de Documentos Markdown Estilo GitHub Dark
- **Sintaxis Estándar y Estética GitHub**: Renderizado fiel de encabezados (H1..H4), citas, listas, bloques de código sombreados, casillas interactivas y pie de estado con conteo de líneas.
- **Índice Vertical de Barras Interactivo**: Barra lateral multinivel que despliega títulos flotantes al pasar el cursor y salta instantáneamente al hacer clic.
- **Multimedia y Enlaces Web**: Soporte de imágenes locales y en línea, tarjetas interactivas de YouTube con miniaturas y botón Play, e hipervínculos clicables en el navegador.

### ⚙️ 5. Personalización y Rendimiento Nativo
- **Redimensionamiento Reactivo**: Adaptación fluida ante cualquier cambio de tamaño del panel de Explorer.
- **Bloqueo Granular de Tamaño**: Opción de fijar el tamaño del visor individualmente por tipo de contenido.
- **Cero Fugas y Máxima Eficiencia**: Implementado en C++23 nativo con gestión segura de memoria y recursos GDI/COM.

### 🧪 6. Comparador de videos experimental
- Disponible desde la vista general de carpetas mediante un activador en Ajustes.
- Usa dos instancias independientes del reproductor Media Foundation, con layout 50/50, línea de tiempo y controles sincronizados tras confirmación de duración.
- Permite pantalla completa temporal con PiP fijo; al pulsar el PiP se intercambian los roles sin pausar la reproducción.
- Los errores de formato o archivos corruptos se muestran dentro del espacio reservado, sin reiniciar Explorer.
- El backend permanece aislado en `ServicioReproductorVideoV2` y `ServicioComparadorVideosV2` para poder sustituirlo o retirarlo sin afectar las demás vistas.

### 🔎 7. Comparadores generales y persistencia nativa
- Dos archivos del mismo tipo activan el comparador correspondiente: general,
  texto/diff, documentos o video.
- El comparador general muestra nombres, peso y SHA-256; texto y documentos
  admitidos (PDF, Word y PowerPoint) usan vista lado a lado, sliders individuales,
  slider sincronizado limitado por el archivo más corto y resumen de tres columnas.
- El botón **Diff View** expande temporalmente el comparador y ofrece diff clásico
  o modo semántico didáctico, alternable desde Ajustes.
- Los programas elegidos en “Abrir con” se guardan con la persistencia nativa
  de Windhawk por categoría, sobreviven a reinicios de Explorer y se migran
  desde el formato anterior sin perder datos.
- La opción **Mostrar versión del mod** expone la versión actual del mod en la
  vista general de carpetas cuando está activada.
- El ciclo de vida de los comparadores desmonta reproductores, callbacks,
  timers y capturas al cerrar, cambiar de vista, contraer el panel o descargar
  el mod, evitando dejar archivos en uso.
- Compatibilidad cooperativa con `gpu-accelerated-explorer-view`: el panel se
  superpone sobre su renderer GPU sin redimensionar ni interferir con su
  superficie DirectComposition; la selección y navegación continúan usando el
  contrato Shell nativo.

### 🗒️ 8. Toolbox de notas y compatibilidad WinKlok
- La vista general de carpetas incluye notas persistentes con título, contenido, edición, copia y eliminación.
- Las notas propias se guardan en el almacenamiento local nativo del mod mediante fragmentos acotados para no perder cuerpos largos.
- La sincronización con WinKlok es experimental, está desactivada por defecto y se
  puede retirar de forma aislada; mientras está apagada, las notas son atómicas de
  Useful Side View. Al activarla se lee y escribe el formato real de `noteIds`,
  `noteNext` y `note<ID>` y cada nota conserva su origen.
- **Forzar sincronización** sólo se habilita con la opción experimental activa y
  permite reconciliar ambos almacenes de forma explícita.

---

## 🚀 Instalación y Compilación

1. **Compilación en 1 solo clic (Recomendado)**:
   - Ejecuta `.\compilador.ps1` en PowerShell.
   - Se abrirá el menú guiado para versionar, integrar, compilar, inyectar y sincronizar Git.
   - El único artefacto final se genera en `dist/useful-sideview.wh.cpp`.
   - Los errores y operaciones quedan registrados en `dist/compilador.log`.

2. **Herramientas de despliegue y calidad (Avanzado)**:
   - `compilador.ps1` incorpora las validaciones de versión, bundle, inyección y Git; acepta un mensaje personalizado o uno genérico para patch, minor y major.

3. **Carga en Windhawk**:
   - Carga el archivo generado en [`dist/useful-sideview.wh.cpp`](file:///D:/Development/windhawk-development/windhawk-useful-sideview/dist/useful-sideview.wh.cpp) como mod local en Windhawk.
   - Abre el Explorador de Archivos de Windows y activa el panel lateral con `Alt + P`.


