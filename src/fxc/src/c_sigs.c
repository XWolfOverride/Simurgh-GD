/* Simurgh: nuevo 2026-11-15 - primera pasada del compilador: firmas de procesos y funciones */
/*
 *  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  Copyright (C) 2026 XWolfOverride
 */

/*
 * FILE        : c_sigs.c
 * DESCRIPTION : lee todo el programa (incluidos los ficheros #include y las macros, porque usa el mismo analizador de
 *               codigo fuente) buscando las cabeceras "process" y "function" y guarda de cada una su firma: numero de
 *               parametros, tipos, valores por defecto y tipo de retorno. La segunda pasada (el compilado de verdad) parte de
 *               esas firmas, asi que se puede llamar a un proceso o funcion antes de su definicion sin declararlo antes.
 *
 * La primera pasada no compila ni comprueba nada del cuerpo: solo busca el patron
 *
 *      process|function [signed|unsigned] [tipo] [*] nombre ( [tipo [*]] parametro [= valor] , ... )
 *
 * Los valores por defecto pueden ser un numero, un real, una cadena (con signo menos delante si es numero) o el nombre de una
 * constante (se resuelve al usarlo en la segunda pasada). Los tipos de parametro que no se escriben (el parametro toma el de una
 * variable local o global del mismo nombre, o es entero) se dejan sin definir.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fxc.h"
#include "messages.h"

extern FXC_TLS int identifier_process ;

typedef struct
{
	char *   name ;
	int      params ;
	BASETYPE type ;				/* tipo de retorno (TYPE_UNDEFINED = no se escribio) */
	BASETYPE paramtype[MAX_PARAMS] ;	/* TYPE_UNDEFINED = no se escribio */
	int      kind[MAX_PARAMS] ;		/* valor por defecto: 0 ninguno, 1 entero, 2 cadena, 3 real, 4 nombre de constante */
	int      ival[MAX_PARAMS] ;		/* entero o bits del real */
	char *   sval[MAX_PARAMS] ;		/* texto de la cadena o nombre de la constante */
}
SIG ;

static FXC_TLS SIG * sigs = 0 ;
static FXC_TLS int sig_count = 0 ;
static FXC_TLS int sig_alloc = 0 ;

void sigs_free (void)
{
	int n, i ;

	for (n = 0 ; n < sig_count ; n++)
	{
		free (sigs[n].name) ;
		for (i = 0 ; i < MAX_PARAMS ; i++) free (sigs[n].sval[i]) ;
	}
	free (sigs) ;
	sigs = 0 ;
	sig_count = sig_alloc = 0 ;
}

static SIG * sig_find (const char * name)
{
	int n ;
	for (n = 0 ; n < sig_count ; n++)
		if (strcmp (sigs[n].name, name) == 0) return &sigs[n] ;
	return 0 ;
}

/* Tipo basico que representa la palabra reservada de tipo code (TYPE_UNDEFINED si no es un tipo) */
static BASETYPE type_word (int code, int is_signed, int is_unsigned)
{
	if (code == identifier_dword)  return is_signed ? TYPE_INT : TYPE_DWORD ;
	if (code == identifier_int)    return is_unsigned ? TYPE_DWORD : TYPE_INT ;
	if (code == identifier_word)   return is_signed ? TYPE_SHORT : TYPE_WORD ;
	if (code == identifier_short)  return is_unsigned ? TYPE_WORD : TYPE_SHORT ;
	if (code == identifier_byte)   return is_signed ? TYPE_SBYTE : TYPE_BYTE ;
	if (code == identifier_char)   return TYPE_CHAR ;
	if (code == identifier_string) return TYPE_STRING ;
	if (code == identifier_float)  return TYPE_FLOAT ;
	return TYPE_UNDEFINED ;
}

static int is_punct (int code)
{
	return token.type == IDENTIFIER && token.code == code ;
}

static int is_separator (void)
{
	return is_punct (identifier_comma) || is_punct (identifier_rightp) || is_punct (identifier_equal) ;
}

/* Lee los parametros (el '(' ya esta leido) hasta el ')'. Devuelve 1 si la cabecera es valida. */
static int scan_params (SIG * s)
{
	int n = 0 ;

	token_next () ;
	if (is_punct (identifier_rightp)) return 1 ;		/* sin parametros */

	for (;;)
	{
		int is_signed = 0, is_unsigned = 0, name_seen = 0 ;
		BASETYPE type = TYPE_UNDEFINED ;

		if (n >= MAX_PARAMS) return 0 ;

		/* [signed|unsigned] [tipo | nombre de struct] [*]... nombre */
		while (!name_seen)
		{
			if (token.type != IDENTIFIER) return 0 ;
			if (token.code == identifier_signed)   { is_signed = 1 ;   token_next () ; continue ; }
			if (token.code == identifier_unsigned) { is_unsigned = 1 ; token_next () ; continue ; }
			if (token.code == identifier_pointer || token.code == identifier_multiply)
			{
				type = TYPE_POINTER ;
				token_next () ;
				continue ;
			}
			if (type_word (token.code, 0, 0) != TYPE_UNDEFINED)
			{
				/* palabra de tipo */
				if (type != TYPE_POINTER) type = type_word (token.code, is_signed, is_unsigned) ;
				token_next () ;
				continue ;
			}
			if (token.code < reserved_words) return 0 ;

			/* Un identificador: es el nombre del parametro, o el nombre de un tipo struct si detras viene otro nombre */
			token_next () ;
			if (is_separator ())
				name_seen = 1 ;
			else
				type = TYPE_POINTER ;		/* era un tipo struct: el parametro es un puntero */
		}

		if (type != TYPE_UNDEFINED) s->paramtype[n] = type ;

		/* valor por defecto */
		if (is_punct (identifier_equal))
		{
			int neg = 0 ;

			token_next () ;
			if (is_punct (identifier_minus)) { neg = 1 ; token_next () ; }
			if (token.type == NUMBER)
			{
				s->kind[n] = 1 ;
				s->ival[n] = neg ? -token.code : token.code ;
			}
			else if (token.type == FLOAT)
			{
				float f = neg ? -token.value : token.value ;
				s->kind[n] = 3 ;
				s->ival[n] = *(int *) &f ;
			}
			else if (token.type == STRING && !neg)
			{
				s->kind[n] = 2 ;
				s->sval[n] = strdup (string_get (token.code)) ;
			}
			else if (token.type == IDENTIFIER && token.code >= reserved_words && !neg)
			{
				s->kind[n] = 4 ;
				s->sval[n] = strdup (identifier_name (token.code)) ;
			}
			else return 0 ;
			token_next () ;
		}
		n++ ;

		if (is_punct (identifier_comma)) { token_next () ; continue ; }
		if (is_punct (identifier_rightp)) break ;
		return 0 ;
	}
	s->params = n ;
	return 1 ;
}

/* Cabecera: la palabra process/function ya esta leida */
static void scan_header (void)
{
	SIG s ;
	int is_signed = 0, is_unsigned = 0, last_name = -1 ;
	BASETYPE ret = TYPE_UNDEFINED ;
	int n ;

	memset (&s, 0, sizeof(s)) ;
	for (n = 0 ; n < MAX_PARAMS ; n++) s.paramtype[n] = TYPE_UNDEFINED ;

	/* [signed|unsigned] [tipo] [*] nombre ( */
	for (;;)
	{
		token_next () ;
		if (token.type != IDENTIFIER) return ;
		if (token.code == identifier_leftp) break ;
		if (token.code == identifier_signed)   { is_signed = 1 ;   continue ; }
		if (token.code == identifier_unsigned) { is_unsigned = 1 ; continue ; }
		if (token.code == identifier_pointer || token.code == identifier_multiply) { ret = TYPE_POINTER ; continue ; }
		if (type_word (token.code, 0, 0) != TYPE_UNDEFINED)
		{
			if (ret != TYPE_POINTER) ret = type_word (token.code, is_signed, is_unsigned) ;
			continue ;
		}
		if (token.code < reserved_words) return ;
		/* un nombre: o el del proceso o el de un tipo struct de retorno (si despues viene otro nombre) */
		if (last_name >= 0) ret = TYPE_POINTER ;
		last_name = token.code ;
	}
	if (last_name < 0) return ;

	s.name = strdup (identifier_name (last_name)) ;
	s.type = ret ;
	if (!scan_params (&s) || sig_find (s.name))
	{
		free (s.name) ;
		for (n = 0 ; n < MAX_PARAMS ; n++) free (s.sval[n]) ;
		return ;
	}

	if (sig_count == sig_alloc)
	{
		sig_alloc += 32 ;
		sigs = (SIG *) realloc (sigs, sizeof(SIG) * sig_alloc) ;
		if (!sigs) fxc_out_of_memory ("firmas") ;
	}
	sigs[sig_count++] = s ;
}

/* Recorre todo el codigo fuente buscando cabeceras de procesos y funciones */
void sigs_scan (void)
{
	for (;;)
	{
		token_next () ;
		if (token.type == NOTOKEN) break ;
		if (token.type == IDENTIFIER && (token.code == identifier_process || token.code == identifier_function))
		{
			scan_header () ;
			/* scan_header deja sin consumir el ultimo token leido: se vuelve a mirar por si fuera otra cabecera */
			token_back () ;
		}
	}
}

/* Aplica a un proceso recien creado la firma que se leyo en la primera pasada (si la hay) */
void sigs_apply (PROCDEF * proc)
{
	SIG * s = sigs ? sig_find (identifier_name (proc->identifier)) : 0 ;
	int n ;

	if (!s) return ;
	proc->params = s->params ;
	proc->sig_known = 1 ;
	if (s->type != TYPE_UNDEFINED) proc->type = s->type ;
	for (n = 0 ; n < s->params ; n++)
	{
		proc->paramtype[n] = s->paramtype[n] ;
		proc->paramdef_kind[n] = s->kind[n] ;
		switch (s->kind[n])
		{
			case 2: proc->paramdef_value[n] = string_new (s->sval[n]) ; break ;
			case 4: proc->paramdef_value[n] = identifier_search_or_add (s->sval[n]) ; break ;
			default: proc->paramdef_value[n] = s->ival[n] ; break ;
		}
	}
}
