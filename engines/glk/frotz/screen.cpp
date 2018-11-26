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

#include "glk/frotz/screen.h"
#include "glk/conf.h"
#include "common/file.h"
#include "image/bmp.h"

namespace Glk {
namespace Frotz {

FrotzScreen::FrotzScreen() : Glk::Screen() {
	g_conf->_tStyles[style_User1].font = CUSTOM;
}

void FrotzScreen::loadFonts(Common::Archive *archive) {
	Screen::loadFonts(archive);

	Image::BitmapDecoder decoder;
	Common::File f;
	if (!f.open("infocom_graphics.bmp", *archive))
		error("Could not load font");

	decoder.loadStream(f);
	_fonts.push_back(new Frotz::BitmapFont(*decoder.getSurface()));
}

/*--------------------------------------------------------------------------*/

BitmapFont::BitmapFont(const Graphics::Surface &src, uint charWidth,
		uint charHeight, unsigned char startingChar) : _startingChar(startingChar) {
	assert(src.format.bytesPerPixel == 1);
	assert((src.w % charWidth) == 0);
	assert((src.h % charHeight) == 0);
	_surface.copyFrom(src);

	Common::Rect r(charWidth, charHeight);
	for (uint y = 0; y < src.h; y += charHeight) {
		r.moveTo(0, y);
		for (uint x = 0; x < src.w; x += charWidth, r.translate(charWidth, 0))
			_chars.push_back(r);
	}
}

BitmapFont::~BitmapFont() {
	_surface.free();
}

void BitmapFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	const Common::Rect &r = _chars[chr - _startingChar];
	for (int yCtr = 0; yCtr < r.height(); ++yCtr) {
		const byte *srcP = (const byte *)_surface.getBasePtr(r.left, r.top + yCtr);

		for (int xCtr = 0; xCtr < r.width(); ++xCtr, ++srcP) {
			if (!*srcP)
				dst->hLine(x + xCtr, y + yCtr, x + xCtr, color);
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
