// Los alias de nombres antiguos (load_map -> load_image) tambien necesitan //$ancient on
program err_ancient3;
global int m;
begin
    m = load_map("x.map");
end
