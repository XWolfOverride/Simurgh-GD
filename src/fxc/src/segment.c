/* Simurgh: modificado 2026-11-15 - memoria a cero antes de escribir el .dcb (salida reproducible) */
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
#include <string.h>

#include "fxc.h"
#include "messages.h"

static FXC_TLS int max_id = 0 ;
static FXC_TLS int free_id[1024] ;
static FXC_TLS int free_count = 0 ;

static FXC_TLS segment * * segments = 0 ;
static FXC_TLS int segments_reserved = 0 ;

FXC_TLS segment * globaldata, * localdata ;

static void segment_register (segment * s)
{
	/* Saves the segment in the global array */

	if (!segments)
	{
		segments = (segment **) xcalloc (16, sizeof(segment *)) ;
		segments_reserved = 16 ;
	}
	if (segments_reserved <= s->id)
	{
		int old_reserved = segments_reserved ;
		segments_reserved = s->id + 16 ;
		segments = (segment **) xrealloc0 (segments, sizeof(segment *) * old_reserved, sizeof(segment *) * segments_reserved) ;
	}
	segments[s->id] = s ;
}

segment * segment_new ()
{
	/* Creates the segment */

	segment * s = (segment *) xcalloc (1, sizeof(segment)) ;

	if (free_count)
		s->id = free_id[--free_count] ;
	else
		s->id = max_id++ ;

	s->current = 0 ;
	s->reserved = 128 ;

	s->bytes = (int *) xcalloc (1, 128) ;

	segment_register (s) ;
	return s ;
}

segment * segment_duplicate (segment * b)
{
	segment * s = (segment *) xcalloc (1, sizeof(segment)) ;

	if (free_count)
		s->id = free_id[--free_count] ;
	else
		s->id = max_id++ ;

	s->current = b->current ;
	s->reserved = b->reserved ;

	s->bytes = (int *) xcalloc (1, s->reserved) ;
	memcpy (s->bytes, b->bytes, s->current) ;

	segment_register (s) ;
	return s ;
}

void segment_destroy (segment * s)
{
	segments[s->id] = 0 ;
	if (free_count < 1024) free_id[free_count++] = s->id ;

	xfree (s->bytes) ;
	xfree (s) ;
}

void segment_alloc (segment * n, int count)
{
	int old_reserved = n->reserved ;
	n->reserved += count ;
	n->bytes = (int *) xrealloc0 (n->bytes, old_reserved, n->reserved) ;
}

int segment_add_as (segment * n, Sint32 value, BASETYPE t)
{
	switch (t)
	{
		case TYPE_DWORD:
		case TYPE_INT:
		case TYPE_FLOAT:
		case TYPE_STRING:
		case TYPE_POINTER:
			return segment_add_dword (n, (Sint32)value) ;
		case TYPE_WORD:
		case TYPE_SHORT:
			return segment_add_word (n, (Sint16)value) ;
		case TYPE_BYTE:
		case TYPE_SBYTE:
		case TYPE_CHAR:
			return segment_add_byte (n, (Sint8)value) ;

		default:
			compile_error (MSG_INCOMP_TYPE) ;
			return 0 ;
	}
}

int segment_add_from (segment * n, segment * s)
{
	if (n->current+s->current >= n->reserved)
		segment_alloc (n, s->current) ;
	memcpy ((Uint8 *)n->bytes + n->current, s->bytes, s->current) ;
	return n->current += s->current ;
}

int segment_add_byte (segment * n, Sint8 value)
{
	if (n->current+1 >= n->reserved)
		segment_alloc (n, 64) ;
	*((Sint8 *)n->bytes + n->current) = value ;
	return n->current ++ ;
}

int segment_add_word (segment * n, Sint16 value)
{
	if (n->current+2 >= n->reserved)
		segment_alloc (n, 64) ;
	*(Sint16 *)((Uint8 *)n->bytes + n->current) = value ;
	n->current += 2 ;
	return n->current - 2 ;
}

int segment_add_dword (segment * n, Sint32 value)
{
	if (n->current+4 >= n->reserved)
		segment_alloc (n, 64) ;
	*(Sint32 *)((Uint8 *)n->bytes + n->current) = value ;
	n->current += 4 ;
	return n->current - 4 ;
}

segment * segment_get (int id)
{
	return segments[id] ;
}

void segment_dump (segment * s)
{
	int i ;

	for (i = 0 ; i < s->current ; i++)
		printf ("%02X  ", *((Uint8*)s->bytes + i)) ;
	printf ("\n") ;
}

void segment_copy(segment *s, int base_offset, int total_length) 
{
	if (s->reserved < s->current + total_length)
		segment_alloc (s, total_length) ;
	memcpy ((Uint8 *)s->bytes + s->current,
	        (Uint8 *)s->bytes + base_offset, total_length) ;
	s->current += total_length ;
}

VARIABLE * variable_new ()
{
	VARIABLE * v = (VARIABLE *) xcalloc (1, sizeof(VARIABLE)) ;

	return v;
}

/* Segmentos nombrados */

#define MAX_NAMED 64
static FXC_TLS segment * named_segs[MAX_NAMED] ;
static FXC_TLS int       named_codes[MAX_NAMED] ;
static FXC_TLS int       named_count = 0 ;

segment * segment_by_name (int code)
{
	int n ;

	for (n = 0 ; n < named_count ; n++)
		if (named_codes[n] == code) 
			return named_segs[n] ;
	return 0 ;
}

void segment_name (segment * s, int code)
{
	if (named_count == MAX_NAMED)
		compile_error (MSG_TOO_MANY_TYPES) ;
	named_segs[named_count] = s ;
	named_codes[named_count] = code ;
	named_count++ ;
}

void segment_reset (void)
{
	max_id = 0 ;
	free_count = 0 ;
	memset (free_id, 0, sizeof(free_id)) ;
	segments = 0 ;
	segments_reserved = 0 ;
	globaldata = localdata = 0 ;
	memset (named_segs, 0, sizeof(named_segs)) ;
	memset (named_codes, 0, sizeof(named_codes)) ;
	named_count = 0 ;
}
