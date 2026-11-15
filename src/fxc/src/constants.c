/* Simurgh: modificado 2026-11-15 - asignacion de memoria con xalloc.h (a cero lo que se escribe en el .dcb), correcciones de desbordes */
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

#include "fxc.h"

/* ---------------------------------------------------------------------- */
/* Gestor de constantes                                                   */
/* ---------------------------------------------------------------------- */

static FXC_TLS CONSTANT * constants ;
static FXC_TLS int constants_used ;
static FXC_TLS int constants_reserved ;

void constants_init ()
{
	constants = (CONSTANT *) xcalloc (16, sizeof(CONSTANT));
	constants_reserved = 16 ;
	constants_used = 0 ;
}

void constants_alloc (int count)
{
	int old_reserved = constants_reserved ;
	constants_reserved += count ;
	constants = (CONSTANT *) xrealloc0 (constants, old_reserved * sizeof(CONSTANT), constants_reserved * sizeof(CONSTANT)) ;
}

CONSTANT * constants_search (int code)
{
	int i ;

	for (i = 0 ; i < constants_used ; i++)
		if (constants[i].code == code)
			return &constants[i] ;
	return 0 ;
}

void constants_add (int code, TYPEDEF type, int value)
{
	if (constants_used == constants_reserved)
		constants_alloc (16) ;

	constants[constants_used].code = code ;
	constants[constants_used].type = type ;
	constants[constants_used].value = value ;
	constants_used++ ;
}

void constants_dump ()
{
	int i ;

	printf ("---- %d constants of %d ----\n", constants_used, constants_reserved) ;
	for (i = 0 ; i < constants_used ; i++)
		printf ("%4d: %-16s= %d\n", i, 
			identifier_name(constants[i].code), constants[i].value) ;
}

void constants_reset (void)
{
	constants = 0 ;
	constants_used = 0 ;
	constants_reserved = 0 ;
}
