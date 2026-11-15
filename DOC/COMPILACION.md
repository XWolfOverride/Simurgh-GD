# Compilación

Todavía no hay código propio: esto es lo verificado con Fenix 0.92a (`upstream/`) y lo previsto.

## Compiladores

| Entorno | Para qué |
|---|---|
| **MSYS2** (`clang64`: `pacman -S mingw-w64-clang-x86_64-toolchain`, SDL2, CMake, Ninja) | Desarrollo diario en Windows; bibliotecas ya compiladas; depurador |
| **MSVC** (carga de trabajo «Desarrollo para el escritorio con C++») | Segundo compilador para evitar extensiones de GNU; depuración |
| **`zig cc`** (solo como compilador, no `zig build`) | Reparto: compilación cruzada a Linux i586 (glibc 2.16 para el XO), Windows 32 bits y ARM64 |
| **VM Fedora 18 i686** | Compilar para el XO con las bibliotecas reales de ese sistema |
| **DJGPP / Open Watcom** | Futuro backend DOS |

Reglas: C11 común a gcc, clang y MSVC; `computed goto` y otras extensiones de GNU solo detrás de macros con
alternativa estándar; sin matrices de longitud variable.

## Zig o MSYS2 para el día a día (tamaños medidos 2026-11-15)

- **Zig 0.17.1** para Windows x86-64: descarga de **96 MB** (zip). Un solo directorio, sin instalador, trae clang, las
  cabeceras de MinGW y el cruce a Linux (incluida glibc 2.16 para el XO), ARM64 y 32 bits. No trae SDL2, CMake ni depurador.
  Espacio descomprimido: no medido.
- **MSYS2 `clang64`** (lo que falta): `clang` 38 MB + `llvm` 69 MB + `crt` 3 MB + `headers` 6 MB + `lld` 3 MB + `lldb` 6 MB
  + `libc++`, `compiler-rt`… ≈ **130–150 MB de descarga y ≈ 1 GB instalado**; con CMake (8 MB), Ninja y SDL2 (1,3 MB) sube un
  poco más.
- Combinación ligera: Zig como compilador y, de MSYS2, solo `cmake`, `ninja` y `SDL2` (≈ 10 MB de descarga, ≈ 60 MB
  instalados). Depurar con MSVC o `lldb`.

## Compilar `fxc` con Zig en Windows (verificado)

```
zig cc -target x86_64-windows-gnu -O2 -w -fcommon -DVERSION=\"0.92a\" -DTARGET_Win32 -DWIN32 -D_WIN32 \r
  -I<cabeceras SDL falsas> -I<zlib> -Iinclude -Ifxc/inc <fxc/src/*.c sin dcbw_old.c> common/files.c common/xctype.c <fuentes de zlib> -lshlwapi -o fxc.exe
```

Todo esto está automatizado en `tools/zig-probe.sh [triple]` (descarga zlib, genera las cabeceras SDL falsas, compila `fxc` en
`build/zig-probe/` y comprueba que el `.dcb` es reproducible). Con `x86-linux-gnu.2.16` solo compila (el destino del XO).

La primera ejecución de Zig tarda varios minutos (construye su caché); después, unos 30 s. Resultado: `fxc.exe` de 514 KB que
compila `test.prg`.

## Fenix 0.92a para el OLPC XO-1 (verificado)

Compilado en Fedora 18 i686 con `SDL-devel`, `SDL_mixer-devel`, `libpng-devel`, `zlib-devel`:

```
./configure --prefix=/mnt/sd/fenix --enable-map=no
make CFLAGS="-O2 -march=geode -mtune=geode -finline-functions -fno-strict-aliasing -w"
```

Cambios necesarios (`fenix-xo-0.92a.patch`):
- Añadir `-ldl -lm` al enlazado (los `Makefile` generados no los ponen).
- Quitar `--as-needed` de `fxi/src/Makefile`.
- Adaptar `img_png.c` y `fpg/fpg.c` a libpng 1.5: `png_jmpbuf(png_ptr)` en lugar de `png_ptr->jmpbuf`, y no tocar el
  campo opaco `info_ptr->palette`.

Resultado: `fxc` (194 KB) y `fxi` (457 KB), sin instrucciones SSE. `fpg` y `map` no compilan sin trabajo extra y
se sustituirán por funciones integradas. Probado solo en la VM (`fxc` compila el ejemplo y `fxi` lo arranca con los
controladores `dummy` de SDL); falta probarlo en el propio XO.

## Pruebas (previstas)
Ver [HOJA-DE-RUTA.md](HOJA-DE-RUTA.md): programas de referencia con salida esperada generada con Fenix 0.92a.
