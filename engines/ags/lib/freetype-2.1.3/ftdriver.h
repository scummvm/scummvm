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
/*  ftdriver.h                                                             */
/*                                                                         */
/*    FreeType font driver interface (specification).                      */
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


#ifndef AGS_LIB_FREETYPE_FTDRIVER_H
#define AGS_LIB_FREETYPE_FTDRIVER_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftmodule.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


typedef FT_Error (*FT_Face_InitFunc)(FT_Stream stream, FT_Face face, FT_Int typeface_index, FT_Int num_params, FT_Parameter *parameters);

typedef void (*FT_Face_DoneFunc)(FT_Face face);

typedef FT_Error (*FT_Size_InitFunc)(FT_Size size);

typedef void (*FT_Size_DoneFunc)(FT_Size size);

typedef FT_Error (*FT_Slot_InitFunc)(FT_GlyphSlot slot);

typedef void (*FT_Slot_DoneFunc)(FT_GlyphSlot slot);

typedef FT_Error (*FT_Size_ResetPointsFunc)(FT_Size size, FT_F26Dot6 char_width, FT_F26Dot6 char_height, FT_UInt horz_resolution, FT_UInt vert_resolution);

typedef FT_Error (*FT_Size_ResetPixelsFunc)(FT_Size size, FT_UInt pixel_width, FT_UInt pixel_height);

typedef FT_Error (*FT_Slot_LoadFunc)(FT_GlyphSlot slot, FT_Size size, FT_UInt glyph_index, FT_Int32 load_flags);

typedef FT_UInt (*FT_CharMap_CharIndexFunc)(FT_CharMap charmap, FT_Long charcode);

typedef FT_Long (*FT_CharMap_CharNextFunc)(FT_CharMap charmap, FT_Long charcode);

typedef FT_Error (*FT_Face_GetKerningFunc)(FT_Face face, FT_UInt left_glyph, FT_UInt right_glyph, FT_Vector *kerning);

typedef FT_Error (*FT_Face_AttachFunc)(FT_Face face, FT_Stream stream);

typedef FT_Error (*FT_Face_GetAdvancesFunc)(FT_Face face, FT_UInt first, FT_UInt count, FT_Bool vertical, FT_UShort *advances);


typedef struct FT_Driver_ClassRec_ {
	FT_Module_Class root;

	FT_Int face_object_size;
	FT_Int size_object_size;
	FT_Int slot_object_size;

	FT_Face_InitFunc init_face;
	FT_Face_DoneFunc done_face;

	FT_Size_InitFunc init_size;
	FT_Size_DoneFunc done_size;

	FT_Slot_InitFunc init_slot;
	FT_Slot_DoneFunc done_slot;

	FT_Size_ResetPointsFunc set_char_sizes;
	FT_Size_ResetPixelsFunc set_pixel_sizes;

	FT_Slot_LoadFunc load_glyph;

	FT_Face_GetKerningFunc get_kerning;
	FT_Face_AttachFunc attach_file;
	FT_Face_GetAdvancesFunc get_advances;

} FT_Driver_ClassRec, *FT_Driver_Class;


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTDRIVER_H */
