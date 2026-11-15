#!/bin/sh
# Prueba de referencia del compilador: compila los programas de tests/fxc con el fxc de src/ y con el de referencia
# (upstream/fenix-0.92a-fixed) y comprueba que dan lo mismo: salida del compilador, codigo de salida y .dcb.
#
#   tools/fxc-compare.sh [--no-build]
#
#   --no-build   no recompila: usa build/zig-probe/fxc-ref.exe y fxc-new.exe ya existentes
#
# Compila con Zig (tools/zig-probe.sh). Pensado para Windows x64 (ejecuta los .exe aqui). Sale con 0 si todo coincide.
# Cuando un cambio en src/ deba alterar la salida a proposito (otro formato de .dcb, mensajes), hay que decidirlo aqui,
# no ocultarlo: actualizar la referencia o anotar la diferencia esperada.
set -u
root=$(cd "$(dirname "$0")/.." && pwd)
out="$root/build/zig-probe"
if [ "${1:-}" != "--no-build" ]; then
    FXC_CFLAGS=-fcommon FXC_SRC=upstream/fenix-0.92a-fixed FXC_OUT=fxc-ref sh "$root/tools/zig-probe.sh" > "$out.ref.log" 2>&1 || { cat "$out.ref.log"; echo "[X] no se pudo compilar la referencia"; exit 2; }
    FXC_SRC=src FXC_OUT=fxc-new sh "$root/tools/zig-probe.sh" > "$out.new.log" 2>&1 || { cat "$out.new.log"; echo "[X] no se pudo compilar src"; exit 2; }
fi
ref="$out/fxc-ref.exe"; new="$out/fxc-new.exe"
[ -x "$ref" ] && [ -x "$new" ] || { echo "Faltan $ref o $new" >&2; exit 2; }

work="$out/compare"
rm -rf "$work"; mkdir -p "$work/ref" "$work/new"
fail=0; n=0
for prg in "$root"/tests/fxc/*.prg; do
    name=$(basename "$prg" .prg); n=$((n+1))
    for v in ref new; do
        cp "$prg" "$work/$v/$name.prg"
        ( cd "$work/$v" && "$out/fxc-$v.exe" "$name.prg" > "$name.out" 2>&1; echo $? > "$name.rc" )
        tail -n +2 "$work/$v/$name.out" > "$work/$v/$name.txt"     # sin la 1.a linea (version y fecha de compilacion)
    done
    problemas=""
    cmp -s "$work/ref/$name.rc" "$work/new/$name.rc"   || problemas="$problemas codigo-de-salida"
    cmp -s "$work/ref/$name.txt" "$work/new/$name.txt" || problemas="$problemas salida"
    if [ -f "$work/ref/$name.dcb" ] || [ -f "$work/new/$name.dcb" ]; then
        cmp -s "$work/ref/$name.dcb" "$work/new/$name.dcb" || {
            # Programas que usan nombres que ahora son alias (LOAD_MAP -> LOAD_IMAGE): su .dcb cambia a proposito
            grep -qx "$name" "$root/tests/fxc/dcb-distinto-esperado.txt" 2>/dev/null || problemas="$problemas dcb"
        }
    fi
    if [ -z "$problemas" ]; then
        printf "[OK] %-10s rc=%s %s\n" "$name" "$(cat "$work/new/$name.rc")" "$( [ -f "$work/new/$name.dcb" ] && echo "dcb $(wc -c < "$work/new/$name.dcb") bytes" || echo "sin dcb (error esperado)")"
    else
        printf "[X]  %-10s distinto en:%s\n" "$name" "$problemas"; fail=1
    fi
done
echo "$n programas; $( [ $fail -eq 0 ] && echo 'todo coincide con la referencia' || echo 'HAY DIFERENCIAS' )"
exit $fail
