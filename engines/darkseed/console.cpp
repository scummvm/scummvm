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
#include "darkseed/kofont.h"
#include "darkseed/console.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

static constexpr Common::Rect consoleArea = {{0x70, 279}, 416, 49};

Console::Console(TosText *tosText, Sound *sound) : _tosText(tosText), _sound(sound) {
	switch (g_engine->getLanguage()) {
	case Common::ZH_ANY :
		_font = new Big5Font();
		_lineHeight = 17;
		_numLines = 3;
		break;
	case Common::KO_KOR :
		_font = new KoFont();
		_lineHeight = 18;
		_numLines = 3;
		break;
	default:
		_font = new GameFont();
		_lineHeight = 11;
		_numLines = 4;
		break;
	}

	_text.resize(10);
}

Console::~Console() {
	delete _font;
}

void Console::printTosText(int tosIndex) {
	const Common::U32String &text = _tosText->getText(tosIndex);
	// debugN("tos %d: ", tosIndex);
	// for (int i = 0; i < text.size(); i++) {
	// 	debugN("%02x,", (unsigned char)text[i]);
	// }
	// debug("");
	debug("%s", text.c_str());
	addTextLineU32(text);
	_sound->playTosSpeech(tosIndex);
}

void Console::addTextLine(const Common::String &text) {
	addTextLineU32(Common::U32String(text));
}

void Console::addToCurrentLine(const Common::String &text) {
	addToCurrentLineU32(Common::U32String(text));
}

void Console::addTextLineU32(const Common::U32String &text) {
	Common::U32StringArray lines;
	_font->wordWrapText(text, consoleArea.width(), lines);
	for (auto &line : lines) {
		addLine(line);
	}
}

void Console::addToCurrentLineU32(const Common::U32String &text) {
	int curIdx = _startIdx == 0 ? _text.size() - 1 : _startIdx - 1;
	_startIdx = curIdx;
	addTextLineU32(_text[_startIdx] + text);
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
		y -= _lineHeight;
		curIdx = curIdx == 0 ? _text.size() - 1 : curIdx - 1;
	}
	_redrawRequired = false;
	g_engine->_screen->addDirtyRect(consoleArea);
}

void Console::drawStringAt(const int x, const int y, const Common::U32String &text) const {
	_font->drawString(g_engine->_screen, text, x, y, consoleArea.width(), 0);
}

void Console::addLine(const Common::U32String &line) {
	_text[_startIdx] = line;
	_startIdx = (_startIdx + 1) % _text.size();
	_redrawRequired = true;
}

} // End of namespace Darkseed
