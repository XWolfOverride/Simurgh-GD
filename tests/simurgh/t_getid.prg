// GET_ID no devuelve nunca al proceso que lo llama (como en DIV): un proceso unico de su tipo no se encuentra a si mismo
program t_getid;
global
    int ok = 0, bad = 0, a1, a2, b1;
local
    int encontrado, cuantos0;
begin
    a1 = unico();
    frame;
    chk(a1.encontrado == 0, "un proceso unico de su tipo no se encuentra a si mismo con get_id(type)");
    chk(a1.cuantos0 == 0, "get_id(0) con un solo proceso mas (el programa) no devuelve al propio proceso");
    a2 = pareja();
    b1 = pareja();
    frame; frame; frame;
    chk(a2.encontrado == b1 && b1.encontrado == a2, "dos procesos del mismo tipo se encuentran el uno al otro");
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
process unico()
begin
    encontrado = get_id(type unico);
    // get_id(0): el unico otro proceso es el programa principal; solo deberia salir una vez
    cuantos0 = 0;
    while (get_id(0) <> 0) cuantos0++; end;
    cuantos0 = cuantos0 - 1;
    loop frame; end;
end
process pareja()
begin
    frame;
    encontrado = get_id(type pareja);
    loop frame; end;
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
