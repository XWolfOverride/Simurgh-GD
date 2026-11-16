#!/bin/sh
# Pruebas del lenguaje (tests/simurgh): TYPE y STRUCT (t_type, t_type2, t_struct), GET_ID (t_getid), unload_map(0,0) como error (err_unload0), parametros por defecto en funciones de usuario (t_defaults.prg, debe dar bad=0) y los
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
for e in err_defaults1 err_defaults2 err_defaults3 err_ancient1 err_ancient2 err_ancient3 err_height; do
    cp "$root/tests/simurgh/$e.prg" .
    rm -f "$e.dcb"
    rc=0; "./$exe.exe" -b "$e.prg" > "$e.out" 2>&1 || rc=$?
    if [ "$rc" = 2 ] && [ ! -s "$e.dcb" ]; then echo "[OK] $e: error esperado ($(grep -i 'error' "$e.out" | head -1 | cut -c1-90))"; else cat "$e.out"; echo "[X] $e: rc=$rc"; fallo=1; fi
done
for t in t_type t_type2 t_struct t_getid t_ancient t_ancient_block t_height t_m7col; do
    cp "$root/tests/simurgh/$t.prg" .
    timeout 30 "./$exe.exe" "$t.prg" > "$t.out" 2>&1 || true
    if grep -q "bad=0" "$t.out"; then echo "[OK] $t: $(grep RESULTADO "$t.out")"; else cat "$t.out"; echo "[X] $t"; fallo=1; fi
done
# unload_map(0,0) es un error de ejecucion (codigo de salida 1 y mensaje)
cp "$root/tests/simurgh/err_unload0.prg" .
rc=0; "./$exe.exe" err_unload0.prg > err_unload0.out 2>&1 || rc=$?
if [ "$rc" = 1 ] && grep -q "codigo de mapa no valido" err_unload0.out; then echo "[OK] err_unload0: error de ejecucion esperado"; else cat err_unload0.out; echo "[X] err_unload0: rc=$rc"; fallo=1; fi
exit $fallo
