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
#include "ultima/shared/core/str.h"

namespace Ultima {
namespace Shared {

#define PROMPT_CHAR '\x1'
#define SUGGESTED_CR '\r'

BEGIN_MESSAGE_MAP(Info, Shared::Gfx::VisualItem)
	ON_MESSAGE(InfoMsg)
	ON_MESSAGE(InfoGetCommandKeypress)
	ON_MESSAGE(InfoGetKeypress)
	ON_MESSAGE(InfoGetInput)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

Info::Info(TreeItem *parent, const Rect &bounds) : Gfx::VisualItem("Info", bounds, parent),
		_commandRespondTo(nullptr) {
	_characterInput = new Gfx::CharacterInput(getGame());
	_textInput = new Gfx::TextInput(getGame());
}

Info::~Info() {
	delete _characterInput;
	delete _textInput;
}

bool Info::InfoMsg(CInfoMsg &msg) {
	// Iterate through text, dealing with lines one at a time
	StringArray lines = String(msg._text).split("\r\n");

	if (!_lines.empty() && msg._replaceLine)
		_lines.back() = _lines.back().firstChar();

	for (uint idx = 0; idx < lines.size(); ++idx) {
		if (_lines.empty() || idx > 0)
			_lines.push_back(" ");

		_lines.back() += lines[idx];
	}

	// Add newline if necessary
	if (msg._newLine)
		_lines.push_back(" ");

	setDirty();

	// Adding any text hides any visible cursor
	_commandRespondTo = nullptr;
	Gfx::TextCursor *textCursor = getGame()->_textCursor;
	textCursor->setVisible(false);

	return true;
}

bool Info::InfoGetCommandKeypress(CInfoGetCommandKeypress &msg) {
	if (_lines.empty() || _lines.back() != " ")
		_lines.push_back("");
	_lines.back() = PROMPT_CHAR;

	Gfx::TextCursor *textCursor = getGame()->_textCursor;
	textCursor->setVisible(true);
	textCursor->setPosition(Point(8, _bounds.bottom - 8));

	_commandRespondTo = msg._responder;
	return true;
}

bool Info::InfoGetKeypress(CInfoGetKeypress &msg) {
	Game *game = getGame();
	Point pt(_bounds.left + _lines.back().size() * 8, _bounds.bottom - 8);
	_characterInput->show(pt, game->_textColor, msg._responder);

	return true;
}

bool Info::InfoGetInput(CInfoGetInput &msg) {
	Game *game = getGame();
	Point pt(_bounds.left + _lines.back().size() * 8, _bounds.bottom - 8);
	_textInput->show(pt, msg._isNumeric, msg._maxCharacters, game->_textColor, msg._responder);

	return true;
}

bool Info::KeypressMsg(CKeypressMsg &msg) {
	// If waiting for a command, dispatch the key to the respond, and hide the cursor
	if (_commandRespondTo) {
		TreeItem *target = _commandRespondTo;
		_commandRespondTo = nullptr;

		getGame()->_textCursor->setVisible(false);
		CCharacterInputMsg cMsg(msg._keyState);
		cMsg.execute(target);
		return true;
	}

	return true;
}

void Info::draw() {
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
		s.writeString(line, Point(x, y));
	}

	_isDirty = false;
}

} // End of namespace Shared
} // End of namespace Ultima
