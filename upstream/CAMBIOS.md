# Fenix 0.92a corregido (`fenix-0.92a-fixed`)

`fenix-0.92a-fixed/` es **Fenix 0.92a original (`fenix-0.92a/`, en esta misma carpeta `upstream/`) más las correcciones mínimas** para que compile con
herramientas actuales y funcione como Fenix 0.92a de verdad. No añade funciones ni cambia el comportamiento previsto. Es la
**versión de referencia** de Fenix sobre la que se mide Simurgh: el trabajo nuevo se hace en `../src/`.

El diff completo entre el original y esta versión está en `upstream-to-fixed.diff`. Se considera «buena» tras probarla en Windows
(32 bits), en el OLPC XO-1 (Linux i586) y en la VM de compilación (Fedora 18): las siete opciones del menú de pruebas corren.

## Cambios (8 ficheros)

| Fichero | Cambio | Por qué |
|---|---|---|
| `fxc/src/typedef.c` | `typedef_new` pone el `TYPEDEF` a cero | Los tramos sin usar acababan en el `.dcb` con basura (`DOC/PROBLEMAS.md`, 29) |
| `fxc/src/segment.c` | `calloc` y puesta a cero de la parte nueva tras `realloc` | Idem: el `.dcb` no era reproducible |
| `fxc/src/strings.c` | Idem | Idem |
| `fxc/src/codeblock.c` | Idem | Idem |
| `fxc/src/dcbw.c` | `calloc` en las tablas del `.dcb`, `memset` de `DCB_FILE` y de `DCB_VAR` | Idem |
| `fxi/src/img_png.c` | `png_jmpbuf(png_ptr)` y sin acceso a `info_ptr->palette` | libpng 1.5 o posterior |
| `fxi/src/g_fpg.c` | `gr_read_lib`: si falla la lectura de un gráfico se destruye (`bitmap_destroy`) y no se añade | Se usaba memoria liberada (valgrind) y el intérprete se caía |
| `bin/test.fpg` | Se reescriben 4 bytes: el CRC del final del gzip | El fichero de la distribución tiene el CRC dañado; zlib 1.2.7 falla al leerlo |

Cada fichero de código modificado lleva el aviso de cambio y la fecha que exige la GPL v2 (apartado 2.a).

## Resultado verificado
- `fxc` produce un `.dcb` **idéntico byte a byte** en Windows x64, Linux de 32 bits y el XO (`test.prg`: MD5
  `ada2a96cc95775249e47eb7103568ef9`).
- El `.dcb` es reproducible: dos compilaciones dan el mismo fichero.
- Las siete opciones del menú de `test.prg` no se caen.

## Reproducir `fenix-0.92a-fixed` desde el original

Desde esta carpeta (`upstream/`):

```sh
cp -r fenix-0.92a /tmp/f
cd /tmp/f
patch --binary -p2 < <ruta>/upstream/upstream-to-fixed.diff       # con --binary: hay ficheros con finales de línea mezclados
# test.fpg es binario y el diff no lo recoge: se cambian 4 bytes en el desplazamiento 41884 (CRC del gzip)
printf '\x6a\x0a\x81\x88' | dd of=bin/test.fpg bs=1 seek=41884 conv=notrunc
```

El resultado es idéntico a `fenix-0.92a-fixed/` (comprobado con `diff -rq`).

## Regenerar el diff
```sh
cd <raíz del proyecto>
diff -ruN upstream/fenix-0.92a upstream/fenix-0.92a-fixed > upstream/upstream-to-fixed.diff
```

## Estructura de `upstream/`

```
upstream/
├─ fenix-0.92a/            Fenix 0.92a original, intacto (sin las carpetas CVS/)
├─ fenix-0.92a-fixed/      el mismo con las correcciones de esta tabla
├─ upstream-to-fixed.diff  diff entre los dos
└─ CAMBIOS.md              este documento
```

Si algún día se importa otra versión de Fenix, irá al lado como `fenixXX/` y `fenixXX-fixed/` con su propio diff.

## Reglas
- **No se modifica** `fenix-0.92a-fixed/` salvo para añadir otra corrección a Fenix 0.92a «tal cual»; si se hace, se regenera el
  diff y se anota aquí.
- Los ficheros no se normalizan (mezcla de finales de línea y codificación ISO-8859 originales): ver `.gitattributes`.
- `tools/upstream-diff.sh --fixed` compara `src/` con esta versión.

## Pendientes conocidos de Fenix 0.92a que NO se corrigen aquí
Van en `../DOC/PROBLEMAS.md` y se resuelven en `src/`: `fxc` depende de SDL solo por `SDL_Init`, estado global y `exit()` en el
compilador, punteros en enteros de 32 bits en la VM, MMX en ensamblador, `-fcommon`, etc.
