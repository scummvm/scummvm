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

#include "common/debug.h"
#include "darkseed/console.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

static constexpr Common::Rect consoleArea = {{0x70, 280}, 416, 44};

Console::Console(TosText *tosText, Sound *sound) : _tosText(tosText), _sound(sound) {
	if (!_font.load()) {
		error("Error loading tosfont.nsp");
	}
	_text.resize(10);
}

void Console::printTosText(int tosIndex) {
	const Common::String &text = _tosText->getText(tosIndex);
	debug("%s", text.c_str());
	addLine(" ");
	addTextLine(text);
	_sound->playTosSpeech(tosIndex);
}

void Console::addTextLine(const Common::String &text) {
	Common::StringArray lines;
	_font.wordWrapText(text, consoleArea.width(), lines);
	for (auto &line : lines) {
		addLine(line);
	}
}

void Console::addToCurrentLine(const Common::String &text) {
	int curIdx = _startIdx == 0 ? _text.size() - 1 : _startIdx - 1;
	_startIdx = curIdx;
	addTextLine(_text[_startIdx] + text);
}

void Console::draw() {
	if (!_redrawRequired) {
		return;
	}

	g_engine->_screen->fillRect(consoleArea, 0);
	int curIdx = _startIdx == 0 ? _text.size() - 1 : _startIdx - 1;
	int y = 0x139;
	for (int i = 0; i < 4 && curIdx != _startIdx && !_text[curIdx].empty(); i++) {
		_font.drawString(g_engine->_screen, _text[curIdx], 0x70, y, consoleArea.width(), 0);
		y -= 11;
		curIdx = curIdx == 0 ? _text.size() - 1 : curIdx - 1;
	}
	_redrawRequired = false;
	g_engine->_screen->addDirtyRect(consoleArea);
}

void Console::addLine(const Common::String &line) {
	_text[_startIdx] = line;
	_startIdx = (_startIdx + 1) % _text.size();
	_redrawRequired = true;
}

} // End of namespace Darkseed
