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

#include "sherlock/tattoo/tattoo_journal.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

#define JOURNAL_BAR_WIDTH	450
#define S_NO_TEXT "Text Not Found !"

static const char *const JOURNAL_COMMANDS[2] = { "Close Journal", "Search Journal" };

static const char *const JOURNAL_SEARCH_COMMANDS[3] = { "Abort Search", "Search Backwards", "Search Forwards" };

TattooJournal::TattooJournal(SherlockEngine *vm) : Journal(vm) {
	_journalImages = nullptr;
	_selector = _oldSelector = 0;
	_wait = false;
	_exitJournal = false;
	_scrollingTimer = 0;
	_savedIndex = _savedSub = _savedPage = 0;

	loadLocations();
}

void TattooJournal::show() {
	Events &events = *_vm->_events;
	Resources &res = *_vm->_res;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	byte palette[PALETTE_SIZE];

	// Load journal images
	_journalImages = new ImageFile("journal.vgs");

	// Load palette
	Common::SeekableReadStream *stream = res.load("journal.pal");
	stream->read(palette, PALETTE_SIZE);
	screen.translatePalette(palette);
	ui.setupBGArea(palette);

	// Set screen to black, and set background
	screen._backBuffer1.blitFrom((*_journalImages)[0], Common::Point(0, 0));
	screen.empty();
	screen.setPalette(palette);

	if (_journal.empty()) {
		_up = _down = false;
	} else {
		drawJournal(0, 0);
	}
	drawControls(0);
	screen.slamRect(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

	_exitJournal = false;
	_scrollingTimer = 0;

	do {
		events.pollEventsAndWait();
		_wait = true;

		handleKeyboardEvents();
		highlightJournalControls(true);
		
		handleButtons();

		if (_wait)
			events.wait(2);
		
	} while (!_vm->shouldQuit() && !_exitJournal);

	// Clear events
	events.clearEvents();

	// Free the images
	delete _journalImages;
}

void TattooJournal::handleKeyboardEvents() {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Common::Point mousePos = events.mousePos();

	if (!events.kbHit())
		return;

	Common::KeyState keyState = events.getKey();

	if (keyState.keycode == Common::KEYCODE_TAB && (keyState.flags & Common::KBD_SHIFT)) {
		// Shift tab
		Common::Rect r(JOURNAL_BAR_WIDTH, BUTTON_SIZE + screen.fontHeight() + 13);
		r.moveTo((SHERLOCK_SCREEN_WIDTH - r.width()) / 2, SHERLOCK_SCREEN_HEIGHT - r.height());

		// See if mouse is over any of the journal controls
		_selector = -1;
		if (Common::Rect(r.left + 3, r.top + 3, r.right - 3, r.top + screen.fontHeight() + 4).contains(mousePos))
			_selector = (mousePos.x - r.left) / (r.width() / 3);

		// If the mouse is not over an option, move the mouse to that it points to the first option
		if (_selector == -1) {
			events.warpMouse(Common::Point(r.left + r.width() / 3 - 10, r.top + screen.fontHeight() + 2));
		} else {
			if (_selector == 0)
				_selector = 2;
			else
				--_selector;

			events.warpMouse(Common::Point(r.left + (r.width() / 3) * (_selector + 1) - 10, mousePos.y));
		}

	} else if (keyState.keycode == Common::KEYCODE_PAGEUP || keyState.keycode == Common::KEYCODE_KP9) {
		// See if they have Shift held down to go forward 10 pages
		if (keyState.flags & Common::KBD_SHIFT) {
			if (_page > 1) {
				// Scroll Up 10 pages if possible
				if (_page < 11)
					drawJournal(1, (_page - 1) * LINES_PER_PAGE);
				else
					drawJournal(1, 10 * LINES_PER_PAGE);

				drawScrollBar();
				screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
				_wait = false;
			}
		} else {
			if (_page > 1) {
				// Scroll Up 1 page
				drawJournal(1, LINES_PER_PAGE);
				drawScrollBar();
				show();
				screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
				_wait = false;
			}
		}

	} else if (keyState.keycode == Common::KEYCODE_PAGEDOWN || keyState.keycode == Common::KEYCODE_KP3) {
		if (keyState.flags & Common::KBD_SHIFT) {
			if (_down) {
				// Scroll down 10 Pages
				if (_page + 10 > _maxPage)
					drawJournal(2, (_maxPage - _page) * LINES_PER_PAGE);
				else
					drawJournal(2, 10 * LINES_PER_PAGE);
				drawScrollBar();
				screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

				_wait = false;
			}
		} else {
			if (_down) {
				// Scroll down 1 page
				drawJournal(2, LINES_PER_PAGE);
				drawScrollBar();
				show();
				screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

				_wait = false;
			}
		}

	} else if (keyState.keycode == Common::KEYCODE_HOME || keyState.keycode == Common::KEYCODE_KP7) {
		// Scroll to start of journal
		if (_page > 1) {
			// Go to the beginning of the journal
			_index = _sub = _up = _down = 0;
			_page = 1;

			drawFrame();
			drawJournal(0, 0);

			drawScrollBar();
			screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

			_wait = false;
		}

	} else if (keyState.keycode == Common::KEYCODE_END || keyState.keycode == Common::KEYCODE_KP1) {
		// Scroll to end of journal
		if (_down) {
			// Go to the end of the journal
			drawJournal(2, 100000);
			drawScrollBar();
			screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

			_wait = false;
		}
	} else if (keyState.keycode == Common::KEYCODE_RETURN || keyState.keycode == Common::KEYCODE_KP_ENTER) {
		events._pressed = false;
		events._released = true;
		events._oldButtons = 0;
	} else if (keyState.keycode == Common::KEYCODE_ESCAPE) {
		_exitJournal = true;
	} else if (keyState.keycode == Common::KEYCODE_TAB) {
		Common::Rect r(JOURNAL_BAR_WIDTH, BUTTON_SIZE + screen.fontHeight() + 13);
		r.moveTo((SHERLOCK_SCREEN_WIDTH - r.width()) / 2, SHERLOCK_SCENE_HEIGHT - r.height());

		// See if the mouse is over any of the journal controls
		_selector = -1;
		if (Common::Rect(r.left + 3, r.top + 3, r.right - 3, r.top + screen.fontHeight() + 4).contains(mousePos))
			_selector = (mousePos.x - r.left) / (r.width() / 3);

		// If the mouse is not over any of the options, move the mouse so that it points to the first option
		if (_selector == -1) {
			events.warpMouse(Common::Point(r.left + r.width() / 3 - 10, r.top + screen.fontHeight() + 2));
		} else {
			if (_selector == 2)
				_selector = 0;
			else
				++_selector;

			events.warpMouse(Common::Point(r.left + (r.width() / 3) * (_selector + 1) - 10, mousePos.y));
		}
	}
}

void TattooJournal::handleButtons() {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	uint32 frameCounter = events.getFrameCounter();

	if (_selector != -1 && events._pressed) {
		if (frameCounter >= _scrollingTimer) {
			// Set next scrolling time
			_scrollingTimer = frameCounter + 6;

			// Handle different scrolling actions
			switch (_selector) {
			case 3:
				// Scroll left (1 page back)
				if (_page > 1) {
					// Scroll Up
					drawJournal(1, LINES_PER_PAGE);
					drawScrollBar();
					screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
					_wait = false;
				}
				break;

			case 4:
				// Page left (10 pages back)
				if (_page > 1) {
					// Scroll Up 10 Pages if possible
					if (_page < 11)
						drawJournal(1, (_page - 1) * LINES_PER_PAGE);
					else
						drawJournal(1, 10 * LINES_PER_PAGE);
					drawScrollBar();
					show();
					screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
					_wait = false;
				}
				break;

			case 5:
				// Page right (10 pages ahead)
				if (_down) {
					// Scroll Down 10 Pages
					if (_page + 10 > _maxPage)
						drawJournal(2, (_maxPage - _page) * LINES_PER_PAGE);
					else
						drawJournal(2, 10 * LINES_PER_PAGE);
					drawScrollBar();
					screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
					_wait = false;
				}
				break;

			case 6:
				// Scroll right (1 Page Ahead)
				if (_down) {
					// Scroll Down
					drawJournal(2, LINES_PER_PAGE);
					drawScrollBar();
					screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
					_wait = false;
				}
				break;

			default:
				break;
			}
		}
	}

	if (events._released || events._rightReleased) {
		_scrollingTimer = 0;

		switch (_selector) {
		case 0:
			_exitJournal = true;
			break;

		case 1: {
			// Search Journal
			disableControls();

			bool notFound = false;
			int dir;

			do {
				if ((dir = getFindName(notFound)) != 0) {
					_savedIndex = _index;
					_savedSub = _sub;
					_savedPage = _page;
					
					if (drawJournal(dir + 2, 1000 * LINES_PER_PAGE) == 0)
					{
						_index = _savedIndex;
						_sub = _savedSub;
						_page = _savedPage;
						
						drawFrame();
						drawJournal(0, 0);
						notFound = true;
					} else {
						break;
					}

					highlightJournalControls(false);
					screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
				} else {
					break;
				}
			} while (!_vm->shouldQuit());
			break;
		}

		case 2:
			// Print Journal - not implemented in ScummVM
			break;

		default:
			break;
		}
	}
}

void TattooJournal::loadLocations() {
	Resources &res = *_vm->_res;

	_directory.clear();
	_locations.clear();

	Common::SeekableReadStream *dir = res.load("talk.lib");
	dir->skip(4);		// Skip header

	// Get the numer of entries
	_directory.resize(dir->readUint16LE());
	dir->seek((_directory.size() + 1) * 8, SEEK_CUR);

	// Read in each entry
	char buffer[17];
	for (uint idx = 0; idx < _directory.size(); ++idx) {
		dir->read(buffer, 17);
		buffer[16] = '\0';

		_directory[idx] = Common::String(buffer);
	}

	delete dir;

	// Load in the locations stored in journal.txt
	Common::SeekableReadStream *loc = res.load("journal.txt");

	// Initialize locations
	_locations.resize(100);
	for (int idx = 0; idx < 100; ++idx)
		_locations[idx] = "No Description";

	while (loc->pos() < loc->size()) {
		// In Rose Tattoo, each location line starts with the location
		// number, followed by a dot, some spaces and its description
		// in quotes
		Common::String line = loc->readLine();
		Common::String locNumStr;
		int locNum = 0;
		int i = 0;
		Common::String locDesc;

		// Get the location
		while (Common::isDigit(line[i])) {
			locNumStr += line[i];
			i++;
		}
		locNum = atoi(locNumStr.c_str());

		// Skip the dot, spaces and initial quotation mark
		while (line[i] == ' ' || line[i] == '.' || line[i] == '\"')
			i++;

		do {
			locDesc += line[i];
			i++;
		} while (line[i] != '\"');

		_locations[locNum] = locDesc;
	}

	delete loc;
}

void TattooJournal::drawFrame() {
	Screen &screen = *_vm->_screen;

	screen._backBuffer1.blitFrom((*_journalImages)[0], Common::Point(0, 0));
	drawControls(0);

}

void TattooJournal::synchronize(Serializer &s) {
	// TODO
}

void TattooJournal::drawControls(int mode) {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;

	Common::Rect r(JOURNAL_BAR_WIDTH, !mode ? (BUTTON_SIZE + screen.fontHeight() + 13) :
		(screen.fontHeight() + 4) * 2 + 9);
	r.moveTo((SHERLOCK_SCREEN_WIDTH - r.width()) / 2, !mode ? (SHERLOCK_SCREEN_HEIGHT - r.height()) :
		(SHERLOCK_SCREEN_HEIGHT - r.height()) / 2);

	Common::Rect inner = r;
	inner.grow(-3);

	if (vm._transparentMenus)
		ui.makeBGArea(inner);
	else
		screen._backBuffer1.fillRect(inner, MENU_BACKGROUND);

	// Draw the four corners of the info box
	screen._backBuffer1.transBlitFrom(images[0], Common::Point(r.left, r.top));
	screen._backBuffer1.transBlitFrom(images[1], Common::Point(r.right - images[1]._width, r.top));
	screen._backBuffer1.transBlitFrom(images[1], Common::Point(r.left, r.bottom - images[1]._height));
	screen._backBuffer1.transBlitFrom(images[1], Common::Point(r.right - images[1]._width, r.bottom - images[1]._height));

	// Draw the top of the info box
	screen._backBuffer1.hLine(r.left + images[0]._width, r.top, r.right - images[0]._height, INFO_TOP);
	screen._backBuffer1.hLine(r.left + images[0]._width, r.top + 1, r.right - images[0]._height, INFO_MIDDLE);
	screen._backBuffer1.hLine(r.left + images[0]._width, r.top + 2, r.right - images[0]._height, INFO_BOTTOM);

	// Draw the bottom of the info box
	screen._backBuffer1.hLine(r.left + images[0]._width, r.bottom - 3, r.right - images[0]._height, INFO_TOP);
	screen._backBuffer1.hLine(r.left + images[0]._width, r.bottom - 2, r.right - images[0]._height, INFO_MIDDLE);
	screen._backBuffer1.hLine(r.left + images[0]._width, r.bottom - 1, r.right - images[0]._height, INFO_BOTTOM);

	// Draw the left side of the info box
	screen._backBuffer1.vLine(r.left, r.top + images[0]._height, r.bottom - images[2]._height, INFO_TOP);
	screen._backBuffer1.vLine(r.left + 1, r.top + images[0]._height, r.bottom - images[2]._height, INFO_MIDDLE);
	screen._backBuffer1.vLine(r.left + 2, r.top + images[0]._height, r.bottom - images[2]._height, INFO_BOTTOM);

	// Draw the right side of the info box
	screen._backBuffer1.vLine(r.right - 3, r.top + images[0]._height, r.bottom - images[2]._height, INFO_TOP);
	screen._backBuffer1.vLine(r.right - 2, r.top + images[0]._height, r.bottom - images[2]._height, INFO_MIDDLE);
	screen._backBuffer1.vLine(r.right - 1, r.top + images[0]._height, r.bottom - images[2]._height, INFO_BOTTOM);

	// Draw the sides of the separator bar above the scroll bar
	int yp = r.top + screen.fontHeight() + 7;
	screen._backBuffer1.transBlitFrom(images[4], Common::Point(r.left, yp - 1));
	screen._backBuffer1.transBlitFrom(images[5], Common::Point(r.right - images[5]._width, yp - 1));

	// Draw the bar above the scroll bar
	screen._backBuffer1.hLine(r.left + images[4]._width, yp, r.right - images[5]._width, INFO_TOP);
	screen._backBuffer1.hLine(r.left + images[4]._width, yp + 1, r.right - images[5]._width, INFO_MIDDLE);
	screen._backBuffer1.hLine(r.left + images[4]._width, yp + 2, r.right - images[5]._width, INFO_BOTTOM);

	if (mode != 2) {
		// Draw the Bars separating the Journal Commands
		int xp = r.right / 3;
		for (int idx = 0; idx < 2; ++idx) {
			screen._backBuffer1.transBlitFrom(images[6], Common::Point(xp - 2, r.top + 1));
			screen._backBuffer1.transBlitFrom(images[7], Common::Point(xp - 2, yp - 1));

			screen._backBuffer1.hLine(xp - 1, r.top + 4, yp - 2, INFO_TOP);
			screen._backBuffer1.hLine(xp, r.top + 4, yp - 2, INFO_MIDDLE);
			screen._backBuffer1.hLine(xp + 1, r.top + 4, yp - 2, INFO_BOTTOM);
			xp = r.right / 3 * 2;
		}
	}

	int savedSelector = _oldSelector;
	_oldSelector = 100;

	switch (mode) {
	case 0: 
		highlightJournalControls(false);
		break;
	case 1:
		highlightSearchControls(false);
		break;
	default:
		break;
	}
	
	_oldSelector = savedSelector;
}

void TattooJournal::highlightJournalControls(bool slamIt) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Common::Point mousePos = events.mousePos();
	Common::Rect r(JOURNAL_BAR_WIDTH, BUTTON_SIZE + screen.fontHeight() + 13);
	r.moveTo((SHERLOCK_SCREEN_WIDTH - r.width()) / 2, SHERLOCK_SCREEN_HEIGHT - r.height());
		
	// Calculate the Scroll Position Bar
	int numPages = (_maxPage + LINES_PER_PAGE) / LINES_PER_PAGE;
	int barWidth = (r.width() - BUTTON_SIZE * 2 - 6) / numPages;
	barWidth = CLIP(barWidth, BUTTON_SIZE, r.width() - BUTTON_SIZE * 2 - 6);

	int barX = (numPages <= 1) ? r.left + 3 + BUTTON_SIZE : (r.width() - BUTTON_SIZE * 2 - 6 - barWidth)
		* FIXED_INT_MULTIPLIER / (numPages - 1) * (_page - 1) / FIXED_INT_MULTIPLIER + r.left + 3 + BUTTON_SIZE;

	// See if the mouse is over any of the Journal Controls
	Common::Rect bounds(r.left, r.top, r.right - 3, r.top + screen.fontHeight() + 7);
	_selector = -1;
	if (bounds.contains(mousePos))
		_selector = (mousePos.x - r.left) / (r.width() / 3);
	
	else if (events._pressed) {
		if (Common::Rect(r.left, r.top + screen.fontHeight() + 10, r.left + BUTTON_SIZE, r.top +
				screen.fontHeight() + 10 + BUTTON_SIZE).contains(mousePos))
			// Press on the Scroll Left button
			_selector = 3;
		else if (Common::Rect(r.left + BUTTON_SIZE + 3, r.top + screen.fontHeight() + 10,
				r.left + BUTTON_SIZE + 3 + (barX - r.left - BUTTON_SIZE - 3), r.top + screen.fontHeight() + 
				10 + BUTTON_SIZE).contains(mousePos))
			// Press on the Page Left button
			_selector = 4;
		else if (Common::Rect(barX + barWidth, r.top + screen.fontHeight() + 10, 
				barX + barWidth + (r.right - BUTTON_SIZE - 3 - barX - barWidth),
				r.top + screen.fontHeight() + 10 + BUTTON_SIZE).contains(mousePos))
			// Press on the Page Right button
			_selector = 5;
		else if (Common::Rect(r.right - BUTTON_SIZE - 3, r.top + screen.fontHeight() + 10, r.right - 3, 
				r.top + screen.fontHeight() + 10 + BUTTON_SIZE).contains(mousePos))
			// Press of the Scroll Right button
			_selector = 6;
		}

	// See if the Search was selected, but is not available
	if (_journal.empty() && (_selector == 1 || _selector == 2))
		_selector = -1;

	if (_selector == 4 && _oldSelector == 5)
		_selector = 5;
	else if (_selector == 5 && _oldSelector == 4)
		_selector = 4;

	// See if they're pointing at a different control
	if (_selector != _oldSelector) {
		// Print the Journal commands
		int xp = r.left + r.width() / 6;
		byte color = (_selector == 0) ? COMMAND_HIGHLIGHTED : INFO_TOP;

		screen.gPrint(Common::Point(xp - screen.stringWidth(JOURNAL_COMMANDS[0]) / 2, r.top),
			color, "%s", JOURNAL_COMMANDS[0]);
		xp += r.width() / 3;

		if (!_journal.empty())
			color = (_selector == 1) ? COMMAND_HIGHLIGHTED : INFO_TOP;
		else
			color = INFO_BOTTOM;
		screen.gPrint(Common::Point(xp - screen.stringWidth(JOURNAL_COMMANDS[0]) / 2, r.top + 5),
			color, "%s", JOURNAL_COMMANDS[1]);

		drawScrollBar();

		if (slamIt)
			screen.slamRect(r);

		_oldSelector = _selector;
	}
}

void TattooJournal::highlightSearchControls(bool slamIt) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Common::Point mousePos = events.mousePos();
	Common::Rect r(JOURNAL_BAR_WIDTH, (screen.fontHeight() + 4) * 2 + 9);
	r.moveTo((SHERLOCK_SCREEN_WIDTH - r.width()) / 2, (SHERLOCK_SCREEN_HEIGHT - r.height()) / 2);

	// See if the mouse is over any of the Journal Controls
	_selector = -1;
	if (Common::Rect(r.left + 3, r.top + 3, r.right - 3, r.top + 7 + screen.fontHeight()).contains(mousePos))
		_selector = (mousePos.x - r.left) / (r.width() / 3);

	// See if they're pointing at a different control
	if (_selector != _oldSelector) {
		// Print the search commands
		int xp = r.left + r.width() / 6;

		for (int idx = 0; idx < 3; ++idx) {
			byte color = (_selector == idx) ? COMMAND_HIGHLIGHTED : INFO_TOP;
			screen.gPrint(Common::Point(xp - screen.stringWidth(JOURNAL_SEARCH_COMMANDS[idx]) / 2,
				r.top + 5), color, "%s", JOURNAL_SEARCH_COMMANDS[idx]);
			xp += r.width() / 3;
		}

		if (slamIt)
			screen.slamRect(r);

		_oldSelector = _selector;
	}
}

void TattooJournal::drawScrollBar() {
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	bool raised;
	byte color;

	Common::Rect r(JOURNAL_BAR_WIDTH, BUTTON_SIZE + screen.fontHeight() + 13);
	r.moveTo((SHERLOCK_SCREEN_WIDTH - r.width()) / 2, SHERLOCK_SCREEN_HEIGHT - r.height());

	// Calculate the Scroll Position Bar
	int numPages = (_maxPage + LINES_PER_PAGE) / LINES_PER_PAGE;
	int barWidth = (r.width() - BUTTON_SIZE * 2 - 6) / numPages;
	barWidth = CLIP(barWidth, BUTTON_SIZE, r.width() - BUTTON_SIZE * 2 - 6);
	int barX;
	if (numPages <= 1) {
		barX = r.left + 3 + BUTTON_SIZE;
	} else {
		barX = (r.width() - BUTTON_SIZE * 2 - 6 - barWidth) * FIXED_INT_MULTIPLIER / (numPages - 1) *
			(_page - 1) / FIXED_INT_MULTIPLIER + r.left + 3 + BUTTON_SIZE;
		if (barX + BUTTON_SIZE > r.left + r.width() - BUTTON_SIZE - 3)
			barX = r.right - BUTTON_SIZE * 2 - 3;
	}

	// Draw the scroll bar here
	// Draw the Scroll Left button
	raised = _selector != 3;
	screen._backBuffer1.fillRect(Common::Rect(r.left, r.top + screen.fontHeight() + 12, r.left + BUTTON_SIZE,
		r.top + screen.fontHeight() + BUTTON_SIZE + 9), INFO_MIDDLE);
	ui.drawDialogRect(screen._backBuffer1, Common::Rect(r.left + 3, r.top + screen.fontHeight() + 10, r.left + 3 + BUTTON_SIZE, 
		r.top + screen.fontHeight() + 10 + BUTTON_SIZE), raised);

	color = (_page > 1) ? INFO_BOTTOM + 2 : INFO_BOTTOM;
	screen._backBuffer1.vLine(r.left + 1 + BUTTON_SIZE / 2, r.top + screen.fontHeight() + 10 + BUTTON_SIZE / 2,
		r.top + screen.fontHeight() + 10 + BUTTON_SIZE / 2, color);
	screen._backBuffer1.vLine(r.left + 2 + BUTTON_SIZE / 2, r.top + screen.fontHeight() + 9 + BUTTON_SIZE / 2,
		r.top + screen.fontHeight() + 11 + BUTTON_SIZE / 2, color);
	screen._backBuffer1.vLine(r.left + 3 + BUTTON_SIZE / 2, r.top + screen.fontHeight() + 8 + BUTTON_SIZE / 2,
		r.top + screen.fontHeight() + 12 + BUTTON_SIZE / 2, color);
	screen._backBuffer1.vLine(r.left + 4 + BUTTON_SIZE / 2, r.top + screen.fontHeight() + 7 + BUTTON_SIZE / 2,
		r.top + screen.fontHeight() + 13 + BUTTON_SIZE / 2, color);

	// Draw the Scroll Right button
	raised = _selector != 6;
	screen._backBuffer1.fillRect(Common::Rect(r.right - BUTTON_SIZE - 1, r.top + screen.fontHeight() + 12, 
		r.right - 5, r.top + screen.fontHeight() + BUTTON_SIZE + 9), INFO_MIDDLE);
	ui.drawDialogRect(screen._backBuffer1, Common::Rect(r.right - BUTTON_SIZE - 3, r.top + screen.fontHeight() + 10, r.right - 3,
		r.top + screen.fontHeight() + BUTTON_SIZE + 9), raised);

	color = _down ? INFO_BOTTOM + 2 : INFO_BOTTOM;
	screen._backBuffer1.vLine(r.right - 1 - BUTTON_SIZE + BUTTON_SIZE / 2, r.top + screen.fontHeight() + 10 + BUTTON_SIZE / 2, 
		r.top + screen.fontHeight() + 10 + BUTTON_SIZE / 2, color);
	screen._backBuffer1.vLine(r.right - 2 - BUTTON_SIZE + BUTTON_SIZE / 2, r.top + screen.fontHeight() + 9 + BUTTON_SIZE / 2, 
		r.top + screen.fontHeight() + 11 + BUTTON_SIZE / 2, color);
	screen._backBuffer1.vLine(r.right - 3 - BUTTON_SIZE + BUTTON_SIZE / 2, r.top + screen.fontHeight() + 8 + BUTTON_SIZE / 2,
		r.top + screen.fontHeight() + 12 + BUTTON_SIZE / 2, color);
	screen._backBuffer1.vLine(r.right - 4 - BUTTON_SIZE + BUTTON_SIZE / 2, r.top + screen.fontHeight() + 7 + BUTTON_SIZE / 2,
		r.top + screen.fontHeight() + 13 + BUTTON_SIZE / 2, color);

	// Draw the scroll bar
	screen._backBuffer1.fillRect(Common::Rect(barX + 2, r.top + screen.fontHeight() + 12, barX + barWidth - 3,
		r.top + screen.fontHeight() + BUTTON_SIZE + 9), INFO_MIDDLE);
	ui.drawDialogRect(screen._backBuffer1, Common::Rect(barX, r.top + screen.fontHeight() + 10, barX + barWidth,
		r.top + screen.fontHeight() + 10 + BUTTON_SIZE), true);
}

void TattooJournal::disableControls() {
	Screen &screen = *_vm->_screen;
	Common::Rect r(JOURNAL_BAR_WIDTH, BUTTON_SIZE + screen.fontHeight() + 13);
	r.moveTo((SHERLOCK_SCREEN_HEIGHT - r.width()) / 2, SHERLOCK_SCREEN_HEIGHT - r.height());

	// Print the Journal commands
	int xp = r.left + r.width() / 6;
	for (int idx = 0; idx < 2; ++idx) {
		screen.gPrint(Common::Point(xp - screen.stringWidth(JOURNAL_COMMANDS[idx]) / 2, r.top + 5),
			INFO_BOTTOM, "%s", JOURNAL_COMMANDS[idx]);

		xp += r.width() / 3;
	}

	screen.slamRect(r);
}

int TattooJournal::getFindName(bool printError) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	int result = 0;
	int done = 0;
	Common::String name;
	int cursorX, cursorY;
	bool flag = false;

	Common::Rect r(JOURNAL_BAR_WIDTH, (screen.fontHeight() + 4) * 2 + 9);
	r.moveTo((SHERLOCK_SCREEN_WIDTH - r.width()) / 2, (SHERLOCK_SCREEN_HEIGHT - r.height()) / 2);

	// Set the cursors Y position
	cursorY = r.top + screen.fontHeight() + 12;

	drawControls(1);
	
	// Backup the area under the text entry
	Surface bgSurface(r.width() - 6, screen.fontHeight());
	bgSurface.blitFrom(screen._backBuffer1, Common::Point(0, 0), Common::Rect(r.left + 3, cursorY, 
		r.right - 3, cursorY + screen.fontHeight()));

	if (printError) {
		screen.gPrint(Common::Point(0, cursorY), INFO_TOP, "%s", S_NO_TEXT);
	} else {
		// If there was a name already entered, copy it to name and display it
		if (!_find.empty()) {
			screen.gPrint(Common::Point(r.left + screen.widestChar() + 3, cursorY), COMMAND_HIGHLIGHTED, "%s", _find.c_str());
			name = _find;
		}
	}

	screen.slamRect(r);

	if (printError) {
		// Pause to allow error to be shown
		int timer = 0;

		do {
			events.pollEvents();
			events.setButtonState();

			++timer;
			events.wait(2);
		} while (!_vm->shouldQuit() && !events.kbHit() && !events._released && !events._rightReleased && timer < 40);

		events.clearEvents();

		// Restore the text background
		screen._backBuffer1.blitFrom(bgSurface, Common::Point(r.left, cursorY));

		// If there was a name already entered, copy it to name and display it
		if (!_find.empty()) {
			screen.gPrint(Common::Point(r.left + screen.widestChar() + 3, cursorY), COMMAND_HIGHLIGHTED, "%s", _find.c_str());
			name = _find;
		}

		screen.slamArea(r.left + 3, cursorY, r.width() - 6, screen.fontHeight());
	}

	// Set the cursors X position
	cursorX = r.left + screen.widestChar() + 3 + screen.stringWidth(name);

	do {
		events._released = events._rightReleased = false;

		while (!events.kbHit() && !events._released && !events._rightReleased) {
			if (talk._talkToAbort)
				return 0;

			// See if a key or a mouse button is pressed
			events.pollEventsAndWait();
			events.setButtonState();

			// Handle blinking cursor
			flag = !flag;
			if (flag) {
				// Draw cursor
				screen._backBuffer1.fillRect(Common::Rect(cursorX, cursorY, cursorX + 7, cursorY + 8), COMMAND_HIGHLIGHTED);
				screen.slamArea(cursorX, cursorY, 8, 9);
			} else {
				// Erase cursor by restoring background and writing current text
				screen._backBuffer1.blitFrom(bgSurface, Common::Point(r.left + 3, cursorY));
				screen.gPrint(Common::Point(r.left + screen.widestChar() + 3, cursorY), COMMAND_HIGHLIGHTED, "%s", name.c_str());
				screen.slamArea(r.left + 3, r.top, r.width() - 3, screen.fontHeight());
			}

			highlightSearchControls(true);

			events.wait(2);
		}

		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();
			Common::Point mousePos = events.mousePos();

			if (keyState.keycode == Common::KEYCODE_BACKSPACE && !name.empty()) {
				cursorX -= screen.charWidth(name.lastChar());
				name.deleteLastChar();
			}

			if (keyState.keycode == Common::KEYCODE_RETURN)
				done = 1;

			else if (keyState.keycode == Common::KEYCODE_ESCAPE)
				done = -1;

			if (keyState.keycode == Common::KEYCODE_TAB) {
				r = Common::Rect(JOURNAL_BAR_WIDTH, BUTTON_SIZE + screen.fontHeight() + 13);
				r.moveTo((SHERLOCK_SCREEN_WIDTH - r.width()) / 2, (SHERLOCK_SCREEN_HEIGHT - r.height()) / 2);

				// See if the mouse is over any of the journal controls
				_selector = -1;
				if (Common::Rect(r.left + 3, r.top + 3, r.right - 3, r.top + screen.fontHeight() + 4).contains(mousePos))
					_selector = (mousePos.x - r.left) / (r.width() / 3);

				// If the mouse is not over any of the options, move the mouse so that it points to the first option
				if (_selector == -1) {
					events.warpMouse(Common::Point(r.left + r.width() / 3, r.top + screen.fontHeight() + 2));
				} else {
					if (keyState.keycode & Common::KBD_SHIFT) {
						if (_selector == 0)
							_selector = 2;
						else
							--_selector;
					} else {
						if (_selector == 2)
							_selector = 0;
						else
							++_selector;
					}

					events.warpMouse(Common::Point(r.left + (r.width() / 3) * (_selector + 1) - 10, mousePos.y));
				}
			}

			if (keyState.ascii && keyState.ascii != '@' && name.size() < 50) {
				if ((cursorX + screen.charWidth(keyState.ascii)) < (r.right - screen.widestChar() * 3)) {
					cursorX += screen.charWidth(keyState.ascii);
					name += toupper(keyState.ascii);
				}
			}

			// Redraw the text
			screen._backBuffer1.blitFrom(bgSurface, Common::Point(r.left + 3, cursorY));
			screen.gPrint(Common::Point(r.left + screen.widestChar() + 3, cursorY), COMMAND_HIGHLIGHTED,
				"%s", name.c_str());
			screen.slamArea(r.left + 3, cursorY, r.right - 3, screen.fontHeight());
		}

		if (events._released || events._rightReleased) {
			switch (_selector)
			{
			case 0:
				done = -1;
				break;
			case 1:
				done = 2;
				break;
			case 2:
				done = 1;
				break;
			default:
				break;
			}
		}
	} while (!done);

	if (done != -1) {
		_find = name;
		result = done;
	} else {
		result = 0;
	}

	drawFrame();
	drawJournal(0, 0);
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	return result;
}

} // End of namespace Tattoo

} // End of namespace Sherlock
