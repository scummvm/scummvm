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
/*  ftglyph.h                                                              */
/*    FreeType convenience functions to handle glyphs (specification).     */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTGLYPH_H
#define AGS_LIB_FREETYPE_FTGLYPH_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/* forward declaration to a private type */
typedef struct FT_Glyph_Class_ FT_Glyph_Class;

typedef struct FT_GlyphRec_ *FT_Glyph;

typedef struct  FT_GlyphRec_ {
	FT_Library             library;
	const FT_Glyph_Class   *clazz;
	FT_Glyph_Format        format;
	FT_Vector              advance;
} FT_GlyphRec;

typedef struct FT_BitmapGlyphRec_ *FT_BitmapGlyph;

typedef struct  FT_BitmapGlyphRec_ {
	FT_GlyphRec  root;
	FT_Int       left;
	FT_Int       top;
	FT_Bitmap    bitmap;
} FT_BitmapGlyphRec;

typedef struct FT_OutlineGlyphRec_ *FT_OutlineGlyph;

typedef struct  FT_OutlineGlyphRec_ {
	FT_GlyphRec  root;
	FT_Outline   outline;
} FT_OutlineGlyphRec;


FT2_1_3_EXPORT(FT_Error)
FT2_1_3_Get_Glyph(FT_GlyphSlot slot, FT_Glyph *aglyph);

FT2_1_3_EXPORT(FT_Error)
FT2_1_3_Glyph_Copy(FT_Glyph source, FT_Glyph *target);

FT2_1_3_EXPORT(FT_Error)
FT_Glyph_Transform(FT_Glyph glyph, FT_Matrix *matrix, FT_Vector *delta);


enum {
	ft_glyph_bbox_unscaled  = 0, /* return unscaled font units           */
	ft_glyph_bbox_subpixels = 0, /* return unfitted 26.6 coordinates     */
	ft_glyph_bbox_gridfit   = 1, /* return grid-fitted 26.6 coordinates  */
	ft_glyph_bbox_truncate  = 2, /* return coordinates in integer pixels */
	ft_glyph_bbox_pixels    = 3  /* return grid-fitted pixel coordinates */
};


FT2_1_3_EXPORT(void)
FT_Glyph_Get_CBox(FT_Glyph glyph, FT_UInt bbox_mode, FT_BBox *acbox);

FT2_1_3_EXPORT(FT_Error)
FT2_1_3_Glyph_To_Bitmap(FT_Glyph *the_glyph, FT_Render_Mode render_mode, FT_Vector *origin, FT_Bool destroy);

FT2_1_3_EXPORT(void)
FT2_1_3_Done_Glyph(FT_Glyph glyph);


/* other helpful functions */

FT2_1_3_EXPORT(void)
FT_Matrix_Multiply(FT_Matrix *a, FT_Matrix *b);

FT2_1_3_EXPORT(FT_Error)
FT_Matrix_Invert(FT_Matrix *matrix);


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTGLYPH_H */
