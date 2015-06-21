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

TattooJournal::TattooJournal(SherlockEngine *vm) : Journal(vm) {
	_journalImages = nullptr;
}

void TattooJournal::show() {
	Resources &res = *_vm->_res;
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	// Load journal images
	_journalImages = new ImageFile("journal.vgs");

	// Load palette
	Common::SeekableReadStream *stream = res.load("journal.pal");
	stream->read(screen._tMap, PALETTE_SIZE);
	screen.translatePalette(screen._tMap);
	ui.setupBGArea(screen._tMap);


	delete _journalImages;
}

void TattooJournal::synchronize(Serializer &s) {
	// TODO
}

} // End of namespace Tattoo

} // End of namespace Sherlock
