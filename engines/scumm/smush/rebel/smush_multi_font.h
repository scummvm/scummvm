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

#ifndef SCUMM_SMUSH_MULTI_FONT_H
#define SCUMM_SMUSH_MULTI_FONT_H

#include "common/scummsys.h"
#include "scumm/nut_renderer.h"
#include "scumm/scumm.h"
#include "scumm/smush/rebel/font_rebel2.h"
#include "scumm/string_v7.h"

namespace Scumm {

class SmushPlayer;

/**
 * SmushMultiFont - Multi-font renderer for SMUSH videos
 *
 * This class implements the GlyphRenderer_v7 interface and supports
 * switching between multiple fonts during text rendering via the ^fXX
 * escape sequence.
 *
 * The original Rebel Assault 2 (and other SCUMM v7 games) used a linked
 * list of font structures that could be traversed when ^f escape codes
 * were encountered. This class provides equivalent functionality by
 * holding an array of NutRenderer pointers and switching between them.
 */
class SmushMultiFont : public GlyphRenderer_v7 {
public:
	static const int MAX_FONTS = 5;

	SmushMultiFont(ScummEngine *vm, SmushPlayer *player, bool useOriginalColors);
	~SmushMultiFont() override;

	// String drawing methods
	void drawString(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int16 col, TextStyleFlags flags);
	void drawString(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags);
	void drawStringWrap(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int16 col, TextStyleFlags flags);
	void drawStringWrap(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags);

	// GlyphRenderer_v7 interface
	int draw2byte(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, uint16 chr) override;
	int drawCharV7(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags, byte chr) override;
	int getCharWidth(uint16 chr) const override;
	int getCharHeight(uint16 chr) const override;
	int getFontHeight() const override;
	int setFont(int id) override;
	bool newStyleWrapping() const override { return true; }

	// Set the initial/default font
	void setDefaultFont(int id) { _defaultFont = id; _currentFont = id; }

private:
	NutRenderer *getFont(int id);
	NutRenderer *getCurrentFont() const;
	Rebel2FontSet getRebel2FontSet();

	ScummEngine *_vm;
	SmushPlayer *_player;
	TextRenderer_v7 *_textRenderer;
	NutRenderer *_rebel2Fonts[Rebel2FontSet::kMaxFonts];

	int _currentFont;
	int _defaultFont;
	bool _hardcodedFontColors;
};

} // End of namespace Scumm

#endif
