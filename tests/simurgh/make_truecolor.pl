# Genera ficheros MAP/FPG de 24 y 32 bits de prueba (formato m24/m32/f24/f32 de Simurgh).
#   perl make_truecolor.pl directorio
use strict;
my $d = shift or die "uso: make_truecolor.pl directorio\n";
sub name32 { my $n = shift; return substr($n . ("\0" x 32), 0, 32); }
sub map_file {
    my ($file, $magic, $bytes, @px) = @_;     # @px: lista de [r,g,b,(a)]
    open(my $f, ">:raw", "$d/$file") or die;
    print $f $magic . "\x1A\x0D\x0A\0\0";
    print $f pack("vvl<", scalar(@px), 1, 7), name32("truecolor"), pack("v", 0);
    print $f pack("C*", @$_) for @px;
    close $f;
}
sub fpg_file {
    my ($file, $magic, $bytes, @graphs) = @_; # cada grafico: [codigo, [px...]]
    open(my $f, ">:raw", "$d/$file") or die;
    print $f $magic . "\x1A\x0D\x0A\0\0";
    for my $g (@graphs) {
        my ($code, $px) = @$g;
        my $w = scalar(@$px);
        print $f pack("l<l<", $code, 64 + $w * $bytes), name32("g$code"), "\0" x 12, pack("l<l<l<", $w, 1, 0);
        print $f pack("C*", @$_) for @$px;
    }
    close $f;
}
my @p24 = ([255,0,0], [0,255,0], [0,0,255], [0,0,0]);
my @p32 = ([255,0,0,255], [0,255,0,255], [0,0,0,255], [255,0,0,0]);
map_file("t_tc24.map", "m24", 3, @p24);
map_file("t_tc32.map", "m32", 4, @p32);
fpg_file("t_tc24.fpg", "f24", 3, [1, \@p24], [2, \@p24]);
fpg_file("t_tc32.fpg", "f32", 4, [1, \@p32], [5, \@p32]);
# Alfa parcial: se debe conservar al volver a grabar (4x1, rojo con alfa 255, 128, 100 y 0)
map_file("t_al32.map", "m32", 4, [255,0,0,255], [255,0,0,128], [255,0,0,100], [255,0,0,0]);
