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

#ifndef CRYOMNI3D_ATLANTIS_MENU_LAYOUT_H
#define CRYOMNI3D_ATLANTIS_MENU_LAYOUT_H

#include "common/array.h"
#include "common/str.h"
#include "common/stream.h"

namespace CryOmni3D {
namespace Atlantis {

// Parsed entry from a SPRLIST\*MENU.TXT layout file (original Atlantis menu
// data).  The format is line-oriented:
//
//   <X,Y>TEXT          — anchor a FONTMAX label at (X, Y_txt).  X may be
//                        empty, e.g. "<>,140>SELECT PLAYER NAME", meaning
//                        the label is horizontally centred on the 640-wide
//                        screen.  Y is the y_txt baseline (same convention
//                        as drawFontMaxText).
//   /spr=I,HX,HY,W,H   — composite SPRMENU.SPR sprite index I with its
//                        hotspot at screen (HX, HY).  W/H are size hints kept
//                        in w/h (slider tracks use them); the sprite still
//                        carries its real pixel size for blitting.
//   <X,Y>//A&&B[&&C…]   — a toggle: a text anchor whose value cycles through
//                        the &&-separated option labels.  Parsed as kToggle.
//   /fin or /FIN       — end of file.  Anything after is ignored.
//
// Blank lines and CR characters are tolerated.
struct MenuItem {
	enum Kind { kText, kSprite, kToggle };

	Kind kind;

	// Text item fields (anchorX/anchorY/centerX also apply to kToggle).
	Common::String text;
	bool centerX;       // true → ignore anchorX and centre on screen.
	int  anchorX;
	int  anchorY;       // y_txt baseline.

	// Sprite item fields.  w/h come from the /spr size hint.
	int sprIdx;
	int hx, hy;
	int w, h;

	// Toggle item fields (kToggle).  `options` holds the label of each
	// state; `selected` indexes the currently active one.
	Common::Array<Common::String> options;
	int selected;

	MenuItem()
	    : kind(kText), centerX(false), anchorX(0), anchorY(0),
	      sprIdx(0), hx(0), hy(0), w(0), h(0), selected(0) {}
};

struct MenuLayout {
	Common::Array<MenuItem> items;

	// Parse the menu-text stream.  Returns false only when the buffer is
	// unreadable; an empty / truncated file yields an empty layout and true.
	bool loadFromStream(Common::ReadStream &s);
};

} // namespace Atlantis
} // namespace CryOmni3D

#endif // CRYOMNI3D_ATLANTIS_MENU_LAYOUT_H
