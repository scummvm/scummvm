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

// Since FreeType2 includes files, which contain forbidden symbols, we need to
// allow all symbols here.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef USE_FREETYPE2

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftglyph.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_Error Init_FreeType(FT2_1_3_Library *alibrary) {
	return FT2_1_3_Init_FreeType(alibrary);
}

FT2_1_3_Error Done_FreeType(FT2_1_3_Library library) {
	return FT2_1_3_Done_FreeType(library);
}

FT2_1_3_Error Load_Glyph(FT2_1_3_Face face, FT2_1_3_UInt glyph_index, FT2_1_3_Int32 load_flags) {
	return FT2_1_3_Load_Glyph(face, glyph_index, load_flags);
}

FT2_1_3_Error Get_Glyph(FT2_1_3_GlyphSlot slot, FT2_1_3_Glyph *aglyph) {
	return FT2_1_3_Get_Glyph(slot, aglyph);
}

FT2_1_3_Error Glyph_Copy(FT2_1_3_Glyph source, FT2_1_3_Glyph *target) {
	return FT2_1_3_Glyph_Copy(source, target);
}

FT2_1_3_Error Glyph_To_Bitmap(FT2_1_3_Glyph *the_glyph, FT2_1_3_Render_Mode render_mode,
		FT2_1_3_Vector *origin, FT2_1_3_Bool destroy) {
	return FT2_1_3_Glyph_To_Bitmap(the_glyph, render_mode, origin, destroy);
}

void Done_Glyph(FT2_1_3_Glyph glyph) {
	return FT2_1_3_Done_Glyph(glyph);
}

FT2_1_3_Error Set_Pixel_Sizes(FT2_1_3_Face face, FT2_1_3_UInt pixel_width,
		FT2_1_3_UInt pixel_height) {
	return FT2_1_3_Set_Pixel_Sizes(face, pixel_width, pixel_height);
}

FT2_1_3_Error New_Face(FT2_1_3_Library library, const char *pathname,
		FT2_1_3_Long face_index, FT2_1_3_Face *aface) {
	return FT2_1_3_New_Face(library, pathname, face_index, aface);
}

FT2_1_3_Error New_Memory_Face(FT2_1_3_Library library, const FT2_1_3_Byte *file_base,
		FT2_1_3_Long file_size, FT2_1_3_Long face_index, FT2_1_3_Face *aface) {
	return FT2_1_3_New_Memory_Face(library, file_base, file_size, face_index, aface);
}

FT2_1_3_Error Done_Face(FT2_1_3_Face face) {
	return FT2_1_3_Done_Face(face);
}

FT2_1_3_UInt Get_Char_Index(FT2_1_3_Face face, FT2_1_3_ULong charcode) {
	return FT2_1_3_Get_Char_Index(face, charcode);
}

FT2_1_3_Error Get_Kerning(FT2_1_3_Face face, FT2_1_3_UInt left_glyph,
	FT2_1_3_UInt right_glyph, FT2_1_3_UInt kern_mode, FT2_1_3_Vector *akerning) {
	return FT2_1_3_Get_Kerning(face, left_glyph, right_glyph, kern_mode, akerning);
}

} // End of namespace FreeType
} // End of namespace AGS3

#endif
