// Carga de MAP y FPG de 24 y 32 bits (se convierten a 16 bits)
program t_truecolor;
global
    int m24, m32, f24, f32, r, ok = 0, bad = 0;
begin
    set_mode(640, 480, 16);
    m24 = load_map("t_tc24.map");        // alias de load_image
    m32 = load_image("t_tc32.map");
    f24 = load_fpg("t_tc24.fpg");
    f32 = load_fpg("t_tc32.fpg");
    say("cargados: " + m24 + " " + m32 + " " + f24 + " " + f32);

    // 24 bits: rojo, verde, azul y negro puro (transparente, = 0)
    chk(map_get_pixel(0, m24, 0, 0) == rgb(255, 0, 0), "m24 rojo");
    chk(map_get_pixel(0, m24, 1, 0) == rgb(0, 255, 0), "m24 verde");
    chk(map_get_pixel(0, m24, 2, 0) == rgb(0, 0, 255), "m24 azul");
    chk(map_get_pixel(0, m24, 3, 0) == 0, "m24 negro = transparente");
    // 32 bits: negro opaco distinto de 0; alfa 0 = 0
    chk(map_get_pixel(0, m32, 0, 0) == rgb(255, 0, 0), "m32 rojo");
    chk(map_get_pixel(0, m32, 2, 0) <> 0, "m32 negro opaco != transparente");
    chk(map_get_pixel(0, m32, 3, 0) == 0, "m32 alfa 0 = transparente");
    // FPG
    chk(map_get_pixel(f24, 2, 2, 0) == rgb(0, 0, 255), "f24 gr.2 azul");
    chk(map_get_pixel(f32, 5, 1, 0) == rgb(0, 255, 0), "f32 gr.5 verde");
    chk(map_get_pixel(f32, 1, 3, 0) == 0, "f32 gr.1 alfa 0");

    // El alfa cargado se conserva al grabar (lo comprueba tools/image-test.sh leyendo los ficheros de salida)
    r = load_image("t_al32.map");
    chk(r > 0, "cargar alfa parcial");
    chk(save_map(0, r, "t_al_out.map"), "grabar m32");
    chk(save_image(0, r, "t_al_out.tga"), "grabar tga con alfa");
    chk(save_image(0, r, "t_al_out.png"), "grabar png con alfa");

    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end

function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
