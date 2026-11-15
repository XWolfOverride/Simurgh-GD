# Migración de Fenix a Simurgh

Documento vivo. Recoge **todo cambio del lenguaje o de su biblioteca estándar** respecto a Fenix 0.92a, para que quien venga de Fenix (o de DIV) sepa qué cambia en sus programas. Regla de trabajo: el lenguaje casi no se toca, para respetar la filosofía de Fenix y reaprovechar a sus programadores; lo que sí cambia se apunta aquí **en el mismo momento** en que se decide.

No se documentan aquí los cambios internos (formato `.dcb`, estructuras del compilador, intérprete): ver `ARQUITECTURA.md`.

## Resumen

| Área | Fenix 0.92a | Simurgh | Estado |
|---|---|---|---|
| Profundidad de color | 8, 16 y 32 bits, con paletas | solo 16 bits (RGB565) | decidido |
| Paletas | `load_pal`, `set_palette`, `fade`… sobre paleta | desaparecen | decidido |
| Negro puro | transparente en 16 bits | transparente por ahora | por resolver antes de la primera versión |
| Formatos de imagen | PNG, PCX, GIF… | PNG integrado; sin GIF | decidido |
| FPG y MAP | bibliotecas externas o funciones sueltas | integrados en el lenguaje | por diseñar |
| Textos | ISO-8859 / código de página | UTF-8 | decidido |
| Punteros en 64 bits | no soportado (la VM guarda punteros en 32 bits) | por resolver antes de la primera versión | pendiente |

## Sin cambios (compatibilidad)

- Sintaxis del lenguaje: procesos, `BEGIN`/`END`, `FRAME`, `CLONE`, `STRUCT`, `SWITCH`, macros, constantes, etc.
- Formatos **FPG** y **MAP**: 100 % compatibles con Fenix. Un cambio de formato sería un formato nuevo, con otro nombre.
- El compilador acepta los programas de Fenix 0.92a que no usan lo listado como cambiado (comprobado con `tools/fxc-compare.sh`, que compara contra `upstream/fenix-0.92a-fixed`).

## Cambios del lenguaje

Cada entrada: qué era, qué es ahora, cómo migrar.

### Color: solo 16 bits
- **Antes:** `set_mode` admitía 8, 16 y 32 bits; con 8 bits había paleta.
- **Ahora:** solo 16 bits. Los programas de 8 bits deben convertir sus gráficos (herramienta de conversión prevista).
- **Migrar:** quitar llamadas a funciones de paleta; los FPG/MAP de 8 bits se convierten a 16.

### Imágenes
- PNG va integrado; GIF desaparece. **Migrar:** convertir los GIF a PNG.
- Las bibliotecas externas de imagen (`Import "image"` y similares) dejan de hacer falta.

### Texto
- Las cadenas son UTF-8. **Migrar:** guardar los fuentes `.prg` en UTF-8; las fuentes bitmap (`.fnt`) se revisarán aparte.

### Funciones que no existen en Fenix 0.92a
- `LOAD_IMAGE(fichero)`: nueva; detecta el formato por la cabecera (MAP, PNG, PCX). **`LOAD_MAP`, `LOAD_PNG`, `LOAD_PCX` y `LOAD_FBM` son alias** de `LOAD_IMAGE`: el código antiguo compila igual. `Import "image"` se acepta y no hace nada.
- `SAVE_MAP(fpg, grafico, fichero)` y `SAVE_FPG(fpg, fichero)`: nuevas (no existían en 0.92a; las usa FenXEdit2). Graban en los formatos MAP y FPG de Fenix.
- Detalle y lista de formatos en `FORMATOS-IMAGEN.md`.

### Formatos y funciones que desaparecen
- **FBM** y **FPL**: se retiran `LOAD_FBM`, `SAVE_FBM`, `UNLOAD_FBM`, `LOAD_FPL` y `SAVE_FPL` (un programa que las use dejará de compilar). Para migrar: abrir el FBM y regrabarlo como MAP (si es un gráfico simple).
- Nuevos formatos de **MAP y FPG de 24 y 32 bits** (`m24`, `m32`, `f24`, `f32`), que se convierten a 16 bits al cargar; ver `FORMATOS-IMAGEN.md`.
- Animaciones por nombre dentro del FPG: previstas, sin diseñar del todo (`FORMATOS-IMAGEN.md`).

### Por rellenar
Aquí se irán apuntando: funciones eliminadas o con otro comportamiento, nuevas funciones de `map` y `fpg`, la solución al negro transparente y cualquier otra decisión que afecte a programas existentes.

## Herramientas que acompañan

- **FenXEdit2** es parte del proyecto (IDE escrito en Fenix, que también hay que migrar al dialecto de Simurgh): ver `IDE-FENXEDIT2.md`.
