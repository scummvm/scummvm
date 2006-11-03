/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $Id: $
 *
 */

#ifndef TOUCHE_GRAPHICS_H
#define TOUCHE_GRAPHICS_H

#include "common/util.h"

namespace Touche {

struct Graphics {
	enum {
		kVFlipped    = 1 << 0,
		kHFlipped    = 1 << 1,
		kTransparent = 1 << 2
	};

	static int getStringWidth16(const char *str);
	static int getCharWidth16(uint8 chr);
	static void drawString16(uint8 *dst, int dstPitch, uint16 color, int x, int y, const char *str);
	static int drawChar16(uint8 *dst, int dstPitch, uint8 chr, int x, int y, uint16 color);
	static void fillRect(uint8 *dst, int dstPitch, int x, int y, int w, int h, uint8 color);
	static void drawRect(uint8 *dst, int dstPitch, int x, int y, int w, int h, uint8 color1, uint8 color2);
	static void drawLineHV(uint8 *dst, int dstPitch, int x1, int y1, int x2, int y2, uint8 color);
	static void drawLine(uint8 *dst, int dstPitch, int x1, int y1, int x2, int y2, uint8 color);
	static void copyRect(uint8 *dst, int dstPitch, int dstX, int dstY, const uint8 *src, int srcPitch, int srcX, int srcY, int w, int h, int flags = 0);
	static void copyMask(uint8 *dst, int dstPitch, int dstX, int dstY, const uint8 *src, int srcPitch, int srcX, int srcY, int w, int h, uint8 fillColor);

	static const uint16 _fontOffs[];
	static const uint8 _fontData[];
};

} // namespace Touche

#endif
