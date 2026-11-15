#!/bin/sh
# Pruebas del lenguaje (tests/simurgh): parametros por defecto en funciones de usuario (t_defaults.prg, debe dar bad=0) y los
# errores esperados (err_defaults*.prg: codigo 2 y ningun .dcb). Usa simurgh (tools/zig-probe-simurgh.sh).
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
exe=${SIMURGH_EXE:-simr}
w="$root/build/zig-probe/simurgh"
[ -x "$w/$exe.exe" ] || { echo "Falta $w/$exe.exe: ejecuta antes tools/zig-probe-simurgh.sh" >&2; exit 2; }
fallo=0
cd "$w"
cp "$root/tests/simurgh/t_defaults.prg" "$root/tests/simurgh/t_defaults_inc.inc" .
timeout 30 "./$exe.exe" t_defaults.prg > t_defaults.out 2>&1 || true
if grep -q "bad=0" t_defaults.out; then echo "[OK] t_defaults: $(grep RESULTADO t_defaults.out)"; else cat t_defaults.out; echo "[X] t_defaults"; fallo=1; fi
for e in err_defaults1 err_defaults2 err_defaults3; do
    cp "$root/tests/simurgh/$e.prg" .
    rm -f "$e.dcb"
    rc=0; "./$exe.exe" -b "$e.prg" > "$e.out" 2>&1 || rc=$?
    if [ "$rc" = 2 ] && [ ! -s "$e.dcb" ]; then echo "[OK] $e: error esperado ($(grep -i 'error' "$e.out" | head -1 | cut -c1-90))"; else cat "$e.out"; echo "[X] $e: rc=$rc"; fallo=1; fi
done
exit $fallo
