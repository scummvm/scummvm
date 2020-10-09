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

#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/views/textview.h"

namespace Ultima {
namespace Ultima4 {

Image *TextView::_charset = nullptr;

TextView::TextView(int x, int y, int columns, int rows) : View(x, y, columns * CHAR_WIDTH, rows * CHAR_HEIGHT) {
	this->_columns = columns;
	this->_rows = rows;
	this->_cursorEnabled = false;
	this->_cursorFollowsText = false;
	this->_cursorX = 0;
	this->_cursorY = 0;
	this->_cursorPhase = 0;
	if (_charset == nullptr)
		_charset = imageMgr->get(BKGD_CHARSET)->_image;
	eventHandler->getTimer()->add(&cursorTimer, /*SCR_CYCLE_PER_SECOND*/4, this);
}

TextView::~TextView() {
	eventHandler->getTimer()->remove(&cursorTimer, this);
}

void TextView::reinit() {
	View::reinit();
	_charset = imageMgr->get(BKGD_CHARSET)->_image;
}

void TextView::drawChar(int chr, int x, int y) {
	assertMsg(x < _columns, "x value of %d out of range", x);
	assertMsg(y < _rows, "y value of %d out of range", y);

	_charset->drawSubRect(SCALED(_bounds.left + (x * CHAR_WIDTH)),
	                      SCALED(_bounds.top + (y * CHAR_HEIGHT)),
	                      0, SCALED(chr * CHAR_HEIGHT),
	                      SCALED(CHAR_WIDTH),
	                      SCALED(CHAR_HEIGHT));
}

void TextView::drawCharMasked(int chr, int x, int y, byte mask) {
	drawChar(chr, x, y);
	for (int i = 0; i < 8; i++) {
		if (mask & (1 << i)) {
			_screen->fillRect(SCALED(_bounds.left + (x * CHAR_WIDTH)),
			                  SCALED(_bounds.top + (y * CHAR_HEIGHT) + i),
			                  SCALED(CHAR_WIDTH),
			                  SCALED(1),
			                  0, 0, 0);
		}
	}
}

void TextView::textSelectedAt(int x, int y, const char *text) {
	if (!settings._enhancements || !settings._enhancementsOptions._textColorization) {
		this->textAt(x, y, "%s", text);
		return;
	}

	this->setFontColorBG(BG_BRIGHT);
	for (int i = 0; i < this->getWidth() - 1; i++)
		this->textAt(x - 1 + i, y, " ");
	this->textAt(x, y, "%s", text);
	this->setFontColorBG(BG_NORMAL);
}

Common::String TextView::colorizeStatus(char statustype) {
	Common::String output;

	if (!settings._enhancements || !settings._enhancementsOptions._textColorization) {
		output = statustype;
		return output;
	}

	switch (statustype) {
	case 'P':
		output = FG_GREEN;
		break;
	case 'S':
		output = FG_PURPLE;
		break;
	case 'D':
		output = FG_RED;
		break;
	default:
		output = statustype;
		return output;
	}
	output += statustype;
	output += FG_WHITE;
	return output;
}

Common::String TextView::colorizeString(Common::String input, ColorFG color, uint colorstart, uint colorlength) {
	if (!settings._enhancements || !settings._enhancementsOptions._textColorization)
		return input;

	Common::String output = "";
	size_t length = input.size();
	size_t i;
	bool colorization = false;

	// loop through the entire Common::String and
	for (i = 0; i < length; i++) {
		if (i == colorstart) {
			output += color;
			colorization = true;
		}
		output += input[i];
		if (colorization) {
			colorlength--;
			if (colorlength == 0) {
				output += FG_WHITE;
				colorization = false;
			}
		}
	}

	// if we reached the end of the Common::String without
	// resetting the color to white, do it now
	if (colorization)
		output += FG_WHITE;

	return output;
}

void TextView::setFontColor(ColorFG fg, ColorBG bg) {
	_charset->setFontColorFG(fg);
	_charset->setFontColorBG(bg);
}

void TextView::setFontColorFG(ColorFG fg) {
	_charset->setFontColorFG(fg);
}

void TextView::setFontColorBG(ColorBG bg) {
	_charset->setFontColorBG(bg);
}

void TextView::textAt(int x, int y, const char *fmt, ...) {
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	optionAt(x, y, '\0', "%s", buffer);
}

void TextView::optionAt(int x, int y, char key, const char *fmt, ...) {
	char buffer[1024];
	uint i;
	uint offset = 0;

	bool reenableCursor = false;
	if (_cursorFollowsText && _cursorEnabled) {
		disableCursor();
		reenableCursor = true;
	}

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	for (i = 0; i < strlen(buffer); i++) {
		switch (buffer[i]) {
		case FG_GREY:
		case FG_BLUE:
		case FG_PURPLE:
		case FG_GREEN:
		case FG_RED:
		case FG_YELLOW:
		case FG_WHITE:
			setFontColorFG((ColorFG)buffer[i]);
			offset++;
			break;
		default:
			drawChar(buffer[i], x + (i - offset), y);
		}
	}

	if (_cursorFollowsText)
		setCursorPos(x + i, y, true);
	if (reenableCursor)
		enableCursor();

	if (key) {
		Common::Rect r(
			SCALED(_bounds.left + (x * CHAR_WIDTH)),
			SCALED(_bounds.top + (y * CHAR_HEIGHT)),
			SCALED(_bounds.left + (x + strlen(buffer) - offset) * CHAR_WIDTH),
			SCALED(_bounds.top + (y + 1) * CHAR_HEIGHT)
		);

		_options.push_back(Option(r, key));
	}
}

void TextView::scroll() {
	_screen->drawSubRectOn(_screen,
	                       SCALED(_bounds.left),
	                       SCALED(_bounds.top),
	                       SCALED(_bounds.left),
	                       SCALED(_bounds.top) + SCALED(CHAR_HEIGHT),
	                       SCALED(_bounds.width()),
	                       SCALED(_bounds.height()) - SCALED(CHAR_HEIGHT));

	_screen->fillRect(SCALED(_bounds.left),
	                  SCALED(_bounds.top + (CHAR_HEIGHT * (_rows - 1))),
	                  SCALED(_bounds.width()),
	                  SCALED(CHAR_HEIGHT),
	                  0, 0, 0);

	update();
}

void TextView::setCursorPos(int x, int y, bool clearOld) {
	while (x >= _columns) {
		x -= _columns;
		y++;
	}
	assertMsg(y < _rows, "y value of %d out of range", y);

	if (clearOld && _cursorEnabled) {
		drawChar(' ', _cursorX, _cursorY);
		update(_cursorX * CHAR_WIDTH, _cursorY * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
	}

	_cursorX = x;
	_cursorY = y;

	drawCursor();
}

void TextView::enableCursor() {
	_cursorEnabled = true;
	drawCursor();
}

void TextView::disableCursor() {
	_cursorEnabled = false;
	drawChar(' ', _cursorX, _cursorY);
	update(_cursorX * CHAR_WIDTH, _cursorY * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
}

void TextView::drawCursor() {
	assertMsg(_cursorPhase >= 0 && _cursorPhase < 4, "invalid cursor phase: %d", _cursorPhase);

	if (!_cursorEnabled)
		return;

	drawChar(31 - _cursorPhase, _cursorX, _cursorY);
	update(_cursorX * CHAR_WIDTH, _cursorY * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
}

void TextView::cursorTimer(void *data) {
	TextView *thiz = static_cast<TextView *>(data);
	thiz->_cursorPhase = (thiz->_cursorPhase + 1) % 4;
	thiz->drawCursor();
}

char TextView::getOptionAt(const Common::Point &mousePos) {
	for (uint idx = 0; idx < _options.size(); ++idx) {
		if (_options[idx].contains(mousePos))
			return _options[idx]._key;
	}

	return '\0';
}

void TextView::clearOptions() {
	_options.clear();
}

Common::Rect TextView::getTextBounds(int x, int y, int textWidth) const {
	return Common::Rect(
		SCALED(_bounds.left + (x * CHAR_WIDTH)),
		SCALED(_bounds.top + (y * CHAR_HEIGHT)),
		SCALED(_bounds.left + (x + textWidth * CHAR_WIDTH)),
		SCALED(_bounds.top + (y + 1) * CHAR_HEIGHT)
	);
}

} // End of namespace Ultima4
} // End of namespace Ultima
