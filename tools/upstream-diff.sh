#!/bin/sh
# Compara src/ (Simurgh) con upstream/fenix-0.92a/ (Fenix intacto).
#
#   tools/upstream-diff.sh [--fixed] [--names | --stat | --patch | --check] [fichero-salida]
#
#   --fixed  compara con upstream/fenix-0.92a-fixed (Fenix 0.92a corregido) en lugar de con upstream/fenix-0.92a (Fenix original). El aviso
#            de la GPL (--check) se exige siempre respecto al original: usar --check sin --fixed.
#
#   --names  lista de ficheros: A (anadido), D (borrado), M (modificado)   [por defecto]
#   --stat   resumen de lineas cambiadas por fichero
#   --patch  diff unificado completo (a la salida estandar o al fichero indicado)
#   --check  comprueba que cada fichero de texto MODIFICADO lleva el aviso de cambio
#            (GPL v2, apartado 2.a: «avisos destacados de que se han cambiado los ficheros y la fecha»);
#            sale con codigo 1 si falta alguno
#
# El aviso es una linea con la marca «Simurgh: modificado» (con la fecha del cambio) en los comentarios iniciales.
# Los ficheros anadidos por Simurgh no la necesitan: llevan su propia cabecera (ver DOC/ARQUITECTURA.md).
set -u
root=$(cd "$(dirname "$0")/.." && pwd)
oldrel=upstream/fenix-0.92a
new="$root/src"
marca='Simurgh: modificado'
modo=--names
for a in "$@"; do
    case "$a" in
        --names|--stat|--patch|--check) modo=$a; shift ;;
        --fixed) oldrel=upstream/fenix-0.92a-fixed; shift ;;
        *) break ;;
    esac
done
old="$root/$oldrel"
salida=${1:-}

[ -d "$old" ] && [ -d "$new" ] || { echo "Faltan $old o $new" >&2; exit 2; }

# Lista A/D/M ordenada por ruta relativa.
lista() {
    diff -rq "$old" "$new" | while IFS= read -r l; do
        case "$l" in
            "Files "*" differ")
                f=${l#Files $old/}; f=${f%% and *}; echo "M $f" ;;
            "Only in $old"*)
                d=${l#Only in }; d=${d%%: *}; n=${l##*: }; r=${d#$old}; r=${r#/}
                echo "D ${r:+$r/}$n" ;;
            "Only in $new"*)
                d=${l#Only in }; d=${d%%: *}; n=${l##*: }; r=${d#$new}; r=${r#/}
                echo "A ${r:+$r/}$n" ;;
        esac
    done | sort -k2
}

case $modo in
    --names) lista ;;
    --stat)  (cd "$root" && diff -ruN "$oldrel" src | diffstat 2>/dev/null) || {
                 echo "(sin 'diffstat': lista de ficheros)"; lista; } ;;
    --patch)
        if [ -n "$salida" ]; then
            (cd "$root" && diff -ruN "$oldrel" src) > "$salida"; rc=$?
            [ $rc -le 1 ] && echo "Diff escrito en $salida" || exit $rc
        else
            (cd "$root" && diff -ruN "$oldrel" src)
        fi
        exit 0 ;;
    --check)
        falta=0
        for f in $(lista | while read -r t r; do [ "$t" = M ] && echo "$r"; done); do
            # solo ficheros de texto
            if grep -Iq . "$new/$f" 2>/dev/null; then
                grep -q "$marca" "$new/$f" || { echo "Sin aviso de cambio: $f"; falta=1; }
            fi
        done
        [ $falta -eq 0 ] && echo "Todos los ficheros modificados llevan el aviso."
        exit $falta ;;
esac
exit 0
