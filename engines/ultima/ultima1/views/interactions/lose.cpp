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

#include "ultima/ultima1/views/interactions/lose.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

constexpr int TEXT_COLOR = 15;   // white
constexpr int BORDER_COLOR = 1;  // dark blue

// Same generic drawDialog box geometry the Win cutscene uses
constexpr int DIALOG_X1 = 31, DIALOG_Y1 = 23, DIALOG_X2 = 287, DIALOG_Y2 = 127;
constexpr int BORDER_X1 = 34, BORDER_Y1 = 26, BORDER_X2 = 284, BORDER_Y2 = 124;

bool Lose::msgFocus(const FocusMessage &msg) {
	setBounds(TextRect(0, 0, 39, 19));

	g_engine->send("Commands", GameMessage("TEXT", "THOU ART DEAD!"));
	g_engine->playFX(2);
	g_engine->playFX(2);
	g_engine->playFX(2);

	return true;
}

void Lose::draw() {
	drawDialog();
}

void Lose::drawDialog() {
	auto s = getSurface();
	s.fillRect(Rect(DIALOG_X1, DIALOG_Y1, DIALOG_X2, DIALOG_Y2), 0);
	s.frameRect(Rect(BORDER_X1, BORDER_Y1, BORDER_X2, BORDER_Y2), BORDER_COLOR);

	s.setColor(TEXT_COLOR);
	s.writeString(Point(5, 6), "   Thou hast been defeated by");
	s.writeString(Point(5, 7), "      Mondain the Wizard!");
	s.writeString(Point(5, 12), "    THE UNIVERSE IS DOOMED!");
}

bool Lose::msgAction(const ActionMessage &msg) {
	// No resurrection from this one - straight back to the main menu
	g_engine->replaceView("MainMenu", true);
	return true;
}

bool Lose::msgKeypress(const KeypressMessage &msg) {
	g_engine->replaceView("MainMenu", true);
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
