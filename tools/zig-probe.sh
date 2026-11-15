#!/bin/sh
# Prueba de compilacion: construye fxc (compilador) desde src/ con Zig, sin SDL, y comprueba que el .dcb es reproducible.
#
#   tools/zig-probe.sh [destino-zig]
#
#   destino-zig  triple de Zig (por defecto x86_64-windows-gnu). Ejemplos: x86-linux-gnu.2.16 (XO, solo compila; no se
#                ejecuta en Windows), x86_64-linux-gnu, aarch64-macos.
#
# Variables:
#   ZIG       ruta de zig (por defecto el de C:/XWolf/Apps/Zig o el del PATH)
#   FXC_SRC   arbol de fuentes a compilar, relativo a la raiz (por defecto upstream/fenix-0.92a-fixed: el fxc de referencia; el
#             compilador de Simurgh ya no es un ejecutable aparte, va en el programa unico: tools/zig-probe-simurgh.sh)
#   FXC_OUT   nombre del ejecutable generado (por defecto fxc-ref)
#   ZLIB_DIR  fuentes de zlib (por defecto build/zig-probe/zlib; se descarga la 1.3.1 si no existe)
#
# Deja todo en build/zig-probe/ (ignorado por git). Es una herramienta temporal hasta que exista el sistema de
# compilacion definitivo (CMake): ver DOC/COMPILACION.md.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
out="$root/build/zig-probe"
target=${1:-x86_64-windows-gnu}
zig=${ZIG:-}
if [ -z "$zig" ]; then
    for c in /c/XWolf/Apps/Zig/zig-*/zig.exe; do [ -x "$c" ] && zig=$c; done
    [ -n "$zig" ] || zig=$(command -v zig || true)
fi
[ -n "$zig" ] || { echo "No se encuentra zig (variable ZIG)." >&2; exit 2; }
mkdir -p "$out/sdlstub"

# Cabeceras SDL falsas: fxc solo usa de SDL los tipos, SDL_BYTEORDER y SDL_Init.
cat > "$out/sdlstub/SDL.h" <<'EOF'
/* Cabecera falsa solo para compilar fxc sin SDL. No forma parte de Simurgh. */
#ifndef SDLSTUB_H
#define SDLSTUB_H
#include <stdint.h>
typedef uint8_t Uint8; typedef int8_t Sint8; typedef uint16_t Uint16; typedef int16_t Sint16;
typedef uint32_t Uint32; typedef int32_t Sint32; typedef uint64_t Uint64; typedef int64_t Sint64;
#define SDL_LIL_ENDIAN 1234
#define SDL_BIG_ENDIAN 4321
#define SDL_BYTEORDER SDL_LIL_ENDIAN
#define SDL_INIT_VIDEO 0x20
static inline int SDL_Init(Uint32 f){(void)f;return 0;}
static inline const char *SDL_GetError(void){return "";}
static inline void SDL_Quit(void){}
#endif
EOF
cp "$out/sdlstub/SDL.h" "$out/sdlstub/SDL_types.h"

# zlib
zl=${ZLIB_DIR:-$out/zlib-1.3.1}
if [ ! -f "$zl/zlib.h" ]; then
    echo ">> Descargando zlib 1.3.1..."
    curl -sL -o "$out/zlib.tgz" https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
    tar xzf "$out/zlib.tgz" -C "$out"
fi
zsrc=""
for f in adler32 crc32 deflate inffast inflate inftrees trees zutil compress uncompr gzclose gzlib gzread gzwrite; do
    zsrc="$zsrc $zl/$f.c"
done

# Notas: -fcommon (Fenix repite definiciones de globales), dcbw_old.c es codigo muerto, VERSION lo daba configure.
srcdir=${FXC_SRC:-upstream/fenix-0.92a-fixed}		# arbol a compilar (relativo a la raiz del proyecto)
cd "$root/$srcdir"
fxc_src=$(ls fxc/src/*.c | grep -v dcbw_old)
extra=""
exe=${FXC_OUT:-fxc-ref}
case $target in
    *windows*) extra="-DTARGET_Win32 -DWIN32 -D_WIN32 -lshlwapi"; exe=${FXC_OUT:-fxc-ref}.exe ;;
    *linux*)   extra="-DTARGET_Linux -D_GNU_SOURCE -DHAVE_UNISTD_H" ;;
    *macos*)   extra="-DTARGET_MAC" ;;
esac
echo ">> Compilando fxc para $target..."
"$zig" cc -target "$target" -O2 -w ${FXC_CFLAGS:--fcommon} -DFILES_STATE=__thread -DVERSION='"0.92a"' $extra \
    -I"$out/sdlstub" -I"$zl" -Iinclude -Ifxc/inc \
    $fxc_src common/files.c common/xctype.c $zsrc -o "$out/$exe"
echo ">> Hecho: $out/$exe"

# Reproducibilidad del .dcb (solo si el binario se puede ejecutar aqui)
case $target in
    x86_64-windows*|*-native*|x86_64-linux*|aarch64-macos*) ;;
    *) echo ">> (destino no ejecutable aqui: sin prueba de reproducibilidad)"; exit 0 ;;
esac
cp "$root/$srcdir/bin/test.prg" "$out/test.prg"
cd "$out"
# Se mira el fichero generado y no el codigo de salida (el de Fenix 0.92a devolvia 1 al compilar bien; Simurgh devuelve 0).
rm -f test.dcb
./"$exe" test.prg > /dev/null 2>&1 || true
[ -s test.dcb ] || { echo "[X] fxc no genera test.dcb" >&2; exit 1; }
cp test.dcb a.dcb
./"$exe" test.prg > /dev/null 2>&1 || true
if cmp -s a.dcb test.dcb; then
    echo "[OK] .dcb reproducible ($(wc -c < test.dcb) bytes)"
else
    echo "[X]  .dcb NO reproducible: $(cmp -l a.dcb test.dcb | wc -l) bytes distintos" >&2
    exit 1
fi
