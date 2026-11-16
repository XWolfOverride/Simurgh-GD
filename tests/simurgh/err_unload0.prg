// unload_map con codigo 0 (el fondo de pantalla) debe dar error de ejecucion
program err_unload0;
begin
    set_mode(320, 200, 16);
    unload_map(0, 0);
    say("no deberia llegar aqui");
end
