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
/*  cffload.h                                                              */
/*                                                                         */
/*    OpenType & CFF data/program tables loader (specification).           */
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


#ifndef AGS_LIB_FREETYPE_CFFLOAD_H
#define AGS_LIB_FREETYPE_CFFLOAD_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cfftypes.h"
#include "engines/ags/lib/freetype-2.1.3/psnames.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


FT_LOCAL(FT_UShort)
cff_get_standard_encoding(FT_UInt charcode);

FT_LOCAL(FT_String *)
cff_index_get_name(CFF_Index idx, FT_UInt element);

FT_LOCAL(FT_String *)
cff_index_get_sid_string(CFF_Index idx, FT_UInt sid, PSNames_Service psnames_interface);

FT_LOCAL(FT_Error)
cff_index_access_element(CFF_Index idx, FT_UInt element, FT_Byte **pbytes, FT_ULong *pbyte_len);

FT_LOCAL(void)
cff_index_forget_element(CFF_Index idx, FT_Byte **pbytes);

FT_LOCAL(FT_Error)
cff_font_load(FT_Stream stream, FT_Int face_index, CFF_Font font);

FT_LOCAL(void)
cff_font_done(CFF_Font font);

FT_LOCAL(FT_Byte)
cff_fd_select_get(CFF_FDSelect select, FT_UInt glyph_index);


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CFFLOAD_H */
