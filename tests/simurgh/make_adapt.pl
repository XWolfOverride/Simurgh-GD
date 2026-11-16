# Genera un MAP y un FPG de 8 bits (formato de DIV, con paleta) en el directorio dado: t_p8.map y t_p8.fpg.
# Paleta: 1 = rojo, 2 = verde, 3 = azul, 4 = casi negro (1,1,1), el resto negro. Graficos de 4x2:  1 2 3 0 / 4 1 1 1
use strict;
my $dir = shift || ".";
my $pal = "";
my @c = ([0,0,0], [63,0,0], [0,63,0], [0,0,63], [1,1,1]);
for my $i (0..255) { my $e = $c[$i] || [0,0,0]; $pal .= pack("C3", @$e); }
$pal .= "\0" x 576;                                     # gamas
my @rows = ([1,2,3,0], [4,1,1,1]);
my $data = "";
for my $r (@rows) { $data .= pack("C4", @$r); }
sub map8 { my ($code) = @_;
    return "map\x1a\x0d\x0a\0\0" . pack("vvV", 4, 2, $code) . pack("a32", "grafico$code") . $pal . pack("v", 1) . pack("vv", 2, 1) . $data; }
open(my $m, ">:raw", "$dir/t_p8.map") or die; print $m map8(0); close $m;
open(my $f, ">:raw", "$dir/t_p8.fpg") or die;
print $f "fpg\x1a\x0d\x0a\0\0" . $pal;
for my $code (3, 7) {
    print $f pack("V", $code) . pack("V", 64 + 4 + 8 * 0 + 0) x 0;    # (se rellena a mano abajo)
}
close $f;
# FPG: cabecera 8 + paleta 768 + gamas 576; por grafico: codigo, longitud, nombre[32], fichero[12], ancho, alto, puntos de control (n) y 4*n bytes, datos
open($f, ">:raw", "$dir/t_p8.fpg") or die;
print $f "fpg\x1a\x0d\x0a\0\0" . $pal;
for my $code (3, 7) {
    my $body = pack("a32", "g$code") . pack("a12", "g$code.map") . pack("VVV", 4, 2, 1) . pack("vv", 2, 1) . $data;
    print $f pack("V", $code) . pack("V", 4 + 4 + length($body)) . $body;
}
close $f;
