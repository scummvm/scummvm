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

#ifndef NANCY_SCRIPT_V5_BITMAPTEXT_H
#define NANCY_SCRIPT_V5_BITMAPTEXT_H

#include "common/array.h"
#include "common/str.h"

#include "graphics/managed_surface.h"

#include "engines/nancy/enginedata.h"
#include "engines/nancy/script/types.h"

namespace Nancy {
namespace Script {

// Nancy26 writes its texts with the fonts made of pictures that Fonts_SC lists.
// A text can switch fonts and colors along the way, and say how it lines up:
//
//   <fN>        the font of number N          <cN>        the font color N
//   <b> </b>    bold                          <i> </i>    italic
//   <hl> <hc> <hr>   lines go left, centered or right
//   <vt> <vc> <vb>   the text goes to the top, the middle or the bottom
//   <n>         a new line
//
// A text starts in font 4 and color 0. Lines wrap between words to fit the
// width they are given.
class BitmapText {
public:
	// Lays the text out in an area of the given size. A font color of -1 lets
	// the text pick its colors; anything else is used for all of it.
	void layout(const Common::String &text, int width, int height, int overrideColor = -1);

	// Draws the letters from first on, as many as count (-1 for all of them),
	// at a place of the surface, with their colors multiplied by modColor
	void draw(Graphics::ManagedSurface &surface, const Common::Point &origin, uint32 modColor, int first = 0, int count = -1) const;

	// The area the letters take up, in the coordinates of the layout
	const ScriptRect &getBounds() const { return _bounds; }

	// What the letters cover when they get drawn, which can stick out of the
	// area of the layout; empty without letters
	Common::Rect getInkArea() const;

	// The number of letters, which drawing counts in
	uint getNumLetters() const { return _letters.size(); }

private:
	struct Letter {
		const ScriptBootData::BitmapFont::Glyph *glyph = nullptr;
		const Graphics::ManagedSurface *image = nullptr;
		Common::Point position;	// of the top left of the glyph
		uint32 color = 0xFFFFFFFF;
	};

	Common::Array<Letter> _letters;
	ScriptRect _bounds;
};

// The width a text takes in a font, for the scripts that measure texts
int getBitmapTextWidth(const Common::String &text, int fontID);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_BITMAPTEXT_H
