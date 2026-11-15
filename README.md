# Simurgh

Lenguaje de programación de juegos 2D basado en procesos, heredero directo de **Fenix 0.92a** (y, por él, del
lenguaje de DIV Games Studio). Está pensado para aprender a programar y para hacer juegos caseros: sintaxis
sencilla, procesos que se ejecutan en paralelo cada fotograma, gráficos, sonido y entrada incluidos.

> **Estado: en diseño.** El repositorio contiene Fenix 0.92a original y corregido (`upstream/`) y la documentación
> de las decisiones (`DOC/`). Todavía no hay código propio. Ver [`DOC/HOJA-DE-RUTA.md`](DOC/HOJA-DE-RUTA.md).

## Qué cambia respecto a Fenix

- Multiplataforma actual: Windows, Linux, macOS (Intel y Apple Silicon), y también el OLPC XO-1 y DOS.
- Binarios de 32 y de 64 bits, en C11.
- Color de 16 bits siempre; sin paletas de 8 bits.
- Compilador reentrante, utilizable desde un programa Simurgh, y herramientas de mapas (`map`) y de colecciones de
  gráficos (`fpg`) integradas en el lenguaje.
- Texto en UTF-8.
- Una capa propia hacia el sistema en lugar de depender de una biblioteca concreta.

Los ficheros `.fpg` y `.map` de Fenix (8 y 16 bits) se leen y escriben sin pérdida.

## Documentación

Índice en [`DOC/README.md`](DOC/README.md): decisiones, genealogía, arquitectura, color y formatos, plataformas,
compilación, problemas conocidos y hoja de ruta.

## Licencia

GNU General Public License versión 2 (ver [`COPYING`](COPYING)); el código heredado es «versión 2 o posterior».
Origen y autores en [`NOTICE.md`](NOTICE.md). Autor del fork: XWolfOverride.
