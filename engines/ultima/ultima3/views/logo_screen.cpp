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

#include "common/file.h"
#include "ultima/ultima3/views/logo_screen.h"
#include "ultima/ultima3/views/menu_input.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

// Each round advances the tile animations six ticks. Roughly the time the
// original took to redraw the whole scene that many times on its first PCs
constexpr int ANIMATIONS_PER_ROUND = 6;
constexpr int FRAMES_PER_ROUND = 3;

// Edits that set a tile below this value take effect without a round of
// animation before the next one
constexpr byte MIN_VISIBLE_TILE = 8;

void LogoScreen::fetchMove() {
	for (;;) {
		_movePos = (_movePos + 1) % MOVES_COUNT;
		byte tile = _moves[_movePos];
		byte value = _moves[MOVES_COUNT + _movePos];

		if (tile == 0xFF) {
			_roundsLeft = value ? value : 256;
			return;
		}

		_demo[tile] = value >> 1;
		if (_demo[tile] >= MIN_VISIBLE_TILE) {
			_roundsLeft = 1;
			return;
		}
	}
}

bool LogoScreen::msgFocus(const FocusMessage &msg) {
	// Coming back from the main menu carries on from where the scene was
	if (!msg._priorView || msg._priorView->getName() != "MainMenu") {
		Common::File f;
		if (!f.open("DEMO.ULT") || f.read(_demo, sizeof(_demo)) != sizeof(_demo))
			error("Could not load DEMO.ULT");
		f.close();
		if (!f.open("MOVES.ULT") || f.read(_moves, sizeof(_moves)) != sizeof(_moves))
			error("Could not load MOVES.ULT");

		_G(shapes).load();
		_movePos = MOVES_COUNT - 1;
		_roundsLeft = 0;
	}

	delayFrames(FRAMES_PER_ROUND);
	return View::msgFocus(msg);
}

void LogoScreen::draw() {
	auto s = getSurface();

	drawBorder(s);
	s.writeString(Common::Point(14, 10), "\x10Ultima III\x11");

	for (int row = 0; row < DEMO_ROWS; ++row) {
		for (int col = 0; col < DEMO_COLS; ++col)
			_G(shapes).drawTile(s, 8 + col * 16, 88 + row * 16, _demo[row * DEMO_COLS + col]);
	}
	s.addDirtyRect(Common::Rect(8, 88, 312, 184));

	drawSpacePrompt(s);
}

void LogoScreen::timeout() {
	if (_roundsLeft == 0)
		fetchMove();

	for (int i = 0; i < ANIMATIONS_PER_ROUND; ++i)
		_G(shapes).animate();
	--_roundsLeft;

	redraw();
	delayFrames(FRAMES_PER_ROUND);
}

bool LogoScreen::msgKeypress(const KeypressMessage &msg) {
	if (!isModifierKey(msg.keycode))
		addView("MainMenu");

	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
