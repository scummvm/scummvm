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

#include "sherlock/tattoo/widget_foolscap.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

WidgetFoolscap::WidgetFoolscap(TattooEngine *vm) : WidgetBase(vm) {
	for (int idx = 0; idx < 3; ++idx) {
		Common::fill(&_answers[idx][0], &_answers[idx][10], 0);
		_solutions[idx] = nullptr;
	}
	_images = nullptr;
	_numWide = 0;
	_spacing = 0;
	_blinkFlag = false;
	_blinkCounter = 0;
	_lineNum = _charNum = 0;
	_solved = false;
}

WidgetFoolscap::~WidgetFoolscap() {
	delete _images;
}

void WidgetFoolscap::show() {
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	switch (_vm->getLanguage()) {
	case Common::FR_FRA:
		_lines[0] = Common::Point(34, 210);
		_lines[1] = Common::Point(72, 242);
		_lines[2] = Common::Point(34, 276);
		_numWide = 8;
		_spacing = 19;
		_images = new ImageFile("paperf.vgs");
		break;

	case Common::DE_DEU:
		_lines[0] = Common::Point(44, 73);
		_lines[1] = Common::Point(56, 169);
		_lines[2] = Common::Point(47, 256);
		_numWide = 7;
		_spacing = 19;
		_images = new ImageFile("paperg.vgs");
		break;

	default:
		// English
		_lines[0] = Common::Point(65, 84);
		_lines[1] = Common::Point(65, 159);
		_lines[2] = Common::Point(75, 234);
		_numWide = 5;
		_spacing = 20;
		_images = new ImageFile("paper.vgs");
		break;
	}

	_solved = false;
	_blinkFlag = false;
	_blinkCounter = 0;
	_lineNum = _charNum = 0;
	_cursorPos = Common::Point(_lines[0].x + 8 - screen.widestChar() / 2, _lines[0].y - screen.fontHeight() - 2);

	// Set up window bounds
	ImageFrame &paperFrame = (*_images)[0];
	_bounds = Common::Rect(paperFrame._width, paperFrame._height);
	_bounds.moveTo(screen._currentScroll.x + (SHERLOCK_SCREEN_WIDTH - paperFrame._width) / 2,
		(SHERLOCK_SCREEN_HEIGHT - paperFrame._height) / 2);

	// Clear answer data and set correct solution strings
	for (int idx = 0; idx < 3; ++idx)
		Common::fill(&_answers[idx][0], &_answers[idx][10], 0);
	_solutions[0] = FIXED(Apply);
	_solutions[1] = FIXED(Water);
	_solutions[2] = FIXED(Heat);

	// Set up the window background
	_surface.create(_bounds.width(), _bounds.height());
	_surface.SHblitFrom(paperFrame, Common::Point(0, 0));

	// If they have already solved the puzzle, put the answer on the graphic
	if (_vm->readFlags(299)) {
		Common::Point cursorPos;
		for (int line = 0; line < 3; ++line) {
			cursorPos.y = _lines[line].y - screen.fontHeight() - 2;

			for (uint idx = 0; idx < strlen(_solutions[line]); ++idx) {
				cursorPos.x = _lines[line].x + 8 - screen.widestChar() / 2 + idx * _spacing;
				char c = _solutions[line][idx];

				Common::String str = Common::String::format("%c", c);
				_surface.writeString(str, Common::Point(cursorPos.x + screen.widestChar() / 2
					- screen.charWidth(c) / 2, cursorPos.y), 0);
			}
		}
	}

	// Show the window
	summonWindow();
	ui._menuMode = FOOLSCAP_MODE;
}

void WidgetFoolscap::handleEvents() {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();
	byte cursorColor = 254;

	if (events._firstPress && !_bounds.contains(mousePos))
		_outsideMenu = true;

	// If they have not solved the puzzle, let them solve it here
	if (!_vm->readFlags(299)) {
		if (!ui._keyState.keycode) {
			if (--_blinkCounter < 0) {
				_blinkCounter = 3;
				_blinkFlag = !_blinkFlag;

				if (_blinkFlag) {
					// Draw the caret
					_surface.fillRect(Common::Rect(_cursorPos.x, _cursorPos.y, _cursorPos.x + screen.widestChar() - 1,
						_cursorPos.y + screen.fontHeight() - 1), cursorColor);

					if (_answers[_lineNum][_charNum]) {
						Common::String str = Common::String::format("%c", _answers[_lineNum][_charNum]);
						_surface.writeString(str, Common::Point(_cursorPos.x + screen.widestChar() / 2
							- screen.charWidth(_answers[_lineNum][_charNum]) / 2, _cursorPos.y), 0);
					}
				} else {
					// Restore background
					restoreChar();

					// Draw the character at that position if there is one
					if (_answers[_lineNum][_charNum]) {
						Common::String str = Common::String::format("%c", _answers[_lineNum][_charNum]);
						_surface.writeString(str, Common::Point(_cursorPos.x + screen.widestChar() / 2
							- screen.charWidth(_answers[_lineNum][_charNum]) / 2, _cursorPos.y), 0);
					}
				}
			}
		} else {
			// Handle keyboard events
			handleKeyboardEvents();
		}
	}

	if ((events._released || events._rightReleased) && _outsideMenu && !_bounds.contains(mousePos)) {
		// Clicked outside window to close it
		events.clearEvents();
		close();
	}
}

void WidgetFoolscap::handleKeyboardEvents() {
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::KeyState keyState = ui._keyState;

	if ((toupper(keyState.ascii) >= 'A') && (toupper(keyState.ascii) <= 'Z')) {
		// Visible key pressed, set it and set the keycode to move the caret to the right
		_answers[_lineNum][_charNum] = keyState.ascii;
		keyState.keycode = Common::KEYCODE_RIGHT;
	}

	// Restore background
	restoreChar();

	if (_answers[_lineNum][_charNum]) {
		Common::String str = Common::String::format("%c", _answers[_lineNum][_charNum]);
		_surface.writeString(str, Common::Point(_cursorPos.x + screen.widestChar() / 2
			- screen.charWidth(_answers[_lineNum][_charNum]) / 2, _cursorPos.y), 0);
	}

	switch (keyState.keycode) {
	case Common::KEYCODE_ESCAPE:
		close();
		break;

	case Common::KEYCODE_UP:
		if (_lineNum) {
			--_lineNum;
			if (_charNum >= (int)strlen(_solutions[_lineNum]))
				_charNum = (int)strlen(_solutions[_lineNum]) - 1;
		}
		break;

	case Common::KEYCODE_DOWN:
		if (_lineNum < 2) {
			++_lineNum;
			if (_charNum >= (int)strlen(_solutions[_lineNum]))
				_charNum = (int)strlen(_solutions[_lineNum]) - 1;
		}
		break;

	case Common::KEYCODE_BACKSPACE:
	case Common::KEYCODE_LEFT:
		if (_charNum)
			--_charNum;
		else if (_lineNum) {
			--_lineNum;

			_charNum = strlen(_solutions[_lineNum]) - 1;
		}

		if (keyState.keycode == Common::KEYCODE_BACKSPACE)
			_answers[_lineNum][_charNum] = ' ';
		break;

	case Common::KEYCODE_RIGHT:
		if (_charNum < (int)strlen(_solutions[_lineNum]) - 1)
			++_charNum;
		else if (_lineNum < 2) {
			++_lineNum;
			_charNum = 0;
		}
		break;

	case Common::KEYCODE_DELETE:
		_answers[_lineNum][_charNum] = ' ';
		break;

	default:
		break;
	}

	_cursorPos.x = _lines[_lineNum].x + 8 - screen.widestChar() / 2 + _charNum * _spacing;
	_cursorPos.y = _lines[_lineNum].y - screen.fontHeight() - 2;

	// See if all of their anwers are correct
	if (!scumm_stricmp(_answers[0], _solutions[0]) && !scumm_stricmp(_answers[1], _solutions[1])
			&& !scumm_stricmp(_answers[2], _solutions[2])) {
		_solved = true;
		close();
	}
}

void WidgetFoolscap::restoreChar() {
	Screen &screen = *_vm->_screen;
	ImageFrame &bgFrame = (*_images)[0];
	_surface.SHblitFrom(bgFrame, _cursorPos, Common::Rect(_cursorPos.x, _cursorPos.y,
		_cursorPos.x + screen.widestChar(), _cursorPos.y + screen.fontHeight()));
}

void WidgetFoolscap::close() {
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Talk &talk = *_vm->_talk;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	delete _images;
	_images = nullptr;

	// Close the window
	banishWindow();
	ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;

	// Don't call the talk files if the puzzle has already been solved
	if (!_vm->readFlags(299)) {
		// Run the appropriate script depending on whether or not they solved the puzzle correctly
		if (_solved) {
			talk.talkTo("SLVE12S.TLK");
			talk.talkTo("WATS12X.TLK");
			_vm->setFlags(299);
		} else {
			talk.talkTo("HOLM12X.TLK");
		}
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
