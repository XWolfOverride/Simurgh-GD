/* Simurgh: nuevo 2026-11-15 - asignacion de memoria con lista de bloques (ver xalloc.h) */
/*
 *  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  Copyright (C) 2026 XWolfOverride
 */

#include <stdlib.h>
#include <string.h>

#include "fxc.h"

/* Cabecera de cada bloque: lista doble. Se rellena hasta 16 bytes para no romper la alineacion de lo que sigue. */
typedef union xblock
{
	struct { union xblock * prev, * next ; } l ;
	char pad[16 > 2*sizeof(void *) ? 16 : 2*sizeof(void *)] ;
}
xblock ;

static FXC_TLS xblock * xlist = 0 ;

static void * xlink (xblock * b, size_t size)
{
	(void) size ;
	b->l.prev = 0 ;
	b->l.next = xlist ;
	if (xlist) xlist->l.prev = b ;
	xlist = b ;
	return (void *)(b + 1) ;
}

static void xunlink (xblock * b)
{
	if (b->l.prev) b->l.prev->l.next = b->l.next ; else xlist = b->l.next ;
	if (b->l.next) b->l.next->l.prev = b->l.prev ;
}

void * xmalloc (size_t size)
{
	xblock * b = (xblock *) malloc (sizeof(xblock) + (size ? size : 1)) ;
	if (!b) fxc_out_of_memory ("malloc") ;
	return xlink (b, size) ;
}

void * xcalloc (size_t count, size_t size)
{
	size_t n ;
	void * p ;
	if (!count) count = 1 ;
	if (!size) size = 1 ;
	if (count > ((size_t)-1 - sizeof(xblock)) / size) fxc_out_of_memory ("calloc") ;
	n = count * size ;
	p = xmalloc (n) ;
	memset (p, 0, n) ;
	return p ;
}

void * xrealloc (void * ptr, size_t size)
{
	xblock * b, * nb ;

	if (!ptr) return xmalloc (size) ;
	b = ((xblock *) ptr) - 1 ;
	xunlink (b) ;
	nb = (xblock *) realloc (b, sizeof(xblock) + (size ? size : 1)) ;
	if (!nb)
	{
		xlink (b, 0) ;		/* el bloque original sigue vivo: que lo libere la limpieza general */
		fxc_out_of_memory ("realloc") ;
	}
	return xlink (nb, size) ;
}

void * xrealloc0 (void * ptr, size_t old_size, size_t new_size)
{
	char * p = (char *) xrealloc (ptr, new_size) ;
	if (new_size > old_size) memset (p + old_size, 0, new_size - old_size) ;
	return p ;
}

char * xstrdup (const char * s)
{
	size_t n = strlen (s) + 1 ;
	return (char *) memcpy (xmalloc (n), s, n) ;
}

void xfree (void * ptr)
{
	xblock * b ;
	if (!ptr) return ;
	b = ((xblock *) ptr) - 1 ;
	xunlink (b) ;
	free (b) ;
}

void xalloc_release_all (void)
{
	while (xlist)
	{
		xblock * b = xlist ;
		xlist = b->l.next ;
		free (b) ;
	}
}
