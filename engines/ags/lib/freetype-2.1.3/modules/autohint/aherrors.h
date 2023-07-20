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
/*  aherrors.h                                                             */
/*    Autohinter error codes (specification only).                         */
/*                                                                         */
/***************************************************************************/


/*************************************************************************/
/*                                                                       */
/* This file is used to define the Autohinter error enumeration          */
/* constants.                                                            */
/*                                                                       */
/*************************************************************************/

#ifndef AGS_LIB_FREETYPE_AHERRORS_H
#define AGS_LIB_FREETYPE_AHERRORS_H

#include "engines/ags/lib/freetype-2.1.3/ftmoderr.h"

#undef AGS_LIB_FREETYPE_FTERRORS_H

#define FT2_1_3_ERR_PREFIX  AH_Err_
#define FT2_1_3_ERR_BASE    FT2_1_3_Mod_Err_Autohint

#include "engines/ags/lib/freetype-2.1.3/fterrors.h"

#endif /* AGS_LIB_FREETYPE_AHERRORS_H */
