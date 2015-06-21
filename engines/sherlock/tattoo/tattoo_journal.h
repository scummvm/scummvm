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

#ifndef SHERLOCK_TATTOO_JOURNAL_H
#define SHERLOCK_TATTOO_JOURNAL_H

#include "sherlock/journal.h"
#include "sherlock/image_file.h"

namespace Sherlock {

namespace Tattoo {

class TattooJournal : public Journal {
private:
	ImageFile *_journalImages;

	/**
	 * Load the list of journal locations
	 */
	void loadJournalLocations();

	/**
	 * Displays the controls used by the journal
	 * @param mode	0: Normal journal buttons, 1: Search interface
	 */
	void drawJournalControls(int mode);
public:
	TattooJournal(SherlockEngine *vm);
	virtual ~TattooJournal() {}

	/**
	 * Show the journal
	 */
	void show();
public:
	/**
	 * Draw the journal background, frame, and interface buttons
	 */
	virtual void drawJournalFrame();

	/**
	 * Synchronize the data for a savegame
	 */
	virtual void synchronize(Serializer &s);
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
