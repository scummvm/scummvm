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
/*  ftgrays.h                                                              */
/*                                                                         */
/*    FreeType smooth renderer declaration                                 */
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


#ifndef AGS_LIB_FREETYPE_FTGRAYS_H
#define AGS_LIB_FREETYPE_FTGRAYS_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _STANDALONE_
#include "ftimage.h"
#else
#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftimage.h"
#endif

namespace AGS3 {
namespace FreeType213 {


/*************************************************************************/
/*                                                                       */
/* To make ftgrays.h independent from configuration files we check       */
/* whether FT_EXPORT_VAR has been defined already.                       */
/*                                                                       */
/* On some systems and compilers (Win32 mostly), an extra keyword is     */
/* necessary to compile the library as a DLL.                            */
/*                                                                       */
#ifndef FT_EXPORT_VAR
#define FT_EXPORT_VAR(x) extern x
#endif

FT_EXPORT_VAR(const FT_Raster_Funcs)
ft_grays_raster;

#ifdef __cplusplus
}
#endif


} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTGRAYS_H */
