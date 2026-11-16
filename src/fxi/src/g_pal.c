/* Simurgh: modificado 2026-11-15 - sin paletas: solo quedan los colores de 16 bits, el fundido de pantalla y la lectura de la paleta de un fichero */
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
 *  Copyright (C) 1999 Jose Luis Cebrian Pague
 *  Copyright (C) 2002 Fenix Team
 *
 */

/*
 * FILE        : g_pal.c
 * DESCRIPTION : colores de 16 bits (rgb, get_rgb), fundido de pantalla (fade) y lectura de la paleta de un fichero.
 *
 * Simurgh trabaja siempre a 16 bits y ya no tiene paleta de pantalla: la paleta de un fichero de 8 bits (MAP, FPG, FNT, PCX) solo
 * se usa para convertir sus graficos a 16 bits al cargarlos (bitmap_adapt16).
 */

#include <string.h>
#include <stdlib.h>

#include "fxi.h"

Uint16     colorghost[65536] ;  /* Deja un color a 50% de intensidad */

static int fade_inc = 0 ;
int        fade_on = 0 ;
int        fade_step = 64 ;
SDL_Color  fade_from ;
SDL_Color  fade_to ;
SDL_Color  fade_pos = { 100, 100, 100 };

void gr_fade_init (int r, int g, int b, int speed)
{
    fade_inc  = speed;
    fade_step = 0;
    fade_on   = 1 ;
    fade_from = fade_pos;
    fade_to.r = r > 200 ? 200 : r ;
    fade_to.g = g > 200 ? 200 : g ;
    fade_to.b = b > 200 ? 200 : b ;

    GLODWORD(FADING) = 1 ;
}

void gr_fade_step()
{
    if (fade_on != 0)
    {
        GLODWORD(FADING) = 1 ;

        fade_step += fade_inc ;
        if (fade_step < 0)
        {
            GLODWORD(FADING) = 0 ;
            fade_step = 0 ;
            fade_on = 0 ;
        }
        if (fade_step >= 64)
        {
            GLODWORD(FADING) = 0 ;
            fade_step = 64 ;
            fade_on = 0 ;
        }

        fade_pos.r = (fade_to.r * fade_step + fade_from.r * (64-fade_step)) / 64;
        fade_pos.g = (fade_to.g * fade_step + fade_from.g * (64-fade_step)) / 64;
        fade_pos.b = (fade_to.b * fade_step + fade_from.b * (64-fade_step)) / 64;

        if ((fade_step + fade_inc < 0 || fade_step + fade_inc > 64) &&
            (fade_pos.r == 100 && fade_pos.g == 100 && fade_pos.b == 100))
        {
            GLODWORD(FADING) = 0 ;
            fade_on = 0;
        }

        if (scrbitmap->depth == 16)
        {
            gr_fade16 (scrbitmap,  fade_pos.r, fade_pos.g, fade_pos.b);
        }
    }
}

/*
 *  FUNCTION : gr_read_pal_keep
 *
 *  Lee la paleta de un fichero de 8 bits (256 colores de 0 a 63 y las 576 bytes de gamas) y la devuelve en rgb (256 tripletes de 0 a
 *  255) para convertir los graficos de ESE fichero a 16 bits. No toca nada mas.
 */

int gr_read_pal_keep (file * fp, unsigned char * rgb)
{
    unsigned char colors[256][3] ;
    int i ;

    if (!file_read (fp, colors, 3 * 256))
        return 0 ;

    file_seek (fp, 576, SEEK_CUR) ;

    for (i = 0 ; i < 256 ; i++)
    {
        rgb[i*3]   = (colors[i][0] << 2) | (colors[i][0] >> 4) ;
        rgb[i*3+1] = (colors[i][1] << 2) | (colors[i][1] >> 4) ;
        rgb[i*3+2] = (colors[i][2] << 2) | (colors[i][2] >> 4) ;
    }

    return 1 ;
}

/* Color de 16 bits (formato de la pantalla) a partir de r, g, b (0-255); el negro puro es transparente, asi que se da casi negro */

int gr_rgb (int r, int g, int b)
{
    int color ;

    if (!scr_initialized) gr_init (320, 200) ;

    r >>= screen->format->Rloss ;
    g >>= screen->format->Gloss ;
    b >>= screen->format->Bloss ;

    color= (r << screen->format->Rshift) |
           (g << screen->format->Gshift) |
           (b << screen->format->Bshift) ;
    if (!color) return 1 ;
    return color ;
}

void gr_get_rgb (int color, int *r, int *g, int *b)
{
    if (!scr_initialized) gr_init (320, 200) ;

    (*r) = ((color & screen->format->Rmask) >> screen->format->Rshift) ;
    (*g) = ((color & screen->format->Gmask) >> screen->format->Gshift) ;
    (*b) = ((color & screen->format->Bmask) >> screen->format->Bshift) ;

    (*r) <<= screen->format->Rloss ;
    (*g) <<= screen->format->Gloss ;
    (*b) <<= screen->format->Bloss ;
}

Uint16 gr_get_color (int r, int g, int b)
{
    return SDL_MapRGB (screen->format, (Uint8)r, (Uint8)g, (Uint8)b) ;
}
