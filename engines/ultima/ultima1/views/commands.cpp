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

#include "ultima/ultima1/views/commands.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

constexpr int COLOR_TEXT = 11;      // light cyan - normal values
constexpr int NEWLINE_DELAY = 50;	// Delay after a newline

Commands::Commands(UIElement *parent) : Shared::UIElement("Commands", parent), _textCursor("CommandsCursor", this) {
}

bool Commands::msgFocus(const FocusMessage &msg) {
	return true;
}

void Commands::draw() {
	// Get a surface. We'll maintain a persistent copy to maintain
	// the text pos across multiple calls
	_surface = getSurface();
	_surface.clear();
	_surface.setTextPos(Common::Point(0, 4));
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
	} else if (msg._name == "SPACE") {
		space();
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

	if (msg.contains('\n')) {
		delay(NEWLINE_DELAY);
	}
}

void Commands::resetLine() {
	_surface.fillRect(TextRect(0, 4, 29, 4), 0);	// Clear entire bottom row
	_surface.setTextPos(Point(0, 4));
}

void Commands::showCursor() {
	_textCursor.setPosition(Common::Point(_surface.getTextPos().x, 24));
	_textCursor.show();
}

void Commands::prompt() {
	resetLine();
	_surface.writeString("\x10");
	showCursor();
}

void Commands::space() {
	resetLine();
	_surface.writeString("\x10Press Space to continue: ");
	showCursor();
}

void Commands::delay(uint milli) {
	g_engine->updateScreen();
	g_engine->pauseMillis(NEWLINE_DELAY);
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
