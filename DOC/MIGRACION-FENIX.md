# Migración de Fenix a Simurgh

Documento vivo. Recoge **todo cambio del lenguaje o de su biblioteca estándar** respecto a Fenix 0.92a, para que quien venga de Fenix (o de DIV) sepa qué cambia en sus programas. Regla de trabajo: el lenguaje casi no se toca, para respetar la filosofía de Fenix y reaprovechar a sus programadores; lo que sí cambia se apunta aquí **en el mismo momento** en que se decide.

No se documentan aquí los cambios internos (formato `.dcb`, estructuras del compilador, intérprete): ver `ARQUITECTURA.md`.

## Resumen

| Área | Fenix 0.92a | Simurgh | Estado |
|---|---|---|---|
| Profundidad de color | 8, 16 y 32 bits, con paletas | solo 16 bits (RGB565); `set_mode` con 8 sigue en 16 y el modo por defecto es de 16 | hecho |
| Paletas | `load_pal`, `set_palette`, `fade`… sobre paleta | desaparecen | decidido |
| Negro puro | transparente en 16 bits | transparente por ahora | por resolver antes de la primera versión |
| Formatos de imagen | PNG, PCX, GIF… | PNG integrado; sin GIF | decidido |
| FPG y MAP | bibliotecas externas o funciones sueltas | integrados en el lenguaje | por diseñar |
| Textos | ISO-8859 / código de página | UTF-8 | decidido |
| Punteros en 64 bits | no soportado (la VM guarda punteros en 32 bits) | por resolver antes de la primera versión | pendiente |

## Sin cambios (compatibilidad)

- Sintaxis del lenguaje: procesos, `BEGIN`/`END`, `FRAME`, `CLONE`, `STRUCT`, `SWITCH`, macros, constantes, etc.
- Formatos **FPG** y **MAP**: 100 % compatibles con Fenix. Un cambio de formato sería un formato nuevo, con otro nombre.
- El compilador acepta los programas de Fenix 0.92a que no usan lo listado como cambiado (comprobado con `tools/fxc-compare.sh`, que compara contra `upstream/fenix-0.92a-fixed`).

## Cambios del lenguaje

Cada entrada: qué era, qué es ahora, cómo migrar.

### Color: solo 16 bits
- **Antes:** `set_mode` admitía 8, 16 y 32 bits; con 8 bits había paleta.
- **Ahora:** solo 16 bits. Los programas de 8 bits deben convertir sus gráficos (herramienta de conversión prevista).
- **Migrar:** quitar llamadas a funciones de paleta; los FPG/MAP de 8 bits se convierten a 16.

### Imágenes
- PNG va integrado; GIF desaparece. **Migrar:** convertir los GIF a PNG.
- Las bibliotecas externas de imagen (`Import "image"` y similares) dejan de hacer falta.

### Texto
- Las cadenas son UTF-8. **Migrar:** guardar los fuentes `.prg` en UTF-8; las fuentes bitmap (`.fnt`) se revisarán aparte.

### Funciones que no existen en Fenix 0.92a
- `LOAD_IMAGE(fichero)`: nueva; detecta el formato por la cabecera (MAP, PNG, PCX). **`LOAD_MAP`, `LOAD_PNG`, `LOAD_PCX` y `LOAD_FBM` son alias** de `LOAD_IMAGE`: el código antiguo compila igual. `Import "image"` se acepta y no hace nada.
- `SAVE_MAP(fpg, grafico, fichero)` y `SAVE_FPG(fpg, fichero)`: nuevas (no existían en 0.92a; las usa FenXEdit2). Graban en los formatos MAP y FPG de Fenix.
- Detalle y lista de formatos en `FORMATOS-IMAGEN.md`.

## Línea de órdenes (programa y opciones)

**Un solo programa, `simr`** (se presenta como «Simurgh»), sustituye a `fxc` y `fxi`:

| Orden | Qué hace |
|---|---|
| `simr programa.dcb [argumentos]` | Ejecuta un programa compilado (lo que hacía `fxi`). |
| `simr programa.prg [argumentos]` | **Compila en memoria** y, si va bien, lo ejecuta de inmediato. No escribe ficheros y la parte de compilación **no imprime nada** si todo va bien; los errores sí se imprimen y se sale con código 2 sin ejecutar. |
| `simr -b programa.prg` | Solo compila: escribe `programa.dcb` (sin salida si va bien). |

Opciones: `-d` depuración (al ejecutar un `.prg` se compila con la información de depurado); `-i dir` directorio de búsqueda de ficheros; al compilar, `-g` (información de depurado en el DCB), `-c` (fuente en caracteres MS-DOS) y `-f fichero...` (incluir ficheros en el DCB). Las letras sin valor se pueden agrupar (`-dg`). **El valor de las opciones va siempre separado por un espacio** (`-i dir`; `-idir` ya no funciona). Lo que va **después del fichero** son argumentos del programa y no se interpreta (ni siquiera si empieza por guion). Códigos de salida al compilar: 0 correcto, 1 error de uso, 2 error de compilación.

Desaparecen respecto a Fenix: de `fxc`, `-s` (ejecutable con el DCB incrustado), `-d` (volcado de depuración del compilador), `-a` (incluir automáticamente los ficheros que abre el programa) y `-l` (idioma; ahora sale de la configuración regional); de `fxi`, `-f` (filtro de 16 bits) y `-r` (no hacía nada). `fxc` devolvía 1 al compilar bien; ahora 0.

### `DECLARE` desaparece: el compilador lee las firmas por su cuenta
El compilador hace **dos pasadas**. La primera lee todo el programa (incluidos los ficheros `include` y las macros, porque usa el mismo analizador) solo para recoger la firma de cada `process` y `function`: número de parámetros, tipos escritos, valores por defecto y tipo de retorno. La segunda es el compilado de verdad y ya conoce todas las firmas, estén definidas antes o después de la llamada. Por eso:

- Se puede llamar a una función que devuelve cadena o real **antes de definirla** sin declararla antes.
- Los valores por defecto y los parámetros omitidos funcionan igual con llamadas anteriores a la definición.
- Los errores de número de parámetros se dan en la línea de la llamada.
- `DECLARE` **ya no existe**: un programa que lo use da el error «DECLARE is not needed in Simurgh: the compiler reads process and function signatures by itself (remove the declaration)»; hay que borrar la declaración (la definición ya basta).

Límites de la primera pasada: solo ve cabeceras escritas con `process` o `function`; un parámetro sin tipo escrito (que toma el tipo de una variable local o global del mismo nombre) se resuelve en la segunda pasada, como siempre; un valor por defecto puede ser un número, un real, una cadena (con signo menos si es numérico) o el nombre de una constante. En Fenix, los ficheros se incluyen con `include "fichero";` (no con `#include`).

### Parámetros opcionales y argumentos vacíos (lenguaje)
Un parámetro puede llevar **valor por defecto**, y entonces se puede omitir en la llamada:

- **Procesos y funciones del usuario:** `function test(a, b=0, c)`, `process p(string s="hola", n=2)`. Los valores por defecto pueden estar **en cualquier posición** (no tienen que ir al final) y ser **enteros, reales o cadenas** (constantes). También valen en `DECLARE`.
- **Funciones del sistema:** se declaran con la firma completa y los valores por defecto (`"IISI|,,,0"` en `include/sysprocs.h`: tipos y, tras `|`, un valor por defecto por parámetro; vacío = obligatorio).

En la llamada se omiten:

- al final: `test(1)` (b y c toman su valor por defecto, si lo tienen);
- en medio, dejando el hueco: `test("uno",,"tres")`; también `test(,2)`, `test(1,)` y `test(,,c)`.

Si se omite un parámetro que **no tiene valor por defecto**, el compilador da un error (`Function F: parameter 2 has no default value` o `Incorrect number of parameters`). Un valor por defecto de otro tipo que el parámetro (una cadena en un entero) también es error. Funciona igual si se llama **antes** de definir el proceso (lo habitual: el programa principal va primero), porque la firma ya se conoce de la primera pasada. Las funciones de bibliotecas externas (`Import`) no usan valores por defecto por ahora.

Nota: el nombre `x` de un parámetro choca con la variable local `x` de los procesos (como en Fenix); use otro nombre.

### Funciones nuevas
- `SAVE_IMAGE(fpg, grafico, fichero [, formato])` y las constantes `IMG_AUTO`, `IMG_MAP`, `IMG_PNG`, `IMG_BMP`, `IMG_TGA` (si un programa antiguo definía un identificador con esos nombres, chocará). `LOAD_IMAGE` carga también BMP y TGA. Ver `FORMATOS-IMAGEN.md`.

### Formatos y funciones que desaparecen
- **FBM**, **FGC** y **FPL**: se retiran `LOAD_FBM`, `SAVE_FBM`, `UNLOAD_FBM`, `LOAD_FGC`, `SAVE_FGC`, `UNLOAD_FGC`, `LOAD_FPL` y `SAVE_FPL` (un programa que las use dejará de compilar). Para migrar: convertir los recursos a MAP o FPG con la herramienta de Fenix 0.92a.
- Nuevos formatos de **MAP y FPG de 24 y 32 bits** (`m24`, `m32`, `f24`, `f32`), que se convierten a 16 bits al cargar; ver `FORMATOS-IMAGEN.md`.
- Animaciones por nombre dentro del FPG: previstas, sin diseñar del todo (`FORMATOS-IMAGEN.md`).

### Códigos de mapa no válidos
- **`UNLOAD_MAP` con un código `<= 0`** (por ejemplo `unload_map(0, 0)`) es ahora un **error de ejecución** ("unload_map: codigo de mapa no valido"). En Fenix `(0, 0)` era el mapa de fondo de pantalla y no hacía nada; un 0 casi siempre indica un mapa que no se pudo crear (p. ej. `write_in_map` con un texto vacío devuelve 0), y callarlo ocultaba errores como pintar el fondo entero sin querer con `map_put(0, dc, 0, x, y)`. El resto de funciones siguen aceptando `(0, 0)` como el fondo, como en Fenix.

### `STRUCT` y `TYPE`
- Un **`struct` en línea** (`struct aaa[3] ... end`) es ahora, por debajo, un `type` con nombre automático `autotype_aaa_struct` y una variable de ese tipo (`autotype_aaa_struct aaa[3]`). Ese nombre se puede usar como cualquier tipo. Si el nombre ya existe (otro proceso con un `struct` del mismo nombre) se numera: `autotype_aaa_struct_2`. `struct` se mantiene por compatibilidad; para tipos reutilizables, `type nombre ... end` y luego `nombre variable;` (sin `struct` delante).
- Los tipos con nombre admiten ya hasta 1024 (antes 64).

### Audio
- `LOAD_WAV` y `LOAD_SONG` devuelven un número pequeño (1, 2, 3…), ya no un puntero, y **`-1` si fallan** (`LOAD_WAV` devolvía `0`). `UNLOAD_WAV`, `UNLOAD_SONG`, `PLAY_WAV`, `PLAY_SONG` y `SET_WAV_VOLUME` con un número que no existe o ya descargado devuelven `-1` sin romper nada.

### Modo 7
- Se pueden apilar varios modos 7 (`start_mode7(0, …)`, `start_mode7(1, …)`…): se pintan por `m7[n].z` (mayor = más al fondo) y un píxel de color 0 no pinta, así que la capa de arriba deja ver la de abajo. Con una `m7[n].height` distinta en cada capa se consigue una pista suspendida sobre el suelo (`tests/simurgh/t_mode7b.prg`, `tools\mode7.cmd b`).
- El modo 7 sigue la proyección del de **DIV**, no la de Fenix 0.92a. Cambios de comportamiento: `m7.height` son unidades de 1/4 de píxel del mapa (el valor 64 equivale a una cámara a 16 píxeles); `m7.focus` **mayor acerca** (antes ensanchaba el campo de visión); `horizon` se respeta (fila de la región donde está el horizonte; antes se ignoraba); `m7.distance` son píxeles detrás del proceso cámara. Los procesos con `ctype = c_m7` se colocan sobre el suelo en su posición real, con la base de su gráfico (o su punto de control 0) en el suelo, se escalan con la distancia (`size·f/(4·Z)` %) y el `angle` ya no gira su gráfico (solo elige la vista de `xgraph`). Funciona en 16 bits. Ver `DOC/PROBLEMAS.md` punto 55.
- **Niebla:** `m7[n].fog_distance` (distancia a la que todo se funde con la niebla, 0 = sin niebla) y `m7[n].fog_color` (color de 16 bits; negro = noche, gris = humo). El suelo y los píxeles de los procesos se mezclan con ese color en proporción a la distancia, y los procesos más allá de `fog_distance` no se pintan. Con `fog_color = -1` no hay color de niebla: el suelo se funde con lo que haya detrás (el fondo o la capa de abajo) y los procesos se desvanecen. Ocupan los dos campos `reserved` de `m7`.
- **Altura:** el suelo del modo 7 está a la altura que marca `m7[n].height` de la cámara, y la **altura de un proceso sobre ese suelo es su variable `z`** (en DIV era la variable `height`, que desaparece como variable local predefinida; con `//$ancient on` `height` es un alias de `z`, de modo que `height = 10; say(z);` escribe 10). Una variable propia llamada `height` tiene preferencia sobre el alias. Para un proceso con `ctype = c_m7` la `z` ya no es la prioridad de dibujo; a la misma distancia de la cámara se dibuja en orden estable (por identificador de proceso), y la `m7[n].z` de la ventana es la que decide el orden respecto a lo demás de la pantalla.
- **`collision()` depende del tipo de coordenadas (`ctype`):** entre procesos de tipos distintos (pantalla, scroll, modo 7) **nunca hay colisión**. En pantalla y en scroll sigue comparando los gráficos píxel a píxel. En el **modo 7** es una colisión de esferas: hay choque si la distancia entre los dos procesos, **contando la altura (`z`)**, es menor que la suma de sus radios. El radio es la nueva variable local **`radius`** (la misma que usaba el modo 8 de DIV); si vale 0 se toma la mitad del ancho del gráfico del proceso, a su `size`. Un proceso sin gráfico colisiona si tiene `radius`. Los procesos que están en ventanas de modo 7 distintas (`cnumber`) no colisionan.

### Modo 8 (nuevo)
- **Mapas de sectores en 2,5D** (como el modo 8 de DIV 2, que Fenix no tenía). Funciones nuevas: `load_wld`, `save_wld`, `start_mode8`, `stop_mode8`, `go_to_flag`, `set_sector_height`, `get_sector_height`, `set_sector_texture`, `get_sector_texture`, `set_wall_texture`, `get_wall_texture`, `set_point_m8`, `get_point_m8`, `set_fog` y `set_env_color`. Estructura global nueva `m8[9]` (`z`, `camera`, `height`, `angle`), variables locales nuevas `m8_wall`, `m8_sector`, `m8_nextsector`, `m8_step` y `m8_height`, y constante `c_m8` (3). Un programa que usara esos nombres para algo propio tiene que cambiarlos.
- El mapa es un **fichero de texto** (`simurgh-wld 1`); los `.wld` de DIV 2 se importan. Detalle en `MODO8.md`.
- `collision()` entre procesos `c_m8` funciona como en el modo 7 (distancia con `z` y suma de `radius`), con la `z` en unidades del mapa.

### Funciones de DIV
- Sonido de DIV (para poder compilar juegos de DIV 2): `load_pcm(fichero [, repetir])`, `sound(id, volumen, frecuencia)`, `stop_sound(canal)`, `change_sound(canal, volumen, frecuencia)`, `change_channel(canal, volumen, paneo)`, `is_playing_sound(canal)`, `reset_sound()` y `unload_pcm` (= `unload_wav`). Volumen 0-511 (256 normal, no se amplifica por encima), frecuencia ignorada, paneo 0-255.
- Un entero que guarda la dirección de una tabla (`[variable+3]`) debe declararse `POINTER`; en DIV todo eran enteros.
- Los juegos de DIV están en CP850: compilarlos con `-c`.

### `get_id`
- `get_id(0)` y `get_id(type proceso)` **no devuelven nunca al proceso que los llama** (como en DIV; Fenix 0.92a sí podía devolverlo). Un proceso único de su tipo ya no se encuentra a sí mismo.

### Modo de 256 colores y carga de gráficos
- **No hay modo de 256 colores.** El modo por defecto es de 16 bits y `set_mode(ancho, alto, 8)` también lo deja en 16.
- Un gráfico de **8 bits** (MAP, FPG, FNT, PCX de DIV) se carga como una **copia de 16 bits**, convertida una sola vez con la paleta del propio fichero. La copia es la que se dibuja, se modifica y se graba: `save_map` y `save_fpg` escriben 16 bits. El índice 0 sigue siendo transparente.
- Por eso `map_get_pixel` sobre un gráfico cargado de 8 bits devuelve un **color de 16 bits**, no un índice de paleta, y `new_map(ancho, alto, 8)` crea un mapa de 16 bits. `m7.color` de 1 a 255 sigue siendo un índice de la paleta actual.

### Paletas, FLI y `//$ancient on`
- **Se quitan** las animaciones FLI (`start_fli`, `frame_fli`, `end_fli`, `reset_fli`) y las funciones de paleta `save_pal`, `roll_palette`, `convert_palette` y `find_color`. No hay paleta de pantalla: un gráfico de 8 bits se convierte al cargarlo con la paleta de su propio fichero (ver arriba).
- `m7.color` es un color de 16 bits (`rgb()`), no un índice.
- **`//$ancient on`** (en un comentario de una línea, o de bloque con `$ancient on`; `off` la apaga) habilita `load_pal`, `set_colors` y `get_colors` como funciones que **no hacen nada** (devuelven 1), para que un programa de DIV compile sin tocar esas llamadas. Sin la directiva dan un error que lo explica. También habilita los **alias de nombres** antiguos (`load_map`, `load_png`, `load_pcx`, `unload_pcm`); sin la directiva se usan `load_image` y `unload_wav`.

### Por rellenar
Aquí se irán apuntando: funciones eliminadas o con otro comportamiento, nuevas funciones de `map` y `fpg`, la solución al negro transparente y cualquier otra decisión que afecte a programas existentes.

## Herramientas que acompañan

- **FenXEdit2** es parte del proyecto (IDE escrito en Fenix, que también hay que migrar al dialecto de Simurgh): ver `IDE-FENXEDIT2.md`.
