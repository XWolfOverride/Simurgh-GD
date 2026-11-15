/* Simurgh: nuevo 2026-11-15 - carga y grabacion de TGA sin bibliotecas externas */
/*
 *  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  Copyright (C) 2026 XWolfOverride
 */

/*
 * FILE        : img_tga.c
 * DESCRIPTION : formato TGA (Truevision)
 *
 * Carga: tipos 1, 2 y 3 sin comprimir y 9, 10 y 11 con RLE (paleta, color real y grises); 8 bits (grises o paleta), 15/16, 24 y
 *        32 bits; origen abajo o arriba. El alfa de 32 bits se usa solo si la cabecera declara 8 bits de alfa.
 * Grabacion: color real de 32 bits (BGRA) sin comprimir, origen arriba a la izquierda; el color transparente (0) sale con alfa 0.
 *
 * TGA no tiene firma en la cabecera: la cabecera se valida antes de leer (tipo, profundidad, tamano coherente).
 */

#include <string.h>
#include <stdlib.h>

#include "fxi.h"

static int rd16 (const Uint8 * p) { return p[0] | (p[1] << 8) ; }

GRAPH * gr_read_tga (const char * filename)
{
	int len ;
	Uint8 * buf = gr_slurp_file (filename, &len) ;
	GRAPH * gr = NULL ;
	int idlen, cmaptype, type, cmapfirst, cmaplen, cmapbits, w, h, bpp, desc ;
	int rle, bytes, cmbytes, topdown, alphabits, use_alpha ;
	const Uint8 * p, * cmap = NULL ;
	int x, y, total ;
	Uint8 px[4] ;
	int run = 0, raw = 0 ;

	if (!buf || len < 18) goto fail ;

	idlen = buf[0] ; cmaptype = buf[1] ; type = buf[2] ;
	cmapfirst = rd16 (buf + 3) ; cmaplen = rd16 (buf + 5) ; cmapbits = buf[7] ;
	w = rd16 (buf + 12) ; h = rd16 (buf + 14) ; bpp = buf[16] ; desc = buf[17] ;

	if (type != 1 && type != 2 && type != 3 && type != 9 && type != 10 && type != 11) goto fail ;
	if (cmaptype > 1) goto fail ;
	if ((type == 1 || type == 9) != (cmaptype == 1)) goto fail ;
	rle = type >= 9 ;
	if (w <= 0 || h <= 0 || w > 8192 || h > 8192) goto fail ;

	if (type == 2 || type == 10) { if (bpp != 15 && bpp != 16 && bpp != 24 && bpp != 32) goto fail ; }
	else if (bpp != 8) goto fail ;
	bytes = (bpp + 7) / 8 ;

	cmbytes = 0 ;
	if (cmaptype)
	{
		if (cmapbits != 15 && cmapbits != 16 && cmapbits != 24 && cmapbits != 32) goto fail ;
		cmbytes = (cmapbits + 7) / 8 ;
		if (18 + idlen + cmaplen * cmbytes > len) goto fail ;
		cmap = buf + 18 + idlen ;
	}
	p = buf + 18 + idlen + cmaplen * cmbytes ;

	/* Sin comprimir: el tamano debe cuadrar con el fichero */
	if (!rle && (p - buf) + (long) w * h * bytes > len) goto fail ;

	topdown = (desc & 0x20) != 0 ;
	alphabits = desc & 15 ;
	use_alpha = (bpp == 32 && alphabits == 8) ;

	gr = gr_new_loaded (w, h) ;
	if (!gr) goto fail ;

	total = w * h ;
	x = 0 ; y = 0 ;
	for (y = 0 ; y < h ; y++)
	{
		Uint16 * dst = (Uint16 *)((Uint8 *)gr->data + gr->pitch * (topdown ? y : h - 1 - y)) ;

		for (x = 0 ; x < w ; x++)
		{
			const Uint8 * s ;
			int r, g, b, a = 255, v ;

			if (rle)
			{
				if (run == 0 && raw == 0)
				{
					if (p >= buf + len) goto fail ;
					v = *p++ ;
					if (v & 0x80) { run = (v & 0x7F) + 1 ; if (p + bytes > buf + len) goto fail ; memcpy (px, p, bytes) ; p += bytes ; }
					else raw = v + 1 ;
				}
				if (run > 0) { s = px ; run-- ; }
				else
				{
					if (p + bytes > buf + len) goto fail ;
					s = p ; p += bytes ; raw-- ;
				}
			}
			else
			{
				s = p ; p += bytes ;
			}

			if (type == 1 || type == 9)		/* con paleta */
			{
				int idx = s[0] - cmapfirst ;
				const Uint8 * c ;
				if (idx < 0 || idx >= cmaplen) idx = 0 ;
				c = cmap + idx * cmbytes ;
				if (cmbytes == 3 || cmbytes == 4) { b = c[0] ; g = c[1] ; r = c[2] ; if (cmbytes == 4 && use_alpha) a = c[3] ; }
				else { v = rd16 (c) ; r = ((v >> 10) & 31) << 3 ; g = ((v >> 5) & 31) << 3 ; b = (v & 31) << 3 ; }
			}
			else if (type == 3 || type == 11)	/* grises */
			{
				r = g = b = s[0] ;
			}
			else if (bytes == 2)			/* A1R5G5B5 */
			{
				v = rd16 (s) ;
				r = ((v >> 10) & 31) << 3 ; g = ((v >> 5) & 31) << 3 ; b = (v & 31) << 3 ;
			}
			else					/* 24 y 32 bits: B,G,R,(A) */
			{
				b = s[0] ; g = s[1] ; r = s[2] ;
				if (bytes == 4 && use_alpha) a = s[3] ;
			}
			dst[x] = gr_pack16 (r, g, b, a, use_alpha) ;
			if (use_alpha) gr_alpha_set (gr, x, topdown ? y : h - 1 - y, a) ;
		}
	}
	(void) total ;
	free (buf) ;
	return gr ;

fail:
	free (buf) ;
	if (gr) bitmap_destroy (gr) ;
	return NULL ;
}

int gr_save_tga (GRAPH * gr, const char * filename)
{
	file * fp ;
	Uint8 head[18] ;
	Uint8 * rgba, * row ;
	int y, x ;

	if (!gr || gr->width > 65535 || gr->height > 65535) return 0 ;
	rgba = (Uint8 *) malloc (gr->width * 4) ;
	row  = (Uint8 *) malloc (gr->width * 4) ;
	if (!rgba || !row) { free (rgba) ; free (row) ; return 0 ; }

	fp = file_open (filename, "wb0") ;	/* sin gzip: ficheros legibles por otras herramientas */
	if (!fp) { free (rgba) ; free (row) ; return 0 ; }

	memset (head, 0, sizeof(head)) ;
	head[2]  = 2 ;				/* color real sin comprimir */
	head[12] = gr->width & 255 ; head[13] = gr->width >> 8 ;
	head[14] = gr->height & 255 ; head[15] = gr->height >> 8 ;
	head[16] = 32 ;
	head[17] = 0x28 ;			/* 8 bits de alfa, origen arriba a la izquierda */
	file_write (fp, head, 18) ;

	for (y = 0 ; y < (int) gr->height ; y++)
	{
		gr_row_to_rgba (gr, y, rgba) ;
		for (x = 0 ; x < (int) gr->width ; x++)
		{
			row[x * 4]     = rgba[x * 4 + 2] ;
			row[x * 4 + 1] = rgba[x * 4 + 1] ;
			row[x * 4 + 2] = rgba[x * 4] ;
			row[x * 4 + 3] = rgba[x * 4 + 3] ;
		}
		file_write (fp, row, gr->width * 4) ;
	}
	file_close (fp) ;
	free (rgba) ;
	free (row) ;
	return 1 ;
}
