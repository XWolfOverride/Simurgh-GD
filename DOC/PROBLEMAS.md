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
32. **Compilación de `fxi` de 32 bits con Zig** (`tools/zig-probe-fxi.sh`): funciona y muestra el menú de pruebas de Fenix. Hizo
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
