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
	uint32 time_left;
};

class MidiParser_XMIDI : public MidiParser {
protected:
	byte *_data;
	uint16 _num_tracks;
	byte *_tracks [16];

	byte _active_track;
	byte *_play_pos;
	uint32 _play_time;
	uint32 _last_event_time;

	NoteTimer _notes_cache[32];

protected:
	uint32 read4high (byte * &data) {
		uint32 val = 0;
		int i;
		for (i = 0; i < 4; ++i) val = (val << 8) | *data++;
		return val;
	}
	uint16 read2low  (byte * &data) {
		uint16 val = 0;
		int i;
		for (i = 0; i < 2; ++i) val |= (*data++) << (i * 8);
		return val;
	}
	uint32 readVLQ (byte * &data);
	uint32 readVLQ2 (byte * &data);

	void playToTime (uint32 psec, bool transmit);

public:
	bool loadMusic (byte *data, uint32 size);
	void unloadMusic();

	void setMidiDriver (MidiDriver *driver) { _driver = driver; }
	void setTimerRate (uint32 rate) { _timer_rate = rate; }
	void onTimer();

	void setTrack (byte track);
	void jumpToTick (uint32 tick);
};



// This delta time is based on an assumed tempo of
// 120 quarter notes per minute (500,000 microseconds per quarter node)
// and 60 ticks per quarter note, i.e. 120 Hz overall.
// 500,000 / 60 = 8333.33 microseconds per tick.
#define MICROSECONDS_PER_TICK 8333



//////////////////////////////////////////////////
//
// MidiParser_XMIDI implementation
//
// Much of this code is adapted from the XMIDI
// implementation from the exult project.
//
//////////////////////////////////////////////////

// This is the conventional (i.e. SMF) variable length quantity
uint32 MidiParser_XMIDI::readVLQ (byte * &data) {
	byte str;
	uint32 value = 0;
	int i;

	for (i = 0; i < 4; ++i) {
		str = data[0];
		++data;
		value = (value << 7) | (str & 0x7F);
		if (!(str & 0x80))
			break;
	}
	return value;
}

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

void MidiParser_XMIDI::onTimer() {
	if (!_play_pos || !_driver)
		return;
	playToTime (_play_time + _timer_rate, true);
}

void MidiParser_XMIDI::playToTime (uint32 psec, bool transmit) {
	uint32 delta;
	uint32 end_time;
	uint32 event_time;
	byte *pos;
	byte *oldpos;
	byte event;
	uint32 length;
	int i;
	NoteTimer *ptr;
	byte note;
	byte vel;
	uint32 note_length;
	
	end_time = psec;
	pos = _play_pos;

	// Send any necessary note off events.
	ptr = &_notes_cache[0];
	for (i = ARRAYSIZE(_notes_cache); i; --i, ++ptr) {
		if (ptr->time_left) {
			if (ptr->time_left <= _timer_rate) {
				if (transmit)
					_driver->send (0x80 | ptr->channel | (ptr->note << 8));
				ptr->time_left = 0;
			} else {
				ptr->time_left -= _timer_rate;
			}
		}
	}

	while (true) {
		oldpos = pos;
		delta = readVLQ2 (pos);
		event_time = _last_event_time + delta * MICROSECONDS_PER_TICK;
		if (event_time > end_time) {
			pos = oldpos;
			break;
		}

		// Process the next event.
		event = *pos++;
		switch (event >> 4) {
		case 0x9: // Note On
			note = pos[0];
			vel = pos[1];
			pos += 2;
			note_length = readVLQ (pos) * MICROSECONDS_PER_TICK;

			ptr = &_notes_cache[0];
			for (i = ARRAYSIZE(_notes_cache); i; --i, ++ptr) {
				if (!ptr->time_left) {
					ptr->time_left = note_length;
					ptr->channel = event & 0x0F;
					ptr->note = note;
					if (transmit)
						_driver->send (event | (note << 8) | (vel << 16));
					break;
				}
			}
			break;

		case 0xC: // Program Change
		case 0xD: // Channel Aftertouch
			if (transmit)
				_driver->send (event | (pos[0] << 8));
			++pos;
			break;

		case 0x8: // Note Off (do these ever occur in XMIDI??)
		case 0xA: // Key Aftertouch
		case 0xB: // Control Change
		case 0xE: // Pitch Bender Change
			if (transmit)
				_driver->send (event | (pos[0] << 8) | (pos[1] << 16));
			pos += 2;
			break;

		case 0xF: // Meta or SysEx event
			switch (event & 0x0F) {
			case 0x2: // Song Position Pointer
				if (transmit)
					_driver->send (event | (pos[0] << 8) | (pos[1] << 16));
				pos += 2;
				break;

			case 0x3: // Song Select
				if (transmit)
					_driver->send (event | (pos[0] << 8));
				++pos;
				break;

			case 0x6: // Tune Request
			case 0x8: // MIDI Timing Clock
			case 0xA: // Sequencer Start
			case 0xB: // Sequencer Continue
			case 0xC: // Sequencer Stop
			case 0xE: // Active Sensing
				if (transmit)
					_driver->send (event);
				break;

			case 0x0: // SysEx
				length = readVLQ (pos);
				if (transmit)
					_driver->sysEx (pos, (uint16)(length - 1));
				pos += length;
				break;

			case 0xF: // META event
				event = *pos++;
				length = readVLQ (pos);

				if (event == 0x2F) {
					// End of song must be processed by us,
					// as well as sending it to the output device.
					ptr = &_notes_cache[0];
					for (i = ARRAYSIZE(_notes_cache); i; --i, ++ptr) {
						if (ptr->time_left) {
							if (transmit)
								_driver->send (0x80 | ptr->channel | (ptr->note << 8));
							ptr->time_left = 0;
						}
					}
					_play_pos = 0;
					if (transmit)
						_driver->metaEvent (event, pos, (uint16) length);
					return;
				}

				if (transmit)
					_driver->metaEvent (event, pos, (uint16) length);
				pos += length;
				break;
			}
		}

		_last_event_time = event_time;
	}

	_play_time = end_time;
	_play_pos = pos;
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
				
				_num_tracks = read2low (pos);

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
		_active_track = 255;
		setTrack (0);
		return true;
	}

	return false;
}

void MidiParser_XMIDI::unloadMusic() {
	int i;
	NoteTimer *ptr;

	_play_pos = NULL;
	_data = NULL;
	_num_tracks = 0;
	_active_track = 0;
	_play_time = 0;
	_last_event_time = 0;

	// Send any necessary note off events.
	ptr = &_notes_cache[0];
	for (i = ARRAYSIZE(_notes_cache); i; --i, ++ptr) {
		if (ptr->time_left) {
			_driver->send ((0x80 | ptr->channel) | (ptr->note << 8));
			ptr->time_left = 0;
		}
	}
}

void MidiParser_XMIDI::setTrack (byte track) {
	if (track >= _num_tracks || track == _active_track)
		return;
	_active_track = track;
	_play_time = 0;
	_last_event_time = 0;
	_play_pos = _tracks[track];
}

void MidiParser_XMIDI::jumpToTick (uint32 tick) {
	if (_active_track >= _num_tracks)
		return;
	_play_pos = _tracks[_active_track];
	_play_time = 0;
	_last_event_time = 0;
	if (tick > 0) {
		printf ("jumpToTick (%ld) not completely implemented!\n", tick);
		playToTime (tick * MICROSECONDS_PER_TICK - 1, false);
	}
}

MidiParser *MidiParser::createParser_XMIDI() { return new MidiParser_XMIDI; }
