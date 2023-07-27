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
/*  ftmm.h                                                                 */
/*                                                                         */
/*    FreeType Multiple Master font interface (specification).             */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTMM_H
#define AGS_LIB_FREETYPE_FTMM_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/t1tables.h"

namespace AGS3 {
namespace FreeType213 {


FT_BEGIN_HEADER


typedef struct  FT_MM_Axis_ {
	FT_String	*name;
	FT_Long     minimum;
	FT_Long     maximum;
} FT_MM_Axis;


typedef struct  FT_Multi_Master_ {
	FT_UInt     num_axis;
	FT_UInt     num_designs;
	FT_MM_Axis  axis[T1_MAX_MM_AXIS];
} FT_Multi_Master;


typedef FT_Error (*FT_Get_MM_Func)(FT_Face face, FT_Multi_Master *master);

typedef FT_Error (*FT_Set_MM_Design_Func)(FT_Face face, FT_UInt num_coords, FT_Long *coords);

typedef FT_Error (*FT_Set_MM_Blend_Func)(FT_Face face, FT_UInt num_coords, FT_Long *coords);

FT_EXPORT(FT_Error)
FT_Get_Multi_Master(FT_Face face, FT_Multi_Master *amaster);

FT_EXPORT(FT_Error)
FT_Set_MM_Design_Coordinates(FT_Face face, FT_UInt num_coords, FT_Long *coords);

FT_EXPORT(FT_Error)
FT_Set_MM_Blend_Coordinates(FT_Face face, FT_UInt num_coords, FT_Fixed *coords);


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTMM_H */
