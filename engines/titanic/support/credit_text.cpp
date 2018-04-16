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

#include "titanic/support/credit_text.h"
#include "titanic/core/game_object.h"
#include "titanic/events.h"
#include "titanic/support/files_manager.h"
#include "titanic/support/screen_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

#define FRAMES_PER_CYCLE 16

CCreditText::CCreditText() : _screenManagerP(nullptr), _ticks(0),
		_fontHeight(1), _objectP(nullptr), _yOffset(0),
		_priorInc(0), _textR(0), _textG(0), _textB(0), _deltaR(0),
		_deltaG(0), _deltaB(0), _counter(0) {
}

void CCreditText::clear() {
	_groups.destroyContents();
	_objectP = nullptr;
}

void CCreditText::load(CGameObject *obj, CScreenManager *screenManager,
		const Rect &rect) {
	_objectP = obj;
	_screenManagerP = screenManager;
	_rect = rect;

	setup();

	_ticks = g_vm->_events->getTicksCount();
	_priorInc = 0;
	_textR = 0xFF;
	_textG = 0xFF;
	_textB = 0xFF;
	_deltaR = 0;
	_deltaG = 0;
	_deltaB = 0;
	_counter = 0;
}

void CCreditText::setup() {
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource(
		CString::format("TEXT/155"));
	int oldFontNumber = _screenManagerP->setFontNumber(3);
	_fontHeight = _screenManagerP->getFontHeight();

	while (stream->pos() < stream->size()) {
		// Read in the line
		CString srcLine = readLine(stream);

		// Create a new group and line within it
		CCreditLineGroup *group = new CCreditLineGroup();
		CCreditLine *line = new CCreditLine(srcLine,
			_screenManagerP->stringWidth(srcLine));
		group->_lines.push_back(line);

		// Loop to add more lines to the group
		bool hasDots = false;
		while (stream->pos() < stream->size()) {
			srcLine = readLine(stream);
			if (srcLine.empty())
				break;

			line = new CCreditLine(srcLine,
				_screenManagerP->stringWidth(srcLine));
			group->_lines.push_back(line);

			if (srcLine.contains("...."))
				hasDots = true;
		}

		_groups.push_back(group);
		if (hasDots)
			handleDots(group);
	}

	_screenManagerP->setFontNumber(oldFontNumber);
	_groupIt = _groups.begin();
	_lineIt = (*_groupIt)->_lines.begin();
	_yOffset = _objectP->_bounds.height() + _fontHeight * 2;
}

CString CCreditText::readLine(Common::SeekableReadStream *stream) {
	CString line;
	char c = stream->readByte();

	while (c != '\r' && c != '\n' && c != '\0') {
		line += c;

		if (stream->pos() == stream->size())
			break;
		c = stream->readByte();
	}

	if (c == '\r') {
		// Read following '\n'
		stream->readByte();
	}

	return line;
}

void CCreditText::handleDots(CCreditLineGroup *group) {
	uint maxWidth = 0;
	CCreditLines::iterator second = group->_lines.begin();
	++second;

	// Figure out the maximum width of secondary lines
	for (CCreditLines::iterator i = second; i != group->_lines.end(); ++i)
		maxWidth = MAX(maxWidth, (*i)->_lineWidth);

	int charWidth = _screenManagerP->stringWidth(".");

	// Process the secondary lines
	for (CCreditLines::iterator i = second; i != group->_lines.end(); ++i) {
		CCreditLine *line = *i;
		if (line->_lineWidth >= maxWidth)
			continue;

		int dotsCount = (maxWidth + charWidth / 2 - line->_lineWidth) / charWidth;
		int dotIndex = line->_line.indexOf("....");

		if (dotIndex > 0) {
			CString leftStr = line->_line.left(dotIndex);
			CString dotsStr('.', dotsCount);
			CString rightStr = line->_line.right(dotIndex);

			line->_line = CString::format("%s%s%s", leftStr.c_str(),
				dotsStr.c_str(), rightStr.c_str());
			line->_lineWidth = maxWidth;
		}
	}
}

bool CCreditText::draw() {
	if (_groupIt == _groups.end())
		return false;

	if (++_counter >= FRAMES_PER_CYCLE) {
		_textR += _deltaR;
		_textG += _deltaG;
		_textB += _deltaB;
		_deltaR = g_vm->getRandomNumber(63) + 192 - _textR;
		_deltaG = g_vm->getRandomNumber(63) + 192 - _textG;
		_deltaB = g_vm->getRandomNumber(63) + 192 - _textB;
		_counter = 0;
	}

	// Positioning adjustment, changing lines and/or group if necessary
	int yDiff = (int)(g_vm->_events->getTicksCount() - _ticks) / 22 - _priorInc;

	while (yDiff > 0) {
		if (_yOffset > 0) {
			if (yDiff < _yOffset) {
				_yOffset -= yDiff;
				_priorInc += yDiff;
				yDiff = 0;
			} else {
				yDiff -= _yOffset;
				_priorInc += _yOffset;
				_yOffset = 0;
			}
		} else {
			if (yDiff < _fontHeight)
				break;

			++_lineIt;
			yDiff -= _fontHeight;
			_priorInc += _fontHeight;

			if (_lineIt == (*_groupIt)->_lines.end()) {
				// Move to next line group
				++_groupIt;
				if (_groupIt == _groups.end())
					// Reached end of groups
					return false;

				_lineIt = (*_groupIt)->_lines.begin();
				_yOffset = _fontHeight * 3 / 2;
			}
		}
	}

	int oldFontNumber = _screenManagerP->setFontNumber(3);
	CCreditLineGroups::iterator groupIt = _groupIt;
	CCreditLines::iterator lineIt = _lineIt;

	Point textPos;
	for (textPos.y = _rect.top + _yOffset - yDiff; textPos.y <= _rect.bottom;
			textPos.y += _fontHeight) {
		int textR = _textR + _deltaR * _counter / FRAMES_PER_CYCLE;
		int textG = _textG + _deltaG * _counter / FRAMES_PER_CYCLE;
		int textB = _textB + _deltaB * _counter / FRAMES_PER_CYCLE;

		// Single iteration loop to figure out RGB values for the line
		do {
			int percent = 0;
			if (textPos.y < (_rect.top + 2 * _fontHeight)) {
				percent = (textPos.y - _rect.top) * 100 / (_fontHeight * 2);
				if (percent < 0)
					percent = 0;
			} else {
				int bottom = _rect.bottom - 2 * _fontHeight;
				if (textPos.y < bottom)
					break;

				percent = (_rect.bottom - textPos.y) * 100
					/ (_fontHeight * 2);
			}

			// Adjust the RGB to the specified percentage intensity
			textR = textR * percent / 100;
			textG = textG * percent / 100;
			textB = textB * percent / 100;
		} while (0);

		// Write out the line
		_screenManagerP->setFontColor(textR, textG, textB);
		textPos.x = _rect.left + (_rect.width() - (*lineIt)->_lineWidth) / 2;
		_screenManagerP->writeString(SURFACE_BACKBUFFER, textPos,
			_rect, (*lineIt)->_line, (*lineIt)->_lineWidth);

		// Move to next line
		++lineIt;
		if (lineIt == (*groupIt)->_lines.end()) {
			++groupIt;
			if (groupIt == _groups.end())
				// Finished all lines
				break;

			lineIt = (*groupIt)->_lines.begin();
			textPos.y += _fontHeight * 3 / 2;
		}
	}

	_objectP->makeDirty();
	_screenManagerP->setFontNumber(oldFontNumber);
	return true;
}

} // End of namespace Titanic
