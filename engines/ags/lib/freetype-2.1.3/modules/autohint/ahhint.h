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
/*  ahhint.h                                                               */
/*                                                                         */
/*    Glyph hinter (declaration).                                          */
/*                                                                         */
/*  Copyright 2000-2001, 2002 Catharon Productions Inc.                    */
/*  Author: David Turner                                                   */
/*                                                                         */
/*  This file is part of the Catharon Typography Project and shall only    */
/*  be used, modified, and distributed under the terms of the Catharon     */
/*  Open Source License that should come with this file under the name     */
/*  `CatharonLicense.txt'.  By continuing to use, modify, or distribute    */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*  Note that this license is compatible with the FreeType license.        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_AHHINT_H
#define AGS_LIB_FREETYPE_AHHINT_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/autohint/ahglobal.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


#define AH_HINT_DEFAULT        0
#define AH_HINT_NO_ALIGNMENT   1
#define AH_HINT_NO_HORZ_EDGES  0x200000L  /* temporary hack */
#define AH_HINT_NO_VERT_EDGES  0x400000L  /* temporary hack */

/* create a new empty hinter object */
FT_LOCAL(FT_Error)
ah_hinter_new(FT_Library library, AH_Hinter *ahinter);

/* Load a hinted glyph in the hinter */
FT_LOCAL(FT_Error)
ah_hinter_load_glyph(AH_Hinter hinter, FT_GlyphSlot slot, FT_Size size, FT_UInt glyph_index, FT_Int32 load_flags);

/* finalize a hinter object */
FT_LOCAL(void)
ah_hinter_done(AH_Hinter hinter);

FT_LOCAL(void)
ah_hinter_done_face_globals(AH_Face_Globals globals);

FT_LOCAL(void)
ah_hinter_get_global_hints(AH_Hinter hinter, FT_Face face, void **global_hints, long *global_len);

FT_LOCAL(void)
ah_hinter_done_global_hints(AH_Hinter hinter, void *global_hints);


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_AHHINT_H */
