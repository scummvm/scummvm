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
#include "glk/frotz/frotz.h"
#include "glk/conf.h"
#include "common/file.h"
#include "graphics/fonts/ttf.h"
#include "image/bmp.h"

namespace Glk {
namespace Frotz {

FrotzScreen::FrotzScreen() : Glk::Screen() {
	g_conf->_tStyles[style_User1].font = CUSTOM;
	g_conf->_gStyles[style_User1].font = CUSTOM;
	g_conf->_tStyles[style_User2].font = CUSTOM2;
}

void FrotzScreen::loadFonts(Common::Archive *archive) {
	// Get the zmachine version. At this point the header isn't loaded, so we have to do it manually
	g_vm->_gameFile.seek(0);
	byte version = g_vm->_gameFile.readByte();

	if (version == 6) {
		// For graphical games, ignore any font configurations and force their size
		g_conf->_monoInfo._size = g_conf->_propInfo._size = 7;
		g_conf->_monoInfo._aspect = g_conf->_propInfo._aspect = 1.0;
		g_vm->_defaultForeground = 0;
		g_vm->_defaultBackground = 0xffffff;
	}

	// Load the basic fonts
	Screen::loadFonts(archive);

	// Add character graphics font
	Image::BitmapDecoder decoder;
	Common::File f;
	if (!f.open("infocom_graphics.bmp", *archive))
		error("Could not load font");

	Common::Point fontSize(_fonts[0]->getMaxCharWidth(), _fonts[0]->getFontHeight());
	decoder.loadStream(f);
	_fonts.push_back(new BitmapFont(*decoder.getSurface(), fontSize));
	f.close();

	// Add Runic font. It provides cleaner versions of the runic characters in the
	// character graphics font
	if (!f.open("NotoSansRunic-Regular.ttf", *archive))
		error("Could not load font");

	_fonts.push_back(Graphics::loadTTFFont(f, g_conf->_propInfo._size, Graphics::kTTFSizeModeCharacter));
	f.close();
}

/*--------------------------------------------------------------------------*/

BitmapFont::BitmapFont(const Graphics::Surface &src, const Common::Point &size,
		uint srcWidth, uint srcHeight, unsigned char startingChar) :
		_startingChar(startingChar), _size(size) {
	assert(src.format.bytesPerPixel == 1);
	assert((src.w % srcWidth) == 0);
	assert((src.h % srcHeight) == 0);

	// Set up a characters array
	_chars.resize((src.w / srcWidth) * (src.h / srcHeight));

	// Iterate through loading characters
	Common::Rect r(srcWidth, srcHeight);
	int charsPerRow = src.w / srcWidth;
	for (uint idx = 0; idx < _chars.size(); ++idx) {
		r.moveTo((idx % charsPerRow) * srcWidth, (idx / charsPerRow) * srcHeight);

		_chars[idx].create(size.x, size.y, src.format);
		_chars[idx].transBlitFrom(src, r, Common::Rect(0, 0, size.x, size.y));
	}
}

void BitmapFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	const Graphics::ManagedSurface &c = _chars[chr - _startingChar];
	for (int yCtr = 0; yCtr < c.h; ++yCtr) {
		const byte *srcP = (const byte *)c.getBasePtr(0, yCtr);

		for (int xCtr = 0; xCtr < c.w; ++xCtr, ++srcP) {
			if (!*srcP)
				dst->hLine(x + xCtr, y + yCtr, x + xCtr, color);
		}
	}
}

} // End of namespace Frotz
} // End of namespace Glk
