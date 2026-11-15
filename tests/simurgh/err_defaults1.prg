// Error esperado: a g le falta el parametro obligatorio b (llamada antes de la definicion)
program err_defaults1;
begin
    g(1);
end
function g(int a = 5, int b, int c = 7)
begin
    return a + b + c;
end
