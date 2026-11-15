# Plataformas

| Plataforma | Arquitecturas | Backend | Compilador | Estado |
|---|---|---|---|---|
| Windows | x86 (32), x86-64, ARM64 | SDL2 | MSVC y clang (MSYS2); `zig cc` para reparto | previsto |
| Linux | i586/i686 (32), x86-64, ARM64 | SDL2; SDL 1.2 en sistemas antiguos | gcc / clang | previsto |
| macOS | x86-64 y arm64 | SDL2 | clang (Xcode CLT) | previsto |
| OLPC XO-1 | i586 con Geode LX, **sin SSE** (MMX sí) | SDL 1.2 | gcc 4.7 en una VM Fedora 18 | Fenix 0.92a ya compilado |
| DOS | **586 (Pentium) mínimo** | Glue DOS (VESA/VBE, SB, INT 33h) | DJGPP u Open Watcom | evaluar |
| Linux ARM (Raspberry Pi 1 a 5) | ARMv6 (Pi 1 y Zero), ARMv7 (Pi 2), ARMv8 (Pi 3 a 5) | SDL2 (SDL 1.2 si hace falta) | gcc, clang o `zig cc` (`arm-linux-gnueabihf`) | previsto; ver abajo |
| Sony PSP | MIPS Allegrex (little-endian) | SDL 1.2 (port de pspdev) o glue propio sobre `sce*` | `psp-gcc` (pspdev) | previsto; ver abajo |

## OLPC XO-1 (Linux, i586)
222 MB de RAM, Geode LX a 433 MHz, SDL 1.2 y libpng 1.5 en Fedora 18; no hay gcc ni SDL2 allí. Los binarios se
compilan en una VM del mismo sistema con `-O2 -march=geode -mtune=geode` (y se comprueba que no usan SSE).
Restricciones: sin instrucciones SSE, memoria justa, bibliotecas de Fedora 18.

## Linux ARM: Raspberry Pi 1 a 5 (previsto)

- **Arquitecturas (de memoria, sin verificar):** Pi 1 y Zero: ARM1176 (ARMv6, coma flotante VFP, 700 MHz a 1 GHz); Pi 2: Cortex-A7 (ARMv7); Pi 3, 4 y 5:
  Cortex-A53, A72 y A76 (ARMv8, capaces de 64 bits). Raspberry Pi OS se ofrece en 32 bits (`armhf`, compatible con todas) y en 64 bits (`aarch64`).
- **Consecuencia del intérprete de 32 bits** (`PROBLEMAS.md`, 31): hasta resolver el modelo de punteros, el binario es **`armhf` de 32 bits**
  (`-mfpu=vfp -mfloat-abi=hard`, `-march=armv6` para que corra también en la Pi 1 y la Zero). En un Raspberry Pi OS de 64 bits habría que
  añadir las bibliotecas `armhf` (multiarch) para ejecutarlo.
- **Compilación:** nativa en la propia Pi, o cruzada con `zig cc -target arm-linux-gnueabihf` (eligiendo la glibc de la imagen). Los ajustes de rendimiento (renderizado por software a 16 bits) se medirán en la Pi 1, la más lenta.
- **Entrada y pantalla:** SDL2 sobre X11, Wayland o la consola (KMS/DRM); sin aceleración propia: el renderizado de Fenix es por software.

## Sony PSP (previsto)

- **Hardware (de memoria, sin verificar):** CPU MIPS Allegrex a 333 MHz como máximo, 32 MB de RAM en los primeros modelos y 64 MB en
  los posteriores, pantalla de 480×272 con 16 bits de color nativos (encaja con RGB565). Ejecutables `EBOOT.PBP`.
- **Toolchain:** [pspdev](https://pspdev.github.io/downloads.html). La página de descargas ofrece tarballs para Ubuntu
  (Windows se usa con WSL2 y el tarball de Ubuntu: no hay instalador nativo), macOS, Fedora y Debian, y la imagen de Docker
  `pspdev/pspdev`. **No he comprobado** si el tarball de Fedora funciona en la Fedora 18 de la VM (compilado contra una
  glibc y un libstdc++ mucho más recientes, es probable que no); alternativas: WSL2 en Windows, Docker, o una Fedora más
  nueva en la VM.
- **Consecuencias para el diseño:** memoria de 32 MB como límite duro (intérprete y recursos), sin SDL2, sistema de
  ficheros y hilos propios, audio por la API de la consola. El renderizado por software de Fenix a 16 bits encaja bien.
  El compilador (`fxc`/`libsimurgh`) se ejecuta en el PC: la PSP solo ejecuta el intérprete y el `.dcb`.

## DOS (586 como mínimo, 16 bits de color)
- **Sistema mínimo: un Pentium (586)** (decisión). Un 486 a 640×480 con 16 bits de color mueve unos 600 KB por
  fotograma; a 30 fps son unos 18 MB/s hasta la tarjeta de vídeo. Una tarjeta ISA o VLB de la época no llega ahí
  con fluidez. Un 486 queda fuera del objetivo (en 320×240 podría ir, sin garantías). Son estimaciones, no medidas.
- **Vídeo:** VESA/VBE 2.0 con **memoria lineal (LFB)** para 16 bits. Sin LFB (VBE 1.x) hay que usar bancos, más lento.
- **Entrada y sonido:** ratón por INT 33h, teclado por interrupción, Sound Blaster (DMA). Allegro 4 es una alternativa
  ya hecha; no probada.
- **Memoria:** modo protegido de 32 bits con DPMI (CWSDPMI). El intérprete debe poder trabajar con poca RAM.
- **Coma flotante:** el intérprete usa `float`; requiere 486DX o superior (o 387).
- **Nombres de fichero:** **LFN (nombres largos) opcionales**: si el sistema los ofrece (DOS con controlador LFN, Windows
  9x), se usan; si no, 8.3. Ningún recurso debería depender de nombres largos para funcionar. Hay que decidir una
  convención de nombres que quepa en 8.3.
- Compilador: DJGPP (gcc para DOS) u Open Watcom; comprobar C11.

## Windows, Linux, macOS
- **Windows:** SDL2 por vcpkg o MSYS2. MSVC soporta C11 con `/std:c11`, sin matrices de longitud variable ni
  `<threads.h>`. Para 32 bits en Windows queda `mingw32`.
- **macOS:** clang de las Command Line Tools; SDL2 con Homebrew o compilado de fuentes. Si se reparte un `.app`:
  firma y notarización. Binario universal (x86-64 + arm64).
- **Linux:** SDL2 del sistema; el reparto de binarios de 32 bits para sistemas viejos se compila contra una glibc
  antigua (por ejemplo 2.16, la de Fedora 18).
