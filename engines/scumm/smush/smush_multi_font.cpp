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

#include "scumm/smush/smush_multi_font.h"
#include "scumm/smush/smush_font.h"
#include "scumm/smush/smush_player.h"
#include "scumm/scumm.h"

namespace Scumm {

SmushMultiFont::SmushMultiFont(ScummEngine *vm, SmushPlayer *player, bool useOriginalColors)
	: _vm(vm), _player(player), _currentFont(0), _defaultFont(0), _hardcodedFontColors(useOriginalColors) {
	_textRenderer = new TextRenderer_v7(vm, this);
}

SmushMultiFont::~SmushMultiFont() {
	delete _textRenderer;
}

NutRenderer *SmushMultiFont::getFont(int id) {
	// Delegate to SmushPlayer to get the font
	// SmushPlayer::getFont() handles font loading and caching
	return _player->getFont(id);
}

NutRenderer *SmushMultiFont::getCurrentFont() const {
	// We need a const version that doesn't trigger loading
	// For const access, use _player's cached fonts directly
	return const_cast<SmushMultiFont*>(this)->getFont(_currentFont);
}

void SmushMultiFont::drawString(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int16 col, TextStyleFlags flags) {
	// Reset to default font before drawing
	_currentFont = _defaultFont;
	_textRenderer->drawString(str, buffer, clipRect, x, y, _vm->_screenWidth, col, flags);
}

void SmushMultiFont::drawStringWrap(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int16 col, TextStyleFlags flags) {
	// Reset to default font before drawing
	_currentFont = _defaultFont;
	_textRenderer->drawStringWrap(str, buffer, clipRect, x, y, _vm->_screenWidth, col, flags);
}

int SmushMultiFont::draw2byte(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, uint16 chr) {
	NutRenderer *font = getCurrentFont();
	if (!font)
		return 0;

	// Adjust color for CMI compatibility
	int16 adjCol = col;
	if (_vm->_game.id == GID_CMI)
		adjCol = 255;
	else if (_vm->_game.id == GID_DIG && col == -1)
		adjCol = 1;

	return font->draw2byte(buffer, clipRect, x, y, pitch, adjCol, chr);
}

int SmushMultiFont::drawCharV7(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags, byte chr) {
	NutRenderer *font = getCurrentFont();
	if (!font)
		return 0;

	return font->drawCharV7(buffer, clipRect, x, y, pitch, col, flags, chr, _hardcodedFontColors, true);
}

int SmushMultiFont::getCharWidth(uint16 chr) const {
	NutRenderer *font = getCurrentFont();
	if (!font)
		return 0;

	return font->getCharWidth(chr & 0xFF);
}

int SmushMultiFont::getCharHeight(uint16 chr) const {
	NutRenderer *font = getCurrentFont();
	if (!font)
		return 0;

	return font->getCharHeight(chr & 0xFF);
}

int SmushMultiFont::getFontHeight() const {
	NutRenderer *font = getCurrentFont();
	if (!font)
		return 0;

	return font->getFontHeight();
}

int SmushMultiFont::setFont(int id) {
	// This is called by TextRenderer_v7 when it encounters ^fXX escape codes
	// Actually switch the current font
	int oldFont = _currentFont;

	if (id >= 0 && id < MAX_FONTS) {
		_currentFont = id;
		debugC(DEBUG_SMUSH, "SmushMultiFont::setFont: switching from font %d to font %d", oldFont, id);
	} else {
		debugC(DEBUG_SMUSH, "SmushMultiFont::setFont: invalid font id %d, keeping font %d", id, _currentFont);
	}

	return oldFont;
}

} // End of namespace Scumm
