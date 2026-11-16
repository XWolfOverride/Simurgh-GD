// TYPE: tipos de datos propios (estructuras con nombre), declarados al nivel del programa
program t_type;
type punto
    int x;
    int y;
end
global
    punto p;
    punto lista[2];
    int ok = 0, bad = 0;
begin
    p.x = 3; p.y = 4;
    lista[1].x = 7;
    chk(p.x == 3 && p.y == 4, "campos de un tipo propio");
    chk(lista[1].x == 7, "vector de tipo propio");
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
