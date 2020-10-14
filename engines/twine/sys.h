/** @file sys.h
	@brief
	This file contains system types definitions

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SYS_H
#define SYS_H

#include <stdlib.h>
#include <math.h>

#define Rnd(x) rand()%x
#define Abs(x) abs(x)

// TYPES

typedef unsigned char byte;

typedef unsigned char uint8;
typedef char int8;

typedef unsigned short uint16;
typedef short int16;

typedef unsigned int uint32;
typedef int int32;

typedef float int64;

// ENDIAN

#ifdef UNIX
#define FORCEINLINE static __inline__
#elif MINGW32
#define FORCEINLINE inline
#else
#define FORCEINLINE __forceinline
#endif

#endif
