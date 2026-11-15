#!/bin/sh
# Compila el IDE (ide/FenXEdit2) con simurgh -b (de tools/zig-probe-simurgh.sh).

set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
out="$root/build/zig-probe/ide"
zig=${ZIG:-}
if [ -z "$zig" ]; then for c in /c/XWolf/Apps/Zig/zig-*/zig.exe; do [ -x "$c" ] && zig=$c; done; fi
[ -n "$zig" ] || zig=$(command -v zig)
exe=${SIMURGH_EXE:-simr}
sim="$root/build/zig-probe/simurgh/$exe.exe"
[ -x "$sim" ] || { echo "Falta $sim: ejecuta antes tools/zig-probe-simurgh.sh" >&2; exit 2; }
rm -rf "$out"; mkdir -p "$out"; cp -r "$root/ide/FenXEdit2/." "$out/"

cd "$out"
"$sim" -b -c FenXEdit2.prg > fxc.log 2>&1 || true
if [ -s FenXEdit2.dcb ]; then echo "[OK] FenXEdit2.dcb ($(wc -c < FenXEdit2.dcb) bytes)"; else tail -3 fxc.log; echo "[X] no compila"; exit 1; fi
