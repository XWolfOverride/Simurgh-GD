// SAVE_IMAGE (por extension y con formato explicito) y LOAD_IMAGE de BMP, TGA, PNG y MAP
program t_formats;
global
    int g, r, ok = 0, bad = 0;
begin
    set_mode(640, 480, 16);
    g = new_map(32, 16, 16);
    map_clear(0, g, rgb(255, 0, 0));
    map_put_pixel(0, g, 3, 4, rgb(0, 255, 0));
    map_put_pixel(0, g, 31, 15, rgb(0, 0, 255));
    map_put_pixel(0, g, 0, 0, 0);                  // transparente

    chk(save_image(0, g, "t_f.bmp"), "guardar bmp");
    chk(save_image(0, g, "t_f.tga"), "guardar tga");
    chk(save_image(0, g, "t_f.png"), "guardar png");
    chk(save_image(0, g, "t_f.map"), "guardar map");
    chk(save_image(0, g, "t_f.dat", IMG_BMP), "guardar bmp con formato explicito");
    chk(save_image(0, g, "t_f.xyz", IMG_TGA), "guardar tga con formato explicito");
    chk(save_image(0, g, "t_f.xyz2"), "extension desconocida: se graba como MAP");
    chk(save_image(0, g, "t_f.xyz3", ), "parametro opcional vacio al final");
    chk(save_image(0, g, "t_f.xyz4", IMG_PNG), "formato explicito png");

    comprobar("t_f.bmp", "bmp");
    comprobar("t_f.tga", "tga");
    comprobar("t_f.png", "png");
    comprobar("t_f.map", "map");
    comprobar("t_f.dat", "bmp (.dat, por cabecera)");
    comprobar("t_f.xyz", "tga con extension rara no se reconoce");   // sin .tga y sin firma: debe fallar la carga
    comprobar("t_f.xyz2", "map con extension desconocida");
    comprobar("t_f.xyz3", "map con parametro opcional vacio");
    comprobar("t_f.xyz4", "png con extension desconocida por cabecera");

    // Ficheros generados aparte: BMP de 8 bits con paleta y TGA con RLE
    r = load_image("t_p8.bmp");
    chk(r > 0 && map_get_pixel(0, r, 0, 0) == rgb(255, 0, 0) && map_get_pixel(0, r, 1, 0) == rgb(0, 255, 0)
        && map_get_pixel(0, r, 2, 0) == rgb(0, 0, 255) && map_get_pixel(0, r, 3, 0) == 0, "bmp 8 bits");
    r = load_image("t_rle.tga");
    chk(r > 0 && map_get_pixel(0, r, 0, 0) == rgb(255, 0, 0) && map_get_pixel(0, r, 1, 0) == rgb(255, 0, 0)
        && map_get_pixel(0, r, 2, 0) == rgb(0, 255, 0) && map_get_pixel(0, r, 3, 0) == rgb(0, 0, 255), "tga rle");

    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end

function comprobar(string fich, string nombre)
private int m;
begin
    m = load_image(fich);
    if (nombre == "tga con extension rara no se reconoce")
        chk(m < 0, nombre);
        return;
    end
    chk(m > 0 && map_get_pixel(0, m, 3, 4) == rgb(0, 255, 0) && map_get_pixel(0, m, 31, 15) == rgb(0, 0, 255)
        && map_get_pixel(0, m, 10, 10) == rgb(255, 0, 0) && map_get_pixel(0, m, 0, 0) == 0, nombre);
end

function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
