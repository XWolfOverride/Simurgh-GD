/* Simurgh: modificado 2026-11-15 - LOAD_IMAGE, SAVE_MAP, SAVE_FPG, adaptacion a 16 bits al cargar, sin paletas */
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

/*
 * FILE        : g_maps.c
 * DESCRIPTION : bitmap & pixbuff functions
 *
 * HISTORY:      0.81 - patched bitmap_new to initializa to 0 values
 *				 0.82 - added gr_save_map
 */

#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "fxi.h"

extern SDL_Surface * screen ;

extern GRAPH * gr_read_png (const char * filename) ;
extern GRAPH * gr_read_pcx (const char * filename) ;

static void gr_file_ext (const char * filename, char * ext, int size) ;

static int free_map_code = 1000 ;

static GRAPH * first = NULL;

/* Returns the code of a new system library graph (1000+). Searchs
   for free slots if the program creates too many system maps */

int bitmap_next_code()
{
	int i;

	if (free_map_code > 2000)
	{
		for (i = free_map_code-1 ; i >= 1000 ; i--)
		{
			if (bitmap_get(0, i) == 0)
				return i;
		}
	}
	return free_map_code++;
}

GRAPH * bitmap_new (int code, int w, int h, int depth, int frames)
{
	GRAPH  * gr ;
	int      bytesPerRow, wb ;

	if (depth != 8 && depth != 16 && depth != 1)
	{
		gr_con_printf ("Profundidad de color no soportada (new bitmap)\n") ;
		return NULL;
	}
	if (frames < 1)
		frames = 1;

	/* Calculate the row size (dword-aligned) */

	wb = w * depth / 8;
	if (wb*8/depth < w)
		wb++;

	bytesPerRow = wb;
	if (bytesPerRow & 0x03)
		bytesPerRow = (bytesPerRow & ~3) + 4;

	/* Create and fill the struct */

	gr = (GRAPH *) malloc (sizeof(GRAPH)) ;
	if (!gr)
	{
		gr_con_printf ("bitmap_new(%dx%dx%d): sin memoria", w, h, depth) ; ;
		return NULL;
	}

	memset (gr, 0, sizeof(GRAPH));

	gr->width            = w ;
	gr->widthb           = wb ;
	gr->pitch            = bytesPerRow ;
	gr->frames		     = frames;
	gr->height           = h ;
	gr->depth            = depth ;
	gr->code             = code ;
	gr->data_start       = (char *) malloc (h * gr->pitch * frames) ;
	gr->data             = gr->data_start;
	gr->cpoints          = NULL ;
	gr->blend_table      = NULL ;
	gr->keyframes        = (FBM_KEYFRAME *) malloc(sizeof(FBM_KEYFRAME));
	gr->sequences	     = (FBM_SEQUENCE *) malloc(sizeof(FBM_SEQUENCE));
	gr->next             = first;
	gr->prev             = NULL;
	gr->current_keyframe = -1;

	if (first)
		first->prev = gr;
	first = gr;

	if (!gr->data || !gr->keyframes || !gr->sequences)
	{
		if (gr->data)
			free(gr->data);
		if (gr->keyframes)
			free(gr->keyframes);
		if (gr->sequences)
			free(gr->sequences);
		free (gr);

		gr_con_printf ("bitmap_new: sin memoria en calloc(%d, %d)", h, gr->pitch) ; ;
		return NULL;
	}

	// Fills default data for the animation

	gr->keyframes->angle = 0;
	gr->keyframes->flags = 0;
	gr->keyframes->frame = 0;
	gr->keyframes->pause = 25;

	gr->sequences->first_keyframe = 0;
	gr->sequences->last_keyframe = 0;
	gr->sequences->next_sequence = 0;
	gr->sequences->name[0] = 0;

	return gr ;
}

GRAPH * bitmap_clone (GRAPH * map)
{
	GRAPH * gr ;
	Uint32 y;

	gr = bitmap_new_syslib(map->width, map->height, map->depth, 1) ;
	if (gr == NULL) return NULL;
	for (y = 0 ; y < map->height ; y++)
	{
		memcpy ((Uint8*)gr->data_start + gr->pitch * y,
			    (Uint8*)map->data + gr->pitch * y,
				gr->widthb);
	}
	if (map->cpoints)
	{
		gr->cpoints = malloc(4 * map->ncpoints) ;
		memcpy (gr->cpoints, map->cpoints, 4 * map->ncpoints) ;
		gr->ncpoints = map->ncpoints ;
	}
	gr->width = map->width;
	gr->blend_table = map->blend_table;
	if (map->alpha)
	{
		gr->alpha = (Uint8 *) malloc (map->width * map->height) ;
		if (gr->alpha) memcpy (gr->alpha, map->alpha, map->width * map->height) ;
	}
	gr->offset = map->offset ;
	gr->info_flags = map->info_flags ;
	gr->modified = map->modified ;
	memcpy (gr->name, map->name, sizeof(map->name)) ;
	return gr ;
}

void bitmap_add_cpoint (GRAPH * map, int x, int y)
{
	map->cpoints = (CPOINT *) realloc (map->cpoints, (map->ncpoints+1) * sizeof(CPOINT)) ;
	map->cpoints[map->ncpoints].x = x ;
	map->cpoints[map->ncpoints].y = y ;
	map->ncpoints++;
}


/*
 *  FUNCTION : bitmap_set_cpoint
 *
 *  Set a control point in a graphic
 *
 *  PARAMS :
 *		map				Pointer to the bitmap
 *		point			Control point index
 *		x				New X coordinate or CPOINT_UNDEFINED to unset
 *		y				New Y coordinate or CPOINT_UNDEFINED to unset
 *
 *  RETURN VALUE :
 *      None
 *
 */

void bitmap_set_cpoint (GRAPH * map, Uint32 point, int x, int y)
{
	Uint32 n;

	if (point < 0)
		return;

	if (map->ncpoints <= point)
	{
		map->cpoints = (CPOINT *) realloc (map->cpoints, (point+1) * sizeof(CPOINT)) ;
		for (n = map->ncpoints; n < point; n++)
		{
			map->cpoints[n].x = CPOINT_UNDEFINED;
			map->cpoints[n].y = CPOINT_UNDEFINED;
		}
		map->ncpoints = point+1 ;
	}
	map->cpoints[point].x = x;
	map->cpoints[point].y = y;
}

void bitmap_destroy (GRAPH * map)
{
    if (!map)
        return;

	if (map->prev)
		map->prev->next = map->next;

	if (map->next)
		map->next->prev = map->prev;
/*
	if (map->next_time)
		map->next->prev = map->next;
*/
	if (first == map)
		first = map->next;

	if (map->cpoints) free(map->cpoints) ;
	if (map->alpha) free(map->alpha) ;
	if (map->keyframes) free(map->keyframes);
	if (map->sequences) free(map->sequences);

	free (map->data) ;
	free (map) ;
}


/*
 *  Guarda el alfa de un pixel cargado. El plano de alfa se crea al ver el primer valor distinto de 255 (las imagenes
 *  totalmente opacas no gastan memoria).
 */

void gr_alpha_set (GRAPH * gr, Uint32 x, Uint32 y, int a)
{
	if (!gr->alpha)
	{
		if (a == 255) return ;
		gr->alpha = (Uint8 *) malloc (gr->width * gr->height) ;
		if (!gr->alpha) return ;
		memset (gr->alpha, 255, gr->width * gr->height) ;
	}
	gr->alpha[y * gr->width + x] = a ;
}

/*
 *  FUNCTION : gr_read_truecolor_rows
 *
 *  Lee los datos de un grafico de 24 bits (R,G,B) o 32 bits (R,G,B,A) del fichero y los guarda en gr, que es de 16 bits.
 *  Filas de arriba a abajo, sin relleno, cada color en un byte; alfa sin premultiplicar.
 *
 *  El alfa se aplica por ahora asi (provisional): alfa < 128 -> color 0 (transparente); si no, opaco, y el negro opaco se
 *  desplaza a un azul casi negro (0x0001) para que no se confunda con el transparente. Sin alfa (24 bits) el negro puro
 *  sigue siendo transparente, como en un MAP de 16 bits.
 *
 *  RETURN VALUE : 1 si va bien, 0 si el fichero esta truncado
 */

int gr_read_truecolor_rows (file * fp, GRAPH * gr, int srcbpp)
{
	int bytes = srcbpp / 8 ;
	Uint32 x, y ;
	Uint8 * src = (Uint8 *) malloc (gr->width * bytes) ;

	if (!src) return 0 ;
	for (y = 0 ; y < gr->height ; y++)
	{
		Uint16 * dst = (Uint16 *)((Uint8 *)gr->data + gr->pitch * y) ;

		if (!file_read (fp, src, gr->width * bytes))
		{
			free (src) ;
			return 0 ;
		}
		for (x = 0 ; x < gr->width ; x++)
		{
			Uint8 * p = src + x * bytes ;
			Uint16 c = ((p[0] & 0xF8) << 8) | ((p[1] & 0xFC) << 3) | (p[2] >> 3) ;

			if (bytes == 4)
			{
				gr_alpha_set (gr, x, y, p[3]) ;
				if (p[3] < 128) c = 0 ;
				else if (c == 0) c = 1 ;
			}
			dst[x] = c ;
		}
		if (scr_initialized) gr_convert16_565ToScreen (dst, gr->width) ;
	}
	free (src) ;
	return 1 ;
}

static GRAPH * gr_read_map (file * fp)
{
	unsigned char filepal[768] ;
	int have_pal = 0 ;
	char header[8] ;
	unsigned short int w, h, c ;
	Uint32 y ;
	int depth, code, srcbpp = 0 ;
	GRAPH * gr ;
	int len;

	/* Carga los datos de cabecera */

	file_read (fp, header, 8) ;
	if (strcmp (header, M16_MAGIC) == 0)
		depth = 16 ;
	else if (strcmp (header, MAP_MAGIC) == 0)
		depth = 8 ;
	else if (strcmp (header, M01_MAGIC) == 0)
		depth = 1 ;
	else if (strcmp (header, M24_MAGIC) == 0)
	{
		depth = 16 ;	/* se convierte a 16 bits al cargar */
		srcbpp = 24 ;
	}
	else if (strcmp (header, M32_MAGIC) == 0)
	{
		depth = 16 ;
		srcbpp = 32 ;
	}
	else
		return 0 ;

	file_readUint16 (fp, &w) ;
	file_readUint16 (fp, &h) ;
	file_readSint32 (fp, &code) ;

	gr = bitmap_new (code, w, h, depth, 1) ;
	if (!gr) return 0 ;
	file_read (fp, gr->name, 32) ;
	gr->name[31] = 0 ;

	/* Datos de paleta */

	if (gr->depth == 8)
	{
		if (!gr_read_pal_keep (fp, filepal)) return 0 ;
		have_pal = 1 ;
	}

	/* Puntos de control */

	file_readUint16 (fp, &c) ;
	gr->ncpoints = c ;

	if (gr->ncpoints)
	{
		gr->cpoints = (CPOINT *) malloc (c * sizeof(CPOINT)) ;
		if (!gr->cpoints) {
		    free(gr) ;
		    return 0 ;
		}

		for (c = 0 ; c < gr->ncpoints ; c++)
		{
			file_readUint16 (fp, &w) ;
			file_readUint16 (fp, &h) ;
			if ((short int) w == -1 && (short int) h == -1)
			{
				w = CPOINT_UNDEFINED;
				h = CPOINT_UNDEFINED;
			}
			gr->cpoints[c].x = w ;
			gr->cpoints[c].y = h ;
		}
	}
	else
	    gr->cpoints = 0 ;

	if (srcbpp)
	{
		if (!gr_read_truecolor_rows (fp, gr, srcbpp))
		{
			free (gr->data) ;
			free (gr) ;
			return 0 ;
		}
		gr->modified = 1 ;
		return gr ;
	}

	len = gr->widthb;

	/* Datos del gráfico */

	for (y = 0 ; y < gr->height ; y++)
	{
		Uint8 * line = (Uint8 *)gr->data + gr->pitch*y;
		if (!file_read (fp, line, len))
		{
			free (gr->data) ;
			free (gr) ;
			return 0 ;
		}
		if (gr->depth == 16)
		{
			ARRANGE_WORDS (line, len/2);
			if (scr_initialized) gr_convert16_565ToScreen ((Uint16 *)line, len/2);
		}
	}

	gr->modified = 1 ;

	/* Los graficos de 8 y 1 bit se adaptan una sola vez, al cargarlos, a los 16 bits del modo (con la paleta del propio fichero) */
	return bitmap_adapt16 (gr, have_pal ? filepal : NULL) ;
}

/* Funciones de carga de nivel superior */

int gr_load_png (const char * mapname)
{
	GRAPH * gr ;

	gr = gr_read_png (mapname) ;
	if (!gr) return -1 ;
	gr->code = bitmap_next_code() ;
	assert (syslib) ;
	grlib_add_map (0, gr) ;
	return gr->code ;
}

int gr_load_pcx (const char * mapname)
{
	GRAPH * gr ;

	gr = gr_read_pcx (mapname) ;
	if (!gr) return -1 ;
	gr->code = bitmap_next_code() ;
	assert (syslib) ;
	grlib_add_map (0, gr) ;
	return gr->code ;
}

int gr_load_map (const char * mapname)
{
	GRAPH * gr ;
	file * fp = file_open (mapname, "rb") ;

	if (!fp)
	{
		gr_con_printf ("Mapa %s no encontrado\n", mapname) ;
		return -1 ;
	}

	gr = gr_read_map (fp) ;
	file_close (fp) ;

	if (!gr) return -1 ;
	gr->code = bitmap_next_code() ;
	assert (syslib) ;
	grlib_add_map (0, gr) ;
	return gr->code ;
}


/*
 *  FUNCTION : gr_load_image
 *
 *  Carga una imagen detectando el formato por la cabecera del fichero (no por la extension). Es la funcion que hay
 *  detras de LOAD_IMAGE y, por los alias del compilador, de LOAD_MAP, LOAD_PNG y LOAD_PCX.
 *
 *  Formatos: MAP (map, m16, m01), PNG, PCX. Para anadir uno basta con reconocer su cabecera aqui.
 *
 *  RETURN VALUE : codigo del grafico en la libreria del sistema, o -1
 */

int gr_load_image (const char * filename)
{
	unsigned char h[16] ;
	int n ;
	file * fp = file_open (filename, "rb") ;

	if (!fp)
	{
		gr_con_printf ("Imagen %s no encontrada\n", filename) ;
		return -1 ;
	}
	memset (h, 0, sizeof(h)) ;
	n = file_read (fp, h, 16) ;		/* devuelve los bytes leidos (file_size no vale con ficheros comprimidos) */
	if (n < 0) n = 0 ;
	file_close (fp) ;

	if (n >= 7 && memcmp (h + 3, "\x1A\x0D\x0A", 3) == 0 &&
	    (memcmp (h, "map", 3) == 0 || memcmp (h, "m16", 3) == 0 || memcmp (h, "m01", 3) == 0 ||
	     memcmp (h, "m24", 3) == 0 || memcmp (h, "m32", 3) == 0))
		return gr_load_map (filename) ;

	if (n >= 8 && memcmp (h, "\x89PNG\x0D\x0A\x1A\x0A", 8) == 0)
		return gr_load_png (filename) ;

	if (n >= 3 && h[0] == 0x0A && h[1] <= 5 && h[2] <= 1)
	{
		if (!scr_initialized) gr_init (320, 200) ;
		return gr_load_pcx (filename) ;
	}

	if (n >= 14 && h[0] == 'B' && h[1] == 'M')
	{
		int r = gr_register_loaded (gr_read_bmp (filename)) ;
		if (r >= 0) return r ;
	}

	/* TGA no tiene firma en la cabecera: se reconoce por la extension .tga (despues se valida su cabecera al leer) */
	{
		char ext[8] ;
		gr_file_ext (filename, ext, sizeof(ext)) ;
		if (strcmp (ext, "tga") == 0)
		{
			int r = gr_register_loaded (gr_read_tga (filename)) ;
			if (r >= 0) return r ;
		}
	}

	gr_con_printf ("Imagen %s: formato no reconocido\n", filename) ;
	return -1 ;
}

/*
 *  FUNCTION : gr_save_map
 *
 *  Graba un grafico en formato MAP de Fenix (map 8 bits, m16 16 bits, m01 1 bit), compatible con DIV/Fenix.
 *  Solo se graba el fotograma actual.
 *
 *  RETURN VALUE : 1 si va bien, 0 si hay error
 */

int gr_save_map (GRAPH * gr, const char * filename)
{
	file * fp ;
	unsigned char header[8] ;
	Uint16 w, h, c ;
	Sint32 code ;
	char name[32] ;
	Uint32 y ;

	if (!gr) return 0 ;
	fp = file_open (filename, "wb0") ;	/* sin gzip: ficheros legibles por otras herramientas */
	if (!fp) return 0 ;

	memset (header, 0, 8) ;
	memcpy (header, gr->alpha ? M32_MAGIC : gr->depth == 16 ? M16_MAGIC : gr->depth == 1 ? M01_MAGIC : MAP_MAGIC, 6) ;
	file_write (fp, header, 8) ;
	w = gr->width ; h = gr->height ; code = gr->code ;
	file_writeUint16 (fp, &w) ;
	file_writeUint16 (fp, &h) ;
	file_writeSint32 (fp, &code) ;
	memset (name, 0, 32) ;
	strncpy (name, gr->name, 31) ;
	file_write (fp, name, 32) ;

	c = gr->ncpoints ;
	file_writeUint16 (fp, &c) ;
	for (c = 0 ; c < gr->ncpoints ; c++)
	{
		Sint16 x = gr->cpoints[c].x, yy = gr->cpoints[c].y ;
		if (gr->cpoints[c].x == CPOINT_UNDEFINED && gr->cpoints[c].y == CPOINT_UNDEFINED) x = yy = -1 ;
		file_writeSint16 (fp, &x) ;
		file_writeSint16 (fp, &yy) ;
	}

	if (gr->alpha)		/* MAP de 32 bits: R,G,B,A conservando el alfa cargado */
	{
		Uint8 * rgba = (Uint8 *) malloc (gr->width * 4) ;
		if (!rgba) { file_close (fp) ; return 0 ; }
		for (y = 0 ; y < gr->height ; y++)
		{
			gr_row_to_rgba (gr, y, rgba) ;
			file_write (fp, rgba, gr->width * 4) ;
		}
		free (rgba) ;
		file_close (fp) ;
		return 1 ;
	}

	for (y = 0 ; y < gr->height ; y++)
	{
		Uint8 * line = (Uint8 *)gr->data + gr->pitch * y ;
		if (gr->depth == 16)
		{
			Uint16 * tmp = (Uint16 *) malloc (gr->widthb) ;
			if (!tmp) { file_close (fp) ; return 0 ; }
			memcpy (tmp, line, gr->widthb) ;
			if (scr_initialized) gr_convert16_ScreenTo565 (tmp, gr->widthb / 2) ;
			ARRANGE_WORDS (tmp, gr->widthb / 2) ;
			file_write (fp, tmp, gr->widthb) ;
			free (tmp) ;
		}
		else
			file_write (fp, line, gr->widthb) ;
	}
	file_close (fp) ;
	return 1 ;
}


/* ---------------------------------------------------------------------- */
/* Ayudas comunes de los formatos de imagen (BMP, TGA...)                  */
/* ---------------------------------------------------------------------- */

/*
 *  Lee un fichero entero en memoria (admite ficheros comprimidos con gzip). Devuelve el bloque (a liberar con free) y su
 *  tamano en *len, o NULL si no se puede leer.
 */

Uint8 * gr_slurp_file (const char * filename, int * len)
{
	file * fp = file_open (filename, "rb") ;
	Uint8 * buf = NULL ;
	int size = 0, reserved = 0, n ;

	if (!fp) return NULL ;
	for (;;)
	{
		if (size + 65536 > reserved)
		{
			Uint8 * nb ;
			reserved = reserved ? reserved * 2 : 65536 * 2 ;
			nb = (Uint8 *) realloc (buf, reserved) ;
			if (!nb) { free (buf) ; file_close (fp) ; return NULL ; }
			buf = nb ;
		}
		n = file_read (fp, buf + size, 65536) ;
		if (n <= 0) break ;
		size += n ;
		if (n < 65536) break ;
	}
	file_close (fp) ;
	if (size == 0) { free (buf) ; return NULL ; }
	*len = size ;
	return buf ;
}

/*
 *  Convierte un color a 16 bits (RGB565). Con use_alpha, alfa < 128 es transparente (color 0) y el negro opaco pasa a
 *  0x0001 para no confundirse con el transparente (ver DOC/FORMATOS-IMAGEN.md). Sin alfa, el negro puro es transparente.
 */

Uint16 gr_pack16 (int r, int g, int b, int a, int use_alpha)
{
	Uint16 c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xFF) >> 3) ;

	if (use_alpha)
	{
		if (a < 128) c = 0 ;
		else if (c == 0) c = 1 ;
	}
	return c ;
}

/*
 *  Convierte la fila y del grafico a bytes R,G,B,A (width*4). Si el grafico conserva el alfa con que se cargo, ese es el alfa;
 *  si no, el color 0 es transparente (alfa 0), salvo en graficos con GI_NOCOLORKEY. Admite graficos de 1, 8 y 16 bits.
 */

void gr_row_to_rgba (GRAPH * gr, Uint32 y, Uint8 * out)
{
	Uint8 * line = (Uint8 *) gr->data + gr->pitch * y ;
	Uint32 x ;

	if (gr->depth == 16)
	{
		Uint16 * tmp = (Uint16 *) malloc (gr->widthb) ;
		if (!tmp) { memset (out, 0, gr->width * 4) ; return ; }
		memcpy (tmp, line, gr->widthb) ;
		if (scr_initialized) gr_convert16_ScreenTo565 (tmp, gr->width) ;
		for (x = 0 ; x < gr->width ; x++)
		{
			int c = tmp[x] ;
			int r = (c >> 11) & 31, g = (c >> 5) & 63, b = c & 31 ;
			*out++ = (r << 3) | (r >> 2) ;
			*out++ = (g << 2) | (g >> 4) ;
			*out++ = (b << 3) | (b >> 2) ;
			*out++ = gr->alpha ? gr->alpha[y * gr->width + x] : ((c == 0 && !(gr->info_flags & GI_NOCOLORKEY)) ? 0 : 255) ;
		}
		free (tmp) ;
	}
	else	/* 1 bit: bit a 1 = blanco, a 0 = transparente */
	{
		for (x = 0 ; x < gr->width ; x++)
		{
			int bit = (line[x >> 3] >> (7 - (x & 7))) & 1 ;
			*out++ = bit ? 255 : 0 ;
			*out++ = bit ? 255 : 0 ;
			*out++ = bit ? 255 : 0 ;
			*out++ = bit ? 255 : 0 ;
		}
	}
}

/* Crea un grafico de 16 bits para una imagen cargada de un fichero y lo deja listo para rellenar */

GRAPH * gr_new_loaded (int w, int h)
{
	if (w <= 0 || h <= 0 || w > 8192 || h > 8192) return NULL ;
	return bitmap_new (0, w, h, 16, 1) ;
}

/* Termina la carga: marca el grafico, lo convierte al formato de pantalla si hace falta y lo registra */

int gr_register_loaded (GRAPH * gr)
{
	Uint32 y ;

	if (!gr) return -1 ;
	if (scr_initialized)
		for (y = 0 ; y < gr->height ; y++)
			gr_convert16_565ToScreen ((Uint16 *)((Uint8 *)gr->data + gr->pitch * y), gr->width) ;
	gr->modified = 1 ;
	gr->code = bitmap_next_code() ;
	assert (syslib) ;
	grlib_add_map (0, gr) ;
	return gr->code ;
}

/* Ayuda para los nombres: ultima extension en minusculas (sin punto), o "" */

static void gr_file_ext (const char * filename, char * ext, int size)
{
	const char * dot = strrchr (filename, '.') ;
	int n = 0 ;

	if (dot && !strchr (dot, '/') && !strchr (dot, '\\'))
		for (dot++ ; *dot && n < size - 1 ; dot++, n++)
			ext[n] = (*dot >= 'A' && *dot <= 'Z') ? *dot + 32 : *dot ;
	ext[n] = 0 ;
}

/*
 *  FUNCTION : gr_save_image
 *
 *  Graba un grafico en el formato pedido (IMG_MAP, IMG_PNG, IMG_BMP, IMG_TGA) o, con IMG_AUTO (0), segun la extension del
 *  nombre (.map, .png, .bmp, .tga); si la extension no se reconoce, en formato MAP.
 *
 *  RETURN VALUE : 1 si va bien, 0 si hay error
 */

int gr_save_image (GRAPH * gr, const char * filename, int format)
{
	if (!gr) return 0 ;
	if (format == IMG_AUTO)
	{
		char ext[8] ;
		gr_file_ext (filename, ext, sizeof(ext)) ;
		if      (strcmp (ext, "map") == 0) format = IMG_MAP ;
		else if (strcmp (ext, "png") == 0) format = IMG_PNG ;
		else if (strcmp (ext, "bmp") == 0) format = IMG_BMP ;
		else if (strcmp (ext, "tga") == 0) format = IMG_TGA ;
		else format = IMG_MAP ;	/* extension desconocida: MAP, el formato nativo */
	}
	switch (format)
	{
		case IMG_MAP: return gr_save_map (gr, filename) ;
		case IMG_PNG: return gr_save_png (gr, filename) ;
		case IMG_BMP: return gr_save_bmp (gr, filename) ;
		case IMG_TGA: return gr_save_tga (gr, filename) ;
	}
	gr_con_printf ("SAVE_IMAGE: formato %d desconocido\n", format) ;
	return 0 ;
}

GRAPH * bitmap_new_syslib (int w, int h, int depth, int frames)
{
	GRAPH * gr ;

	gr = bitmap_new (0, w, h, depth, frames) ;
	gr->code = bitmap_next_code() ;
	assert (syslib) ;
	grlib_add_map (0, gr) ;
	return gr ;
}

/* Análisis */

void bitmap_analize (GRAPH * bitmap)
{
	Uint32 x, y;

	if (bitmap->modified > 0)
		bitmap->modified = 0 ;
	bitmap->info_flags = 0 ;

	/* Search for transparent pixels (value 0).
	 * If none found, set the flag GI_NOCOLORKEY */

	if (bitmap->depth == 8)
	{
		for (y = 0 ; y < bitmap->height ; y++)
		{
			Uint8 * ptr = memchr ((Uint8 *)bitmap->data + bitmap->pitch*y, 0, bitmap->width) ;
			if (ptr) break;
		}
		if (y == bitmap->height)
			bitmap->info_flags |= GI_NOCOLORKEY ;
	}
	else
	{
		Uint16 * ptr = (Uint16 *)bitmap->data ;
		for (y = 0 ; y < bitmap->height ; y++, ptr += bitmap->pitch/2)
		{
			for (x = 0 ; x < bitmap->width ; x++)
				if (ptr[x] == 0) break;
			if (x != bitmap->width)
				break;
		}
		if (y == bitmap->height)
			bitmap->info_flags |= GI_NOCOLORKEY ;
	}
}

/* Animación */

void bitmap_animate_to (GRAPH * bitmap, int pos, int speed)
{
}

/*
 *  FUNCTION : bitmap_animate
 *
 *  Update the current bitmap frame using the current_time global,
 *  advancing the bitmap animation if needed
 *
 *  PARAMS :
 *		dest			Destination bitmap or NULL for screen
 *		clip			Clipping region or NULL for the whole screen
 *		scrx, scry		Pixel coordinates of the center on screen
 *      gr				Pointer to the graphic object to draw
 *
 *  RETURN VALUE :
 *      None
 *
 */

void bitmap_animate (GRAPH * map)
{
	if (map->last_frame == frame_count || map->next_time > current_time || current_time == 0)
	    return;

	// Check for internal error conditions that shouldn't happen

	if (map->current_keyframe > map->max_keyframe) map->current_keyframe = 0;
	if (map->current_sequence > map->max_sequence) map->current_sequence = 0;

	map->last_frame = frame_count;

	// Update the animation

	map->end_of_sequence = 0;

	// Advance to the next keyframe

	map->current_keyframe++;
	map->next_time += map->keyframes[map->current_keyframe].pause;

	// At end of sequence, move to the next sequence of mark the map
	// as if it is at the end of the animation

	if (map->current_keyframe > map->sequences[map->current_sequence].last_keyframe)
	{
		if (map->sequences[map->current_sequence].next_sequence >= 0)
		{
			map->current_sequence = map->sequences[map->current_sequence].next_sequence;
			if (map->current_sequence > map->max_sequence) map->current_sequence = 0;

			map->current_keyframe = map->sequences[map->current_sequence].first_keyframe;
		}
		else
		{
			map->current_keyframe--;
			map->end_of_sequence = 1;
		}
	}

	// This is an internal error condition; shouldn't happen
	if (map->current_keyframe > map->max_keyframe) map->current_keyframe = 0;

	map->current_frame = map->keyframes[map->current_keyframe].frame;
	if (map->current_frame > map->frames-1) map->current_frame = 0;

	// Calculate the next time, only if there was a long wait since
	// the last update

	map->next_time = current_time + map->keyframes[map->current_keyframe].pause;

	// Adjust the graphic data

	map->data = (Uint8*)map->data_start + map->height * map->pitch * map->current_frame;

}

/*
 *  FUNCTION : bitmap_16bits_conversion
 *
 *  When 16 bits mode is initialized for the first time, this
 *  function will convert every 16 bit bitmap in memory to
 *  the screen format
 *
 *  PARAMS :
 *		None
 *
 *  RETURN VALUE :
 *      None
 *
 */

void bitmap_16bits_conversion()
{
	GRAPH * map = first;

	while (map)
	{
		if (map->depth == 16)
			gr_convert16_565ToScreen (map->data_start, map->width * map->height * map->frames);

		map = map->next;
	}
}

/*
 *  FUNCTION : bitmap_adapt16
 *
 *  Devuelve una copia en 16 bits (formato de la pantalla) de un grafico de 8 o de 1 bit y destruye el original: los graficos se
 *  adaptan al modo UNA vez, al cargarlos, y a partir de ahi se dibujan, se modifican y se graban en 16 bits. Un grafico que ya es de
 *  16 bits se devuelve tal cual.
 *
 *  PARAMS :
 *      src     grafico a adaptar
 *      rgb     paleta de 256 colores (tripletes r, g, b de 0 a 255); NULL = la paleta actual
 *
 *  El indice 0 es transparente y sigue siendo 0 (negro puro); otro color que diera negro puro se deja en el casi negro 1.
 */

GRAPH * bitmap_adapt16 (GRAPH * src, const unsigned char * rgb)
{
	GRAPH * dst ;
	Uint16  table[256] ;
	Uint32  x, y ;
	int     n ;

	if (!src || src->depth == 16) return src ;

	if (!scr_initialized) gr_init (320, 200) ;		/* hace falta conocer el formato de la pantalla */

	if (src->depth == 8)
	{
		for (n = 0 ; n < 256 ; n++)
		{
			Uint32 c ;
			if (rgb) c = SDL_MapRGB (screen->format, rgb[n*3], rgb[n*3+1], rgb[n*3+2]) ;
			else     c = SDL_MapRGB (screen->format, n, n, n) ;
			if (n && c == 0) c = 1 ;
			table[n] = (Uint16) c ;
		}
		table[0] = 0 ;
	}

	dst = bitmap_new (src->code, src->width, src->height, 16, 1) ;
	if (!dst) return src ;

	memcpy (dst->name, src->name, sizeof(dst->name)) ;
	dst->cpoints  = src->cpoints ;
	dst->ncpoints = src->ncpoints ;
	src->cpoints  = NULL ;
	dst->modified = 1 ;

	for (y = 0 ; y < src->height ; y++)
	{
		Uint8  * from = (Uint8 *)src->data + (size_t)src->pitch * y ;
		Uint16 * to   = (Uint16 *)((Uint8 *)dst->data + (size_t)dst->pitch * y) ;

		if (src->depth == 8)
			for (x = 0 ; x < src->width ; x++) to[x] = table[from[x]] ;
		else
		{
			Uint16 white = (Uint16) SDL_MapRGB (screen->format, 255, 255, 255) ;
			for (x = 0 ; x < src->width ; x++) to[x] = (from[x >> 3] & (0x80 >> (x & 7))) ? white : 0 ;
		}
	}

	bitmap_destroy (src) ;
	return dst ;
}
