// STRUCT en linea: debe comportarse igual que un TYPE con nombre automatico (autotype_<nombre>_struct)
program t_struct;
global
    int antes = 11;
    string sa = "hola";
    struct pt
        int x = 5;
        int y;
        string nom = "pt";
    end
    struct arr[2]
        int v = 7;
        string s = "s";
        float f = 1.5;
    end
    struct ini[1]
        int a;
        int b;
    end = 1, 2, 3, 4;
    struct mat[1][2]
        int c;
    end
    struct fuera
        struct dentro[1]
            string txt = "d";
            int n = 3;
        end
        int z = 9;
    end
    autotype_pt_struct pt2;          // el tipo automatico de "struct pt" se puede usar por su nombre
    int despues = 22;
    int ok = 0, bad = 0;
begin
    chk(antes == 11 && despues == 22 && sa == "hola", "variables vecinas intactas");
    chk(pt.x == 5 && pt.y == 0 && pt.nom == "pt", "struct simple con defaults");
    chk(pt2.x == 5 && pt2.nom == "pt", "variable del tipo automatico autotype_pt_struct");
    pt2.nom = "a" + "b";
    chk(pt2.nom == "ab" && pt.nom == "pt", "cadena independiente en la variable del tipo automatico");
    chk(arr[0].v == 7 && arr[1].v == 7 && arr[1].s == "s" && arr[1].f == 1.5, "array de struct con defaults");
    arr[1].s = "cambiado" + " " + "texto";
    arr[0].s = arr[1].s + "!";
    chk(arr[0].s == "cambiado texto!" && arr[1].s == "cambiado texto", "cadenas en array de struct");
    chk(ini[0].a == 1 && ini[0].b == 2 && ini[1].a == 3 && ini[1].b == 4, "inicializador de array de struct");
    mat[1][1].c = 8;
    chk(mat[1][1].c == 8 && mat[0][0].c == 0, "struct con dos dimensiones");
    chk(fuera.dentro[1].txt == "d" && fuera.dentro[0].n == 3 && fuera.z == 9, "struct anidada");
    fuera.dentro[1].txt = "x" + "y";
    chk(fuera.dentro[1].txt == "xy" && fuera.dentro[0].txt == "d", "cadena en struct anidada");
    p();
    p2();
    frame;
    say("RESULTADO ok=" + ok + " bad=" + bad);
    exit("", 0);
end
process p()
private
    struct loc[1]
        string t = "p";
        int k = 4;
    end
    string a = "A";
begin
    chk(loc[0].t == "p" && loc[1].k == 4 && a == "A", "struct private de un proceso");
    loc[1].t = "q" + "r";
    chk(loc[1].t == "qr" && a == "A", "cadena en struct private");
end
process p2()
private
    struct loc[2]
        int k = 5;
    end
begin
    chk(loc[2].k == 5, "otro proceso con un struct del mismo nombre (nombre automatico distinto)");
end
function chk(int cond, string nombre)
begin
    if (cond) ok++; else bad++; say("FALLO: " + nombre); end
end
