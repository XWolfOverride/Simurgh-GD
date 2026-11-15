/*
 *  Simurgh - asignacion de memoria del compilador (fxc).
 *
 *  Un solo sitio para pedir memoria. Todo bloque queda anotado en una lista del hilo, de modo que xalloc_release_all()
 *  lo libera todo de golpe: asi un error de compilacion (que sale con longjmp, ver fxc_fatal) no deja nada perdido.
 *  Ademas, todo lo que se escribe en el .dcb o se lee sin haberlo rellenado sale a cero:
 *  xcalloc, y xrealloc0 que pone a cero la parte nueva (solo la nueva: no se repite el trabajo sobre lo ya usado).
 *
 *  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  Copyright (C) 2026 XWolfOverride
 */

#ifndef __XALLOC_H
#define __XALLOC_H

#include <stdlib.h>
#include <string.h>

/* Estado global propio de cada hilo: asi dos compilaciones a la vez no se pisan (FXC_NO_TLS lo desactiva en destinos sin TLS). */
#if defined(FXC_NO_TLS)
#define FXC_TLS
#elif defined(__GNUC__) || defined(__clang__)
#define FXC_TLS __thread
#else
#define FXC_TLS _Thread_local
#endif

/* Definida en error.c. No vuelve. */
extern void fxc_out_of_memory (const char * where) ;

extern void * xmalloc (size_t size) ;
extern void * xcalloc (size_t count, size_t size) ;			/* memoria a cero */
extern void * xrealloc (void * ptr, size_t size) ;
extern void * xrealloc0 (void * ptr, size_t old_size, size_t new_size) ;	/* los bytes nuevos salen a cero */
extern char * xstrdup (const char * s) ;
extern void   xfree (void * ptr) ;						/* acepta NULL */
extern void   xalloc_release_all (void) ;					/* libera todo lo pedido y aun vivo */

#endif /* __XALLOC_H */
