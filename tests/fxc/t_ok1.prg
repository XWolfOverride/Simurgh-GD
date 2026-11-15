PROGRAM t_ok1;
#define MAXN 10
#define SUMA(a,b) ((a)+(b))
CONST
    pi2 = 6;
GLOBAL
    STRUCT pt[MAXN]
        int x, y;
        string nombre;
    END
    int total;
    float f = 1.5;
    string s = "hola\nmundo";
LOCAL
    int vida = 3;
PRIVATE
    int i;
PROCESS hijo(x, y, graph)
PRIVATE
    int k;
BEGIN
    LOOP
        k = SUMA(x, y) + pi2;
        IF (k > 100) BREAK; END
        FRAME;
    END
END
BEGIN
    FROM i = 0 TO MAXN-1:
        pt[i].x = i; pt[i].y = i * 2; pt[i].nombre = "p";
        total += SUMA(pt[i].x, pt[i].y);
    END
    SWITCH (total)
        CASE 1: f = 2.0; END
        DEFAULT: f = f * 2; END
    END
    hijo(10, 20, 0);
    WHILE (!key(_esc)) FRAME; END
END
