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
/*  fttypes.h                                                              */
/*                                                                         */
/*    FreeType simple types definitions (specification only).              */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTTYPES_H
#define AGS_LIB_FREETYPE_FTTYPES_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/config/ftconfig.h"
#include "engines/ags/lib/freetype-2.1.3/ftsystem.h"
#include "engines/ags/lib/freetype-2.1.3/ftimage.h"

#include <stddef.h>

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


/**** BASIC TYPES ****/

typedef unsigned char FT_Bool;
typedef signed short FT_FWord;
typedef unsigned short FT_UFWord;
typedef signed char FT_Char;
typedef unsigned char FT_Byte;
typedef char FT_String;
typedef signed short FT_Short;
typedef unsigned short FT_UShort;
typedef int FT_Int;
typedef unsigned int FT_UInt;
typedef signed long FT_Long;
typedef unsigned long FT_ULong;
typedef signed short FT_F2Dot14;
typedef signed long FT_F26Dot6;
typedef signed long FT_Fixed;
typedef int FT_Error;
typedef void *FT_Pointer;
typedef size_t FT_Offset;
typedef size_t FT_PtrDist;

typedef struct FT_UnitVector_ {
	FT_F2Dot14 x;
	FT_F2Dot14 y;
} FT_UnitVector;

typedef struct FT_Matrix_ {
	FT_Fixed xx, xy;
	FT_Fixed yx, yy;
} FT_Matrix;

typedef struct FT_Data_ {
	const FT_Byte *pointer;
	FT_Int length;
} FT_Data;

typedef void (*FT_Generic_Finalizer)(void *object);

typedef struct FT_Generic_ {
	void *data;
	FT_Generic_Finalizer finalizer;
} FT_Generic;

#define FT_MAKE_TAG(_x1, _x2, _x3, _x4) \
	(((FT_ULong)_x1 << 24) | ((FT_ULong)_x2 << 16) | ((FT_ULong)_x3 << 8) | (FT_ULong)_x4)


/**** LIST MANAGEMENT ****/

typedef struct FT_ListNodeRec_ *FT_ListNode;
typedef struct FT_ListRec_ *FT_List;

typedef struct FT_ListNodeRec_ {
	FT_ListNode prev;
	FT_ListNode next;
	void *data;
} FT_ListNodeRec;

typedef struct FT_ListRec_ {
	FT_ListNode head;
	FT_ListNode tail;
} FT_ListRec;


#define FT_IS_EMPTY(list) ((list).head == 0)

/* return base error code (without module-specific prefix) */
#define FT_ERROR_BASE(x) ((x)&0xFF)

/* return module error code */
#define FT_ERROR_MODULE(x) ((x)&0xFF00U)

#define FT_BOOL(x) ((FT_Bool)(x))


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTTYPES_H */
