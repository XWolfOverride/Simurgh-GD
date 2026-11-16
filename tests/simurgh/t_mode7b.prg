// Dos modos 7 uno sobre otro: el de arriba (pista) tiene fondo transparente (color 0) y deja ver el de abajo (suelo), como la pista
// suspendida de F-Zero. Cada capa tiene su propia altura de camara, asi que la pista y el suelo se mueven con distinta perspectiva.
program t_mode7b;
global
    int suelo, pista, cam, k, ok = 0, bad = 0, a, b, c, d;
    int manual = 0;
begin
    set_mode(640, 480, 16);
    suelo = new_map(64, 64, 16);
    map_clear(0, suelo, rgb(40, 140, 40));
    pista = new_map(512, 512, 16);
    map_clear(0, pista, 0);                                   // todo transparente
    drawing_map(0, pista);
    drawing_color(rgb(200, 30, 30));
    draw_box(0, 100, 511, 156);                               // una pista roja de 56 pixeles de ancho, a lo largo del eje x
    start_mode7(0, 0, 0, suelo, 0, 150);                     // capa de abajo: suelo que se repite
    start_mode7(1, 0, pista, 0, 0, 150);                     // capa de arriba: la pista (fuera de ella, transparente)
    m7[0].height = 96;  m7[0].distance = 64;  m7[0].z = 300; // se pinta primero (z mayor = mas al fondo)
    m7[1].height = 64;  m7[1].distance = 64;  m7[1].z = 256;
    cam = camara();
    if (argc >= 1 && argv[1] == "-i")
        manual = 1;
        loop frame; end;
    end;
    for (k = 0; k < 5; k++) frame; end;
    k = get_screen();
    save_image(0, k, "m7b_a.png");
    a = map_get_pixel(0, k, 320, 300);                         // centro de la pista
    b = map_get_pixel(0, k, 20, 300);                          // lado izquierdo, fuera de la pista
    c = map_get_pixel(0, k, 320, 200);                         // lejos, debajo del horizonte
    chk(a == rgb(200, 30, 30), "la pista (capa de arriba) se ve");
    chk(b == rgb(40, 140, 40), "fuera de la pista se ve el suelo (capa de abajo)");
    chk(map_get_pixel(0, k, 320, 100) <> rgb(200, 30, 30), "encima del horizonte no hay pista");
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
process camara()
begin
    x = 128; y = 128; angle = 0;
    m7[0].camera = id; m7[1].camera = id;
    loop
        if (manual)
            if (key(_w) or key(_up))   x += get_distx(angle, 3);          y += get_disty(angle, 3);          end;
            if (key(_s) or key(_down)) x += get_distx(angle + 180000, 3); y += get_disty(angle + 180000, 3); end;
            if (key(_a))               x += get_distx(angle + 90000, 3);  y += get_disty(angle + 90000, 3);  end;
            if (key(_d))               x += get_distx(angle - 90000, 3);  y += get_disty(angle - 90000, 3);  end;
            if (key(_left))  angle += 2500; end;
            if (key(_right)) angle -= 2500; end;
            if (key(_r)) m7[0].height += 1; m7[1].height += 1; end;
            if (key(_f)) m7[0].height -= 1; m7[1].height -= 1; end;
            if (key(_esc)) exit("", 0); end;
        end;
        frame;
    end;
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
