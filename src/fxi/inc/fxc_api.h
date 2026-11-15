/* Simurgh: nuevo 2026-11-15 - interfaz del compilador (fxc) que usa el interprete cuando se construye como programa unico */
#ifndef __FXC_API_H
#define __FXC_API_H

#include "dcb.h"

/* Compila un .prg en memoria y devuelve el .dcb (bloque de malloc). 0 = correcto, 2 = error de compilacion (ya impreso). */
extern int fxc_compile_memory (const char * filename, int options, int dos_chars, const char ** embed, int embed_count, void ** data, int * size) ;

#endif
