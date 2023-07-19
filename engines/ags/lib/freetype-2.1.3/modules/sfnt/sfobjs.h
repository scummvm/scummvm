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
/*  sfobjs.h                                                               */
/*    SFNT object management (specification).                              */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_SFOBJS_H
#define AGS_LIB_FREETYPE_SFOBJS_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/sfnt.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


FT2_1_3_LOCAL(FT_Error)
sfnt_init_face(FT_Stream stream, TT_Face face, FT_Int face_index, FT_Int num_params, FT_Parameter *params);

FT2_1_3_LOCAL(FT_Error)
sfnt_load_face(FT_Stream stream, TT_Face face, FT_Int face_index, FT_Int num_params, FT_Parameter *params);

FT2_1_3_LOCAL(void)
sfnt_done_face(TT_Face face);


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_SFOBJS_H */
