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
/*  tttags.h                                                               */
/*    Tags for TrueType tables (specification only).                       */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_TTAGS_H
#define AGS_LIB_FREETYPE_TTAGS_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"


FT_BEGIN_HEADER


#define TTAG_cmap  FT_MAKE_TAG('c', 'm', 'a', 'p')
#define TTAG_cvt   FT_MAKE_TAG('c', 'v', 't', ' ')
#define TTAG_CFF   FT_MAKE_TAG('C', 'F', 'F', ' ')
#define TTAG_DSIG  FT_MAKE_TAG('D', 'S', 'I', 'G')
#define TTAG_bhed  FT_MAKE_TAG('b', 'h', 'e', 'd')
#define TTAG_bdat  FT_MAKE_TAG('b', 'd', 'a', 't')
#define TTAG_bloc  FT_MAKE_TAG('b', 'l', 'o', 'c')
#define TTAG_EBDT  FT_MAKE_TAG('E', 'B', 'D', 'T')
#define TTAG_EBLC  FT_MAKE_TAG('E', 'B', 'L', 'C')
#define TTAG_EBSC  FT_MAKE_TAG('E', 'B', 'S', 'C')
#define TTAG_fpgm  FT_MAKE_TAG('f', 'p', 'g', 'm')
#define TTAG_fvar  FT_MAKE_TAG('f', 'v', 'a', 'r')
#define TTAG_gasp  FT_MAKE_TAG('g', 'a', 's', 'p')
#define TTAG_glyf  FT_MAKE_TAG('g', 'l', 'y', 'f')
#define TTAG_GSUB  FT_MAKE_TAG('G', 'S', 'U', 'B')
#define TTAG_hdmx  FT_MAKE_TAG('h', 'd', 'm', 'x')
#define TTAG_head  FT_MAKE_TAG('h', 'e', 'a', 'd')
#define TTAG_hhea  FT_MAKE_TAG('h', 'h', 'e', 'a')
#define TTAG_hmtx  FT_MAKE_TAG('h', 'm', 't', 'x')
#define TTAG_kern  FT_MAKE_TAG('k', 'e', 'r', 'n')
#define TTAG_loca  FT_MAKE_TAG('l', 'o', 'c', 'a')
#define TTAG_LTSH  FT_MAKE_TAG('L', 'T', 'S', 'H')
#define TTAG_maxp  FT_MAKE_TAG('m', 'a', 'x', 'p')
#define TTAG_MMSD  FT_MAKE_TAG('M', 'M', 'S', 'D')
#define TTAG_MMFX  FT_MAKE_TAG('M', 'M', 'F', 'X')
#define TTAG_name  FT_MAKE_TAG('n', 'a', 'm', 'e')
#define TTAG_OS2   FT_MAKE_TAG('O', 'S', '/', '2')
#define TTAG_OTTO  FT_MAKE_TAG('O', 'T', 'T', 'O')
#define TTAG_PCLT  FT_MAKE_TAG('P', 'C', 'L', 'T')
#define TTAG_post  FT_MAKE_TAG('p', 'o', 's', 't')
#define TTAG_prep  FT_MAKE_TAG('p', 'r', 'e', 'p')
#define TTAG_true  FT_MAKE_TAG('t', 'r', 'u', 'e')
#define TTAG_ttc   FT_MAKE_TAG('t', 't', 'c', ' ')
#define TTAG_ttcf  FT_MAKE_TAG('t', 't', 'c', 'f')
#define TTAG_VDMX  FT_MAKE_TAG('V', 'D', 'M', 'X')
#define TTAG_vhea  FT_MAKE_TAG('v', 'h', 'e', 'a')
#define TTAG_vmtx  FT_MAKE_TAG('v', 'm', 't', 'x')


FT_END_HEADER

#endif /* AGS_LIB_FREETYPE_TTAGS_H */
