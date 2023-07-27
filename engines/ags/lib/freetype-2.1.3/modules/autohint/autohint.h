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
/*  autohint.h                                                             */
/*                                                                         */
/*    High-level `autohint' module-specific interface (specification).     */
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


/*************************************************************************/
/*                                                                       */
/* The auto-hinter is used to load and automatically hint glyphs if a    */
/* format-specific hinter isn't available.                               */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_AUTOHINT_H
#define AGS_LIB_FREETYPE_AUTOHINT_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


typedef struct FT_AutoHinterRec_ *FT_AutoHinter;
typedef void (*FT_AutoHinter_GlobalGetFunc)(FT_AutoHinter hinter, FT_Face face, void **global_hints, long *global_len);
typedef void (*FT_AutoHinter_GlobalDoneFunc)(FT_AutoHinter hinter, void *global);
typedef void (*FT_AutoHinter_GlobalResetFunc)(FT_AutoHinter hinter, FT_Face face);
typedef FT_Error (*FT_AutoHinter_GlyphLoadFunc)(FT_AutoHinter hinter, FT_GlyphSlot slot, FT_Size size, FT_UInt glyph_index, FT_Int32 load_flags);

typedef struct FT_AutoHinter_ServiceRec_ {
	FT_AutoHinter_GlobalResetFunc reset_face;
	FT_AutoHinter_GlobalGetFunc   get_global_hints;
	FT_AutoHinter_GlobalDoneFunc  done_global_hints;
	FT_AutoHinter_GlyphLoadFunc   load_glyph;
} FT_AutoHinter_ServiceRec, *FT_AutoHinter_Service;


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_AUTOHINT_H */
