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

#include "ultima/ultima2/views/commands.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

constexpr int COLOR_TEXT = 3; // white, within the game's 4-color CGA palette
constexpr int NEWLINE_DELAY = 50;
constexpr int PROMPT_ROW = 3; // last of the 4 rows within Commands' own bounds
constexpr int PROMPT_ABSOLUTE_ROW = 23;

Commands::Commands(UIElement *parent) : Shared::UIElement("Commands", parent), _textCursor("CommandsCursor", this, COLOR_TEXT) {
}

void Commands::draw() {
	_surface = getSurface();
	_surface.clear();
	_surface.setTextPos(Common::Point(0, PROMPT_ROW));
	_surface.setScrollable(true);
	_surface.setColor(COLOR_TEXT);

	prompt();
}

bool Commands::msgGame(const GameMessage &msg) {
	if (msg._name == "TEXT") {
		writeString(msg._stringValue);
		return true;
	} else if (msg._name == "RESET_LINE") {
		resetLine();
		return true;
	} else if (msg._name == "SHOW_CURSOR") {
		showCursor();
		return true;
	} else if (msg._name == "PROMPT") {
		prompt();
		return true;
	} else if (msg._name == "DELAY") {
		delay(msg._value);
		return true;
	}

	return false;
}

void Commands::writeString(const Common::String &msg) {
	_textCursor.hide();
	_surface.writeString(msg);

	if (msg.contains('\n'))
		delay(NEWLINE_DELAY);
}

void Commands::resetLine() {
	_surface.fillRect(TextRect(0, PROMPT_ROW, 29, PROMPT_ROW), 0);
	_surface.setTextPos(Common::Point(0, PROMPT_ROW));
}

void Commands::showCursor() {
	_textCursor.setPosition(Common::Point(_surface.getTextPos().x, PROMPT_ABSOLUTE_ROW));
	_textCursor.show();
}

void Commands::prompt() {
	resetLine();
	_surface.writeString("CMD: ");
	showCursor();
}

void Commands::delay(uint milli) {
	g_engine->updateScreen();
	g_engine->pauseMillis(milli);
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
