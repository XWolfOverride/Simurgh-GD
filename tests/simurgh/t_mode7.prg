// Modo 7: suelo en perspectiva y procesos sobre el. Un marcador amarillo en el mapa y un proceso (barra vertical roja) en la
// misma posicion: la base del proceso debe quedar sobre el marcador, y a la misma distancia el tamano debe ser coherente.
//
// Sin argumentos comprueba y termina (tools/image-test.sh). Con el argumento -i es interactivo (tools/mode7.cmd):
//   W / S o flecha arriba / abajo   avanzar / retroceder
//   A / D                           desplazarse a izquierda / derecha (sin girar)
//   flecha izquierda / derecha      girar la camara
//   R / F                           subir / bajar la camara (m7.height)
//   T / G                           mas / menos zoom (m7.focus)
//   Y / H                           mover el horizonte (m7.horizon)
//   Esc                             salir
program t_mode7;
global
    int mapa, fondo, cam, sp, k, yb, ym, yy, ok = 0, bad = 0;
    int manual = 0, gx, gy, ga, gh, gf, ghz, vc, xp;
    int vistas[4];
    int r0, g0, b0;
begin
    set_mode(640, 480, 16);
    mapa = new_map(512, 512, 16);
    map_clear(0, mapa, rgb(60, 120, 60));
    drawing_map(0, mapa);
    for (k = 0; k < 512; k += 32)
        drawing_color(rgb(200, 200, 200)); draw_line(k, 0, k, 511);
        draw_line(0, k, 511, k);
    end;
    drawing_color(rgb(255, 255, 0)); draw_box(292, 120, 308, 136);        // marcador en (300, 128)
    drawing_color(rgb(255, 0, 0));   draw_box(100, 100, 110, 110);
    fondo = new_map(64, 64, 16);
    map_clear(0, fondo, rgb(40, 40, 120));
    drawing_map(0, fondo);
    drawing_color(rgb(90, 90, 200)); draw_box(0, 0, 31, 31); draw_box(32, 32, 63, 63);
    start_mode7(0, 0, mapa, fondo, 0, 150);
    m7[0].height = 64;
    m7[0].distance = 64;
    cam = camara();
    sp = poste(300, 128);
    if (argc >= 1 && argv[1] == "-i")
        interactivo();
        loop frame; end;          // el programa vive hasta que el proceso interactivo termina con Esc
    end;
    for (k = 0; k < 6; k++) frame; end;
    k = get_screen();
    save_image(0, k, "m7_a.png");
    comprobar(k);
    vistas_xgraph();
    mapas8();
    niebla();
    altura_z();
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
process camara()
begin
    x = 128; y = 128; angle = 0;
    m7[0].camera = id;
    loop
        if (manual)
            if (key(_w) or key(_up))    x += get_distx(angle, 3);          y += get_disty(angle, 3);          end;
            if (key(_s) or key(_down))  x += get_distx(angle + 180000, 3); y += get_disty(angle + 180000, 3); end;
            if (key(_a))                x += get_distx(angle + 90000, 3);  y += get_disty(angle + 90000, 3);  end;
            if (key(_d))                x += get_distx(angle - 90000, 3);  y += get_disty(angle - 90000, 3);  end;
            if (key(_left))  angle += 2500; end;
            if (key(_right)) angle -= 2500; end;
            if (key(_r)) m7[0].height += 1; end;
            if (key(_f)) m7[0].height -= 1; end;
            if (key(_t)) m7[0].focus += 4; end;
            if (key(_g)) m7[0].focus -= 4; end;
            if (key(_y)) m7[0].horizon += 1; end;
            if (key(_h)) m7[0].horizon -= 1; end;
            gx = x; gy = y; ga = angle / 1000; gh = m7[0].height; gf = m7[0].focus; ghz = m7[0].horizon;
        end;
        frame;
    end;
end
process poste(x, y)
begin
    graph = new_map(20, 80, 16);
    map_clear(0, graph, rgb(220, 30, 30));
    drawing_map(0, graph); drawing_color(rgb(255, 255, 255)); draw_box(0, 0, 19, 4);
    ctype = c_m7;
    loop frame; end;
end
function comprobar(int m)
private int yy, yb, ym;
begin
    yb = -1; ym = -1;
    for (yy = 100; yy < 300; yy++)
        if (map_get_pixel(0, m, 320, yy) == rgb(220, 30, 30)) yb = yy; end;        // ultima fila roja del poste
        if (map_get_pixel(0, m, 308, yy) == rgb(255, 255, 0)) ym = yy; end;        // fila del marcador (amarillo)
    end;
    chk(yb > 0 && ym > 0, "se ven el poste y el marcador (yb=" + yb + " ym=" + ym + ")");
    chk(abs(yb - ym) <= 2, "la base del proceso queda sobre su marcador del suelo");
    chk(map_get_pixel(0, m, 200, 430) == rgb(60, 120, 60), "suelo debajo del horizonte");
    chk(map_get_pixel(0, m, 100, 400) <> map_get_pixel(0, m, 100, 120), "encima del horizonte no hay suelo");
    chk(map_get_pixel(0, m, 320, 100) == map_get_pixel(0, m, 100, 100), "el poste no sobresale mas de lo que debe");
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end

// Modo interactivo: la camara se mueve con el teclado y se ve el estado en pantalla
process interactivo()
begin
    manual = 1;
    poste(200, 200); poste(400, 300); poste(300, 400); poste(150, 60); poste(450, 100);
    set_text_color(rgb(255, 255, 255));
    write(0, 6, 6, 0, "x");       write_int(0, 20, 6, 0, &gx);
    write(0, 60, 6, 0, "y");      write_int(0, 74, 6, 0, &gy);
    write(0, 114, 6, 0, "angulo"); write_int(0, 160, 6, 0, &ga);
    write(0, 200, 6, 0, "altura"); write_int(0, 246, 6, 0, &gh);
    write(0, 280, 6, 0, "zoom");   write_int(0, 316, 6, 0, &gf);
    write(0, 350, 6, 0, "horizonte"); write_int(0, 420, 6, 0, &ghz);
    write(0, 6, 466, 0, "WASD mover  flechas girar/avanzar  R/F altura  T/G zoom  Y/H horizonte  Esc salir");
    loop
        if (key(_esc)) exit("", 0); end;
        frame;
    end;
end

// xgraph en modo 7: la vista que se elige depende del angulo del proceso RESPECTO a la camara (cuatro vistas, una por color)
function vistas_xgraph()
private int g, c1, c2, i;
begin
    signal(sp, s_kill);
    vistas[0] = 4;
    for (i = 1; i <= 4; i++)
        g = new_map(20, 80, 16);
        map_clear(0, g, rgb(10 + i * 50, 10 + i * 40, 255 - i * 40));
        vistas[i] = g;
    end;
    xp = coche_vistas(300, 128);
    for (i = 0; i < 4; i++) frame; end;
    c1 = map_get_pixel(0, get_screen(), 320, 170);          // proceso mirando en el mismo sentido que la camara
    xp.angle = 90000;                                        // gira un cuarto de vuelta
    for (i = 0; i < 4; i++) frame; end;
    c2 = map_get_pixel(0, get_screen(), 320, 170);
    chk(c1 <> c2, "xgraph: la vista cambia con el angulo del proceso respecto a la camara");
    chk(c1 == rgb(10 + 2 * 50, 10 + 2 * 40, 255 - 2 * 40), "xgraph: de espaldas a la camara se ve la vista 2");
    chk(c2 == rgb(10 + 3 * 50, 10 + 3 * 40, 255 - 3 * 40), "xgraph: girado 90 grados se ve la vista 3");
end
process coche_vistas(x, y)
begin
    xgraph = &vistas[0];
    ctype = c_m7;
    loop frame; end;
end

// Un mapa de 8 bits se crea de 16 (no hay modo de 256 colores) y sirve igual de fondo repetido del modo 7
function mapas8()
private int m8, i, a, b;
begin
    signal(xp, s_kill);
    m8 = new_map(64, 64, 8);
    map_clear(0, m8, rgb(60, 120, 20));
    start_mode7(0, 0, 0, m8, 0, 150);
    m7[0].height = 64;
    m7[0].distance = 64;
    m7[0].camera = cam;
    for (i = 0; i < 4; i++) frame; end;
    a = map_get_pixel(0, get_screen(), 200, 430);
    b = map_get_pixel(0, get_screen(), 420, 450);
    chk(graphic_info(0, m8, g_depth) == 16, "new_map de 8 bits crea un mapa de 16");
    chk(a <> 0 and a == b, "mapa de 8 bits sobre pantalla de 16: suelo de un solo color, distinto de 0");
end

// Niebla: lo lejano se funde con el color de la niebla (negro = se pierde en la oscuridad); lo cercano casi no cambia
function brillo(int color)
begin
    get_rgb(color, &r0, &g0, &b0);
    return r0 + g0 + b0;
end
function niebla()
private int cerca0, lejos0, cerca1, lejos1, i;
begin
    for (i = 0; i < 3; i++) frame; end;
    cerca0 = brillo(map_get_pixel(0, get_screen(), 320, 470));
    lejos0 = brillo(map_get_pixel(0, get_screen(), 320, 170));
    m7[0].fog_distance = 250;
    m7[0].fog_color = 0;
    for (i = 0; i < 3; i++) frame; end;
    cerca1 = brillo(map_get_pixel(0, get_screen(), 320, 470));
    lejos1 = brillo(map_get_pixel(0, get_screen(), 320, 170));
    chk(lejos1 < lejos0 / 4, "niebla negra: lo lejano se oscurece mucho (" + lejos0 + " -> " + lejos1 + ")");
    chk(cerca1 > cerca0 * 6 / 10, "niebla negra: lo cercano casi no cambia (" + cerca0 + " -> " + cerca1 + ")");
    m7[0].fog_color = rgb(255, 255, 255);
    for (i = 0; i < 3; i++) frame; end;
    chk(brillo(map_get_pixel(0, get_screen(), 320, 170)) > lejos0, "niebla blanca: lo lejano se aclara");
    m7[0].fog_distance = 0;
    for (i = 0; i < 3; i++) frame; end;
    chk(brillo(map_get_pixel(0, get_screen(), 320, 170)) == lejos0, "fog_distance 0: sin niebla");
end

// La altura de un proceso del modo 7 es su z: a mayor z, su base queda mas arriba en pantalla
function ultima_fila_roja(int m)
private int yy, yb;
begin
    yb = -1;
    for (yy = 100; yy < 479; yy++)
        if (map_get_pixel(0, m, 320, yy) == rgb(220, 30, 30)) yb = yy; end;
    end;
    return yb;
end
function altura_z()
private int p, base0, base1, i;
begin
    m7[0].fog_distance = 0;
    m7[0].height = 64;
    m7[0].distance = 64;
    p = poste(300, 128);
    for (i = 0; i < 4; i++) frame; end;
    base0 = ultima_fila_roja(get_screen());
    p.z = 40;
    for (i = 0; i < 4; i++) frame; end;
    base1 = ultima_fila_roja(get_screen());
    chk(base0 > 0 and base1 > 0 and base1 < base0 - 5, "z es la altura del proceso: con z = 40 la base sube en pantalla (" + base0 + " -> " + base1 + ")");
end
