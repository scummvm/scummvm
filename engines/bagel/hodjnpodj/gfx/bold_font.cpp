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

#include "bagel/hodjnpodj/gfx/bold_font.h"

namespace Bagel {
namespace HodjNPodj {
namespace Gfx {

BoldFont::~BoldFont() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _font;
}

int BoldFont::getFontHeight() const {
	return _font->getFontHeight();
}

int BoldFont::getFontAscent() const {
	return _font->getFontAscent();
}

int BoldFont::getFontDescent() const {
	return _font->getFontDescent();
}

int BoldFont::getFontLeading() const {
	return _font->getFontLeading();
}

int BoldFont::getMaxCharWidth() const {
	return _font->getMaxCharWidth() + 1;
}

int BoldFont::getCharWidth(uint32 chr) const {
	return _font->getCharWidth(chr) + 1;
}

int BoldFont::getKerningOffset(uint32 left, uint32 right) const {
	return _font->getKerningOffset(left, right);
}

void BoldFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	_font->drawChar(dst, chr, x, y, color);
	_font->drawChar(dst, chr, x + 1, y, color);
}

void BoldFont::drawChar(Graphics::ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const {
	_font->drawChar(dst, chr, x, y, color);
	_font->drawChar(dst, chr, x + 1, y, color);
}

} // namespace Gfx
} // namespace HodjNPodj
} // namespace Bagel
