/* Simurgh: modificado 2026-11-15 - linea de ordenes unificada (simurgh), compilacion en memoria, ficheros en memoria */
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

/*
 * FILE        : main.c
 * DESCRIPTION : Main entry point for FXI
 *
 * HISTORY:	0.81 - Removed -w option
 *
 */

/*
 * INCLUDES
 */

#ifdef TARGET_BEOS
#include <posix/assert.h>
#else
#include <assert.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fxi.h"
#include "font.h"
#include "dcb.h"

/* os versions */
#ifdef WIN32
	#define OS_ID 0
#endif
#ifdef TARGET_Linux
	#define OS_ID 1
#endif
#ifdef TARGET_BEOS
	#define OS_ID 2
#endif
#ifdef TARGET_MAC
	#define OS_ID 3
#endif
#ifdef TARGET_GP32
	#define OS_ID 4
#endif
#ifdef TARGET_DC
	#define OS_ID 5
#endif
#ifdef TARGET_BSD
	#define OS_ID 6
#endif
#ifdef WIN32
#include <windows.h>
#endif

#ifdef SIMURGH
#include "fxc_api.h"
#endif

#define DCB_MAGIC	"DCB Stub\x1A\x0D\x0A"
/*
 *  GLOBAL VARIABLES
 */

int debug	 = 0;		/* 1 if running in debug mode					 */
int fxi		 = 0;		/* 1 only if this is an standalone interpreter   */
int embedded = 0;		/* 1 only if this is a stub with an embedded DCB */

#ifdef TARGET_MAC
    int current_file = 0;
    char files[][256];
#endif

extern int full_screen, double_buffer ;
extern char *apptitle;


/*
 *  FUNCTION : do_exit
 *
 *  Exits from the program cleanly ending operations
 *
 *  PARAMS:
 *      INT n: ERROR LEVEL to return to OS
 *
 *  RETURN VALUE:
 *      No value
 *
 */

void
do_exit(int n)
{
    int i;

	if (keytab_initialized) keytab_free() ;

	for (i=0;i<MAX_JOYS;i++)
	    if (_joysticks[i]!=NULL) SDL_JoystickClose(_joysticks[i]) ;

	SDL_Quit() ;
	exit(n) ;
}

/*
 *  FUNCTION : main
 *
 *  Main function for FXI
 *
 *  PARAMS:
 *      INT n: ERROR LEVEL to return to OS
 *
 *  RETURN VALUE:
 *      No value
 *
 */

int main (int argc, char **argv)
{
	char *				filename = 0 ;
	char				dcbname[256] ;
	INSTANCE *			mainproc_running ;
	int					i, j ;
	int					norun = 0 ;
	const SDL_version * sdl_version ;
	char *				ptr ;
	file *				fp = NULL;
	void *				memdcb = NULL ;		/* DCB compilado en memoria (.prg) */
	int					memdcb_size = 0 ;

	struct
	{
		char	magic[12];
		int		dcb_offset;
	}
	dcb_signature;

	/* Find out if we are calling fxi.exe or whatever.exe */

 	ptr = argv[0] + strlen(argv[0]) ;
 	while (ptr > argv[0] && ptr[-1] != '\\' && ptr[-1] != '/')
 		ptr-- ;
 	/* Programa empotrado: si el propio ejecutable lleva un DCB pegado al final, se ejecuta ese. Si no, es el
	 * interprete normal. No depende del nombre del ejecutable. */
	{
		const char * self = argv[0] ;
#ifdef WIN32
		char exepath[MAX_PATH] ;
		GetModuleFileName (NULL, exepath, sizeof(exepath)) ;
		self = exepath ;
#endif
		if (file_exists (self))
			fp = file_open (self, "rb0") ;
		if (fp != NULL)
		{
			file_seek (fp, -(int)sizeof(dcb_signature), SEEK_END) ;
			file_read (fp, &dcb_signature, sizeof(dcb_signature)) ;
			if (strcmp (dcb_signature.magic, DCB_MAGIC) == 0)
			{
				ARRANGE_DWORD (&dcb_signature.dcb_offset) ;
				filename = argv[0] ;
				embedded = 1 ;
			}
			else
			{
				file_close (fp) ;
				fp = NULL ;
			}
		}
	}
	fxi = !embedded ;


	/*
	 *  Linea de ordenes de simurgh (interprete fxi):
	 *
	 *      simurgh [opciones] programa.dcb [argumentos del programa]    ejecuta un DCB
	 *      simurgh [opciones] programa.prg [argumentos del programa]    compila en memoria y, si va bien, ejecuta
	 *      simurgh -b [opciones] programa.prg                           solo compila: escribe programa.dcb
	 *
	 *  Opciones: -d depuracion; -i dir directorio de busqueda de ficheros; y para compilar -g (informacion de depurado en el
	 *  DCB), -c (fuente en caracteres MS-DOS) y -f fichero... (ficheros a incluir en el DCB). Las letras sin valor se pueden
	 *  agrupar (-dg); las opciones con valor (-i, -f) llevan el valor separado por un espacio. Lo que va despues del fichero
	 *  son argumentos del programa y no se interpreta. Al compilar para ejecutar no se imprime nada si todo va bien.
	 */

	if (fxi)
	{
		int build_only = 0, compile_debug = 0, compile_dos = 0, nembed = 0 ;
		const char * embed[64] ;
		char * a ;

		for (i = 1 ; i < argc && !filename ; i++)
		{
			if (argv[i][0] != '-')
			{
				filename = argv[i] ;
				argc-- ;
				memmove (&argv[i], &argv[i+1], sizeof(char*) * (argc-i+1)) ;
				break ;
			}
			for (j = 1 ; argv[i][j] ; j++)
			{
				switch (argv[i][j])
				{
					case 'd': debug = 1 ; break ;
					case 'b': build_only = 1 ; break ;
					case 'g': compile_debug = 1 ; break ;
					case 'c': compile_dos = 1 ; break ;
					case 'i':
						if (argv[i][j+1] || i == argc-1)
						{
							printf ("Falta el directorio de -i (se escribe separado: -i dir)\n") ;
							return 1 ;
						}
						file_addp (argv[++i]) ;
						goto next_option ;
					case 'f':
						if (argv[i][j+1] || i == argc-1 || argv[i+1][0] == '-')
						{
							printf ("Falta el fichero de -f (se escribe separado: -f fichero)\n") ;
							return 1 ;
						}
						while (i+1 < argc && argv[i+1][0] != '-' && nembed < 64)
							embed[nembed++] = argv[++i] ;
						goto next_option ;
					default:
						printf ("Opcion desconocida -%c\n\n", argv[i][j]) ;
						filename = NULL ;
						i = argc ;
						goto usage ;
				}
			}
next_option:	;
		}

usage:
		if (!filename)
		{
			printf ("Simurgh (" FXI_VERSION ")\n"
				"Copyright (C) 2026 XWolfOverride\n"
				"Based on Fenix. Copyright (C) 2002 Fenix Team, 1999 Jose Luis Cebrian Pague\n"
				"Simurgh comes with ABSOLUTELY NO WARRANTY; see COPYING for details\n\n"
				"Uso:\n"
				"   %s [opciones] programa.dcb [argumentos]   ejecuta un programa compilado\n"
				"   %s [opciones] programa.prg [argumentos]   compila en memoria y lo ejecuta\n"
				"   %s -b [opciones] programa.prg             solo compila (escribe programa.dcb)\n\n"
				"Opciones (el valor va separado por un espacio):\n"
				"   -d         Modo depuracion\n"
				"   -i dir     Anade el directorio a la ruta de busqueda de ficheros\n"
				"   -b         Solo compilar (no ejecutar)\n"
				"   -g         Al compilar, guarda informacion de depurado en el DCB\n"
				"   -c         Al compilar, el fuente esta en caracteres MS-DOS\n"
				"   -f fichero Al compilar, incluye uno o mas ficheros en el DCB\n\n"
				"Lo que va despues del fichero son argumentos del programa.\n"
				"Codigo de salida al compilar: 0 = correcto, 1 = error de uso, 2 = error de compilacion.\n\n"
				"Programa libre bajo la GNU General Public License version 2 o posterior.\n",
				argv[0], argv[0], argv[0]) ;
			return 1 ;
		}

		/* Un .prg se compila en memoria (o a disco con -b) */
		a = filename + strlen(filename) ;
		if (a - filename > 4 && a[-4] == '.' && (a[-3] == 'p' || a[-3] == 'P') && (a[-2] == 'r' || a[-2] == 'R') && (a[-1] == 'g' || a[-1] == 'G'))
		{
#ifdef SIMURGH
			void * dcbdata = NULL ;
			int dcbsize = 0, rc ;

			rc = fxc_compile_memory (filename, (compile_debug || debug) ? DCB_DEBUG : 0, compile_dos, embed, nembed, &dcbdata, &dcbsize) ;
			if (rc != 0)
				return rc == 2 ? 2 : 1 ;

			strcpy (dcbname, filename) ;
			strcpy (dcbname + strlen(dcbname) - 4, ".dcb") ;

			if (build_only)
			{
				FILE * out = fopen (dcbname, "wb") ;
				if (!out || fwrite (dcbdata, 1, dcbsize, out) != (size_t) dcbsize)
				{
					printf ("Error: imposible escribir %s\n", dcbname) ;
					return 1 ;
				}
				fclose (out) ;
				free (dcbdata) ;
				return 0 ;
			}
			memdcb = dcbdata ;
			memdcb_size = dcbsize ;
#else
			printf ("Este ejecutable no incluye el compilador: compile antes %s con fxc\n", filename) ;
			return 1 ;
#endif
		}
		else if (build_only)
		{
			printf ("-b solo se aplica a ficheros .prg\n") ;
			return 1 ;
		}
	}

	/* Init RAND generator */

	srand (time(NULL)) ;

	/* Init SDL */

	if ( SDL_Init (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_CDROM) < 0 )
	{
		printf ("SDL Init Error: %s\n", SDL_GetError()) ;
		do_exit(1) ;
	}

	SDL_JoystickEventState (SDL_ENABLE) ;

    gr_con_printf ("¬15" FXI_VERSION) ;

	if (fxi)
	{
		/* Standalone interpreter, dump some info */

		sdl_version = SDL_Linked_Version();
		gr_con_printf ("¬14SDL: %d.%d.%d (DLL loaded: %d.%d.%d)",
				SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL,
				sdl_version->major, sdl_version->minor, sdl_version->patch) ;
	}

	/* Initialization (modules needed before dcb_load) */

	gprof_init () ;
	string_init () ;
	init_c_type() ;

	/* Init application title for windowed modes */

	strcpy (dcbname, filename) ;
	apptitle = strdup(filename) ;

#ifdef TARGET_MAC
        strcpy (files[current_file], filename);
#endif

	if (memdcb)
	{
		/* Compilado en memoria: el fichero virtual lleva el nombre del .dcb para fijar el directorio de trabajo */
		fp = file_mem_open (dcbname, memdcb, memdcb_size) ;
		if (!dcb_load_from (fp, 0))
		{
			gr_error ("%s: el DCB compilado no es valido", filename) ;
			return -1 ;
		}
	}
	else if (!embedded)
	{
		/* First try to load directly (we expect fxi myfile.dcb) */
		if (!dcb_load(dcbname))
		{
			/* not successful... try for DCB */

			strcat (dcbname, ".dcb") ;

			if (!dcb_load(dcbname))
			{
				/* not successful... try for DAT */

				strcpy (dcbname, filename) ;
 				strcat (dcbname, ".dat") ;

				if (!dcb_load(dcbname))
				{
					gr_error ("%s: no existe o no es un DCB version %d o compatible", filename, DCB_VERSION >> 8) ;
					return -1 ;
				}
			}
		}
	}
	else
	{
		dcb_load_from(fp, dcb_signature.dcb_offset);
/*		file_close (fp); */
	}

	/* If the dcb is not in debug mode, switch off the profiler */

	if (dcb.data.NID == 0)
	{
		gprof_toggle();
		debug = 0;
	}

	/* Initialization (modules needed after dcb_load) */

	fnc_init();
	#ifdef MMX_FUNCTIONS
		MMX_init();
	#endif

	grlib_init () ;
	sysproc_init () ;
	gr_font_systemfont (default_font);

	if (argc > 32) argc = 32 ;
	for (i = 0 ; i < argc ; i++)
	{
		int * ptr = &GLODWORD(ARGV_TABLE+i*4) ;
		*ptr = string_new(argv[i]) ;
		string_use (*ptr) ;
	}
	GLODWORD(ARGC) = argc-1 ;
	GLODWORD(FXI_OS) = OS_ID ;
	if (mainproc)
	{
		mainproc_running = instance_new (mainproc, 0) ;
		instance_go_all () ;
	}

	//if (cd_playing()) cd_stop() ;
	//string_dump() ;
	//gprof_dump ("profile.txt");

	do_exit(0);
	return 0;
}

