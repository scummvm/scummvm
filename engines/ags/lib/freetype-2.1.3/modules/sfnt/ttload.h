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
/*  ttload.h                                                               */
/*    Load the basic TrueType tables, i.e., tables that can be either in   */
/*    TTF or OTF fonts (specification).                                    */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_TTLOAD_H
#define AGS_LIB_FREETYPE_TTLOAD_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftstream.h"
#include "engines/ags/lib/freetype-2.1.3/tttypes.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL(TT_Table)
tt_face_lookup_table(TT_Face face, FT_ULong tag);

FT2_1_3_LOCAL(FT_Error)
tt_face_goto_table(TT_Face face, FT_ULong tag, FT_Stream stream, FT_ULong *length);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_sfnt_header(TT_Face face, FT_Stream stream, FT_Long face_index, SFNT_Header sfnt);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_directory(TT_Face face, FT_Stream stream, SFNT_Header sfnt);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_any(TT_Face face, FT_ULong tag, FT_Long offset, FT_Byte *buffer, FT_ULong *length);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_header(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_metrics_header(TT_Face face, FT_Stream stream, FT_Bool vertical);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_cmap(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_max_profile(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_names(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_os2(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_postscript(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_hdmx(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_pclt(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(void)
tt_face_free_names(TT_Face face);

FT2_1_3_LOCAL(void)
tt_face_free_hdmx(TT_Face face);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_kern(TT_Face face, FT_Stream stream);

FT2_1_3_LOCAL(FT_Error)
tt_face_load_gasp(TT_Face face, FT_Stream stream);

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS

FT2_1_3_LOCAL(FT_Error)
tt_face_load_bitmap_header(TT_Face face, FT_Stream stream);

#endif /* TT_CONFIG_OPTION_EMBEDDED_BITMAPS */

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_TTLOAD_H */
