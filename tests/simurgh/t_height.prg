// //$ancient on: la variable local "height" (altura de un proceso del modo 7 en DIV) es un alias de z. Una variable propia llamada
// height tiene preferencia.
program t_height;
//$ancient on
global
    int ok = 0, bad = 0, a, b;
begin
    a = alias();
    b = propia();
    frame;
    chk(a.z == 7, "height = 10; z = 7 -> el alias y z son la misma variable (z = " + a.z + ")");
    chk(a.height == 7, "acceso remoto (otro.height) tambien es z");
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
process alias()
begin
    height = 10;
    chk(z == 10, "height = 10 -> z vale 10");
    z = 7;
    chk(height == 7, "z = 7 -> height vale 7");
    loop frame; end;
end
process propia()
private int height;
begin
    height = 99;
    chk(height == 99 and z == 0, "una variable propia llamada height tiene preferencia (height = " + height + ", z = " + z + ")");
    loop frame; end;
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
