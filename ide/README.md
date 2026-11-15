# ide/

Entorno de desarrollo de Simurgh.

| Carpeta | Contenido | Estado |
|---|---|---|
| [FenXEdit2/](FenXEdit2/) | **FenXEdit ][** 0.31, el IDE escrito en Fenix (editor con resaltado, proyectos `.fxp`, explorador, visor de mapas y FPG, sonido). Fuentes y recursos tal como estaban | importado sin cambios; compila y arranca (uso sin probar) |

## Qué se importó y qué no
- **Sí:** los `.prg`, fuentes (`.fnt`, `Font/`), `.fpg`, imágenes, sonidos y scripts de compilación originales.
- **No:** ejecutables y bibliotecas de terceros (`*.dll`, `*.so`, `*.exe`), el `.dcb` compilado, el `.rar` de copia de seguridad, y los programas auxiliares de otros autores (`Prg/`: conversores de fuentes y animador) mientras no se aclare su licencia.

## Estado
**Compila** sin cambios en el IDE (`tools/ide-compile.sh`): `Import "image"` se acepta (la biblioteca es ya del núcleo) y `LOAD_IMAGE`, `SAVE_MAP` y `SAVE_FPG` existen en `fxc`/`fxi` (ver `DOC/FORMATOS-IMAGEN.md`).

**Arranca** con el `fxi` de 32 bits (ventana y pantalla de inicio sin errores). Falta probar su uso (abrir ficheros, visor de mapas y FPG, guardar con `SAVE_MAP`/`SAVE_FPG`) y pasar el código a UTF-8.

## Notas
- Fuentes en ISO-8859 (Fenix); pasarán a UTF-8 al migrar.
- El código principal es del propio proyecto (autor XWolf); el resto de cabeceras citan a otros autores y se revisarán antes de la primera versión.
