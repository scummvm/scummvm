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

#ifndef AGS_LIB_FREETYPE_PFR_H
#define AGS_LIB_FREETYPE_PFR_H

#include "engines/ags/lib/freetype-2.1.3/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/ft213build.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


typedef FT_Error (*FT2_1_3_PFR_GetMetricsFunc)(FT_Face face, FT_UInt *aoutline, FT_UInt *ametrics,
											   FT_Fixed *ax_scale, FT_Fixed *ay_scale);

typedef FT_Error (*FT2_1_3_PFR_GetKerningFunc)(FT_Face face, FT_UInt left, FT_UInt right, FT_Vector *avector);

typedef FT_Error (*FT2_1_3_PFR_GetAdvanceFunc)(FT_Face face, FT_UInt gindex, FT_Pos *aadvance);

typedef struct FT2_1_3_PFR_ServiceRec_ {
	FT2_1_3_PFR_GetMetricsFunc get_metrics;
	FT2_1_3_PFR_GetKerningFunc get_kerning;
	FT2_1_3_PFR_GetAdvanceFunc get_advance;
} FT2_1_3_PFR_ServiceRec, *FT2_1_3_PFR_Service;

#define FT2_1_3_PFR_SERVICE_NAME "pfr"


/**** FUNCTIONS ****/

FT2_1_3_EXPORT(FT_Error)
FT_Get_PFR_Metrics(FT_Face face, FT_UInt *aoutline_resolution, FT_UInt *ametrics_resolution,
				   FT_Fixed *ametrics_x_scale, FT_Fixed *ametrics_y_scale);

FT2_1_3_EXPORT(FT_Error)
FT_Get_PFR_Kerning(FT_Face face, FT_UInt left, FT_UInt right, FT_Vector *avector);

FT2_1_3_EXPORT(FT_Error)
FT_Get_PFR_Advance(FT_Face face, FT_UInt gindex, FT_Pos *aadvance);


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PFR_H */
