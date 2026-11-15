#!/bin/sh
# Prueba de compilacion: construye simr.exe, el programa unico (interprete + compilador) de 32 bits para Windows con Zig.
#
#   tools/zig-probe-simurgh.sh
#
# Descarga SDL 1.2.15 (MinGW), SDL_mixer 1.2.12 (VC + runtime), libpng 1.6.43 y zlib 1.3.1 en build/zig-probe/w32-deps.
# Resultado: build/zig-probe/simurgh/<nombre>.exe con sus DLL. El nombre del ejecutable (por defecto simr) se cambia con
# SIMURGH_EXE=nombre. Por que 32 bits: la maquina virtual guarda punteros en enteros de 32 bits (DOC/PROBLEMAS.md, 5 y 31).
# Es una herramienta temporal hasta que exista el sistema de compilacion definitivo (CMake). Ver DOC/COMPILACION.md.
#
# El compilador (fxc) y el interprete (fxi) definen algunos simbolos globales con el mismo nombre (string_get, debug, procs...):
# los del compilador se renombran con fxc_rename.h al compilarlo, para que convivan en el mismo ejecutable.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
dep="$root/build/zig-probe/w32-deps"
out="$root/build/zig-probe/simurgh"
zig=${ZIG:-}
if [ -z "$zig" ]; then
    for c in /c/XWolf/Apps/Zig/zig-*/zig.exe; do [ -x "$c" ] && zig=$c; done
    [ -n "$zig" ] || zig=$(command -v zig || true)
fi
[ -n "$zig" ] || { echo "No se encuentra zig (variable ZIG)." >&2; exit 2; }
exe=${SIMURGH_EXE:-simr}
mkdir -p "$dep"
(
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
)
rm -rf "$out"; mkdir -p "$out/o_fxi" "$out/o_fxc" "$out/o_lib"
zl="$dep/zlib-1.3.1"

# Renombrado de los simbolos del compilador que chocan con los del interprete
cat > "$out/fxc_rename.h" <<'EOS'
/* Simbolos globales que fxc y fxi definen con el mismo nombre: los de fxc llevan el prefijo fxc_ */
#define debug              fxc_debug
#define globaldata         fxc_globaldata
#define localdata          fxc_localdata
#define mainproc           fxc_mainproc
#define mnemonic_dump      fxc_mnemonic_dump
#define mnemonics          fxc_mnemonics
#define procdef_count      fxc_procdef_count
#define procdef_get        fxc_procdef_get
#define procs              fxc_procs
#define string_dump        fxc_string_dump
#define string_get         fxc_string_get
#define string_init        fxc_string_init
#define string_load        fxc_string_load
#define string_new         fxc_string_new
#define sysproc_add        fxc_sysproc_add
#define sysproc_get        fxc_sysproc_get
#define sysprocs           fxc_sysprocs
EOS
cp "$dep/winmain.c" "$out/winmain.c"

cd "$root/src"
fxi_src=$(cd fxi/src && ls *.c | grep -v -E '^(mmx_main|mmx_hspan)\.c$')
fxc_src=$(cd fxc/src && ls *.c | grep -v -E '^(dcbw_old|cli)\.c$')
png_src=$(ls "$dep"/libpng-1.6.43/png*.c | grep -v pngtest)
z_src=""
for f in adler32 crc32 deflate inffast inflate inftrees trees zutil compress uncompr gzclose gzlib gzread gzwrite; do z_src="$z_src $zl/$f.c"; done

common="-target x86-windows-gnu -O2 -w -DVERSION=\"0.92a\" -DTARGET_Win32 -DWIN32 -D_WIN32"
fxi_flags="$common -fgnu89-inline -fcommon -Wno-incompatible-function-pointer-types -Wno-incompatible-pointer-types -Wno-int-conversion -Wno-implicit-function-declaration -DSIMURGH -DDEBUG -DPNG_ARM_NEON_OPT=0 -DPNG_INTEL_SSE_OPT=0 -I$dep/SDL-1.2.15/include/SDL -I$dep/SDL-1.2.15/include -I$dep/SDL_mixer-1.2.12/include -I$dep/libpng-1.6.43 -I$zl -I$root/src/include -I$root/src/fxi/inc"
fxc_flags="$common -fno-common -include $out/fxc_rename.h -I$zl -I$root/src/include -I$root/src/fxc/inc"
lib_flags="$common -fgnu89-inline -fcommon -Wno-implicit-function-declaration -DPNG_ARM_NEON_OPT=0 -DPNG_INTEL_SSE_OPT=0 -I$dep/SDL-1.2.15/include/SDL -I$dep/SDL-1.2.15/include -I$dep/SDL_mixer-1.2.12/include -I$dep/libpng-1.6.43 -I$zl -I$root/src/include -I$root/src/fxi/inc"

# zig cc no genera varios .o en una sola orden para COFF: un .o por fichero, en paralelo
cc1() { # destino flags fichero
    "$zig" cc -c $2 "$3" -o "$1/$(basename "${3%.c}").o"
}
export zig; export -f cc1 2>/dev/null || true
jobs_run() { # destino flags ficheros...
    d=$1; fl=$2; shift 2
    for s in "$@"; do
        echo "$d|$fl|$s"
    done
}
build_group() {
    d=$1; fl=$2; shift 2
    for s in "$@"; do
        ( "$zig" cc -c $fl "$s" -o "$d/$(basename "${s%.c}").o" ) &
        # como mucho 8 a la vez
        while [ "$(jobs -r | wc -l)" -ge 8 ]; do sleep 0.2; done
    done
    wait
    # los que hayan fallado por la concurrencia se repiten de uno en uno
    for s in "$@"; do
        [ -f "$d/$(basename "${s%.c}").o" ] || "$zig" cc -c $fl "$s" -o "$d/$(basename "${s%.c}").o"
    done
}
echo ">> Compilando el interprete..."
build_group "$out/o_fxi" "$fxi_flags" $(for s in $fxi_src; do echo "$root/src/fxi/src/$s"; done)
echo ">> Compilando el compilador..."
build_group "$out/o_fxc" "$fxc_flags" $(for s in $fxc_src; do echo "$root/src/fxc/src/$s"; done)
echo ">> Compilando bibliotecas comunes..."
build_group "$out/o_lib" "$lib_flags" "$root/src/common/dirs.c" "$root/src/common/regex.c" "$root/src/common/files.c" "$root/src/common/xctype.c" $png_src $z_src "$out/winmain.c"
echo ">> Enlazando $exe.exe..."
"$zig" cc -target x86-windows-gnu "$out"/o_fxi/*.o "$out"/o_fxc/*.o "$out"/o_lib/*.o \
    "$dep/SDL-1.2.15/lib/libSDL.dll.a" "$dep/SDL_mixer-1.2.12/lib/x86/SDL_mixer.lib" -lshlwapi -o "$out/$exe.exe"
cp "$dep/SDL-1.2.15/bin/SDL.dll" "$dep"/mixrt/*.dll "$out/"
echo ">> Hecho: $out/$exe.exe"
