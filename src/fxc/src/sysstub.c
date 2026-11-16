/* Simurgh: modificado 2026-11-15 - asignacion de memoria con xalloc.h (a cero lo que se escribe en el .dcb), correcciones de desbordes, directiva //$ancient, alias solo con //$ancient on */
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

/* Fast access sysproc list, by identifier code */

FXC_TLS SYSPROC * * sysproc_list = NULL;
FXC_TLS int         sysproc_maxid = 0;

/* Este fichero contiene sólo las definiciones de las funciones del sistema */

#define SYSMACRO(a) 0
#include "sysprocs.h"

/*
 *  FUNCTION : sysproc_add
 *
 *  Add a new system function to the internal system function list
 *
 *  PARAMS:
 *		name			Name of the process
 *		paramtypes		String representation of the parameter 
 *		type			Type of the returning value
 *		func			Pointer to the function itself or a stub
 *
 *  RETURN VALUE: 
 *      Identifier code allocated for the function
 */

/* La tabla de funciones del sistema se modifica al compilar (codigos de identificador, funciones importadas): cada hilo
 * trabaja sobre su propia copia de la original. */
static FXC_TLS SYSPROC * sp_work = 0 ;

static SYSPROC * sp_tab (void)
{
	if (!sp_work)
	{
		sp_work = (SYSPROC *) xmalloc (sizeof(sysprocs)) ;
		memcpy (sp_work, sysprocs, sizeof(sysprocs)) ;
	}
	return sp_work ;
}

static FXC_TLS SYSPROC * sp_last = 0 ;
static FXC_TLS int sp_count = 0 ;

int sysproc_add (char * name, char * paramtypes, int type, void * func)
{

	if (!sp_last)
	{
		sp_last = sp_tab () ;
		sp_count++ ;
		while (sp_last[1].name) sp_last++, sp_count++ ;
	}
	sp_last[1].code = sp_last[0].code + 1 ;
	sp_last[1].name = name ;
	sp_last[1].paramtypes = paramtypes ;
	sp_last[1].params = strcspn(paramtypes, "|") ;
	sp_last[1].type = type ;
	sp_last[1].id   = 0 ;
	sp_last[1].next = NULL ;
	sp_last++ ;
	sp_count++ ;
	if (sp_count == MAX_SYSPROCS)
		compile_error (MSG_TOO_MANY_SYSPROCS) ;

	/* If the fast-access list is already filled, free it to fill it again
	 * in sysproc_get. We should add the new process to the list, but this
	 * is a very rare possibility and we're lazy */

	if (sysproc_list != NULL)
	{
		xfree (sysproc_list);
		sysproc_list = NULL;
		sysproc_maxid = 0;
	}

	return sp_last->code ;
}

/*
 *  FUNCTION : sysproc_get
 *
 *  Search for a system function by name and return a pointer
 *  to the first ocurrence or NULL if none exists
 *
 *  PARAMS:
 *		id			Unique code of the identifier of the name
 *
 *  RETURN VALUE: 
 *      Pointer to the SYSPROC object or NULL if none exists
 */

/*
 *  ALIAS DE FUNCIONES DEL SISTEMA
 *
 *  Para que el codigo escrito para versiones anteriores siga compilando: una llamada a un nombre antiguo (origen) se compila
 *  como una llamada al nombre nuevo (destino). El fichero .dcb solo contiene el destino; el interprete no sabe que hubo alias.
 *  El destino debe aceptar los mismos parametros que el origen. Ver DOC/MIGRACION-FENIX.md.
 */

static const struct { const char * from ; const char * to ; } sysproc_aliases[] =
{
	{ "LOAD_MAP", "LOAD_IMAGE" },
	{ "LOAD_PNG", "LOAD_IMAGE" },
	{ "LOAD_PCX", "LOAD_IMAGE" },
	{ "UNLOAD_PCM", "UNLOAD_WAV" },
	{ 0, 0 }
} ;

/* Devuelve 1 si name es el origen de un alias */
static int sysproc_is_alias (const char * name)
{
	int n ;
	for (n = 0 ; sysproc_aliases[n].from ; n++)
		if (strcmp (sysproc_aliases[n].from, name) == 0) return 1 ;
	return 0 ;
}

/* Destino de un alias (NULL si name no es un alias). Los nombres antiguos (load_map...) solo se aceptan con //$ancient on */

const char * sysproc_alias_target (const char * name)
{
	int n ;
	for (n = 0 ; sysproc_aliases[n].from ; n++)
		if (strcmp (sysproc_aliases[n].from, name) == 0) return sysproc_aliases[n].to ;
	return NULL ;
}

/* Funciones de programas antiguos (DIV) que Simurgh acepta solo como funciones vacias, y solo con //$ancient on */

static const char * ancient_only[] = { "LOAD_PAL", "SET_COLORS", "GET_COLORS", 0 } ;

int sysproc_is_ancient (const char * name)
{
	int n ;
	for (n = 0 ; ancient_only[n] ; n++)
		if (strcmp (ancient_only[n], name) == 0) return 1 ;
	return sysproc_alias_target (name) != NULL ;
}

static int ancient_id (int id)
{
	int n ;
	for (n = 0 ; ancient_only[n] ; n++)
		if (identifier_search_or_add (ancient_only[n]) == id) return 1 ;
	for (n = 0 ; sysproc_aliases[n].from ; n++)
		if (identifier_search_or_add (sysproc_aliases[n].from) == id) return 1 ;
	return 0 ;
}

static SYSPROC * sysproc_get_raw (int id) ;

SYSPROC * sysproc_get (int id)
{
	SYSPROC * s ;

	/* Un alias (load_map...) o una funcion vacia (load_pal...) sin //$ancient on: es como si no existiera */
	if (!fxc_ancient && ancient_id (id)) return NULL ;

	s = sysproc_get_raw (id) ;
	if (s && !fxc_ancient && sysproc_is_ancient (s->name)) return NULL ;
	return s ;
}

static SYSPROC * sysproc_get_raw (int id)
{
	SYSPROC * s ;
	int n ;

	/* If the table is filled, get the process with direct access */

	if (id < sysproc_maxid)
		return sysproc_list[id];

	/* Fill the table */

	if (sysproc_list == NULL)
	{
		/* Alloc IDs if necessary and get the maximum one */

		for (s = sp_tab () ; s->name ; s++)
		{
			if (s->id == 0) 
				s->id = identifier_search_or_add(s->name) ;
			if (s->id > sysproc_maxid)
				sysproc_maxid = s->id;

			s->next = NULL;
		}

		/* Alloc the table */

		sysproc_maxid = ((sysproc_maxid+1) & ~31) + 32;
		sysproc_list  = (SYSPROC **) xcalloc (sysproc_maxid, sizeof(SYSPROC *));

		/* Fill it */

		for (s = sp_tab () ; s->name ; s++)
		{
			if (s->id > 0 && !sysproc_is_alias (s->name))
			{
				s->next = sysproc_list[s->id];
				sysproc_list[s->id] = s;
			}
		}

		/* Los alias comparten la lista de funciones de su destino */

		for (n = 0 ; sysproc_aliases[n].from ; n++)
		{
			int from = identifier_search_or_add (sysproc_aliases[n].from) ;
			int to   = identifier_search_or_add (sysproc_aliases[n].to) ;
			if (from < sysproc_maxid && to < sysproc_maxid)
				sysproc_list[from] = sysproc_list[to] ;
		}

		if (id < sysproc_maxid)
			return sysproc_list[id];
	}
	return NULL;
}

/*
 *  FUNCTION : sysproc_getall
 *
 *  Search for a system function by name and return a pointer
 *  to a zero-terminated table with pointers to all ocurrences.
 *  Up to 31 occurences can be found with this function.
 *
 *  PARAMS:
 *		id			Unique code of the identifier of the name
 *
 *  RETURN VALUE: 
 *      Pointer to a new SYSPROC table allocated with malloc()
 *      NULL if no process with this id exists
 */

SYSPROC * * sysproc_getall (int id)
{
	SYSPROC * s = sysproc_get(id) ;
	SYSPROC * * table;
	int found = 0 ;

	if (s == NULL)
		return NULL;

	table = (SYSPROC **) xcalloc (33, sizeof(SYSPROC *)) ;	/* 32 entradas mas el NULL final */
	do
	{
		table[found++] = s;
		s = s->next;
	}
	while (s && found <= 31);
	table[found] = NULL;
	return table ;
}

/*
 *  FUNCTION : sysproc_name
 *
 *  Return the name of a given system function
 *
 *  PARAMS:
 *		code		Internal code of the function
 *
 *  RETURN VALUE: 
 *      Pointer to the name or NULL if it was not found
 */

char * sysproc_name (int code)
{
	SYSPROC * s = sp_tab () ;

	while (s->name)
	{
		if (s->code == code) return s->name ;
		s++ ;
	}
	return 0 ;
}


/* Vuelve al estado inicial (la memoria la libera xalloc_release_all) */
void sysstub_reset (void)
{
	sp_work = 0 ;
	sp_last = 0 ;
	sp_count = 0 ;
	sysproc_list = NULL ;
	sysproc_maxid = 0 ;
}
