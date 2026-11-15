# IDE FenXEdit2

**FenXEdit ][** (versión 0.31) es un entorno de desarrollo completo **escrito en Fenix**: editor de código con
resaltado de sintaxis, gestión de proyectos (`.fxp`), explorador de ficheros, visor de mapas y FPG, ayuda y sonido.
Unas 3 200 líneas de `.prg` repartidas en módulos (`XCode`, `XControl`, `XDir`, `XExt`, `XFpg`, `XMap`, `XSound`,
`XSyntax`...). En su día no podía compilar desde dentro: lanzaba `fxc` y `fxi` con ficheros por lotes. El objetivo de Simurgh
es que sí pueda.

## Qué necesita del lenguaje o del núcleo
- Compilar desde un programa (compilador reentrante, `compile(...)`) y recibir los errores como texto con línea.
- Lanzar un programa compilado como proceso aparte, y recoger su salida.
- Cargar PNG (usaba `Import "image"` con `Load_png`/`Load_image`, de una biblioteca externa): pasa al núcleo.
- Funciones de FPG y mapas integradas (el IDE tiene visor de FPG).
- Acceso a directorios y ficheros (`XDir`), UTF-8 y rutas portables.

## Estado

Importado en `ide/FenXEdit2/` (ver `ide/README.md`). **Compila** con `tools/ide-compile.sh` sin cambios en el IDE y **arranca** con `fxi` de 32 bits (uso sin probar).

## Pendiente
- Leer el código fuente completo y listar las funciones que usa.
- Revisar las cabeceras de autoría (el código principal es del propio proyecto; hay piezas de otros autores en los auxiliares no importados).
- Decidir cómo se versiona la sintaxis para que un programa escrito en el dialecto de Fenix siga compilando.
