/* Simurgh: modificado 2026-11-15 - modo 7 reescrito con la proyeccion del modo 7 de DIV, para 16 bits, la altura de un proceso es su z; 2026-11-16 - los procesos solidos (solid) detienen a los demas */
/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.84
 *  Last stable release   :
 *  Project documentation : http://fenix.divsite.net
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  Copyright © 1999 José Luis Cebrián Pagüe
 *  Copyright © 2002 Fenix Team
 *
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <math.h>

#include "fxi.h"
#include "fmath.h"



typedef struct _mode7
{
    int     active ;

    REGION *    region ;
    GRAPH * indoor ;
    GRAPH * outdoor ;
}
MODE7 ;

typedef struct _mode7_info
{
    Uint32  camera_id ;
    Uint32  height ;
    Uint32  distance ;
    Uint32  horizon ;
    Uint32  focus ;
    Uint32  z ;
    Uint32  color ;
    Uint32  flags ;
    Uint32  fog_distance ;      /* niebla: distancia a la que todo es del color de la niebla (0 = sin niebla) */
    Uint32  fog_color ;         /* niebla: color de 16 bits */
}
__PACKED
MODE7_INFO ;

MODE7 current_mode7[10] ;

void gr_mode7_bbox (int n, REGION * r)
{
    if (n >= 0 && n <= 9)
        *r = *current_mode7[n].region;
}

int gr_mode7_active (int n)
{
    return (n >= 0 && n <= 9) ? current_mode7[n].active : 0 ;
}

void gr_mode7_stop (int n)
{
    object_list_dirty = 1;
    current_mode7[n].active = 0 ;
}

void gr_mode7_start (int n, int fileid, int inid, int outid, int region, int horizon)
{
    MODE7_INFO * dat ;
    if (n < 0 || n > 9) return ;

    dat = &((MODE7_INFO *)&GLODWORD(M7STRUCTS))[n];
    dat->height     = 32;
    dat->distance   = 64;
    dat->horizon    = horizon;
    dat->focus      = 256;
    dat->z          = 256;
    dat->color      = 0;
    dat->flags      = 0;
    dat->fog_distance = 0;
    dat->fog_color  = 0;

    object_list_dirty = 1;

    current_mode7[n].active  = 1 ;
    /* El grafico 0 significa "ninguno" (en el fichero 0 bitmap_get(0, 0) seria el fondo de pantalla) */
    current_mode7[n].indoor  = inid  ? bitmap_get (fileid, inid)  : NULL ;
    current_mode7[n].outdoor = outid ? bitmap_get (fileid, outid) : NULL ;
    current_mode7[n].region  = region_get (region) ;

}

/*
 * Modo 7 de Simurgh.
 *
 * Es una proyeccion en perspectiva (camara de agujero) como la del modo 7 de DIV: la camara esta a "height" unidades sobre el
 * suelo (4 unidades = 1 pixel del mapa), "distance" pixeles detras del proceso camara, mirando en su direccion (angle), con
 * una distancia focal en pixeles f = ancho_de_region * focus / 320, y el horizonte en la fila "horizon" de la region.
 *
 *   - Fila y de la region (debajo del horizonte): dy = y + 1 - horizon; avance por pixel de pantalla paso = (height/4) / dy;
 *     distancia hacia delante Z = paso * f.
 *   - Punto del mapa visto en la columna x:  u = Cx + Z*cos(a) + (x - ancho/2)*paso*sin(a)
 *                                            v = Cy - Z*sin(a) + (x - ancho/2)*paso*cos(a)
 *     (los angulos giran en sentido contrario a las agujas del reloj, la y de pantalla crece hacia abajo).
 *   - Un proceso (ctype = c_m7) a (dx, dy) de la camara se coloca asi: Z = distancia hacia delante, L = distancia lateral;
 *     columna = ancho/2 + f*L/Z; fila = horizon + (altura_camara - z_del_proceso)/4 * f/Z - 1 (la base del grafico); y se
 *     dibuja a tamano proceso.size * f / (4*Z) por ciento. El punto de anclaje es el punto de control 0 o, si no lo tiene,
 *     el centro de la base del grafico.
 *   - Niebla: si fog_distance > 0, lo que esta a distancia Z se mezcla con el color fog_color en la proporcion Z / fog_distance
 *     (a fog_distance todo es del color de la niebla; con fog_color negro se pierde en la oscuridad). Los pixeles de los
 *     procesos se mezclan con el mismo color y la misma proporcion, y mas alla de fog_distance no se pintan. Con
 *     fog_color = -1 no hay color de niebla: el suelo se funde con lo que hay detras (el fondo o la capa de abajo) y los
 *     procesos se desvanecen (alfa) con la distancia.
 *   - Varios modos 7 apilados: se pintan por m7[n].z y un pixel de color 0 no pinta, asi que el de arriba deja ver el de abajo.
 *
 * Todo es de 16 bits: los mapas indoor y outdoor y la pantalla.
 */

/* Mezcla al 50 % de dos pixeles de 16 bits (RGB565) */
#define M7_AVG16(a,b) ((Uint16)((((a) & 0xF7DE) >> 1) + (((b) & 0xF7DE) >> 1)))

/* Mezcla un color de 16 bits con el de la niebla; f de 0 (nada) a 256 (todo niebla) */
static Uint32 m7_fog16 (Uint32 c, Uint32 fogc, int f)
{
    int r  = (c    >> RedShift) & ((1 << RedBits) - 1),   g  = (c    >> GreenShift) & ((1 << GreenBits) - 1), b  = c    & ((1 << BlueBits) - 1) ;
    int fr = (fogc >> RedShift) & ((1 << RedBits) - 1),   fg = (fogc >> GreenShift) & ((1 << GreenBits) - 1), fb = fogc & ((1 << BlueBits) - 1) ;

    r += ((fr - r) * f) >> 8 ;
    g += ((fg - g) * f) >> 8 ;
    b += ((fb - b) * f) >> 8 ;
    return (Uint32)((r << RedShift) | (g << GreenShift) | b) ;
}

typedef struct _m7sprite
{
    INSTANCE * instance ;
    double     forward ;      /* distancia hacia delante de la camara, en pixeles del mapa */
    double     lateral ;      /* distancia lateral (positiva = a la derecha) */
}
M7SPRITE ;

static int compare_by_distance (const void * ptr1, const void * ptr2)
{
    const M7SPRITE * s1 = (const M7SPRITE *)ptr1 ;
    const M7SPRITE * s2 = (const M7SPRITE *)ptr2 ;

    if (s1->forward > s2->forward) return -1 ;          /* primero los mas lejanos */
    if (s1->forward < s2->forward) return 1 ;
    return LOCDWORD(s2->instance,PROCESS_ID) - LOCDWORD(s1->instance,PROCESS_ID) ;      /* a igual distancia, en orden estable */
}

/* Indice dentro de una imagen que se repite (potencia de 2: con mascara; si no, con resto) */
static int m7_tile (int v, int size, int mask)
{
    if (mask >= 0) return v & mask ;
    v %= size ;
    return v < 0 ? v + size : v ;
}

static int m7_mask (int size)
{
    return (size > 0 && (size & (size - 1)) == 0) ? size - 1 : -1 ;
}

/* Dibuja una fila del suelo. u, v: posicion del mapa en 16.16 para el pixel de la izquierda; du, dv: incremento por pixel;
 * fog: proporcion de niebla de la fila (0-256) y fogc su color */
static void m7_row (GRAPH * dest, GRAPH * indoor, GRAPH * outdoor, Uint32 color, int translucent, int fog, Uint32 fogc, int fogbg,
                    int x0, int y, int width, long long u, long long v, long long du, long long dv)
{
    int x, sx, sy ;
    Uint32 c ;
    int hmask = outdoor ? m7_mask ((int)outdoor->width)  : 0 ;
    int vmask = outdoor ? m7_mask ((int)outdoor->height) : 0 ;
    Uint16 * row = (Uint16 *)((Uint8 *)dest->data + (size_t)dest->pitch * y) + x0 ;

    for (x = 0 ; x < width ; x++, u += du, v += dv)
    {
        sx = (int)(u >> 16) ;
        sy = (int)(v >> 16) ;

        if (indoor && sx >= 0 && sx < (int)indoor->width && sy >= 0 && sy < (int)indoor->height)
            c = ((Uint16 *)((Uint8 *)indoor->data + (size_t)indoor->pitch * sy))[sx] ;
        else if (outdoor)
        {
            sx = m7_tile (sx, (int)outdoor->width,  hmask) ;
            sy = m7_tile (sy, (int)outdoor->height, vmask) ;
            c = ((Uint16 *)((Uint8 *)outdoor->data + (size_t)outdoor->pitch * sy))[sx] ;
        }
        else
            c = color ;

        if (!c) continue ;                              /* el color 0 es transparente */
        if (fog) c = m7_fog16 (c, fogbg ? row[x] : fogc, fog) ;    /* fogbg: se funde con lo que ya hay pintado */
        row[x] = translucent ? M7_AVG16 (c, row[x]) : (Uint16)c ;
    }
}

void gr_mode7_draw (int n)
{
    MODE7      * mode7   = &current_mode7[n] ;
    GRAPH      * indoor  = mode7->indoor ;
    GRAPH      * outdoor = mode7->outdoor ;
    GRAPH      * dest    = scrbitmap ;
    MODE7_INFO * dat     = &((MODE7_INFO *)&GLODWORD(M7STRUCTS))[n] ;
    INSTANCE   * camera, * i ;
    REGION       clip ;

    double angle, ca, sa ;
    double cam_x, cam_y, ground_x, ground_y ;
    double h, f, half_w, fog_dist ;
    int    x0, y0, width, height, horizon, y, half ;
    int    translucent ;
    Uint32 fill, fogc ;
    int    fogbg ;                                       /* fog_color = -1: niebla que se funde con el fondo */

    static M7SPRITE * sprites = 0 ;
    static int sprites_reserved = 0 ;
    int sprite_count, k ;

    camera = instance_get (dat->camera_id) ;
    if (!camera || !dest) return ;

    gr_mode7_solids () ;                              /* los procesos solidos detienen a los que se han movido */

    if (dest->depth != 16 || (indoor && indoor->depth != 16) || (outdoor && outdoor->depth != 16))
        gr_error ("El modo 7 necesita mapas y pantalla de 16 bits") ;

    /* Region de dibujo, recortada a la pantalla */

    clip = *mode7->region ;
    if (clip.x < 0) clip.x = 0 ;
    if (clip.y < 0) clip.y = 0 ;
    if (clip.x2 >= (int)dest->width)  clip.x2 = dest->width - 1 ;
    if (clip.y2 >= (int)dest->height) clip.y2 = dest->height - 1 ;
    x0 = clip.x ;
    y0 = clip.y ;
    width  = clip.x2 - clip.x + 1 ;
    height = clip.y2 - clip.y + 1 ;
    if (width <= 0 || height <= 0) return ;

    /* Camara: "distance" pixeles detras del proceso, segun su angulo (milesimas de grado) */

    angle = (double)LOCDWORD (camera, ANGLE) * M_PI / 180000.0 ;
    ca = cos (angle) ;
    sa = sin (angle) ;

    cam_x = (double)LOCDWORD (camera, COORDX) ;
    cam_y = (double)LOCDWORD (camera, COORDY) ;
    RESOLXY (camera, cam_x, cam_y) ;
    ground_x = cam_x - ca * (double)(int)dat->distance ;
    ground_y = cam_y + sa * (double)(int)dat->distance ;

    h = (double)(int)dat->height / 4.0 ;                  /* altura de la camara en pixeles del mapa */
    if (dat->flags & B_VMIRROR) h = -h ;
    f = (double)width * (double)(int)dat->focus / 320.0 ; /* distancia focal en pixeles */
    half = width >> 1 ;
    half_w = (double)half ;
    horizon = (int)dat->horizon ;
    translucent = (dat->flags & B_TRANSLUCENT) ? 1 : 0 ;
    fog_dist = (double)(int)dat->fog_distance ;           /* 0 = sin niebla */
    fogbg = (dat->fog_color == 0xFFFFFFFFu) ;
    fogc = fogbg ? 0 : (dat->fog_color & 0xFFFF) ;

    /* Color de fuera del mapa (sin "outdoor"): un color de 16 bits (rgb()) */
    fill = dat->color ;

    /* Suelo */

    if (h != 0.0)
    for (y = 0 ; y < height ; y++)
    {
        double dy = (double)(y + 1 - horizon) ;     /* filas por debajo del horizonte */
        double step, z, u, v, du, dv ;
        int fog = 0 ;

        if (h > 0.0 ? dy <= 0.0 : dy >= 0.0) continue ;

        step = h / dy ;                              /* pixeles del mapa por pixel de pantalla */
        z    = step * f ;

        u  = ground_x + z * ca + (0.0 - half_w) * step * sa ;
        v  = ground_y - z * sa + (0.0 - half_w) * step * ca ;
        du = step * sa ;
        dv = step * ca ;

        if (u > 1e8 || u < -1e8 || v > 1e8 || v < -1e8) continue ;

        if (fog_dist > 0.0)
        {
            double t = (z < 0 ? -z : z) / fog_dist ;
            fog = t >= 1.0 ? 256 : (int)(t * 256.0) ;
        }

        if (dat->flags & B_HMIRROR)
        {
            u += du * (width - 1) ;
            v += dv * (width - 1) ;
            du = -du ;
            dv = -dv ;
        }

        m7_row (dest, indoor, outdoor, fill, translucent, fog, fogc, fogbg, x0, y0 + y, width,
                (long long)floor (u * 65536.0), (long long)floor (v * 65536.0),
                (long long)floor (du * 65536.0), (long long)floor (dv * 65536.0)) ;
    }

    /* Lista de procesos a dibujar (ctype = c_m7) con su posicion respecto a la camara */

    sprite_count = 0 ;
    for (i = first_instance ; i ; i = i->next)
    {
        int status = LOCDWORD(i,STATUS) & ~STATUS_WAITING_MASK ;
        double px, py, dx, dy, dist, a ;

        if ((status != STATUS_RUNNING && status != STATUS_FROZEN) || LOCDWORD(i,CTYPE) != 2) continue ;
        if (LOCDWORD(i,CFLAGS) && !(LOCDWORD(i,CFLAGS) & (1 << n))) continue ;

        px = (double)LOCDWORD(i,COORDX) ;
        py = (double)LOCDWORD(i,COORDY) ;
        RESOLXY (i, px, py) ;

        dx = px - ground_x ;
        dy = ground_y - py ;                         /* hacia arriba positivo, como el angulo */
        if (dx == 0.0 && dy == 0.0) { LOCDWORD(i,DISTANCE_1) = 0 ; continue ; }

        dist = sqrt (dx * dx + dy * dy) ;
        a = angle - atan2 (dy, dx) ;                 /* respecto a la direccion en que mira la camara */

        if (sprite_count == sprites_reserved)
        {
            sprites_reserved += 16 ;
            sprites = (M7SPRITE *) realloc (sprites, sizeof(M7SPRITE) * sprites_reserved) ;
            if (!sprites) { sprites_reserved = 0 ; return ; }
        }
        sprites[sprite_count].instance = i ;
        sprites[sprite_count].forward  = dist * cos (a) ;
        sprites[sprite_count].lateral  = dist * sin (a) ;

        /* Como en DIV: distance1 = distancia hacia delante y distance2 = lateral, en 1/16 de pixel */
        LOCDWORD(i,DISTANCE_1) = (int)(sprites[sprite_count].forward * 16.0) ;
        LOCDWORD(i,DISTANCE_2) = (int)(sprites[sprite_count].lateral * 16.0) ;
        sprite_count++ ;
    }

    qsort (sprites, sprite_count, sizeof(M7SPRITE), compare_by_distance) ;

    /* Dibuja primero los mas lejanos */

    for (k = 0 ; k < sprite_count ; k++)
    {
        INSTANCE * p = sprites[k].instance ;
        double zf = sprites[k].forward, lat = sprites[k].lateral ;
        double rel_h, base_y, screen_x, scale, visible = 1.0 ;
        int sprite_fog = 0 ;                         /* proporcion de niebla del proceso (0-256) cuando la niebla tiene color */
        static unsigned char * backup = 0 ;
        static size_t backup_size = 0 ;
        size_t bytes = 0 ;
        int used_backup = 0 ;
        int saved_size, saved_sizex, saved_sizey, saved_angle, saved_alpha, draw_angle, draw_x, draw_y ;
        GRAPH * pgr ;

        if (zf < 2.0) continue ;                    /* en la posicion de la camara o detras */

        if (fog_dist > 0.0)
        {
            double t = zf / fog_dist ;
            if (t >= 1.0) continue ;                 /* mas alla de la niebla no se ve */
            if (fogbg) visible = 1.0 - t ;           /* sin color de niebla: el proceso se desvanece con la distancia */
            else       sprite_fog = (int)(t * 256.0) ;
        }

        /* El angulo de un proceso con xgraph elige la vista segun su direccion respecto a la camara */
        saved_angle = LOCDWORD(p,ANGLE) ;
        draw_angle = 0 ;                             /* en el modo 7 el angulo no gira el grafico */
        if (LOCDWORD(p,XGRAPH))
        {
            int rel = saved_angle - LOCDWORD(camera,ANGLE) + 90000 ;
            int views = *(int *)LOCDWORD(p,XGRAPH) ;
            if (views > 0) rel += 180000 / views ;   /* redondeo al mas cercano */
            rel %= 360000 ;
            if (rel < 0) rel += 360000 ;
            draw_angle = rel ;
        }
        /* El angulo se deja puesto mientras se elige y se dibuja el grafico: draw_instance_at vuelve a elegir la vista con el */
        LOCDWORD(p,ANGLE) = draw_angle ;
        pgr = instance_graph (p) ;
        if (!pgr) { LOCDWORD(p,ANGLE) = saved_angle ; continue ; }

        rel_h    = ((double)(int)dat->height - (double)LOCDWORD(p,COORDZ)) / 4.0 ;
        base_y   = (double)horizon + rel_h * f / zf - 1.0 ;
        screen_x = (double)half + f * lat / zf ;
        scale    = (double)LOCDWORD(p,GRAPHSIZE) * f / (4.0 * zf) ;

        if (scale < 1.0 || scale > 20000.0) { LOCDWORD(p,ANGLE) = saved_angle ; continue ; }

        draw_x = x0 + (int)floor (screen_x + 0.5) ;
        draw_y = y0 + (int)floor (base_y + 0.5) ;

        /* Sin punto de control 0 el grafico se ancla por el centro de su base (el centro es el anclaje por defecto) */
        if (pgr->ncpoints == 0 || pgr->cpoints[0].x == CPOINT_UNDEFINED)
            draw_y -= (int)floor ((double)((int)pgr->height - 1 - (int)pgr->height / 2) * scale / 100.0 + 0.5) ;

        saved_size  = LOCDWORD(p,GRAPHSIZE) ;
        saved_sizex = LOCDWORD(p,GRAPHSIZEX) ;
        saved_sizey = LOCDWORD(p,GRAPHSIZEY) ;
        saved_alpha = LOCDWORD(p,ALPHA) ;
        LOCDWORD(p,GRAPHSIZE)  = (int)(scale + 0.5) ;
        LOCDWORD(p,GRAPHSIZEX) = 100 ;
        LOCDWORD(p,GRAPHSIZEY) = 100 ;
        if (visible < 1.0)
        {
            int a = (int)(saved_alpha * visible) ;
            LOCDWORD(p,ALPHA) = a < 1 ? 1 : a ;
        }

        /* Con niebla de color, los pixeles del grafico se mezclan con ese color mientras se dibuja y despues se restauran */
        if (sprite_fog > 0 && pgr->depth == 16)
        {
            bytes = (size_t)pgr->pitch * pgr->height ;
            if (bytes > backup_size)
            {
                unsigned char * nb = (unsigned char *) realloc (backup, bytes) ;
                if (nb) { backup = nb ; backup_size = bytes ; }
            }
            if (bytes <= backup_size)
            {
                Uint32 yy, xx ;
                memcpy (backup, pgr->data, bytes) ;
                used_backup = 1 ;
                for (yy = 0 ; yy < pgr->height ; yy++)
                {
                    Uint16 * px = (Uint16 *)((Uint8 *)pgr->data + (size_t)pgr->pitch * yy) ;
                    for (xx = 0 ; xx < pgr->width ; xx++)
                        if (px[xx])
                        {
                            Uint32 mixed = m7_fog16 (px[xx], fogc, sprite_fog) ;
                            px[xx] = (Uint16)(mixed ? mixed : 1) ;     /* el negro puro seria transparente */
                        }
                }
            }
        }

        draw_instance_at (p, &clip, draw_x, draw_y) ;

        if (used_backup) memcpy (pgr->data, backup, bytes) ;

        LOCDWORD(p,ANGLE)      = saved_angle ;
        LOCDWORD(p,GRAPHSIZE)  = saved_size ;
        LOCDWORD(p,GRAPHSIZEX) = saved_sizex ;
        LOCDWORD(p,GRAPHSIZEY) = saved_sizey ;
        LOCDWORD(p,ALPHA)      = saved_alpha ;
    }
}
