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
/*  t1cmap.h                                                               */
/*                                                                         */
/*    Type 1 character map support (specification).                        */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_T1CMAP_H
#define AGS_LIB_FREETYPE_T1CMAP_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/t1types.h"
#include "engines/ags/lib/freetype-2.1.3/psnames.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


/**** TYPE1 STANDARD (AND EXPERT) ENCODING CMAPS ****/

/* standard (and expert) encoding cmaps */
typedef struct T1_CMapStdRec_ *T1_CMapStd;

typedef struct T1_CMapStdRec_ {
	FT_CMapRec cmap;

	const FT_UShort *code_to_sid;
	PS_Adobe_Std_Strings_Func sid_to_string;

	FT_UInt num_glyphs;
	const char *const *glyph_names;
} T1_CMapStdRec;

FT_CALLBACK_TABLE const FT_CMap_ClassRec t1_cmap_standard_class_rec;
FT_CALLBACK_TABLE const FT_CMap_ClassRec t1_cmap_expert_class_rec;


/**** TYPE1 CUSTOM ENCODING CMAP ****/

typedef struct T1_CMapCustomRec_ *T1_CMapCustom;

typedef struct T1_CMapCustomRec_ {
	FT_CMapRec cmap;
	FT_UInt    first;
	FT_UInt    count;
	FT_UShort  *indices;
} T1_CMapCustomRec;

FT_CALLBACK_TABLE const FT_CMap_ClassRec t1_cmap_custom_class_rec;


/**** TYPE1 SYNTHETIC UNICODE ENCODING CMAP ****/

/* unicode (syntehtic) cmaps */
typedef struct T1_CMapUnicodeRec_ *T1_CMapUnicode;

typedef struct T1_CMapUniPairRec_ {
	FT_UInt32 unicode;
	FT_UInt   gindex;
} T1_CMapUniPairRec, *T1_CMapUniPair;

typedef struct T1_CMapUnicodeRec_ {
	FT_CMapRec 	   cmap;
	FT_UInt 	   num_pairs;
	T1_CMapUniPair pairs;
} T1_CMapUnicodeRec;

FT_CALLBACK_TABLE const FT_CMap_ClassRec t1_cmap_unicode_class_rec;


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1CMAP_H */
