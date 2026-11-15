# Problemas conocidos

Lista de riesgos del código heredado y del plan. Al resolver uno, anotar cómo y dónde.

## Licencia
1. Base GPL v2 o posterior: Simurgh también (`GPL-2.0-or-later`); no se puede relicenciar a MIT.
2. Mantener los avisos de copyright originales y la genealogía.
3. El IDE rescatado (FenXEdit2) es otro código con su licencia: comprobarla antes de incluirlo.

## Código heredado
4. **Compilador no reentrante:** estado global y unos 22 `exit()` en `fxc/src`. Imprescindible arreglarlo.
5. **Suposiciones de 32 bits:** punteros y tamaños mezclados con enteros en el bytecode y `offsets.h`.
6. **Orden de bytes:** el `.dcb` se guarda como está en memoria; hay que fijar el formato (little-endian).
7. **Codificación:** fuentes y mensajes en Latin-1; pasar a UTF-8.
8. **Rutas:** mayúsculas/minúsculas, `\` y `/`, nombres con acentos, 8.3 y LFN en DOS.
9. **Dependencias:** libpng 1.5 (ya adaptado en el parche del XO), SDL_mixer, `libungif` (fuera). PNG se integra con una
   biblioteca pequeña propia o de dominio público.
10. **`-ffast-math` en `configure.in`:** cambia resultados de coma flotante entre sistemas; quitar para pruebas repetibles.
11. **Carga de bibliotecas (`fnx_loadlib`, `dlopen`):** distinto en Windows (`LoadLibrary`) y macOS (firma); definir una
    interfaz estable si habrá extensiones.
12. **MMX en ensamblador** (`mmx_hspan.c`, `mmx_main.c`): solo 32 bits x86; opcional y medir.
13. **Escaladores por software** (`hq2x`, `scale2x`, `scanline`): decidir si se conservan.

## Portabilidad
14. SDL 1.2 y SDL2 detrás de una capa propia; la paleta de 8 bits desaparece, lo que simplifica la presentación.
15. `computed goto` no existe en MSVC: macros con alternativa.
16. C11 en MSVC: sin matrices de longitud variable ni `<threads.h>`.
17. Sin SSE en el XO ni en un 586 sin SSE: ninguna optimización puede depender de SSE sin alternativa.
18. macOS: arm64 y x86-64, universal, firma y notarización si se reparte `.app`.
19. DOS: mínimo 586; LFB, DPMI, memoria (ver `PLATAFORMAS.md`).

## Diseño
20. Compilador como biblioteca, con `compile(...)` desde el lenguaje; intérprete no reentrante.
21. `map` y `fpg` integrados: definir las funciones del lenguaje y sus formatos.
22. **FenXEdit2** usa un dialecto antiguo y una biblioteca de imágenes externa: ver `IDE-FENXEDIT2.md`.
23. **Modelo de procesos:** el planificador por prioridad y por fotograma condiciona las optimizaciones de opcodes.
24. **Compatibilidad:** nada es sagrado, pero hay que mantener la filosofía de Fenix. El lenguaje tendrá versión
    explícita y, probablemente, un modo de compatibilidad.
25. **Negro puro y transparencia** (`COLOR-Y-FORMATOS.md`): resolver antes de la primera versión.
26. **Pruebas:** sin un conjunto de programas con salida esperada, cualquier cambio de opcodes es un riesgo.

## Hallazgos de la primera compilación con Zig (2026-11-15)
27. **`fxc` compila y funciona para Windows x64 con Zig 0.17.1 (`zig cc`, clang 22)** sobre `src/` sin cambiar código, con tres
    adaptaciones de la línea de órdenes: `-fcommon` (Fenix define variables globales en varios ficheros; clang moderno
    lo rechaza por defecto, más adelante hay que arreglarlo con `extern`), `-lshlwapi` (`PathRemoveFileSpecA`) y
    `-DVERSION="0.92a"` (lo definía el `configure`). Excluir `fxc/src/dcbw_old.c` (código muerto: no está en el `Makefile`).
28. `fxc` depende de SDL solo por los tipos (`Uint8`...), `SDL_BYTEORDER` y un `SDL_Init` al arrancar: no necesita SDL
    de verdad. Hay que quitarlo del compilador. Necesita zlib (Zig no la trae; se compiló zlib 1.3.1 junto).
29. **El `.dcb` no era reproducible (RESUELTO 2026-11-15).** `fxc` escribía memoria sin inicializar: `typedef_new` dejaba sin
    poner a cero el resto de los `TYPEDEF` (los contadores de tramos sin usar acababan en el fichero, incluso restos de
    punteros), y los bloques de segmentos, cadenas y código crecían con `realloc` sin limpiar la parte nueva. Corregido con
    `calloc`/`memset` en `typedef.c`, `segment.c`, `strings.c`, `codeblock.c` y `dcbw.c` (con aviso de cambio de la GPL).
30. **Resultado:** dos ejecuciones de `fxc` dan el mismo `.dcb`, y el de Windows x64 (Zig) y el de Linux de 32 bits (gcc)
    son **idénticos byte a byte** (29 787 bytes para `test.prg`). `fxc` devuelve el código de salida 1 aun compilando bien
    (comportamiento heredado): hay que mirar el fichero generado, no el código.

## Primera compilación del intérprete con Zig (2026-11-15)
31. **`fxi` no funciona compilado para 64 bits.** Compila y enlaza (Windows x64, Zig 0.17.1), pero se cae (violación de segmento)
    al cargar cualquier `.dcb`. Causa confirmada con los avisos de clang (`-Wpointer-to-int-cast`): la **máquina virtual
    guarda punteros en enteros de 32 bits** (la pila y las variables de la VM; `i_main.c` líneas 551-770, `instance.c` 739).
    No es un fallo suelto, es el diseño: portar el intérprete a 64 bits exige otro modelo (celdas de 64 bits o
    desplazamientos en lugar de punteros). **Por ahora el intérprete es de 32 bits**; en Windows y Linux de 64 bits se ejecuta
    como proceso de 32 bits (en macOS actual los programas de 32 bits ya no se ejecutan: habrá que resolverlo antes).
32. **Compilación de `fxi` de 32 bits con Zig** (entonces `tools/zig-probe-fxi.sh`, hoy `tools/zig-probe-simurgh.sh`): funciona y muestra el menú de pruebas de Fenix. Hizo
    falta: `-fgnu89-inline` (Fenix usa `_inline` con la semántica antigua; sin ella faltan `_HLine8_nostipple` y similares),
    `-Wno-incompatible-function-pointer-types` y `-Wno-incompatible-pointer-types` (clang moderno los trata como error),
    adaptar `img_png.c` a libpng 1.5 o posterior (cambio hecho en `src/`, con aviso de la GPL), excluir `mmx_*.c`, y un `WinMain`
    propio porque el `SDLmain` de MinGW pide `__iob`.
33. MSYS2 ya no ofrece el entorno de 32 bits (`mingw32` no tiene SDL, SDL_mixer ni libpng); por eso se usan las bibliotecas
    de SDL 1.2.15 para MinGW, SDL_mixer 1.2.12 (VC) y libpng/zlib compiladas desde fuentes junto al intérprete.
34. `SDL_Init` de `fxi` pide `SDL_INIT_CDROM`: SDL 1.2 sobre sdl12-compat (MSYS2 actual) funciona igualmente, pero el CD de
    audio no tiene sentido en Simurgh (se retira, ver `ARQUITECTURA.md`).

## Fallo al elegir una opción en el menú de pruebas (2026-11-15)
35. **`test.fpg` de la distribución de Fenix 0.92a está dañado:** el flujo gzip se descomprime completo (68 861 bytes, idéntico con
    `zcat` y con Node) pero el CRC del final no coincide (guardado `64d73513`, calculado `88810a6a`). Con zlib 1.2.7 (Fedora 18) el
    `gzread` que cruza el final devuelve error, la lectura del gráfico `FONDO` falla en su fila 193 y el intérprete se cae al elegir
    cualquier opción del menú (todas empiezan con `put_screen(0, 1)`). Se reescribió el CRC en la copia de `src/bin/test.fpg`
    (`gzip -t` pasa; `upstream/` queda intacto). El contenido parece íntegro (el `FONDO` se ve bien); no se ha podido comparar con un
    original sano.
36. **Error de memoria en `gr_read_lib` (`fxi/src/g_fpg.c`):** si falla la lectura de un gráfico, se hacía `free(gr)` y se seguía con
    `grlib_add_map(libid, gr)` sobre memoria liberada (y el gráfico seguía enlazado en la lista global). Valgrind lo detectó
    («Invalid read ... free'd»). Corregido: `bitmap_destroy`, no se añade a la librería y se deja de cargar. Con el FPG dañado
    original ya no se cae (falta el gráfico, pero el resto funciona).
37. **Cómo se encontró:** reproducible sin pantalla (`SDL_VIDEODRIVER=dummy`) con copias de `test.prg` que fijan `option = N`; `gdb`
    dio la línea y `valgrind` la causa. Las siete opciones corren sin caerse en la VM, en Windows de 32 bits (opción 1 con captura:
    «400 sprites» y el fondo) y en el XO.

## Sonido de la demo (2026-11-15)
38. **El «fallo» del sonido no es de `fxi`.** `test.prg` no restaura el volumen: tras la primera vez que se ejecuta la prueba de sonido y se
    sale, el fundido de salida deja el volumen en 0 y la siguiente vez la música «solo suena» cuando el programa vuelve a tocar el volumen.
    `fxi` y SDL_mixer inician el volumen al máximo (128) y la mezcla es correcta (capturada con el controlador `disk` de SDL en Windows y
    en Linux). No se corrige: es de la demo, y `upstream/fenix-0.92a-fixed` no toca programas de ejemplo salvo para que no se caigan.
39. **Estructura de las fuentes** (decisión del 2026-11-15): `upstream/fenix-0.92a` (Fenix original) → `upstream/fenix-0.92a-fixed` (Fenix corregido, versión
    de referencia buena, con su diff y `CAMBIOS.md` en `upstream/`) → `src/` (Simurgh). El 2026-11-15 se movió `upstream-fixed/` dentro de `upstream/`.

## Limpieza de `fxc` (2026-11-15)
Norma del autor: el **lenguaje** no se toca (mucho), para respetar la filosofía de Fenix; el **binario de compilación** (formato del
`.dcb`, estructuras internas, código del compilador) es otra cosa y no hay que respetarlo. Esta pasada no cambia el `.dcb`.

40. **`fxc` ya no depende de SDL.** `include/simtypes.h` da los tipos (`Uint8`…) y `SIM_BIG_ENDIAN`; se quitó el `SDL_Init` del arranque. El
    compilador compila y enlaza sin ninguna cabecera ni biblioteca de SDL (los ficheros compartidos, `files.c` y `dcb.h`, también).
41. **`-fcommon` ya no hace falta:** solo había una variable definida dos veces (`debug`); ahora es `extern` en `fxc.h`. Se borró
    `dcbw_old.c` (código muerto, ni estaba en el `Makefile`).
42. **Un solo sitio para pedir memoria: `fxc/inc/xalloc.h`** (`xmalloc`, `xcalloc`, `xrealloc`, `xrealloc0`, `xstrdup`). Reglas: lo que va al
    `.dcb` o se lee sin rellenar sale a cero (`xcalloc`); al crecer con `xrealloc0` solo se pone a cero la parte nueva (antes había
    `malloc`+`memset` repartidos y a veces duplicados); lo que se rellena entero antes de usarse (`string_offset`, `constants`…) no se
    pone a cero. La falta de memoria ya no se comprueba en cada sitio (había ocho mensajes distintos).
43. **Errores reales encontrados y corregidos** (varios no afectaban al resultado, otros eran desbordes):
    - `token.c`: `realloc(text, part)` pedía el tamaño de la pieza y no el total: **desborde de memoria** al expandir macros con parámetros largos.
    - `error.c`: reserva sin hueco para el `\0` (desborde de 1 byte), `msg` sin inicializar si una línea del catálogo no tenía texto, y fuga.
    - `segment.c`: al crecer, el vector de segmentos se **reservaba nuevo con `malloc` y perdía** el contenido anterior (debía ser `realloc`).
    - `procedure.c`: las posiciones nuevas del vector `procs` quedaban sin inicializar y `procdef_get` podía devolverlas.
    - `sysstub.c`: la tabla de 32 entradas guardaba además un `NULL` final: desborde de una posición.
    - `strings.c` (`string_load`): máscara `& 1023` en lugar de `& ~1023` al calcular el tamaño.
    - `c_data.c`: `type` y `typeb` se leían sin inicializar en la primera vuelta de `compile_varspace`.
    - `c_code.c`, `c_data.c`, `c_main.c`, `token.c`: todas las variables `expresion_result` locales se leían sin rellenar del todo (valgrind: 18 avisos).
    - `main.c`: fuga del buffer del fichero fuente.
44. **Comprobación:** `valgrind --track-origins=yes --leak-check=full` sobre cinco programas (correctos y con error): **0 errores y 0 bytes
    perdidos**. El `.dcb` de `test.prg` sigue siendo el mismo (MD5 `ada2a96cc95775249e47eb7103568ef9`).
45. **Pruebas de referencia (primer paso):** `tests/fxc/` (5 programas: uno con estructuras, macros, `SWITCH`, procesos…, tres con errores de
    compilación y el menú de pruebas) y `tools/fxc-compare.sh`, que compila `src/` y `upstream/fenix-0.92a-fixed` con Zig y compara la salida del
    compilador, el código de salida y el `.dcb`. Resultado: **5 de 5 idénticos**.
46. **Reentrancia (2026-11-15):** `fxc` es ya una biblioteca: `fxc_main(argc, argv)` (en `main.c`; la orden `fxc` es solo `cli.c`) compila y devuelve el código de salida.
    - **Errores sin `exit()`:** todo error fatal pasa por `fxc_fatal(código)`, que salta (`longjmp`) al retorno de `fxc_main`. Fuera de `fxc_main` termina el proceso.
    - **Memoria:** `xalloc` apunta cada bloque en una lista; al terminar (bien o con error) `fxc_cleanup()` lo libera todo y reinicia el estado de cada módulo, así que se puede llamar muchas veces seguidas sin fugas, también tras un error. `free` pasa a ser `xfree`.
    - **Hilos:** todo el estado global de `fxc` (y el de `common/files.c` y `common/xctype.c`, activado con `-DFILES_STATE=__thread`) es propio de cada hilo (`FXC_TLS`), y la tabla de funciones del sistema se copia por hilo. Se pueden compilar varios programas a la vez. `-DFXC_NO_TLS` lo desactiva en destinos sin TLS (p. ej. DOS).
    - **Pruebas:** `tests/fxc/reentrant.c` + `tools/fxc-reentrant.sh`: 100 compilaciones seguidas con errores entre medias y 8 hilos × 100 vueltas, con el `.dcb` idéntico al de referencia. Compila también para Linux 32 bits y ARM64.
    - `messages.c` pasa a `messages.h` (solo eran macros). Se borra el código de memoria agotada que ya no podía ejecutarse.
47. **Formato del `.dcb`:** ya era de ancho fijo, empaquetado (`#pragma pack(1)`) y little-endian (reordenado en big-endian); el mismo `.dcb` sale en Windows 64, Linux 32 y XO. Se fijan los tamaños con `_Static_assert` en `dcb.h` para que no se rompa por accidente. Un formato nuevo (con número de versión propio) se hará junto con el trabajo en `fxi`, que es quien lo lee; no se cambia solo en el compilador.
48. **Pendiente menor en `fxc`:** avisos de `-Wall` (`-Wunused-but-set-variable`, `-Wpointer-sign`, codificación Latin-1 en cadenas); no se ha repetido valgrind tras la reentrancia.

49. **Error heredado corregido en `fxc`:** el tipo de retorno `int`, `dword`, `byte` o `short` en `function` daba siempre «Conflict with previous declaration» por un error de precedencia en la comprobación del prototipo (`proc->type != x ? A : B`). Ahora funciona `function int f(...)`.
50. **Programa único `simr` (2026-11-15):** intérprete y compilador en un ejecutable. El compilador entra por `fxc_compile_memory` (compila en memoria, sin imprimir si va bien) y escribe el `.dcb` en un fichero en memoria (`file_mem_new`, nuevo tipo `F_MEMFILE` en `common/files.c`) que el intérprete lee con `dcb_load_from`. Pendiente: los símbolos duplicados entre `fxc` y `fxi` se resuelven por renombrado (`fxc_rename.h`) y no están separados en bibliotecas; el estado de ficheros (`common/files.c`) es compartido (en `fxc` suelto es por hilo).
51. **Dos pasadas en `fxc` (2026-11-15):** la primera (`fxc/src/c_sigs.c`) recorre el programa con el mismo analizador y recoge las firmas de procesos y funciones; `fxc_compile_source` reinicia el estado entre pasadas (`fxc_pass_reset`, conservando los ficheros a incluir, las rutas y las opciones) y la segunda compila con las firmas conocidas (`sigs_apply` en `procdef_new`). Se quita `DECLARE` (error específico: no es necesario). El mecanismo de llamadas diferidas de `procedure.c` (huecos y desvíos al final del bloque) queda como respaldo para procesos cuya cabecera la primera pasada no reconoce (p. ej. definidos con otra sintaxis). Cuesta una lectura más del fuente. Pendiente: comprobar con valgrind que el reinicio entre pasadas no deja fugas ni accesos sin inicializar.
