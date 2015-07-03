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
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_journal.h"
#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo_talk.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

#define STATEMENT_NUM_X 6
#define NUM_VISIBLE_TALK_LINES 6

WidgetTalk::WidgetTalk(SherlockEngine *vm) : WidgetBase(vm) {
	_talkScrollIndex = 0;
	_selector = _oldSelector = -1;
	_talkTextX = 0;
	_dialogTimer = 0;
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
		_scroll = false;
	} else {
		// Set up the height to a constrained amount, and add extra width for the scrollbar
		width += BUTTON_SIZE + 3;
		height = (_surface.fontHeight() + 1) * 6 + 9;
		_scroll = false;
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
	if (_scroll) {
		int xp = _surface.w() - BUTTON_SIZE - 6;
		_surface.vLine(xp, 3, _surface.h() - 4, INFO_TOP);
		_surface.vLine(xp + 1, 3, _surface.h() - 4, INFO_MIDDLE);
		_surface.vLine(xp + 2, 3, _surface.h() - 4, INFO_BOTTOM);
		_surface.transBlitFrom(images[6], Common::Point(xp - 1, 1));
		_surface.transBlitFrom(images[7], Common::Point(xp - 1, _surface.h() - 4));
	}
}

void WidgetTalk::handleEvents() {
	Events &events = *_vm->_events;
	TattooJournal &journal = *(TattooJournal *)_vm->_journal;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Sound &sound = *_vm->_sound;
	TattooTalk &talk = *(TattooTalk *)_vm->_talk;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();
	Common::KeyCode keycode = ui._keyState.keycode;
	bool hotkey = false;
	bool callParrotFile = false;

	// Handle scrollbar events
	ScrollHighlight oldHighlight = ui._scrollHighlight;
	handleScrollbarEvents(_talkScrollIndex, NUM_VISIBLE_TALK_LINES, _statementLines.size());

	// If the highlight has changed, redraw the scrollbar
	if (ui._scrollHighlight != oldHighlight)
		render(HL_SCROLLBAR_ONLY);

	if (ui._scrollHighlight != SH_NONE || keycode == Common::KEYCODE_HOME || keycode == Common::KEYCODE_END
			|| keycode == Common::KEYCODE_PAGEUP || keycode == Common::KEYCODE_PAGEDOWN) {
		int scrollIndex = _talkScrollIndex;

		// Check for the scrollbar
		if (ui._scrollHighlight == SH_THUMBNAIL) {
			int yp = mousePos.y;
			yp = CLIP(yp, _bounds.top + BUTTON_SIZE + 3, _bounds.bottom - BUTTON_SIZE - 3);

			// Calculate the line number that corresponds to the position that the mouse is on the scrollbar
			int lineNum = (yp - _bounds.top - BUTTON_SIZE - 3) * 100 / (_bounds.height() - BUTTON_SIZE * 2 - 6)
				* _statementLines.size() / 100 - 3;

			// If the new position would place part of the text outsidethe text window, adjust it so it doesn't
			if (lineNum < 0)
				lineNum = 0;
			else if (lineNum + NUM_VISIBLE_TALK_LINES > (int)_statementLines.size()) {
				lineNum = (int)_statementLines.size() - NUM_VISIBLE_TALK_LINES;

				// Make sure it's not below zero now
				if (lineNum < 0)
					lineNum = 0;
			}

			_talkScrollIndex = lineNum;
		}

		// Get the current frame so we can check the scroll timer against it
		uint32 frameNum = events.getFrameCounter();

		if (frameNum > _dialogTimer) {
			// Set the timeout for the next scroll if the mouse button remains held down
			_dialogTimer = (_dialogTimer == 0) ? frameNum + NUM_VISIBLE_TALK_LINES : frameNum + 1;

			// Check for Scroll Up
			if (ui._scrollHighlight == SH_SCROLL_UP && _talkScrollIndex)
				--_talkScrollIndex;

			// Check for Page Up
			if ((ui._scrollHighlight == SH_PAGE_UP || keycode == Common::KEYCODE_PAGEUP) && _talkScrollIndex)
				_talkScrollIndex -= NUM_VISIBLE_TALK_LINES;

			// Check for Page Down
			if ((ui._scrollHighlight == SH_PAGE_DOWN || keycode == Common::KEYCODE_PAGEDOWN) 
					&& (_talkScrollIndex + NUM_VISIBLE_TALK_LINES < (int)_statementLines.size())) {
				_talkScrollIndex += 6;
				if (_talkScrollIndex + NUM_VISIBLE_TALK_LINES >(int)_statementLines.size())
					_talkScrollIndex = _statementLines.size() - NUM_VISIBLE_TALK_LINES;
			}

			// Check for Scroll Down
			if (ui._scrollHighlight == SH_SCROLL_DOWN && (_talkScrollIndex + NUM_VISIBLE_TALK_LINES < (int)_statementLines.size()))
				_talkScrollIndex++;
		}

		if (keycode == Common::KEYCODE_END)
			_talkScrollIndex = _statementLines.size() - NUM_VISIBLE_TALK_LINES;

		if (_talkScrollIndex < 0 || keycode == Common::KEYCODE_HOME)
			_talkScrollIndex = 0;

		// Only redraw the window if the the scrollbar position has changed
		if (scrollIndex != _talkScrollIndex) {
			_surface.fillRect(Common::Rect(4, 5, _surface.w() - BUTTON_SIZE - 8, _surface.h() - 4), TRANSPARENCY);
			render(HL_NO_HIGHLIGHTING);
		}
	}

	// Flag if they started pressing outside of the window
	if (events._firstPress && !_bounds.contains(mousePos))
		_outsideMenu = true;
		
	// Check for which statement they are pointing at
	_selector = -1;
	if (ui._scrollHighlight == SH_NONE) {
		if (Common::Rect(_bounds.left, _bounds.top + 5, _bounds.right - 3, _bounds.bottom - 5).contains(mousePos)) {
			if (_scroll) {
				// Disregard the scrollbar when setting the statement number
				if (Common::Rect(_bounds.right - BUTTON_SIZE - 6, _bounds.top + 3, _bounds.right - 3, _bounds.bottom - 3).contains(mousePos))
					_selector = (mousePos.y - _bounds.top - 5) / (_surface.fontHeight() + 1) + _talkScrollIndex;
			} else {
				_selector = (mousePos.y - _bounds.top - 5) / (_surface.fontHeight() + 1);
			}

			// Now translate the line number of the displayed line into the appropriate
			// Statement number or set it to 255 to indicate no Statement selected
			if (_selector < (int)_statementLines.size())
				_selector = _statementLines[_selector]._num;
			else
				_selector = -1;
		}
	}

	// Check for the tab keys
	if (keycode == Common::KEYCODE_TAB && ui._scrollHighlight == SH_NONE) {
		if (_selector == -1) {
			_selector = _statementLines[_scroll ? _talkScrollIndex : 0]._num;
			
			events.warpMouse(Common::Point(_bounds.right - BUTTON_SIZE - 10, _bounds.top + _surface.fontHeight() + 2));
		} else {
			if (ui._keyState.flags & Common::KBD_SHIFT) {
				_selector = (mousePos.y - _bounds.top - 5) / (_surface.fontHeight() + 1) + _talkScrollIndex;
				if (_statementLines[_selector]._num == _statementLines[_talkScrollIndex]._num) {
					_selector = (_bounds.height() - 10) / (_surface.fontHeight() + 1) + _talkScrollIndex;
				} else {
					int idx = _selector;
					do {
						--_selector;
					} while (_selector > 0 && _statementLines[idx]._num == _statementLines[_selector]._num);
				}

				int idx = _selector;
				while ((_statementLines[idx]._num == _statementLines[_selector - 1]._num) && (_selector > _talkScrollIndex))
					--_selector;
			} else {
				_selector = (mousePos.y - _bounds.top - 5) / (_surface.fontHeight() + 1) + _talkScrollIndex;
				if (_statementLines[_selector]._num == _statementLines[(_bounds.height() - 10) / (_surface.fontHeight() + 1) + _talkScrollIndex]._num) {
					_selector = _talkScrollIndex;
				} else {
					int idx = _selector;
					do {
						++_selector;
					} while (_selector < (int)_statementLines.size() && _statementLines[idx]._num == _statementLines[_selector]._num);
				}
			}

			events.warpMouse(Common::Point(mousePos.x, _bounds.top + _surface.fontHeight() + 2 + (_surface.fontHeight() + 1)
				* (_selector - _talkScrollIndex)));
			_selector = _statementLines[_selector]._num;
		}
	}

	// Handle selecting a talk entry if a numeric key has been pressed
	if (keycode >= Common::KEYCODE_1 && keycode <= Common::KEYCODE_9) {
		int x = 0, t = 0, y = 0;

		do {
			if (y == (keycode - Common::KEYCODE_1)) {
				_selector = _statementLines[t]._num;
				_outsideMenu = false;
				hotkey = true;
				break;
			}

			++t;
			if (_statementLines[x]._num != _statementLines[t]._num) {
				x = t;
				++y;
			}
		} while (t < (int)_statementLines.size());
	}

	// Display the selected statement highlighted and reset the last statement.
	if (_selector != _oldSelector) {
		render(HL_CHANGED_HIGHLIGHTS);
		_oldSelector = _selector;
	}

	if (events._released || events._rightReleased || keycode == Common::KEYCODE_ESCAPE || hotkey) {
		_dialogTimer = 0;
		ui._scrollHighlight = SH_NONE;

		// See if they want to close the menu (click outside the window or Escape pressed)
		if ((_outsideMenu && _bounds.contains(mousePos)) || keycode == Common::KEYCODE_ESCAPE) {
			if (keycode == Common::KEYCODE_ESCAPE)
				_selector = -1;

			talk.freeTalkVars();
			talk.pullSequence();

			for (int idx = 1; idx < MAX_CHARACTERS; ++idx) {
				if (people[idx]._type == CHARACTER) {
					while (people[idx]._pathStack.empty())
						people[idx].pullNPCPath();
				}
			}

			banishWindow();
			ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;

			if (scene._currentScene == 52)
				callParrotFile = true;
		}

		_outsideMenu = false;

		// See if they have selected a statement to say
		if (_selector != -1) {
			if (!talk._talkHistory[talk._converseNum][_selector] && talk._statements[_selector]._journal)
				journal.record(talk._converseNum, _selector);
			talk._talkHistory[talk._converseNum][_selector] = true;

			talk._speaker = _vm->readFlags(76) ? HOLMES : WATSON;
			_scroll = false;
			const byte *msg = (const byte *)talk._statements[_selector]._statement.c_str();
			talk.talkInterface(msg);

			if (sound._speechOn)
				sound._talkSoundFile += Common::String::format("%02dA", _selector + 1);

			int msgLen = MAX((int)talk._statements[_selector]._statement.size(), 160);
			people.setTalkSequence(talk._speaker);

			talk.waitForMore(msgLen);
			if (talk._talkToAbort)
				return;

			people.setListenSequence(talk._speaker);

			do {
				talk._scriptSelect = _selector;
				talk._speaker = talk._talkTo;
				talk.talkTo(talk._statements[_selector]._reply);

				// Reset the misc field in case any people changed their sequences
				for (int idx = 0; idx < MAX_CHARACTERS; ++idx)
					people[idx]._misc = 0;

				if (!talk._talkToAbort) {
					if (!talk._statements[_selector]._modified.empty()) {
						for (uint idx = 0; idx < talk._statements[_selector]._modified.size(); ++idx)
							_vm->setFlags(talk._statements[_selector]._modified[idx]);

						talk.setTalkMap();
					}

					// See if there is another talk file linked to this.
					if (!talk._statements[_selector]._linkFile.empty() && !talk._scriptMoreFlag) {
						Common::String linkFile = talk._statements[_selector]._linkFile;
						talk.freeTalkVars();
						talk.loadTalkFile(linkFile);

						_talkScrollIndex = 0;
						int select = -1;
						_selector = _oldSelector = -1;

						// Find the first statement that has all it's flags set correctly
						for (uint idx = 0; idx < talk._statements.size() && select == -1; ++select) {
							if (!talk._statements[idx]._talkMap)
								select = idx;
						}

						if (select == -1) {
							talk.freeTalkVars();
							ui.putMessage("%s", FIXED(NothingToSay));
							return;
						}

						// See is the new statement is in stealth mode
						talk._talkStealth = (talk._statements[select]._statement.hasPrefix("^")) ? 2 : 0;

						// See if the new file is a standard file, a reply first file, or a Stealth Mode file
						if (!talk._statements[select]._statement.hasPrefix("*") && !talk._statements[select]._statement.hasPrefix("^")) {
							talk.openTalkWindow();
							setStatementLines();
							render(HL_NO_HIGHLIGHTING);
							break;
						} else {
							_selector = select;

							if (!talk._talkHistory[talk._converseNum][_selector] && talk._statements[_selector]._journal)
								journal.record(talk._converseNum, _selector);

							talk._talkHistory[talk._converseNum][_selector] = true;
						}
					} else {
						talk.freeTalkVars();
						talk.pullSequence();

						for (int idx = 1; idx < MAX_CHARACTERS; ++idx) {
							if (people[idx]._type == CHARACTER)
								while (!people[idx]._pathStack.empty())
									people[idx].pullNPCPath();
						}

						banishWindow();
						ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
						break;
					}
				} else {
					break;
				}
			} while (!_vm->shouldQuit());

			events.clearEvents();

			// Now, if a script was pushed onto the script stack, restore them to allow the previous script to continue.
			talk.popStack();
		}
	}

	if (callParrotFile)
		talk.talkTo("POUT52A");
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
	if (_scroll && highlightMode != HL_CHANGED_HIGHLIGHTS)
		drawScrollBar(_talkScrollIndex, NUM_VISIBLE_TALK_LINES, _statementLines.size());
}

void WidgetTalk::setStatementLines() {
	TattooTalk &talk = *(TattooTalk *)_vm->_talk;
	const char *numStr = "19.";

	// See how many statements are going to be available
	int numStatements = 0;
	for (uint idx = 0; idx < talk._statements.size(); ++idx) {
		if (talk._statements[idx]._talkMap != -1)
			++numStatements;
	}

	// If there are more lines than can be displayed in the interface window at one time, adjust the allowed 
	// width to take into account needing a scrollbar
	int xSize = _scroll ? _bounds.width() - BUTTON_SIZE - 3 : _bounds.width();

	// Also adjust the width to allow room for the statement numbers at the left edge of the display
	int n = (numStatements < 10) ? 1 : 0;
	xSize -= _surface.stringWidth(numStr + n) + _surface.widestChar() / 2 + 9;
	_talkTextX = _surface.stringWidth(numStr + n) + _surface.widestChar() / 4 + 6;
	_statementLines.clear();

	for (uint statementNum = 0; statementNum < talk._statements.size(); ++statementNum) {
		// See if this statment meets all of it's flag requirements
		if (talk._statements[statementNum]._talkMap != -1) {
			// Get the next statement text to process
			Common::String str = talk._statements[statementNum]._statement;

			// Process the statement
			Common::String line;
			do {
				line = "";

				// Find out how much of the statement will fit on the line
				int width = 0;
				const char *ch = line.c_str();
				const char *space = nullptr;

				while (width < xSize && *ch) {
					width += _surface.charWidth(*ch);

					// Keep track of where spaces are
					if (*ch == ' ')
						space = ch;
					++ch;
				}

				// If the line was too wide to fit on a single line, go back to the last space and split it there.
				// But if there isn't (and this shouldn't ever happen), just split the line right at that point
				if (width > xSize) {
					if (space) {
						line = Common::String(str.c_str(), space - 1);
						str = Common::String(space + 1);
					} else {
						line = Common::String(str.c_str(), ch);
						str = Common::String(ch);
					}
				} else {
					line = str;
					str = "";
				}

				// Add the line in
				_statementLines.push_back(StatementLine(line, statementNum));
			} while (!line.empty());
		}
	}
}

void WidgetTalk::refresh() {
	_talkScrollIndex = 0;
	_selector = _oldSelector = -1;

	setStatementLines();
	render(HL_NO_HIGHLIGHTING);
}

} // End of namespace Tattoo

} // End of namespace Sherlock
