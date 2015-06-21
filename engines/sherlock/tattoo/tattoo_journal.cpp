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

TattooJournal::TattooJournal(SherlockEngine *vm) : Journal(vm) {
	_journalImages = nullptr;

	loadJournalLocations();
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

void TattooJournal::loadJournalLocations() {
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

void TattooJournal::drawJournalFrame() {
	Screen &screen = *_vm->_screen;

	screen._backBuffer1.blitFrom((*_journalImages)[0], Common::Point(0, 0));
	drawJournalControls(0);

}

void TattooJournal::synchronize(Serializer &s) {
	// TODO
}

void TattooJournal::drawJournalControls(int mode) {
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
	// TODO
}

void TattooJournal::highlightSearchControls(bool slamIt) {
	// TODO
}

} // End of namespace Tattoo

} // End of namespace Sherlock
