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
#include "darkseed/big5font.h"
#include "darkseed/console.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

static constexpr Common::Rect consoleArea = {{0x70, 279}, 416, 49};

Console::Console(TosText *tosText, Sound *sound) : _tosText(tosText), _sound(sound) {
	if (g_engine->getLanguage() == Common::ZH_ANY) {
		_font = new Big5Font();
		_isBig5 = true;
	} else {
		_font = new GameFont();
	}

	_numLines = _isBig5 ? 3 : 4;

	_text.resize(10);
}

void Console::printTosText(int tosIndex) {
	const Common::String &text = _tosText->getText(tosIndex);
	// debugN("tos %d: ", tosIndex);
	// for (int i = 0; i < text.size(); i++) {
	// 	debugN("%02x,", (unsigned char)text[i]);
	// }
	// debug("");
	debug("%s", text.c_str());
	addTextLine(text);
	_sound->playTosSpeech(tosIndex);
}

void Console::addTextLine(const Common::String &text) {
	Common::StringArray lines;
	if (_isBig5) {
		big5WordWrap(text, 23, lines);
	} else {
		_font->wordWrapText(text, consoleArea.width(), lines);
	}
	for (auto &line : lines) {
		addLine(line);
	}
}

void Console::addToCurrentLine(const Common::String &text) {
	int curIdx = _startIdx == 0 ? _text.size() - 1 : _startIdx - 1;
	_startIdx = curIdx;
	addTextLine(_text[_startIdx] + text);
}

void Console::addI18NText(const I18nText &text) {
	addTextLine(getI18NText(text));
}

void Console::draw(bool forceRedraw) {
	if (!_redrawRequired && !forceRedraw) {
		return;
	}

	g_engine->_screen->fillRect(consoleArea, 0);
	int curIdx = _startIdx == 0 ? _text.size() - 1 : _startIdx - 1;
	int y = 0x139;
	for (int i = 0; i < _numLines && curIdx != _startIdx && !_text[curIdx].empty(); i++) {
		drawStringAt(0x70, y, _text[curIdx]);
		y -= _isBig5 ? 17 : 11;
		curIdx = curIdx == 0 ? _text.size() - 1 : curIdx - 1;
	}
	_redrawRequired = false;
	g_engine->_screen->addDirtyRect(consoleArea);
}

void Console::drawStringAt(const int x, const int y, const Common::String &text) const {
	if (_isBig5) {
		Common::Point charPos = {(int16)x, (int16)y};
		for (const char *curCharPtr = text.c_str(); *curCharPtr; ++curCharPtr) {
			byte curChar = *curCharPtr;
			byte nextChar = curCharPtr[1];
			if ((curChar & 0x80) && nextChar) {
				curCharPtr++;
				uint16 point = (curChar << 8) | nextChar;
				_font->drawChar(g_engine->_screen, point, charPos.x, charPos.y, 0);
				charPos.x += (int16)_font->getCharWidth(point) + 1;
			}
		}
	} else {
		_font->drawString(g_engine->_screen, text, x, y, consoleArea.width(), 0);
	}
}

void Console::addLine(const Common::String &line) {
	_text[_startIdx] = line;
	_startIdx = (_startIdx + 1) % _text.size();
	_redrawRequired = true;
}

void Console::big5WordWrap(const Common::String &str, int maxWidth, Common::StringArray &lines) {
	Common::String line;
	int charCount = 0;
	for (const char *curCharPtr = str.c_str(); *curCharPtr; ++curCharPtr) {
		byte curChar = *curCharPtr;
		byte nextChar = curCharPtr[1];
		if ((curChar & 0x80) && nextChar) {
			curCharPtr++;
			if (charCount < maxWidth) {
				line += (char)curChar;
				line += (char)nextChar;
				charCount++;
			} else {
				lines.push_back(line);
				line.clear();
				line += (char)curChar;
				line += (char)nextChar;
				charCount = 1;
			}
		}
	}
	if (!line.empty()) {
		lines.push_back(line);
	}
}

} // End of namespace Darkseed
