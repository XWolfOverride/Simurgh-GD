// Directiva //$ancient on: activa las funciones de programas antiguos (DIV) que Simurgh solo acepta como funciones vacias
// (load_pal, set_colors, get_colors). Se puede apagar con //$ancient off; tambien vale el comentario de bloque.
program t_ancient;
global
    int ok = 0, bad = 0;
    byte pal[2];
//$ancient on
begin
    set_mode(320, 200, 16);
    chk(load_pal("no_existe.pal") == 1, "load_pal no hace nada y no falla (//$ancient on)");
    chk(set_colors(0, 1, &pal) == 1, "set_colors no hace nada");
    chk(get_colors(0, 1, &pal) == 1, "get_colors no hace nada");
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
