#!/bin/sh
# Pruebas de imagenes (tests/simurgh): t_image.prg (LOAD_IMAGE por cabecera, alias LOAD_MAP/LOAD_PNG, SAVE_MAP, SAVE_FPG) y
# t_truecolor.prg (MAP y FPG de 24 y 32 bits). Necesita haber ejecutado tools/zig-probe.sh y tools/zig-probe-fxi.sh
# (usa build/zig-probe/w32). Abre una ventana unos segundos por prueba.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
w="$root/build/zig-probe/w32"
[ -x "$w/fxi.exe" ] || { echo "Falta fxi.exe: ejecuta antes tools/zig-probe-fxi.sh" >&2; exit 2; }
cp "$root/build/zig-probe/fxc.exe" "$w/fxc.exe"
fallo=0
cd "$w"
perl "$root/tests/simurgh/make_truecolor.pl" .
for t in t_image t_truecolor; do
    cp "$root/tests/simurgh/$t.prg" .
    rm -f "$t.dcb"
    ./fxc.exe "$t.prg" > /dev/null 2>&1 || true
    [ -s "$t.dcb" ] || { echo "[X] $t: no compila"; fallo=1; continue; }
    timeout 30 ./fxi.exe "$t.dcb" > "$t.out" 2>&1 || true
    if grep -q "bad=0" "$t.out"; then echo "[OK] $t: $(grep RESULTADO "$t.out")"; else cat "$t.out"; echo "[X] $t"; fallo=1; fi
done
exit $fallo
