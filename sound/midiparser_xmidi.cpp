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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sound/midiparser.h"
#include "sound/mididrv.h"
#include "common/util.h"

/**
 * The XMIDI version of MidiParser.
 *
 * Much of this code is adapted from the XMIDI implementation from the exult
 * project.
 */
class MidiParser_XMIDI : public MidiParser {
protected:
	NoteTimer _notes_cache[32];
	uint32 _inserted_delta; // Track simulated deltas for note-off events

	struct Loop {
		byte *pos;
		byte repeat;
	};

	Loop _loop[4];
	int _loopCount;

	XMidiCallbackProc _callbackProc;
	void *_callbackData;

protected:
	uint32 readVLQ2(byte * &data);
	void resetTracking();
	void parseNextEvent(EventInfo &info);

public:
	MidiParser_XMIDI(XMidiCallbackProc proc, void *data) : _inserted_delta(0), _callbackProc(proc), _callbackData(data) {}
	~MidiParser_XMIDI() { }

	bool loadMusic(byte *data, uint32 size);
};


// This is a special XMIDI variable length quantity
uint32 MidiParser_XMIDI::readVLQ2(byte * &pos) {
	uint32 value = 0;
	while (!(pos[0] & 0x80)) {
		value += *pos++;
	}
	return value;
}

void MidiParser_XMIDI::parseNextEvent(EventInfo &info) {
	info.start = _position._play_pos;
	info.delta = readVLQ2(_position._play_pos) - _inserted_delta;

	// Process the next event.
	_inserted_delta = 0;
	info.event = *(_position._play_pos++);
	switch (info.event >> 4) {
	case 0x9: // Note On
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = *(_position._play_pos++);
		info.length = readVLQ(_position._play_pos);
		if (info.basic.param2 == 0) {
			info.event = info.channel() | 0x80;
			info.length = 0;
		}
		break;

	case 0xC:
	case 0xD:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = 0;
		break;

	case 0x8:
	case 0xA:
	case 0xE:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = *(_position._play_pos++);
		break;

	case 0xB:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = *(_position._play_pos++);

		// This isn't a full XMIDI implementation, but it should
		// hopefully be "good enough" for most things.

		switch (info.basic.param1) {
		// Simplified XMIDI looping.
		case 0x74: {	// XMIDI_CONTROLLER_FOR_LOOP
				byte *pos = _position._play_pos;
				if (_loopCount < ARRAYSIZE(_loop) - 1)
					_loopCount++;
				else
					warning("XMIDI: Exceeding maximum loop count %d", ARRAYSIZE(_loop));

				_loop[_loopCount].pos = pos;
				_loop[_loopCount].repeat = info.basic.param2;
				break;
			}

		case 0x75:	// XMIDI_CONTORLLER_NEXT_BREAK
			if (_loopCount >= 0) {
				if (info.basic.param2 < 64) {
					// End the current loop.
					_loopCount--;
				} else {
					// Repeat 0 means "loop forever".
					if (_loop[_loopCount].repeat) {
						if (--_loop[_loopCount].repeat == 0)
							_loopCount--;
						else
							_position._play_pos = _loop[_loopCount].pos;
					} else {
						_position._play_pos = _loop[_loopCount].pos;
					}
				}
			}
			break;

		case 0x77:	// XMIDI_CONTROLLER_CALLBACK_TRIG
			if (_callbackProc)
				_callbackProc(info.basic.param2, _callbackData);
			break;

		case 0x6e:	// XMIDI_CONTROLLER_CHAN_LOCK
		case 0x6f:	// XMIDI_CONTROLLER_CHAN_LOCK_PROT
		case 0x70:	// XMIDI_CONTROLLER_VOICE_PROT
		case 0x71:	// XMIDI_CONTROLLER_TIMBRE_PROT
		case 0x72:	// XMIDI_CONTROLLER_BANK_CHANGE
		case 0x73:	// XMIDI_CONTROLLER_IND_CTRL_PREFIX
		case 0x76:	// XMIDI_CONTROLLER_CLEAR_BB_COUNT
		case 0x78:	// XMIDI_CONTROLLER_SEQ_BRANCH_INDEX
		default:
			if (info.basic.param1 >= 0x6e && info.basic.param1 <= 0x78) {
				warning("Unsupported XMIDI controller %d (0x%2x)",
					info.basic.param1, info.basic.param1);
			}
		}

		// Should we really keep passing the XMIDI controller events to
		// the MIDI driver, or should we turn them into some kind of
		// NOP events? (Dummy meta events, perhaps?) Ah well, it has
		// worked so far, so it shouldn't cause any damage...

		break;

	case 0xF: // Meta or SysEx event
		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer
			info.basic.param1 = *(_position._play_pos++);
			info.basic.param2 = *(_position._play_pos++);
			break;

		case 0x3: // Song Select
			info.basic.param1 = *(_position._play_pos++);
			info.basic.param2 = 0;
			break;

		case 0x6:
		case 0x8:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xE:
			info.basic.param1 = info.basic.param2 = 0;
			break;

		case 0x0: // SysEx
			info.length = readVLQ(_position._play_pos);
			info.ext.data = _position._play_pos;
			_position._play_pos += info.length;
			break;

		case 0xF: // META event
			info.ext.type = *(_position._play_pos++);
			info.length = readVLQ(_position._play_pos);
			info.ext.data = _position._play_pos;
			_position._play_pos += info.length;
			if (info.ext.type == 0x51 && info.length == 3) {
				// Tempo event. We want to make these constant 500,000.
				info.ext.data[0] = 0x07;
				info.ext.data[1] = 0xA1;
				info.ext.data[2] = 0x20;
			}
			break;

		default:
			warning("MidiParser_XMIDI::parseNextEvent: Unsupported event code %x", info.event);
		}
	}
}

bool MidiParser_XMIDI::loadMusic(byte *data, uint32 size) {
	uint32 i = 0;
	byte *start;
	uint32 len;
	uint32 chunk_len;
	char buf[32];

	_loopCount = -1;

	unloadMusic();
	byte *pos = data;

	if (!memcmp(pos, "FORM", 4)) {
		pos += 4;

		// Read length of
		len = read4high(pos);
		start = pos;

		// XDIRless XMIDI, we can handle them here.
		if (!memcmp(pos, "XMID", 4)) {
			warning("XMIDI doesn't have XDIR");
			pos += 4;
			_num_tracks = 1;
		} else if (memcmp(pos, "XDIR", 4)) {
			// Not an XMIDI that we recognise
			warning("Expected 'XDIR' but found '%c%c%c%c'", pos[0], pos[1], pos[2], pos[3]);
			return false;
		} else {
			// Seems Valid
			pos += 4;
			_num_tracks = 0;

			for (i = 4; i < len; i++) {
				// Read 4 bytes of type
				memcpy(buf, pos, 4);
				pos += 4;

				// Read length of chunk
				chunk_len = read4high(pos);

				// Add eight bytes
				i += 8;

				if (memcmp(buf, "INFO", 4)) {
					// Must align
					pos += (chunk_len + 1) & ~1;
					i += (chunk_len + 1) & ~1;
					continue;
				}

				// Must be at least 2 bytes long
				if (chunk_len < 2) {
					warning("Invalid chunk length %d for 'INFO' block", (int)chunk_len);
					return false;
				}

				_num_tracks = (byte)read2low(pos);

				if (chunk_len > 2) {
					warning("Chunk length %d is greater than 2", (int)chunk_len);
					pos += chunk_len - 2;
				}
				break;
			}

			// Didn't get to fill the header
			if (_num_tracks == 0) {
				warning("Didn't find a valid track count");
				return false;
			}

			// Ok now to start part 2
			// Goto the right place
			pos = start + ((len + 1) & ~1);

			if (memcmp(pos, "CAT ", 4)) {
				// Not an XMID
				warning("Expected 'CAT ' but found '%c%c%c%c'", pos[0], pos[1], pos[2], pos[3]);
				return false;
			}
			pos += 4;

			// Now read length of this track
			len = read4high(pos);

			if (memcmp(pos, "XMID", 4)) {
				// Not an XMID
				warning("Expected 'XMID' but found '%c%c%c%c'", pos[0], pos[1], pos[2], pos[3]);
				return false;
			}
			pos += 4;

		}

		// Ok it's an XMIDI.
		// We're going to identify and store the location for each track.
		if (_num_tracks > ARRAYSIZE(_tracks)) {
			warning("Can only handle %d tracks but was handed %d", (int)ARRAYSIZE(_tracks), (int)_num_tracks);
			return false;
		}

		int tracks_read = 0;
		while (tracks_read < _num_tracks) {
			if (!memcmp(pos, "FORM", 4)) {
				// Skip this plus the 4 bytes after it.
				pos += 8;
			} else if (!memcmp(pos, "XMID", 4)) {
				// Skip this.
				pos += 4;
			} else if (!memcmp(pos, "TIMB", 4)) {
				// Custom timbres?
				// We don't support them.
				// Read the length, skip it, and hope there was nothing there.
				pos += 4;
				len = read4high(pos);
				pos += (len + 1) & ~1;
			} else if (!memcmp(pos, "EVNT", 4)) {
				// Ahh! What we're looking for at last.
				_tracks[tracks_read] = pos + 8; // Skip the EVNT and length bytes
				pos += 4;
				len = read4high(pos);
				pos += (len + 1) & ~1;
				++tracks_read;
			} else {
				warning("Hit invalid block '%c%c%c%c' while scanning for track locations", pos[0], pos[1], pos[2], pos[3]);
				return false;
			}
		}

		// If we got this far, we successfully established
		// the locations for each of our tracks.
		// Note that we assume the original data passed in
		// will persist beyond this call, i.e. we do NOT
		// copy the data to our own buffer. Take warning....
		_ppqn = 60;
		resetTracking();
		setTempo(500000);
		_inserted_delta = 0;
		setTrack(0);
		return true;
	}

	return false;
}

void MidiParser_XMIDI::resetTracking() {
	MidiParser::resetTracking();
	_inserted_delta = 0;
}

void MidiParser::defaultXMidiCallback(byte eventData, void *data) {
	warning("MidiParser: defaultXMidiCallback(%d)", eventData);
}

MidiParser *MidiParser::createParser_XMIDI(XMidiCallbackProc proc, void *data) {
	return new MidiParser_XMIDI(proc, data);
}
