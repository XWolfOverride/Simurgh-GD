#!/bin/sh
# Pruebas de imagenes (tests/simurgh): t_image.prg (LOAD_IMAGE por cabecera, alias LOAD_MAP/LOAD_PNG, SAVE_MAP, SAVE_FPG),
# t_truecolor.prg (MAP y FPG de 24 y 32 bits) y t_formats.prg (SAVE_IMAGE y BMP/TGA). Cada programa se compila y ejecuta con
# simurgh (build/zig-probe/simurgh, de tools/zig-probe-simurgh.sh). Abre una ventana unos segundos por prueba.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
exe=${SIMURGH_EXE:-simr}
w="$root/build/zig-probe/simurgh"
[ -x "$w/$exe.exe" ] || { echo "Falta $w/$exe.exe: ejecuta antes tools/zig-probe-simurgh.sh" >&2; exit 2; }
fallo=0
cd "$w"
perl "$root/tests/simurgh/make_truecolor.pl" .
perl "$root/tests/simurgh/make_formats.pl" .
for t in t_image t_truecolor t_formats; do
    cp "$root/tests/simurgh/$t.prg" .
    timeout 60 "./$exe.exe" "$t.prg" > "$t.out" 2>&1 || true
    if grep -q "bad=0" "$t.out"; then echo "[OK] $t: $(grep RESULTADO "$t.out")"; else cat "$t.out"; echo "[X] $t"; fallo=1; fi
done
# El alfa parcial cargado de t_al32.map debe salir igual en los ficheros grabados
perl -e '
    sub rd { open(F,"<:raw",$_[0]) or die "no existe $_[0]"; local $/; my $t=<F>; close F; $t }
    my $m = rd("t_al_out.map"); my $tga = rd("t_al_out.tga");
    my @am = map { ord(substr($m, 8+2+2+4+32+2 + 4*$_ + 3, 1)) } 0..3;
    my @at = map { ord(substr($tga, 18 + 4*$_ + 3, 1)) } 0..3;
    my $ok = ("@am" eq "255 128 100 0" && "@at" eq "255 128 100 0" && substr($m,0,3) eq "m32");
    print($ok ? "[OK] alfa conservado al grabar (m32 y tga): @am\n" : "[X] alfa: m32=@am tga=@at\n"); exit($ok ? 0 : 1);
' || fallo=1
exit $fallo
