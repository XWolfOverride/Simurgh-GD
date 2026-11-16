// Audio: cargar, reproducir y descargar WAV y OGG sin que el programa se caiga (WAV sin datos, ficheros que no son audio,
// descargas repetidas y numeros que no existen). Los manejadores son numeros pequenos (1, 2, 3...), no punteros.
program t_audio;
global
    int w, m, c, i, ok = 0, bad = 0;
begin
    set_mode(320, 200, 16);

    for (i = 0; i < 20; i++)
        w = load_wav("t_ok.wav");
        chk(w > 0, "cargar wav valido");
        c = play_wav(w, 0);
        frame;
        stop_wav(c);
        chk(unload_wav(w) == 0, "descargar wav");
    end;
    chk(unload_wav(w) == -1, "descargar dos veces el mismo wav da -1");
    chk(unload_wav(123456) == -1, "descargar un wav que no existe da -1");
    chk(play_wav(w, 0) == -1, "reproducir un wav ya descargado da -1");
    chk(set_wav_volume(w, 64) == -1, "volumen de un wav ya descargado da -1");

    chk(load_wav("t_empty.wav") == -1, "wav sin datos: error y no se cae");
    chk(load_wav("t_empty.wav") == -1, "wav sin datos otra vez");
    chk(load_wav("t_bad.wav") == -1, "fichero que no es audio");
    chk(load_wav("no_existe.wav") == -1, "fichero que no existe");

    for (i = 0; i < 4; i++)
        m = load_song("NOOKIE.ogg");
        chk(m > 0, "cargar ogg");
        chk(play_song(m, 0) <> -1, "reproducir ogg");
        for (c = 0; c < 10; c++) frame; end;
        if (i % 2 == 0) stop_song(); end;       // unas veces se para antes de descargar y otras no
        chk(unload_song(m) == 0, "descargar ogg");
    end;
    chk(unload_song(m) == -1, "descargar dos veces el mismo ogg da -1");
    chk(unload_song(987654) == -1, "descargar un ogg que no existe da -1");
    chk(load_song("t_bad.wav") == -1, "cargar como musica algo que no es musica");
    m = load_song("NOOKIE.ogg");
    chk(unload_song(m) == 0, "cargar y descargar sin reproducir");

    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
