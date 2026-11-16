// Los graficos de 8 bits (MAP y FPG de DIV) se cargan como una copia de 16 bits, convertida una sola vez con la paleta del propio
// fichero; se modifican y se graban ya en 16 bits.
program t_adapt;
global
    int m, f, ok = 0, bad = 0;
begin
    set_mode(320, 200, 8);                       // el modo de 256 colores no existe: se queda en 16 bits
    m = load_image("t_p8.map");
    chk(m > 0, "cargar el map de 8 bits");
    chk(graphic_info(0, m, g_depth) == 16, "el map de 8 bits se carga como copia de 16 bits");
    chk(map_get_pixel(0, m, 0, 0) == rgb(255, 0, 0) && map_get_pixel(0, m, 1, 0) == rgb(0, 255, 0)
        && map_get_pixel(0, m, 2, 0) == rgb(0, 0, 255), "colores con la paleta del fichero");
    chk(map_get_pixel(0, m, 3, 0) == 0, "el indice 0 sigue siendo transparente");
    chk(map_get_pixel(0, m, 0, 1) <> 0, "un color casi negro no se vuelve transparente");
    map_put_pixel(0, m, 0, 0, rgb(10, 20, 30));
    chk(map_get_pixel(0, m, 0, 0) == rgb(10, 20, 30), "se modifica la copia adaptada");
    chk(save_map(0, m, "t_p8_out.map"), "grabar el map");
    f = load_fpg("t_p8.fpg");
    chk(f >= 0, "cargar el fpg de 8 bits");
    chk(graphic_info(f, 3, g_depth) == 16 && graphic_info(f, 7, g_depth) == 16, "los graficos del fpg se cargan de 16 bits");
    chk(map_get_pixel(f, 7, 1, 0) == rgb(0, 255, 0) and map_get_pixel(f, 3, 2, 0) == rgb(0, 0, 255), "colores del fpg con su paleta");
    chk(save_fpg(f, "t_p8_out.fpg"), "grabar el fpg");
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
