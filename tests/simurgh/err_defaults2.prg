// Error esperado: hueco en un parametro sin valor por defecto (llamada despues de la definicion)
program err_defaults2;
function f(int a, int b, int c = 0)
begin
    return a + b + c;
end
begin
    f(1,, 3);
end
