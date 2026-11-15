// Prueba de LOAD_IMAGE (por cabecera), alias LOAD_MAP/LOAD_PNG, SAVE_MAP y SAVE_FPG
program t_image;
global
    int fpg, g, g2, g3, g4, f2, ok = 0, bad = 0;
begin
    set_mode(640, 480, 16);
    g = new_map(32, 16, 16);
    map_clear(0, g, rgb(255, 0, 0));
    map_put_pixel(0, g, 3, 4, rgb(0, 255, 0));
    map_put_pixel(0, g, 31, 15, rgb(0, 0, 255));
    map_set_name(0, g, "prueba");

    save_map(0, g, "t_img.map");
    save_png(0, g, "t_img.png");

    g2 = load_map("t_img.map");        // alias de load_image
    g3 = load_image("t_img.png");      // PNG detectado por cabecera
    g4 = load_image("t_img.map");      // MAP detectado por cabecera

    say("graficos: " + g2 + " " + g3 + " " + g4);
    check(g2, "map");
    check(g3, "png");
    check(g4, "image-map");

    fpg = fpg_new();
    fpg_add(fpg, 1, 0, g);
    fpg_add(fpg, 2, 0, g2);
    say("save_fpg: " + save_fpg(fpg, "t_img.fpg"));
    f2 = load_fpg("t_img.fpg");
    g = map_get_pixel(f2, 1, 3, 4);
    say("fpg pixel(3,4)=" + g + " esperado " + rgb(0, 255, 0));
    if (g == rgb(0, 255, 0) && map_get_pixel(f2, 2, 31, 15) == rgb(0, 0, 255)) ok++; else bad++; end

    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end

function check(int m, string nombre)
begin
    if (m > 0 && map_get_pixel(0, m, 3, 4) == rgb(0, 255, 0) && map_get_pixel(0, m, 31, 15) == rgb(0, 0, 255)
        && map_get_pixel(0, m, 10, 10) == rgb(255, 0, 0))
        ok++;
    else
        bad++;
        say("FALLO en " + nombre + " (codigo " + m + ")");
    end
end
