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
// The Standard MIDI File version of MidiParser
//
//////////////////////////////////////////////////

class MidiParser_SMF : public MidiParser {
protected:
	byte *_data;
	byte *_buffer;
	uint16 _num_tracks;
	byte *_tracks [16];

	bool _malformedPitchBends;
	byte _active_track;
	byte *_play_pos;
	uint32 _play_time;
	uint32 _last_event_time;
	byte _running_status; // Cached MIDI command

	uint32 _ppqn;
	uint32 _psec_per_tick; // Microseconds per delta tick

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

	void compressToType0();
	void playToTime (uint32 psec, bool transmit);
	void allNotesOff();

public:
	~MidiParser_SMF();

	bool loadMusic (byte *data, uint32 size);
	void unloadMusic();

	void property (int property, int value);
	void setMidiDriver (MidiDriver *driver) { _driver = driver; }
	void setTimerRate (uint32 rate) { _timer_rate = rate; }
	void onTimer();

	void setTrack (byte track);
	void jumpToTick (uint32 tick);
};



//////////////////////////////////////////////////
//
// MidiParser_SMF implementation
//
// Much of this code is adapted from the XMIDI
// implementation from the exult project.
//
//////////////////////////////////////////////////

static byte command_lengths[8] = { 3, 3, 3, 3, 2, 2, 3, 0 };
static byte special_lengths[16] = { 0, 1, 1, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 };

MidiParser_SMF::~MidiParser_SMF() {
	if (_buffer)
		free (_buffer);
}

void MidiParser_SMF::property (int property, int value) {
	switch (property) {
	case mpMalformedPitchBends:
		_malformedPitchBends = (value > 0);
	}
}

// This is the conventional (i.e. SMF) variable length quantity
uint32 MidiParser_SMF::readVLQ (byte * &data) {
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

void MidiParser_SMF::onTimer() {
	if (!_play_pos || !_driver)
		return;
	playToTime (_play_time + _timer_rate, true);
}

void MidiParser_SMF::playToTime (uint32 psec, bool transmit) {
	uint32 delta;
	uint32 end_time;
	uint32 event_time;
	byte *pos;
	byte *oldpos;
	byte event;
	uint32 length;

	end_time = psec;
	pos = _play_pos;

	while (true) {
		oldpos = pos;
		delta = readVLQ (pos);
		event_time = _last_event_time + delta * _psec_per_tick;
		if (event_time > end_time) {
			pos = oldpos;
			break;
		}

		// Process the next event.
		do {
			if ((pos[0] & 0xF0) >= 0x80)
				event = *pos++;
			else
				event = _running_status;
		} while (_malformedPitchBends && (event & 0xF0) == 0xE0 && pos++);

		if (event < 0x80) {
			printf ("ERROR! Bad command or running status %02X", event);
			_play_pos = 0;
			return;
		}

		_running_status = event;
		switch (event >> 4) {
		case 0xC: // Program Change
		case 0xD: // Channel Aftertouch
			if (transmit)
				_driver->send (event | (pos[0] << 8));
			++pos;
			break;

		case 0x9: // Note On
		case 0x8: // Note Off
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
					// End of Track must be processed by us,
					// as well as sending it to the output device.
					_play_pos = 0;
					if (transmit) {
						_driver->metaEvent (event, pos, (uint16) length);
					}
					return;
				} else if (event == 0x51) {
					if (length >= 3) {
						delta = pos[0] << 16 | pos[1] << 8 | pos[2];
						_psec_per_tick = (delta + (_ppqn >> 2)) / _ppqn;
					}
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

bool MidiParser_SMF::loadMusic (byte *data, uint32 size) {
	uint32 len;
	byte midi_type;
	uint32 total_size;
	bool isGMF;

	unloadMusic();
	byte *pos = data;
	isGMF = false;

	if (!memcmp (pos, "RIFF", 4)) {
		// Skip the outer RIFF header.
		pos += 8;
	}

	if (!memcmp (pos, "MThd", 4)) {
		// SMF with MTHd information.
		pos += 4;
		len = read4high (pos);
		if (len != 6) {
			printf ("Warning: MThd length 6 expected but found %d\n", (int) len);
			return false;
		}

		// Verify that this MIDI either is a Type 2
		// or has only 1 track. We do not support
		// multitrack Type 1 files.
		_num_tracks = pos[2] << 8 | pos[3];
		midi_type = pos[1];
		if (midi_type > 2 /*|| (midi_type < 2 && _num_tracks > 1)*/) {
			printf ("Warning: No support for a Type %d MIDI with %d tracks\n", (int) midi_type, (int) _num_tracks);
			return false;
		}
		_ppqn = pos[4] << 8 | pos[5];
		pos += len;
	} else if (!memcmp (pos, "GMF\x1", 4)) {
		// Older GMD/MUS file with no header info.
		// Assume 1 track, 192 PPQN, and no MTrk headers.
		isGMF = true;
		midi_type = 0;
		_num_tracks = 1;
		_ppqn = 192;
		pos += 7; // 'GMD\x1' + 3 bytes of useless (translate: unknown) information
	} else {
		printf ("Expected MThd or GMD header but found '%c%c%c%c' instead.\n", pos[0], pos[1], pos[2], pos[3]);
		return false;
	}

	// Now we identify and store the location for each track.
	if (_num_tracks > 16) {
		printf ("Can only handle 16 tracks but was handed %d\n", (int) _num_tracks);
		return false;
	}

	total_size = 0;
	int tracks_read = 0;
	while (tracks_read < _num_tracks) {
		if (memcmp (pos, "MTrk", 4) && !isGMF) {
			printf ("Position: %p ('%c')\n", pos, *pos);
			printf ("Hit invalid block '%c%c%c%c' while scanning for track locations\n", pos[0], pos[1], pos[2], pos[3]);
			return false;
		}

		// If needed, skip the MTrk and length bytes
		_tracks[tracks_read] = pos + (isGMF ? 0 : 8);
		if (!isGMF) {
			pos += 4;
			len = read4high (pos);
			total_size += len;
			pos += len;
		} else {
			// An SMF End of Track meta event must be placed
			// at the end of the stream.
			data[size++] = 0xFF;
			data[size++] = 0x2F;
			data[size++] = 0x00;
			data[size++] = 0x00;
		}
		++tracks_read;
	}

	// If this is a Type 1 MIDI, we need to now compress
	// our tracks down into a single Type 0 track.
	if (_buffer) {
		free (_buffer);
	}

	if (midi_type == 1) {
		_buffer = (byte *) calloc (size, 1);
		compressToType0();
		_data = _buffer;
		_num_tracks = 1;
		_tracks[0] = _buffer;
	} else {
		_data = data;
	}

	// Note that we assume the original data passed in
	// will persist beyond this call, i.e. we do NOT
	// copy the data to our own buffer. Take warning....
	_active_track = 255;
	_psec_per_tick = (500000 + (_ppqn >> 2)) / _ppqn; // Default to 120 BPM
	setTrack (0);
	return true;
}

void MidiParser_SMF::compressToType0() {
	// We assume that _buffer has been allocated
	// to sufficient size for this operation.
	byte *track_pos[16];
	byte running_status[16];
	uint32 track_timer[16];
	uint32 delta;
	int i;

	for (i = 0; i < _num_tracks; ++i) {
		running_status[i] = 0;
		track_pos[i] = _tracks[i];
		track_timer[i] = readVLQ (track_pos[i]);
	}

	int best_i;
	uint32 length;
	byte *output = _buffer;
	byte *pos, *pos2;
	byte event;
	uint32 copy_bytes;
	bool write;
	byte active_tracks = (byte) _num_tracks;

	while (active_tracks) {
		write = true;
		best_i = 255;
		for (i = 0; i < _num_tracks; ++i) {
			if (track_pos[i] && (best_i == 255 || track_timer[i] < track_timer[best_i]))
				best_i = i;
		}
		if (best_i == 255) {
			printf ("Premature end of tracks!\n");
			break;
		}

		// Initial VLQ delta computation
		delta = 0;
		length = track_timer[best_i];
		for (i = 0; length; ++i) {
			delta = (delta << 8) | (length & 0x7F);
			length >>= 7;
		}

		// Process MIDI event.
		copy_bytes = 0;
		pos = track_pos[best_i];
		do {
			event = *(pos++);
			if (event < 0x80)
				event = running_status[best_i];
		} while (_malformedPitchBends && (event & 0xF0) == 0xE0 && pos++);
		running_status[best_i] = event;

		if (command_lengths [(event >> 4) - 8] > 0) {
			copy_bytes = command_lengths [(event >> 4) - 8];
		} else if (special_lengths [(event & 0x0F)] > 0) {
			copy_bytes = special_lengths [(event & 0x0F)];
		} else if (event == 0xF0) {
			// SysEx
			pos2 = pos;
			length = readVLQ (pos);
			copy_bytes = 1 + (pos - pos2) + length;
		} else if (event == 0xFF) {
			// META
			event = *(pos++);
			if (event == 0x2F && active_tracks > 1) {
				track_pos[best_i] = 0;
				write = false;
			} else {
				pos2 = pos;
				length = readVLQ (pos);
				copy_bytes = 2 + (pos - pos2) + length;
			}
			if (event == 0x2F)
				--active_tracks;
		} else {
			printf ("Bad MIDI command %02X!\n", (int) event);
			track_pos[best_i] = 0;
		}

		if (track_pos[best_i]) {
			// Update all tracks' deltas
			for (i = 0; i < _num_tracks; ++i) {
				if (track_pos[i] && i != best_i)
					track_timer[i] -= track_timer[best_i];
			}

			if (write) {
				// Write VLQ delta
				do {
					*output++ = (byte) (delta & 0xFF | (delta > 0xFF ? 0x80 : 0));
					delta >>= 8;
				} while (delta);

				// Write MIDI data
				memcpy (output, track_pos[best_i], copy_bytes);
				output += copy_bytes;
			}

			// Fetch new VLQ delta for winning track
			track_pos[best_i] += copy_bytes;
			track_timer[best_i] = readVLQ (track_pos[best_i]);
		}
	}

	*output++ = 0x00;
}

void MidiParser_SMF::allNotesOff() {
	if (!_driver)
		return;

	int i;
	for (i = 0; i < 15; ++i) {
		_driver->send (0x007BB0 | i);
	}
}

void MidiParser_SMF::unloadMusic() {
	_play_pos = NULL;
	_data = NULL;
	_num_tracks = 0;
	_active_track = 255;
	_play_time = 0;
	_last_event_time = 0;
	_running_status = 0;
	allNotesOff();
}

void MidiParser_SMF::setTrack (byte track) {
	if (track >= _num_tracks || track == _active_track)
		return;
	_active_track = track;
	_play_time = 0;
	_last_event_time = 0;
	_play_pos = _tracks[track];
	_running_status = 0;
	allNotesOff();
}

void MidiParser_SMF::jumpToTick (uint32 tick) {
	if (_active_track >= _num_tracks)
		return;
	_play_pos = _tracks[_active_track];
	_play_time = 0;
	_last_event_time = 0;
	allNotesOff();
	if (tick == 0)
		return;

	uint32 current_tick = 0;
	byte *start;
	uint32 event_count = 0;

	while (current_tick < tick) {
		start = _play_pos;
		uint32 delta = readVLQ (_play_pos);

		if (current_tick + delta >= tick) {
			_play_pos = start;
			_play_time += (tick - current_tick) * _psec_per_tick;
			break;
		}

		++event_count;
		current_tick += delta;
		_play_time += delta * _psec_per_tick;
		_last_event_time = _play_time;

		byte event;
		do {
			event = *_play_pos;
			if (event < 0x80)
				event = _running_status;
		} while (_malformedPitchBends && (event & 0xF0) == 0xE0 && _play_pos++);
		_running_status = event;

		byte bytes_to_skip = 0;
		if (command_lengths[(event >> 4) - 8] > 0) {
			_play_pos += command_lengths[(event >> 4) - 8];
		} else if (special_lengths[event & 0xF] > 0) {
			_play_pos += special_lengths[event & 0xF];
		} else if (event == 0xF0) {
			uint32 length = readVLQ (++_play_pos);
			_play_pos += length;
		} else if (event == 0xFF) {
			event = *(++_play_pos);
			uint32 length = readVLQ (++_play_pos);
			if (event == 0x2F) { // End of track
				_play_pos = 0;
				_driver->metaEvent (event, _play_pos, (uint16) length);
				break;
			} else if (event == 0x51) { // Tempo
				if (length >= 3) {
					delta = _play_pos[0] << 16 | _play_pos[1] << 8 | _play_pos[2];
					_psec_per_tick = (delta + (_ppqn >> 2)) / _ppqn;
				}
			}
			_play_pos += length;
		}
	}
}

MidiParser *MidiParser::createParser_SMF() { return new MidiParser_SMF; }
