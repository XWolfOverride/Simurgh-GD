// Modo 8 (mapas de sectores): carga de un mapa en el formato de texto (t_mode8.wld), dibujo de suelo, techo, paredes y escalones,
// procesos dentro del mapa (visibles por un paso, tapados por una pared), colisiones con las paredes, escalones (m8_step),
// las funciones de consulta y cambio del mapa, la niebla, el cielo, la linea de vision y los procesos solidos. Sin argumentos comprueba y termina (captura en mode8_a.png); con -i se recorre:
// W/S o arriba/abajo avanzar, flechas girar, A/D paso lateral, Q/Z subir y bajar, R/F mirar arriba y abajo, ESC salir.
program t_mode8;
global
    int ok = 0, bad = 0, tex, cam, sp, m, k, a, b, c, manual = 0;
    int verde, azul, rojo, amarillo, blanco, magenta, cian, cielo;
local
    int visto;
begin
    set_mode(320, 200, 16);
    verde = rgb(40, 200, 40); azul = rgb(40, 40, 200); rojo = rgb(200, 40, 40); amarillo = rgb(200, 200, 40);
    blanco = rgb(240, 240, 240); magenta = rgb(200, 40, 200); cian = rgb(40, 200, 200);
    tex = fpg_new();
    textura(1, verde); textura(2, azul); textura(3, rojo); textura(4, amarillo); textura(5, blanco); textura(6, magenta);
    chk(load_wld("no_existe.wld", tex) == 0, "load_wld de un fichero que no existe devuelve 0");
    chk(load_wld("t_mode8.wld", tex) == 1, "load_wld carga el mapa de texto");
    cam = camara();
    if (argc >= 1 && argv[1] == "-i") manual = 1; sp = objeto(550, 130); loop frame; end; end;
    frame; frame;
    chk(cam.x == 200 and cam.y == 200 and cam.z == 0, "go_to_flag coloca en la bandera, a la altura del suelo");
    chk(cam.m8_sector == 0, "m8_sector: la camara esta en el sector 0 (" + cam.m8_sector + ")");

    // Dibujo: camara en (200,200) mirando hacia el paso (x = 400)
    m = get_screen();
    save_image(0, m, "mode8_a.png");
    chk(map_get_pixel(0, m, 160, 190) == verde, "abajo, el suelo de la sala");
    chk(map_get_pixel(0, m, 160, 55) == azul, "arriba, el techo del pasillo");
    chk(map_get_pixel(0, m, 10, 100) == rojo, "a la izquierda, la pared de la sala");
    chk(map_get_pixel(0, m, 160, 100) == magenta, "al fondo, por los dos pasos, el escalon del sector 2");
    chk(map_get_pixel(0, m, 160, 104) == amarillo, "por el paso se ve el suelo del pasillo");
    chk(map_get_pixel(0, m, 160, 112) == magenta, "y el escalon de entrada al pasillo");
    chk(map_get_pixel(0, m, 160, 2) == blanco, "y el dintel (el techo del pasillo es mas bajo)");
    unload_map(0, m);

    // Un proceso en el pasillo: se ve por el paso
    sp = objeto(550, 130);
    frame; frame;
    chk(sp.m8_sector == 1 and sp.z == 24, "el proceso queda en el sector 1, sobre su suelo (" + sp.m8_sector + ", z=" + sp.z + ")");
    m = get_screen();
    save_image(0, m, "mode8_b.png");
    chk(map_get_pixel(0, m, 128, 95) == cian, "proceso visible por el paso");
    unload_map(0, m);
    // Desde (200,20) lo tapa la pared de la sala
    cam.x = 200; cam.y = 20; cam.m8_sector = -1;
    frame; frame;
    m = get_screen();
    chk(map_get_pixel(0, m, 210, 95) == rojo, "proceso tapado por una pared");
    unload_map(0, m);
    signal(sp, s_kill);

    // Movimiento: de la sala al pasillo (escalon de 24, m8_step = 32) y hasta el escalon del fondo (76: no sube)
    cam.x = 200; cam.y = 200; cam.m8_sector = -1;
    frame;
    for (k = 0; k < 8; k++) cam.x += 30; frame; end;
    chk(cam.m8_sector == 1 and cam.z == 24, "sube el escalon y entra en el sector 1 (" + cam.m8_sector + ", z=" + cam.z + ")");
    for (k = 0; k < 12; k++) cam.x += 30; frame; end;
    chk(cam.x <= 700 - 20 and cam.x >= 700 - 22, "el escalon alto lo detiene a la distancia de su radio (x=" + cam.x + ")");
    chk(cam.m8_wall == 7 and cam.m8_nextsector == 2, "m8_wall y m8_nextsector dicen con que ha chocado (" + cam.m8_wall + ", " + cam.m8_nextsector + ")");
    for (k = 0; k < 8; k++) cam.y -= 30; frame; end;
    chk(cam.y >= 100 + 20 and cam.y <= 100 + 22, "una pared solida lo detiene (y=" + cam.y + ")");
    chk(cam.m8_wall == 6 and cam.m8_nextsector == -1, "pared solida: m8_nextsector = -1 (" + cam.m8_wall + ", " + cam.m8_nextsector + ")");
    frame;
    chk(cam.m8_wall == -1, "sin moverse no hay choque");

    // Cambios en el mapa
    chk(get_sector_height(2, &a, &b) == 1 and a == 100 and b == 200, "get_sector_height");
    set_sector_height(2, 40, -1);
    get_sector_height(2, &a, &b);
    chk(a == 40 and b == 200, "set_sector_height con -1 mantiene el techo");
    cam.y = 200; frame;
    for (k = 0; k < 6; k++) cam.x += 30; frame; end;
    chk(cam.m8_sector == 2 and cam.z == 40, "bajado el suelo del sector 2, ya puede entrar (" + cam.m8_sector + ", z=" + cam.z + ")");
    chk(get_sector_texture(1, &a, &b, &c) == 1 and a == 4 and b == 2 and c == 15, "get_sector_texture");
    set_sector_texture(1, 1, -1, 8);
    get_sector_texture(1, &a, &b, &c);
    chk(a == 1 and b == 2 and c == 8, "set_sector_texture");
    chk(get_wall_texture(0, &a, &b) == 1 and a == 3 and b == 15, "get_wall_texture");
    set_wall_texture(0, 5, 10);
    get_wall_texture(0, &a, &b);
    chk(a == 5 and b == 10, "set_wall_texture");
    chk(get_point_m8(8, &a, &b) == 1 and a == 900 and b == 100, "get_point_m8");
    set_point_m8(8, 950, 90);
    get_point_m8(8, &a, &b);
    chk(a == 950 and b == 90, "set_point_m8");
    chk(get_sector_height(99, &a, &b) == 0, "un sector que no existe devuelve 0");

    // Luz y niebla: con luz 8 el suelo del pasillo (ahora verde) se oscurece hacia el color de ambiente (negro)
    cam.x = 200; cam.y = 200; cam.z = 0; cam.angle = 0; cam.m8_sector = -1;
    frame; frame;
    m = get_screen();
    get_rgb(map_get_pixel(0, m, 160, 104), &a, &b, &c);
    chk(b > 80 and b < 130, "luz 8: el suelo se ve a media luz (verde=" + b + ")");
    unload_map(0, m);
    set_env_color(100, 100, 100); set_fog(2, 5);
    frame; frame;
    m = get_screen();
    get_rgb(map_get_pixel(0, m, 10, 100), &a, &b, &c);
    chk(a > 230 and b > 230 and c > 230, "niebla: la pared lejana toma el color de ambiente (" + a + "," + b + "," + c + ")");
    chk(map_get_pixel(0, m, 160, 199) == verde, "y lo cercano no cambia");
    unload_map(0, m);

    // Niebla por ventana, como en el modo 7: m8.fog_distance y m8.fog_color. set_fog y set_env_color escriben esos campos
    chk(m8.fog_distance == 204 and m8.fog_color == rgb(255, 255, 255), "set_fog y set_env_color dejan la niebla en m8 (" + m8.fog_distance + ", " + m8.fog_color + ")");
    set_fog(0, 0);
    chk(m8.fog_distance == 0, "set_fog(0, 0) quita la niebla");
    sp = objeto(350, 200);
    m8.fog_distance = 300; m8.fog_color = rgb(0, 0, 255);
    frame; frame;
    m = get_screen();
    get_rgb(map_get_pixel(0, m, 10, 100), &a, &b, &c);
    chk(a < 100 and c > 150, "niebla de color: la pared lejana se tine (" + a + "," + b + "," + c + ")");
    get_rgb(map_get_pixel(0, m, 160, 199), &a, &b, &c);
    chk(b > 150 and c < 100, "y lo cercano, poco (" + a + "," + b + "," + c + ")");
    get_rgb(map_get_pixel(0, m, 160, 110), &a, &b, &c);
    chk(b > 80 and b < 150 and c > 190, "los procesos se tinen hacia el color de la niebla (" + a + "," + b + "," + c + ")");
    unload_map(0, m);
    m8.fog_color = -1;
    frame; frame;
    m = get_screen();
    get_rgb(map_get_pixel(0, m, 10, 100), &a, &b, &c);
    chk(a > 30 and a < 100 and c < 40, "fog_color = -1: se funde con lo que hay detras de la ventana (" + a + "," + b + "," + c + ")");
    get_rgb(map_get_pixel(0, m, 160, 110), &a, &b, &c);
    chk(b > 60 and b < 140 and c < 140, "y los procesos se desvanecen (" + a + "," + b + "," + c + ")");
    unload_map(0, m);
    m8.fog_distance = 150;
    frame; frame;
    m = get_screen();
    chk(map_get_pixel(0, m, 10, 100) == 0, "mas alla de fog_distance no se pinta");
    unload_map(0, m);
    m8.fog_distance = 0; m8.fog_color = 0;
    signal(sp, s_kill);

    // Cielo: lo pone el programa en cada ventana (set_sky_m8) y se ve por los techos sin textura. Gira con la camara y sube y
    // baja con m8.angle; ocupa un alto de ventana por encima del horizonte. El grafico es de 2 x 4: las dos filas de arriba verdes,
    // debajo cian y blanco, y en la ultima (la que toca el horizonte) amarillo y magenta.
    cielo = new_map(2, 4, 16);
    map_clear(0, cielo, verde);
    map_put_pixel(0, cielo, 0, 2, cian);     map_put_pixel(0, cielo, 1, 2, blanco);
    map_put_pixel(0, cielo, 0, 3, amarillo); map_put_pixel(0, cielo, 1, 3, magenta);
    set_sector_height(0, -1, 60);
    set_sector_texture(0, -1, 0, -1);
    frame; frame;
    m = get_screen();
    chk(map_get_pixel(0, m, 100, 10) == 0, "techo sin textura y sin cielo: se ve lo que hay detras de la ventana");
    unload_map(0, m);
    chk(set_sky_m8(0, cielo, 2) == 1 and set_sky_m8(0, 9999) == 0, "set_sky_m8 devuelve 1, o 0 si el grafico no existe");
    frame; frame;
    m = get_screen();
    chk(map_get_pixel(0, m, 100, 10) == blanco and map_get_pixel(0, m, 220, 10) == cian, "cielo: dos repeticiones por vuelta, media imagen en pantalla");
    chk(map_get_pixel(0, m, 100, 60) == magenta, "de frente se ve la mitad de abajo de la imagen, hasta el horizonte");
    unload_map(0, m);
    m8.angle = 64;
    frame; frame;
    m = get_screen();
    chk(map_get_pixel(0, m, 100, 60) == blanco, "el cielo baja al mirar hacia arriba (m8.angle)");
    unload_map(0, m);
    m8.angle = 0; cam.angle = 90000;
    frame; frame;
    m = get_screen();
    chk(map_get_pixel(0, m, 100, 10) == cian and map_get_pixel(0, m, 220, 10) == blanco, "el cielo gira con la camara");
    unload_map(0, m);
    set_sky_m8(0, 0);
    frame; frame;
    m = get_screen();
    chk(map_get_pixel(0, m, 100, 10) == 0, "set_sky_m8(0, 0) quita el cielo");
    unload_map(0, m);
    cam.angle = 0;
    set_sector_height(0, -1, 200);
    set_sector_texture(0, -1, 2, -1);

    // Linea de vision (m8_sight): desde los ojos (z + m8_height) del que mira hasta la parte de arriba, el centro o la base del otro
    sp = objeto(550, 130);
    frame; frame;
    chk(m8_sight(sp, cam) == sp, "m8_sight: se ve por el paso");
    chk(m8_sight(type objeto, cam) == sp, "m8_sight con un tipo de proceso devuelve el que se ve");
    chk(m8_sight(sp) == 0, "m8_sight desde un proceso que no esta en el modo 8 devuelve 0");
    cam.x = 200; cam.y = 20; cam.m8_sector = -1;
    frame; frame;
    chk(m8_sight(sp, cam) == 0 and m8_sight(type objeto, cam) == 0, "m8_sight: lo tapa una pared");
    signal(sp, s_kill);
    cam.x = 200; cam.y = 200; cam.m8_sector = -1;
    set_sector_height(2, 100, -1);
    sp = objeto(800, 200);
    sp.m8_height = 10;
    frame; frame;
    chk(sp.z == 100 and m8_sight(sp, cam) == 0, "m8_sight: un proceso bajo, detras de un escalon alto, no se ve (z=" + sp.z + ")");
    sp.m8_height = 60;
    chk(m8_sight(sp, cam) == sp, "m8_sight: si es mas alto que el escalon, asoma y se ve");
    cam.z = 100;
    chk(m8_sight(sp, cam) == sp, "m8_sight: desde mas arriba se ve");
    cam.z = 0;
    signal(sp, s_kill);
    set_sector_height(2, 40, -1);

    // Procesos solidos: un solido detiene a los demas a la suma de los radios (20 + 10) y tocarlo es colision
    sp = objeto(300, 200);
    sp.solid = solid_sphere;
    frame;
    for (k = 0; k < 12; k++) cam.x += 10; frame; end;
    chk(cam.x >= 269 and cam.x <= 271 and cam.y == 200, "un proceso solido detiene a los demas (x=" + cam.x + ")");
    frame; frame;
    chk(cam.visto == sp, "tocar un proceso solido es colision");
    sp.solid = 0;
    frame; frame;
    chk(cam.visto == 0, "sin solido, en el limite del radio no hay colision");
    for (k = 0; k < 3; k++) cam.x += 10; frame; end;
    frame;
    chk(cam.x == 300 and cam.visto == sp, "y se puede entrar en su radio: colision (x=" + cam.x + ")");
    sp.solid = solid_sphere;
    cam.x = 240; cam.y = 195; frame;
    for (k = 0; k < 40; k++) cam.x += 10; frame; end;
    chk(cam.x > 330 and cam.y < 175 and cam.y > 165, "resbala alrededor del solido (" + cam.x + "," + cam.y + ")");
    // Esfera: cuenta la diferencia de altura. Cilindro: el radio, mientras las alturas (z y alto del grafico) se solapen
    cam.x = 240; cam.y = 200; cam.z = 30; frame;
    for (k = 0; k < 6; k++) cam.x += 10; frame; end;
    chk(cam.x == 300, "esfera: a 30 de altura ya no la toca (x=" + cam.x + ")");
    sp.solid = solid_cylinder;
    cam.x = 240; frame;
    for (k = 0; k < 6; k++) cam.x += 10; frame; end;
    chk(cam.x >= 269 and cam.x <= 271, "cilindro: a 30 de altura (el grafico mide 40) lo detiene (x=" + cam.x + ")");
    cam.z = 60; frame;
    for (k = 0; k < 3; k++) cam.x += 10; frame; end;
    chk(cam.x >= 299 and cam.x <= 301, "cilindro: por encima de su alto, pasa (x=" + cam.x + ")");
    signal(sp, s_kill);
    cam.x = 200; cam.y = 200; cam.z = 0; cam.m8_sector = -1;
    frame;

    // Grabar y volver a cargar
    chk(save_wld("mode8_out.wld") == 1, "save_wld");
    chk(load_wld("mode8_out.wld", tex) == 1, "el mapa grabado se vuelve a cargar");
    get_point_m8(8, &a, &b);
    chk(a == 950 and b == 90, "y conserva los cambios");
    stop_mode8(0);
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
function textura(int codigo, int color)
private
    int g;
begin
    g = new_map(16, 16, 16);
    map_clear(0, g, color);
    fpg_add(tex, codigo, 0, g);
    unload_map(0, g);
end
process camara()
begin
    ctype = c_m8;
    radius = 20;
    start_mode8(id, 0, 0);
    go_to_flag(0);
    loop
        visto = collision(type objeto);
        if (manual)
            if (key(_w) or key(_up))   advance(8); end;
            if (key(_s) or key(_down)) advance(-8); end;
            if (key(_a)) xadvance(angle + 90000, 8); end;
            if (key(_d)) xadvance(angle - 90000, 8); end;
            if (key(_left))  angle += 4000; end;
            if (key(_right)) angle -= 4000; end;
            if (key(_q)) z += 4; end;
            if (key(_z)) z -= 4; end;
            if (key(_r)) m8.angle += 4; end;
            if (key(_f)) m8.angle -= 4; end;
            if (key(_esc)) exit("", 0); end;
        end;
        frame;
    end;
end
process objeto(x, y)
begin
    ctype = c_m8;
    graph = new_map(20, 40, 16);
    map_clear(0, graph, cian);
    loop frame; end;
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
