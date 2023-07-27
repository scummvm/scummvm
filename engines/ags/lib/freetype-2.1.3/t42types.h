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
/*  t42types.h                                                             */
/*                                                                         */
/*    Type 42 font data types (specification only).                        */
/*                                                                         */
/*  Copyright 2002 by Roberto Alameda.                                     */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_T42TYPES_H
#define AGS_LIB_FREETYPE_T42TYPES_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/t1tables.h"
#include "engines/ags/lib/freetype-2.1.3/t1types.h"
#include "engines/ags/lib/freetype-2.1.3/psnames.h"
#include "engines/ags/lib/freetype-2.1.3/pshints.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


typedef struct  T42_FaceRec_ {
	FT_FaceRec     root;
	T1_FontRec     type1;
	const void     *psnames;
	const void     *psaux;
	const void     *afm_data;
	FT_Byte        *ttf_data;
	FT_ULong       ttf_size;
	FT_Face        ttf_face;
	FT_CharMapRec  charmaprecs[2];
	FT_CharMap     charmaps[2];
	PS_Unicodes    unicode_map;
} T42_FaceRec, *T42_Face;


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T42TYPES_H */
