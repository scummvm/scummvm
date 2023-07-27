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
/*  ft2build.h                                                             */
/*                                                                         */
/*    FreeType 2 build and setup macros.                                   */
/*    (Generic version)                                                    */
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


/*************************************************************************/
/*                                                                       */
/* This file corresponds to the default "ft2build.h" file for            */
/* FreeType 2.  It uses the "freetype" include root.                     */
/*                                                                       */
/* Note that specific platforms might use a different configuration.     */
/* See builds/unix/ft2unix.h for an example.                             */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_FT213_BUILD_H
#define AGS_LIB_FREETYPE_FT213_BUILD_H


#ifdef __cplusplus
#define FT_BEGIN_HEADER  extern "C" {
#define FT_END_HEADER  }
#else
#define FT_BEGIN_HEADER  /* nothing */
#define FT_END_HEADER
#endif

#endif /* AGS_LIB_FREETYPE_FT213_BUILD_H */