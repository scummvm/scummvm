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
/*  ahloader.h                                                             */
/*    Glyph loader for the auto-hinting module (declaration only).         */
/*                                                                         */
/***************************************************************************/



#ifndef AGS_LIB_FREETYPE_AHLOADER_H
#define AGS_LIB_FREETYPE_AHLOADER_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"


FT2_1_3_BEGIN_HEADER

#include "engines/ags/lib/freetype-2.1.3/ftgloadr.h"

#define AH_Load    FT_GlyphLoad
#define AH_Loader  FT_GlyphLoader

#define ah_loader_new              FT_GlyphLoader_New
#define ah_loader_done             FT_GlyphLoader_Done
#define ah_loader_reset            FT_GlyphLoader_Reset
#define ah_loader_rewind           FT_GlyphLoader_Rewind
#define ah_loader_create_extra     FT_GlyphLoader_CreateExtra
#define ah_loader_check_points     FT_GlyphLoader_CheckPoints
#define ah_loader_check_subglyphs  FT_GlyphLoader_CheckSubGlyphs
#define ah_loader_prepare          FT_GlyphLoader_Prepare
#define ah_loader_add              FT_GlyphLoader_Add
#define ah_loader_copy_points      FT_GlyphLoader_CopyPoints


FT2_1_3_END_HEADER

#endif /* AGS_LIB_FREETYPE_AHLOADER_H */
