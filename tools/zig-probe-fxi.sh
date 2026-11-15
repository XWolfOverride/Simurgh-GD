#!/bin/sh
# Prueba de compilacion: construye fxi (interprete) de 32 bits para Windows con Zig y SDL 1.2, y deja todo listo para ejecutar.
#
#   tools/zig-probe-fxi.sh
#
# Resultado: build/zig-probe/w32/fxi.exe con sus DLL (SDL, SDL_mixer...). Para probar:
#   cd build/zig-probe/w32 && ./fxi.exe test.dcb      (test.dcb lo genera tools/zig-probe.sh)
#
# Por que 32 bits: la maquina virtual de Fenix guarda PUNTEROS en enteros de 32 bits (pila de la VM, i_main.c, instance.c);
# compilado para 64 bits el interprete se cae al cargar un programa. Ver DOC/PROBLEMAS.md (punto 5 y 31).
# Descarga SDL 1.2.15 (MinGW), SDL_mixer 1.2.12 (VC + runtime), libpng 1.6.43 y zlib 1.3.1 en build/zig-probe/w32-deps.
# Es una herramienta temporal hasta que exista el sistema de compilacion definitivo (CMake).
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
dep="$root/build/zig-probe/w32-deps"
out="$root/build/zig-probe/w32"
zig=${ZIG:-}
if [ -z "$zig" ]; then
    for c in /c/XWolf/Apps/Zig/zig-*/zig.exe; do [ -x "$c" ] && zig=$c; done
    [ -n "$zig" ] || zig=$(command -v zig || true)
fi
[ -n "$zig" ] || { echo "No se encuentra zig (variable ZIG)." >&2; exit 2; }
mkdir -p "$dep" "$out"
cd "$dep"
get() { [ -f "$2" ] || { echo ">> Descargando $2"; curl -sL -o "$2" "$1"; }; }
get https://www.libsdl.org/release/SDL-devel-1.2.15-mingw32.tar.gz sdl.tgz
get https://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-devel-1.2.12-VC.zip mixdev.zip
get https://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-1.2.12-win32.zip mixrt.zip
get https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.43.tar.gz png.tgz
get https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz zlib.tgz
[ -d SDL-1.2.15 ] || tar xzf sdl.tgz
[ -d SDL_mixer-1.2.12 ] || unzip -oq mixdev.zip
[ -d mixrt ] || unzip -oq mixrt.zip -d mixrt
[ -d libpng-1.6.43 ] || tar xzf png.tgz
[ -d zlib-1.3.1 ] || tar xzf zlib.tgz
cp libpng-1.6.43/scripts/pnglibconf.h.prebuilt libpng-1.6.43/pnglibconf.h

# SDLmain de MinGW pide __iob (no existe): se sustituye por un WinMain minimo.
cat > winmain.c <<'EOS'
#include <windows.h>
#include <stdlib.h>
extern int SDL_main(int argc, char *argv[]);
extern void SDL_SetModuleHandle(void *);
int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR cmd, int show)
{
    SDL_SetModuleHandle(GetModuleHandle(NULL));
    return SDL_main(__argc, __argv);
}
EOS

cd "$root/src"
fxi_src=$(cd fxi/src && ls *.c | grep -v -E '^(mmx_main|mmx_hspan)\.c$' | sed 's#^#fxi/src/#')
png_src=$(ls "$dep"/libpng-1.6.43/png*.c | grep -v pngtest)
zl="$dep/zlib-1.3.1"
z_src=""
for f in adler32 crc32 deflate inffast inflate inftrees trees zutil compress uncompr gzclose gzlib gzread gzwrite; do
    z_src="$z_src $zl/$f.c"
done
# Banderas: -fgnu89-inline (Fenix usa "_inline" con semantica antigua), -fcommon, y avisos que clang moderno trata como
# error pero el codigo heredado incumple (se arreglaran en el codigo mas adelante). MMX fuera: ensamblador solo de 32 bits.
echo ">> Compilando fxi (x86-windows-gnu)..."
"$zig" cc -target x86-windows-gnu -O2 -w -fgnu89-inline -fcommon \
    -Wno-incompatible-function-pointer-types -Wno-incompatible-pointer-types -Wno-int-conversion \
    -Wno-implicit-function-declaration \
    -DVERSION='"0.92a"' -DTARGET_Win32 -DWIN32 -D_WIN32 -DDEBUG -DPNG_ARM_NEON_OPT=0 -DPNG_INTEL_SSE_OPT=0 \
    -I"$dep/SDL-1.2.15/include/SDL" -I"$dep/SDL-1.2.15/include" -I"$dep/SDL_mixer-1.2.12/include" \
    -I"$dep/libpng-1.6.43" -I"$zl" -Iinclude -Ifxi/inc \
    $fxi_src common/dirs.c common/regex.c common/files.c common/xctype.c $png_src $z_src "$dep/winmain.c" \
    "$dep/SDL-1.2.15/lib/libSDL.dll.a" "$dep/SDL_mixer-1.2.12/lib/x86/SDL_mixer.lib" -lshlwapi -o "$out/fxi.exe"
cp "$dep/SDL-1.2.15/bin/SDL.dll" "$dep"/mixrt/*.dll "$out/"
cp bin/test.fpg bin/game.s3m "$out/"
echo ">> Hecho: $out/fxi.exe"
# Compilador (de tools/zig-probe.sh, si existe) y lanzador para probar con doble clic.
[ -f "$root/build/zig-probe/fxc.exe" ] && cp "$root/build/zig-probe/fxc.exe" "$out/"
cp bin/test.prg "$out/"
printf '@echo off
cd /d "%%~dp0"
if exist fxc.exe (fxc.exe test.prg) else (echo [fxc.exe no encontrado: ejecuta tools/zig-probe.sh])
fxi.exe test.dcb
' > "$out/probar.cmd"
perl -pi -e 's/?
/
/' "$out/probar.cmd" 2>/dev/null || sed -i 's/$//' "$out/probar.cmd"
echo "   Para probar: doble clic en $out/probar.cmd (compila test.prg y lo ejecuta; fxi es de 32 bits)."
