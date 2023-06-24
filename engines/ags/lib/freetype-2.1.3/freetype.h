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

#ifndef AGS_LIB_FREETYPE_H
#define AGS_LIB_FREETYPE_H

#ifdef USE_FREETYPE2

#include "common/scummsys.h"
#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_FREETYPE_H
#include FT2_1_3_GLYPH_H

namespace AGS3 {
namespace FreeType213 {

extern FT2_1_3_Error Init_FreeType(FT2_1_3_Library *alibrary);
extern FT2_1_3_Error Done_FreeType(FT2_1_3_Library library);
extern FT2_1_3_Error Load_Glyph(FT2_1_3_Face face, FT2_1_3_UInt glyph_index, FT2_1_3_Int32 load_flags);
extern FT2_1_3_Error Get_Glyph(FT2_1_3_GlyphSlot slot, FT2_1_3_Glyph *aglyph);
extern FT2_1_3_Error Glyph_Copy(FT2_1_3_Glyph source, FT2_1_3_Glyph *target);
extern FT2_1_3_Error Glyph_To_Bitmap(FT2_1_3_Glyph *the_glyph, FT2_1_3_Render_Mode render_mode,
	FT2_1_3_Vector *origin, FT2_1_3_Bool destroy);
extern void Done_Glyph(FT2_1_3_Glyph glyph);
extern FT2_1_3_Error Set_Pixel_Sizes(FT2_1_3_Face face, FT2_1_3_UInt pixel_width,
	FT2_1_3_UInt pixel_height);
extern FT2_1_3_Error New_Face(FT2_1_3_Library library, const char *pathname,
	FT2_1_3_Long face_index, FT2_1_3_Face *aface);
extern FT2_1_3_Error New_Memory_Face(FT2_1_3_Library library, const FT2_1_3_Byte *file_base,
	FT2_1_3_Long file_size, FT2_1_3_Long face_index, FT2_1_3_Face *aface);
extern FT2_1_3_Error Done_Face(FT2_1_3_Face face);
extern FT2_1_3_UInt Get_Char_Index(FT2_1_3_Face face, FT2_1_3_ULong charcode);
extern FT2_1_3_Error Get_Kerning(FT2_1_3_Face face, FT2_1_3_UInt left_glyph,
	FT2_1_3_UInt right_glyph, FT2_1_3_UInt kern_mode, FT2_1_3_Vector *akerning);

} // End of namespace FreeType213
} // End of namespace AGS3

#endif

#endif

