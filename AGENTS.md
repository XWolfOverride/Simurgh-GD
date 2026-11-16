# Simurgh — AGENTS.md

Fork directo de **Fenix 0.92a**, el lenguaje de programación de juegos 2D compatible en espíritu con DIV Games Studio.
Un lenguaje de procesos pensado para aprender y para software casero, con compilador y intérprete. Simurgh lo
moderniza (multiplataforma actual, 16 bits de color, herramientas integradas) sin perder la filosofía de Fenix.
Nombre: el Simurgh, ave persa antigua que reaparece tras haber visto caer otros mundos (otros forks de Fenix).

Este fichero es un índice de decisiones y estado; el detalle está en `DOC/` (índice en `DOC/README.md`).

## Decisiones tomadas
- **Base:** `upstream/fenix-0.92a/` es el árbol de Fenix 0.92a **intacto** (sin las carpetas `CVS/`; el historial
  anterior se descarta a propósito). **No se modifica**: es la referencia. El código de Simurgh vive en `src/`, que
  empezó como copia exacta de `upstream/fenix-0.92a-fixed/`. `tools/upstream-diff.*` compara con el original (ver más abajo).
- **`upstream/fenix-0.92a-fixed/`:** Fenix 0.92a original **más las correcciones mínimas** (compila con herramientas actuales,
  `.dcb` reproducible, un error de memoria y un `test.fpg` dañado). Es la **versión de Fenix «buena»** de referencia: probada en Windows
  (32 bits), en el XO y en la VM. El diff contra el original está en `upstream/upstream-to-fixed.diff`; los cambios, en
  `upstream/CAMBIOS.md`. No se toca salvo para otra corrección a Fenix tal cual.
  Estructura: `upstream/fenix-0.92a` (original) → `upstream/fenix-0.92a-fixed` (corregido, junto al original; cada versión de Fenix que se
  importe irá como `fenixXX/` + `fenixXX-fixed/`) → `src/` (Simurgh, trabajo nuevo).
- **Licencia: GNU GPL v2** (`COPYING`). El código heredado es «v2 o posterior»; el nuevo se publica igual
  (`GPL-2.0-or-later`). Se conserva el copyright de José Luis Cebrián Pagüe y del Fenix Team y se anota la
  genealogía (`NOTICE.md`, `DOC/GENEALOGIA.md`). No se puede relicenciar a MIT.
- **Lenguaje de implementación: C11** (subconjunto común a gcc, clang y MSVC; sin extensiones de GNU fuera de
  macros de compatibilidad). Binarios de **32 bits** (compatibles con el OLPC XO-1, Linux i586 sin SSE) y de **64 bits**.
- **Capa «glue»** propia hacia el sistema (vídeo, audio, entrada, tiempo, ficheros). El renderizado es de Fenix (por
  software, `g_blit`, `g_draw`...), no del sistema. Backends: SDL 1.2 (XO, sistemas antiguos), SDL2 (Windows,
  macOS, Linux actuales) y, más adelante, DOS (VESA). Si SDL se cambia algún día, solo se toca el glue.
- **Color: 16 bits (RGB565) siempre.** Se eliminan los 8 bits y la **manipulación de paletas**; los recursos de 8 bits El modo de 256 colores ya no existe (`set_mode` con 8 deja 16) y los gráficos de 8 bits se cargan como una copia de 16 convertida una sola vez con la paleta de su fichero (`DOC/MIGRACION-FENIX.md`). Sin paletas, sin FLI y con `//$ancient on` para las tres funciones de paleta que quedan vacias (`DOC/MIGRACION-FENIX.md`).
  se convierten a 16 al cargarlos. Los sistemas modernos reciben el fotograma convertido al formato de su pantalla.
  El negro puro es transparente, como en Fenix 16 bits. Sin cambios de formato por ahora: ver `DOC/COLOR-Y-FORMATOS.md`.
- **FPG y MAP:** los de 8 y 16 bits de Fenix se leen y se escriben **100 % compatibles**. Un cambio de formato
  implica un formato nuevo, nunca modificar el existente. PNG integrado (importar); GIF y JPEG, fuera del núcleo.
- **Texto en UTF-8.**
- **Herramientas integradas:** `map` y `fpg` pasan a ser parte del lenguaje (funciones). Las herramientas externas
  se retiran.
- **Compilador reentrante** y utilizable como biblioteca (`libsimurgh`): se puede compilar desde un programa Simurgh. El
  intérprete **no** es reentrante: ejecutar un programa compilado es lanzar otro proceso `simr`. El depurador
  se evaluará más adelante.
- **IDE:** FenXEdit2 (IDE escrito en Fenix, unas 3 200 líneas) forma parte del proyecto (importado en `ide/FenXEdit2/`; compila con `tools/ide-compile.sh` y arranca con `fxi` de 32 bits; uso sin probar), ver `DOC/IDE-FENXEDIT2.md`.
- **Modo 8:** motor 2,5D de mapas de sectores con las funciones de DIV 2 y un formato de mapa propio, de texto (`simurgh-wld 1`); importa los `.wld` de DIV (a su escala: un punto de textura = una unidad). El cielo y la niebla son de cada ventana y los pone el programa (`set_sky_m8`, `m8[n].fog_distance`, `m8[n].fog_color`), no el mapa; hay procesos sólidos (`solid`, también en el modo 7) y línea de visión (`m8_sight`). Ver `DOC/MODO8.md`.
- **Migración:** todo cambio del lenguaje o de su biblioteca respecto a Fenix se apunta en `DOC/MIGRACION-FENIX.md` en el momento de decidirlo. Lo específico de llevar un programa de DIV 2 va en `DOC/MIGRACION-DIV2.md`.
- **No hay binarios `fxc` ni `fxi`:** solo el programa único (el nombre del ejecutable es independiente del nombre «Simurgh» que usa en sus textos; `SIMURGH_EXE` lo fija al construir). Las pruebas compilan y ejecutan con él; el único `fxc` que se construye es el de Fenix 0.92a como referencia de comparación. Los programas empotrados en el ejecutable (y con ficheros) vuelven más adelante, después de los paquetes; el ejecutable detecta un DCB pegado a sí mismo sin depender de su nombre.
- **Un solo programa, `simr`** (nombre del ejecutable decidido; se presenta como Simurgh): `simr x.dcb` ejecuta, `simr x.prg` compila en memoria (sin salida si va bien) y ejecuta, `simr -b x.prg` solo compila. Opciones `-d`, `-i dir`, `-g`, `-c`, `-f fichero...`; el valor siempre separado; lo que va tras el fichero son argumentos del programa. Se retiran `-s -d -a -l` de fxc y `-f -r` de fxi; fxc devuelve 0 al compilar bien. Ver `DOC/MIGRACION-FENIX.md`; `tools/zig-probe-simurgh.sh` y `tools/simurgh-test.sh`.
- **Sin `DECLARE`:** el compilador hace dos pasadas; la primera recoge las firmas de `process`/`function` (con sus `include`), así que se puede llamar antes de definir. Ver `DOC/MIGRACION-FENIX.md`.
- **Parámetros opcionales:** funciones del sistema con la firma completa y valores por defecto (`"IISI|,,,0"` en `sysprocs.h`) y funciones/procesos del usuario con `function f(a, b=0, c)` (cualquier posición; enteros, reales y cadenas); se omiten al final o en medio con huecos (`f(a,,c)`); funciona también si se llama antes de definirlos. Ver `DOC/MIGRACION-FENIX.md`. Alfa: se conserva el cargado y se respeta al grabar; al dibujar, umbral provisional (`DOC/FORMATOS-IMAGEN.md`).
- **Alias del compilador y `LOAD_IMAGE`:** los nombres antiguos se compilan como su destino (`LOAD_MAP`, `LOAD_PNG`, `LOAD_PCX` → `LOAD_IMAGE`, que detecta el formato por cabecera); `SAVE_MAP`, `SAVE_FPG` y `SAVE_IMAGE` (formato opcional: MAP, PNG, BMP, TGA) existen; BMP y TGA se cargan y graban sin bibliotecas. FBM, FGC y FPL se retiran; MAP/FPG de 24 y 32 bits se leen (se convierten a 16). Ver `DOC/FORMATOS-IMAGEN.md`.
- **Compatibilidad:** nada es sagrado, pero sin alejarse de la filosofía de Fenix para poder reciclar programadores.
  El lenguaje tiene versión explícita.
- **Objetivos:** Windows (32/64), Linux (32/64), Linux ARM (Raspberry Pi 1 a 5), macOS (Intel y Apple Silicon), OLPC XO-1, DOS con **586
  (Pentium) como mínimo** y Sony PSP (previsto)
  (16 bits de color, con LFN si el sistema lo ofrece, sin exigirlo). Ver `DOC/PLATAFORMAS.md`.
- **Dominio:** registrado `simurgh-gd.dev` (de «game development»; `simurgh.dev` está en venta en un registrador, descartado por precio/ocupación). Uso y alojamiento de la web: por decidir.
- «Más adelante» significa **antes de la primera versión publicada**: optimizar opcodes, MMX y 3DNow, transparencia y
  alfa propios, depurador y otros temas de `DOC/HOJA-DE-RUTA.md`.

## Estado
- Fase 0: andamio y documentación (hecho). `src/` difiere de `upstream/` en ocho ficheros (memoria a cero en `fxc/src` para que el `.dcb` sea
  reproducible; `fxi/src/img_png.c` adaptado a libpng 1.5+; `fxi/src/g_fpg.c` con el error de memoria de `gr_read_lib` corregido;
  `bin/test.fpg` con el CRC reparado); `tools/upstream-diff.sh --names` los lista.
- **`fxc` limpio (2026-11-15):** sin SDL, memoria por `xalloc.h`, errores de memoria corregidos, valgrind limpio; `tools/fxc-compare.sh` lo compara con
  `upstream/fenix-0.92a-fixed` (5 de 5 idénticos). Reentrante (`fxc_main`; `tools/fxc-reentrant.sh`). Pendiente: formato nuevo de `.dcb` (va con `fxi`). Regla: el lenguaje no se toca (mucho); el
  binario de compilación sí puede cambiar.
- **Intérprete: de momento solo 32 bits.** La VM guarda punteros en enteros de 32 bits; compilado para 64 bits se cae al cargar un
  `.dcb` (`DOC/PROBLEMAS.md`, puntos 31-32). El compilador sí funciona en 64 bits. Verificado: `fxi` de 32 bits compilado con Zig para
  Windows arranca y muestra el menú de pruebas (`tools/zig-probe-simurgh.sh`). Pendiente de decisión: modelo de punteros de 64 bits.
- Compilación de Fenix 0.92a verificada para el OLPC XO-1 (parche en `DOC/fenix-xo-0.92a.patch`): ver
  `DOC/COMPILACION.md`.
- Sin repositorio git inicializado ni scripts `build.*` (se crean con la primera versión utilizable).

## Trabajar aquí
- No tocar `upstream/` ni normalizar sus finales de línea. El resto del repositorio sigue las convenciones del autor
  (CRLF, `.sh` en LF).
- **GPL v2, apartado 2.a:** cada fichero de texto de `src/` que se modifique respecto a `upstream/` debe llevar un aviso
  destacado del cambio y su fecha: una línea `Simurgh: modificado AAAA-MM-DD` en los comentarios iniciales. Los ficheros
  nuevos llevan su propia cabecera. `tools/upstream-diff.sh --check` (o `.cmd` en Windows) lo comprueba; `--names`,
  `--stat` y `--patch` listan y generan el diff contra Fenix 0.92a.
- Documentación publicable impersonal: nada de datos de equipos o personas.
- Probar contra Fenix 0.92a como referencia antes de cambiar comportamiento (ver «Pruebas» en la hoja de ruta).
