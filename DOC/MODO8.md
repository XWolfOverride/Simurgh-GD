# El modo 8 de Simurgh

Motor **2,5D de mapas de sectores** (al estilo de Doom o Duke Nukem 3D), en 16 bits de color. Sigue el modelo del modo 8 de DIV 2
(descrito en [MODO8-DIV.md](MODO8-DIV.md)): las mismas funciones, la estructura `m8` y `ctype = c_m8`. Lo que cambia es el **formato
del mapa**, que es nuevo, y algunos detalles que se listan al final. Código: `src/fxi/src/g_mode8.c`.

## Qué es

El mapa es un plano dividido en **sectores**: polígonos (no hace falta que sean convexos, y uno puede estar dentro de otro) con una
altura de suelo y otra de techo. Las **paredes** unen dos vértices y separan dos sectores (pared de paso) o un sector y nada (pared
sólida). Si los dos sectores de una pared de paso tienen alturas distintas se ven los **escalones**: la franja de abajo (el suelo del
otro sector es más alto) y la de arriba (su techo es más bajo, un dintel). Las texturas son gráficos de un fichero (FPG) y los objetos
son procesos con `ctype = c_m8`, dibujados como gráficos planos que se escalan con la distancia.

```
tex = load_fpg("texturas.fpg");
load_wld("mapa.wld", tex);
ctype = c_m8;                      // el proceso vive dentro del mapa
radius = 24; m8_height = 56;       // ancho y alto del proceso, para las colisiones con las paredes
start_mode8(id, 0, 0);             // ventana 0, a toda la pantalla, con la cámara en este proceso
go_to_flag(0);
loop
    if (key(_left))  angle += 5000; end;
    if (key(_right)) angle -= 5000; end;
    if (key(_up))    advance(8); end;       // las paredes lo detienen
    frame;
end
```

## Formato del mapa (`simurgh-wld 1`)

Un fichero de **texto**, pensado para escribirlo a mano, generarlo con un programa o leerlo en un control de versiones. Una orden por
línea; los campos se separan con espacios; `#` empieza una línea de comentario. La primera línea útil es la firma `simurgh-wld 1`.
Los elementos de cada clase se **numeran desde 0 en el orden en que aparecen**, y esos números son los que usan las paredes y las
funciones del lenguaje. No guarda rutas ni paleta ni el nombre del fichero de texturas (se pasa a `load_wld`).

| Orden | Significado |
|---|---|
| `vertex x y` | Un vértice. Admite decimales. La `y` crece hacia abajo, como en pantalla. |
| `sector suelo techo tex_suelo tex_techo [luz] [etiqueta]` | Un sector: alturas, texturas del suelo y del techo, luz (0 a 15; por defecto 15) y una etiqueta libre. Con textura 0 no se pinta: en el techo se ve el fondo (`sky`) o lo que haya detrás de la ventana. Si el techo no está por encima del suelo, el sector es infranqueable (una columna). |
| `wall v1 v2 delante [detras] [tex] [tex_arriba] [tex_abajo] [luz] [etiqueta]` | Una pared entre los vértices `v1` y `v2`. `delante` y `detras` son los sectores que separa; `detras` = -1 (por defecto) es una pared sólida. `tex` es la textura de la pared sólida; `tex_arriba` y `tex_abajo`, las de los escalones de una pared de paso (si alguna vale 0 se usa otra de las tres que no lo sea; así se pintan también los lados de un sector infranqueable). Cada tramo se escribe **una sola vez**, da igual desde qué sector. |
| `flag numero x y` | Una bandera: un punto con nombre numérico para `go_to_flag`. |
| `scale n` | Unidades del mapa que ocupa un punto de textura (por defecto 1). |
| `sky n` | Gráfico de fondo para los techos sin textura (0 = ninguno). Gira con la cámara. |
| `ambient r g b` | Color de ambiente y de niebla, de 0 a 255 (por defecto negro). |
| `fog inicio fin` | Niebla, como `set_fog` (0 0 = sin niebla). |

Ejemplo mínimo (una sala cuadrada): `tests/simurgh/t_mode8.wld` tiene uno con tres sectores.

```
simurgh-wld 1
vertex 0 0
vertex 400 0
vertex 400 400
vertex 0 400
sector 0 200 1 2
wall 0 1 0 -1 3
wall 1 2 0 -1 3
wall 2 3 0 -1 3
wall 3 0 0 -1 3
flag 0 200 200
```

Un sector queda definido por las paredes que lo nombran (como `delante` o `detras`); no hay lista de vértices por sector ni orden de
giro obligatorio. Un sector interior (una columna, una tarima) son paredes de paso cuyo otro lado es el sector que lo rodea.

**Mapas de DIV 2:** `load_wld` reconoce también los `.wld` de DIV por su cabecera y los importa (vértices, sectores, paredes, banderas,
texturas y luz; cada punto de textura ocupa 2 unidades). `save_wld(fichero)` graba el mapa cargado en el formato de texto, así que
cargar uno de DIV y grabarlo es la conversión. Los `.wld` de DIV traen cada pared de paso dos veces (una por sector): al cargar se usa
la primera y la otra queda sin efecto, para conservar la numeración. No se importan los efectos ni el fondo del editor.

## Funciones

| Función | Qué hace |
|---|---|
| `load_wld(fichero, fpg)` | Carga un mapa (texto de Simurgh o `.wld` de DIV). `fpg` es el fichero de texturas. Devuelve 1, o 0 si no se pudo. Solo hay un mapa cargado. |
| `save_wld(fichero)` | Graba el mapa cargado, con los cambios hechos, en el formato de texto. |
| `start_mode8(camara, n, region)` | Abre la ventana `n` (0 a 9) en una región, con la cámara en el proceso `camara`. `n` y `region` valen 0 si se omiten. |
| `stop_mode8(n)` | Cierra la ventana `n`. |
| `go_to_flag(n)` | Coloca al proceso que llama en la bandera `n`, a la altura del suelo. Devuelve 0 si no existe. |
| `set_sector_height(sector, suelo, techo)` / `get_sector_height(sector, &suelo, &techo)` | Alturas de un sector (-1 mantiene la actual). |
| `set_sector_texture(sector, suelo, techo, luz)` / `get_sector_texture(sector, &suelo, &techo, &luz)` | Texturas y luz de un sector (-1 mantiene). |
| `set_wall_texture(pared, textura, luz)` / `get_wall_texture(pared, &textura, &luz)` | Textura y luz de una pared (-1 mantiene). |
| `set_point_m8(vertice, x, y)` / `get_point_m8(vertice, &x, &y)` | Mueve o consulta un vértice. |
| `set_fog(inicio, fin)` | Niebla: porcentajes de la distancia máxima (4096 unidades) en que empieza y en que lo tapa todo. `set_fog(0, 0)` la quita. |
| `set_env_color(r, g, b)` | Color de ambiente y de niebla, en porcentajes (0 a 100). |

Las funciones `set_*` y `get_*` devuelven 0 si el sector, la pared o el vértice no existen.

## La estructura `m8` y los procesos

`m8[0]` a `m8[9]`, una por ventana: `camera` (el proceso de la cámara), `height` (altura de los ojos sobre la `z` del proceso; 32),
`angle` (mirada arriba o abajo, de -128 a 128, desplazando el horizonte) y `z` (orden de dibujo de la ventana; 256).

Para un proceso con `ctype = c_m8` (constante 3): `x`, `y` son un punto del mapa, **`z` es la altura** y `angle` la dirección.
`cnumber` limita las ventanas en que se ve; `xgraph` elige la vista según el ángulo desde el que se mira; `size`, y los `flags` de
espejo y transparencia, se aplican. El gráfico se apoya en su punto de control 0 o, si no lo tiene, en el centro de su base, y un
punto del gráfico mide lo mismo que un punto de textura (`scale`).

| Variable local | Uso |
|---|---|
| `radius` | Radio del proceso: distancia a la que lo detienen las paredes (y radio de `collision()`). |
| `m8_height` | Alto del proceso: no entra en un sector donde no cabe (32 por defecto). |
| `m8_step` | Escalón máximo que sube de un sector a otro (32 por defecto). |
| `m8_sector` | Sector en el que está (-1 fuera del mapa). Lo mantiene el sistema. |
| `m8_wall`, `m8_nextsector` | Pared con la que ha chocado en este fotograma y sector que hay detrás (-1 si no hay choque o es sólida). |

**Movimiento:** el programa mueve el proceso como quiera (`advance`, `xadvance`, `x += ...`) y en cada `frame` el sistema compara su
posición con la del fotograma anterior y resuelve el camino: lo separa de las paredes que no puede atravesar, lo pasa de sector por
las paredes de paso (subiéndole la `z` si hay escalón) y no le deja bajar del suelo ni atravesar el techo. Cuentan las paredes de su
sector y las de los sectores contiguos que pisa con su radio, así que se queda a la distancia de `radius` de la pared y resbala por
ella sin tirones. La posición se lleva con decimales por dentro, aunque `x`, `y` sean enteras. La `z` por encima del
suelo es libre (no hay gravedad: se programa). Para **saltar** a otro punto sin recorrer el camino: `go_to_flag`, o asignar `x`, `y`
y poner `m8_sector = -1`.

**Colisiones entre procesos:** `collision()` funciona entre procesos `c_m8` por distancia (contando la `z`) y suma de radios, como en
el modo 7.

## Diferencias con el modo 8 de DIV

- Formato de mapa nuevo, de texto y de un solo bloque (el `.wld` de DIV guarda el mapa dos veces, para el editor y para el motor, con
  rutas absolutas y nombre de paleta); los de DIV se importan.
- 16 bits: la luz (0 a 15) y la niebla son una mezcla con el color de ambiente, no un cambio de paleta.
- La altura del proceso para las colisiones es `m8_height` (en DIV, `height`).
- `collision()` funciona entre procesos del modo 8.
- `save_wld` no existía. No hay editor de mapas.
- Las paredes de paso no tienen textura central (rejas, cristales) ni hay efectos de textura.
- El campo de visión horizontal es de 90 grados y la mirada vertical desplaza el horizonte (no inclina la perspectiva).

## Pruebas

`tests/simurgh/t_mode8.prg` (con `tools/image-test.sh`): carga, dibujo de suelo, techo, pared y escalones, procesos visibles y
tapados, colisiones, escalones, funciones de consulta y cambio, luz, niebla y grabación. `tools\mode8.cmd` lo abre para recorrerlo
(W/S avanzar, A/D paso lateral, flechas girar, Q/Z subir y bajar, R/F mirar arriba y abajo); `tools\mode8.cmd div` abre un visor de
un mapa de DIV si se ha dejado en `build\zig-probe\div2\wld` (`view.prg`, `WLD_VIEW.WLD` y `WLD_VIEW.FPG`).
