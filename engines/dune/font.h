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

#ifndef DUNE_FONT_H
#define DUNE_FONT_H

#include "common/scummsys.h"
#include "common/str.h"

#include "dune/archive.h"

namespace Dune {

class DuneEngine;

/**
 * The game writes with DNCHAR.BIN, which holds two whole faces:
 *
 *   byte widths[2][128];    // how far the pen moves for each character
 *   byte tall[128][9];      // at 0x100: nine rows of eight pixels each
 *   byte small[128][7];     // at 0x580: seven rows, the same 128 characters
 *
 * The original keeps a routine for each - 0xD096 steps nine bytes to a glyph
 * out of the pointer at DS:0x2514 and 0xD12F seven bytes out of DS:0x2516 -
 * and each reads the width out of its own table, so a face is whole and not
 * just a different set of widths over the same glyphs.  What is spoken is set
 * in the tall face and the panel's own rows in the small one.
 *
 * The codes 6 and 8 in the middle of a line move the pen by the other face's
 * widths, which is what 0x8F15 does to the pointer at DS:0x47A0; the glyphs
 * drawn stay the ones the routine in hand knows.  The pen advances by the
 * width; a glyph is drawn from its high bit down.
 */
class Font {
public:
	enum Face {
		kFaceNormal = 0,
		kFaceAlternate = 1
	};

	enum {
		kSwitchToAlternate = 6,
		kSwitchToNormal = 8
	};

	// How tall a glyph of the face in hand is, which is also how many bytes
	// of the file it takes.
	int glyphHeight() const { return _face == kFaceNormal ? 9 : 7; }

	Font() : _face(kFaceNormal) {}

	bool load(Archive &archive, const Common::String &name);

	void setFace(Face face) { _face = face; }
	Face face() const { return _face; }

	int charWidth(byte c) const;
	// Rendered advance, including immediate inline face changes. Return the
	// ending face as well when measuring consecutive strings.
	int stringWidth(const Common::String &text, Face *endingFace = nullptr) const;
	// 8ED3 latches the width face for one word, while codes 6/8 change
	// the face used to measure the next word. Glyph drawing switches at once.
	int wordWidth(const Common::String &text, Face *endingFace = nullptr) const;

	// Draws onto a 320 pixel wide screen and answers with where the pen ended
	// up.  A character of 6 or 8 changes the face rather than drawing.
	// DNVGA writes a nonzero background over exactly the glyph's width.
	int drawChar(byte *screen, int x, int y, byte c, byte colour, byte background = 0);
	int drawString(byte *screen, int x, int y, const Common::String &text, byte colour, byte background = 0);

private:
	const byte *glyph(byte c) const;

	Common::Array<byte> _data;
	Face _face;
};

} // End of namespace Dune

#endif
