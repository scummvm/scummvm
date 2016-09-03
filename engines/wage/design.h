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

#include "common/memstream.h"
#include "common/rect.h"

#include "graphics/nine_patch.h"
#include "graphics/transparent_surface.h"

#include "graphics/macgui/macwindowmanager.h"

namespace Wage {

using namespace Graphics::MacGUIConstants;

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

    void paint(Graphics::ManagedSurface *canvas, Graphics::MacPatterns &patterns, int x, int y);
	bool isPointOpaque(int x, int y);
	static void drawRect(Graphics::ManagedSurface *surface, Common::Rect &rect, int thickness, int color, Graphics::MacPatterns &patterns, byte fillType);
	static void drawRect(Graphics::ManagedSurface *surface, int x1, int y1, int x2, int y2, int thickness, int color, Graphics::MacPatterns &patterns, byte fillType);
	static void drawFilledRect(Graphics::ManagedSurface *surface, Common::Rect &rect, int color, Graphics::MacPatterns &patterns, byte fillType);
	static void drawFilledRoundRect(Graphics::ManagedSurface *surface, Common::Rect &rect, int arc, int color, Graphics::MacPatterns &patterns, byte fillType);
	static void drawHLine(Graphics::ManagedSurface *surface, int x1, int x2, int y, int thickness, int color, Graphics::MacPatterns &patterns, byte fillType);
	static void drawVLine(Graphics::ManagedSurface *surface, int x, int y1, int y2, int thickness, int color, Graphics::MacPatterns &patterns, byte fillType);

	bool isBoundsCalculation() { return _boundsCalculationMode; }
	void adjustBounds(int16 x, int16 y);

private:
	byte *_data;
	int _len;
	Common::Rect *_bounds;
	Graphics::ManagedSurface *_surface;
	bool _boundsCalculationMode;

private:
	void render(Graphics::MacPatterns &patterns);
	void drawRect(Graphics::ManagedSurface *surface, Common::ReadStream &in,
		Graphics::MacPatterns &patterns, byte fillType, byte borderThickness, byte borderFillType);
	void drawRoundRect(Graphics::ManagedSurface *surface, Common::ReadStream &in,
		Graphics::MacPatterns &patterns, byte fillType, byte borderThickness, byte borderFillType);
	void drawPolygon(Graphics::ManagedSurface *surface, Common::ReadStream &in,
		Graphics::MacPatterns &patterns, byte fillType, byte borderThickness, byte borderFillType);
	void drawOval(Graphics::ManagedSurface *surface, Common::ReadStream &in,
		Graphics::MacPatterns &patterns, byte fillType, byte borderThickness, byte borderFillType);
	void drawBitmap(Graphics::ManagedSurface *surface, Common::SeekableReadStream &in);
};

} // End of namespace Wage

#endif
