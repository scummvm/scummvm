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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_VDRAW_H
#define SAGA2_VDRAW_H

#include "saga2/gdraw.h"
#include "saga2/vpage.h"

namespace Saga2 {
class gDisplayPort : public gPort {
public:
	virtual ~gDisplayPort() {}

	vDisplayPage protoPage;

	//  Lowest-level drawing functions, (virtually) retargeted to
	//  call SVGA drawing routines

	void fillRect(const Rect16 r);

	void clear() {
		protoPage.fillRect(clip, fgPen);
	}

	//  Blitting functions
	//  NOTE: svga port cannot be used a source!!!
	void bltPixels(const gPixelMap &src,
	               int src_x, int src_y,
	               int dst_x, int dst_y,
	               int width, int height);

	//  Bresenham line-drawing functions
	void line(int16 x1, int16 y1, int16 x2, int16 y2);

	void scrollPixels(const Rect16 r, int dx, int dy);
};

extern vDisplayPage *drawPage;

} // end of namespace Saga2

#endif
