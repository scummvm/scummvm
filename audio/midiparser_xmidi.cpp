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

#include "audio/midiparser.h"
#include "audio/mididrv.h"
#include "common/textconsole.h"
#include "common/util.h"

/**
 * The XMIDI version of MidiParser.
 *
 * Much of this code is adapted from the XMIDI implementation from the exult
 * project.
 */
class MidiParser_XMIDI : public MidiParser {
protected:
	static const uint8 MAXIMUM_TRACK_BRANCHES = 128;

	struct Loop {
		byte *pos;
		byte repeat;
	};

	Loop _loop[4];
	int _loopCount;

	/**
	 * The source number to use when sending MIDI messages to the driver.
	 * When using multiple sources, use source 0 and higher. This must be
	 * used when source volume or channel locking is used.
	 * By default this is -1, which means the parser is the only source
	 * of MIDI messages and multiple source functionality is disabled.
	 */
	int8 _source;
	/**
	 * The sequence branches defined for each track. These point to
	 * positions in the MIDI data.
	 */
	byte *_trackBranches[MAXIMUM_TRACKS][MAXIMUM_TRACK_BRANCHES];

	XMidiCallbackProc _callbackProc;
	void *_callbackData;

	// TODO:
	// This should possibly get cleaned up at some point, but it's very tricks.
	// We need to support XMIDI TIMB for 7th guest, which uses
	// Miles Audio drivers. The MT32 driver needs to get the TIMB chunk, so that it
	// can install all required timbres before the song starts playing.
	// But we can't easily implement this directly like for example creating
	// a special Miles Audio class for usage in this XMIDI-class, because other engines use this
	// XMIDI-parser but w/o using Miles Audio drivers.
	XMidiNewTimbreListProc _newTimbreListProc;
	MidiDriver_BASE       *_newTimbreListDriver;

	byte  *_tracksTimbreList[120]; ///< Timbre-List for each track.
	uint32 _tracksTimbreListSize[120]; ///< Size of the Timbre-List for each track.
	byte  *_activeTrackTimbreList;
	uint32 _activeTrackTimbreListSize;

protected:
	uint32 readVLQ2(byte * &data);
	/**
	 * Platform independent LE uint32 read-and-advance.
	 * This helper function reads Little Endian 32-bit numbers
	 * from a memory pointer, at the same time advancing
	 * the pointer.
	 */
	uint32 read4low(byte *&data);

	void parseNextEvent(EventInfo &info) override;

	virtual void resetTracking() override {
		MidiParser::resetTracking();
		_loopCount = -1;
	}

	void sendToDriver(uint32 b) override;
	void sendMetaEventToDriver(byte type, byte *data, uint16 length) override;
public:
	MidiParser_XMIDI(XMidiCallbackProc proc, void *data, XMidiNewTimbreListProc newTimbreListProc, MidiDriver_BASE *newTimbreListDriver, int8 source = -1) :
			_callbackProc(proc),
			_callbackData(data),
			_newTimbreListProc(newTimbreListProc),
			_newTimbreListDriver(newTimbreListDriver),
			_source(source),
			_loopCount(-1),
			_activeTrackTimbreList(NULL),
			_activeTrackTimbreListSize(0) {
		memset(_loop, 0, sizeof(_loop));
		memset(_trackBranches, 0, sizeof(_trackBranches));
		memset(_tracksTimbreList, 0, sizeof(_tracksTimbreList));
		memset(_tracksTimbreListSize, 0, sizeof(_tracksTimbreListSize));
	}
	~MidiParser_XMIDI() { }

	bool loadMusic(byte *data, uint32 size) override;
	bool hasJumpIndex(uint8 index) override;
	bool jumpToIndex(uint8 index, bool stopNotes) override;
};

// This is a special XMIDI variable length quantity
uint32 MidiParser_XMIDI::readVLQ2(byte * &pos) {
	uint32 value = 0;
	while (!(pos[0] & 0x80)) {
		value += *pos++;
	}
	return value;
}

uint32 MidiParser_XMIDI::read4low(byte *&data) {
	uint32 val = READ_LE_UINT32(data);
	data += 4;
	return val;
}

bool MidiParser_XMIDI::hasJumpIndex(uint8 index) {
	if (_activeTrack >= _numTracks)
		return false;

	return index < MAXIMUM_TRACK_BRANCHES && _trackBranches[_activeTrack][index] != 0;
}

bool MidiParser_XMIDI::jumpToIndex(uint8 index, bool stopNotes) {
	if (_activeTrack >= _numTracks || _pause)
		return false;

	if (index >= MAXIMUM_TRACK_BRANCHES || _trackBranches[_activeTrack][index] == 0) {
		warning("MidiParser-XMIDI: jumpToIndex called with invalid sequence branch index %x", index);
		return false;
	}

	// Prevent concurrent execution of multiple jumps
	assert(!_jumpingToTick);
	_jumpingToTick = true;

	if (stopNotes) {
		if (!_smartJump || !_position._playPos) {
			allNotesOff();
		} else {
			hangAllActiveNotes();
		}
	}

	resetTracking();
	_position._playPos = _trackBranches[_activeTrack][index];
	parseNextEvent(_nextEvent);

	_jumpingToTick = false;

	return true;
}

void MidiParser_XMIDI::parseNextEvent(EventInfo &info) {
	info.start = _position._playPos;
	info.delta = readVLQ2(_position._playPos);

	// Process the next event.
	info.event = *(_position._playPos++);
	switch (info.event >> 4) {
	case 0x9: // Note On
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = *(_position._playPos++);
		info.length = readVLQ(_position._playPos);
		if (info.length == 0) {
			// Notes with length 0 are played with a very short duration by the AIL driver.
			// However, the MidiParser will treat notes with length 0 as "active notes"; i.e.
			// they will only get turned off when a corresponding Note Off event is encountered.
			// Because XMIDI does not contain Note Off events, this will cause the note to hang.
			// Set length to 1 to prevent this from happening.
			info.length = 1;
		}
		if (info.basic.param2 == 0) {
			info.event = info.channel() | 0x80;
			info.length = 0;
		}
		break;

	case 0xC:
	case 0xD:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = 0;
		break;

	case 0x8:
	case 0xA:
	case 0xE:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = *(_position._playPos++);
		break;

	case 0xB:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = *(_position._playPos++);

		// This isn't a full XMIDI implementation, but it should
		// hopefully be "good enough" for most things.

		switch (info.basic.param1) {
		// Simplified XMIDI looping.
		case 0x74: {	// XMIDI_CONTROLLER_FOR_LOOP
				byte *pos = _position._playPos;
				if (_loopCount < ARRAYSIZE(_loop) - 1)
					_loopCount++;
				else
					warning("XMIDI: Exceeding maximum loop count %d", ARRAYSIZE(_loop));

				_loop[_loopCount].pos = pos;
				_loop[_loopCount].repeat = info.basic.param2;
				break;
			}

		case 0x75:	// XMIDI_CONTROLLER_NEXT_BREAK
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
							_position._playPos = _loop[_loopCount].pos;
					} else {
						_position._playPos = _loop[_loopCount].pos;
					}
				}
			}
			break;

		case 0x77:	// XMIDI_CONTROLLER_CALLBACK_TRIG
			if (_callbackProc)
				_callbackProc(info.basic.param2, _callbackData);
			break;

		case 0x78:	// XMIDI_CONTROLLER_SEQ_BRANCH_INDEX
			// This controller marks a branch point. It is converted
			// to an entry in the RBRN header by the XMIDI conversion
			// tool. For playback it is unnecessary.
			break;

		case 0x6e:	// XMIDI_CONTROLLER_CHAN_LOCK
		case 0x6f:	// XMIDI_CONTROLLER_CHAN_LOCK_PROT
		case 0x70:	// XMIDI_CONTROLLER_VOICE_PROT
		case 0x71:	// XMIDI_CONTROLLER_TIMBRE_PROT
		case 0x72:	// XMIDI_CONTROLLER_BANK_CHANGE
			// These controllers are handled in the Miles drivers
			break;

		case 0x73:	// XMIDI_CONTROLLER_IND_CTRL_PREFIX
		case 0x76:	// XMIDI_CONTROLLER_CLEAR_BB_COUNT
		default:
			if (info.basic.param1 >= 0x73 && info.basic.param1 <= 0x76) {
				warning("Unsupported XMIDI controller %d (0x%2x)",
					info.basic.param1, info.basic.param1);
			}
			break;
		}

		// Should we really keep passing the XMIDI controller events to
		// the MIDI driver, or should we turn them into some kind of
		// NOP events? (Dummy meta events, perhaps?) Ah well, it has
		// worked so far, so it shouldn't cause any damage...

		break;

	case 0xF: // Meta or SysEx event
		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer
			info.basic.param1 = *(_position._playPos++);
			info.basic.param2 = *(_position._playPos++);
			break;

		case 0x3: // Song Select
			info.basic.param1 = *(_position._playPos++);
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
			info.length = readVLQ(_position._playPos);
			info.ext.data = _position._playPos;
			_position._playPos += info.length;
			break;

		case 0xF: // META event
			info.ext.type = *(_position._playPos++);
			info.length = readVLQ(_position._playPos);
			info.ext.data = _position._playPos;
			_position._playPos += info.length;
			if (info.ext.type == 0x51 && info.length == 3) {
				// Tempo event. We want to make these constant 500,000.
				info.ext.data[0] = 0x07;
				info.ext.data[1] = 0xA1;
				info.ext.data[2] = 0x20;
			}
			break;

		default:
			warning("MidiParser_XMIDI::parseNextEvent: Unsupported event code %x", info.event);
			break;
		}
		break;

	default:
		break;
	}
}

bool MidiParser_XMIDI::loadMusic(byte *data, uint32 size) {
	uint32 i = 0;
	byte *start;
	uint32 len;
	uint32 chunkLen;
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
			_numTracks = 1;
		} else if (memcmp(pos, "XDIR", 4)) {
			// Not an XMIDI that we recognize
			warning("Expected 'XDIR' but found '%c%c%c%c'", pos[0], pos[1], pos[2], pos[3]);
			return false;
		} else {
			// Seems Valid
			pos += 4;
			_numTracks = 0;

			for (i = 4; i < len; i++) {
				// Read 4 bytes of type
				memcpy(buf, pos, 4);
				pos += 4;

				// Read length of chunk
				chunkLen = read4high(pos);

				// Add eight bytes
				i += 8;

				if (memcmp(buf, "INFO", 4) == 0) {
					// Must be at least 2 bytes long
					if (chunkLen < 2) {
						warning("Invalid chunk length %d for 'INFO' block", (int)chunkLen);
						return false;
					}

					_numTracks = (byte)read2low(pos);

					if (chunkLen > 2) {
						warning("Chunk length %d is greater than 2", (int)chunkLen);
						//pos += chunkLen - 2;
					}
					break;
				}

				// Must align
				pos += (chunkLen + 1) & ~1;
				i += (chunkLen + 1) & ~1;
			}

			// Didn't get to fill the header
			if (_numTracks == 0) {
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
		if (_numTracks > ARRAYSIZE(_tracks)) {
			warning("Can only handle %d tracks but was handed %d", (int)ARRAYSIZE(_tracks), (int)_numTracks);
			return false;
		}

		int tracksRead = 0;
		uint32 branchOffsets[128];
		memset(branchOffsets, 0, sizeof(branchOffsets));
		while (tracksRead < _numTracks) {
			if (!memcmp(pos, "FORM", 4)) {
				// Skip this plus the 4 bytes after it.
				pos += 8;
			} else if (!memcmp(pos, "XMID", 4)) {
				// Skip this.
				pos += 4;
			} else if (!memcmp(pos, "TIMB", 4)) {
				// Custom timbres
				// chunk data is as follows:
				// UINT16LE timbre count (amount of custom timbres used by this track)
				//   BYTE     patchId
				//   BYTE     bankId
				//    * timbre count
				pos += 4;
				len = read4high(pos);
				_tracksTimbreList[tracksRead] = pos; // Skip the length bytes
				_tracksTimbreListSize[tracksRead] = len;
				pos += (len + 1) & ~1;
			} else if (!memcmp(pos, "EVNT", 4)) {
				// Ahh! What we're looking for at last.
				_tracks[tracksRead] = pos + 8; // Skip the EVNT and length bytes
				pos += 4;
				len = read4high(pos);
				pos += (len + 1) & ~1;
				// Calculate branch index positions using the track position we just found
				for (int j = 0; j < MAXIMUM_TRACK_BRANCHES; ++j) {
					if (branchOffsets[j] != 0) {
						byte *branchPos = _tracks[tracksRead] + branchOffsets[j];
						if (branchPos >= pos) {
							warning("Invalid sequence branch position (after track end)");
							branchPos = _tracks[tracksRead];
						}
						_trackBranches[tracksRead][j] = branchPos;
					}
				}
				// Clear the branch offsets for the next track
				memset(branchOffsets, 0, sizeof(branchOffsets));
				++tracksRead;
			} else if (!memcmp(pos, "RBRN", 4)) {
				// optional branch point offsets
				pos += 4;
				len = read4high(pos);
				uint16 numBranches = (len - 2) / 6;
				uint16 numBranches2 = read2low(pos);
				if (numBranches != numBranches2) {
					warning("Number of sequence branch definitions %d does not match RBRN block length %d", numBranches2, len);
					numBranches = 0;
				}
				for (int j = 0; j < numBranches; ++j) {
					uint16 index = read2low(pos);
					if (index >= MAXIMUM_TRACK_BRANCHES) {
						warning("Invalid sequence branch index value %x", index);
						pos += 4;
						continue;
					}
					// This is the offset from the start of the track
					branchOffsets[index] = read4low(pos);
				}
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
		setTrack(0);
		_activeTrackTimbreList = _tracksTimbreList[0];
		_activeTrackTimbreListSize = _tracksTimbreListSize[0];

		if (_newTimbreListProc)
			_newTimbreListProc(_newTimbreListDriver, _activeTrackTimbreList, _activeTrackTimbreListSize);

		return true;
	}

	return false;
}

void MidiParser_XMIDI::sendToDriver(uint32 b) {
	if (_source < 0) {
		MidiParser::sendToDriver(b);
	} else {
		_driver->send(_source, b);
	}
}

void MidiParser_XMIDI::sendMetaEventToDriver(byte type, byte *data, uint16 length) {
	if (_source < 0) {
		MidiParser::sendMetaEventToDriver(type, data, length);
	} else {
		_driver->metaEvent(_source, type, data, length);
	}
}

void MidiParser::defaultXMidiCallback(byte eventData, void *data) {
	warning("MidiParser: defaultXMidiCallback(%d)", eventData);
}

MidiParser *MidiParser::createParser_XMIDI(XMidiCallbackProc proc, void *data, XMidiNewTimbreListProc newTimbreListProc, MidiDriver_BASE *newTimbreListDriver, int source) {
	return new MidiParser_XMIDI(proc, data, newTimbreListProc, newTimbreListDriver, source);
}
