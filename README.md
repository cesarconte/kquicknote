# KQuickNote — Nota Rápida para KDE Plasma 6.8 + Wayland

**KQuickNote** ofrece una experiencia ágil de notas rápidas de forma nativa en **KDE Plasma 6.8 sobre Wayland** con diseño **Kirigami / Breeze** e integración directa con el índice de conocimiento local (**RAG**) de **Kiri**.

---

## Características principales

- **Ventana nativa KWin Breeze (`Kirigami.ApplicationWindow`)**:
  - Decoraciones y barra de título nativas del sistema, respetando los botones y esquemas configurados en Preferencias del Sistema.
- **Guardado automático en Markdown (`~/Documentos/QuickNotes/`)**:
  - Cada nota se guarda instantáneamente como `<id>.md` mientras escribes.
  - El título de la nota se extrae automáticamente del primer encabezado `# Título` o primera línea con texto.
  - Selector rápido de notas recientes, creación de nuevas notas y eliminación segura.
- **Captura de selección / portapapeles Wayland como cita (`> ...`)**:
  - Captura la selección primaria de Wayland o el portapapeles (`QClipboard` / `wl-paste`) y la añade al documento actual formateada como bloque de cita Markdown.
- **Sincronización en un clic con Kiri Local RAG**:
  - Exporta la nota activa a `~/.local/share/kiri/knowledge/quicknote-<id>.md` para que el asistente local **Kiri** pueda consultarla inmediatamente.
- **Atajos globales (`KF6::GlobalAccel`) y servicio D-Bus (`org.cesar.kquicknote`)**:
  - `Meta+Alt+N`: Mostrar / ocultar la Nota Rápida.
  - `Meta+Shift+N`: Crear nueva nota rápida capturando la selección actual.
  - Interfaz D-Bus en `org.cesar.kquicknote` (`/QuickNote`) con métodos `Toggle()`, `Show()`, `Hide()` y `NewNoteWithSelection()`.

---

## Requisitos

- KDE Plasma 6.8+ (Wayland)
- Qt 6.7+ (`Core`, `Gui`, `Widgets`, `Qml`, `Quick`, `QuickControls2`, `DBus`, `Test`)
- KDE Frameworks 6 (`ECM`, `CoreAddons`, `I18n`, `GlobalAccel`, `Kirigami`)
- Opcional: `wl-clipboard` (`wl-paste`) como respaldo para captura de selección en Wayland.

---

## Compilación e instalación

```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=$HOME/.local -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
cmake --install build
kbuildsycoca6 >/dev/null 2>&1
```

---

## Uso

1. **Lanzar la aplicación**:
   ```bash
   ~/.local/bin/kquicknote &
   ```
   Si `kquicknote` ya está en ejecución, volver a ejecutar `kquicknote` invoca `Toggle()` automáticamente por D-Bus.
2. **Invocar mediante D-Bus**:
   ```bash
   qdbus6 org.cesar.kquicknote /QuickNote org.cesar.kquicknote.Toggle
   qdbus6 org.cesar.kquicknote /QuickNote org.cesar.kquicknote.NewNoteWithSelection
   ```
