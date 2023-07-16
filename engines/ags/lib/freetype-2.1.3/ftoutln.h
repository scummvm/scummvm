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
/*  ftoutln.h                                                              */
/*    Support for the FT_Outline type used to store glyph shapes of        */
/*    most scalable font formats (specification).                          */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTOUTLN_H
#define AGS_LIB_FREETYPE_FTOUTLN_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


FT2_1_3_EXPORT(FT_Error)
FT_Outline_Decompose(FT_Outline *outline, const FT_Outline_Funcs *func_interface, void *user);

FT2_1_3_EXPORT(FT_Error)
FT_Outline_New(FT_Library library, FT_UInt numPoints, FT_Int numContours, FT_Outline *anoutline);

FT2_1_3_EXPORT(FT_Error)
FT_Outline_New_Internal(FT_Memory memory, FT_UInt numPoints, FT_Int numContours, FT_Outline *anoutline);

FT2_1_3_EXPORT(FT_Error)
FT_Outline_Done(FT_Library library, FT_Outline *outline);

FT2_1_3_EXPORT(FT_Error)
FT_Outline_Done_Internal(FT_Memory memory, FT_Outline *outline);

FT2_1_3_EXPORT(FT_Error)
FT_Outline_Check(FT_Outline *outline);

FT2_1_3_EXPORT(void)
FT_Outline_Get_CBox(FT_Outline *outline, FT_BBox *acbox);

FT2_1_3_EXPORT(void)
FT_Outline_Translate(FT_Outline *outline, FT_Pos xOffset, FT_Pos yOffset);

FT2_1_3_EXPORT(FT_Error)
FT_Outline_Copy(FT_Outline *source, FT_Outline *target);

FT2_1_3_EXPORT(void)
FT_Outline_Transform(FT_Outline *outline, FT_Matrix *matrix);

FT2_1_3_EXPORT(void)
FT_Outline_Reverse(FT_Outline *outline);

FT2_1_3_EXPORT(FT_Error)
FT_Outline_Get_Bitmap(FT_Library library, FT_Outline *outline, FT_Bitmap *abitmap);

FT2_1_3_EXPORT(FT_Error)
FT_Outline_Render(FT_Library library, FT_Outline *outline, FT_Raster_Params *params);


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTOUTLN_H */
