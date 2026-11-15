#!/bin/sh
# Compila el IDE (ide/FenXEdit2) con el fxc de build/zig-probe (Import "image" ya es del nucleo: se acepta y no hace nada).

set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
out="$root/build/zig-probe/ide"
zig=${ZIG:-}
if [ -z "$zig" ]; then for c in /c/XWolf/Apps/Zig/zig-*/zig.exe; do [ -x "$c" ] && zig=$c; done; fi
[ -n "$zig" ] || zig=$(command -v zig)
[ -x "$root/build/zig-probe/fxc.exe" ] || { echo "Falta fxc.exe: ejecuta antes tools/zig-probe.sh" >&2; exit 2; }
rm -rf "$out"; mkdir -p "$out"; cp -r "$root/ide/FenXEdit2/." "$out/"

cd "$out"
"$root/build/zig-probe/fxc.exe" -c FenXEdit2.prg > fxc.log 2>&1 || true	# fxc devuelve 1 aunque compile bien
if [ -s FenXEdit2.dcb ]; then echo "[OK] FenXEdit2.dcb ($(wc -c < FenXEdit2.dcb) bytes)"; else tail -3 fxc.log; echo "[X] no compila"; exit 1; fi
