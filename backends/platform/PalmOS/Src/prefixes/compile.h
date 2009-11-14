/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef __COMPILE_H__
#define __COMPILE_H__

#undef ENABLE_SCUMM
#undef ENABLE_SCUMM_7_8
#undef ENABLE_HE

#undef ENABLE_AGOS
#undef ENABLE_SKY
#undef ENABLE_SWORD1
#undef ENABLE_SWORD2
#undef ENABLE_QUEEN
#undef ENABLE_SAGA
#undef ENABLE_KYRA
#undef ENABLE_AWE
#undef ENABLE_GOB
#undef ENABLE_LURE
#undef ENABLE_CINE
#undef ENABLE_AGI
#undef ENABLE_TOUCHE
#undef ENABLE_PARALLACTION
#undef ENABLE_CRUISE
#undef ENABLE_DRASCULA

// ScummVM
#define DISABLE_HQ_SCALERS
#define DISABLE_FANCY_THEMES
//#define CT_NO_TRANSPARENCY
//#define REDUCE_MEMORY_USAGE

#include "compile_base.h"

//#define DISABLE_ADLIB
//#define DISABLE_LIGHTSPEED

#ifdef COMPILE_ZODIAC
#	undef	DISABLE_FANCY_THEMES
#	define	USE_ZLIB
// set an external ZLIB since save/load implementation
// doesn't support built-in zodiac version which is 1.1.4
// (seen inflateInit2 which err on "MAX_WBITS + 32")
#	define	USE_ZLIB_EXTERNAL
#	define	DISABLE_SONY
#endif

#ifdef COMPILE_OS5
#	define	DISABLE_TAPWAVE
#	define	USE_ZLIB
#endif

#endif
