// Ida y vuelta de 24 y 32 bits: se carga, se graba en cada formato y se vuelve a cargar; el alfa tiene que sobrevivir
program t_roundtrip;
global
    int a, b, f, g, i, ok = 0, bad = 0;
    string ext[3] = "map", "png", "tga";
begin
    set_mode(640, 480, 16);

    // 32 bits con alfa parcial: MAP, PNG y TGA lo conservan (se vuelve a grabar como MAP para que lo compruebe image-test.sh)
    a = load_image("t_al32.map");
    chk(a > 0, "cargar alfa parcial");
    for (i = 0; i <= 2; i++)
        chk(save_image(0, a, "t_rt_a." + ext[i]), "grabar alfa en " + ext[i]);
    end
    for (i = 0; i <= 2; i++)
        b = load_image("t_rt_a." + ext[i]);
        chk(b > 0, "recargar " + ext[i]);
        chk(map_get_pixel(0, b, 0, 0) == map_get_pixel(0, a, 0, 0), "color 0 igual en " + ext[i]);
        chk(map_get_pixel(0, b, 1, 0) == map_get_pixel(0, a, 1, 0), "color 1 igual en " + ext[i]);
        chk(save_map(0, b, "t_rt_b_" + ext[i] + ".map"), "volver a grabar m32 desde " + ext[i]);
    end

    // 24 bits: sin alfa; los colores sobreviven a MAP, PNG, TGA y BMP
    a = load_image("t_tc24.map");
    for (i = 0; i <= 2; i++)
        chk(save_image(0, a, "t_rt_c." + ext[i]), "grabar 24 bits en " + ext[i]);
        b = load_image("t_rt_c." + ext[i]);
        chk(map_get_pixel(0, b, 0, 0) == rgb(255, 0, 0) && map_get_pixel(0, b, 1, 0) == rgb(0, 255, 0)
            && map_get_pixel(0, b, 2, 0) == rgb(0, 0, 255) && map_get_pixel(0, b, 3, 0) == 0, "colores 24 bits en " + ext[i]);
    end
    chk(save_image(0, a, "t_rt_c.bmp"), "grabar 24 bits en bmp");
    b = load_image("t_rt_c.bmp");
    chk(map_get_pixel(0, b, 1, 0) == rgb(0, 255, 0), "colores 24 bits en bmp");

    // FPG de 32 bits: se graba como f32 y se recarga
    f = load_fpg("t_tc32.fpg");
    chk(f >= 0, "cargar fpg 32");
    chk(save_fpg(f, "t_rt32.fpg"), "grabar fpg 32");
    g = load_fpg("t_rt32.fpg");
    chk(g >= 0 && map_get_pixel(g, 5, 1, 0) == rgb(0, 255, 0) && map_get_pixel(g, 1, 3, 0) == 0, "fpg 32 recargado");
    // FPG de 24 bits
    f = load_fpg("t_tc24.fpg");
    chk(save_fpg(f, "t_rt24.fpg"), "grabar fpg 24");
    g = load_fpg("t_rt24.fpg");
    chk(g >= 0 && map_get_pixel(g, 2, 2, 0) == rgb(0, 0, 255), "fpg 24 recargado");

    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end

function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
