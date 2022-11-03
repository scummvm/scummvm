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

#ifndef GRAPHICS_FONTS_FREETYPE_H
#define GRAPHICS_FONTS_FREETYPE_H

#ifdef USE_FREETYPE2

#include "common/scummsys.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace Graphics {
namespace FreeType {

extern FT_Error Init_FreeType(FT_Library *alibrary);
extern FT_Error Done_FreeType(FT_Library library);
extern FT_Error Load_Glyph(FT_Face face, FT_UInt glyph_index, FT_Int32 load_flags);
extern FT_Error Get_Glyph(FT_GlyphSlot slot, FT_Glyph *aglyph);
extern FT_Error Glyph_Copy(FT_Glyph source, FT_Glyph *target);
extern FT_Error Glyph_To_Bitmap(FT_Glyph *the_glyph, FT_Render_Mode render_mode,
	FT_Vector *origin, FT_Bool destroy);
extern void Done_Glyph(FT_Glyph glyph);
extern FT_Error Set_Pixel_Sizes(FT_Face face, FT_UInt pixel_width,
	FT_UInt pixel_height);
extern FT_Error New_Face(FT_Library library, const char *pathname,
	FT_Long face_index, FT_Face *aface);
extern FT_Error New_Memory_Face(FT_Library library, const FT_Byte *file_base,
	FT_Long file_size, FT_Long face_index, FT_Face *aface);
extern FT_Error Done_Face(FT_Face face);
extern FT_UInt Get_Char_Index(FT_Face face, FT_ULong charcode);
extern FT_Error Get_Kerning(FT_Face face, FT_UInt left_glyph,
	FT_UInt right_glyph, FT_UInt kern_mode, FT_Vector *akerning);

} // End of namespace FreeType
} // End of namespace Graphics

#endif

#endif

