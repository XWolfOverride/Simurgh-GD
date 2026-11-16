# Migración de DIV Games Studio 2 a Simurgh

Documento vivo. Recoge lo que hay que cambiar (o lo que ya funciona) para llevar un programa de **DIV 2** a Simurgh. Complementa a
[`MIGRACION-FENIX.md`](MIGRACION-FENIX.md): Simurgh parte de Fenix, que ya era un camino de DIV a sistemas actuales, y por eso muchos
cambios de DIV a Fenix siguen valiendo. Aquí se apuntan solo los que se han **comprobado con programas reales** o se han decidido.

Programas de DIV 2 probados hasta ahora:

| Programa | Estado |
|---|---|
| `SPEED` (juego de carreras, 1997) | compila y se juega en 16 bits; adaptado: sin `load_pal` y con `m7.color` en `rgb()` (era el índice 203 de la paleta, que era verde en un nivel y arena en otro), y el mapa de durezas se lee por color y no por índice de paleta |

## Cómo se prueba un programa de DIV

1. **Una sola carpeta.** DIV organiza los ficheros **por formato** y dentro de cada formato por proyecto (`FPG\SPEED\`, `PCM\SPEED\`,
   `FNT\SPEED\`, `PRG\JUEGOS\SPEED.PRG`…), y las rutas relativas del programa parten siempre de la carpeta de instalación de DIV (por
   ejemplo `load_fpg("speed\speed.fpg")` busca en `FPG\speed\speed.fpg`). Simurgh busca cada ruta relativa desde el directorio de
   trabajo y en los directorios de `-i`, sin esa búsqueda por formato. Para probar un juego se reúne todo en una carpeta: el `.prg` y
   una subcarpeta con el nombre del proyecto (`speed\`) con los FPG, FNT, PCM… que cargue.
2. **Compilar y ejecutar** desde esa carpeta: `simr -c programa.prg`.
3. **`-c`:** los programas de DIV están en **CP850** (el juego de caracteres de MS-DOS); `-c` le dice al compilador que lo lea así.
   Sin él, los acentos de los textos salen mal.
4. Las rutas con `\` funcionan en Windows. En Linux y macOS hay que escribirlas con `/` (pendiente: que el cargador acepte `\`).

## Cambios necesarios en el código

### Direcciones de memoria y punteros

En DIV todas las variables son enteros y `[expresión]` lee la memoria en esa posición; una tabla (por ejemplo la de `xgraph`) se
guarda en una variable entera. En Simurgh `[expresión]` exige un **puntero**: la variable que guarde una dirección (de una tabla o de
otra variable) hay que declararla `POINTER`.

```
// DIV
GLOBAL   primero; segundo; tercero; cuarto;      // guardan xgraph (dirección de una tabla)
...      objeto2([cuarto+3], 280, 220, 1);
// Simurgh
GLOBAL   POINTER primero; POINTER segundo; POINTER tercero; POINTER cuarto;
```

No es un cambio que se pueda automatizar sin mirar el código: las direcciones de DIV eran **índices** de su memoria y las de Simurgh
son direcciones reales, así que convertir un entero en puntero a ciegas daría otros valores. Error que se obtiene si falta:
`Pointer required`.

### Sonido

Fenix no tenía las funciones de sonido de DIV; Simurgh las incluye (volumen 0–511, 256 es el normal; SDL_mixer solo llega a la mitad,
así que no se amplifica por encima de 256; **la frecuencia se acepta pero se ignora**, SDL_mixer no cambia la frecuencia de un sonido
que ya suena; el paneo va de 0 a 255):

| DIV | Simurgh |
|---|---|
| `load_pcm(fichero, repetir)` | igual (`repetir` es opcional: 0 por defecto) |
| `unload_pcm(id)` | alias de `unload_wav` |
| `sound(id, volumen, frecuencia)` | igual; devuelve el canal |
| `stop_sound(canal)` | igual (`-1` para todos los canales) |
| `change_sound(canal, volumen, frecuencia)` | igual (la frecuencia se ignora) |
| `change_channel(canal, volumen, paneo)` | igual |
| `is_playing_sound(canal)` | igual |
| `reset_sound()` | igual (para todos los canales) |

Los ficheros `.pcm` de DIV son WAV y se cargan como tales. `load_song(fichero, repetir)` de DIV tiene dos parámetros y el de Simurgh
uno (`load_song(fichero)`): **pendiente**.

### Gráficos, paletas y color

- **No hace falta cambiar nada para pasar a 16 bits:** el modo de 256 colores no existe y el modo por defecto es de 16 bits (`set_mode(ancho, alto)` basta; `set_mode(ancho, alto, 8)` también deja 16 bits). Los FPG, MAP, FNT y PCX de 8 bits se cargan como una **copia de 16 bits convertida una sola vez con la paleta del propio fichero**; la copia es la que se dibuja, se modifica y se graba (`save_map`/`save_fpg` escriben 16 bits). Con eso el recuadro del menú de `SPEED` se ve bien: no depende de la paleta actual.
- **Lo que sí hay que mirar:** el código que trata los colores como **índices de paleta**. `map_get_pixel` sobre un gráfico cargado devuelve un color de 16 bits (en `SPEED`, el mapa de durezas se comparaba con índices 192-207 para detectar la hierba: se detecta por color, por ejemplo con `get_rgb`); `new_map(ancho, alto, 8)` crea un mapa de 16 bits; los colores literales (`draw_*`, `map_clear`…) con números de paleta ya no valen y se escriben con `rgb()`. `m7.color` de 1 a 255 sigue siendo un índice de la paleta actual (`load_pal`).
- **Paletas:** ya no existen (`load_pal`, `roll_palette`, `convert_palette`, `find_color`, `save_pal` se quitan). Para que un programa de DIV compile sin tocar sus llamadas a `load_pal`, `set_colors` y `get_colors`, se escribe `//$ancient on` al principio: pasan a ser funciones que no hacen nada. `m7.color` es un color de 16 bits (`rgb()`), no un índice de paleta. Las animaciones FLI (`start_fli`…) también se quitan.
  `MIGRACION-FENIX.md`.
- **Modo 7.** El modo 7 de Simurgh sigue la proyección del de DIV (no la de Fenix): ver «Modo 7» en `MIGRACION-FENIX.md`. Un programa de
  DIV con modo 7 debería verse como en DIV (altura en cuartos de píxel, `focus` mayor acerca, `horizon` respetado, procesos con
  `ctype = c_m7` colocados y escalados como en DIV). **Pendiente de comprobar** con un juego de DIV real que lo use.

### Lenguaje

- **`get_id`** no devuelve al proceso que llama (como en DIV); ver `MIGRACION-FENIX.md`.
- **`height`** (la altura de un proceso del modo 7) es ahora `z`; con `//$ancient on` `height` es un alias de `z`, así que un programa de DIV que la use compila sin cambios (SPEED).
- **`DECLARE`** no existe (ni hace falta): el compilador lee las firmas de procesos y funciones en una primera pasada.
- **Parámetros opcionales** y **estructuras con nombre** (`TYPE`): ver `MIGRACION-FENIX.md`; un programa de DIV no los usa.

### Modo 8

Está implementado (`MODO8.md`) con las funciones de DIV y los mismos nombres. Para llevar un programa:

- **El mapa:** `load_wld` carga directamente los `.wld` de DIV 2; `save_wld` los graba en el formato de texto de Simurgh.
- **`height`** (el alto del proceso para las colisiones) pasa a ser **`m8_height`**. Con `//$ancient on`, `height` es un alias de `z`, así que en un programa del modo 8 hay que cambiarlo a mano.
- **`get_sector_height`, `get_point_m8`, `get_sector_texture`, `get_wall_texture`:** los parámetros de salida son punteros (`&variable`; `OFFSET variable` también vale).
- La luz y la niebla se mezclan con el color de ambiente (no hay paleta); `set_fog` y `set_env_color` usan los mismos porcentajes.
- Las paredes de paso con textura central (rejas) y los efectos de textura no se dibujan.

## Funciones de DIV que Simurgh todavía no tiene

Comparando la lista de funciones de DIV 2 con las de Simurgh faltan (no se han usado en `SPEED`, así que no se ha comprobado cuáles
importan):

- **Cadenas** (en DIV eran funciones; en Simurgh las cadenas son un tipo): `strcpy`, `strcat`, `strlen`, `strcmp`, `strchr`, `strstr`,
  `strset`, `strupr`, `strlwr`, `strdel`. Se pueden sustituir por el operador `=`, `+`, `len`, `ucase`, `lcase`, `find`, `substr`…
  `get_sector_texture`, `set_wall_texture`, `get_wall_texture`, `set_point_m8`, `get_point_m8`, `set_fog`, `set_env_color`,
  `go_to_flag`, `load_wld`.
- **Red:** `net_join_game`, `net_get_games`.
- **Sistema y ficheros:** `system`, `remove`, `filelength`, `flush`, `get_dirinfo`, `get_fileinfo`, `getdrive`, `setdrive`, `disk_free`,
  `ignore_error`, `malloc` (Simurgh tiene `alloc`), y las de compresión y cifrado (`encode`, `encode_file`, `decode_file`,
  `compress_file`, `uncompress_file`).
- **Música:** `song`, `set_song_pos`, `get_song_pos`, `get_song_line`, `play_cd`, `stop_cd`, `is_playing_cd`, `set_volume`.
- **Gráficos:** `set_color`, `screen_copy`, `load_screen`, `save_pcx`, `draw`, `char`, `refresh_scroll`, `path_line`, `path_free`,
  `x_advance`, `calculate`, `atan2`, `force_pal`.

## Cómo seguir

Cada juego de DIV que se pruebe amplía este documento: qué cambió en el código, qué función hubo que añadir y qué quedó mal. Los
ejemplos originales de DIV 2 (`ALIEN`, `BILLAR`, `BLASTUP`, `CHAPAS`, `CHECKOUT`, `DARK`, `FOST3D`, `FOSTIATO`, `GALAX`, `HELIOBAL`,
`MALVADO`, `NOID`, `PACOMAN`, `PUZZLE`, `SOCCER`, `STEROID`, `TOKENKAI`, `WSPORTX`) son la lista de pendientes.
