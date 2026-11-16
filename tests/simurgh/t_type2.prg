// TYPE usado en procesos: privadas, locales, parametros
program t_type2;
type punto
    int x;
    int y;
end
local
    punto pos;
global
    punto g;
    int ok = 0, bad = 0;
begin
    g.x = 1; g.y = 2;
    a();
    frame;
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
process a()
private
    punto q;
begin
    q.x = 5; q.y = 6;
    pos.x = 9;
    ok++;
    say("privada " + q.x + " " + q.y + " local " + pos.x);
    suma(&q);
    say("tras puntero " + q.x);
end
process suma(punto pointer pp)
begin
    pp.x = pp.x + 100;
end
