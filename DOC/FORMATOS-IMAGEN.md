# Formatos de imagen y de recursos

## Carga de imágenes: `LOAD_IMAGE`
`LOAD_IMAGE(fichero)` detecta el formato **por la cabecera** del fichero (no por la extensión) y devuelve el código del gráfico. Es la
única función de carga: `LOAD_MAP`, `LOAD_PNG` y `LOAD_PCX` son **alias** que el compilador sustituye por `LOAD_IMAGE`
(ver «Alias» abajo). Hoy reconoce MAP (8, 16, 1, 24 y 32 bits), PNG y PCX; para añadir otro formato basta reconocer su cabecera en `gr_load_image`
(`fxi/src/g_maps.c`).

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
| FPG 8 / 16 / 1 bit | `fpg` / `f16` / `f01` | `LOAD_FPG` | sí (`SAVE_FPG`) | colección de gráficos |
| FGC (Fenix Graphic Collection) | `FenixLibrary\x1A\x0D\x0A` | `LOAD_FGC` | `SAVE_FGC` | colección comprimida, propia de Fenix |
| FNT (fuentes) | `fnt` | `LOAD_FNT` | `SAVE_FNT` | |
| BDF (fuentes) | texto | `LOAD_BDF` | no | cargador simple |
| PAL (paletas) | `pal` | `LOAD_PAL` | `SAVE_PAL` | `LOAD_PAL` también saca la paleta de MAP, FPG, FNT, PNG y FGC; desaparecerán con las paletas |
| FLI / FLC (animación) | cabecera FLI | `START_FLI` | no | `g_flic.c` |
| WAV, OGG, MOD/S3M/XM… (audio) | | `LOAD_WAV`, `LOAD_SONG` | no | vía SDL_mixer |

Fenix no usa SDL_image: PNG va con libpng y PCX con código propio.

## Formatos descartados

**FBM** (Fenix Bitmap) y **FPL** (paleta de Fenix) se retiran: ningún programa conocido los usaba. Desaparecen `LOAD_FBM`, `SAVE_FBM`, `UNLOAD_FBM`, `LOAD_FPL` y `SAVE_FPL`; `LOAD_IMAGE` ya no reconoce FBM. El código de FBM sigue dentro del intérprete solo porque **FGC** (colección de gráficos de Fenix) guarda sus gráficos en ese formato: queda pendiente decidir qué se hace con FGC.

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

**Estado:** el intérprete **lee** estos formatos y los convierte a 16 bits (RGB565) al cargar; todavía no los **graba** (no hay superficies de 24/32 bits en memoria). El alfa se aplica de forma **provisional** y simple: alfa < 128 → transparente (color 0); si no, opaco, y el negro opaco pasa a `0x0001` para no confundirse con el transparente. En 24 bits, el negro puro sigue siendo transparente. Qué hacer de verdad con el alfa queda por decidir (ver «negro transparente» en `COLOR-Y-FORMATOS.md`). Se prueba con `tests/simurgh/t_truecolor.prg`.

## Animaciones por nombre (diseño, sin implementar)
Idea tomada de las secuencias de FBM, pero **dentro del FPG** y apoyada en lo que ya tiene el FPG: los identificadores de imagen.

- Una **animación** tiene un **nombre** y una **tabla de fotogramas**; cada fotograma es un **id de gráfico del FPG** más una pausa (ms) y, opcionalmente, flags o ángulo a sumar.
- El FPG se amplía (formato nuevo, el actual no cambia) con un bloque de animaciones; un programa viejo sigue viendo solo los gráficos.
- Desde el lenguaje: reproducir por nombre (`anim("andar")` o similar), sin que el programa gestione a mano el cambio de `graph`. Por decidir: sintaxis, bucles y encadenado («al terminar, seguir con otra»), eventos.
- La definición textual podría ir en el propio programa o en un fichero que se funde en el FPG al construirlo.

Pendiente de diseño detallado antes de tocar el intérprete.
