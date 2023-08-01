/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/***************************************************************************/
/*                                                                         */
/*  ftsystem.h                                                             */
/*                                                                         */
/*    FreeType low-level system interface definition (specification).      */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTSYSTEM_H
#define AGS_LIB_FREETYPE_FTSYSTEM_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


typedef struct FT_MemoryRec_ *FT_Memory;
typedef void *(*FT_Alloc_Func)(FT_Memory memory, long size);
typedef void (*FT_Free_Func)(FT_Memory memory, void *block);
typedef void *(*FT_Realloc_Func)(FT_Memory memory, long cur_size, long new_size, void *block);

struct FT_MemoryRec_ {
	void            *user;
	FT_Alloc_Func   alloc;
	FT_Free_Func    free;
	FT_Realloc_Func realloc;
};

/**** I/O  MANAGEMENT ****/

typedef struct FT_StreamRec_*  FT_Stream;

typedef union  FT_StreamDesc_ {
	long   value;
	void   *pointer;
} FT_StreamDesc;

typedef unsigned long (*FT_Stream_IoFunc)(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);

typedef void (*FT_Stream_CloseFunc)( FT_Stream  stream );

typedef struct  FT_StreamRec_ {
	unsigned char        *base;
	unsigned long        size;
	unsigned long        pos;

	FT_StreamDesc        descriptor;
	FT_StreamDesc        pathname;
	FT_Stream_IoFunc     read;
	FT_Stream_CloseFunc  close;

	FT_Memory            memory;
	unsigned char        *cursor;
	unsigned char        *limit;
} FT_StreamRec;


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTSYSTEM_H */
