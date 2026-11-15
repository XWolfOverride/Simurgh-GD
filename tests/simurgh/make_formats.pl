# Genera un BMP de 8 bits con paleta y un TGA con RLE (4x1 pixeles: rojo, rojo, verde, azul) para probar los cargadores.
#   perl make_formats.pl directorio
use strict;
my $d = shift or die "uso: make_formats.pl directorio\n";
# BMP 8 bits: indices 0,1,2,3 -> rojo, verde, azul, negro
open(my $f, ">:raw", "$d/t_p8.bmp") or die;
my $pal = pack("C*", 0,0,255,0,  0,255,0,0,  255,0,0,0,  0,0,0,0);          # B,G,R,0
my $data = pack("C*", 0,1,2,3);
print $f "BM", pack("V", 54 + 16 + 4), pack("vv", 0, 0), pack("V", 54 + 16);
print $f pack("VVVvvVVVVVV", 40, 4, 1, 1, 8, 0, 4, 2835, 2835, 4, 0), $pal, $data;
close $f;
# TGA tipo 10 (RLE color real), 32 bits con alfa, origen arriba: paquete repetido x2 (rojo) + paquete literal x2 (verde, azul)
open($f, ">:raw", "$d/t_rle.tga") or die;
print $f pack("C*", 0,0,10, 0,0,0,0,0, 0,0,0,0), pack("vv", 4, 1), pack("CC", 32, 0x28);
print $f pack("C*", 0x80 | 1, 0,0,255,255);                                      # rojo (B,G,R,A) x2
print $f pack("C*", 1, 0,255,0,255,  255,0,0,255);                               # literal x2: verde, azul
close $f;
