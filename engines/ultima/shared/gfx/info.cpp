/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/shared/gfx/info.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/gfx/text_cursor.h"

namespace Ultima {
namespace Shared {

#define PROMPT_CHAR '\x1'

BEGIN_MESSAGE_MAP(Info, Gfx::VisualItem)
	ON_MESSAGE(InfoMsg)
END_MESSAGE_MAP()

bool Info::InfoMsg(CInfoMsg &msg) {
	// Add new text
	if (_lines.empty())
		_lines.push_back(" ");
	_lines.back() += msg._text;

	if (msg._newLine)
		_lines.push_back(" ");

	return true;
}

void Info::draw() {
	Game *game = getGame();

	// If the bottom line doesn't yet have a prompt, add it in
	if (_lines.empty())
		_lines.push_back(" ");
	if (_lines.back() == " ") {
		_lines.back() = PROMPT_CHAR;
		
		Gfx::TextCursor *textCursor = game->_textCursor;
		textCursor->setVisible(true);
		textCursor->setPosition(Point(8, _bounds.bottom - 8));
	}

	// Clear the background
	Gfx::VisualSurface s = getSurface();
	s.clear();

	// Get the number og lines to display
	uint lineHeight = s.fontHeight();
	uint numLines = (s.h + lineHeight - 1) / lineHeight;

	// Discard any stored lines beyond the top of the display
	while (_lines.size() > numLines)
		_lines.remove_at(0);

	// Display the lines
	int x = 0, y = s.h - _lines.size() * lineHeight;
	for (uint idx = 0; idx < _lines.size(); ++idx, x = 0, y += lineHeight) {
		// Handle drawing the prompt character at the start of lines if necessary
		Common::String line = _lines[idx];
		if (!line.empty() && line[0] == PROMPT_CHAR) {
			drawPrompt(s, Point(0, y));
			x = 8;
			line.deleteChar(0);
		}

		// Write the remainder of the line
		s.writeString(line, Point(x, y), game->_textColor);
	}
}

} // End of namespace Shared
} // End of namespace Ultima
