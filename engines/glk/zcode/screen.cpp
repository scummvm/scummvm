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

#include "glk/zcode/screen.h"
#include "glk/zcode/bitmap_font.h"
#include "glk/zcode/zcode.h"
#include "glk/conf.h"
#include "common/file.h"
#include "graphics/fonts/ttf.h"
#include "image/bmp.h"

namespace Glk {
namespace ZCode {

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
		loadVersion6Fonts(archive);
	} else {
		// Load the basic fonts
		Screen::loadFonts(archive);
	}

	// Add character graphics and runic fonts
	loadExtraFonts(archive);
}

void FrotzScreen::loadVersion6Fonts(Common::Archive *archive) {
	// Set the basic font properties
	MonoFontInfo &mi = g_conf->_monoInfo;
	PropFontInfo &pi = g_conf->_propInfo;
	mi._size = pi._size = 7;
	mi._aspect = pi._aspect = 1.0;
	pi._quotes = 0;
	pi._dashes = 0;
	pi._spaces = 0;
	pi._morePrompt = "[MORE]";
	pi._lineSeparation = 0;

	g_vm->_defaultForeground = 0;
	g_vm->_defaultBackground = (int)zcolor_Transparent;
	g_conf->_tMarginX = 3;
	g_conf->_tMarginY = 3;

	for (uint idx = 0; idx < style_NUMSTYLES; ++idx) {
		g_conf->_tStyles[idx].bg = g_conf->_tStylesDefault[idx].bg = zcolor_Transparent;
		g_conf->_gStyles[idx].bg = g_conf->_gStylesDefault[idx].bg = zcolor_Transparent;
	}

	_fonts.resize(8);

	// Load up the 8x8 Infocom font
	Image::BitmapDecoder decoder;
	Common::File f;
	if (!f.open("infocom6x8.bmp", *archive))
		error("Could not load font");

	Common::Point fontSize(6, 8);
	decoder.loadStream(f);
	f.close();

	// Add normal fonts
	_fonts[MONOR] = new FixedWidthBitmapFont(*decoder.getSurface(), fontSize, 6, 8);
	_fonts[MONOB] = new FixedWidthBitmapFont(*decoder.getSurface(), fontSize, 6, 8);
	_fonts[PROPR] = new VariableWidthBitmapFont(*decoder.getSurface(), fontSize, 6, 8);
	_fonts[PROPB] = new VariableWidthBitmapFont(*decoder.getSurface(), fontSize, 6, 8);

	// Create a new version of the font with every character unlined for the emphasized fonts
	const Graphics::Surface &norm = *decoder.getSurface();
	Graphics::ManagedSurface emph(norm.w, norm.h);
	emph.blitFrom(norm);

	for (int y = 8 - 2; y < emph.h; y += 8) {
		byte *lineP = (byte *)emph.getBasePtr(0, y);
		Common::fill(lineP, lineP + emph.w, 0);
	}

	// Add them to the font list
	_fonts[MONOI] = new FixedWidthBitmapFont(emph, fontSize, 6, 8);
	_fonts[MONOZ] = new FixedWidthBitmapFont(emph, fontSize, 6, 8);
	_fonts[PROPI] = new VariableWidthBitmapFont(emph, fontSize, 6, 8);
	_fonts[PROPZ] = new VariableWidthBitmapFont(emph, fontSize, 6, 8);
}

void FrotzScreen::loadExtraFonts(Common::Archive *archive) {
	Image::BitmapDecoder decoder;
	Common::File f;
	if (!f.open("infocom_graphics.bmp", *archive))
		error("Could not load font");

	Common::Point fontSize(_fonts[0]->getMaxCharWidth(), _fonts[0]->getFontHeight());
	decoder.loadStream(f);
	_fonts.push_back(new FixedWidthBitmapFont(*decoder.getSurface(), fontSize));
	f.close();

	// Add Runic font. It provides cleaner versions of the runic characters in the
	// character graphics font
	if (!f.open("NotoSansRunic-Regular.ttf", *archive))
		error("Could not load font");

	_fonts.push_back(Graphics::loadTTFFont(f, g_conf->_propInfo._size, Graphics::kTTFSizeModeCharacter));
	f.close();
}

} // End of namespace ZCode
} // End of namespace Glk
