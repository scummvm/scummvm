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
/*  ahglyph.h                                                              */
/*                                                                         */
/*    Routines used to load and analyze a given glyph before hinting       */
/*    (specification).                                                     */
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


#ifndef AGS_LIB_FREETYPE_AHGLYPH_H
#define AGS_LIB_FREETYPE_AHGLYPH_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/autohint/ahtypes.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


typedef enum  AH_UV_ {
	AH_UV_FXY,
	AH_UV_FYX,
	AH_UV_OXY,
	AH_UV_OYX,
	AH_UV_OX,
	AH_UV_OY,
	AH_UV_YX,
	AH_UV_XY  /* should always be last! */
} AH_UV;


FT_LOCAL(void)
ah_setup_uv(AH_Outline outline, AH_UV source);

/* AH_OutlineRec functions - they should be typically called in this order */

FT_LOCAL(FT_Error)
ah_outline_new(FT_Memory memory, AH_Outline *aoutline);

FT_LOCAL(FT_Error)
ah_outline_load(AH_Outline outline, FT_Face face);

FT_LOCAL(void)
ah_outline_compute_segments(AH_Outline outline);

FT_LOCAL(void)
ah_outline_link_segments(AH_Outline outline);

FT_LOCAL(void)
ah_outline_detect_features(AH_Outline outline);

FT_LOCAL(void)
ah_outline_compute_blue_edges(AH_Outline outline, AH_Face_Globals globals);

FT_LOCAL(void)
ah_outline_scale_blue_edges(AH_Outline outline, AH_Face_Globals globals);

FT_LOCAL(void)
ah_outline_save(AH_Outline outline, AH_Loader loader);

FT_LOCAL(void)
ah_outline_done(AH_Outline outline);


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_AHGLYPH_H */
