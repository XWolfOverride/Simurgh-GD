/* Simurgh: nuevo 2026-11-15 - carga y grabacion de BMP sin bibliotecas externas */
/*
 *  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  Copyright (C) 2026 XWolfOverride
 */

/*
 * FILE        : img_bmp.c
 * DESCRIPTION : formato BMP de Windows
 *
 * Carga: 1, 4, 8, 16 (555 o mascaras), 24 y 32 bits, sin compresion (BI_RGB y BI_BITFIELDS), de abajo arriba o de arriba abajo.
 *        Los de 32 bits usan el alfa solo si la cabecera es V4 o V5 y trae mascara de alfa. No se admite RLE.
 * Grabacion: siempre 24 bits sin compresion (BMP no guarda bien la transparencia; para conservarla, PNG o TGA).
 */

#include <string.h>
#include <stdlib.h>

#include "fxi.h"

static Uint32 rd16 (const Uint8 * p) { return p[0] | (p[1] << 8) ; }
static Uint32 rd32 (const Uint8 * p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((Uint32)p[3] << 24) ; }

/* Posicion del bit menos significativo y numero de bits de una mascara */
static void mask_info (Uint32 mask, int * shift, int * bits)
{
	*shift = 0 ; *bits = 0 ;
	if (!mask) return ;
	while (!(mask & 1)) { mask >>= 1 ; (*shift)++ ; }
	while (mask & 1)    { mask >>= 1 ; (*bits)++ ; }
}

static int scale8 (Uint32 v, int bits)
{
	if (bits == 0) return 0 ;
	if (bits >= 8) return (v >> (bits - 8)) & 255 ;
	return (v * 255) / ((1 << bits) - 1) ;
}

GRAPH * gr_read_bmp (const char * filename)
{
	int len ;
	Uint8 * buf = gr_slurp_file (filename, &len) ;
	Uint8 * d ;
	GRAPH * gr = NULL ;
	Uint32 offset, hsize, compression, ncolors ;
	int w, h, bpp, topdown = 0, y, x, rowbytes ;
	Uint32 pal[256], masks[4] = { 0, 0, 0, 0 } ;
	int use_alpha = 0 ;
	int sh[4], bi[4] ;

	if (!buf) return NULL ;
	if (len < 26 || buf[0] != 'B' || buf[1] != 'M') goto fail ;

	offset = rd32 (buf + 10) ;
	hsize  = rd32 (buf + 14) ;
	compression = 0 ;
	ncolors = 0 ;

	if (hsize == 12)		/* cabecera OS/2 */
	{
		w = rd16 (buf + 18) ;
		h = rd16 (buf + 20) ;
		bpp = rd16 (buf + 24) ;
	}
	else if (hsize >= 40 && (int) hsize + 14 <= len)
	{
		w = (int) rd32 (buf + 18) ;
		h = (int) rd32 (buf + 22) ;
		bpp = rd16 (buf + 28) ;
		compression = rd32 (buf + 30) ;
		ncolors = rd32 (buf + 46) ;
		if (h < 0) { h = -h ; topdown = 1 ; }
	}
	else goto fail ;

	if (w <= 0 || h <= 0 || w > 8192 || h > 8192) goto fail ;
	if (compression != 0 && compression != 3) goto fail ;		/* BI_RGB y BI_BITFIELDS */
	if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32) goto fail ;

	/* Mascaras de color (16 y 32 bits) */
	if (bpp == 16) { masks[0] = 0x7C00 ; masks[1] = 0x03E0 ; masks[2] = 0x001F ; }
	if (bpp == 32) { masks[0] = 0x00FF0000 ; masks[1] = 0x0000FF00 ; masks[2] = 0x000000FF ; }
	if (compression == 3 && (bpp == 16 || bpp == 32))
	{
		if (hsize >= 52 && (int) hsize + 14 <= len)
		{
			masks[0] = rd32 (buf + 54) ; masks[1] = rd32 (buf + 58) ; masks[2] = rd32 (buf + 62) ;
			if (hsize >= 56) masks[3] = rd32 (buf + 66) ;
		}
		else if (len >= 54 + 12)
		{
			masks[0] = rd32 (buf + 54) ; masks[1] = rd32 (buf + 58) ; masks[2] = rd32 (buf + 62) ;
			if ((int) offset < 54 + 12) goto fail ;
		}
	}
	else if (bpp == 32 && hsize >= 108)
		masks[3] = rd32 (buf + 66) ;
	if (bpp == 32 && masks[3]) use_alpha = 1 ;
	for (x = 0 ; x < 4 ; x++) mask_info (masks[x], &sh[x], &bi[x]) ;

	/* Paleta */
	if (bpp <= 8)
	{
		int psize = (hsize == 12) ? 3 : 4 ;
		int n = ncolors ? (int) ncolors : (1 << bpp) ;
		const Uint8 * p = buf + 14 + hsize ;
		if (n > 256) n = 256 ;
		memset (pal, 0, sizeof(pal)) ;
		if ((p - buf) + n * psize > len) goto fail ;
		for (x = 0 ; x < n ; x++, p += psize)
			pal[x] = (p[2] << 16) | (p[1] << 8) | p[0] ;		/* R,G,B */
	}

	rowbytes = ((w * bpp + 31) / 32) * 4 ;
	if ((Uint32) offset + (Uint32) rowbytes * h > (Uint32) len) goto fail ;

	gr = gr_new_loaded (w, h) ;
	if (!gr) goto fail ;

	for (y = 0 ; y < h ; y++)
	{
		const Uint8 * src = buf + offset + rowbytes * (topdown ? y : h - 1 - y) ;
		Uint16 * dst = (Uint16 *)((Uint8 *)gr->data + gr->pitch * y) ;

		for (x = 0 ; x < w ; x++)
		{
			int r, g, b, a = 255 ;

			if (bpp <= 8)
			{
				int idx ;
				if (bpp == 8) idx = src[x] ;
				else if (bpp == 4) idx = (src[x >> 1] >> ((x & 1) ? 0 : 4)) & 15 ;
				else idx = (src[x >> 3] >> (7 - (x & 7))) & 1 ;
				r = (pal[idx] >> 16) & 255 ; g = (pal[idx] >> 8) & 255 ; b = pal[idx] & 255 ;
			}
			else if (bpp == 24)
			{
				b = src[x * 3] ; g = src[x * 3 + 1] ; r = src[x * 3 + 2] ;
			}
			else
			{
				Uint32 v = (bpp == 16) ? rd16 (src + x * 2) : rd32 (src + x * 4) ;
				r = scale8 ((v & masks[0]) >> sh[0], bi[0]) ;
				g = scale8 ((v & masks[1]) >> sh[1], bi[1]) ;
				b = scale8 ((v & masks[2]) >> sh[2], bi[2]) ;
				if (use_alpha) a = scale8 ((v & masks[3]) >> sh[3], bi[3]) ;
			}
			dst[x] = gr_pack16 (r, g, b, a, use_alpha) ;
			if (use_alpha) gr_alpha_set (gr, x, y, a) ;
		}
	}
	free (buf) ;
	return gr ;

fail:
	free (buf) ;
	if (gr) bitmap_destroy (gr) ;
	return NULL ;
}

static void wr16 (Uint8 * p, Uint32 v) { p[0] = v ; p[1] = v >> 8 ; }
static void wr32 (Uint8 * p, Uint32 v) { p[0] = v ; p[1] = v >> 8 ; p[2] = v >> 16 ; p[3] = v >> 24 ; }

int gr_save_bmp (GRAPH * gr, const char * filename)
{
	file * fp ;
	Uint8 head[54] ;
	Uint8 * rgba, * row ;
	int rowbytes, y, x ;

	if (!gr) return 0 ;
	rowbytes = ((gr->width * 24 + 31) / 32) * 4 ;
	rgba = (Uint8 *) malloc (gr->width * 4) ;
	row  = (Uint8 *) malloc (rowbytes) ;
	if (!rgba || !row) { free (rgba) ; free (row) ; return 0 ; }

	fp = file_open (filename, "wb0") ;	/* sin gzip: ficheros legibles por otras herramientas */
	if (!fp) { free (rgba) ; free (row) ; return 0 ; }

	memset (head, 0, sizeof(head)) ;
	head[0] = 'B' ; head[1] = 'M' ;
	wr32 (head + 2, 54 + rowbytes * gr->height) ;
	wr32 (head + 10, 54) ;
	wr32 (head + 14, 40) ;
	wr32 (head + 18, gr->width) ;
	wr32 (head + 22, gr->height) ;		/* positivo: de abajo arriba */
	wr16 (head + 26, 1) ;
	wr16 (head + 28, 24) ;
	wr32 (head + 34, rowbytes * gr->height) ;
	wr32 (head + 38, 2835) ;			/* 72 ppp */
	wr32 (head + 42, 2835) ;
	file_write (fp, head, 54) ;

	for (y = (int) gr->height - 1 ; y >= 0 ; y--)
	{
		gr_row_to_rgba (gr, y, rgba) ;
		memset (row, 0, rowbytes) ;
		for (x = 0 ; x < (int) gr->width ; x++)
		{
			row[x * 3]     = rgba[x * 4 + 2] ;
			row[x * 3 + 1] = rgba[x * 4 + 1] ;
			row[x * 3 + 2] = rgba[x * 4] ;
		}
		file_write (fp, row, rowbytes) ;
	}
	file_close (fp) ;
	free (rgba) ;
	free (row) ;
	return 1 ;
}
