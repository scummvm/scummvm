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
 */

#ifndef COLONY_GFX_H
#define COLONY_GFX_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "graphics/font.h"

namespace Colony {

class Gfx {
public:
	Gfx(OSystem *system, int width, int height);
	~Gfx();

	void clear(uint32 color);
	void drawLine(int x1, int y1, int x2, int y2, uint32 color);
	void drawRect(const Common::Rect &rect, uint32 color);
	void fillRect(const Common::Rect &rect, uint32 color);
	
	void drawString(const Graphics::Font *font, const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align = Graphics::kTextAlignLeft);
	void scroll(int dx, int dy, uint32 background);
	
	void copyToScreen();

	uint32 white() { return 255; }
	uint32 black() { return 0; }

private:
	OSystem *_system;
	Graphics::ManagedSurface _surface;
	int _width;
	int _height;
};

} // End of namespace Colony

#endif
