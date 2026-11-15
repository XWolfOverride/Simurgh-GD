// Parametros por defecto en function/process: en cualquier posicion, enteros, cadenas y reales; llamadas antes y despues de la definicion
program t_defaults;

const
    DEF_N = 9;
    DEF_S = "xyz";

global
    int ok = 0, bad = 0;
begin
    // Llamadas ANTES de las definiciones (se resuelven al terminar de compilar)
    chk(f(1) == 11000, "f(1)");
    chk(f(1, 2) == 10200, "f(1,2)");
    chk(f(1,, 3) == 11003, "f(1,,3)");
    chk(f(1, 2, 3) == 10203, "f(1,2,3)");
    chk(g(, 2) == 527, "g(,2): por defecto al principio y al final");
    chk(g(1, 2) == 127, "g(1,2)");
    chk(g(,2,) == 527, "g(,2,)");
    chk(g(1, 2, 3) == 123, "g(1,2,3)");
    chk(h() == "hola2", "h()");
    chk(h("x") == "x2", "h(x)");
    chk(h(, 5) == "hola5", "h(,5)");
    chk(h("a", 1) == "a1", "h(a,1)");
    chk(k() * 2 == 3.0, "k() real por defecto");
    chk(k(4.0) == 4.0, "k(4.0)");
    chk(f(f(1), 1) == 100000 + 100 + 0 + 10000 * 0 + (11000 - 11000) + 1000 * 0 + 0 || 1, "anidado compila");

    chk(m(1) == 109, "valor por defecto que es una constante");
    chk(ms() == "xyz", "constante de cadena por defecto");
    chk(incf(3) == 35, "funcion de un fichero incluido, llamada antes del include");
    chk(incf(3, 1) == 31, "incf(3,1)");

    // Llamadas DESPUES de las definiciones (firma ya conocida)
    chk(f(2) == 21000, "f(2) con firma conocida");
    chk(f(1,, 3) == 11003, "f(1,,3) con firma conocida");
    chk(g(, 2) == 527, "g(,2) con firma conocida");
    chk(h(, 7) == "hola7", "h(,7) con firma conocida");

    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end

function f(int a, int b = 10, int c = 0)
begin
    return a * 10000 + b * 100 + c;
end

function g(int a = 5, int b, int c = 7)
begin
    return a * 100 + b * 10 + c;
end

function string h(string s = "hola", int n = 2)
begin
    return s + n;
end

function float k(float v = 1.5)
begin
    return v;
end

function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end

function m(int a, int n = DEF_N)
begin
    return a * 100 + n;
end

function string ms(string s = DEF_S)
begin
    return s;
end

include "t_defaults_inc.inc";
