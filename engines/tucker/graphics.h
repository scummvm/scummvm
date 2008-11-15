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

#ifndef TUCKER_GRAPHICS_H__
#define TUCKER_GRAPHICS_H__

#include "common/util.h"

namespace Tucker {

class Graphics {
public:

	struct Charset {
		int charW;
		int charH;
		int xCount;
		int yCount;
	};

	static int encodeRLE(const uint8 *src, uint8 *dst, int w, int h);
	static int encodeRAW(const uint8 *src, uint8 *dst, int w, int h);

	static void decodeRLE(uint8 *dst, const uint8 *src, int w, int h);
	static void decodeRLE_224(uint8 *dst, const uint8 *src, int w, int h);
	static void decodeRLE_248(uint8 *dst, const uint8 *src, int w, int h, int y1, int y2, bool xflip);
	static void decodeRLE_320(uint8 *dst, const uint8 *src, int w, int h);

	static void copyFrom640(const uint8 *src, uint8 *dst, int w, int h);
	static void copyTo640(uint8 *dst, const uint8 *src, int w, int srcPitch, int h);

	static void drawStringChar(uint8 *dst, uint8 chr, int pitch, uint8 chrColor, const uint8 *src);

	static const Charset _enCharset;
	static const Charset _frCharset;
	static const Charset _creditsCharset;

	static const Charset *_charset;
};

} // namespace Tucker

#endif
