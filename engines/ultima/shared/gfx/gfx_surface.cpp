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

#include "ultima/shared/gfx/gfx_surface.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

void GfxSurface::writeString(const Common::Point &pt, const Common::String &str,
		Graphics::TextAlign align) {
	_textPos = pt;
	writeString(str, align);
}

void GfxSurface::writeString(const Common::String &str, Graphics::TextAlign align) {
	size_t strSize = 0;
	for (const char *p = str.c_str(); *p; ++p)
		strSize += Common::isPrint(*p) ? 1 : 0;

	switch (align) {
	case Graphics::kTextAlignCenter:
		_textPos.x -= strSize / 2;
		break;
	case Graphics::kTextAlignRight:
		_textPos.x -= strSize;
		break;
	case Graphics::kTextAlignLeft:
	default:
		break;
	}

	for (const char *p = str.c_str(); *p; ++p) {
		if (*p == '\n') {
			assert(align == Graphics::kTextAlignLeft);
			newLine();
		} else if (*p < 32) {
			setColor((byte)*p);
		} else {
			writeChar(*p);
		}
	}
}

void GfxSurface::writeChar(uint32 chr) {
	const int charW = _font.getMaxCharWidth();
	const int charH = _font.getFontHeight();
	const int textW = this->w / charW;

	if (chr >= ' ') {
		_font.drawChar(this, chr, _textPos.x * charW, _textPos.y * charH, _textColor);
		_textPos.x++;
	}

	if (_textPos.x >= textW || chr == '\n') {
		newLine();
	}
}

void GfxSurface::newLine() {
	_textPos.x = 0;
	_textPos.y++;

	const int fontH = _font.getFontHeight();
	const int textH = fontH / _font.getFontHeight();
	if (_textPos.y >= textH) {
		_textPos.y = textH - 1;

		// Scroll the screen contents up
		blitFrom(*this, Common::Rect(0, fontH, this->w, this->h), Common::Point(0, 0));
		fillRect(Common::Rect(0, this->h - fontH, this->w, this->h), 0);
	}
}

void GfxSurface::setTextPos(const Common::Point &pt) {
	_textPos = pt;
}

byte GfxSurface::setColor(byte color) {
	byte oldColor = _textColor;
	_textColor = color;
	return oldColor;
}

} // namespace Gfx
} // namespace Shared
} // namespace Ultima
