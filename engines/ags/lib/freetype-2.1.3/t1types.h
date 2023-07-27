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
/*  t1types.h                                                              */
/*                                                                         */
/*    Basic Type1/Type2 type definitions and interface (specification      */
/*    only).                                                               */
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


#ifndef AGS_LIB_FREETYPE_T1TYPES_H
#define AGS_LIB_FREETYPE_T1TYPES_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/t1tables.h"
#include "engines/ags/lib/freetype-2.1.3/psnames.h"
#include "engines/ags/lib/freetype-2.1.3/pshints.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


/**** REQUIRED TYPE1/TYPE2 TABLES DEFINITIONS ****/

typedef struct  T1_EncodingRecRec_ {
	FT_Int       num_chars;
	FT_Int       code_first;
	FT_Int       code_last;

	FT_UShort    *char_index;
	FT_String    **char_name;
} T1_EncodingRec, *T1_Encoding;

typedef enum  T1_EncodingType_ {
	T1_ENCODING_TYPE_NONE = 0,
	T1_ENCODING_TYPE_ARRAY,
	T1_ENCODING_TYPE_STANDARD,
	T1_ENCODING_TYPE_ISOLATIN1,
	T1_ENCODING_TYPE_EXPERT
} T1_EncodingType;

typedef struct  T1_FontRec_ {
	PS_FontInfoRec   font_info;         /* font info dictionary */
	PS_PrivateRec    private_dict;      /* private dictionary   */
	FT_String        *font_name;        /* top-level dictionary */

	T1_EncodingType  encoding_type;
	T1_EncodingRec   encoding;

	FT_Byte          *subrs_block;
	FT_Byte          *charstrings_block;
	FT_Byte          *glyph_names_block;

	FT_Int           num_subrs;
	FT_Byte          **subrs;
	FT_Int           *subrs_len;

	FT_Int           num_glyphs;
	FT_String        **glyph_names;       /* array of glyph names       */
	FT_Byte          **charstrings;       /* array of glyph charstrings */
	FT_Int           *charstrings_len;

	FT_Byte          paint_type;
	FT_Byte          font_type;
	FT_Matrix        font_matrix;
	FT_Vector        font_offset;
	FT_BBox          font_bbox;
	FT_Long          font_id;

	FT_Int           stroke_width;
} T1_FontRec, *T1_Font;

typedef struct CID_SubrsRec_ {
	FT_UInt  num_subrs;
	FT_Byte  **code;
} CID_SubrsRec, *CID_Subrs;


/**** ORIGINAL T1_FACE CLASS DEFINITION ****/

typedef struct T1_FaceRec_ *T1_Face;
typedef struct CID_FaceRec_ *CID_Face;

typedef struct  T1_FaceRec_ {
	FT_FaceRec     root;
	T1_FontRec     type1;
	const void     *psnames;
	const void     *psaux;
	const void     *afm_data;
	FT_CharMapRec  charmaprecs[2];
	FT_CharMap     charmaps[2];
	PS_Unicodes    unicode_map;

	/* support for Multiple Masters fonts */
	PS_Blend       blend;

	/* since FT 2.1 - interface to PostScript hinter */
	const void     *pshinter;
} T1_FaceRec;

typedef struct  CID_FaceRec_ {
	FT_FaceRec       root;
	void             *psnames;
	void             *psaux;
	CID_FaceInfoRec  cid;
	void             *afm_data;
	CID_Subrs        subrs;

	/* since FT 2.1 - interface to PostScript hinter */
	void             *pshinter;
} CID_FaceRec;


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1TYPES_H */
