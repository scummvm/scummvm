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

#ifndef GOB_DRIVER_VGA_H
#define GOB_DRIVER_VGA_H

#include "gob/video.h"

namespace Gob {

class VGAVideoDriver : public VideoDriver {
public:
	VGAVideoDriver() {}
	virtual ~VGAVideoDriver() {}

	void putPixel(int16 x, int16 y, byte color, SurfaceDesc &dest);
	void drawLine(SurfaceDesc &dest, int16 x0, int16 y0,
			int16 x1, int16 y1, byte color);
	void fillRect(SurfaceDesc &dest, int16 left, int16 top,
			int16 right, int16 bottom, byte color);
	void drawLetter(unsigned char item, int16 x, int16 y,
			Video::FontDesc *fontDesc, byte color1, byte color2,
			byte transp, SurfaceDesc &dest);
	void drawSprite(SurfaceDesc &source, SurfaceDesc &dest, int16 left,
			int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp);
	void drawSpriteDouble(SurfaceDesc &source, SurfaceDesc &dest, int16 left,
			int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp);
	void drawPackedSprite(byte *sprBuf, int16 width, int16 height,
			int16 x, int16 y, byte transp, SurfaceDesc &dest);
};

}

#endif // GOB_DRIVER_VGA_H
