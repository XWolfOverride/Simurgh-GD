# El modo 8 de DIV Games Studio 2

Documentación de **cómo funcionaba el modo 8 de DIV 2**, hecha a partir de su ayuda electrónica (`HELP.DIV`, castellano) y del formato de
fichero `.wld`. El modo 8 de **Simurgh** sigue este modelo con un formato de mapa propio y algunas diferencias: está en
[MODO8.md](MODO8.md). El nombre en DIV es «modo 8» por seguir al modo 7 (plano abatido).

## Qué es

Un **motor 2,5D de mapas de sectores** al estilo de Doom o Duke Nukem 3D: el mundo es un plano 2D dividido en **sectores** (regiones
poligonales), cada uno con una altura de suelo y otra de techo, y las **paredes** son los lados de esos polígonos. Se ve en primera persona
desde una cámara; los suelos y techos se dibujan con perspectiva y las paredes como columnas con textura. Los objetos (procesos) son
gráficos planos que se escalan con la distancia (sprites), como en el modo 7.

- El mapa se construye con el **editor de mapas tridimensionales** de DIV y se guarda en un fichero `.wld`.
- Las **texturas** (de paredes, suelos y techos) son gráficos de un único `.fpg` que se carga antes del mapa.
- Una **ventana de modo 8** es una región rectangular de la pantalla que muestra el mapa. Puede haber **hasta 10** (números 0 a 9), cada
  una con su cámara, como en el modo 7.
- Es un modo «avanzado»: la ayuda original avisa de que requiere algo de experiencia.

## Cómo se usa

```
fichero = load_fpg("texturas.fpg");          // texturas del mapa
load_wld("mapa.wld", fichero);               // carga el mapa de sectores
start_mode8(id, 0, 0);                       // ventana 0, en toda la pantalla (región 0); el proceso actual es la cámara
ctype = c_m8;                                // el proceso vive dentro del mapa
height = 64;  radius = 64;  m8.height = 32;  // altura del proceso, radio y altura de los ojos
go_to_flag(0);                               // se coloca en la bandera 0 del mapa
loop
    if (key(_right)) angle -= 5000; end;     // girar
    if (key(_up))    advance(24);   end;     // avanzar (choca con las paredes)
    if (key(_q))     z += 16;       end;     // subir
    if (key(_w))     m8.angle += 8; end;     // mirar hacia arriba
    frame;
end
```

La cámara está en las coordenadas `(x, y)` del proceso, a su altura `z` más la altura de los ojos (`m8.height`), mirando en la dirección
`angle`; el ángulo vertical es `m8.angle`.

## Funciones

| Función | Qué hace |
|---|---|
| `load_wld(fichero, fpg)` | Carga un mapa de sectores `.wld`; `fpg` es el identificador del `.fpg` de texturas (cargado antes con `load_fpg`). |
| `start_mode8(camara, n, region)` | Crea la ventana `n` (0-9) en la región `region` (0 = toda la pantalla; las demás se definen antes con `define_region`). `camara` es el proceso donde se coloca la cámara. |
| `stop_mode8(n)` | Elimina la ventana `n`. |
| `go_to_flag(n)` | El proceso que llama se sitúa, en el siguiente `frame`, en las coordenadas de la **bandera** `n` del mapa. Su altura es la mínima del sector en que cae la bandera. Equivale a asignar `x`, `y`, `z` a mano. |
| `set_sector_height(sector, suelo, techo)` | Cambia la altura del suelo y del techo (valores de 0 a 4096; con `-1` se mantiene la actual). El techo no puede quedar por debajo del suelo; si son iguales, el sector es **infranqueable** (dentro de otro sector se ve como una columna). |
| `get_sector_height(sector, &suelo, &techo)` | Devuelve las alturas (por referencia). |
| `set_point_m8(vertice, x, y)` | Mueve un vértice del mapa (coordenadas de 0 a 30200). **Peligrosa**: puede producir errores de visibilidad. Un vértice solo debe moverse dentro de su sector, sin cruzar otros vértices ni líneas, y no deben moverse sectores que contengan procesos. |
| `get_point_m8(vertice, &x, &y)` | Devuelve las coordenadas de un vértice. |
| `set_sector_texture(sector, suelo, techo, luz)` | Cambia las texturas de suelo y techo (número de gráfico en el `.fpg` de texturas) y la **luminosidad** (0 = color de ambiente, normalmente negro; 15 = colores originales). Con `-1` se mantiene lo actual. |
| `get_sector_texture(sector, &suelo, &techo, &luz)` | Devuelve las texturas y la luminosidad de un sector. |
| `set_wall_texture(pared, textura, luz)` | Cambia la textura y la luminosidad de una pared (`-1` mantiene lo actual). |
| `get_wall_texture(pared, &textura, &luz)` | Devuelve la textura y la luminosidad de una pared. |
| `set_fog(inicio, fin)` | **Niebla**: `inicio` es el porcentaje (0-100) de distancia desde el que empieza el efecto y `fin` aquel en que es totalmente opaca. Lo normal es que `fin` sea el doble de `inicio`; cuanto más separados, más lenta es la entrada de la niebla. |
| `set_env_color(r, g, b)` | Color del ambiente o de la niebla, en porcentaje (0 a 100) por componente. Por defecto negro. |

Los números de sector, pared y vértice son los que muestra el editor de mapas; las banderas se colocan también en el editor.

## La estructura global `m8`

Diez registros (`m8[0]` a `m8[9]`, uno por ventana; `m8.campo` es `m8[0].campo`) con los campos que acompañan a los parámetros de
`start_mode8`:

| Campo | Significado | Por defecto |
|---|---|---|
| `camera` | Identificador del proceso en el que está la cámara (imprescindible; `start_mode8` lo rellena). | — |
| `height` | Altura de la cámara respecto a la base del proceso (la altura de «los ojos»). | 32 |
| `angle` | Ángulo **vertical** de la cámara, de -128 a 128 (0 = mira al centro). El ángulo horizontal es el `angle` del proceso cámara. | 0 |
| `z` | Prioridad de impresión de la ventana frente a los demás procesos (como `m7[n].z`). | 256 |

Con `z = 256` y los procesos normales a `z = 0`, la ventana queda al fondo y los gráficos de los procesos se ven por encima; para cambiarlo
se baja el `z` de la ventana (por ejemplo a -1) o se sube el de los procesos (por ejemplo a 257).

## Procesos en el modo 8

Un proceso pertenece al modo 8 con `ctype = c_m8` (constante de valor **3**; `c_screen`, `c_scroll`, `c_m7` y `c_m8` son los cuatro tipos de
coordenadas). Entonces:

- `x`, `y` son un punto del mapa de sectores y **`z` es la altura** (en DIV el modo 7 usaba `height` para la altura del proceso; Simurgh usa `z` en los dos).
- `height` es la altura del propio proceso y `radius` su radio (ancho): el sistema lo usa para las colisiones con las paredes. Con un radio
  menor el proceso se acerca más a las paredes y los puntos (texels) de las texturas se ven más grandes.
- `cnumber` limita en qué ventanas se ve un proceso (por defecto en todas); `xgraph` permite varias vistas del gráfico según el ángulo desde
  el que se mira; `size` y `flags` (espejados y transparencia) se aplican como siempre.
- El sistema comprueba las colisiones de estos procesos con las paredes del mapa (al chocar rellena `m8_wall` y `m8_nextsector`, ver abajo); `advance()` es la forma de mover la cámara hacia delante.

El sistema mantiene automáticamente estas variables locales de los procesos del modo 8:

| Variable | Contenido |
|---|---|
| `m8_sector` | Número del sector en el que está el proceso (útil para disparar eventos al entrar en un sector). |
| `m8_wall` | Número de la pared contra la que ha chocado (útil para puertas, interruptores, zonas activas). |
| `m8_nextsector` | Sector que hay tras la pared con la que ha chocado, si lo hay (permite consultar sus alturas para decidir si hay que subir o bajar). |
| `m8_step` | Altura máxima que el proceso puede subir de un sector a otro contiguo, «la altura de escalón». Si el sector contiguo es más alto que eso, la pared lo detiene; si no, su altura se ajusta sola. |
| `reserved.m8_object` | Número de objeto relativo al mapa (variable interna). |

## Colisiones

`collision()` **no funciona** entre procesos del modo 7 o del modo 8 (ni con procesos sin gráfico). Para detectar choques entre procesos de
estos modos se usa `get_dist()` y se compara con la distancia de choque que se elija.

## Formato de fichero `.wld`

Datos tomados de la descripción del formato `.wld` (little endian). Hay dos bloques: el mapa en sí y, a partir de un desplazamiento, una
copia con los datos del editor.

**Cabecera:** firma `wld` `1A 0D 0A 01 00`; un entero de 4 bytes con el desplazamiento del bloque del editor; ruta del `.wld` (256 bytes) y
nombre (16); un número de mapa; ruta del `.fpg` de texturas (256 bytes) y su nombre (16).

**Mapa:** cuatro listas, cada una precedida por su número de elementos (enteros de 32 bits), y al final un entero con la textura del fondo.

| Lista | Campos de cada elemento (32 bits, `s` con signo) |
|---|---|
| Puntos (vértices) | `activo`, `x` (s), `y` (s), `enlaces` |
| Paredes | `activo`, `tipo` (s), `p1` (s), `p2` (s), `region_frente` (s), `region_atras` (s), `textura` (s), `textura_arriba` (s), `textura_abajo` (s), `fade` (0-16) |
| Regiones (sectores) | `activo`, `tipo` (s), `altura_suelo` (s), `altura_techo` (s), `textura_suelo` (s), `textura_techo` (s), `fade` (0-16) |
| Banderas | `activo`, `x` (s), `y` (s), `numero` (s) |

Una pared une dos puntos (`p1`, `p2`) y separa dos regiones (frente y atrás; sin región al otro lado es una pared sólida). Las texturas de una
pared son tres, para la parte central y para las franjas superior e inferior que quedan cuando el sector de al lado tiene otra altura. El
campo `fade` es la luminosidad (0 a 16 en el fichero; la función `set_*_texture` usa 0 a 15).

**Bloque del editor** (en el desplazamiento indicado, con la marca `DAT`): números de puntos, regiones, paredes y banderas (enteros de 16 bits),
los mismos elementos con campos de 16 bits (incluyen nombres de efecto y etiquetas que siempre valen lo mismo), título, nombre de la paleta,
texturas de pantalla y de fondo, ángulo del fondo, vista activa y una «fuerza» por defecto.

El significado exacto de `tipo` en paredes y regiones, y del campo `enlaces`, no está en la ayuda.

## Lo que la ayuda no explica

- El algoritmo de dibujo (ordenación de sectores, recorte de paredes, cómo se dibuja el cielo o fondo) y la proyección del ángulo vertical.
- Cómo se calculan las colisiones con `radius`, y cómo se mueve un proceso entre sectores de distinta altura más allá de `m8_step`.
- El significado de `tipo`, `enlaces` y de las etiquetas del bloque del editor del `.wld`.
- El manejo del editor de mapas tridimensionales (la ayuda lo remite al manual de usuario).

## El modo 8 de Simurgh

Lo decidido para Simurgh (formato de mapa de texto, 16 bits, `m8_height`, `collision()` entre procesos, importación de los `.wld`
de DIV) está en [MODO8.md](MODO8.md).
