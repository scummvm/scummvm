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
#include "graphics/views/rect.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

constexpr int NEWLINE_DELAY_LONG = 500;
constexpr int NEWLINE_DELAY_SHORT = 50;

Commands::Commands(UIElement *parent) : Graphics::Views::UIElement("Commands", parent), _textCursor("CommandsCursor", this) {
}

void Commands::draw() {
	// Get a surface. We'll maintain a persistent copy to maintain
	// the text pos across multiple calls
	_surface = getSurface();
	_surface.clear();
	_surface.setTextPos(Common::Point(0, 4));
	_surface.setScrollable(true);
	_surface.setColor(_G(colors)._text);

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
	} else if (msg._name == "SPACE2") {
		space2();
		return true;
	} else if (msg._name == "DELAY") {
		delay(msg._value);
		return true;
	}

	return false;
}

void Commands::writeString(const Common::String &msg) {
	// We only want short delays when printing movement actions on the map
	bool isShort = _surface.getTextPos().x == 1 && msg.hasSuffix("\n");

	_textCursor.hide();

	Common::String line;
	auto flush = [&]() {
		if (line.empty())
			return;

		// Text starting a line is indented a column, lining up with the command
		// text that follows the '>' prompt. The prompt glyph itself stays in column 0
		if (_surface.getTextPos().x == 0 && line[0] != '\x10')
			line = " " + line;

		_surface.writeString(line);
		_lineWrapped = _surface.getTextPos().x == 0;
		line.clear();
	};

	for (uint i = 0; i < msg.size(); ++i) {
		if (msg[i] != '\n') {
			line += msg[i];
			continue;
		}

		flush();

		// Text that exactly filled the line has already moved to the next one
		if (_lineWrapped)
			_lineWrapped = false;
		else
			_surface.writeString("\n");
	}

	flush();

	// Delay after each line. For command lines, like movement east/west/etc. keep it short,
	// but all other lines have a more substantial delay to allow time for reading
	if (msg.contains('\n'))
		delay(isShort ? NEWLINE_DELAY_SHORT : NEWLINE_DELAY_LONG);
}

void Commands::resetLine() {
	_surface.fillRect(Graphics::Views::TextRect(0, 4, 29, 4), 0);	// Clear entire bottom row
	_surface.setTextPos(Common::Point(0, 4));
	_lineWrapped = false;
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

void Commands::space2() {
	resetLine();
	_surface.writeString(" Press Space to continue: ");
	showCursor();
}

void Commands::delay(uint milli) {
	g_engine->updateScreen();
	g_engine->pauseMillis(milli);
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
