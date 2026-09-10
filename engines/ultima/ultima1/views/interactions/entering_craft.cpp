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

#include "common/util.h"
#include "ultima/ultima1/views/interactions/entering_craft.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

constexpr int TEXT_COLOR = 11;   // light cyan - matches Commands/Stats
constexpr int BORDER_COLOR = 1;  // dark blue

// The dialog's own area (matching fillDialogArea's fill rect) and, inset
// within it, the thin border line (matching drawDialogBorder) - both given
// here in absolute screen pixels, as in the original
constexpr int DIALOG_X1 = 22, DIALOG_Y1 = 15, DIALOG_X2 = 300, DIALOG_Y2 = 143;
constexpr int BORDER_X1 = 25, BORDER_Y1 = 18, BORDER_X2 = 297, BORDER_Y2 = 140;

struct CraftLine {
	int _x, _y;
	const char *_text;
};

// Text cell positions here are the original's setCursorPos/writeStringAt
// coordinates, adjusted by (-3, -2) to line up with this dialog's own
// (smaller, inset) text area
static const CraftLine PAGE1_LINES[] = {
	{ 2, 1, "Entering the craft, thou dost" },
	{ 2, 2, "remark upon four holes marked:" },
	{ 9, 4, "R    G    B    W" },
	{ 1, 7, "The proper gems fit in each hole." },
	{ 2, 9, "Further examination leads thee" },
	{ 8, 10, "to two discoveries:" },
	{ 1, 12, "There is a button marked LAUNCH," },
	{ 3, 13, "and thou art locked in, with" },
	{ 3, 14, "nothing to do but press it!" }
};

static const CraftLine PAGE2_LINES[] = {
	{ 1, 2, "As soon as thou hast pressed the" },
	{ 1, 3, "large black button, lights begin" },
	{ 1, 4, "  to dance across the control" },
	{ 13, 5, "screens." },
	{ 1, 7, " Generators shake the very floor" },
	{ 9, 8, "beneath thy feet." },
	{ 1, 10, "The small craft begins to tremble" },
	{ 5, 11, "and rattle as thy journey" },
	{ 6, 12, "through time begins..." }
};

static const CraftLine PAGE3_LINES[] = {
	{ 1, 6, "After but a few moments thou dost" },
	{ 1, 7, "feel a strong magic pulling thee" },
	{ 9, 8, "from thy craft..." }
};

static const CraftLine PAGE4_LINES[] = {
	{ 8, 4, "...A moment later," },
	{ 6, 5, "thou art face to face" },
	{ 1, 6, " with the evil Mondain himself!" },
	{ 6, 11, "Good luck, this is it!" }
};

// X positions of the four gem holes, under the R/G/B/W heading on page 1
// (also adjusted by -3); the row is the heading's row + 1, i.e. 5
static const int GEM_HOLE_X[4] = { 9, 14, 19, 24 };
constexpr int GEM_HOLE_Y = 5;

// Red/Green/Blue/White, matching the gem each hole is labelled for
static const byte GEM_COLORS[4] = { 4, 2, 9, 15 };

// Per-row half-width of an 8x8 diamond, used to draw each gem as a small
// solid diamond rather than relying on font glyphs (which the DosFont
// doesn't have meaningful shapes for at these code points - that's set up
// by the Mondain executable's own startup code, which isn't ported)
static const byte GEM_DIAMOND_HALF_WIDTH[8] = { 0, 1, 2, 3, 3, 2, 1, 0 };

bool EnteringCraft::msgFocus(const FocusMessage &msg) {
	setBounds(Common::Rect(DIALOG_X1, DIALOG_Y1, DIALOG_X2, DIALOG_Y2));

	_page = 1;
	showPage();
	return true;
}

void EnteringCraft::showPage() {
	redraw();
	g_engine->send("Commands", GameMessage("SPACE"));
}

void EnteringCraft::draw() {
	auto s = getSurface();
	s.fillRect(Common::Rect(0, 0, DIALOG_X2 - DIALOG_X1, DIALOG_Y2 - DIALOG_Y1), 0);
	s.frameRect(Common::Rect(BORDER_X1 - DIALOG_X1, BORDER_Y1 - DIALOG_Y1,
		BORDER_X2 - DIALOG_X1, BORDER_Y2 - DIALOG_Y1), BORDER_COLOR);

	drawPageText();
	if (_page == 1)
		drawGemHoles();
}

void EnteringCraft::drawPageText() {
	const CraftLine *lines = nullptr;
	int count = 0;

	switch (_page) {
	case 1:
		lines = PAGE1_LINES;
		count = ARRAYSIZE(PAGE1_LINES);
		break;
	case 2:
		lines = PAGE2_LINES;
		count = ARRAYSIZE(PAGE2_LINES);
		break;
	case 3:
		lines = PAGE3_LINES;
		count = ARRAYSIZE(PAGE3_LINES);
		break;
	case 4:
		lines = PAGE4_LINES;
		count = ARRAYSIZE(PAGE4_LINES);
		break;
	default:
		break;
	}

	auto s = getSurface();
	s.setColor(TEXT_COLOR);
	for (int i = 0; i < count; ++i)
		s.writeString(Common::Point(lines[i]._x, lines[i]._y), lines[i]._text);
}

void EnteringCraft::drawGemHoles() {
	for (int i = 0; i < 4; ++i)
		drawGem(GEM_HOLE_X[i], GEM_HOLE_Y, GEM_COLORS[i]);
}

void EnteringCraft::drawGem(int cellX, int cellY, byte color) {
	auto s = getSurface();
	int px = cellX * 8, py = cellY * 8;

	for (int row = 0; row < 8; ++row) {
		int halfWidth = GEM_DIAMOND_HALF_WIDTH[row];
		s.drawLine(px + 3 - halfWidth, py + row, px + 3 + halfWidth, py + row, color);
	}
}

bool EnteringCraft::msgAction(const ActionMessage &msg) {
	nextPage();
	return true;
}

bool EnteringCraft::msgKeypress(const KeypressMessage &msg) {
	nextPage();
	return true;
}

void EnteringCraft::nextPage() {
	if (_page < 4) {
		++_page;
		showPage();
	} else {
		close();
		_G(map).load(Data::MAP_MONDAIN);
		_G(logic)->entering();
	}
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
