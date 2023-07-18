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
/*  t1objs.h                                                               */
/*    Type 1 objects manager (specification).                              */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_T1OBJS_H
#define AGS_LIB_FREETYPE_T1OBJS_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/config/ftconfig.h"
#include "engines/ags/lib/freetype-2.1.3/t1types.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/* The following structures must be defined by the hinter */
typedef struct T1_Size_Hints_  T1_Size_Hints;
typedef struct T1_Glyph_Hints_ T1_Glyph_Hints;

typedef struct T1_DriverRec_ 	*T1_Driver;
typedef struct T1_SizeRec_ 		*T1_Size;
typedef struct T1_GlyphSlotRec_ *T1_GlyphSlot;
typedef struct T1_CharMapRec_ 	*T1_CharMap;


/* HERE BEGINS THE TYPE1 SPECIFIC STUFF */

typedef struct T1_SizeRec_ {
	FT_SizeRec root;
} T1_SizeRec;


FT2_1_3_LOCAL(void)
T1_Size_Done(T1_Size size);

FT2_1_3_LOCAL(FT_Error)
T1_Size_Reset(T1_Size size);

FT2_1_3_LOCAL(FT_Error)
T1_Size_Init(T1_Size size);


typedef struct T1_GlyphSlotRec_ {
	FT_GlyphSlotRec root;

	FT_Bool hint;
	FT_Bool scaled;

	FT_Int max_points;
	FT_Int max_contours;

	FT_Fixed x_scale;
	FT_Fixed y_scale;
} T1_GlyphSlotRec;


FT2_1_3_LOCAL(FT_Error)
T1_Face_Init(FT_Stream stream, T1_Face face, FT_Int face_index, FT_Int num_params, FT_Parameter *params);

FT2_1_3_LOCAL(void)
T1_Face_Done(T1_Face face);

FT2_1_3_LOCAL(FT_Error)
T1_GlyphSlot_Init(T1_GlyphSlot slot);

FT2_1_3_LOCAL(void)
T1_GlyphSlot_Done(T1_GlyphSlot slot);

FT2_1_3_LOCAL(FT_Error)
T1_Driver_Init(T1_Driver driver);

FT2_1_3_LOCAL(void)
T1_Driver_Done(T1_Driver driver);


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_T1OBJS_H */
