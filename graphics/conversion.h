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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/graphics/conversion.h $
 * $Id: conversion.h 43626 2009-08-22 00:27:13Z dhewg $
 *
 */

#ifndef GRAPHICS_CONVERSION_H
#define GRAPHICS_CONVERSION_H

#include "common/util.h"
#include "graphics/pixelformat.h"

namespace Graphics {

/** Converting a color from YUV to RGB colorspace. */
inline static void YUV2RGB(byte y, byte u, byte v, byte &r, byte &g, byte &b) {
	r = CLIP<int>(y + ((1357 * (v - 128)) >> 10), 0, 255);
	g = CLIP<int>(y - (( 691 * (v - 128)) >> 10) - ((333 * (u - 128)) >> 10), 0, 255);
	b = CLIP<int>(y + ((1715 * (u - 128)) >> 10), 0, 255);
}

/** Converting a color from RGB to YUV colorspace. */
inline static void RGB2YUV(byte r, byte g, byte b, byte &y, byte &u, byte &v) {
	y = CLIP<int>( ((r * 306) >> 10) + ((g * 601) >> 10) + ((b * 117) >> 10)      , 0, 255);
	u = CLIP<int>(-((r * 172) >> 10) - ((g * 340) >> 10) + ((b * 512) >> 10) + 128, 0, 255);
	v = CLIP<int>( ((r * 512) >> 10) - ((g * 429) >> 10) - ((b *  83) >> 10) + 128, 0, 255);
}

} // end of namespace Graphics

#endif // GRAPHICS_CONVERSION_H
