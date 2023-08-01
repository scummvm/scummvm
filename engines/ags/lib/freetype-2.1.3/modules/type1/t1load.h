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
/*  t1load.h                                                               */
/*                                                                         */
/*    Type 1 font loader (specification).                                  */
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


#ifndef AGS_LIB_FREETYPE_T1LOAD_H
#define AGS_LIB_FREETYPE_T1LOAD_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/psaux.h"
#include "engines/ags/lib/freetype-2.1.3/ftmm.h"

#include "engines/ags/lib/freetype-2.1.3/modules/type1/t1parse.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


typedef struct T1_Loader_ {
	T1_ParserRec  parser;          /* parser used to read the stream */

	FT_Int        num_chars;       /* number of characters in encoding */
	PS_TableRec   encoding_table;  /* PS_Table used to store the       */
	/* encoding character names         */

	FT_Int        num_glyphs;
	PS_TableRec   glyph_names;
	PS_TableRec   charstrings;
	PS_TableRec   swap_table;      /* For moving .notdef glyph to index 0. */

	FT_Int        num_subrs;
	PS_TableRec   subrs;
	FT_Bool       fontdata;
} T1_LoaderRec, *T1_Loader;


FT_LOCAL(FT_Error)
T1_Open_Face(T1_Face face);

#ifndef T1_CONFIG_OPTION_NO_MM_SUPPORT

FT_LOCAL(FT_Error)
T1_Get_Multi_Master(T1_Face face, FT_Multi_Master *master);

FT_LOCAL(FT_Error)
T1_Set_MM_Blend(T1_Face face, FT_UInt num_coords, FT_Fixed *coords);

FT_LOCAL(FT_Error)
T1_Set_MM_Design(T1_Face face, FT_UInt num_coords, FT_Long *coords);

FT_LOCAL(void)
T1_Done_Blend(T1_Face face);

#endif /* !T1_CONFIG_OPTION_NO_MM_SUPPORT */


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1LOAD_H */
