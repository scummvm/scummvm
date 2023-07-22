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
/*  fttrigon.h                                                             */
/*    FreeType trigonometric functions (specification).                    */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTTRIGON_H
#define AGS_LIB_FREETYPE_FTTRIGON_H

#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


typedef FT_Fixed  FT_Angle;

#define FT2_1_3_ANGLE_PI  (180L << 16)
#define FT2_1_3_ANGLE_2PI (FT2_1_3_ANGLE_PI * 2)
#define FT2_1_3_ANGLE_PI2 (FT2_1_3_ANGLE_PI / 2)
#define FT2_1_3_ANGLE_PI4 (FT2_1_3_ANGLE_PI / 4)


FT_EXPORT(FT_Fixed)
FT_Sin(FT_Angle angle);

FT_EXPORT(FT_Fixed)
FT_Cos(FT_Angle angle);

FT_EXPORT(FT_Fixed)
FT_Tan(FT_Angle angle);

FT_EXPORT(FT_Angle)
FT_Atan2(FT_Fixed x, FT_Fixed y);

FT_EXPORT(FT_Angle)
FT_Angle_Diff(FT_Angle angle1, FT_Angle angle2);

FT_EXPORT(void)
FT_Vector_Unit(FT_Vector *vec, FT_Angle angle);

FT_EXPORT(void)
FT_Vector_Rotate(FT_Vector *vec, FT_Angle angle);

FT_EXPORT(FT_Fixed)
FT_Vector_Length(FT_Vector *vec);

FT_EXPORT(void)
FT_Vector_Normalize(FT_Vector *vec);

FT_EXPORT(void)
FT_Vector_Polarize(FT_Vector *vec, FT_Fixed *length, FT_Angle *angle);

FT_EXPORT(void)
FT_Vector_From_Polar(FT_Vector *vec, FT_Fixed length, FT_Angle angle);


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTTRIGON_H */
