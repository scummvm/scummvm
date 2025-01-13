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

#include "got/views/dialogs/high_scores.h"
#include "got/vars.h"

namespace Got {
namespace Views {
namespace Dialogs {

#define TIMEOUT 500

HighScores::HighScores() : Dialog("HighScores") {
	setBounds(Common::Rect(0, 0 + 24, 320, 192 + 24));
}

void HighScores::draw() {
	// Draw background outside dialog
	GfxSurface s = GfxSurface(*g_events->getScreen(), Common::Rect(0, 0, 320, 240));

	for (int col = 0, xp = 0; col < 10; ++col, xp += 32) {
		for (int yp = 0; yp < 240; yp += 32)
			s.simpleBlitFrom(_G(gfx[26]), Common::Point(xp, yp));
	}

	// Draw the dialog frame
	Dialog::draw();

	// Clear the inner content first
	s = getSurface(true);
	s.clear(215);

	// Draw title
	Common::String title = "High Scores - Part I";
	for (int area = 2; area <= _currentArea; ++area)
		title += 'I';

	int titleStart = (s.w - title.size() * 8) / 2;
	s.print(Common::Point(titleStart, 4), title, 54);

	for (int i = 0; i < 7; ++i) {
		const HighScore &hs = _G(highScores)._scores[_currentArea - 1][i];

		// Draw frames for name and score
		s.frameRect(Common::Rect(10, 20 + i * 18, 210, 20 + i * 18 + 16),
					206);
		s.frameRect(Common::Rect(220, 20 + i * 18, 280, 20 + i * 18 + 16),
					206);

		// Write out the name and scores
		s.print(Common::Point(15, 24 + i * 18), hs._name, 14);
		Common::String score = Common::String::format("%d", hs._total);
		s.print(Common::Point(275 - (score.size() * 8), 24 + i * 18), score, 14);
	}
}

bool HighScores::msgFocus(const FocusMessage &msg) {
	_currentArea = 1;
	_showAll = true;
	_timeoutCtr = TIMEOUT;

	Gfx::load_palette();
	return true;
}

bool HighScores::msgGame(const GameMessage &msg) {
	if (msg._name == "HIGH_SCORES") {
		// Finished one of the parts of the game, so just
		// set the view to show only scores for that area
		replaceView("HighScores", true);
		_currentArea = msg._value;
		_showAll = false;

		draw();
		fadeIn();

		return true;
	}

	return false;
}

bool HighScores::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		goToMainMenu();
	} else {
		goToNextArea();
	}
	return true;
}

void HighScores::goToMainMenu() {
	fadeOut();
	send("TitleBackground", GameMessage("MAIN_MENU"));
}

void HighScores::goToNextArea() {
	if (_showAll && _currentArea < 3) {
		fadeOut();
		++_currentArea;
		draw();
		fadeIn();
		_timeoutCtr = TIMEOUT;

	} else {
		// Done all the areas, so
		goToMainMenu();
	}
}

bool HighScores::tick() {
	if (--_timeoutCtr < 0)
		goToNextArea();

	return true;
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
