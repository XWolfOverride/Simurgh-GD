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
#include <string.h>
#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif
#include <setjmp.h>
#include "fxc.h"
#include "errors.h"
#include "files.h"

/* Errors indexed table */
FXC_TLS struct _errindex errtable[10] ;

/* Destino del salto de fxc_fatal (lo fija fxc_main). */
FXC_TLS jmp_buf * fxc_jmp = 0 ;

void fxc_fatal (int code)
{
	if (fxc_jmp) longjmp (*fxc_jmp, code) ;
	exit (code) ;
}


/*
 *  Generate & populate the error table
 */

void err_buildErrorTable() {

	file * fp ;
	char   fname[64] ;
	char   line[1024] ;
	char * token ;
	int	   code ;
	int	   size ;
	char * msg = NULL ;
	struct _errmsg * err = NULL ;

	/* open the desired localized error file, check for existence if not open EN default */
	strcpy(fname,"msg/") ;
	strcat(fname,langinfo) ;
	strcat(fname,".msg") ;

	if (!file_exists(fname))
		strcpy(fname,"msg/en.msg") ;
	fp = file_open(fname,"r") ;

	if (fp) {
		while (file_gets (fp,line,1024)) {
			// Check for multiline...
			if (line[0]=='_') {
				// must resize the string and concatenate
				if (err) {
					size = strlen(err->msg) + strlen(line) + 1 ;
					msg = (char *) xmalloc (size) ;
					strcpy(msg,err->msg) ;
					line[0]='\n' ;
					strcat(msg,line) ;
					xfree (err->msg) ;
					err->msg = msg ;	/* msg ya es una copia propia: no hace falta duplicarla (antes se perdia) */
				}
			} else {
				// Check for comment
				token = strtok(line,"#") ;
				code = 0 ;
				msg = "" ;		/* una linea sin texto no debe usar el de la anterior */
				while (token!=NULL) {
					/* don't process comments */
					if (strlen(token)==0) break ;
					if (!code) code = atoi(token) ;
					else msg = token ;
					token = strtok(NULL,"#") ;
					}
				if (code) err = err_addError(code,msg) ;
				}
			}
		file_close(fp) ;
	}
}

struct _errmsg * err_addError(int code, const char * msg) {

	int idx ;
	struct _errmsg * err ;
	struct _errmsg * cerr ;
	/* build error object */

	err = (struct _errmsg *) xcalloc (1, sizeof(struct _errmsg)) ;
	{
		err->code = code ;
		err->msg  = xstrdup (msg) ;
		err->prev = NULL ;
		err->next = NULL ;

		/* determine index to use... */
		idx = code/100 ;

		/* find insertion point */

		cerr = errtable[idx].errlist ;

		if (cerr) {
			while (cerr->next) cerr = cerr->next ;
			cerr->next = err ;
			err->prev = cerr ;
		} else {
			errtable[idx].errlist = err ;
			}

		errtable[idx].count++ ;
		}
	return err ;

}

void err_delError(struct _errmsg * err) {

	int idx ;

	if (!err) return ;

	idx = err->code/100 ;

	if (err->next) {
		err->next->prev = err->prev ;
		}

	if (err->prev)
		err->prev->next = err->next ;
	else
		errtable[idx].errlist = err->next ;

	xfree (err->msg) ;
	xfree (err) ;

	errtable[idx].count-- ;


	}

void err_destroyErrorTable(void) {

	int i ;

	for (i=0;i<10;i++) {
		while (errtable[i].count>0)  {
			err_delError(errtable[i].errlist) ;
		}
	}
	memset (errtable, 0, sizeof(errtable)) ;
}

const char * err_getErrorByCode(int code) {

	int idx ;
	struct _errmsg * err ;

	idx = code/100 ;

	err = errtable[idx].errlist ;

	while (err && err->code!=code) err = err->next ;


	if (!err) return 0 ;
	else return err->msg ;

	}



/* Memoria agotada (la usan xmalloc y similares, ver xalloc.h). No vuelve. */
void fxc_out_of_memory (const char * where)
{
	fprintf (stdout, "fxc: sin memoria (%s)\n", where) ;
	fxc_fatal (2) ;
}
