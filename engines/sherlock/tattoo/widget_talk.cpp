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

#include "sherlock/tattoo/widget_talk.h"
#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo_talk.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

#define STATEMENT_NUM_X 6
#define VISIBLE_TALK_LINES 6

WidgetTalk::WidgetTalk(SherlockEngine *vm) : WidgetBase(vm) {
	_talkScroll = false;
	_talkScrollIndex = 0;
	_selector = _oldSelector = -1;
	_talkTextX = 0;
}

void WidgetTalk::getTalkWindowSize() {
	TattooTalk &talk = *(TattooTalk *)_vm->_talk;
	Common::StringArray lines;
	const char *const NUM_STR = "19.";
	int width, height;

	// See how many statements are going to be available
	int numStatements = 0;
	for (uint idx = 0; idx < talk._statements.size(); ++idx) {
		if (talk._statements[idx]._talkMap != -1)
			++numStatements;
	}

	// Figure out the width, allowing room for both the text and the statement numbers on the side
	width = SHERLOCK_SCREEN_WIDTH * 2 / 3;
	int n = (numStatements < 10) ? 1 : 0;
	width -= _surface.stringWidth(NUM_STR + n) + _surface.widestChar() / 2 + 9;

	// Now that we have a width, split up the text into individual lines
	int numLines = 0;
	for (uint idx = 0; idx < talk._statements.size(); ++idx) {
		if (talk._statements[idx]._talkMap != -1) {
			splitLines(talk._statements[idx]._statement, lines, width, 999);
			numLines += lines.size();
		}
	}

	// Make sure that the window does not get too big
	if (numLines < 7) {
		height = (_surface.fontHeight() + 1) * numLines + 9;
		_talkScroll = false;
	} else {
		// Set up the height to a constrained amount, and add extra width for the scrollbar
		width += BUTTON_SIZE + 3;
		height = (_surface.fontHeight() + 1) * 6 + 9;
		_talkScroll = false;
	}

	_bounds = Common::Rect(width, height);

	// Allocate a surface for the window
	_surface.create(_bounds.width(), _bounds.height());
	_surface.fill(TRANSPARENCY);

	// Form the background for the new window
	makeInfoArea();

	int yp = 5;
	for (int lineNum = 0; yp < (_bounds.height() - _surface.fontHeight() / 2); ++lineNum) {
		_surface.writeString(lines[lineNum], Common::Point(_surface.widestChar(), yp), INFO_TOP);
		yp += _surface.fontHeight() + 1;
	}
}

void WidgetTalk::load() {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;

	// Figure out the window size
	getTalkWindowSize();

	// Place the window centered above the player
	Common::Point pt;
	int scaleVal = scene.getScaleVal(people[HOLMES]._position);
	pt.x = people[HOLMES]._position.x / FIXED_INT_MULTIPLIER;

	if (scaleVal == SCALE_THRESHOLD) {
		pt.x += people[0].frameWidth() / 2;
		pt.y = people[HOLMES]._position.y / FIXED_INT_MULTIPLIER - people[HOLMES].frameHeight()
			- _bounds.height() - _surface.fontHeight();
	} else {
		pt.x += people[HOLMES]._imageFrame->sDrawXSize(scaleVal) / 2;
		pt.y = people[HOLMES]._position.y / FIXED_INT_MULTIPLIER - people[HOLMES]._imageFrame->sDrawYSize(scaleVal)
			- _bounds.height() - _surface.fontHeight();
	}

	_bounds.moveTo(pt);

	// Set up the surface
	_surface.create(_bounds.width(), _bounds.height());
	_surface.fill(TRANSPARENCY);

	// Form the background for the new window
	makeInfoArea();

	// If a scrollbar is needed, draw it in
	if (_talkScroll) {
		int xp = _surface.w() - BUTTON_SIZE - 6;
		_surface.vLine(xp, 3, _surface.h() - 4, INFO_TOP);
		_surface.vLine(xp + 1, 3, _surface.h() - 4, INFO_MIDDLE);
		_surface.vLine(xp + 2, 3, _surface.h() - 4, INFO_BOTTOM);
		_surface.transBlitFrom(images[6], Common::Point(xp - 1, 1));
		_surface.transBlitFrom(images[7], Common::Point(xp - 1, _surface.h() - 4));
	}
}

void WidgetTalk::handleEvents() {
	// TODO
}

void WidgetTalk::render(Highlight highlightMode) {
	TattooTalk &talk = *(TattooTalk *)_vm->_talk;
	int yp = 5;
	int statementNum = 1;
	byte color;

	if (highlightMode != HL_SCROLLBAR_ONLY) {
		// Draw all the statements
		// Check whether scrolling has occurred, and if so, figure out what the starting
		// number for the first visible statement will be
		if (_talkScrollIndex) {
			for (int idx = 1; idx <= _talkScrollIndex; ++idx) {
				if (_statementLines[idx - 1]._num != _statementLines[idx]._num)
					++statementNum;
			}
		}

		// Main drawing loop
		for (uint idx = _talkScrollIndex; idx < _statementLines.size() && yp < (_bounds.height() - _surface.fontHeight()); ++idx) {
			if (highlightMode == HL_NO_HIGHLIGHTING || _statementLines[idx]._num == _selector ||
					_statementLines[idx]._num == _oldSelector) {
				// Different coloring based on whether the option has been previously chosen or not
				color = (!talk._talkHistory[talk._converseNum][_statementLines[idx]._num]) ?
					INFO_TOP : INFO_BOTTOM;

				if (_statementLines[idx]._num == _selector && highlightMode == HL_CHANGED_HIGHLIGHTS)
					color = COMMAND_HIGHLIGHTED;

				// See if it's the start of a new statement, so needs the statement number to be displayed
				if (!idx || _statementLines[idx]._num != _statementLines[idx - 1]._num) {
					Common::String numStr = Common::String::format("%d.", statementNum);
					_surface.writeString(numStr, Common::Point(STATEMENT_NUM_X, yp), color);
				}

				// Display the statement line
				_surface.writeString(_statementLines[idx]._line, Common::Point(_talkTextX, yp), color);
			}
			yp += _surface.fontHeight() + 1;

			// If the next line starts a new statement, then increment the statement number
			if (idx == (_statementLines.size() - 1) || _statementLines[idx]._num != _statementLines[idx + 1]._num)
				++statementNum;
		}
	}

	// See if the scroll bar needs to be drawn
	if (_talkScroll && highlightMode != HL_CHANGED_HIGHLIGHTS)
		drawScrollBar(_talkScrollIndex, VISIBLE_TALK_LINES, _statementLines.size());
}

} // End of namespace Tattoo

} // End of namespace Sherlock
