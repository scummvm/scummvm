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
/*  ahangles.h                                                             */
/*                                                                         */
/*    A routine used to compute vector angles with limited accuracy        */
/*    and very high speed (specification).                                 */
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


#ifndef AGS_LIB_FREETYPE_AHANGLES_H
#define AGS_LIB_FREETYPE_AHANGLES_H


#include <ft2build.h>
#include "ags/lib/freetype-2.1.3/autohint/ahtypes.h"

namespace AGS3 {
namespace FreeType213 {


/* PI expressed in ah_angles -- we don't really need an important */
/* precision, so 256 should be enough                             */
#define AH_PI 	   256
#define AH_2PI 	   (AH_PI * 2)
#define AH_HALF_PI (AH_PI / 2)
#define AH_2PIMASK (AH_2PI - 1)

/* the number of bits used to express an arc tangent; */
/* see the structure of the lookup table              */
#define AH_ATAN_BITS  8

extern const AH_Angle ah_arctan[1L << AH_ATAN_BITS];

AH_Angle ah_angle(FT_Vector *v);
AH_Angle ah_angle_diff(AH_Angle angle1, AH_Angle angle2);


} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_AHANGLES_H */
