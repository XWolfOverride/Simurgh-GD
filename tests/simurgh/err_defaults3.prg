// Error esperado: valor por defecto de cadena en un parametro entero
program err_defaults3;
function f(int a = "texto")
begin
    return a;
end
begin
    f();
end
