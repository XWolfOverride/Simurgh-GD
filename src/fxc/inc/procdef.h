/* Simurgh: modificado 2026-11-15 - estado propio de cada hilo (reentrancia de fxc) */
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

#ifndef __PROCDEF_H
#define __PROCDEF_H

#include "xalloc.h"

/* Procesos (un "PROCDEF" es, en realidad, simplemente su definición) */

#ifndef __SEGMENT_H
#include "segment.h"
#endif

#ifndef __VARSPACE_H
#include "varspace.h"
#endif

#ifndef __CODEBLOCK_H
#include "codeblock.h"
#endif

#ifndef __TYPEDEF_H
#include "typedef.h"
#endif

#define MAX_PARAMS 16

#define PROC_USES_FRAME 	0x01
#define PROC_USES_LOCALS	0x02
#define PROC_FUNCTION		0x04
#define PROC_USES_PUBLICS   0x08

typedef struct _sentence
{
	int file ;
	int line ;
	int col ;
	int offset ;
}
SENTENCE ;

typedef struct _procdef
{
	VARSPACE * privars ;
	segment  * pridata ;

    /* (2006/11/19 23:15 GMT-03:00, Splinter - jj_arg@yahoo.com) */
	VARSPACE * pubvars ;
	segment  * pubdata ;
	/* (2006/11/19 23:15 GMT-03:00, Splinter - jj_arg@yahoo.com) */

	int typeid ;
	int identifier ;
	int params ;
	int defined ;
	int declared ;
	int flags ;

	BASETYPE   paramtype[MAX_PARAMS] ;

	/* Valores por defecto de los parametros (en cualquier posicion): 0 = obligatorio, 1 = entero, 2 = cadena, 3 = real */
	int        paramdef_kind[MAX_PARAMS] ;
	int        paramdef_value[MAX_PARAMS] ;	/* valor, codigo de la cadena o bits del real */
	int        sig_known ;			/* 1 cuando ya se ha visto la definicion o la declaracion (params es definitivo) */
	BASETYPE   type ;

	CODEBLOCK  code ;

    int        exitcode;

	SENTENCE   * sentences ;
	int          sentence_count ;
}
PROCDEF ;

extern FXC_TLS int procdef_count ;
extern FXC_TLS int procdef_maxid ;

extern int       procdef_getid() ;
extern PROCDEF * procdef_new (int typeid, int identifier) ;
extern PROCDEF * procdef_get (int typeid) ;
extern PROCDEF * procdef_search (int identifier) ;
extern void      procdef_destroy(PROCDEF *) ;

/* Llamadas a procesos con parametros omitidos (valores por defecto), ver procedure.c */
extern void      procdef_push_default (CODEBLOCK * blk, PROCDEF * proc, int index) ;
extern void      procdef_patch_default (CODEBLOCK * blk, int pos, PROCDEF * proc, int index) ;
extern void      procdef_note_call (CODEBLOCK * blk, int pos, PROCDEF * proc, int supplied, int nholes, const int * holepos, const int * holeidx) ;
extern void      procdef_check_calls (PROCDEF * proc) ;
extern void      procdef_finish_calls (void) ;

/* Primera pasada: firmas de procesos y funciones (c_sigs.c) */
extern void      sigs_scan (void) ;
extern void      sigs_apply (PROCDEF * proc) ;
extern void      sigs_free (void) ;

/* Proceso "principal", el primero en definirse y ejecutarse */
extern FXC_TLS PROCDEF * mainproc ;

#endif
