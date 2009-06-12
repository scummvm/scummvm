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

#ifndef ASYLUM_SCREEN_H
#define ASYLUM_SCREEN_H

#include "common/scummsys.h"
#include "common/rect.h"

class OSystem;

#define	SCREEN_WIDTH	640
#define	SCREEN_DEPTH	480

#define PAL_SIZE		256
#define BRIGHTNESS		6

namespace Asylum {

class Screen {
public:
    Screen(OSystem *system);
    ~Screen();

	void setFrontBuffer(int32 x, int32 y, int32 width, int32 height, uint8 *buffer);
	void setBackBuffer(int32 x, int32 y, int32 width, int32 height, uint8 *buffer);
	void copyBuffer(int32 x, int32 y, int32 width, int32 height, uint8 *src, uint8 *dst);
	void copyBuffer(int32 xs, int32 ys, int32 xd, int32 yd, int32 width, int32 height, uint8 *src, uint8 *dst);
	void copyBuffer(int32 xs, int32 ys, int32 xd, int32 yd, int32 width, int32 height, uint8 *src, uint8 *dst, int32 dstWidth);

	void clearScreen();
	void updateScreen();
	void updateScreen(int32 x, int32 y, int32 width, int32 height);

	void setPalette(uint8 *palette);

	void setGammaLevel(double gamma);

	void drawLine(int32 x0, int32 y0, int32 x1, int32 y1, uint8 colour);

private:
	OSystem *_system;

	uint8  *_frontBuf;
	uint8  *_backBuf;
	bool   _fullRefresh;

	uint8 _currentPalette[PAL_SIZE * 4];
	bool   _updatePalette;

}; // end of class Screen

} // end of namespace Asylum

#endif
