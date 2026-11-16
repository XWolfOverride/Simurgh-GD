/* Simurgh: modificado 2026-11-15 - asignacion de memoria con xalloc.h (a cero lo que se escribe en el .dcb), correcciones de desbordes, alias height -> z con //$ancient on */
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

/* ---------------------------------------------------------------------- */
/* Este módulo contiene funciones de utilidad para crear e ir rellenando  */
/* varspaces (tablas con identificador, offset y tipo de cada VARIABLE)   */
/* y segmentos de datos                                                   */
/* ---------------------------------------------------------------------- */

FXC_TLS VARSPACE global, local ;

/*
 *  FUNCTION : varspace_dump
 *
 *  Dumps descriptively the variables of a given varspace to the
 *  standard output. Recursively describes struct types.
 *
 *  PARAMS : 
 *      n				Pointer to the varspace
 *		indent			Left indentation in chars (internal; use 0)
 *
 *  RETURN VALUE : 
 *      None
 */

void varspace_dump (VARSPACE * n, int indent)
{
	int i, t, to ;
	char buffer[128] ;

	for (i = 0 ; i < n->count ; i++)
	{
		if (i < n->count-1)
			to = n->vars[i+1].offset - 1 ;
		else
			to = n->last_offset - 1 ;

		printf ("[%04d:%04d]\t", n->vars[i].offset, to) ;
		for (t = 0 ; t < indent ; t++) 
			printf (" + ") ;
		typedef_describe (buffer, n->vars[i].type) ;
		printf ("%s %s", buffer, identifier_name(n->vars[i].code)) ;

		// Describe arrays of structs

		if (typedef_is_array(n->vars[i].type))
		{
			TYPEDEF r = typedef_reduce(n->vars[i].type);
			while (typedef_is_array(r))
				r = typedef_reduce(r);
			if (typedef_is_struct(r))
			{
				printf (":\n") ;
				varspace_dump (typedef_members(r), indent+1) ;
			}
			else
				printf ("\n");
		}

		// Describe structs

		else if (typedef_is_struct(n->vars[i].type))
		{
			printf (":\n") ;
			varspace_dump (typedef_members(n->vars[i].type), indent+1) ;
		}

		else	printf ("\n") ;
	}
}

/*
 *  FUNCTION : varspace_new
 *
 *  Create a new varspace in dynamic memory, and initialize
 *  it using varspace_init
 *
 *  PARAMS : 
 *      None
 *
 *  RETURN VALUE : 
 *      Pointer to the new varspace
 */

VARSPACE * varspace_new ()
{
	VARSPACE * v = (VARSPACE *) xcalloc (1, sizeof(VARSPACE)) ;

	varspace_init (v) ;
	return v ;
}


/*
 *  FUNCTION : varspace_destroy
 *
 *  Destroy a dynamic varspace created with varspace_new
 *
 *  PARAMS : 
 *      v				Pointer to the varspace
 *
 *  RETURN VALUE : 
 *      None
 */

void varspace_destroy (VARSPACE * v)
{
	xfree (v->vars) ;
	xfree (v) ;
}

/*
 *  FUNCTION : varspace_init
 *
 *  Initialize all members of a varspace. Use this function
 *  to initialize a varspace that is created in local
 *	memory instead of a dynamic one created with varspace_new.
 *
 *  PARAMS : 
 *      n				Pointer to the not-initialized varspace
 *
 *  RETURN VALUE : 
 *      None
 */

void varspace_init (VARSPACE * n)
{
	n->vars = (VARIABLE *) xcalloc (16, sizeof(VARIABLE)) ;
	n->reserved = 16 ;
	n->count = 0 ;
	n->size = 0 ;
	n->stringvars = 0 ;
	n->stringvar_reserved = 0 ;
	n->stringvar_count = 0 ;
	if (!n->vars) compile_error ("varspace_init: out of memory\n") ;
}

/*
 *  FUNCTION : varspace_varstring
 *
 *  Add a new string offset to the varspace. A varspace has
 *  a count of every string variable it contains, but this
 *	list is not updated automatically. You should mark
 *	every string you create using this function.
 *
 *  PARAMS : 
 *      n				Pointer to the varspace
 *		offset			Offset of the new string
 *
 *  RETURN VALUE : 
 *      None
 */

void varspace_varstring (VARSPACE * n, int offset)
{
	if (n->stringvar_reserved == n->stringvar_count)
	{
		n->stringvars = (int *) xrealloc (n->stringvars, (n->stringvar_reserved += 16) * sizeof(int)) ;
	}
	n->stringvars[n->stringvar_count++] = offset ;
}

/*
 *  FUNCTION : varspace_alloc
 *
 *  Allocate space for new variables in the varspace. This
 *  is an internal function. Use varspace_add instead.
 *
 *  PARAMS : 
 *      n				Pointer to the varspace
 *		count			Number of new variables of space to reserve
 *
 *  RETURN VALUE : 
 *      None
 */

void varspace_alloc (VARSPACE * n, int count)
{
	int old_reserved = n->reserved ;
	n->reserved += count ;
	n->vars = (VARIABLE *) xrealloc0 (n->vars, sizeof(VARIABLE) * old_reserved, sizeof(VARIABLE) * n->reserved) ;
}

/*
 *  FUNCTION : varspace_add
 *
 *  Add a new variable to a given varspace. This function does not
 *	mark string variables. You should use varspace_varstring after
 *	adding new strings to a varspace.
 *
 *  PARAMS : 
 *      n				Pointer to the varspace
 *		v				Variable to add
 *
 *  RETURN VALUE : 
 *      None
 */

void varspace_add (VARSPACE * n, VARIABLE v)
{
	if (n->count == n->reserved)
		varspace_alloc (n, 16) ;
	n->vars[n->count++] = v ;
	n->size += typedef_size(v.type) ;
}

/*
 *  FUNCTION : varspace_search
 *
 *  Search a variable in a varspace by its identifier.
 *
 *  PARAMS : 
 *      n				Pointer to the varspace
 *		code			Identifier of the variable
 *
 *  RETURN VALUE : 
 *      Pointer to the variable found or NULL if none
 */

VARIABLE * varspace_find (VARSPACE * n, int code)
{
	int i ;

	for (i = 0 ; i < n->count ; i++)
		if (n->vars[i].code == code)
			return &n->vars[i] ;
	return 0 ;
}

/* Con //$ancient on, la variable local "height" (la altura de un proceso del modo 7 en DIV) es un alias de "z": se busca en el
 * espacio de variables locales predefinidas (una variable propia llamada height tiene preferencia). */
VARIABLE * varspace_search (VARSPACE * n, int code)
{
	VARIABLE * v = varspace_find (n, code) ;

	if (!v && fxc_ancient && n == &local)
	{
		/* los codigos de identificador cambian de una pasada de compilacion a otra: no se guardan */
		if (code == identifier_search_or_add ("HEIGHT")) return varspace_find (n, identifier_search_or_add ("Z")) ;
	}
	return v ;
}


void varspace_reset (void)
{
	memset (&global, 0, sizeof(global)) ;
	memset (&local, 0, sizeof(local)) ;
}
