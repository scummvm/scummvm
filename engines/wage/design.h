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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef WAGE_DESIGN_H
#define WAGE_DESIGN_H

#include "graphics/surface.h"
#include "common/memstream.h"
#include "common/rect.h"

namespace Wage {

class Design {
public:
	Design(Common::SeekableReadStream *data);
	~Design();

	void setBounds(Common::Rect *bounds) {
		_bounds = bounds;
	}

	Common::Rect *getBounds() {
		return _bounds;
	}

    void paint(Graphics::Surface *canvas, Patterns &patterns, int x, int y);
	bool isPointOpaque(int x, int y);
	static void drawRect(Graphics::Surface *surface, Common::Rect &rect, int thickness, int color, Patterns &patterns, byte fillType);
	static void drawFilledRect(Graphics::Surface *surface, Common::Rect &rect, int color, Patterns &patterns, byte fillType);
	static void drawFilledRoundRect(Graphics::Surface *surface, Common::Rect &rect, int arc, int color, Patterns &patterns, byte fillType);


private:
	byte *_data;
    int _len;
	Common::Rect *_bounds;
	Graphics::Surface *_surface;

private:
    void drawRect(Graphics::Surface *surface, Common::ReadStream &in,
        Patterns &patterns, byte fillType, byte borderThickness, byte borderFillType);
	void drawRoundRect(Graphics::Surface *surface, Common::ReadStream &in,
        Patterns &patterns, byte fillType, byte borderThickness, byte borderFillType);
	void drawPolygon(Graphics::Surface *surface, Common::ReadStream &in,
        Patterns &patterns, byte fillType, byte borderThickness, byte borderFillType);
	void drawOval(Graphics::Surface *surface, Common::ReadStream &in,
		Patterns &patterns, byte fillType, byte borderThickness, byte borderFillType);
	void drawBitmap(Graphics::Surface *surface, Common::ReadStream &in);

	void drawFilledRect(Common::Rect &rect, int color, void (*plotProc)(int, int, int, void *), void *data);
	static void drawRoundRect(Common::Rect &rect, int arc, int color, bool filled, void (*plotProc)(int, int, int, void *), void *data);
	void drawPolygonScan(int *polyX, int *polyY, int npoints, Common::Rect &bbox, int color,
									void (*plotProc)(int, int, int, void *), void *data);
	void drawEllipse(int x0, int y0, int x1, int y1, bool filled, void (*plotProc)(int, int, int, void *), void *data);
	static void drawHLine(int x1, int x2, int y, int color, void (*plotProc)(int, int, int, void *), void *data);
	static void drawVLine(int x, int y1, int y2, int color, void (*plotProc)(int, int, int, void *), void *data);
	void drawThickLine (int x1, int y1, int x2, int y2, int thick, int color,
									void (*plotProc)(int, int, int, void *), void *data);
};

class FloodFill {
public:
	FloodFill(Graphics::Surface *surface, byte color1, byte color2);
	~FloodFill();
	void addSeed(int x, int y);
	void fill();

private:
	Common::List<Common::Point *> _queue;
	Graphics::Surface *_surface;
	byte _color1, _color2;
	byte *_visited;
	int _w, _h;
};

} // End of namespace Wage

#endif
