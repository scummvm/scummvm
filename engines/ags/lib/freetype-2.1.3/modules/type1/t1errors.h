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
/*  t1errors.h                                                             */
/*    Type 1 error codes (specification only).                             */
/*                                                                         */
/***************************************************************************/


/*************************************************************************/
/*                                                                       */
/* This file is used to define the Type 1 error enumeration constants.   */
/*                                                                       */
/*************************************************************************/

#ifndef AGS_LIB_FREETYPE_T1ERRORS_H
#define AGS_LIB_FREETYPE_T1ERRORS_H

#include "engines/ags/lib/freetype-2.1.3/ftmoderr.h"

#undef AGS_LIB_FREETYPE_TTERRORS_H

#define FT2_1_3_ERR_PREFIX T1_Err_
#define FT2_1_3_ERR_BASE FT2_1_3_Mod_Err_Type1

#include "engines/ags/lib/freetype-2.1.3/fterrors.h"

#endif /* AGS_LIB_FREETYPE_T1ERRORS_H */