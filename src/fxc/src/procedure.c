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
#include <string.h>
#include <stdlib.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include "fxc.h"
#include "messages.h"

/* ---------------------------------------------------------------------- */
/* Gestor de procesos y bloques de código. Este módulo contiene funciones */
/* de utilidad para crear procesos y bloques de código así como otras que */
/* se emplean durante y después del compilado.                            */
/* ---------------------------------------------------------------------- */

FXC_TLS PROCDEF * mainproc = 0 ;
FXC_TLS int procdef_count = 0 ;

FXC_TLS int procdef_maxid = -1 ;
FXC_TLS PROCDEF ** procs = 0 ;
FXC_TLS int procs_allocated = 0 ;

int procdef_getid()
{
    return ++procdef_maxid ;
}

PROCDEF * procdef_new (int typeid, int id)
{
    PROCDEF * proc = (PROCDEF *) xcalloc (1, sizeof(PROCDEF)) ;
    int n ;

    proc->pridata = segment_new() ;
    proc->privars = varspace_new() ;

    /* (2006/11/19 23:15 GMT-03:00, Splinter - jj_arg@yahoo.com) */
    proc->pubdata = segment_new() ;
    proc->pubvars = varspace_new() ;
    /* (2006/11/19 23:15 GMT-03:00, Splinter - jj_arg@yahoo.com) */

    proc->params    = -1 ;
    proc->defined   = 0 ;
    proc->declared  = 0 ;
    proc->type      = TYPE_DWORD ;
    proc->flags     = 0 ;

    proc->sentence_count = 0 ;
    proc->sentences      = 0 ;

    if (typeid >= procs_allocated)
    {
        int old_allocated = procs_allocated ;
        procs_allocated = typeid + 15 ;
        procs = (PROCDEF **) xrealloc0 (procs, sizeof(PROCDEF *) * old_allocated, sizeof(PROCDEF *) * procs_allocated) ;
    }
    proc->typeid     = typeid ;
    proc->identifier = id ;
    procs[typeid]    = proc ;

    for (n = 0 ; n < MAX_PARAMS ; n++)
        proc->paramtype[n] = TYPE_UNDEFINED ;

    proc->exitcode  = 0 ;

    codeblock_init (&proc->code) ;
    procdef_count++ ;
    sigs_apply (proc) ;		/* firma leida en la primera pasada, si la hay */
    return proc ;
}

PROCDEF * procdef_search (int id)
{
    int n ;

    for (n = 0 ; n <= procdef_maxid; n++)
    {
        if (procs[n]->identifier == id)
            return procs[n] ;
    }
    return 0 ;
}

PROCDEF * procdef_get (int typeid)
{
    return procs_allocated > typeid ? procs[typeid] : 0 ;
}

void procdef_destroy (PROCDEF * proc)
{
    varspace_destroy (proc->privars) ;
    segment_destroy  (proc->pridata) ;

    /* (2006/11/20 01:09 GMT-03:00, Splinter - jj_arg@yahoo.com) */
    varspace_destroy (proc->pubvars);
    segment_destroy  (proc->pubdata) ;

    procs[proc->typeid] = 0 ;
    xfree (proc->code.data) ;
    xfree (proc->code.loops) ;
    xfree (proc->code.labels) ;
    xfree (proc) ;

    procdef_count-- ;
}



/* ---------------------------------------------------------------------- */
/* Parametros por defecto de procesos y funciones del usuario              */
/* ---------------------------------------------------------------------- */

/*
 *  Un parametro puede declararse con valor por defecto en cualquier posicion (function f(a, b=0, c="x")) y se omite en la
 *  llamada: al final (f(1)) o en medio dejando el hueco (f(1,,3)). Si el proceso ya se conoce al llamarlo, el compilador pone
 *  el valor por defecto en el momento. Si se llama antes de su definicion (lo habitual: el programa principal va primero),
 *  se anota la llamada y se resuelve al terminar de compilar:
 *    - un hueco ya lleva un push de relleno en el codigo: se sustituye por el valor por defecto;
 *    - si faltan parametros al final, la llamada se desvia (salto) a un trozo de codigo anadido al final del bloque que mete
 *      los valores por defecto, hace la llamada y salta de vuelta.
 */

typedef struct
{
    CODEBLOCK * blk ;       /* bloque de codigo que contiene la llamada */
    int         pos ;       /* posicion de la instruccion MN_CALL / MN_PROC */
    PROCDEF   * proc ;
    int         supplied ;  /* argumentos escritos (con los huecos) */
    int         nholes ;
    int         holepos[MAX_PARAMS] ;   /* posicion del push de relleno de cada hueco */
    int         holeidx[MAX_PARAMS] ;   /* numero de parametro de cada hueco */
}
CALLSITE ;

static FXC_TLS CALLSITE * callsites = 0 ;
static FXC_TLS int callsite_count = 0 ;
static FXC_TLS int callsite_alloc = 0 ;

/* Un valor por defecto que es el nombre de una constante se resuelve al usarlo */
static void resolve_default (PROCDEF * proc, int index)
{
    CONSTANT * c ;

    if (proc->paramdef_kind[index] != 4) return ;
    c = constants_search (proc->paramdef_value[index]) ;
    if (!c)
        compile_error (MSG_BAD_DEFAULT, index + 1) ;
    switch (typedef_base (c->type))
    {
        case TYPE_STRING: proc->paramdef_kind[index] = 2 ; break ;
        case TYPE_FLOAT:  proc->paramdef_kind[index] = 3 ; break ;
        default:          proc->paramdef_kind[index] = 1 ; break ;
    }
    proc->paramdef_value[index] = c->value ;
}

static int default_opcode (PROCDEF * proc, int index)
{
    resolve_default (proc, index) ;
    switch (proc->paramdef_kind[index])
    {
        case 2:  return MN_STRING | MN_PUSH ;
        case 3:  return MN_PUSH | MN_FLOAT ;
        default: return MN_PUSH | MN_DWORD ;
    }
}

/* Mete en la pila el valor por defecto del parametro index */
void procdef_push_default (CODEBLOCK * blk, PROCDEF * proc, int index)
{
    int op = default_opcode (proc, index) ;	/* resuelve antes de leer el valor */

    codeblock_add (blk, op, proc->paramdef_value[index]) ;
}

/* Sustituye el push de relleno de la posicion pos por el valor por defecto del parametro index */
void procdef_patch_default (CODEBLOCK * blk, int pos, PROCDEF * proc, int index)
{
    if (blk->data[pos] != (MN_PUSH | MN_DWORD))
        compile_error ("Error interno: hueco de argumento por defecto alterado") ;
    int op = default_opcode (proc, index) ;

    blk->data[pos]   = op ;
    blk->data[pos+1] = proc->paramdef_value[index] ;
}

void procdef_note_call (CODEBLOCK * blk, int pos, PROCDEF * proc, int supplied, int nholes, const int * holepos, const int * holeidx)
{
    CALLSITE * c ;

    if (callsite_count == callsite_alloc)
    {
        int old = callsite_alloc ;
        callsite_alloc += 64 ;
        callsites = (CALLSITE *) xrealloc0 (callsites, sizeof(CALLSITE) * old, sizeof(CALLSITE) * callsite_alloc) ;
    }
    c = &callsites[callsite_count++] ;
    c->blk = blk ;
    c->pos = pos ;
    c->proc = proc ;
    c->supplied = supplied ;
    c->nholes = nholes ;
    memcpy (c->holepos, holepos, sizeof(int) * nholes) ;
    memcpy (c->holeidx, holeidx, sizeof(int) * nholes) ;
}

/* Al ver la definicion (o declaracion) de proc: comprueba las llamadas anotadas antes */
void procdef_check_calls (PROCDEF * proc)
{
    int n, h, k ;

    for (n = 0 ; n < callsite_count ; n++)
    {
        CALLSITE * c = &callsites[n] ;
        if (c->proc != proc) continue ;

        if (c->supplied > proc->params)
            compile_error (MSG_INCORRECT_PARAMC, identifier_name(proc->identifier), proc->params) ;
        for (h = 0 ; h < c->nholes ; h++)
            if (!proc->paramdef_kind[c->holeidx[h]])
                compile_error (MSG_NO_DEFAULT_PARAM, identifier_name(proc->identifier), c->holeidx[h] + 1) ;
        for (k = c->supplied ; k < proc->params ; k++)
            if (!proc->paramdef_kind[k])
                compile_error (MSG_INCORRECT_PARAMC, identifier_name(proc->identifier), proc->params) ;
    }
}

/* Al terminar de compilar: rellena los huecos y crea los desvios de las llamadas a las que les faltan parametros */
void procdef_finish_calls (void)
{
    int n, h, k ;

    for (n = 0 ; n < callsite_count ; n++)
    {
        CALLSITE * c = &callsites[n] ;
        PROCDEF * p = c->proc ;

        if (!p->sig_known) continue ;		/* proceso sin definir: lo avisa codeblock_postprocess */

        for (h = 0 ; h < c->nholes ; h++)
            procdef_patch_default (c->blk, c->holepos[h], p, c->holeidx[h]) ;

        if (c->supplied < p->params)
        {
            /* Los saltos llevan numero de etiqueta (codeblock_postprocess lo cambia por la posicion) */
            int label_thunk = codeblock_label_add (c->blk) ;
            int label_back  = codeblock_label_add (c->blk) ;
            int opcode = c->blk->data[c->pos] ;
            int target = c->blk->data[c->pos + 1] ;

            codeblock_label_set (c->blk, label_thunk, c->blk->current) ;
            for (k = c->supplied ; k < p->params ; k++)
                procdef_push_default (c->blk, p, k) ;
            codeblock_add (c->blk, opcode, target) ;
            codeblock_add (c->blk, MN_JUMP, label_back) ;
            codeblock_label_set (c->blk, label_back, c->pos + 2) ;

            c->blk->data[c->pos]     = MN_JUMP ;
            c->blk->data[c->pos + 1] = label_thunk ;
        }
    }
}

/* Realiza acciones posteriores al compilado sobre el código:
 * - Convierte saltos de código de etiqueta a offset
 * - Convierte identificador de procesos en CALL o TYPE a typeid */

void program_postprocess ()
{
    int n ;

    procdef_finish_calls () ;

    for (n = 0 ; n <= procdef_maxid ; n++)
        codeblock_postprocess (&procs[n]->code) ;
}

void procedure_reset (void)
{
	callsites = 0 ;
	callsite_count = 0 ;
	callsite_alloc = 0 ;
	mainproc = 0 ;
	procdef_count = 0 ;
	procdef_maxid = -1 ;
	procs = 0 ;
	procs_allocated = 0 ;
}
