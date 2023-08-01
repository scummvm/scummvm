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
/*  ftcalc.h                                                               */
/*                                                                         */
/*    Arithmetic computations (specification).                             */
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


#ifndef AGS_LIB_FREETYPE_FTCALC_H
#define AGS_LIB_FREETYPE_FTCALC_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"


// FT_BEGIN_HEADER

#define SQRT_32(x) FT_Sqrt32(x)

namespace AGS3 {
namespace FreeType213 {

FT_EXPORT(FT_Int32)
FT_SqrtFixed(FT_Int32 x);

FT_EXPORT(FT_Int32)
FT_Sqrt32(FT_Int32 x);

} // End of namespace FreeType213
} // End of namespace AGS3

#define INT_TO_F26DOT6(x) 	 ((FT_Long)(x) << 6)
#define INT_TO_F2DOT14(x) 	 ((FT_Long)(x) << 14)
#define INT_TO_FIXED(x) 	 ((FT_Long)(x) << 16)
#define F2DOT14_TO_FIXED(x)	 ((FT_Long)(x) << 2)
#define FLOAT_TO_FIXED(x) 	 ((FT_Long)(x * 65536.0))

#define ROUND_F26DOT6(x) 	 (x >= 0 ? (((x) + 32) & -64) \
									 : (-((32 - (x)) & -64)))

// FT_END_HEADER

#endif /* AGS_LIB_FREETYPE_FTCALC_H */
