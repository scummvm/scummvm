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

// Tab Size = 4

#ifndef ULTIMA8_AUDIO_MIDI_XMIDIEVENTLIST_H
#define ULTIMA8_AUDIO_MIDI_XMIDIEVENTLIST_H

#include "ultima/ultima8/audio/midi/xmidi_event.h"

namespace Ultima8 {

class ODataSource;

class XMidiEventList {
	int             counter;

	// Helper funcs for Write
	int             putVLQ(ODataSource *dest, uint32 value);
	uint32          convertListToMTrk(ODataSource *dest);

	static void     deleteEventList(XMidiEvent *list);

public:
	uint16          chan_mask;
	XMidiEvent      *events;

	//! XMidiFile Seq Branch Index controllers. For Ult126 = loop start
	XMidiEvent      *branches;

	//! Write the list to a DataSource
	int             write(ODataSource *dest);

	//! Increments the counter
	void            incerementCounter() {
		counter++;
	}

	//! Decrement the counter and delete the event list, if possible
	void            decerementCounter();

	//! Find the Sequence Branch Event for the index
	//! \param index The index to search for
	//! \return The event found, or 0
	XMidiEvent     *findBranchEvent(int index) {

		XMidiEvent *b = branches;

		while (b) {
			if (b->data[1] == (index & 0xFF)) break;
			b = b->ex.branch_index.next_branch;
		}

		return b;
	}
};

} // End of namespace Ultima8

#endif
