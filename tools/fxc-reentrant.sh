#!/bin/sh
# Prueba de reentrancia de fxc como biblioteca (tests/fxc/reentrant.c): serie con errores y varios hilos a la vez.
#   tools/fxc-reentrant.sh [hilos] [vueltas]
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
out="$root/build/zig-probe"
zig=${ZIG:-}
if [ -z "$zig" ]; then for c in /c/XWolf/Apps/Zig/zig-*/zig.exe; do [ -x "$c" ] && zig=$c; done; fi
[ -n "$zig" ] || zig=$(command -v zig)
zl=$out/zlib-1.3.1
[ -f "$zl/zlib.h" ] || { echo "Falta zlib: ejecuta antes tools/zig-probe.sh" >&2; exit 2; }
zsrc=""; for f in adler32 crc32 deflate inffast inflate inftrees trees zutil compress uncompr gzclose gzlib gzread gzwrite; do zsrc="$zsrc $zl/$f.c"; done
cd "$root/src"
srcs=$(ls fxc/src/*.c | grep -v 'dcbw_old\|cli.c')
mkdir -p "$out/reent" && cd "$out/reent"
"$zig" cc -O1 -w -fno-common -DTARGET_Win32 -DWIN32 -D_WIN32 -DFILES_STATE=__thread -DVERSION='"0.92a"' -I"$zl" -I"$root/src/include" -I"$root/src/fxc/inc" \
    $(for s in $srcs; do echo "$root/src/$s"; done) "$root/src/common/files.c" "$root/src/common/xctype.c" $zsrc \
    "$root/tests/fxc/reentrant.c" -lshlwapi -o reentrant.exe
cp "$root/tests/fxc/t_menu.prg" ok.prg; cp "$root/tests/fxc/t_err1.prg" err.prg
./reentrant.exe ok.prg err.prg "${1:-4}" "${2:-10}" | grep -E '^\[(OK|X)\]'
