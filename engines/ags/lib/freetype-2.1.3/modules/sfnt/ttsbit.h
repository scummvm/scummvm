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
/*  ttsbit.h                                                               */
/*    TrueType and OpenType embedded bitmap support (specification).       */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_TTSBIT_H
#define AGS_LIB_FREETYPE_TTSBIT_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/sfnt/ttload.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL(FT_Error)
tt_face_load_sbit_strikes(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(void)
tt_face_free_sbit_strikes(TT_Face face);

FT2_1_3_LOCAL(FT_Error)
tt_face_set_sbit_strike(TT_Face face, FT_Int x_ppem, FT_Int y_ppem, FT_ULong *astrike_index);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_sbit_image(TT_Face face, FT_ULong strike_index, FT_UInt glyph_index, FT_UInt load_flags, FT_Stream stream, FT_Bitmap *map, TT_SBit_MetricsRec *metrics);


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_TTSBIT_H */
