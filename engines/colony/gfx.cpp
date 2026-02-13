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

#include "common/system.h"
#include "common/util.h"
#include "colony/gfx.h"
#include "graphics/font.h"

namespace Colony {

Gfx::Gfx(OSystem *system, int width, int height) : _system(system), _width(width), _height(height) {
	_surface.create(width, height, _system->getScreenFormat());
}

Gfx::~Gfx() {
	_surface.free();
}

void Gfx::clear(uint32 color) {
	_surface.clear(color);
}

void Gfx::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	_surface.drawLine(x1, y1, x2, y2, color);
}

void Gfx::drawRect(const Common::Rect &rect, uint32 color) {
	_surface.frameRect(rect, color);
}

void Gfx::fillRect(const Common::Rect &rect, uint32 color) {
	_surface.fillRect(rect, color);
}

void Gfx::drawString(const Graphics::Font *font, const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align) {
	font->drawString(&_surface, str, x, y, (align == Graphics::kTextAlignCenter && x == 0) ? _width : (_width - x), color, align);
}

void Gfx::scroll(int dx, int dy, uint32 background) {
	if (abs(dx) >= _width || abs(dy) >= _height) {
		clear(background);
		return;
	}

	Graphics::ManagedSurface tmp;
	tmp.create(_width, _height, _surface.format);
	tmp.blitFrom(_surface);

	clear(background);
	_surface.blitFrom(tmp, Common::Rect(0, 0, _width, _height), Common::Rect(dx, dy, dx + _width, dy + _height));
	tmp.free();
}

void Gfx::drawEllipse(int x, int y, int rx, int ry, uint32 color) {
	_surface.drawEllipse(x - rx, y - ry, x + rx, y + ry, color, false);
}

void Gfx::fillEllipse(int x, int y, int rx, int ry, uint32 color) {
	_surface.drawEllipse(x - rx, y - ry, x + rx, y + ry, color, true);
}

void Gfx::fillDitherRect(const Common::Rect &rect, uint32 color1, uint32 color2) {
	for (int y = rect.top; y < rect.bottom; y++) {
		for (int x = rect.left; x < rect.right; x++) {
			if ((x + y) % 2 == 0)
				_surface.setPixel(x, y, color1);
			else
				_surface.setPixel(x, y, color2);
		}
	}
}

void Gfx::copyToScreen() {
	_system->copyRectToScreen(_surface.getPixels(), _surface.pitch, 0, 0, _surface.w, _surface.h);
	_system->updateScreen();
}

} // End of namespace Colony
