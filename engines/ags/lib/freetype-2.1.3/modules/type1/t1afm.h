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
/*  t1afm.h                                                                */
/*    AFM support for Type 1 fonts (specification).                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_T1AFM_H
#define AGS_LIB_FREETYPE_T1AFM_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/type1/t1objs.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


typedef struct T1_Kern_Pair_ {
	FT_UInt    glyph1;
	FT_UInt    glyph2;
	FT_Vector  kerning;
} T1_Kern_Pair;

typedef struct T1_AFM_ {
	FT_Int         num_pairs;
	T1_Kern_Pair   *kern_pairs;
} T1_AFM;


FT_LOCAL(FT_Error)
T1_Read_AFM(FT_Face face, FT_Stream stream);

FT_LOCAL(void)
T1_Done_AFM(FT_Memory memory, T1_AFM *afm);

FT_LOCAL(void)
T1_Get_Kerning(T1_AFM *afm, FT_UInt glyph1, FT_UInt glyph2, FT_Vector *kerning);


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1AFM_H */
