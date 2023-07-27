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
/*  t1parse.h                                                              */
/*                                                                         */
/*    Type 1 parser (specification).                                       */
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


#ifndef AGS_LIB_FREETYPE_T1PARSE_H
#define AGS_LIB_FREETYPE_T1PARSE_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/t1types.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


typedef struct T1_ParserRec_ {
	PS_ParserRec  root;
	FT_Stream     stream;

	FT_Byte       *base_dict;
	FT_Long       base_len;

	FT_Byte       *private_dict;
	FT_Long       private_len;

	FT_Byte       in_pfb;
	FT_Byte       in_memory;
	FT_Byte       single_block;
} T1_ParserRec, *T1_Parser;


#define T1_Add_Table(p, i, o, l) (p)->funcs.add((p), i, o, l)

#define T1_Done_Table(p)        \
	do {                        \
		if ((p)->funcs.done)    \
			(p)->funcs.done(p); \
	} while (0)

#define T1_Release_Table(p)        \
	do {                           \
		if ((p)->funcs.release)    \
			(p)->funcs.release(p); \
	} while (0)

#define T1_Skip_Spaces(p) (p)->root.funcs.skip_spaces(&(p)->root)
#define T1_Skip_Alpha(p)  (p)->root.funcs.skip_alpha(&(p)->root)

#define T1_ToInt(p) 	 (p)->root.funcs.to_int(&(p)->root)
#define T1_ToFixed(p, t) (p)->root.funcs.to_fixed(&(p)->root, t)

#define T1_ToCoordArray(p, m, c) 	(p)->root.funcs.to_coord_array(&(p)->root, m, c)
#define T1_ToFixedArray(p, m, f, t) (p)->root.funcs.to_fixed_array(&(p)->root, m, f, t)
#define T1_ToToken(p, t) 			(p)->root.funcs.to_token(&(p)->root, t)
#define T1_ToTokenArray(p, t, m, c) (p)->root.funcs.to_token_array(&(p)->root, t, m, c)

#define T1_Load_Field(p, f, o, m, pf) (p)->root.funcs.load_field(&(p)->root, f, o, m, pf)

#define T1_Load_Field_Table(p, f, o, m, pf) (p)->root.funcs.load_field_table(&(p)->root, f, o, m, pf)


FT_LOCAL(FT_Error)
T1_New_Parser(T1_Parser parser, FT_Stream stream, FT_Memory memory, PSAux_Service psaux);

FT_LOCAL(FT_Error)
T1_Get_Private_Dict(T1_Parser parser, PSAux_Service psaux);

FT_LOCAL(void)
T1_Finalize_Parser(T1_Parser parser);


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1PARSE_H */
