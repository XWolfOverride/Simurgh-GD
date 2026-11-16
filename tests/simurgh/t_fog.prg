// Niebla del modo 7: un camino de postes rojos que se alejan de la camara, con niebla gris (smog). Sin argumentos guarda una captura
// (fog_a.png) y termina; con -i se puede recorrer (W/S avanzar, flechas girar, F/G cambia fog_distance, N cambia el modo de la niebla).
program t_fog;
global
    int suelo, cam, k, i, manual = 0, modo = 0, fd = 600, ok = 0, bad = 0, m, r1, g1, b1, r2, g2, b2;
begin
    set_mode(640, 480, 16);
    suelo = new_map(64, 64, 16);
    map_clear(0, suelo, rgb(50, 150, 50));
    drawing_map(0, suelo);
    drawing_color(rgb(40, 120, 40)); draw_box(0, 0, 31, 31); draw_box(32, 32, 63, 63);
    start_mode7(0, 0, 0, suelo, 0, 150);
    m7[0].height = 64; m7[0].distance = 64;
    m7[0].fog_distance = fd;
    m7[0].fog_color = rgb(170, 170, 170);                // niebla gris
    cam = camara();
    for (i = 0; i < 10; i++)                              // postes a cada 60 pixeles, a izquierda y derecha del camino
        poste(128 + 60 + i * 60, 128 - 40);
        poste(128 + 60 + i * 60, 128 + 40);
    end;
    if (argc >= 1 && argv[1] == "-i") manual = 1; loop frame; end; end;
    for (k = 0; k < 5; k++) frame; end;
    m = get_screen();
    save_image(0, m, "fog_a.png");
    get_rgb(map_get_pixel(0, m, 485, 190), &r1, &g1, &b1);      // poste cercano
    get_rgb(map_get_pixel(0, m, 253, 165), &r2, &g2, &b2);      // poste lejano
    chk(r1 > 190 and g1 < 70, "poste cercano: casi rojo puro (" + r1 + "," + g1 + "," + b1 + ")");
    chk(g2 > g1 + 25 and b2 > b1 + 25, "poste lejano: se tine hacia el gris de la niebla (" + r2 + "," + g2 + "," + b2 + ")");
    chk(r2 < r1, "y pierde parte del rojo");
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
process camara()
begin
    x = 128; y = 128; angle = 0;
    m7[0].camera = id;
    loop
        if (manual)
            if (key(_w) or key(_up))   x += get_distx(angle, 3);          y += get_disty(angle, 3);          end;
            if (key(_s) or key(_down)) x += get_distx(angle + 180000, 3); y += get_disty(angle + 180000, 3); end;
            if (key(_left))  angle += 2500; end;
            if (key(_right)) angle -= 2500; end;
            if (key(_f)) fd += 10; m7[0].fog_distance = fd; end;
            if (key(_g)) fd -= 10; m7[0].fog_distance = fd; end;
            if (key(_esc)) exit("", 0); end;
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
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
