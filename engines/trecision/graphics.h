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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TRECISION_GRAPHICS_H
#define TRECISION_GRAPHICS_H

#include "graphics/pixelformat.h"

namespace Trecision {
class TrecisionEngine;

class GraphicsManager {
private:
	TrecisionEngine *_vm;

	Graphics::PixelFormat _screenFormat;
	uint16 _bitMask[3];

	static const Graphics::PixelFormat kImageFormat;

public:
	GraphicsManager(TrecisionEngine *vm);
	~GraphicsManager();
	
	uint16 *_screenPtr;

	bool initScreen();

	void lock();
	void unlock();
	void clearScreen();
	void copyToScreen(int px, int py, int dx, int dy);

	void vCopy(uint32 Sco, uint16 *Src, uint32 Len);

	uint16 palTo16bit(uint8 r, uint8 g, uint8 b) const;
	void updatePixelFormat(uint16 *p, uint32 len) const;
	uint16 restorePixelFormat(uint16 t) const;

	uint16 shadow(uint32 val, uint8 num);
	uint16 aliasing(uint32 val1, uint32 val2, uint8 num);
}; // end of class

} // end of namespace
#endif
