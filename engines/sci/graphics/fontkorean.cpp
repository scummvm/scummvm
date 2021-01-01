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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/scifont.h"
#include "sci/graphics/fontkorean.h"

namespace Sci {

GfxFontKorean::GfxFontKorean(GfxScreen *screen, GuiResourceId resourceId)
	: _resourceId(resourceId), _screen(screen) {
	assert(resourceId != -1);

	_commonFont = Graphics::FontKorean::createFont("korean.fnt");
	//warning("GfxFontKorean: created Font");

	if (!_commonFont)
		error("Could not load ScummVM's 'korean.fnt'");
}

GfxFontKorean::~GfxFontKorean() {
}

GuiResourceId GfxFontKorean::getResourceId() {
	return _resourceId;
}

// Returns true for first byte of double byte characters
bool GfxFontKorean::isDoubleByte(uint16 chr) {
	uint16 ch = chr & 0xFF;
	if ((ch >= 0xA1) && (ch <= 0xFE))
		return true;
	return false;
}

// We can do >>1, because returned char width/height is 8 or 16 exclusively. Font returns hires size, we need lowres
byte GfxFontKorean::getHeight() {
	if (getSciVersion() >= SCI_VERSION_2)
		return _commonFont->getFontHeight();
	else
		return _commonFont->getFontHeight() >> 1;
}

byte GfxFontKorean::getCharWidth(uint16 chr) {
	if (getSciVersion() >= SCI_VERSION_2)
		return _commonFont->getCharWidth(chr);
	else
		return _commonFont->getCharWidth(chr) >> 1;
}

void GfxFontKorean::draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput) {
	// TODO: Check, if character fits on screen - if it doesn't we need to skip it
	_screen->putHangulChar(_commonFont, left & 0xFFC, top, chr, color);
}

#ifdef ENABLE_SCI32
void GfxFontKorean::drawToBuffer(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput, byte *buffer, int16 bufWidth, int16 bufHeight) {
	byte *displayPtr = buffer + top * bufWidth + left;
	// we don't use outline, so color 0 is actually not used
	_commonFont->drawChar(displayPtr, chr, bufWidth, 1, color, 0, bufWidth - left, bufHeight - top);
}

#endif

} // End of namespace Sci
