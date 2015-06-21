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

static const char *const JOURNAL_COMMANDS[2] = { "Close Journal", "Search Journal" };

static const char *const JOURNAL_SEARCH_COMMANDS[3] = { "Abort Search", "Search Backwards", "Search Forwards" };

TattooJournal::TattooJournal(SherlockEngine *vm) : Journal(vm) {
	_journalImages = nullptr;

	loadLocations();
}

void TattooJournal::show() {
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

	// TODO

	// Free the images
	delete _journalImages;
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
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();
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

} // End of namespace Tattoo

} // End of namespace Sherlock
