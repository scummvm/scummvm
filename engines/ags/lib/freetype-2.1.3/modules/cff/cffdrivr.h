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
/*  cffdrivr.h                                                             */
/*    High-level OpenType driver interface (specification).                */
/*                                                                         */
/***************************************************************************/

#ifndef AGS_LIB_FREETYPE_CFFDRIVER_H
#define AGS_LIB_FREETYPE_CFFDRIVER_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftdriver.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


FT_CALLBACK_TABLE const FT_Driver_ClassRec cff_driver_class;


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CFFDRIVER_H */
