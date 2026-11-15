# Color y formatos

## Decisión actual: 16 bits siempre

- El búfer de pantalla y los gráficos del intérprete son **RGB565** (16 bits por píxel). Fenix ya tiene este modo
  (`g_conversion.c`: tablas RGB565 y mezcla alfa de 16 bits); Simurgh lo adopta como único.
- Se elimina el modo de 8 bits y **toda la manipulación de paletas** (`g_pal.c`, ≈790 líneas): fundidos por paleta,
  ciclos de color, edición de paleta. Los fundidos se hacen con las operaciones de 16 bits que Fenix ya tiene.
- Un recurso de 8 bits (mapa o FPG) se **convierte a 16 al cargarlo**, usando su paleta.
- **Transparencia:** como en Fenix 16 bits, el **negro puro (0) es transparente**. Se mantiene por ahora.
- Los programas que usan paleta de forma intensiva no compilan. Se considera una pérdida aceptable: el uso de la
  paleta es herencia de DIV, que era de 8 bits; el uso habitual (fundidos) ya está resuelto en 16 bits.
- Un solo modo de dar color a los programas: RGB (`rgb()`), documentado.

## Presentación en el sistema

El núcleo siempre trabaja en 16 bits. La capa glue entrega el fotograma al sistema:
- Sistemas modernos (Windows, macOS, Linux con SDL2): la pantalla del escritorio es de 32 bits, así que el
  fotograma se **convierte** de 16 a 32 bits al presentarlo (una textura). Los sistemas operativos actuales ya no ofrecen
  de verdad un modo de 16 bits, así que esto es lo normal y no supone ningún problema de compatibilidad. El coste es una
  pasada de conversión por fotograma, que hay que medir en equipos modestos.
- XO y sistemas antiguos con SDL 1.2: la pantalla puede ser de 16 bits y se copia directamente.
- DOS: VESA a 16 bits, directo.

Si algún día un sistema impide el camino de 16 bits, se evalúa al final (no se espera).

## Compatibilidad de formatos (regla)

- **FPG y MAP** de Fenix, de **8 y 16 bits**, se leen y se escriben con **compatibilidad total**.
- Cualquier cambio que no sea compatible (alfa por píxel, otra transparencia, nuevos metadatos) **no modifica** el
  formato existente: se define un **formato nuevo** (nueva firma/versión) y se conserva el antiguo.
- El `.dcb` (programa compilado) no es un formato de intercambio con Fenix: se define de nuevo, con versión.
- PNG: importación integrada (en Fenix ya existe `load_png`). GIF y JPEG, fuera del núcleo.

## Temas abiertos (resolver antes de la primera versión)

1. **Negro puro y transparencia.** Poder usar el negro puro como color y definir la transparencia de otro modo (color
   clave configurable, o máscara, o alfa por píxel). Hay que decidir el mecanismo y qué formato nuevo de FPG/MAP lo
   guarda. Mientras tanto, comportamiento de Fenix.
2. **Alfa por píxel.** Fenix tiene tablas de mezcla de 16 bits (`g_blendop.c`) pero no alfa por píxel en el recurso.
3. **Color de más de 16 bits.** Descartado por ahora; si se quisiera, sería otro formato.
4. **Conversión 8 → 16:** qué índice se considera transparente (el 0 de la paleta) y cómo se trata si la paleta lo
   convierte en un color no negro.
5. **Rendimiento del paso 16 → 32** en equipos modestos, y los escaladores heredados (`hq2x`, `scale2x`, `scanline`).
