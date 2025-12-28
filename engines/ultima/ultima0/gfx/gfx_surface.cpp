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

#include "ultima/ultima0/gfx/gfx_surface.h"
#include "ultima/ultima0/gfx/font.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Gfx {


void GfxSurface::init() {
	_textColor = g_engine->_palette.findBestColor(0, 255, 0);
}

void GfxSurface::writeString(const char *fmt, ...) {
	va_list alist;
	va_start(alist, fmt);
	Common::String str = Common::String::vformat(fmt, alist);
	va_end(alist);

	for (const char *p = str.c_str(); *p; ++p) {
		if (*p == '\n') {
			_textPos.x = 0;
			_textPos.y++;
		} else {
			writeChar(*p);
		}
	}
}

void GfxSurface::writeChar(uint32 chr) {
	if (chr >= ' ') {
		Font::writeChar(this, chr, Common::Point(_textPos.x * CHAR_WIDTH,
			_textPos.y * CHAR_HEIGHT), _textColor);
		++_textPos.x;
	}

	if (_textPos.x >= TEXT_WIDTH || chr == '\n') {
		_textPos.x = 0;
		_textPos.y++;
	}

	if (_textPos.y >= TEXT_HEIGHT) {
		_textPos.y = TEXT_HEIGHT - 1;

		// Scroll the screen contents up
		blitFrom(*this, Common::Rect(0, CHAR_HEIGHT, DEFAULT_SCX, DEFAULT_SCY),
			Common::Point(0, 0));
		fillRect(Common::Rect(0, DEFAULT_SCX - CHAR_HEIGHT, DEFAULT_SCX, DEFAULT_SCY), 0);
	}
}

} // namespace Gfx
} // namespace Ultima0
} // namespace Ultima
