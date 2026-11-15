# Migración de Fenix a Simurgh

Documento vivo. Recoge **todo cambio del lenguaje o de su biblioteca estándar** respecto a Fenix 0.92a, para que quien venga de Fenix (o de DIV) sepa qué cambia en sus programas. Regla de trabajo: el lenguaje casi no se toca, para respetar la filosofía de Fenix y reaprovechar a sus programadores; lo que sí cambia se apunta aquí **en el mismo momento** en que se decide.

No se documentan aquí los cambios internos (formato `.dcb`, estructuras del compilador, intérprete): ver `ARQUITECTURA.md`.

## Resumen

| Área | Fenix 0.92a | Simurgh | Estado |
|---|---|---|---|
| Profundidad de color | 8, 16 y 32 bits, con paletas | solo 16 bits (RGB565) | decidido |
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

### Por rellenar
Aquí se irán apuntando: funciones eliminadas o con otro comportamiento, nuevas funciones de `map` y `fpg`, la solución al negro transparente y cualquier otra decisión que afecte a programas existentes.

## Herramientas que acompañan

- **FenXEdit2** es parte del proyecto (IDE escrito en Fenix, que también hay que migrar al dialecto de Simurgh): ver `IDE-FENXEDIT2.md`.
