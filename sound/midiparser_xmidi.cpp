/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "midiparser.h"
#include "mididrv.h"
#include "common/util.h"

#include <stdio.h>
#include <memory.h>

//////////////////////////////////////////////////
//
// The XMIDI version of MidiParser
//
//////////////////////////////////////////////////

struct NoteTimer {
	byte channel;
	byte note;
	uint32 off_time;
};

class MidiParser_XMIDI : public MidiParser {
protected:
	byte *_data;
	NoteTimer _notes_cache[32];
	uint32 _inserted_delta; // Track simulated deltas for note-off events

protected:
	uint32 readVLQ2 (byte * &data);
	void parseNextEvent (EventInfo &info);

public:
	~MidiParser_XMIDI() { }

	bool loadMusic (byte *data, uint32 size);
	void unloadMusic();
};



//////////////////////////////////////////////////
//
// MidiParser_XMIDI implementation
//
// Much of this code is adapted from the XMIDI
// implementation from the exult project.
//
//////////////////////////////////////////////////

// This is a special XMIDI variable length quantity
uint32 MidiParser_XMIDI::readVLQ2 (byte * &pos) {
	uint32 value = 0;
	int i;
	
	for (i = 0; i < 4; ++i) {
		if (pos[0] & 0x80)
			break;
		value += *pos++;
	}
	return value;
}

void MidiParser_XMIDI::parseNextEvent (EventInfo &info) {
	info.start = _play_pos;
	info.delta = readVLQ2 (_play_pos) - _inserted_delta;

	// Scan our active notes for the note
	// with the nearest off time. It might turn out
	// to be closer than the next regular event.
	uint32 note_length;
	NoteTimer *best = 0;
	NoteTimer *ptr = &_notes_cache[0];
	int i;
	for (i = ARRAYSIZE(_notes_cache); i; --i, ++ptr) {
		if (ptr->off_time && ptr->off_time >= _last_event_tick && (!best || ptr->off_time < best->off_time))
			best = ptr;
	}

	// See if we need to simulate a note off event.
	if (best && (best->off_time - _last_event_tick) <= info.delta) {
		_play_pos = info.start;
		info.delta = best->off_time - _last_event_tick;
		info.event = 0x80 | best->channel;
		info.param1 = best->note;
		info.param2 = 0;
		best->off_time = 0;
		_inserted_delta += info.delta;
		return;
	}

	// Process the next event.
	_inserted_delta = 0;
	info.event = *(_play_pos++);
	switch (info.event >> 4) {
	case 0x9: // Note On
		info.param1 = *(_play_pos++);
		info.param2 = *(_play_pos++);
		note_length = readVLQ (_play_pos);

		// In addition to sending this back, we must
		// store a note timer so we know when to turn it off.
		ptr = &_notes_cache[0];
		for (i = ARRAYSIZE(_notes_cache); i; --i, ++ptr) {
			if (!ptr->off_time)
				break;
		}

		if (i) {
			ptr->channel = info.channel();
			ptr->note = info.param1;
			ptr->off_time = _last_event_tick + info.delta + note_length;
		}
		break;

	case 0xC: case 0xD:
		info.param1 = *(_play_pos++);
		info.param2 = 0;
		break;

	case 0x8: case 0xA: case 0xB: case 0xE:
		info.param1 = *(_play_pos++);
		info.param2 = *(_play_pos++);
		break;

	case 0xF: // Meta or SysEx event
		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer
			info.param1 = *(_play_pos++);
			info.param2 = *(_play_pos++);
			break;

		case 0x3: // Song Select
			info.param1 = *(_play_pos++);
			info.param2 = 0;
			break;

		case 0x6: case 0x8: case 0xA: case 0xB: case 0xC: case 0xE:
			info.param1 = info.param2 = 0;
			break;

		case 0x0: // SysEx
			info.length = readVLQ (_play_pos);
			info.data = _play_pos;
			_play_pos += info.length;
			break;

		case 0xF: // META event
			info.type = *(_play_pos++);
			info.length = readVLQ (_play_pos);
			info.data = _play_pos;
			_play_pos += info.length;
			if (info.type == 0x51 && info.length == 3) {
				// Tempo event. We want to make these constant 500,000.
				info.data[0] = 0x07;
				info.data[1] = 0xA1;
				info.data[2] = 0x20;
			}
			break;
		}
	}
}

bool MidiParser_XMIDI::loadMusic (byte *data, uint32 size) {
	uint32 i = 0;
	byte *start;
	uint32 len;
	uint32 chunk_len;
	char buf[32];

	unloadMusic();
	byte *pos = data;

	if (!memcmp (pos, "FORM", 4)) {
		pos += 4;

		// Read length of 
		len = read4high (pos);
		start = pos;
		
		// XDIRless XMIDI, we can handle them here.
		if (!memcmp (pos, "XMID", 4)) {	
			printf ("Warning: XMIDI doesn't have XDIR\n");
			pos += 4;
			_num_tracks = 1;
		} else if (memcmp (pos, "XDIR", 4)) {
			// Not an XMIDI that we recognise
			printf ("Expected 'XDIR' but found '%c%c%c%c'\n", pos[0], pos[1], pos[2], pos[3]);
			return false;
		} else {
			// Seems Valid
			pos += 4;
			_num_tracks = 0;

			for (i = 4; i < len; i++) {
				// Read 4 bytes of type
				memcpy (buf, pos, 4);
				pos += 4;

				// Read length of chunk
				chunk_len = read4high (pos);
			
				// Add eight bytes
				i += 8;
				
				if (memcmp (buf, "INFO", 4)) {	
					// Must align
					pos += (chunk_len + 1) & ~1;
					i += (chunk_len + 1) & ~1;
					continue;
				}

				// Must be at least 2 bytes long
				if (chunk_len < 2) {
					printf ("Invalid chunk length %d for 'INFO' block!\n", (int) chunk_len);
					return false;
				}
				
				_num_tracks = (byte) read2low (pos);

				if (chunk_len > 2) {
					printf ("Chunk length %d is greater than 2\n", (int) chunk_len);
					pos += chunk_len - 2;
				}
				break;
			}

			// Didn't get to fill the header
			if (_num_tracks == 0) {
				printf ("Didn't find a valid track count\n");
				return false;
			}
		
			// Ok now to start part 2
			// Goto the right place
			pos = start + ((len + 1) & ~1);
		
			if (memcmp (pos, "CAT ", 4)) {
				// Not an XMID
				printf ("Expected 'CAT ' but found '%c%c%c%c'\n", pos[0], pos[1], pos[2], pos[3]);
				return false;
			}
			pos += 4;
			
			// Now read length of this track
			len = read4high (pos);
			
			if (memcmp (pos, "XMID", 4)) {
				// Not an XMID
				printf ("Expected 'XMID' but found '%c%c%c%c'\n", pos[0], pos[1], pos[2], pos[3]);
				return false;
			}
			pos += 4;

		}

		// Ok it's an XMIDI.
		// We're going to identify and store the location for each track.
		if (_num_tracks > 16) {
			printf ("Can only handle 16 tracks but was handed %d\n", (int) _num_tracks);
			return false;
		}

		int tracks_read = 0;
		while (tracks_read < _num_tracks) {
			if (!memcmp (pos, "FORM", 4)) {
				// Skip this plus the 4 bytes after it.
				pos += 8;
			} else if (!memcmp (pos, "XMID", 4)) {
				// Skip this.
				pos += 4;
			} else if (!memcmp (pos, "TIMB", 4)) {
				// Custom timbres?
				// We don't support them.
				// Read the length, skip it, and hope there was nothing there.
				pos += 4;
				len = read4high (pos);
				pos += (len + 1) & ~1;
			} else if (!memcmp (pos, "EVNT", 4)) {
				// Ahh! What we're looking for at last.
				_tracks[tracks_read] = pos + 8; // Skip the EVNT and length bytes
				pos += 4;
				len = read4high (pos);
				pos += (len + 1) & ~1;
				++tracks_read;
			} else {
				printf ("Hit invalid block '%c%c%c%c' while scanning for track locations\n", pos[0], pos[1], pos[2], pos[3]);
				return false;
			}
		}

		// If we got this far, we successfully established
		// the locations for each of our tracks.
		// Note that we assume the original data passed in
		// will persist beyond this call, i.e. we do NOT
		// copy the data to our own buffer. Take warning....
		_data = data;
		_ppqn = 60;
		resetTracking();
		_inserted_delta = 0;
		setTrack (0);
		return true;
	}

	return false;
}

void MidiParser_XMIDI::unloadMusic() {
	resetTracking();
	allNotesOff();
	_inserted_delta = 0;
	_data = 0;
	_num_tracks = 0;
	_active_track = 255;
}

MidiParser *MidiParser::createParser_XMIDI() { return new MidiParser_XMIDI; }
