#!/bin/sh
# Prueba de audio (tests/simurgh/t_audio.prg): cargar, reproducir y descargar WAV y OGG, incluidos un WAV sin datos, un fichero que
# no es audio, descargas repetidas y numeros que no existen. El OGG sale del IDE (ide/FenXEdit2/NOOKIE.ogg). Abre una ventana y
# suena unos segundos. Usa simurgh (tools/zig-probe-simurgh.sh). Si el programa se cae, sale con error (no imprime RESULTADO).
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
exe=${SIMURGH_EXE:-simr}
w="$root/build/zig-probe/simurgh"
[ -x "$w/$exe.exe" ] || { echo "Falta $w/$exe.exe: ejecuta antes tools/zig-probe-simurgh.sh" >&2; exit 2; }
cd "$w"
perl "$root/tests/simurgh/make_audio.pl" .
cp "$root/ide/FenXEdit2/NOOKIE.ogg" "$root/tests/simurgh/t_audio.prg" .
rc=0; timeout 120 "./$exe.exe" t_audio.prg > t_audio.out 2>&1 || rc=$?
if grep -q "bad=0" t_audio.out; then echo "[OK] t_audio: $(grep RESULTADO t_audio.out)"; else cat t_audio.out; echo "[X] t_audio (codigo de salida $rc)"; exit 1; fi
