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
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/font.h"
#include "sci/graphics/fontsjis.h"

namespace Sci {

GfxFontSjis::GfxFontSjis(GfxScreen *screen, GuiResourceId resourceId)
	: _resourceId(resourceId), _screen(screen) {
	assert(resourceId != -1);

	if (!_screen->getUpscaledHires())
		error("I don't want to initialize, when not being in upscaled hires mode");

	_commonFont = Graphics::FontSJIS::createFont(Common::kPlatformPC98);

	if (!_commonFont)
		error("Could not load ScummVM's 'SJIS.FNT'");
}

GfxFontSjis::~GfxFontSjis() {
}

GuiResourceId GfxFontSjis::getResourceId() {
	return _resourceId;
}

// Returns true for first byte of double byte characters
bool GfxFontSjis::isDoubleByte(uint16 chr) {
	if ((chr == 0x5C) || (chr == 0x7E) || ((chr >= 0x81) && (chr <= 0x9F)) || ((chr >= 0xE0) && (chr <= 0xEF)))
		return true;
	return false;
}

// We can do >>1, because returned char width/height is 8 or 16 exclusively. Font returns hires size, we need lowres
byte GfxFontSjis::getHeight() {
	return _commonFont->getFontHeight() >> 1;
}

byte GfxFontSjis::getCharWidth(uint16 chr) {
	return _commonFont->getCharWidth(chr) >> 1;

}
byte GfxFontSjis::getCharHeight(uint16 chr) {
	return _commonFont->getFontHeight() >> 1;
}

void GfxFontSjis::draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput) {
	// TODO: Check, if character fits on screen - if it doesn't we need to skip it
	//  Normally SCI cuts the character and draws the part that fits, but the common SJIS doesn't support that
	_screen->putKanjiChar(_commonFont, left, top, chr, color);
}

} // End of namespace Sci
