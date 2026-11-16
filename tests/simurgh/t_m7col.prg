// collision() segun el tipo de coordenadas. En el modo 7 es una colision de esferas: hay choque si la distancia entre los dos procesos,
// contando la altura (z, en cuartos de pixel), es menor que la suma de sus radios (radius; si vale 0, la mitad del ancho del grafico).
// Entre procesos de tipos de coordenadas distintos (pantalla, scroll, modo 7) nunca hay colision. Al final, los procesos solidos (solid) del modo 7.
program t_m7col;
global
    int ok = 0, bad = 0, g, a, b, c, d, e, f, s, k;
local
    int visto;
begin
    set_mode(320, 200, 16);
    g = new_map(20, 20, 16);
    map_clear(0, g, rgb(255, 0, 0));
    a = bola(100, 100, 0, 0, c_m7);       // radio 10 (mitad del ancho del grafico)
    b = bola(115, 100, 0, 0, c_m7);       // a 15 de a: 15 < 10 + 10
    frame; frame;
    chk(a.visto == b and b.visto == a, "dos procesos del modo 7 a menos distancia que la suma de sus radios");
    signal(b, s_kill);
    c = bola(125, 100, 0, 0, c_m7);       // a 25 de a: no llega
    frame; frame;
    chk(a.visto == 0, "a mas distancia que la suma de los radios no hay colision");
    c.radius = 20;                        // 25 < 10 + 20
    frame; frame;
    chk(a.visto == c, "radius agranda el alcance de la colision");
    signal(c, s_kill);
    d = bola(100, 100, 200, 0, c_m7);     // misma x, y; 200 de z = 50 pixeles mas arriba
    frame; frame;
    chk(a.visto == 0, "la altura (z) cuenta: encima, pero lejos, no hay colision");
    d.z = 40;                             // 10 pixeles mas arriba: 10 < 20
    frame; frame;
    chk(a.visto == d, "con poca diferencia de altura si hay colision");
    signal(d, s_kill);
    e = bola(100, 100, 0, 0, c_screen);   // mismo sitio, pero en coordenadas de pantalla
    frame; frame;
    chk(a.visto == 0 and e.visto == 0, "entre tipos de coordenadas distintos no hay colision");
    f = bola(105, 100, 0, 0, c_screen);
    frame; frame;
    chk(e.visto == f, "en pantalla sigue siendo la colision por graficos");
    s = bola(100, 100, 0, 15, c_m7);      // sin grafico, solo con radius
    s.graph = 0;
    frame; frame;
    chk(a.visto == s, "un proceso sin grafico colisiona en el modo 7 si tiene radius");
    // Procesos solidos (solid): con una ventana del modo 7 abierta, un solido detiene a los demas a la suma de los radios
    // (10 + 10), resbalan a su alrededor y tocarlo es colision
    signal(type bola, s_kill);
    frame;
    a = bola(100, 100, 0, 0, c_m7); a.solid = solid_sphere;
    b = bola(60, 100, 0, 0, c_m7);
    start_mode7(0, 0, g, 0, 0, 50); m7.camera = b;
    frame;
    for (k = 0; k < 8; k++) b.x += 5; frame; end;
    chk(b.x == 80 and b.y == 100, "modo 7: un proceso solido detiene a los demas (x=" + b.x + ")");
    frame; frame;
    chk(b.visto == a and a.visto == b, "modo 7: tocar un proceso solido es colision");
    a.solid = 0;
    frame; frame;
    chk(b.visto == 0, "modo 7: sin solido, en el limite del radio no hay colision");
    for (k = 0; k < 4; k++) b.x += 5; frame; end;
    frame;
    chk(b.x == 100 and b.visto == a, "modo 7: y se puede entrar en su radio (x=" + b.x + ")");
    // Esfera: cuenta la altura (z, en cuartos de pixel). A 30 de altura (7,5 pixeles) el contacto es a 18,5 en vez de a 20
    a.solid = solid_sphere;
    b.x = 60; b.z = 30; frame;
    for (k = 0; k < 8; k++) b.x += 5; frame; end;
    chk(b.x >= 81 and b.x <= 82, "modo 7: esfera, la altura acerca el punto de contacto (x=" + b.x + ")");
    b.x = 60; b.z = 200; frame;
    for (k = 0; k < 8; k++) b.x += 5; frame; end;
    chk(b.x == 100, "modo 7: esfera, muy por encima pasa (x=" + b.x + ")");
    // Cilindro: el radio en el plano, mientras las alturas se solapen (el grafico mide 20 de alto)
    a.solid = solid_cylinder;
    b.x = 60; b.z = 30; frame;
    for (k = 0; k < 8; k++) b.x += 5; frame; end;
    chk(b.x == 100, "modo 7: cilindro, por encima de su alto pasa (x=" + b.x + ")");
    b.x = 60; b.z = 15; frame;
    for (k = 0; k < 8; k++) b.x += 5; frame; end;
    chk(b.x == 80, "modo 7: cilindro, dentro de su alto lo detiene a la suma de los radios (x=" + b.x + ")");
    b.x = 60; b.y = 97; frame;
    for (k = 0; k < 30; k++) b.x += 5; frame; end;
    chk(b.x > 130 and b.y < 85 and b.y > 75, "modo 7: resbala alrededor del solido (" + b.x + "," + b.y + ")");
    stop_mode7(0);
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
process bola(x, y, z, radius, ctype)
begin
    graph = g;
    loop
        visto = collision(type bola);
        frame;
    end;
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
