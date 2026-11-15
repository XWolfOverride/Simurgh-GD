/* Simurgh: prueba de reentrancia de fxc_main (biblioteca): compilaciones seguidas, con errores entre medias, y varios hilos a la vez.
 *   reentrant ok.prg err.prg [hilos] [vueltas]
 * Compila ok.prg varias veces (con err.prg en medio) y comprueba que el .dcb sale siempre igual; luego lo repite en varios
 * hilos, cada uno con su copia. Devuelve 0 si todo coincide. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

extern int fxc_main (int argc, char ** argv) ;

static char * slurp (const char * f, long * n)
{
	FILE * fp = fopen (f, "rb") ; char * b ;
	if (!fp) { *n = -1 ; return 0 ; }
	fseek (fp, 0, SEEK_END) ; *n = ftell (fp) ; fseek (fp, 0, SEEK_SET) ;
	b = (char *) malloc (*n + 1) ; if (fread (b, 1, *n, fp) != (size_t)*n) *n = -1 ; fclose (fp) ; return b ;
}

static int compile (const char * prg)
{
	char * argv[3] = { "fxc", (char *) prg, 0 } ;
	return fxc_main (2, argv) ;
}

static char * ref ; static long refn ;
static int vueltas = 10 ;

static char * refs[16] ; static long refsn[16] ;

static int same_as (const char * dcb, const char * r, long rn)
{
	long n ; char * b = slurp (dcb, &n) ;
	int ok = b && n == rn && memcmp (b, r, n) == 0 ;
	free (b) ; return ok ;
}

static int same_dcb (const char * dcb) { return same_as (dcb, ref, refn) ; }

static char names[16][64] ;
static void * worker (void * arg)
{
	int id = (int)(size_t) arg, i, bad = 0 ;
	char dcb[80] ;
	strcpy (dcb, names[id]) ; strcpy (strrchr (dcb, '.'), ".dcb") ;
	for (i = 0 ; i < vueltas ; i++)
	{
		remove (dcb) ;
		compile (names[id]) ;
		if (!same_as (dcb, refs[id], refsn[id])) bad++ ;
	}
	return (void *)(size_t) bad ;
}

int main (int argc, char ** argv)
{
	int hilos = argc > 3 ? atoi (argv[3]) : 4, i, bad = 0 ;
	long n ; char * src ; pthread_t th[16] ;
	if (argc > 4) vueltas = atoi (argv[4]) ;
	if (argc < 3 || hilos > 16) return 9 ;

	/* En serie: ok, error, ok, error, ok */
	remove ("r0.dcb") ; src = slurp (argv[1], &n) ;
	{ FILE * f = fopen ("r0.prg", "wb") ; fwrite (src, 1, n, f) ; fclose (f) ; }
	compile ("r0.prg") ;
	ref = slurp ("r0.dcb", &refn) ;
	if (refn <= 0) { printf ("[X] no hay .dcb de referencia\n") ; return 1 ; }
	for (i = 0 ; i < vueltas ; i++)
	{
		int rc ;
		remove ("r0.dcb") ;
		if (compile (argv[2]) != 2) { printf ("[X] el error no dio codigo 2\n") ; return 1 ; }
		rc = compile ("r0.prg") ;
		if (rc != 0 || !same_dcb ("r0.dcb")) { printf ("[X] serie: vuelta %d distinta (rc=%d)\n", i, rc) ; return 1 ; }
	}
	printf ("[OK] serie: %d compilaciones con errores entre medias, .dcb identico (%ld bytes)\n", vueltas, refn) ;

	/* En paralelo */
	for (i = 0 ; i < hilos ; i++)
	{
		FILE * f ;
		sprintf (names[i], "h%d.prg", i) ;
		f = fopen (names[i], "wb") ; fwrite (src, 1, n, f) ; fclose (f) ;
		{ char d[80] ; strcpy (d, names[i]) ; strcpy (strrchr (d, '.'), ".dcb") ; remove (d) ; compile (names[i]) ; refs[i] = slurp (d, &refsn[i]) ; }
	}
	for (i = 0 ; i < hilos ; i++) pthread_create (&th[i], 0, worker, (void *)(size_t) i) ;
	for (i = 0 ; i < hilos ; i++) { void * r ; pthread_join (th[i], &r) ; bad += (int)(size_t) r ; }
	if (bad) { printf ("[X] paralelo: %d compilaciones distintas\n", bad) ; return 1 ; }
	printf ("[OK] paralelo: %d hilos x %d vueltas, .dcb identico\n", hilos, vueltas) ;
	return 0 ;
}
