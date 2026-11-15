# Arquitectura

## Piezas

```
programa.prg ──► compilador (fxc → libsimurgh) ──► programa.dcb ──► intérprete (fxi → simurgh) ──► pantalla/sonido
                                                                          │
                                                                    capa glue (sistema)
```

| Pieza | Hoy en Fenix 0.92a | Objetivo en Simurgh |
|---|---|---|
| Compilador | `fxc/` (≈11 800 líneas de C), ejecutable aparte, estado global, `exit()` en errores | Biblioteca **reentrante** `libsimurgh`; el ejecutable `simurgh` la usa. Un programa Simurgh puede invocarla (`compile(...)`) |
| Intérprete | `fxi/` (≈36 600 líneas) con SDL repartido en varios ficheros | Intérprete **no reentrante**; ejecutar un programa es lanzar otro proceso `simurgh` |
| Código común | `common/`, `include/` (≈9 700 líneas): ficheros, rutas, expresiones regulares, `xctype` | Se conserva, saneando portabilidad y UTF-8 |
| `map`, `fpg` | Programas externos | Funciones del lenguaje y módulo del núcleo |
| Formato `.dcb` | Volcado directo de estructuras en memoria | Formato de fichero definido (little-endian, tamaños fijos), con versión |

## Capa glue

El renderizado es de Fenix, **no** de SDL: `g_blit.c`, `g_draw.c`, `g_blendop.c`, `g_mode7.c`, `g_scroll.c`, fuentes,
texto, FLI... pintan en un búfer de píxeles propio. El sistema solo recibe el fotograma terminado. La capa glue define una
interfaz mínima y pequeña:

| Área | Funciones |
|---|---|
| Vídeo | Crear pantalla (ancho, alto, ventana o completa), obtener el búfer RGB565, presentar el fotograma, título, cursor, escalado |
| Entrada | Teclado (códigos propios), ratón, mando; cola de eventos |
| Audio | Abrir dispositivo con callback de mezcla; la mezcla es del núcleo |
| Tiempo | Reloj monotónico, espera |
| Ficheros | Rutas, búsqueda de carpetas, LFN opcional en DOS, UTF-8 |
| Sistema | Lanzar un proceso (para ejecutar otros programas Simurgh), variables de entorno |

Backends previstos: SDL 1.2, SDL2 y DOS (VESA); queda abierto cambiar SDL por otra biblioteca sin tocar el núcleo.

### SDL en el código heredado (referencia)

Llamadas a SDL por fichero del intérprete: `g_main.c` 63, `f_cd.c` 44 (CD de audio, se retira), `g_pal.c` 26
(paletas, se retira), `sound.c` 19, `f_joystick.c` 18, `i_func.c` 10, `main.c` 8. El resto de ficheros apenas toca SDL.
Hay además **escaladores por software** (`scaler_hq2x.c`, `scaler_scale2x.c`, `scaler_scanline.c`) y rutinas MMX
(`mmx_hspan.c`, `mmx_main.c`), que se evaluarán más adelante.

## Hecho el diseño, ¿qué tocar primero?

1. Separar compilador y estado global (reentrancia): la parte más delicada.
2. Introducir la capa glue sin cambiar comportamiento.
3. Quitar paletas y pasar a 16 bits siempre.
Orden detallado en [HOJA-DE-RUTA.md](HOJA-DE-RUTA.md).

## Compilador reentrante: qué significa

Todo lo que el compilador recuerda entre llamadas (identificadores, constantes, código generado, errores) va en una
estructura `SimurghCompiler` que se crea, se usa y se destruye. Ante un error no se llama a `exit()`: se registra y se
devuelve un código, con el texto del error. Así se puede compilar varias veces en el mismo proceso, o desde el IDE,
sin que una compilación contamine a la siguiente ni cierre el programa.

## Estado del compilador (2026-11-15)
`fxc` ya no usa SDL (`include/simtypes.h`), pide toda la memoria por `fxc/inc/xalloc.h` y pasa valgrind sin errores ni fugas. Es reentrante y utilizable como biblioteca (`fxc_main`, estado propio de cada hilo, errores sin `exit()`). Pruebas de referencia contra Fenix 0.92a corregido: `tools/fxc-compare.sh` (ver `PROBLEMAS.md`, 40 a 48).
