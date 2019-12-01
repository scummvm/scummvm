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

#ifndef ULTIMA8_AUDIO_MIDI_XMIDINOTESTACK_H
#define ULTIMA8_AUDIO_MIDI_XMIDINOTESTACK_H

#include "XMidiEvent.h"

namespace Ultima8 {

class XMidiNoteStack {
	XMidiEvent      *notes;     // Top of the stack
	int             polyphony;
	int             max_polyphony;
public:

	XMidiNoteStack() : notes(0), polyphony(0), max_polyphony(0) { }

	// Just clear it. Don't care about what's actually in it
	inline void clear() {
		notes = 0;
		polyphony = 0;
		max_polyphony = 0;
	}

	// Pops the top of the stack if its off_time is <= time (6000th of second)
	inline XMidiEvent *PopTime(uint32 time) {
		if (notes && notes->ex.note_on.note_time <= time)  {
			XMidiEvent *note = notes;
			notes = note->ex.note_on.next_note;
			note->ex.note_on.next_note = 0;
			polyphony--;
			return note;
		}

		return 0;
	}

	// Pops the top of the stack
	inline XMidiEvent *Pop() {
		if (notes)  {
			XMidiEvent *note = notes;
			notes = note->ex.note_on.next_note;
			note->ex.note_on.next_note = 0;
			polyphony--;
			return note;
		}

		return 0;
	}

	// Pops the top of the stack
	inline XMidiEvent *Remove(XMidiEvent *event) {
		XMidiEvent *prev = 0;
		XMidiEvent *note = notes;
		while (note) {

			if (note == event) {
				if (prev) prev->ex.note_on.next_note = note->ex.note_on.next_note;
				else notes = note->ex.note_on.next_note;
				note->ex.note_on.next_note = 0;
				polyphony--;
				return note;
			}
			prev = note;
			note = note->ex.note_on.next_note;
		}
		return 0;
	}

	// Finds the note that has same pitch and channel, and pops it
	inline XMidiEvent *FindAndPop(XMidiEvent *event) {

		XMidiEvent *prev = 0;
		XMidiEvent *note = notes;
		while (note) {

			if ((note->status & 0xF) == (event->status & 0xF) && note->data[0] == event->data[0]) {
				if (prev) prev->ex.note_on.next_note = note->ex.note_on.next_note;
				else notes = note->ex.note_on.next_note;
				note->ex.note_on.next_note = 0;
				polyphony--;
				return note;
			}
			prev = note;
			note = note->ex.note_on.next_note;
		}
		return 0;
	}

	// Pushes a note onto the top of the stack
	inline void Push(XMidiEvent *event) {
		event->ex.note_on.next_note = notes;
		notes = event;
		polyphony++;
		if (max_polyphony < polyphony) max_polyphony = polyphony;
	}

	inline void Push(XMidiEvent *event, uint32 time) {
		event->ex.note_on.note_time = time;
		event->ex.note_on.next_note = 0;

		polyphony++;
		if (max_polyphony < polyphony) max_polyphony = polyphony;

		if (!notes || time <= notes->ex.note_on.note_time) {
			event->ex.note_on.next_note = notes;
			notes = event;
		} else {
			XMidiEvent *prev = notes;
			while (prev) {
				XMidiEvent *note = prev->ex.note_on.next_note;

				if (!note || time <= note->ex.note_on.note_time) {
					event->ex.note_on.next_note = note;
					prev->ex.note_on.next_note = event;
					return;
				}
				prev = note;
			}
		}
	}

	// Finds the note that has same pitch and channel, and sets its after touch to our velocity
	inline void SetAftertouch(XMidiEvent *event) {

		XMidiEvent *prev = 0;
		XMidiEvent *note = notes;
		while (note) {

			if ((note->status & 0xF) == (event->status & 0xF) && note->data[0] == event->data[0]) {
				note->ex.note_on.actualvel = event->data[1];
				return;
			}
			prev = note;
			note = note->ex.note_on.next_note;
		}
	}

	inline XMidiEvent *GetNotes() {
		return notes;
	}

	inline int GetPolyphony() {
		return polyphony;
	}

	inline int GetMaxPolyphony() {
		return max_polyphony;
	}
};

} // End of namespace Ultima8

#endif
