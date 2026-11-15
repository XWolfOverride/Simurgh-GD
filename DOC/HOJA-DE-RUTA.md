# Hoja de ruta

«Más adelante» significa **antes de la primera versión publicada**. Estado de cada fase al final.

| Fase | Contenido | Estado |
|---|---|---|
| 0 | Andamio, documentación y árbol de Fenix 0.92a intacto en `upstream/` | hecho |
| 1 | `src/` copiado de `upstream/` (**hecho**, con `tools/upstream-diff`); **pruebas de referencia**: conjunto de programas `.prg` con salida esperada (texto y sumas de comprobación de fotogramas) generada con Fenix 0.92a; compilación con CMake para Windows, Linux y macOS | en curso |
| 2 | **Compilador reentrante** (`libsimurgh`): sin globales ni `exit()`; formato de `.dcb` definido y con versión | pendiente |
| 3 | **Capa glue** (SDL 1.2 y SDL2) sin cambiar el comportamiento; el renderizado queda en el núcleo | pendiente |
| 4 | **16 bits siempre**: eliminar paletas y los 8 bits; conversión 8 → 16 al cargar; UTF-8 | pendiente |
| 5 | `map` y `fpg` integrados; PNG en el núcleo; compatibilidad total de FPG y MAP (8 y 16 bits) | pendiente |
| 6 | Rescatar **FenXEdit2** y que compile desde dentro | pendiente |
| 7 | Backend DOS (586, LFB, LFN opcional) y comprobación en el XO | pendiente |
| 7a | Backend **Linux ARM** (Raspberry Pi 1 a 5): binarios `armhf` de 32 bits (ARMv6 para llegar a la Pi 1), pruebas en la Pi más lenta | pendiente |
| 7b | Backend **Sony PSP** (pspdev; SDL 1.2 o glue propio); la toolchain se prueba primero en WSL2/Docker y, si funciona, en la VM Fedora | pendiente |
| 8 | Antes de la primera versión: optimizar opcodes, MMX/3DNow, transparencia y alfa (formato nuevo si hace falta), depurador, decisión final sobre color de 16 bits | pendiente |

## Pruebas
- Los programas de `upstream/fenix-0.92a/bin` (`test.prg`, `fbm.prg`...) sirven de arranque.
- Un fotograma de referencia se compara por suma de comprobación del búfer RGB565, sin depender de la pantalla.
- Cada optimización de opcodes se acepta o se descarta por medición, no por intuición.
- Las pruebas de rendimiento se hacen antes de tocar los opcodes.
