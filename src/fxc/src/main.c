/* Simurgh: modificado 2026-11-15 - sin dependencia de SDL (simtypes.h) */
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

#include <time.h>

#ifdef WIN32
	// NEEDED FOR LOCALE DETECTION
	#include <windows.h>
	#include <windef.h>
	#include <winnls.h>
	#include "Shlwapi.h"
#endif

#include <setjmp.h>
#include "fxc.h"

#define FXC_VERSION "FXC " VERSION " (" __DATE__ " " __TIME__ ")"

#include "messages.h"
#include "errors.h"


/* ---------------------------------------------------------------------- */

FXC_TLS int debug = 0 ;

#define MAX_FILES 64

FXC_TLS int path_file = 5;
FXC_TLS int n_files = 0 ;
FXC_TLS char files[MAX_FILES][256] ;

FXC_TLS char langinfo[64] ;

int load_file (char * filename)
{
#ifdef TARGET_MAC
    char fileMac[256] ;
    snprintf (fileMac, 256, "%s/%s", files[path_file], filename) ;
    snprintf (filename, 256, fileMac);
#endif

	long   size ;
	file * fp = file_open (filename, "rb0") ;
	char * source ;

        if (n_files == MAX_FILES)
                compile_error (MSG_TOO_MANY_FILES) ;
        strcpy (files[n_files++], filename) ;

	if (!fp)
	{
		fprintf (stdout, MSG_FILE_NOT_FOUND "\n", filename) ;
		fxc_fatal (1) ;
	}

	size = file_size (fp) ;

	source = (char *) xmalloc (size+1) ;
	if (size == 0)
	{
		fprintf (stdout, MSG_FILE_EMPTY "\n", filename) ;
		fxc_fatal (1) ;
	}

	if (!file_read (fp, source, size))
	{
		fprintf (stdout, MSG_READ_ERROR "\n", filename) ;
		fxc_fatal (1) ;
	}
	source[size] = 0 ;
	file_close (fp) ;

	token_init (source, n_files-1) ;
	xfree (source) ;	/* token_init hace su propia copia limpia del texto: este buffer ya no hace falta */
        return n_files-1 ;
}

extern int full_screen, double_buffer ;

FXC_TLS int dcb_options = 0 ;

/* Idioma de los mensajes: el de la configuracion regional (EN por defecto) */

static void fxc_set_language (void)
{
	strcpy(langinfo,"EN") ;
#ifdef WIN32
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SABBREVCTRYNAME,langinfo,64) ;
	strlwr(langinfo) ;
#else
	if (getenv("LANG")!=NULL && strlen(getenv("LANG"))>=2)
		strcpy(langinfo,getenv("LANG")) ;
#endif
	langinfo[2] = 0 ;
}

/* Compila el fichero fuente (deja el resultado en las tablas del compilador, sin escribir nada) */

extern void token_reset (void) ;
extern void constants_reset (void) ;
extern void procedure_reset (void) ;
extern void segment_reset (void) ;
extern void string_reset (void) ;
extern void typedef_reset (void) ;
extern void varspace_reset (void) ;
extern void ccode_reset (void) ;
extern void cmain_reset (void) ;
extern void sysstub_reset (void) ;

/* Prepara el estado del compilador para una pasada (tablas de errores, tipos, identificadores, constantes, cadenas...) */
static void fxc_pass_init (void)
{
	memset (&dcb, 0, sizeof(dcb));

	/* build error messages list */
	err_buildErrorTable() ;

	init_c_type() ;
	identifier_init() ;
	constants_init() ;
	string_init () ;
	compile_init () ;
	div_init() ;
}

/* Deja el estado como antes de la pasada, conservando lo que viene de la linea de ordenes (ficheros a incluir en el DCB,
 * directorios de busqueda, opciones) y las firmas leidas. La memoria de la pasada se libera al terminar. */
static void fxc_pass_reset (void)
{
	err_destroyErrorTable () ;
	token_reset () ;
	identifier_init () ;
	constants_reset () ;
	procedure_reset () ;
	segment_reset () ;
	string_reset () ;
	typedef_reset () ;
	varspace_reset () ;
	ccode_reset () ;
	cmain_reset () ;
	sysstub_reset () ;
	n_files = 0 ;
}

/*
 *  Compila el fichero fuente (deja el resultado en las tablas del compilador, sin escribir nada).
 *
 *  Dos pasadas: la primera lee todo el programa (con sus #include y macros) solo para recoger las firmas de procesos y
 *  funciones (c_sigs.c); la segunda es el compilado de verdad, que ya conoce las firmas de todo lo que se llame, este definido
 *  antes o despues.
 */

static void fxc_compile_source (const char * filename)
{
	/* Primera pasada: firmas */
	fxc_pass_init () ;
	load_file ((char *) filename) ;
	sigs_scan () ;
	fxc_pass_reset () ;

	/* Segunda pasada: compilado */
	fxc_pass_init () ;
	load_file ((char *) filename) ;
	compile_program() ;
}

static int fxc_run (int argc, char **argv)
{
	char * filename = 0 ;
	char dcbname[256] ;
	int i, j ;

	printf (FXC_VERSION " - Copyright (C) 2026 XWolfOverride\n") ;
	printf ("Based on Fenix. Copyright (C) 2002 Fenix Team, 1999 Jose Luis Cebrian Pague\n") ;
	printf ("Simurgh comes with ABSOLUTELY NO WARRANTY; see COPYING for details\n\n") ;

	fxc_set_language () ;

	srand (time(NULL)) ;

	/* Get command line parameters.
	 * Opciones: -g (informacion de depurado en el DCB), -c (fuente en caracteres MS-DOS), -i dir (directorio de busqueda),
	 * -f fichero... (incluir ficheros en el DCB). Las letras sin valor se pueden agrupar (-gc); las opciones con valor (-i, -f)
	 * van al final del grupo y su valor siempre va separado por un espacio (-i dir, no -idir). */

	for (i = 1 ; i < argc ; i++)
	{
		if (argv[i][0] == '-')
		{
			for (j = 1 ; argv[i][j] ; j++)
			{
				switch (argv[i][j])
				{
					case 'g':
						dcb_options |= DCB_DEBUG ;
						break ;

					case 'c':
						dos_chars = 1 ;
						break ;

					case 'i':
						/* -i dir: anade un directorio a la ruta de busqueda */
						if (argv[i][j+1] || i == argc-1)
						{
							printf (MSG_DIRECTORY_MISSING "\n") ;
							return 1 ;
						}
						file_addp (argv[++i]) ;
						goto next_arg ;

					case 'f':
						/* -f fichero...: incluye en el DCB los ficheros siguientes (hasta la proxima opcion) */
						if (argv[i][j+1] || i == argc-1 || argv[i+1][0] == '-')
						{
							printf (MSG_FILE_MISSING "\n") ;
							return 1 ;
						}
						while (i+1 < argc && argv[i+1][0] != '-')
							dcb_add_file (argv[++i]) ;
						goto next_arg ;

					default:
						printf (MSG_UNKNOWN_OPTION "\n", argv[i][j]) ;
						printf (MSG_USING MSG_OPTIONS, argv[0]) ;
						return 1 ;
				}
			}
next_arg:	;
		}
		else
		{
			if (filename)
			{
				printf (MSG_TOO_MANY_FILES "\n") ;
				return 1 ;
			}
			filename = argv[i] ;
		}
	}

	if (!filename)
	{
		printf (MSG_USING MSG_OPTIONS, argv[0]) ;
		return 1 ;
	}

	strcpy (dcbname, filename) ;
	if (strrchr(dcbname, '.'))
		*strrchr(dcbname, '.') = 0 ;
	strcat (dcbname, ".dcb") ;

	fxc_compile_source (filename) ;

	dcb_save (dcbname, dcb_options, NULL) ;

	/* destroy error messages list */
	err_destroyErrorTable() ;

	return 0 ;	/* 0 = correcto (Fenix devolvia 1) */
}


/* ---------------------------------------------------------------------- */
/* Punto de entrada como biblioteca                                       */
/* ---------------------------------------------------------------------- */

extern FXC_TLS jmp_buf * fxc_jmp ;

extern void dcbw_reset (void) ;

/* Deja todo el estado del compilador como recien arrancado y libera la memoria */
static void fxc_cleanup (int keep_paths)
{
	sigs_free () ;
	err_destroyErrorTable () ;
	token_reset () ;
	identifier_init () ;
	constants_reset () ;
	dcbw_reset () ;
	procedure_reset () ;
	segment_reset () ;
	string_reset () ;
	typedef_reset () ;
	varspace_reset () ;
	ccode_reset () ;
	cmain_reset () ;
	sysstub_reset () ;
	debug = 0 ;
	path_file = 5 ;
	n_files = 0 ;
	dcb_options = 0 ;
	dos_chars = 0 ;
	if (!keep_paths) file_resetp () ;
	xalloc_release_all () ;
}

int fxc_main (int argc, char ** argv)
{
	jmp_buf jb ;
	jmp_buf * prev = fxc_jmp ;
	int code ;

	fxc_jmp = &jb ;
	code = setjmp (jb) ;
	if (code == 0) code = fxc_run (argc, argv) ;
	fxc_jmp = prev ;
	fxc_cleanup (0) ;
	return code ;
}

/*
 *  Compila un programa en memoria, sin escribir ficheros y sin imprimir nada si va bien (los errores de compilacion si se
 *  imprimen, como siempre). Es lo que usa simurgh para compilar y ejecutar un .prg de una vez.
 *
 *  PARAMS:
 *      filename    fichero fuente (.prg)
 *      options     0 o DCB_DEBUG (guardar informacion de depurado en el DCB)
 *      dos_chars_  1 si el fuente esta en caracteres MS-DOS
 *      embed       ficheros a incluir en el DCB (vector de embed_count nombres) o NULL
 *      data, size  salida: el .dcb (bloque de malloc, lo libera quien llama) y su tamano
 *
 *  Los directorios de busqueda (-i) los pone antes quien llama con file_addp y se conservan.
 *
 *  RETURN VALUE: 0 correcto, 2 error de compilacion (ya impreso), 3 sin memoria o al grabar
 */

int fxc_compile_memory (const char * filename, int options, int dos_chars_, const char ** embed, int embed_count, void ** data, int * size)
{
	jmp_buf jb ;
	jmp_buf * prev = fxc_jmp ;
	int code, n ;

	fxc_jmp = &jb ;
	code = setjmp (jb) ;
	if (code == 0)
	{
		fxc_set_language () ;
		dcb_options = options ;
		dos_chars = dos_chars_ ;
		for (n = 0 ; n < embed_count ; n++)
			dcb_add_file (embed[n]) ;
		fxc_compile_source (filename) ;
		if (!dcb_save_memory (dcb_options, data, size))
			code = 3 ;
	}
	fxc_jmp = prev ;
	fxc_cleanup (1) ;
	return code ;
}
