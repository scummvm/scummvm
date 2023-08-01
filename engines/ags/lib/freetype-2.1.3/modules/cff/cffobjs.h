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
/*  cffobjs.h                                                              */
/*                                                                         */
/*    OpenType objects manager (specification).                            */
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


#ifndef AGS_LIB_FREETYPE_CFFOBJS_H
#define AGS_LIB_FREETYPE_CFFOBJS_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cfftypes.h"
#include "engines/ags/lib/freetype-2.1.3/tttypes.h"
#include "engines/ags/lib/freetype-2.1.3/psnames.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


typedef struct CFF_DriverRec_ *CFF_Driver;

typedef TT_Face  CFF_Face;
typedef FT_Size  CFF_Size;

typedef struct CFF_GlyphSlotRec_ {
	FT_GlyphSlotRec  root;

	FT_Bool          hint;
	FT_Bool          scaled;

	FT_Fixed         x_scale;
	FT_Fixed         y_scale;
} CFF_GlyphSlotRec, *CFF_GlyphSlot;


typedef struct CFF_Transform_ {
	FT_Fixed    xx, xy;     /* transformation matrix coefficients */
	FT_Fixed    yx, yy;
	FT_F26Dot6  ox, oy;     /* offsets        */
} CFF_Transform;


/* this is only used in the case of a pure CFF font with no charmap */
typedef struct  CFF_CharMapRec_ {
	TT_CharMapRec  root;
	PS_Unicodes    unicodes;
} CFF_CharMapRec, *CFF_CharMap;


/* TrueType driver class.                                              */
typedef struct  CFF_DriverRec_ {
	FT_DriverRec  root;
	void          *extension_component;
} CFF_DriverRec;

FT_LOCAL(FT_Error)
cff_size_init(CFF_Size size);

FT_LOCAL(void)
cff_size_done(CFF_Size size);

FT_LOCAL(FT_Error)
cff_size_reset(CFF_Size size);

// Conversion function to silence incompatible function pointer warnings
FT_LOCAL(FT_Error)
cff_pixels_size_reset(CFF_Size size, FT_UInt pixel_width, FT_UInt pixel_height);
// Conversion function to silence incompatible function pointer warnings
FT_LOCAL(FT_Error)
cff_points_size_reset(CFF_Size size, FT_F26Dot6 char_width, FT_F26Dot6 char_height, FT_UInt horz_resolution, FT_UInt vert_resolution);

FT_LOCAL(void)
cff_slot_done(CFF_GlyphSlot slot);

FT_LOCAL(FT_Error)
cff_slot_init(CFF_GlyphSlot slot);


/* Face functions */

FT_LOCAL(FT_Error)
cff_face_init(FT_Stream stream, CFF_Face face, FT_Int face_index, FT_Int num_params, FT_Parameter *params);

FT_LOCAL(void)
cff_face_done(CFF_Face face);


/* Driver functions */

FT_LOCAL(FT_Error)
cff_driver_init(CFF_Driver driver);

FT_LOCAL(void)
cff_driver_done(CFF_Driver driver);


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CFFOBJS_H */
