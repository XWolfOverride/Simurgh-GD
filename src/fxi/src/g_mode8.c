/* Simurgh: nuevo 2026-11-15 - modo 8: mapas de sectores en 2,5D; modificado 2026-11-16 - escala de los mapas de DIV, cielo por ventana (set_sky_m8), niebla por ventana (m8.fog_distance, m8.fog_color), procesos solidos (solid) en los modos 7 y 8, linea de vision (m8_sight) */
/*
 *  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  Copyright (C) 2026 XWolfOverride
 */

/*
 * FILE        : g_mode8.c
 * DESCRIPTION : modo 8 de Simurgh, un motor 2,5D de mapas de sectores (al estilo de Doom o Duke Nukem 3D).
 *
 * El mapa es un plano dividido en sectores (poligonos con una altura de suelo y otra de techo). Las paredes unen dos vertices
 * y separan dos sectores (pared de paso) o uno y nada (pared solida). El formato de fichero es de texto (ver DOC/MODO8.md);
 * tambien se importan los .wld de DIV 2.
 *
 * Dibujo: por cada columna de la ventana se lanza un rayo desde la camara. En el sector en que esta, el rayo sale por la pared
 * mas cercana: se pintan el suelo y el techo del sector hasta esa distancia y, si la pared es solida, la pared (fin de la
 * columna); si es de paso, los escalones (la parte de pared que sobresale por arriba o por abajo), se estrecha la ventana
 * vertical de la columna y se sigue por el sector de al lado. Los sectores no tienen que ser convexos. Los procesos (ctype =
 * c_m8) se pintan despues, del mas lejano al mas cercano, recortados columna a columna con la ventana que habia a su distancia.
 *
 * Coordenadas: x, y del mapa (la y crece hacia abajo, los angulos giran en sentido contrario a las agujas del reloj) y z, la
 * altura. La camara esta en el proceso m8[n].camera, a su altura z mas m8[n].height (los ojos), y m8[n].angle (-128..128) sube
 * o baja la mirada desplazando el horizonte.
 *
 * Movimiento: en cada fotograma se compara la posicion de cada proceso c_m8 con la del fotograma anterior y se resuelve el
 * movimiento contra las paredes del mapa con su radio (radius), su altura de escalon (m8_step) y su altura (m8_height).
 *
 * Procesos solidos (variable local solid; tambien en el modo 7): un proceso que se mueve no puede entrar en el radio de otro
 * que sea solido (esfera o cilindro) y resbala a su alrededor. Aqui estan tambien el radio y la prueba de colision entre
 * procesos de los modos 3D que usa collision() (gr_collision_3d).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "fxi.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define M8_MAX_PORTALS   96          /* sectores que puede atravesar el rayo de una columna */
#define M8_MAX_EVENTS    24          /* cambios de ventana vertical que se recuerdan por columna (para recortar procesos) */
#define M8_FOG_RANGE     4096.0      /* distancia que corresponde al 100 % de set_fog */
#define M8_NEAR          1.0         /* distancia minima a la que se dibuja algo */

typedef struct { double x, y ; } M8VERTEX ;

typedef struct
{
    int   floor, ceil ;              /* alturas */
    int   tex_floor, tex_ceil ;      /* graficos del fichero de texturas (0 = no se pinta: se ve el fondo) */
    int   light ;                    /* 0 (color de ambiente) a 15 (colores originales) */
    int   tag ;
    int   nwalls ;
    int * walls ;                    /* paredes que tocan este sector */
}
M8SECTOR ;

typedef struct
{
    int v1, v2 ;
    int front, back ;                /* sectores a cada lado (back = -1: pared solida) */
    int tex_mid, tex_top, tex_bot ;
    int light ;
    int tag ;
    int twin ;                       /* 1: repite otra pared (mismo tramo, visto desde el otro sector) y no se usa */
}
M8WALL ;

typedef struct { int number ; double x, y ; int angle ; } M8FLAG ;

typedef struct
{
    int        loaded ;
    int        fpg ;                 /* fichero de texturas */
    double     scale ;               /* unidades del mapa por punto de textura */
    int        env_r, env_g, env_b ; /* color de ambiente y de niebla (0-255) */
    int        fog_start, fog_end ;  /* ultimo set_fog: porcentajes de M8_FOG_RANGE (0, 0 = sin niebla). La niebla en si es de cada
                                        ventana (m8[n].fog_distance); de aqui solo se usa el inicio */
    int        nvertices, nsectors, nwalls, nflags ;
    M8VERTEX * vertices ;
    M8SECTOR * sectors ;
    M8WALL   * walls ;
    M8FLAG   * flags ;
}
M8MAP ;

typedef struct
{
    Uint32  z ;
    Uint32  camera_id ;
    Uint32  height ;
    Uint32  angle ;
    Uint32  fog_distance ;           /* niebla: distancia a la que todo es del color de la niebla (0 = sin niebla) */
    Uint32  fog_color ;              /* niebla: color de 16 bits; -1 = fundir con lo que hay detras de la ventana */
    Uint32  reserved[2] ;
}
MODE8_INFO ;

typedef struct
{
    int      active ;
    REGION * region ;
    int      sky_file, sky_graph ;   /* cielo de la ventana (grafico 0 = ninguno): lo fija el programa con set_sky_m8 */
    int      sky_reps ;              /* veces que se repite en una vuelta completa */
}
MODE8 ;

/* Lo que se recuerda de cada proceso c_m8 de un fotograma a otro */
typedef struct
{
    int id ; int sector ; int alive ;
    int x, y ;                       /* coordenadas del proceso (enteras) al acabar el fotograma anterior */
    double ex, ey ;                  /* la misma posicion, en unidades del mapa y sin redondear */
}
M8BODY ;

static M8MAP    map ;
static MODE8    windows[10] ;
static M8BODY * bodies = 0 ;
static int      body_count = 0, body_alloc = 0 ;

/* ---------------------------------------------------------------------- */
/* Mapa                                                                   */
/* ---------------------------------------------------------------------- */

static void m8_free_map (void)
{
    int n ;
    for (n = 0 ; n < map.nsectors ; n++) free (map.sectors[n].walls) ;
    free (map.vertices) ; free (map.sectors) ; free (map.walls) ; free (map.flags) ;
    memset (&map, 0, sizeof(map)) ;
    body_count = 0 ;
}

/* Construye, para cada sector, la lista de paredes que lo tocan. Devuelve 0 si el mapa tiene indices fuera de rango. */
static int m8_link (void)
{
    int n, s ;

    for (n = 0 ; n < map.nsectors ; n++) { free (map.sectors[n].walls) ; map.sectors[n].walls = 0 ; map.sectors[n].nwalls = 0 ; }
    for (n = 0 ; n < map.nwalls ; n++)
    {
        M8WALL * w = &map.walls[n] ;
        if (w->v1 < 0 || w->v1 >= map.nvertices || w->v2 < 0 || w->v2 >= map.nvertices) return 0 ;
        if (w->front < 0 || w->front >= map.nsectors || w->back < -1 || w->back >= map.nsectors) return 0 ;
        /* Los .wld de DIV traen cada pared de paso dos veces, una por sector: se usa solo la primera */
        w->twin = 0 ;
        if (w->back >= 0)
        {
            int m ;
            for (m = 0 ; m < n ; m++)
            {
                M8WALL * o = &map.walls[m] ;
                if (!o->twin && ((o->v1 == w->v1 && o->v2 == w->v2) || (o->v1 == w->v2 && o->v2 == w->v1)) &&
                    ((o->front == w->back && o->back == w->front) || (o->front == w->front && o->back == w->back))) { w->twin = 1 ; break ; }
            }
        }
        if (w->twin) continue ;
        map.sectors[w->front].nwalls++ ;
        if (w->back >= 0 && w->back != w->front) map.sectors[w->back].nwalls++ ;
    }
    for (s = 0 ; s < map.nsectors ; s++)
    {
        map.sectors[s].walls = (int *) malloc (sizeof(int) * (map.sectors[s].nwalls + 1)) ;
        if (!map.sectors[s].walls) return 0 ;
        map.sectors[s].nwalls = 0 ;
    }
    for (n = 0 ; n < map.nwalls ; n++)
    {
        M8WALL * w = &map.walls[n] ;
        if (w->twin) continue ;
        map.sectors[w->front].walls[map.sectors[w->front].nwalls++] = n ;
        if (w->back >= 0 && w->back != w->front) map.sectors[w->back].walls[map.sectors[w->back].nwalls++] = n ;
    }
    return 1 ;
}

/* Carga de un .wld de DIV 2: los vertices y las banderas del primer bloque (coordenadas del editor, que son las que usan los
 * programas) y los sectores y paredes del bloque que usaba su motor, que es el que tiene los dos lados de cada pared. */
static int m8_load_div (const unsigned char * d, long size)
{
    long o, vpe ;
    int n, np, nw, nr, nf, vp, vr, vw ;

#define RD32(p) ((int)((Uint32)(d)[p] | ((Uint32)(d)[(p)+1] << 8) | ((Uint32)(d)[(p)+2] << 16) | ((Uint32)(d)[(p)+3] << 24)))
#define RD16(p) ((int)(short)((d)[p] | ((d)[(p)+1] << 8)))

    if (size < 8 + 4 + 256 + 16 + 4 + 256 + 16 + 4) return 0 ;
    vpe = RD32(8) + 12 ;
    o = 8 + 4 + 256 + 16 + 4 + 256 + 16 ;
    np = RD32(o) ; o += 4 ;
    if (np < 0 || o + 16L * np + 4 > size) return 0 ;
    map.vertices = (M8VERTEX *) calloc (np + 1, sizeof(M8VERTEX)) ;
    for (n = 0 ; n < np ; n++) { map.vertices[n].x = RD32(o + 16*n + 4) ; map.vertices[n].y = RD32(o + 16*n + 8) ; }
    map.nvertices = np ;
    o += 16L * np ;
    nw = RD32(o) ; o += 4 + 40L * nw ;
    if (nw < 0 || o + 4 > size) return 0 ;
    nr = RD32(o) ; o += 4 + 28L * nr ;
    if (nr < 0 || o + 4 > size) return 0 ;
    nf = RD32(o) ; o += 4 ;
    if (nf < 0 || o + 16L * nf > size) return 0 ;
    map.flags = (M8FLAG *) calloc (nf + 1, sizeof(M8FLAG)) ;
    for (n = 0 ; n < nf ; n++)
    {
        map.flags[n].x = RD32(o + 16*n + 4) ; map.flags[n].y = RD32(o + 16*n + 8) ; map.flags[n].number = RD32(o + 16*n + 12) ;
    }
    map.nflags = nf ;

    /* Bloque del motor: "DAT", numeros de puntos, regiones, paredes y banderas, y las tablas */
    if (vpe + 12 > size || memcmp (d + vpe, "DAT", 4) != 0) return 0 ;
    vp = RD16(vpe + 4) ; vr = RD16(vpe + 6) ; vw = RD16(vpe + 8) ;
    o = vpe + 12 ;
    if (vp != np || vr < 0 || vw < 0 || o + 16L*vp + 36L*vr + 44L*vw > size) return 0 ;
    o += 16L * vp ;
    map.sectors = (M8SECTOR *) calloc (vr + 1, sizeof(M8SECTOR)) ;
    for (n = 0 ; n < vr ; n++, o += 36)
    {
        int fade = RD16(o + 30) ;
        map.sectors[n].floor = RD16(o + 4) ;      map.sectors[n].ceil = RD16(o + 6) ;
        map.sectors[n].tex_floor = RD32(o + 12) ; map.sectors[n].tex_ceil = RD32(o + 16) ;
        map.sectors[n].light = fade < 0 ? 15 : fade > 15 ? 0 : 15 - fade ;
    }
    map.nsectors = vr ;
    map.walls = (M8WALL *) calloc (vw + 1, sizeof(M8WALL)) ;
    for (n = 0 ; n < vw ; n++, o += 44)
    {
        int fade = RD16(o + 34) ;
        map.walls[n].v1 = RD16(o + 4) ;     map.walls[n].v2 = RD16(o + 6) ;
        map.walls[n].front = RD16(o + 8) ;  map.walls[n].back = RD16(o + 10) ;
        map.walls[n].tex_top = RD32(o + 12) ; map.walls[n].tex_mid = RD32(o + 16) ; map.walls[n].tex_bot = RD32(o + 20) ;
        map.walls[n].light = fade < 0 ? 15 : fade > 15 ? 0 : 15 - fade ;
    }
    map.nwalls = vw ;
    /* El grafico de cielo del .wld no se importa (el cielo lo elige el programa: set_sky_m8) y la escala es la de DIV: un punto
       de textura ocupa una unidad del mapa */
    return 1 ;
#undef RD32
#undef RD16
}

/* Carga del formato de texto de Simurgh (ver DOC/MODO8.md) */
static int m8_load_text (char * text)
{
    char * line = text, * next ;
    int pass ;

    /* Dos pasadas: contar y rellenar */
    for (pass = 0 ; pass < 2 ; pass++)
    {
        int nv = 0, ns = 0, nw = 0, nf = 0, first = 1 ;

        if (pass == 1)
        {
            map.vertices = (M8VERTEX *) calloc (map.nvertices + 1, sizeof(M8VERTEX)) ;
            map.sectors  = (M8SECTOR *) calloc (map.nsectors + 1, sizeof(M8SECTOR)) ;
            map.walls    = (M8WALL *)   calloc (map.nwalls + 1, sizeof(M8WALL)) ;
            map.flags    = (M8FLAG *)   calloc (map.nflags + 1, sizeof(M8FLAG)) ;
            if (!map.vertices || !map.sectors || !map.walls || !map.flags) return 0 ;
        }
        for (line = text ; *line ; line = next)
        {
            char word[32], saved ;
            int a[10], k ;
            double fx, fy ;

            next = line ;
            while (*next && *next != '\n') next++ ;
            saved = *next ;
            *next = 0 ;
            word[0] = 0 ;
            for (k = 0 ; k < 10 ; k++) a[k] = 0 ;
            if (sscanf (line, " %31s", word) == 1 && word[0] != '#')
            {
                if (first)
                {
                    if (strcmp (word, "simurgh-wld") != 0) { *next = saved ; return 0 ; }
                    first = 0 ;
                }
                else if (strcmp (word, "vertex") == 0)
                {
                    if (pass && sscanf (line, " %*s %lf %lf", &fx, &fy) == 2) { map.vertices[nv].x = fx ; map.vertices[nv].y = fy ; }
                    nv++ ;
                }
                else if (strcmp (word, "sector") == 0)
                {
                    if (pass)
                    {
                        a[4] = 15 ;
                        sscanf (line, " %*s %d %d %d %d %d %d", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]) ;
                        map.sectors[ns].floor = a[0] ; map.sectors[ns].ceil = a[1] ;
                        map.sectors[ns].tex_floor = a[2] ; map.sectors[ns].tex_ceil = a[3] ;
                        map.sectors[ns].light = a[4] ; map.sectors[ns].tag = a[5] ;
                    }
                    ns++ ;
                }
                else if (strcmp (word, "wall") == 0)
                {
                    if (pass)
                    {
                        a[3] = -1 ; a[7] = 15 ;
                        sscanf (line, " %*s %d %d %d %d %d %d %d %d %d", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8]) ;
                        map.walls[nw].v1 = a[0] ; map.walls[nw].v2 = a[1] ; map.walls[nw].front = a[2] ; map.walls[nw].back = a[3] ;
                        map.walls[nw].tex_mid = a[4] ; map.walls[nw].tex_top = a[5] ; map.walls[nw].tex_bot = a[6] ;
                        map.walls[nw].light = a[7] ; map.walls[nw].tag = a[8] ;
                    }
                    nw++ ;
                }
                else if (strcmp (word, "flag") == 0)
                {
                    if (pass && sscanf (line, " %*s %d %lf %lf %d", &a[0], &fx, &fy, &a[1]) >= 3)
                    {
                        map.flags[nf].number = a[0] ; map.flags[nf].x = fx ; map.flags[nf].y = fy ; map.flags[nf].angle = a[1] ;
                    }
                    nf++ ;
                }
                else if (pass && strcmp (word, "scale") == 0)   { if (sscanf (line, " %*s %lf", &fx) == 1 && fx > 0) map.scale = fx ; }
                else if (pass && strcmp (word, "ambient") == 0) sscanf (line, " %*s %d %d %d", &map.env_r, &map.env_g, &map.env_b) ;
                else if (pass && strcmp (word, "fog") == 0)     sscanf (line, " %*s %d %d", &map.fog_start, &map.fog_end) ;
            }
            *next = saved ;
            if (*next) next++ ;
        }
        if (first) return 0 ;
        map.nvertices = nv ; map.nsectors = ns ; map.nwalls = nw ; map.nflags = nf ;
    }
    return 1 ;
}

static void m8_fog_color_all (void) ;

int gr_m8_load (const char * filename, int fpg)
{
    file * fp = file_open (filename, "rb") ;
    char * data ;
    long size = 0, cap = 65536 ;
    int n, ok ;

    if (!fp) { gr_con_printf ("load_wld: no se encuentra %s", filename) ; return 0 ; }
    data = (char *) malloc (cap + 1) ;
    while (data && (n = file_read (fp, data + size, cap - size)) > 0)
    {
        size += n ;
        if (size == cap) { cap *= 2 ; data = (char *) realloc (data, cap + 1) ; }
    }
    file_close (fp) ;
    if (!data) return 0 ;
    data[size] = 0 ;

    m8_free_map () ;
    map.scale = 1.0 ;
    if (size > 8 && memcmp (data, "wld\x1A\x0D\x0A\x01", 7) == 0) ok = m8_load_div ((const unsigned char *)data, size) ;
    else                                                           ok = m8_load_text (data) ;
    free (data) ;

    if (ok) ok = m8_link () ;
    if (!ok)
    {
        gr_con_printf ("load_wld: %s no es un mapa valido", filename) ;
        m8_free_map () ;
        return 0 ;
    }
    map.fpg = fpg ;
    map.loaded = 1 ;
    if (map.fog_end > map.fog_start)                /* un mapa con niebla la pone en todas las ventanas, como set_fog */
    {
        gr_m8_set_fog (map.fog_start, map.fog_end) ;
        m8_fog_color_all () ;
    }
    return 1 ;
}

/* Graba el mapa cargado en el formato de texto de Simurgh */
int gr_m8_save (const char * filename)
{
    file * fp ;
    char line[256] ;
    int n ;

    if (!map.loaded) return 0 ;
    fp = file_open (filename, "wb0") ;
    if (!fp) return 0 ;
#define OUT(...) do { sprintf (line, __VA_ARGS__) ; file_write (fp, line, strlen(line)) ; } while (0)
    OUT ("simurgh-wld 1\n") ;
    OUT ("# Mapa de sectores del modo 8 de Simurgh. Los elementos se numeran desde 0, en el orden en que aparecen.\n") ;
    OUT ("scale %g\n", map.scale) ;
    OUT ("ambient %d %d %d\n", map.env_r, map.env_g, map.env_b) ;
    OUT ("fog %d %d\n", map.fog_start, map.fog_end) ;
    OUT ("# vertex x y\n") ;
    for (n = 0 ; n < map.nvertices ; n++) OUT ("vertex %g %g\n", map.vertices[n].x, map.vertices[n].y) ;
    OUT ("# sector suelo techo textura_suelo textura_techo luz etiqueta\n") ;
    for (n = 0 ; n < map.nsectors ; n++)
        OUT ("sector %d %d %d %d %d %d\n", map.sectors[n].floor, map.sectors[n].ceil, map.sectors[n].tex_floor,
             map.sectors[n].tex_ceil, map.sectors[n].light, map.sectors[n].tag) ;
    OUT ("# wall vertice1 vertice2 sector_delante sector_detras textura textura_arriba textura_abajo luz etiqueta\n") ;
    for (n = 0 ; n < map.nwalls ; n++)
        OUT ("wall %d %d %d %d %d %d %d %d %d\n", map.walls[n].v1, map.walls[n].v2, map.walls[n].front, map.walls[n].back,
             map.walls[n].tex_mid, map.walls[n].tex_top, map.walls[n].tex_bot, map.walls[n].light, map.walls[n].tag) ;
    OUT ("# flag numero x y\n") ;
    for (n = 0 ; n < map.nflags ; n++)
        OUT ("flag %d %g %g\n", map.flags[n].number, map.flags[n].x, map.flags[n].y) ;
#undef OUT
    file_close (fp) ;
    return 1 ;
}

/* ---------------------------------------------------------------------- */
/* Consultas sobre el mapa                                                */
/* ---------------------------------------------------------------------- */

static int m8_point_in_sector (int s, double x, double y)
{
    M8SECTOR * sec = &map.sectors[s] ;
    int n, inside = 0 ;

    for (n = 0 ; n < sec->nwalls ; n++)
    {
        M8WALL * w = &map.walls[sec->walls[n]] ;
        M8VERTEX * a = &map.vertices[w->v1], * b = &map.vertices[w->v2] ;
        if (w->back == w->front) continue ;
        if ((a->y > y) != (b->y > y) && x < (b->x - a->x) * (y - a->y) / (b->y - a->y) + a->x) inside = !inside ;
    }
    return inside ;
}

static int m8_find_sector (double x, double y, int hint)
{
    int s ;
    if (hint >= 0 && hint < map.nsectors && m8_point_in_sector (hint, x, y)) return hint ;
    for (s = 0 ; s < map.nsectors ; s++)
        if (map.sectors[s].nwalls >= 3 && m8_point_in_sector (s, x, y)) return s ;
    return -1 ;
}

/* ---------------------------------------------------------------------- */
/* Movimiento de los procesos dentro del mapa                             */
/* ---------------------------------------------------------------------- */

static M8BODY * m8_body (int id, int create)
{
    int n ;
    for (n = 0 ; n < body_count ; n++) if (bodies[n].id == id) return &bodies[n] ;
    if (!create) return 0 ;
    if (body_count == body_alloc)
    {
        M8BODY * nb = (M8BODY *) realloc (bodies, sizeof(M8BODY) * (body_alloc + 32)) ;
        if (!nb) return 0 ;
        bodies = nb ; body_alloc += 32 ;
    }
    bodies[body_count].id = id ;
    bodies[body_count].sector = -1 ;
    bodies[body_count].alive = 1 ;
    return &bodies[body_count++] ;
}

/* Puede un proceso (a la altura z, con su altura de escalon y su alto) pasar al sector s? */
static int m8_can_enter (int s, int z, int step, int height)
{
    M8SECTOR * sec = &map.sectors[s] ;
    if (sec->ceil <= sec->floor) return 0 ;
    if (sec->floor - z > step) return 0 ;
    if (sec->ceil - (sec->floor > z ? sec->floor : z) < height) return 0 ;
    return 1 ;
}

/* Hay una pared del sector s que el proceso no puede atravesar entre el punto (x, y) y el punto (cx, cy)? Sirve para no hacer caso
 * a una pared de un sector vecino que esta cerca en el plano pero al otro lado de una pared propia. */
static int m8_wall_between (int s, double x, double y, double cx, double cy, int z, int step, int height)
{
    M8SECTOR * sec = &map.sectors[s] ;
    double dx = cx - x, dy = cy - y ;
    int n ;

    for (n = 0 ; n < sec->nwalls ; n++)
    {
        M8WALL * w = &map.walls[sec->walls[n]] ;
        M8VERTEX * a = &map.vertices[w->v1], * b = &map.vertices[w->v2] ;
        int other = (w->front == s) ? w->back : w->front ;
        double wx = b->x - a->x, wy = b->y - a->y, den = dx * wy - dy * wx, t, u ;

        if (w->back == w->front) continue ;
        if (other >= 0 && m8_can_enter (other, z, step, height)) continue ;
        if (den > -1e-12 && den < 1e-12) continue ;
        t = ((a->x - x) * wy - (a->y - y) * wx) / den ;
        u = ((a->x - x) * dy - (a->y - y) * dx) / den ;
        if (t > 0.001 && t < 0.999 && u > 0.001 && u < 0.999) return 1 ;
    }
    return 0 ;
}

#define M8_NEAR_SECTORS 24           /* sectores cercanos que se miran a la vez en una colision */

/* Separa el punto (x, y) de las paredes que el proceso no puede atravesar, hasta dejarlo a la distancia de su radio. Se miran las
 * paredes del sector en que esta y las de los sectores contiguos a los que podria pasar y que el circulo del proceso ya pisa (si
 * solo se mirasen las del suyo, al entrar en un sector estrecho apareceria de golpe pegado a una pared y saldria despedido).
 * En closeby/nclose deja los sectores mirados. */
static void m8_push_out (INSTANCE * p, int sector, double * px, double * py, double radius, int z, int step, int height,
                         int * closeby, int * nclose)
{
    double x = *px, y = *py, r2 = radius * radius ;
    int pass, i, k, n, count = 1 ;

    closeby[0] = sector ;
    for (pass = 0 ; pass < 4 ; pass++)
    {
        int moved = 0 ;
        count = 1 ;
        for (i = 0 ; i < count ; i++)
        {
            M8SECTOR * sec = &map.sectors[closeby[i]] ;
            int zz = sec->floor > z ? sec->floor : z ;          /* altura a la que estaria en ese sector */

            for (n = 0 ; n < sec->nwalls ; n++)
            {
                int wi = sec->walls[n] ;
                M8WALL * w = &map.walls[wi] ;
                M8VERTEX * a = &map.vertices[w->v1], * b = &map.vertices[w->v2] ;
                int other = (w->front == closeby[i]) ? w->back : w->front ;
                double wx = b->x - a->x, wy = b->y - a->y, l2 = wx*wx + wy*wy, t, cx, cy, d2, d ;

                if (w->back == w->front || l2 <= 0.0) continue ;
                t = ((x - a->x) * wx + (y - a->y) * wy) / l2 ;
                if (t < 0.0) t = 0.0 ; else if (t > 1.0) t = 1.0 ;
                cx = a->x + wx * t ; cy = a->y + wy * t ;
                d2 = (x - cx)*(x - cx) + (y - cy)*(y - cy) ;
                if (d2 >= r2) continue ;

                if (other >= 0 && m8_can_enter (other, zz, step, height))
                {
                    /* Pared de paso que puede cruzar: se miran tambien las paredes del otro lado */
                    for (k = 0 ; k < count ; k++) if (closeby[k] == other) break ;
                    if (k == count && count < M8_NEAR_SECTORS) closeby[count++] = other ;
                    continue ;
                }
                d = sqrt (d2) ;
                if (d < 1e-6) continue ;
                if (i > 0 && m8_wall_between (sector, x, y, cx, cy, z, step, height)) continue ;
                x = cx + (x - cx) * radius / d ;
                y = cy + (y - cy) * radius / d ;
                LOCDWORD(p,M8_WALL) = wi ;
                LOCDWORD(p,M8_NEXTSECTOR) = other ;
                moved = 1 ;
            }
        }
        if (!moved) break ;
    }
    *px = x ; *py = y ; *nclose = count ;
}

/* ---------------------------------------------------------------------- */
/* Procesos solidos y colision entre procesos (modos 7 y 8)               */
/* ---------------------------------------------------------------------- */

#define SOLID_SPHERE     1           /* valores de la variable local solid */
#define SOLID_CYLINDER   2
#define SOLID_TOUCH      1.0         /* margen con el que collision() da por tocado un solido (las coordenadas son enteras) */

static int solid_alive (INSTANCE * p)
{
    int status = LOCDWORD(p,STATUS) & ~STATUS_WAITING_MASK ;
    return status == STATUS_RUNNING || status == STATUS_FROZEN ;
}

/* Unidades del mapa que mide un punto del grafico de un proceso (en el modo 8, la escala del mapa) */
static double solid_unit (INSTANCE * p)
{
    return (LOCDWORD(p,CTYPE) == 3 && map.loaded) ? map.scale : 1.0 ;
}

/* Radio de un proceso para las colisiones de los modos 3D: su variable local radius o, si vale 0, la mitad del ancho de su grafico
 * (a su tamano); 0 si no tiene ni lo uno ni lo otro */
double gr_collision_radius (INSTANCE * p)
{
    GRAPH * g ;
    int r = LOCDWORD(p,RADIUS) ;

    if (r > 0) return (double)r ;
    g = instance_graph (p) ;
    if (!g) return 0.0 ;
    return (double)g->width * (double)LOCDWORD(p,GRAPHSIZE) / 200.0 * solid_unit (p) ;
}

/* Alto de un proceso, en unidades de z, para los solidos cilindricos: el de su grafico (a su tamano); sin grafico, m8_height en
 * el modo 8 y nada en el modo 7. En el modo 7 un punto del grafico mide una unidad de z (un cuarto de pixel del mapa). */
static double solid_height (INSTANCE * p)
{
    GRAPH * g = instance_graph (p) ;
    double h ;

    if (g) h = (double)g->height * (double)LOCDWORD(p,GRAPHSIZE) / 100.0 * solid_unit (p) ;
    else   h = LOCDWORD(p,CTYPE) == 3 ? (double)LOCDWORD(p,M8_HEIGHT) : 0.0 ;
    return h < 1.0 ? 1.0 : h ;
}

static void solid_position (INSTANCE * p, double * x, double * y)
{
    int res = LOCDWORD(p,RESOLUTION) ;
    *x = LOCDWORD(p,COORDX) ; *y = LOCDWORD(p,COORDY) ;
    if (res > 0) { *x /= res ; *y /= res ; }
}

/* Distancia en el plano a la que el proceso a, puesto a la altura az, toca al proceso b tomado como solido de la forma shape;
 * 0 si a esa altura no se tocan. Esfera: la suma de los radios, contando la diferencia de altura. Cilindro: la suma de los
 * radios mientras las alturas de los dos (de su z a su z mas su alto) se solapen. */
static double solid_reach (INSTANCE * a, double az, INSTANCE * b, int shape)
{
    double r = gr_collision_radius (a) + gr_collision_radius (b), dz = az - (double)LOCDWORD(b,COORDZ) ;

    if (r <= 0.0) return 0.0 ;
    if (shape == SOLID_CYLINDER) return (dz < solid_height (b) && -dz < solid_height (a)) ? r : 0.0 ;
    if (LOCDWORD(b,CTYPE) == 2) dz /= 4.0 ;         /* en el modo 7 la z va en cuartos de pixel del mapa */
    if (dz * dz >= r * r) return 0.0 ;
    return sqrt (r * r - dz * dz) ;
}

/* Colision entre dos procesos de un modo 3D (collision()). Si ninguno es solido: dos esferas, hay colision si la distancia entre
 * los dos, contando la altura (z), es menor que la suma de sus radios. Si alguno es solido no se puede entrar en el, asi que hay
 * colision desde que se le toca (con un margen de una unidad), con la forma del solido. */
int gr_collision_3d (INSTANCE * proc1, INSTANCE * proc2)
{
    double x1, y1, x2, y2, d2, dz, r, reach ;
    int c1 = LOCDWORD(proc1,CFLAGS), c2 = LOCDWORD(proc2,CFLAGS) ;
    int s1 = LOCDWORD(proc1,SOLID), s2 = LOCDWORD(proc2,SOLID) ;

    if (c1 && c2 && !(c1 & c2)) return 0 ;         /* estan en ventanas distintas */

    r = gr_collision_radius (proc1) + gr_collision_radius (proc2) ;
    if (r <= 0.0) return 0 ;

    solid_position (proc1, &x1, &y1) ;
    solid_position (proc2, &x2, &y2) ;
    d2 = (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) ;

    if (!s1 && !s2)
    {
        dz = (double)(LOCDWORD(proc1,COORDZ) - LOCDWORD(proc2,COORDZ)) ;
        if (LOCDWORD(proc1,CTYPE) == 2) dz /= 4.0 ;
        return d2 + dz*dz < r*r ;
    }
    if (s2)
    {
        reach = solid_reach (proc1, (double)LOCDWORD(proc1,COORDZ), proc2, s2) ;
        if (reach > 0.0 && d2 < (reach + SOLID_TOUCH) * (reach + SOLID_TOUCH)) return 1 ;
    }
    if (s1)
    {
        reach = solid_reach (proc2, (double)LOCDWORD(proc2,COORDZ), proc1, s1) ;
        if (reach > 0.0 && d2 < (reach + SOLID_TOUCH) * (reach + SOLID_TOUCH)) return 1 ;
    }
    return 0 ;
}

/* Saca el punto (x, y), donde estaria el proceso p a la altura z, de los procesos solidos de su mismo modo: lo deja en el borde
 * (a la suma de los radios), en la direccion en que estaba, de modo que al insistir resbala alrededor. Devuelve 1 si lo mueve. */
static int solid_push (INSTANCE * p, double * px, double * py, double z)
{
    INSTANCE * q ;
    int ctype = LOCDWORD(p,CTYPE), cflags = LOCDWORD(p,CFLAGS), moved = 0 ;

    for (q = first_instance ; q ; q = q->next)
    {
        double qx, qy, dx, dy, d2, d, reach ;

        if (q == p || !LOCDWORD(q,SOLID) || LOCDWORD(q,CTYPE) != ctype || !solid_alive (q)) continue ;
        if (cflags && LOCDWORD(q,CFLAGS) && !(cflags & LOCDWORD(q,CFLAGS))) continue ;
        reach = solid_reach (p, z, q, LOCDWORD(q,SOLID)) ;
        if (reach <= 0.0) continue ;
        solid_position (q, &qx, &qy) ;
        dx = *px - qx ; dy = *py - qy ;
        d2 = dx*dx + dy*dy ;
        if (d2 >= reach * reach) continue ;
        d = sqrt (d2) ;
        if (d < 1e-6) { dx = 1.0 ; dy = 0.0 ; d = 1.0 ; }        /* justo encima: hacia cualquier lado */
        *px = qx + dx * reach / d ;
        *py = qy + dy * reach / d ;
        moved = 1 ;
    }
    return moved ;
}

/* Hay algun proceso solido del tipo de coordenadas ctype? */
static int solid_any (int ctype)
{
    INSTANCE * q ;
    for (q = first_instance ; q ; q = q->next)
        if (LOCDWORD(q,SOLID) && LOCDWORD(q,CTYPE) == ctype && solid_alive (q)) return 1 ;
    return 0 ;
}

/* Modo 7: no hay paredes, pero los procesos solidos detienen a los demas igual que en el modo 8. Una vez por fotograma se compara
 * la posicion de cada proceso c_m7 con la del fotograma anterior y el camino se recorre en tramos de medio radio. Como en el
 * modo 8, cada proceso conserva su posicion sin redondear para resbalar sin tirones. Un salto de mas de 32 radios en un fotograma
 * se toma como un cambio de sitio: solo se mira donde cae. Mientras no haya ningun solido en el modo 7 no se hace nada. */
typedef struct { int id, alive, x, y ; double ex, ey ; } M7BODY ;

void gr_mode7_solids (void)
{
    static M7BODY * list = 0 ;
    static int count = 0, alloc = 0, last_frame = -1 ;
    INSTANCE * p ;
    int n ;

    if (last_frame == (int)frame_count) return ;    /* una vez por fotograma, aunque haya varias ventanas */
    last_frame = frame_count ;

    if (!solid_any (2)) { count = 0 ; return ; }

    for (n = 0 ; n < count ; n++) list[n].alive = 0 ;
    for (p = first_instance ; p ; p = p->next)
    {
        M7BODY * b = 0 ;
        int id = LOCDWORD(p,PROCESS_ID), res = LOCDWORD(p,RESOLUTION) ;

        if (LOCDWORD(p,CTYPE) != 2 || !solid_alive (p)) continue ;
        for (n = 0 ; n < count ; n++) if (list[n].id == id) { b = &list[n] ; break ; }
        if (!b)
        {
            if (count == alloc)
            {
                M7BODY * nl = (M7BODY *) realloc (list, sizeof(M7BODY) * (alloc + 32)) ;
                if (!nl) continue ;
                list = nl ; alloc += 32 ;
            }
            b = &list[count++] ;
            b->id = id ;
            solid_position (p, &b->ex, &b->ey) ;
        }
        else if (LOCDWORD(p,COORDX) != b->x || LOCDWORD(p,COORDY) != b->y)
        {
            double mx = LOCDWORD(p,COORDX) - b->x, my = LOCDWORD(p,COORDY) - b->y, x = b->ex, y = b->ey, len, radius ;
            double z = (double)LOCDWORD(p,COORDZ) ;
            int steps, i, pushed = 0 ;

            if (res > 0) { mx /= res ; my /= res ; }
            len = sqrt (mx*mx + my*my) ;
            radius = gr_collision_radius (p) ;
            if (radius < 1.0) radius = 1.0 ;
            steps = (int)(len / (radius * 0.5)) + 1 ;
            if (steps > 64) steps = 1 ;
            for (i = 0 ; i < steps ; i++)
            {
                x += mx / steps ; y += my / steps ;
                if (solid_push (p, &x, &y, z)) pushed = 1 ;
            }
            b->ex = x ; b->ey = y ;
            if (pushed || b->ex != floor (b->ex) || b->ey != floor (b->ey) || res > 1)
            {
                if (res > 0) { x *= res ; y *= res ; }
                LOCDWORD(p,COORDX) = (int)floor (x + 0.5) ;
                LOCDWORD(p,COORDY) = (int)floor (y + 0.5) ;
            }
        }
        b->alive = 1 ;
        b->x = LOCDWORD(p,COORDX) ;
        b->y = LOCDWORD(p,COORDY) ;
    }
    for (n = 0 ; n < count ; )
        if (!list[n].alive) list[n] = list[--count] ; else n++ ;
}

/* Lleva un proceso de (ox, oy) hacia (nx, ny) dentro del mapa, sin atravesar paredes ni procesos solidos */
static void m8_move (INSTANCE * p, M8BODY * body, double ox, double oy, double * nx, double * ny)
{
    double radius = (double)LOCDWORD(p,RADIUS), x = ox, y = oy, vx = *nx - ox, vy = *ny - oy ;
    int step = LOCDWORD(p,M8_STEP), height = LOCDWORD(p,M8_HEIGHT), z = LOCDWORD(p,COORDZ) ;
    int steps, i, n, closeby[M8_NEAR_SECTORS], nclose, solids = solid_any (3) ;
    double len = sqrt (vx*vx + vy*vy) ;

    if (radius < 1.0) radius = 1.0 ;
    steps = (int)(len / (radius * 0.5)) + 1 ;       /* en tramos de medio radio como mucho, para no saltarse una pared */
    if (steps > 64) steps = 64 ;

    for (i = 0 ; i < steps ; i++)
    {
        double px = x, py = y ;
        int s = -1 ;

        x += vx / steps ;
        y += vy / steps ;
        m8_push_out (p, body->sector, &x, &y, radius, z, step, height, closeby, &nclose) ;
        /* Los procesos solidos lo apartan; si al apartarlo se mete en una pared, manda la pared */
        if (solids && solid_push (p, &x, &y, (double)z))
            m8_push_out (p, body->sector, &x, &y, radius, z, step, height, closeby, &nclose) ;

        /* Ha salido del sector? Solo puede ser a uno de los cercanos (a los que puede pasar) */
        if (!m8_point_in_sector (body->sector, x, y))
        {
            for (n = 1 ; n < nclose && s < 0 ; n++)
                if (m8_point_in_sector (closeby[n], x, y)) s = closeby[n] ;
            if (s < 0)
            {
                /* Ni en el suyo ni en uno al que pueda pasar (una esquina entre dos paredes): se queda donde estaba */
                x = px ; y = py ;
            }
            else
            {
                body->sector = s ;
                if (map.sectors[s].floor > z) z = map.sectors[s].floor ;
            }
        }
    }
    LOCDWORD(p,COORDZ) = z ;
    *nx = x ; *ny = y ;
}

/* Pone al dia todos los procesos c_m8: sector, colisiones con las paredes y altura minima (el suelo) */
static void m8_update_bodies (void)
{
    static int last_frame = -1 ;
    INSTANCE * p ;
    int n ;

    if (last_frame == frame_count) return ;         /* una vez por fotograma, aunque haya varias ventanas */
    last_frame = frame_count ;

    for (n = 0 ; n < body_count ; n++) bodies[n].alive = 0 ;

    for (p = first_instance ; p ; p = p->next)
    {
        M8BODY * body ;
        double x, y ;
        int res = LOCDWORD(p,RESOLUTION), status = LOCDWORD(p,STATUS) & ~STATUS_WAITING_MASK ;
        M8SECTOR * sec ;

        if (LOCDWORD(p,CTYPE) != 3 || (status != STATUS_RUNNING && status != STATUS_FROZEN)) continue ;
        body = m8_body (LOCDWORD(p,PROCESS_ID), 1) ;
        if (!body) continue ;
        body->alive = 1 ;

        x = LOCDWORD(p,COORDX) ; y = LOCDWORD(p,COORDY) ;
        if (res > 0) { x /= res ; y /= res ; }

        LOCDWORD(p,M8_WALL) = -1 ;                              /* solo valen durante el fotograma del choque */
        LOCDWORD(p,M8_NEXTSECTOR) = -1 ;
        if (body->sector < 0 || body->sector >= map.nsectors || LOCDWORD(p,M8_SECTOR) == -1)
        {
            body->sector = m8_find_sector (x, y, -1) ;          /* recien llegado, venia de fuera del mapa o ha saltado */
            body->ex = x ; body->ey = y ;
        }
        else if (LOCDWORD(p,COORDX) != body->x || LOCDWORD(p,COORDY) != body->y)
        {
            /* Se ha movido: el camino sale de la posicion sin redondear del fotograma anterior, y lo que se guarda en x, y es el
               resultado redondeado. Asi el redondeo no se acumula ni hace temblar al proceso cuando resbala por una pared. */
            double mx = LOCDWORD(p,COORDX) - body->x, my = LOCDWORD(p,COORDY) - body->y ;
            if (res > 0) { mx /= res ; my /= res ; }
            x = body->ex + mx ; y = body->ey + my ;
            m8_move (p, body, body->ex, body->ey, &x, &y) ;
            body->ex = x ; body->ey = y ;
            if (res > 0) { x *= res ; y *= res ; }
            LOCDWORD(p,COORDX) = (int)floor (x + 0.5) ;
            LOCDWORD(p,COORDY) = (int)floor (y + 0.5) ;
        }
        body->x = LOCDWORD(p,COORDX) ;
        body->y = LOCDWORD(p,COORDY) ;
        LOCDWORD(p,M8_SECTOR) = body->sector ;

        if (body->sector >= 0)
        {
            int height = LOCDWORD(p,M8_HEIGHT) ;
            sec = &map.sectors[body->sector] ;
            if (LOCDWORD(p,COORDZ) < sec->floor) LOCDWORD(p,COORDZ) = sec->floor ;
            if (LOCDWORD(p,COORDZ) + height > sec->ceil && sec->ceil - height >= sec->floor) LOCDWORD(p,COORDZ) = sec->ceil - height ;
        }
    }

    for (n = 0 ; n < body_count ; )
        if (!bodies[n].alive) bodies[n] = bodies[--body_count] ; else n++ ;
}

/* ---------------------------------------------------------------------- */
/* Dibujo                                                                 */
/* ---------------------------------------------------------------------- */

static Uint32 m8_env16 ;                /* color de ambiente en el formato de la pantalla */

/* Niebla de la ventana que se esta dibujando (m8[n].fog_distance y m8[n].fog_color; la misma convencion que el modo 7) */
static double m8_fog_dist ;             /* distancia a la que todo es niebla (0 = sin niebla) */
static double m8_fog_from ;             /* distancia a la que empieza (solo la cambia set_fog; si no, 0) */
static Uint32 m8_fog16 ;                /* color de la niebla */
static int    m8_fog_bg ;               /* fog_color = -1: no hay color, se funde con lo que hay detras de la ventana */

/* Mezcla dos colores de 16 bits; f de 0 (todo c) a 256 (todo to) */
static Uint32 m8_mix (Uint32 c, Uint32 to, int f)
{
    int r  = (c  >> RedShift) & ((1 << RedBits) - 1), g  = (c  >> GreenShift) & ((1 << GreenBits) - 1), b  = c  & ((1 << BlueBits) - 1) ;
    int tr = (to >> RedShift) & ((1 << RedBits) - 1), tg = (to >> GreenShift) & ((1 << GreenBits) - 1), tb = to & ((1 << BlueBits) - 1) ;

    r += ((tr - r) * f) >> 8 ; g += ((tg - g) * f) >> 8 ; b += ((tb - b) * f) >> 8 ;
    return (Uint32)((r << RedShift) | (g << GreenShift) | b) ;
}

/* Proporcion de ambiente (0-256) para una luz de 0 (color de ambiente) a 15 (colores originales) */
static int m8_light_factor (int light)
{
    double keep = (light < 0 ? 0 : light > 15 ? 15 : light) / 15.0 ;
    return 256 - (int)(keep * 256.0) ;
}

/* Proporcion de niebla (0-256) a una distancia */
static int m8_fog_factor (double depth)
{
    if (m8_fog_dist <= 0.0 || depth <= m8_fog_from) return 0 ;
    if (depth >= m8_fog_dist) return 256 ;
    return (int)((depth - m8_fog_from) * 256.0 / (m8_fog_dist - m8_fog_from)) ;
}

/* Pinta un punto del mapa: primero la luz (hacia el color de ambiente) y despues la niebla (hacia su color o, sin color, hacia
 * lo que ya hay pintado: con la niebla completa no se pinta nada) */
static void m8_put (Uint16 * dst, Uint32 c, int light, int fog)
{
    if (light > 0) c = m8_mix (c, m8_env16, light) ;
    if (fog > 0)
    {
        if (m8_fog_bg) { if (fog >= 256) return ; c = m8_mix (c, *dst, fog) ; }
        else           c = m8_mix (c, m8_fog16, fog) ;
    }
    *dst = (Uint16)(c ? c : 1) ;
}

static GRAPH * m8_texture (int code)
{
    GRAPH * g ;
    if (code <= 0) return 0 ;
    g = bitmap_get (map.fpg, code) ;
    return (g && g->depth == 16 && g->width > 0 && g->height > 0) ? g : 0 ;
}

#define M8_TEXEL(g,u,v) (((Uint16 *)((Uint8 *)(g)->data + (size_t)(g)->pitch * (v)))[u])

static int m8_wrap (int v, int size)
{
    v %= size ;
    return v < 0 ? v + size : v ;
}

/* Estado de una columna mientras se dibuja */
typedef struct
{
    GRAPH * dest ;
    int     x ;                 /* columna de pantalla */
    int     ytop, ybot ;        /* ventana vertical abierta (filas de pantalla, ambas inclusive) */
    double  horizon, f, eye ;
    double  cx, cy, dx, dy ;    /* camara y direccion del rayo (por unidad de profundidad) */
}
M8COL ;

/* Trozo vertical de pared: filas y0..y1 a profundidad depth; top_z es la altura del mapa donde empieza la textura */
static void m8_wall_span (M8COL * c, int y0, int y1, double depth, GRAPH * tex, double along, double top_z, int light)
{
    int y, u, shade, fog ;
    Uint16 * dst ;

    if (y0 < c->ytop) y0 = c->ytop ;
    if (y1 > c->ybot) y1 = c->ybot ;
    if (y0 > y1 || !tex) return ;

    shade = m8_light_factor (light) ;
    fog = m8_fog_factor (depth) ;
    if (fog >= 256 && m8_fog_bg) return ;          /* perdida en la niebla: se ve lo que hay detras */
    u = m8_wrap ((int)floor (along / map.scale), tex->width) ;
    dst = (Uint16 *)((Uint8 *)c->dest->data + (size_t)c->dest->pitch * y0) + c->x ;
    for (y = y0 ; y <= y1 ; y++, dst = (Uint16 *)((Uint8 *)dst + c->dest->pitch))
    {
        double z = c->eye + (c->horizon - (y + 0.5)) * depth / c->f ;
        Uint32 px = M8_TEXEL (tex, u, m8_wrap ((int)floor ((top_z - z) / map.scale), tex->height)) ;
        m8_put (dst, px, shade, fog) ;
    }
}

/* Trozo vertical de suelo o de techo (plano horizontal a la altura plane_z): filas y0..y1 */
static void m8_flat_span (M8COL * c, int y0, int y1, GRAPH * tex, double plane_z, int light)
{
    int y, shade = m8_light_factor (light) ;
    Uint16 * dst ;

    if (y0 < c->ytop) y0 = c->ytop ;
    if (y1 > c->ybot) y1 = c->ybot ;
    if (y0 > y1 || !tex) return ;

    dst = (Uint16 *)((Uint8 *)c->dest->data + (size_t)c->dest->pitch * y0) + c->x ;
    for (y = y0 ; y <= y1 ; y++, dst = (Uint16 *)((Uint8 *)dst + c->dest->pitch))
    {
        double dyrow = c->horizon - (y + 0.5), depth ;
        Uint32 px ;
        if (dyrow > -0.01 && dyrow < 0.01) continue ;
        depth = (plane_z - c->eye) * c->f / dyrow ;
        if (depth <= 0.0) continue ;
        px = M8_TEXEL (tex, m8_wrap ((int)floor ((c->cx + c->dx * depth) / map.scale), tex->width),
                            m8_wrap ((int)floor ((c->cy + c->dy * depth) / map.scale), tex->height)) ;
        m8_put (dst, px, shade, m8_fog_factor (depth)) ;
    }
}

/* Cielo de la ventana: una imagen que gira con la camara y no se acerca. Se repite reps veces en una vuelta completa; a lo alto
 * ocupa un alto de ventana desde el horizonte hacia arriba (un techo nunca se ve por debajo del horizonte), asi que sube y baja
 * con la mirada (m8[n].angle): de frente se ve su mitad de abajo y mirando arriba del todo, entera. angle es la direccion de la
 * columna; top, la fila de pantalla donde empieza la imagen. Los puntos de color 0
 * no se pintan (se ve lo que hay detras de la ventana). La niebla no le afecta. */
static void m8_sky_span (M8COL * c, int y0, int y1, GRAPH * sky, double angle, int reps, double top, int height)
{
    int y, u ;
    if (y0 < c->ytop) y0 = c->ytop ;
    if (y1 > c->ybot) y1 = c->ybot ;
    if (y0 > y1 || !sky) return ;
    u = m8_wrap ((int)floor (-angle * (double)sky->width * reps / (2.0 * M_PI)), sky->width) ;
    for (y = y0 ; y <= y1 ; y++)
    {
        int v = (int)floor ((y + 0.5 - top) * sky->height / height) ;
        Uint32 px ;
        if (v < 0) v = 0 ; else if (v >= (int)sky->height) v = sky->height - 1 ;
        px = M8_TEXEL (sky, u, v) ;
        if (px) ((Uint16 *)((Uint8 *)c->dest->data + (size_t)c->dest->pitch * y))[c->x] = (Uint16)px ;
    }
}

typedef struct { INSTANCE * instance ; double depth, lateral ; } M8SPRITE ;

static int m8_compare_sprites (const void * a, const void * b)
{
    const M8SPRITE * s1 = (const M8SPRITE *)a, * s2 = (const M8SPRITE *)b ;
    if (s1->depth > s2->depth) return -1 ;
    if (s1->depth < s2->depth) return 1 ;
    return LOCDWORD(s2->instance,PROCESS_ID) - LOCDWORD(s1->instance,PROCESS_ID) ;
}

void gr_mode8_draw (int n)
{
    MODE8_INFO * dat = &((MODE8_INFO *)&GLODWORD(M8STRUCTS))[n] ;
    GRAPH      * dest = scrbitmap, * sky ;
    INSTANCE   * camera, * p ;
    M8BODY     * cambody ;
    REGION       clip ;
    M8COL        col ;
    double       angle, fx, fy, rx, ry, cam_x, cam_y, eye, f, horizon ;
    int          x0, y0, width, height, x, cam_sector, res ;

    static double * ev_depth = 0 ;          /* por columna: profundidades en las que cambia la ventana vertical */
    static short  * ev_top = 0, * ev_bot = 0 ;
    static int    * ev_count = 0 ;
    static int      ev_width = 0 ;
    static M8SPRITE * sprites = 0 ;
    static int      sprites_alloc = 0 ;
    int             sprite_count = 0, k ;

    if (!map.loaded || !windows[n].active || !dest || dest->depth != 16) return ;
    camera = instance_get (dat->camera_id) ;
    if (!camera) return ;

    m8_update_bodies () ;

    clip = *windows[n].region ;
    if (clip.x < 0) clip.x = 0 ;
    if (clip.y < 0) clip.y = 0 ;
    if (clip.x2 >= (int)dest->width)  clip.x2 = dest->width - 1 ;
    if (clip.y2 >= (int)dest->height) clip.y2 = dest->height - 1 ;
    x0 = clip.x ; y0 = clip.y ;
    width = clip.x2 - clip.x + 1 ; height = clip.y2 - clip.y + 1 ;
    if (width <= 0 || height <= 0) return ;

    if (width > ev_width)
    {
        ev_depth = (double *) realloc (ev_depth, sizeof(double) * width * M8_MAX_EVENTS) ;
        ev_top   = (short *)  realloc (ev_top,   sizeof(short) * width * M8_MAX_EVENTS) ;
        ev_bot   = (short *)  realloc (ev_bot,   sizeof(short) * width * M8_MAX_EVENTS) ;
        ev_count = (int *)    realloc (ev_count, sizeof(int) * width) ;
        ev_width = (ev_depth && ev_top && ev_bot && ev_count) ? width : 0 ;
        if (!ev_width) return ;
    }

    /* Camara */
    angle = (double)LOCDWORD(camera,ANGLE) * M_PI / 180000.0 ;
    fx = cos (angle) ; fy = -sin (angle) ;          /* hacia delante */
    rx = sin (angle) ; ry = cos (angle) ;           /* hacia la derecha */
    cam_x = LOCDWORD(camera,COORDX) ; cam_y = LOCDWORD(camera,COORDY) ;
    res = LOCDWORD(camera,RESOLUTION) ;
    if (res > 0) { cam_x /= res ; cam_y /= res ; }
    eye = (double)LOCDWORD(camera,COORDZ) + (double)(int)dat->height ;
    f = width / 2.0 ;                               /* 90 grados de campo de vision horizontal */
    horizon = height / 2.0 + (double)(int)dat->angle * height / 256.0 ;

    cambody = m8_body (LOCDWORD(camera,PROCESS_ID), 0) ;
    if (cambody && cambody->sector >= 0 && cambody->x == LOCDWORD(camera,COORDX) && cambody->y == LOCDWORD(camera,COORDY))
    {
        cam_x = cambody->ex ; cam_y = cambody->ey ;      /* sin el redondeo de x, y: la camara no da saltitos al resbalar */
    }
    cam_sector = m8_find_sector (cam_x, cam_y, cambody ? cambody->sector : -1) ;
    if (cam_sector < 0) return ;

    m8_env16 = gr_rgb (map.env_r, map.env_g, map.env_b) ;

    /* Niebla de la ventana: como en el modo 7 */
    m8_fog_dist = (double)(int)dat->fog_distance ;
    m8_fog_from = map.fog_start * M8_FOG_RANGE / 100.0 ;
    if (m8_fog_from >= m8_fog_dist) m8_fog_from = 0.0 ;
    m8_fog_bg   = (dat->fog_color == 0xFFFFFFFFu) ;
    m8_fog16    = m8_fog_bg ? 0 : (dat->fog_color & 0xFFFF) ;

    /* Cielo de la ventana (set_sky_m8) */
    sky = 0 ;
    if (windows[n].sky_graph > 0)
    {
        sky = bitmap_get (windows[n].sky_file, windows[n].sky_graph) ;
        if (sky && (sky->depth != 16 || sky->width <= 0 || sky->height <= 0)) sky = 0 ;
    }

    col.dest = dest ; col.horizon = y0 + horizon ; col.f = f ; col.eye = eye ; col.cx = cam_x ; col.cy = cam_y ;

    for (x = 0 ; x < width ; x++)
    {
        double k1 = (x + 0.5 - width / 2.0) / f, depth_in = 0.0 ;
        int sector = cam_sector, from_wall = -1, portals, events = 0 ;
        double * evd = ev_depth + (size_t)x * M8_MAX_EVENTS ;
        short  * evt = ev_top + (size_t)x * M8_MAX_EVENTS, * evb = ev_bot + (size_t)x * M8_MAX_EVENTS ;

        col.x = x0 + x ;
        col.dx = fx + rx * k1 ; col.dy = fy + ry * k1 ;
        col.ytop = y0 ; col.ybot = y0 + height - 1 ;

        evd[0] = 0.0 ; evt[0] = (short)col.ytop ; evb[0] = (short)col.ybot ; events = 1 ;

        for (portals = 0 ; portals < M8_MAX_PORTALS && col.ytop <= col.ybot ; portals++)
        {
            M8SECTOR * sec = &map.sectors[sector] ;
            M8WALL * hit = 0 ;
            int hit_index = -1, i, other, yf, yc ;
            double best = 1e30, best_u = 0.0, wall_len = 0.0 ;

            /* Pared mas cercana por la que sale el rayo */
            for (i = 0 ; i < sec->nwalls ; i++)
            {
                int wi = sec->walls[i] ;
                M8WALL * w = &map.walls[wi] ;
                M8VERTEX * a = &map.vertices[w->v1], * b = &map.vertices[w->v2] ;
                double wx = b->x - a->x, wy = b->y - a->y, den = col.dx * wy - col.dy * wx, t, u ;

                if (wi == from_wall || w->back == w->front) continue ;
                if (den > -1e-12 && den < 1e-12) continue ;
                t = ((a->x - cam_x) * wy - (a->y - cam_y) * wx) / den ;
                if (t <= depth_in + 1e-7 || t >= best) continue ;
                u = ((a->x - cam_x) * col.dy - (a->y - cam_y) * col.dx) / den ;
                if (u < -1e-9 || u > 1.0 + 1e-9) continue ;
                best = t ; best_u = u ; hit = w ; hit_index = wi ;
                wall_len = sqrt (wx*wx + wy*wy) ;
            }
            if (!hit) break ;
            if (best < M8_NEAR) best = M8_NEAR ;

            /* Suelo y techo del sector hasta la pared */
            yf = (int)floor (col.horizon - (sec->floor - eye) * f / best) ;      /* primera fila del suelo */
            yc = (int)floor (col.horizon - (sec->ceil  - eye) * f / best) - 1 ;  /* ultima fila del techo */
            if (eye > sec->floor)
            {
                GRAPH * t = m8_texture (sec->tex_floor) ;
                if (t) m8_flat_span (&col, yf, col.ybot, t, sec->floor, sec->light) ;
                if (yf - 1 < col.ybot) col.ybot = yf - 1 ;
            }
            if (eye < sec->ceil)
            {
                GRAPH * t = m8_texture (sec->tex_ceil) ;
                if (t) m8_flat_span (&col, col.ytop, yc, t, sec->ceil, sec->light) ;
                else   m8_sky_span (&col, col.ytop, yc, sky, angle - atan (k1), windows[n].sky_reps, col.horizon - height, height) ;
                if (yc + 1 > col.ytop) col.ytop = yc + 1 ;
            }
            if (col.ytop > col.ybot) { depth_in = best ; break ; }

            other = (hit->front == sector) ? hit->back : hit->front ;
            if (other < 0 || map.sectors[other].ceil <= map.sectors[other].floor)
            {
                /* Pared solida (o sector cerrado): fin de la columna */
                m8_wall_span (&col, col.ytop, col.ybot, best, m8_texture (hit->tex_mid ? hit->tex_mid : hit->tex_bot ? hit->tex_bot : hit->tex_top), best_u * wall_len, sec->ceil, hit->light) ;
                depth_in = best ;
                col.ytop = col.ybot + 1 ;
                break ;
            }
            else
            {
                M8SECTOR * nxt = &map.sectors[other] ;
                if (nxt->floor > sec->floor)             /* escalon de abajo */
                {
                    int ys = (int)floor (col.horizon - (nxt->floor - eye) * f / best) ;
                    m8_wall_span (&col, ys, col.ybot, best, m8_texture (hit->tex_bot ? hit->tex_bot : hit->tex_mid ? hit->tex_mid : hit->tex_top), best_u * wall_len, nxt->floor, hit->light) ;
                    if (ys - 1 < col.ybot) col.ybot = ys - 1 ;
                }
                if (nxt->ceil < sec->ceil)               /* escalon de arriba */
                {
                    int ys = (int)floor (col.horizon - (nxt->ceil - eye) * f / best) - 1 ;
                    m8_wall_span (&col, col.ytop, ys, best, m8_texture (hit->tex_top ? hit->tex_top : hit->tex_mid ? hit->tex_mid : hit->tex_bot), best_u * wall_len, sec->ceil, hit->light) ;
                    if (ys + 1 > col.ytop) col.ytop = ys + 1 ;
                }
                if (events < M8_MAX_EVENTS)
                {
                    evd[events] = best ; evt[events] = (short)col.ytop ; evb[events] = (short)col.ybot ; events++ ;
                }
                sector = other ;
                from_wall = hit_index ;
                depth_in = best ;
            }
        }
        /* La columna se cierra a la profundidad depth_in */
        if (events == M8_MAX_EVENTS) events-- ;
        evd[events] = depth_in ; evt[events] = 1 ; evb[events] = 0 ; events++ ;
        ev_count[x] = events ;
    }

    /* Procesos: del mas lejano al mas cercano */
    for (p = first_instance ; p ; p = p->next)
    {
        int status = LOCDWORD(p,STATUS) & ~STATUS_WAITING_MASK ;
        double px, py, depth ;

        if (p == camera || LOCDWORD(p,CTYPE) != 3 || (status != STATUS_RUNNING && status != STATUS_FROZEN)) continue ;
        if (LOCDWORD(p,CFLAGS) && !(LOCDWORD(p,CFLAGS) & (1 << n))) continue ;
        px = LOCDWORD(p,COORDX) ; py = LOCDWORD(p,COORDY) ;
        res = LOCDWORD(p,RESOLUTION) ;
        if (res > 0) { px /= res ; py /= res ; }
        depth = (px - cam_x) * fx + (py - cam_y) * fy ;
        if (depth < M8_NEAR * 4) continue ;
        if (sprite_count == sprites_alloc)
        {
            M8SPRITE * ns = (M8SPRITE *) realloc (sprites, sizeof(M8SPRITE) * (sprites_alloc + 32)) ;
            if (!ns) break ;
            sprites = ns ; sprites_alloc += 32 ;
        }
        sprites[sprite_count].instance = p ;
        sprites[sprite_count].depth = depth ;
        sprites[sprite_count].lateral = (px - cam_x) * rx + (py - cam_y) * ry ;
        sprite_count++ ;
    }
    qsort (sprites, sprite_count, sizeof(M8SPRITE), m8_compare_sprites) ;

    for (k = 0 ; k < sprite_count ; k++)
    {
        INSTANCE * sp = sprites[k].instance ;
        double depth = sprites[k].depth, scale, left, top, sw, sh ;
        int saved_angle = LOCDWORD(sp,ANGLE), flags, sx, sx0, sx1, shade, fog, sector, cpx, cpy ;
        GRAPH * g ;

        /* Con xgraph, la vista depende de la direccion del proceso respecto a la camara */
        if (LOCDWORD(sp,XGRAPH))
        {
            int rel = saved_angle - LOCDWORD(camera,ANGLE) + 90000, views = *(int *)LOCDWORD(sp,XGRAPH) ;
            if (views > 0) rel += 180000 / views ;
            rel %= 360000 ; if (rel < 0) rel += 360000 ;
            LOCDWORD(sp,ANGLE) = rel ;
        }
        g = instance_graph (sp) ;
        LOCDWORD(sp,ANGLE) = saved_angle ;
        if (!g || g->depth != 16) continue ;
        flags = LOCDWORD(sp,FLAGS) ^ LOCDWORD(sp,XGRAPH_FLAGS) ;

        /* Un punto del grafico mide lo mismo que un punto de textura (map.scale unidades); se ancla por el punto de control 0 o por el centro de su base */
        scale = map.scale * LOCDWORD(sp,GRAPHSIZE) / 100.0 * f / depth ;
        if (scale <= 0.0) continue ;
        if (g->ncpoints > 0 && g->cpoints[0].x != CPOINT_UNDEFINED) { cpx = g->cpoints[0].x ; cpy = g->cpoints[0].y ; }
        else                                                          { cpx = g->width / 2 ; cpy = g->height - 1 ; }
        sw = g->width * scale ; sh = g->height * scale ;
        left = x0 + width / 2.0 + sprites[k].lateral * f / depth - cpx * scale ;
        top  = col.horizon - ((double)LOCDWORD(sp,COORDZ) - eye) * f / depth - (cpy + 1) * scale ;
        sx0 = (int)floor (left) ; sx1 = (int)floor (left + sw) ;
        if (sx0 < x0) sx0 = x0 ;
        if (sx1 > x0 + width - 1) sx1 = x0 + width - 1 ;

        sector = LOCDWORD(sp,M8_SECTOR) ;
        shade = m8_light_factor (sector >= 0 && sector < map.nsectors ? map.sectors[sector].light : 15) ;
        fog = m8_fog_factor (depth) ;
        if (fog >= 256) continue ;                  /* mas alla de la niebla no se ve */

        for (sx = sx0 ; sx <= sx1 ; sx++)
        {
            int cx = sx - x0, e, ytop, ybot, y, ya, yb, tu ;
            double * evd = ev_depth + (size_t)cx * M8_MAX_EVENTS ;

            /* Ventana vertical que habia a la profundidad del proceso */
            for (e = ev_count[cx] - 1 ; e > 0 && evd[e] > depth ; e--) ;
            ytop = ev_top[(size_t)cx * M8_MAX_EVENTS + e] ; ybot = ev_bot[(size_t)cx * M8_MAX_EVENTS + e] ;
            if (ytop > ybot) continue ;

            tu = (int)((sx + 0.5 - left) / scale) ;
            if (tu < 0 || tu >= (int)g->width) continue ;
            if (flags & B_HMIRROR) tu = g->width - 1 - tu ;
            ya = (int)floor (top) ; yb = (int)floor (top + sh) ;
            if (ya < ytop) ya = ytop ;
            if (yb > ybot) yb = ybot ;
            for (y = ya ; y <= yb ; y++)
            {
                int tv = (int)((y + 0.5 - top) / scale) ;
                Uint32 px ;
                Uint16 * dst ;
                if (tv < 0 || tv >= (int)g->height) continue ;
                px = M8_TEXEL (g, tu, tv) ;
                if (!px) continue ;
                dst = (Uint16 *)((Uint8 *)dest->data + (size_t)dest->pitch * y) + sx ;
                /* La luz de su sector y la niebla: se tine hacia el color de la niebla o, sin color, se desvanece */
                if (shade > 0) px = m8_mix (px, m8_env16, shade) ;
                if (fog > 0)   px = m8_mix (px, m8_fog_bg ? *dst : m8_fog16, fog) ;
                if (!px) px = 1 ;
                *dst = (flags & B_TRANSLUCENT) ? (Uint16)(((px & 0xF7DE) >> 1) + ((*dst & 0xF7DE) >> 1)) : (Uint16)px ;
            }
        }
    }
}

/* ---------------------------------------------------------------------- */
/* Funciones para el lenguaje                                             */
/* ---------------------------------------------------------------------- */

void gr_mode8_bbox (int n, REGION * r)
{
    if (n >= 0 && n <= 9 && windows[n].region) *r = *windows[n].region ;
}

int gr_mode8_active (int n)
{
    return (n >= 0 && n <= 9) ? windows[n].active : 0 ;
}

void gr_mode8_stop (int n)
{
    if (n < 0 || n > 9) return ;
    object_list_dirty = 1 ;
    windows[n].active = 0 ;
}

void gr_mode8_start (int camera, int n, int region)
{
    MODE8_INFO * dat ;
    if (n < 0 || n > 9) return ;
    dat = &((MODE8_INFO *)&GLODWORD(M8STRUCTS))[n] ;
    dat->camera_id = camera ;
    object_list_dirty = 1 ;
    windows[n].active = 1 ;
    windows[n].region = region_get (region) ;
}

/* Coloca un proceso en una bandera del mapa (a la altura del suelo de su sector) */
int gr_m8_go_to_flag (INSTANCE * p, int number)
{
    int n, s, res = LOCDWORD(p,RESOLUTION) ;
    M8BODY * body ;

    if (!map.loaded) return 0 ;
    for (n = 0 ; n < map.nflags ; n++) if (map.flags[n].number == number) break ;
    if (n == map.nflags) return 0 ;
    LOCDWORD(p,COORDX) = (int)floor (map.flags[n].x * (res > 0 ? res : 1) + 0.5) ;
    LOCDWORD(p,COORDY) = (int)floor (map.flags[n].y * (res > 0 ? res : 1) + 0.5) ;
    s = m8_find_sector (map.flags[n].x, map.flags[n].y, -1) ;
    if (s >= 0) LOCDWORD(p,COORDZ) = map.sectors[s].floor ;
    LOCDWORD(p,M8_SECTOR) = s ;
    body = m8_body (LOCDWORD(p,PROCESS_ID), 1) ;       /* ha saltado: no se calcula el camino desde donde estaba */
    if (body) { body->sector = s ; body->x = LOCDWORD(p,COORDX) ; body->y = LOCDWORD(p,COORDY) ; body->ex = map.flags[n].x ; body->ey = map.flags[n].y ; }
    return 1 ;
}

int gr_m8_set_sector_height (int s, int floor_h, int ceil_h)
{
    if (!map.loaded || s < 0 || s >= map.nsectors) return 0 ;
    if (floor_h != -1) map.sectors[s].floor = floor_h ;
    if (ceil_h  != -1) map.sectors[s].ceil  = ceil_h ;
    if (map.sectors[s].ceil < map.sectors[s].floor) map.sectors[s].ceil = map.sectors[s].floor ;
    return 1 ;
}

int gr_m8_get_sector_height (int s, int * floor_h, int * ceil_h)
{
    if (!map.loaded || s < 0 || s >= map.nsectors) return 0 ;
    if (floor_h) *floor_h = map.sectors[s].floor ;
    if (ceil_h)  *ceil_h  = map.sectors[s].ceil ;
    return 1 ;
}

int gr_m8_set_point (int v, int x, int y)
{
    if (!map.loaded || v < 0 || v >= map.nvertices) return 0 ;
    map.vertices[v].x = x ; map.vertices[v].y = y ;
    return 1 ;
}

int gr_m8_get_point (int v, int * x, int * y)
{
    if (!map.loaded || v < 0 || v >= map.nvertices) return 0 ;
    if (x) *x = (int)map.vertices[v].x ;
    if (y) *y = (int)map.vertices[v].y ;
    return 1 ;
}

int gr_m8_set_sector_texture (int s, int tex_floor, int tex_ceil, int light)
{
    if (!map.loaded || s < 0 || s >= map.nsectors) return 0 ;
    if (tex_floor != -1) map.sectors[s].tex_floor = tex_floor ;
    if (tex_ceil  != -1) map.sectors[s].tex_ceil  = tex_ceil ;
    if (light     != -1) map.sectors[s].light     = light < 0 ? 0 : light > 15 ? 15 : light ;
    return 1 ;
}

int gr_m8_get_sector_texture (int s, int * tex_floor, int * tex_ceil, int * light)
{
    if (!map.loaded || s < 0 || s >= map.nsectors) return 0 ;
    if (tex_floor) *tex_floor = map.sectors[s].tex_floor ;
    if (tex_ceil)  *tex_ceil  = map.sectors[s].tex_ceil ;
    if (light)     *light     = map.sectors[s].light ;
    return 1 ;
}

/* Cambia la textura de una pared (la del centro y las de los escalones) */
int gr_m8_set_wall_texture (int w, int tex, int light)
{
    if (!map.loaded || w < 0 || w >= map.nwalls) return 0 ;
    if (tex != -1) map.walls[w].tex_mid = map.walls[w].tex_top = map.walls[w].tex_bot = tex ;
    if (light != -1) map.walls[w].light = light < 0 ? 0 : light > 15 ? 15 : light ;
    return 1 ;
}

int gr_m8_get_wall_texture (int w, int * tex, int * light)
{
    if (!map.loaded || w < 0 || w >= map.nwalls) return 0 ;
    if (tex)   *tex   = map.walls[w].tex_mid ? map.walls[w].tex_mid : map.walls[w].tex_top ? map.walls[w].tex_top : map.walls[w].tex_bot ;
    if (light) *light = map.walls[w].light ;
    return 1 ;
}

/* Pone en las diez ventanas el color de la niebla igual al color de ambiente */
static void m8_fog_color_all (void)
{
    MODE8_INFO * dat = (MODE8_INFO *)&GLODWORD(M8STRUCTS) ;
    Uint32 c = (Uint32)gr_rgb (map.env_r, map.env_g, map.env_b) & 0xFFFF ;
    int n ;
    for (n = 0 ; n < 10 ; n++) dat[n].fog_color = c ;
}

/* Niebla al estilo de DIV: porcentajes de M8_FOG_RANGE en que empieza y en que lo tapa todo. Es una forma de escribir la niebla de
 * las ventanas: deja en las diez m8[n].fog_distance = fin y m8[n].fog_color = el color de ambiente, y guarda el inicio (lo unico
 * que no se puede poner con la estructura m8). set_fog (0, 0) la quita. */
int gr_m8_set_fog (int start, int end)
{
    MODE8_INFO * dat = (MODE8_INFO *)&GLODWORD(M8STRUCTS) ;
    int n ;

    map.fog_start = start < 0 ? 0 : start ;
    map.fog_end   = end < 0 ? 0 : end ;
    if (map.fog_end <= map.fog_start) map.fog_start = map.fog_end = 0 ;
    for (n = 0 ; n < 10 ; n++) dat[n].fog_distance = (Uint32)(int)(map.fog_end * M8_FOG_RANGE / 100.0) ;
    m8_fog_color_all () ;
    return 1 ;
}

/* Color de ambiente (el de la luz 0), en porcentajes de 0 a 100. Como en DIV es tambien el de la niebla: se copia a
 * m8[n].fog_color de las diez ventanas. */
int gr_m8_set_env_color (int r, int g, int b)
{
    map.env_r = (r < 0 ? 0 : r > 100 ? 100 : r) * 255 / 100 ;
    map.env_g = (g < 0 ? 0 : g > 100 ? 100 : g) * 255 / 100 ;
    map.env_b = (b < 0 ? 0 : b > 100 ? 100 : b) * 255 / 100 ;
    m8_fog_color_all () ;
    return 1 ;
}

/* Cielo de una ventana: el grafico que se ve por los techos sin textura y las veces que se repite en una vuelta completa.
 * Con el grafico 0 no hay cielo (se ve lo que haya dibujado por debajo de la ventana). */
int gr_m8_set_sky (int file, int graph, int reps, int n)
{
    if (n < 0 || n > 9) return 0 ;
    if (graph > 0 && !bitmap_get (file, graph)) return 0 ;
    windows[n].sky_file  = file ;
    windows[n].sky_graph = graph > 0 ? graph : 0 ;
    windows[n].sky_reps  = reps > 0 ? reps : 1 ;
    return 1 ;
}

/* Hay camino libre para la vista entre el punto (x0, y0, z0), que esta en el sector dado, y el punto (x1, y1, z1)? Recorre los
 * sectores que cruza el segmento: lo tapan una pared solida, un sector infranqueable y un escalon o un dintel si el rayo pasa
 * por debajo del suelo mas alto o por encima del techo mas bajo de los dos sectores de la pared. */
static int m8_ray_clear (double x0, double y0, double z0, int sector, double x1, double y1, double z1)
{
    double dx = x1 - x0, dy = y1 - y0, t_in = 0.0 ;
    int from_wall = -1, n ;

    for (n = 0 ; n < 4 * M8_MAX_PORTALS ; n++)
    {
        M8SECTOR * sec = &map.sectors[sector], * nxt ;
        double best = 2.0, zr, lo, hi ;
        int hit = -1, i, other ;

        for (i = 0 ; i < sec->nwalls ; i++)
        {
            int wi = sec->walls[i] ;
            M8WALL * w = &map.walls[wi] ;
            M8VERTEX * a = &map.vertices[w->v1], * b = &map.vertices[w->v2] ;
            double wx = b->x - a->x, wy = b->y - a->y, den = dx * wy - dy * wx, t, u ;

            if (wi == from_wall || w->back == w->front) continue ;
            if (den > -1e-12 && den < 1e-12) continue ;
            t = ((a->x - x0) * wy - (a->y - y0) * wx) / den ;
            if (t <= t_in + 1e-9 || t >= best) continue ;
            u = ((a->x - x0) * dy - (a->y - y0) * dx) / den ;
            if (u < -1e-9 || u > 1.0 + 1e-9) continue ;
            best = t ; hit = wi ;
        }
        if (hit < 0 || best >= 1.0) return 1 ;          /* el segmento acaba dentro de este sector */

        other = (map.walls[hit].front == sector) ? map.walls[hit].back : map.walls[hit].front ;
        if (other < 0) return 0 ;                       /* pared solida */
        nxt = &map.sectors[other] ;
        if (nxt->ceil <= nxt->floor) return 0 ;         /* sector infranqueable */
        zr = z0 + (z1 - z0) * best ;
        lo = sec->floor > nxt->floor ? sec->floor : nxt->floor ;
        hi = sec->ceil  < nxt->ceil  ? sec->ceil  : nxt->ceil ;
        if (zr < lo || zr > hi) return 0 ;              /* escalon o dintel */
        sector = other ; from_wall = hit ; t_in = best ;
    }
    return 0 ;
}

/* Linea de vision entre dos procesos del modo 8: 1 si desde los ojos de from (su z mas su m8_height: su parte de arriba) se ve
 * la parte de arriba, el centro o la base de to sin que lo tape el mapa; 0 si no, o si alguno no esta en el mapa. No cuentan los
 * demas procesos ni la niebla. */
int gr_m8_sight (INSTANCE * from, INSTANCE * to)
{
    double x0, y0, x1, y1, z0, z1, h ;
    int s0, s1 ;

    if (!map.loaded || !from || !to || LOCDWORD(from,CTYPE) != 3 || LOCDWORD(to,CTYPE) != 3) return 0 ;
    if (from == to) return 1 ;
    solid_position (from, &x0, &y0) ;
    solid_position (to, &x1, &y1) ;
    s0 = m8_find_sector (x0, y0, LOCDWORD(from,M8_SECTOR)) ;
    s1 = m8_find_sector (x1, y1, LOCDWORD(to,M8_SECTOR)) ;
    if (s0 < 0 || s1 < 0) return 0 ;
    z0 = (double)LOCDWORD(from,COORDZ) + (double)LOCDWORD(from,M8_HEIGHT) ;
    z1 = (double)LOCDWORD(to,COORDZ) ;
    h  = (double)LOCDWORD(to,M8_HEIGHT) ;
    return m8_ray_clear (x0, y0, z0, s0, x1, y1, z1 + h) || m8_ray_clear (x0, y0, z0, s0, x1, y1, z1 + h / 2.0) ||
           m8_ray_clear (x0, y0, z0, s0, x1, y1, z1) ;
}
