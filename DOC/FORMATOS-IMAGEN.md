# Formatos de imagen y de recursos

## Carga de imágenes: `LOAD_IMAGE`
`LOAD_IMAGE(fichero)` detecta el formato **por la cabecera** del fichero (no por la extensión) y devuelve el código del gráfico. Es la
única función de carga: `LOAD_MAP`, `LOAD_PNG` y `LOAD_PCX` son **alias** que el compilador sustituye por `LOAD_IMAGE`
(ver «Alias» abajo). Hoy reconoce MAP (8, 16, 1, 24 y 32 bits), PNG, PCX, BMP y TGA; para añadir otro formato basta reconocer su cabecera en `gr_load_image`
(`fxi/src/g_maps.c`).

`SAVE_IMAGE(fpg, grafico, fichero [, formato])` graba en MAP, PNG, BMP o TGA. El formato es **opcional**: sin él (o con `IMG_AUTO`, 0) se adivina por la extensión (`.map`, `.png`, `.bmp`, `.tga`) y, si no se reconoce, se graba como MAP. Constantes del lenguaje: `IMG_AUTO` (0), `IMG_MAP` (1), `IMG_PNG` (2), `IMG_BMP` (3), `IMG_TGA` (4).

`SAVE_MAP(fpg, grafico, fichero)` y `SAVE_FPG(fpg, fichero)` graban en los formatos de Fenix, compatibles con DIV/Fenix (solo el
fotograma actual en MAP; en FPG, los gráficos de código 1 a 999 y todos de la misma profundidad).

## Formatos implementados en Fenix 0.92a (y por tanto en Simurgh)

| Formato | Cabecera | Carga | Grabación | Notas |
|---|---|---|---|---|
| MAP 8 bits | `map\x1A\x0D\x0A` | sí | sí (`SAVE_MAP`) | con paleta; formato de DIV |
| MAP 16 bits | `m16\x1A\x0D\x0A` | sí | sí | RGB565 |
| MAP 1 bit | `m01\x1A\x0D\x0A` | sí | sí | |
| PNG | `\x89PNG` | sí (libpng) | `SAVE_PNG` | |
| PCX | `0x0A` + versión | sí | no | |
| BMP | `BM` | sí | sí (siempre 24 bits) | sin bibliotecas; carga 1, 4, 8, 16, 24 y 32 bits sin compresión (no RLE) |
| TGA | sin firma: extensión `.tga` | sí | sí (32 bits con alfa) | sin bibliotecas; carga tipos 1, 2, 3, 9, 10 y 11 (paleta, color real y grises, con y sin RLE) |
| FPG 8 / 16 / 1 bit | `fpg` / `f16` / `f01` | `LOAD_FPG` | sí (`SAVE_FPG`) | colección de gráficos |
| FNT (fuentes) | `fnt` | `LOAD_FNT` | `SAVE_FNT` | |
| BDF (fuentes) | texto | `LOAD_BDF` | no | cargador simple |
| PAL (paletas) | `pal` | `LOAD_PAL` | `SAVE_PAL` | `LOAD_PAL` también saca la paleta de MAP, FPG, FNT y PNG; desaparecerán con las paletas |
| FLI / FLC (animación) | cabecera FLI | `START_FLI` | no | `g_flic.c` |
| WAV, OGG, MOD/S3M/XM… (audio) | | `LOAD_WAV`, `LOAD_SONG` | no | vía SDL_mixer |

Fenix no usa SDL_image: PNG va con libpng y PCX con código propio.

## Formatos descartados

Ningún programa conocido usaba estos formatos propios de Fenix 0.85, y se retiran:

- **FBM** (Fenix Bitmap): un MAP con 24 bits, varios fotogramas y secuencias de animación.
- **FGC** (Fenix Graphic Collection): una colección como el FPG, pero con los gráficos guardados en formato FBM. Aparte de lo que trae FBM solo añadía el nombre de la colección, un número de versión y una tabla de posiciones para acceder directamente a cada gráfico; nada que el FPG no pueda hacer. Estaba limitada a gráficos de código 0 a 999, igual que el FPG.
- **FPL** (paleta de Fenix): una paleta de 8 bits por canal; las paletas desaparecen.

Desaparecen `LOAD_FBM`, `SAVE_FBM`, `UNLOAD_FBM`, `LOAD_FGC`, `SAVE_FGC`, `UNLOAD_FGC`, `LOAD_FPL` y `SAVE_FPL`, la carga en segundo plano `LOAD_FGC(fichero, variable)`, y `LOAD_IMAGE` y `LOAD_PAL` ya no reconocen esos formatos. Se borraron `fbm.c`, `fgc.c`, `fpl.c` y sus cabeceras (salvo `fbm.h`, que solo conserva los tipos de secuencia y fotograma que usa la estructura de gráfico, hasta que se diseñen las animaciones por nombre en el FPG).

## Decisiones

Simurgh **no usa SDL_image**. **PNG** va integrado (libpng); **GIF** y **JPEG** quedan fuera del núcleo. Con `LOAD_IMAGE` por cabecera, un formato nuevo se añade reconociendo su cabecera en `gr_load_image`, sin tocar el lenguaje. El formato interno es 16 bits RGB565.

## Alias de funciones (compilador)
Mecanismo para que el código escrito para versiones anteriores siga compilando: una llamada a un nombre antiguo se **compila como una
llamada al destino**; el `.dcb` solo contiene el destino y el intérprete no sabe que hubo alias. La tabla está en `fxc/src/sysstub.c`
(`sysproc_aliases`). El destino debe aceptar los mismos parámetros que el origen.

| Origen | Destino |
|---|---|
| `LOAD_MAP` | `LOAD_IMAGE` |
| `LOAD_PNG` | `LOAD_IMAGE` |
| `LOAD_PCX` | `LOAD_IMAGE` |

Además, `Import "image"` se sigue aceptando (código antiguo) y no hace nada: la biblioteca ya es del núcleo.
Consecuencia: un `.dcb` compilado con Simurgh que use `LOAD_MAP` no es idéntico al de Fenix (las pruebas de `tools/fxc-compare.sh` lo tienen en cuenta con `tests/fxc/dcb-distinto-esperado.txt`).

## Pruebas
`tools/image-test.sh` compila y ejecuta `tests/simurgh/t_image.prg`: guarda un gráfico con `SAVE_MAP` y `SAVE_PNG`, lo recarga con el alias `LOAD_MAP` y con `LOAD_IMAGE` (MAP y PNG por cabecera) y comprueba los píxeles; graba un FPG con `SAVE_FPG`, lo recarga y comprueba un píxel de cada gráfico.

## MAP y FPG de 24 y 32 bits (formato nuevo)
Se amplían MAP y FPG con profundidades de color real, **sin tocar los formatos existentes** (`map`, `m16`, `m01`, `fpg`, `f16`, `f01` siguen igual): son cabeceras nuevas.

| Fichero | Cabecera (8 bytes) | Píxel |
|---|---|---|
| MAP 24 bits | `m24\x1A\x0D\x0A\0\0` | R, G, B (1 byte cada uno) |
| MAP 32 bits | `m32\x1A\x0D\x0A\0\0` | R, G, B, A (1 byte cada uno; alfa sin premultiplicar, 255 = opaco) |
| FPG 24 bits | `f24\x1A\x0D\x0A\0\0` | igual que el MAP de 24 bits |
| FPG 32 bits | `f32\x1A\x0D\x0A\0\0` | igual que el MAP de 32 bits |

El resto de la estructura es la de `m16`/`f16`: ancho, alto, código, nombre, **sin paleta**, puntos de control y los datos por filas de arriba abajo, sin relleno (en el FPG, el mismo registro de 64 bytes por gráfico).

**Estado:** el intérprete **lee** estos formatos y los convierte a 16 bits (RGB565) al cargar (conservando el alfa, ver «Alfa conservado»); **graba** `m32`/`f32` cuando el gráfico trae alfa. Todavía no graba 24 bits (no hay superficies de 24/32 bits en memoria). El alfa se aplica de forma **provisional** y simple: alfa < 128 → transparente (color 0); si no, opaco, y el negro opaco pasa a `0x0001` para no confundirse con el transparente. En 24 bits, el negro puro sigue siendo transparente. Qué hacer de verdad con el alfa queda por decidir (ver «negro transparente» en `COLOR-Y-FORMATOS.md`). Se prueba con `tests/simurgh/t_truecolor.prg`.

## Animaciones por nombre (diseño, sin implementar)
Idea tomada de las secuencias de FBM, pero **dentro del FPG** y apoyada en lo que ya tiene el FPG: los identificadores de imagen.

- Una **animación** tiene un **nombre** y una **tabla de fotogramas**; cada fotograma es un **id de gráfico del FPG** más una pausa (ms) y, opcionalmente, flags o ángulo a sumar.
- El FPG se amplía (formato nuevo, el actual no cambia) con un bloque de animaciones; un programa viejo sigue viendo solo los gráficos.
- Desde el lenguaje: reproducir por nombre (`anim("andar")` o similar), sin que el programa gestione a mano el cambio de `graph`. Por decidir: sintaxis, bucles y encadenado («al terminar, seguir con otra»), eventos.
- La definición textual podría ir en el propio programa o en un fichero que se funde en el FPG al construirlo.

Pendiente de diseño detallado antes de tocar el intérprete.

## Notas de BMP, TGA y PNG
- **Sin bibliotecas externas:** BMP y TGA son código propio (`fxi/src/img_bmp.c`, `img_tga.c`). El resultado de cargar siempre es un gráfico de 16 bits; el alfa de 32 bits sigue la regla provisional de arriba.
- **TGA no tiene firma en la cabecera**, así que `LOAD_IMAGE` solo lo reconoce por la extensión `.tga` (y luego valida la cabecera). Un TGA con otra extensión no se detecta.
- **Transparencia al grabar:** el color 0 (transparente) sale con alfa 0 en PNG y TGA. BMP de 24 bits no tiene alfa: lo transparente sale negro.
- **Alfa conservado:** una imagen cargada con alfa (MAP/FPG de 32 bits, PNG, TGA y BMP de 32 bits con alfa) guarda su canal alfa en el gráfico (un byte por píxel, solo si algún píxel no es opaco) y **lo respeta al grabar**: `SAVE_MAP` escribe `m32`, `SAVE_FPG` escribe `f32` (si algún gráfico tiene alfa, todo el FPG va en 32 bits), `SAVE_IMAGE` a PNG o TGA lleva el alfa original. Al dibujar sigue valiendo el umbral provisional (alfa < 128 = transparente); el siguiente paso será el alfa con más niveles de mezcla en el renderizado. Si se modifica el gráfico después de cargarlo, el alfa guardado no se actualiza.
- **Compresión PNG:** nivel 7 de 9 con todos los filtros (media-alta).
- **Sin gzip al grabar:** MAP, FPG, BMP y TGA se escriben sin comprimir, para que otras herramientas puedan leerlos (hasta ahora `file_open` con `"wb"` los habría dejado en gzip). Al leer, el gzip sigue siendo transparente.
- **JPEG:** no está. Se podría hacer sin biblioteca externa (la lectura de JPEG básico y la escritura son unos cientos de líneas cada una; el progresivo es bastante más); queda fuera del núcleo salvo que se decida lo contrario.
