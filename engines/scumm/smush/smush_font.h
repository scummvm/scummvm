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

#ifndef SCUMM_SMUSH_FONT_H
#define SCUMM_SMUSH_FONT_H

#include "common/scummsys.h"
#include "scumm/nut_renderer.h"
#include "scumm/scumm.h"
#include "scumm/string_v7.h"

namespace Scumm {

class SmushFont : public NutRenderer, public GlyphRenderer_v7 {
public:
	SmushFont(ScummEngine *vm, const char *filename, bool useOriginalColors) :
		NutRenderer(vm, filename), _hardcodedFontColors(useOriginalColors) {
		_r = new TextRenderer_v7(vm, this);
	}

	~SmushFont() override {	delete _r;}

	void drawString(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int16 col, TextStyleFlags flags) {
		_r->drawString(str, buffer, clipRect, x, y, _vm->_screenWidth, col, flags);
	}

	void drawStringWrap(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int16 col, TextStyleFlags flags) {
		_r->drawStringWrap(str, buffer, clipRect, x, y, _vm->_screenWidth, col, flags);
	}

private:
	int draw2byte(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, uint16 chr) override {
		return NutRenderer::draw2byte(buffer, clipRect, x, y, pitch, _vm->_game.id == GID_CMI ? 255 : (_vm->_game.id == GID_DIG && col == -1 ? 1 : col), chr);
	}

	int drawChar(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags, byte chr) override {
		return NutRenderer::drawChar(buffer, clipRect, x, y, pitch, col, flags, chr, _hardcodedFontColors, true);
	}

	int getCharWidth(uint16 chr) const override { return NutRenderer::getCharWidth(chr & 0xFF); }
	int getCharHeight(uint16 chr) const override { return NutRenderer::getCharHeight(chr & 0xFF); }
	int getFontHeight() const override { return NutRenderer::getFontHeight(); }
	int setFont(int) override { return 0; }
	bool newStyleWrapping() const override { return true; }

	TextRenderer_v7 *_r;
	const bool _hardcodedFontColors;
};

} // End of namespace Scumm

#endif
