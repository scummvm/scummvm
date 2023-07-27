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
/*  ftmoderr.h                                                             */
/*                                                                         */
/*    FreeType module error offsets (specification).                       */
/*                                                                         */
/*  Copyright 2001, 2002 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef AGS_LIB_FREETYPE_FTMODERR_H
#define AGS_LIB_FREETYPE_FTMODERR_H


/* SETUP MACROS */

#undef FT_NEED_EXTERN_C

#ifndef FT_MODERRDEF

#ifdef FT_CONFIG_OPTION_USE_MODULE_ERRORS
#define FT_MODERRDEF(e, v, s) FT_Mod_Err_##e = v,
#else
#define FT_MODERRDEF(e, v, s) FT_Mod_Err_##e = 0,
#endif

#define FT_MODERR_START_LIST  enum {
#define FT_MODERR_END_LIST    FT_Mod_Err_Max };

#ifdef __cplusplus
#define FT_NEED_EXTERN_C
extern "C" {
#endif

#endif /* !FT_MODERRDEF */


/* LIST MODULE ERROR BASES */

#ifdef FT_MODERR_START_LIST
FT_MODERR_START_LIST
#endif

FT_MODERRDEF(Base,     0x000, "base module")
FT_MODERRDEF(Autohint, 0x100, "autohinter module")
FT_MODERRDEF(Cache,    0x200, "cache module")
FT_MODERRDEF(CFF,      0x300, "CFF module")
FT_MODERRDEF(CID,      0x400, "CID module")
FT_MODERRDEF(PCF,      0x500, "PCF module")
FT_MODERRDEF(PSaux,    0x600, "PS auxiliary module")
FT_MODERRDEF(PSnames,  0x700, "PS names module")
FT_MODERRDEF(Raster,   0x800, "raster module")
FT_MODERRDEF(SFNT,     0x900, "SFNT module")
FT_MODERRDEF(Smooth,   0xA00, "smooth raster module")
FT_MODERRDEF(TrueType, 0xB00, "TrueType module")
FT_MODERRDEF(Type1,    0xC00, "Type 1 module")
FT_MODERRDEF(Winfonts, 0xD00, "Windows FON/FNT module")
FT_MODERRDEF(PFR,      0xE00, "PFR module")

#ifdef FT_MODERR_END_LIST
FT_MODERR_END_LIST
#endif


/* CLEANUP */

#ifdef FT_NEED_EXTERN_C
}
#endif

#undef FT_MODERR_START_LIST
#undef FT_MODERR_END_LIST
#undef FT_MODERRDEF
#undef FT_NEED_EXTERN_C


#endif /* AGS_LIB_FREETYPE_FTMODERR_H */
