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
/*  ftincrem.h                                                             */
/*    FreeType incremental loading (specification).                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTINCREM_H
#define AGS_LIB_FREETYPE_FTINCREM_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


typedef struct FT_IncrementalRec_ *FT_Incremental;

typedef struct  FT_Incremental_MetricsRec_ {
	FT_Long  bearing_x;
	FT_Long  bearing_y;
	FT_Long  advance;
} FT_Incremental_MetricsRec, *FT_Incremental_Metrics;

typedef FT_Error (*FT_Incremental_GetGlyphDataFunc)(FT_Incremental incremental, FT_UInt glyph_index, FT_Data *adata);

typedef void (*FT_Incremental_FreeGlyphDataFunc)(FT_Incremental incremental, FT_Data *data);

typedef FT_Error (*FT_Incremental_GetGlyphMetricsFunc)(FT_Incremental incremental, FT_UInt glyph_index, FT_Bool vertical,
													   FT_Incremental_MetricsRec *ametrics, FT_Bool *afound);

typedef struct  FT_Incremental_FuncsRec_ {
	FT_Incremental_GetGlyphDataFunc     get_glyph_data;
	FT_Incremental_FreeGlyphDataFunc    free_glyph_data;
	FT_Incremental_GetGlyphMetricsFunc  get_glyph_metrics;
} FT_Incremental_FuncsRec;

typedef struct  FT_Incremental_InterfaceRec_ {
	const FT_Incremental_FuncsRec	*funcs;
	FT_Incremental                  object;
} FT_Incremental_InterfaceRec;


#define FT2_1_3_PARAM_TAG_INCREMENTAL FT2_1_3_MAKE_TAG('i', 'n', 'c', 'r')


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTINCREM_H */
