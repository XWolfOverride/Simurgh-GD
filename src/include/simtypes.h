/*
 *  Simurgh - tipos basicos y orden de bytes, sin depender de SDL.
 *
 *  Sustituye a <SDL_types.h> en las partes que no usan SDL (el compilador fxc). Si SDL 1.2 ya se ha incluido, sus
 *  definiciones son identicas (typedefs de <stdint.h>) y no se repiten.
 *
 *  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  Copyright (C) 2026 XWolfOverride
 */

#ifndef __SIMTYPES_H
#define __SIMTYPES_H

#include <stdint.h>
#include <stddef.h>

#ifndef _SDL_stdinc_h
typedef int8_t    Sint8;
typedef uint8_t   Uint8;
typedef int16_t   Sint16;
typedef uint16_t  Uint16;
typedef int32_t   Sint32;
typedef uint32_t  Uint32;
typedef int64_t   Sint64;
typedef uint64_t  Uint64;
#endif

/* Orden de bytes de la maquina: SIM_BIG_ENDIAN vale 1 en big-endian y 0 en little-endian. */
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define SIM_BIG_ENDIAN 1
#else
#define SIM_BIG_ENDIAN 0
#endif

#endif /* __SIMTYPES_H */
