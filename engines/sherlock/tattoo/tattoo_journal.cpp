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
#include "sherlock/sherlock.h"

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
	// TODO
}

} // End of namespace Tattoo

} // End of namespace Sherlock
