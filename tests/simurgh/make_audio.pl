# Genera los ficheros de audio de prueba en el directorio dado: un WAV valido (0,1 s de silencio), un WAV sin datos (solo la
# cabecera de 44 bytes) y un fichero que no es audio.
use strict;
my $dir = shift || ".";
sub wav { my ($n) = @_;
    return "RIFF" . pack("V", 36 + $n) . "WAVEfmt " . pack("VvvVVvv", 16, 1, 1, 22050, 44100, 2, 16) . "data" . pack("V", $n) . ("\0" x $n); }
for my $f (["t_ok.wav", wav(4410)], ["t_empty.wav", wav(0)], ["t_bad.wav", "esto no es un fichero de audio" x 20]) {
    open(my $o, ">:raw", "$dir/$f->[0]") or die "no se pudo crear $f->[0]";
    print $o $f->[1]; close $o;
}
