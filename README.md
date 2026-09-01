# Useful Side View

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

---

## 🚀 Instalación y Compilación

1. Compila el mod empaquetado ejecutando:
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\tools\deployment-manager.ps1 build
   ```
2. Ejecuta la puerta de calidad estricta:
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\tools\deployment-manager.ps1 quality
   ```
3. Carga el archivo generado en `dist/useful-sideview.wh.cpp` como mod local en Windhawk.
4. Abre el Explorador de Archivos de Windows y activa el panel lateral con `Alt + P`.

