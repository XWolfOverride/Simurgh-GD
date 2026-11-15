#!/bin/sh
# Prueba del programa unico (build/zig-probe/simurgh/$exe.exe, de tools/zig-probe-simurgh.sh): ejecutar un .dcb, compilar en
# memoria y ejecutar un .prg, solo compilar (-b), errores, ayuda y argumentos del programa.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
exe=${SIMURGH_EXE:-simr}
d="$root/build/zig-probe/simurgh"
[ -x "$d/$exe.exe" ] || { echo "Falta $exe.exe: ejecuta antes tools/zig-probe-simurgh.sh" >&2; exit 2; }
cd "$d"
cp "$root/tests/simurgh/t_defaults.prg" "$root/tests/simurgh/t_defaults_inc.inc" "$root/tests/simurgh/err_defaults1.prg" "$root/tests/simurgh/t_args.prg" .
rm -f t_defaults.dcb t_args.dcb
fallo=0
ok()  { echo "[OK] $1"; }
mal() { echo "[X] $1"; fallo=1; }

# 1. .prg: compila en memoria y ejecuta; no deja .dcb y el compilador no imprime nada
out=$(timeout 30 ./$exe.exe t_defaults.prg 2>&1) && rc=0 || rc=$?
[ "$rc" = 0 ] && [ "$out" = "RESULTADO ok=23 bad=0" ] && [ ! -e t_defaults.dcb ] && ok ".prg: compila en memoria y ejecuta (salida solo del programa, sin .dcb)" || mal ".prg: rc=$rc salida=[$out]"

# 2. -b: solo compila, sin salida
out=$(./$exe.exe -b t_defaults.prg 2>&1) && rc=0 || rc=$?
[ "$rc" = 0 ] && [ -z "$out" ] && [ -s t_defaults.dcb ] && ok "-b: solo compila (sin salida, escribe el .dcb)" || mal "-b: rc=$rc salida=[$out]"

# 3. .dcb: lo ejecuta
out=$(timeout 30 ./$exe.exe t_defaults.dcb 2>&1) && rc=0 || rc=$?
[ "$rc" = 0 ] && [ "$out" = "RESULTADO ok=23 bad=0" ] && ok ".dcb: ejecuta" || mal ".dcb: rc=$rc salida=[$out]"

# 4. error de compilacion: se imprime y sale con 2 sin ejecutar
out=$(./$exe.exe err_defaults1.prg 2>&1) && rc=0 || rc=$?
[ "$rc" = 2 ] && echo "$out" | grep -q "Incorrect number of parameters" && ok "error de compilacion: codigo 2 y mensaje" || mal "error: rc=$rc salida=[$out]"

# 5. ayuda y opciones: sin argumentos, desconocida, valor pegado
./$exe.exe > /dev/null 2>&1 && mal "sin argumentos deberia dar 1" || { [ $? = 1 ] && ok "sin argumentos: ayuda y codigo 1" || mal "sin argumentos"; }
./$exe.exe -x t_defaults.prg > /dev/null 2>&1 && mal "opcion desconocida" || ok "opcion desconocida: error"
./$exe.exe -idir t_defaults.prg > /dev/null 2>&1 && mal "-idir pegado deberia fallar" || ok "-idir pegado: ya no funciona (-i dir)"
./$exe.exe -f t_defaults.prg > /dev/null 2>&1 && mal "-f sin fichero" || ok "-f sin fichero: error"

# 6. argumentos del programa (lo que va despues del fichero no se interpreta, ni siquiera con guion)
out=$(timeout 30 ./$exe.exe t_args.prg uno -dos 2>&1) || true
[ "$out" = "argc=2 a1=uno a2=-dos" ] && ok "argumentos del programa tras el fichero" || mal "argumentos: [$out]"
exit $fallo
