/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/midiparser_smf.h"

#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "common/textconsole.h"
#include "common/util.h"

static const byte commandLengths[8] = { 3, 3, 3, 3, 2, 2, 3, 0 };
static const byte specialLengths[16] = { 0, 2, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 };

MidiParser_SMF::MidiParser_SMF(int8 source) : MidiParser(source), _buffer(nullptr) {
	for (int i = 0; i < ARRAYSIZE(_noteChannelToTrack); i++)
		_noteChannelToTrack[i] = -1;
}

MidiParser_SMF::~MidiParser_SMF() {
	free(_buffer);
}

void MidiParser_SMF::parseNextEvent(EventInfo &info) {
	info.start = _position._playPos;
	info.delta = readVLQ(_position._playPos);

	// Process the next info.
	if ((_position._playPos[0] & 0xF0) >= 0x80)
		info.event = *(_position._playPos++);
	else
		info.event = _position._runningStatus;
	if (info.event < 0x80)
		return;

	_position._runningStatus = info.event;
	switch (info.command()) {
	case 0x9: // Note On
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = *(_position._playPos++);
		if (info.basic.param2 == 0)
			info.event = info.channel() | 0x80;
		info.length = 0;
		break;

	case 0xC:
	case 0xD:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = 0;
		break;

	case 0x8:
	case 0xA:
	case 0xB:
	case 0xE:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = *(_position._playPos++);
		info.length = 0;
		break;

	case 0xF: // System Common, Meta or SysEx event
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
			break;

		default:
			warning("MidiParser_SMF::parseNextEvent: Unsupported event code %x", info.event);
			break;
		}
		break;

	default:
		break;
	}
}

bool MidiParser_SMF::loadMusic(byte *data, uint32 size) {
	uint32 len;
	byte midiType;

	unloadMusic();
	byte *pos = data;

	if (!memcmp(pos, "RIFF", 4)) {
		// Skip the outer RIFF header.
		pos += 8;
	}

	if (!memcmp(pos, "MThd", 4)) {
		// SMF with MTHd information.
		pos += 4;
		len = read4high(pos);
		if (len != 6) {
			warning("MThd length 6 expected but found %d", (int)len);
			return false;
		}

		// Verify that this MIDI either is a Type 2
		// or has only 1 track. We do not support
		// multitrack Type 1 files.
		_numTracks = pos[2] << 8 | pos[3];
		midiType = pos[1];
		if (midiType > 2 /*|| (midiType < 2 && _numTracks > 1)*/) {
			warning("No support for a Type %d MIDI with %d tracks", (int)midiType, (int)_numTracks);
			return false;
		}
		_ppqn = pos[4] << 8 | pos[5];
		pos += len;
	} else {
		warning("Expected MThd header but found '%c%c%c%c' instead", pos[0], pos[1], pos[2], pos[3]);
		return false;
	}

	// Now we identify and store the location for each track.
	if (_numTracks > ARRAYSIZE(_tracks)) {
		warning("Can only handle %d tracks but was handed %d", (int)ARRAYSIZE(_tracks), (int)_numTracks);
		return false;
	}

	int tracksRead = 0;
	while (tracksRead < _numTracks) {
		if (memcmp(pos, "MTrk", 4)) {
			warning("Position: %p ('%c')", (void *)pos, *pos);
			warning("Hit invalid block '%c%c%c%c' while scanning for track locations", pos[0], pos[1], pos[2], pos[3]);
			return false;
		}

		// Skip the MTrk and length bytes
		_tracks[tracksRead] = pos + 8;
		pos += 4;
		len = read4high(pos);
		pos += len;
		++tracksRead;
	}

	// If this is a Type 1 MIDI, we need to now compress
	// our tracks down into a single Type 0 track.
	free(_buffer);
	_buffer = nullptr;

	if (midiType == 1) {
		// FIXME: Doubled the buffer size to prevent crashes with the
		// Inherit the Earth MIDIs. Jamieson630 said something about a
		// better fix, but this will have to do in the meantime.
		_buffer = (byte *)malloc(size * 2);
		compressToType0(_tracks, _numTracks, _buffer, false);
		_numTracks = 1;
		_tracks[0] = _buffer;
	}

	// Note that we assume the original data passed in
	// will persist beyond this call, i.e. we do NOT
	// copy the data to our own buffer. Take warning....
	resetTracking();
	setTempo(500000);
	setTrack(0);
	return true;
}

int32 MidiParser_SMF::determineDataSize(Common::SeekableReadStream *stream) {
	// Determine the MIDI data size by skipping over the header and all the
	// MIDI tracks, then comparing start and end stream positions.
	uint32 startPos = stream->pos();

	// Skip over the header.
	byte buf[4];
	Common::fill(buf, buf + 4, 0);
	stream->read(buf, 4);
	if (memcmp(buf, "MThd", 4) != 0) {
		warning("Expected MThd but found '%c%c%c%c' instead", buf[0], buf[1], buf[2], buf[3]);
		return -1;
	}
	stream->seek(stream->readUint32BE(), SEEK_CUR);

	// Now skip all the MTrk blocks.
	while (true) {
		Common::fill(buf, buf + 4, 0);
		int read = stream->read(buf, 4);
		if (read < 4 || memcmp(buf, "MTrk", 4) != 0) {
			stream->seek(-read, SEEK_CUR);
			break;
		}
		stream->seek(stream->readUint32BE(), SEEK_CUR);
	}

	// The stream is now at the end of the MIDI data, so the size is the
	// difference between the current and starting stream position.
	return stream->pos() - startPos;
}

uint32 MidiParser_SMF::compressToType0(byte *tracks[], byte numTracks, byte *buffer, bool malformedPitchBends) {
	// We assume that buffer has been allocated
	// to sufficient size for this operation.

	// using 0xFF since it could write trackPos[0 to numTracks] here
	// this would cause some illegal writes and could lead to segfaults
	// (it crashed for some midis for me, they're not used in any game
	// scummvm supports though). *Maybe* handle this in another way,
	// it's at the moment only to be sure, that nothing goes wrong.
	byte *trackPos[0xFF];
	byte runningStatus[0xFF];
	uint32 trackTimer[0xFF];
	uint32 delta;
	int i;

	for (i = 0; i < numTracks; ++i) {
		runningStatus[i] = 0;
		trackPos[i] = tracks[i];
		trackTimer[i] = readVLQ(trackPos[i]);
		runningStatus[i] = 0;
	}

	int bestTrack;
	uint32 length;
	byte *output = buffer;
	byte *pos, *pos2;
	byte event;
	uint32 copyBytes;
	bool write;
	byte activeTracks = numTracks;

	while (activeTracks) {
		write = true;
		bestTrack = 255;
		for (i = 0; i < numTracks; ++i) {
			if (trackPos[i] && (bestTrack == 255 || trackTimer[i] < trackTimer[bestTrack]))
				bestTrack = i;
		}
		if (bestTrack == 255) {
			warning("Premature end of tracks");
			break;
		}

		// Initial VLQ delta computation
		delta = 0;
		length = trackTimer[bestTrack];
		for (i = 0; length; ++i) {
			delta = (delta << 8) | (length & 0x7F) | (i ? 0x80 : 0);
			length >>= 7;
		}

		// Process MIDI event.
		bool implicitEvent = false;
		copyBytes = 0;
		pos = trackPos[bestTrack];
		do {
			event = *(pos++);
			if (event < 0x80) {
				event = runningStatus[bestTrack];
				implicitEvent = true;
			}
		} while (malformedPitchBends && (event & 0xF0) == 0xE0 && pos++);
		runningStatus[bestTrack] = event;

		if (commandLengths[(event >> 4) - 8] > 0) {
			copyBytes = commandLengths[(event >> 4) - 8];
			if ((event & 0xf0) == MidiDriver_BASE::MIDI_COMMAND_NOTE_ON)
				_noteChannelToTrack[event & 0x0f] = bestTrack;
		} else if (specialLengths[(event & 0x0F)] > 0) {
			copyBytes = specialLengths[(event & 0x0F)];
		} else if (event == 0xF0) {
			// SysEx
			pos2 = pos;
			length = readVLQ(pos);
			copyBytes = 1 + (pos - pos2) + length;
		} else if (event == 0xFF) {
			// META
			event = *(pos++);
			if (event == 0x2F && activeTracks > 1) {
				trackPos[bestTrack] = nullptr;
				write = false;
			} else {
				pos2 = pos;
				length = readVLQ(pos);
				copyBytes = 2 + (pos - pos2) + length;
			}
			if (event == 0x2F)
				--activeTracks;
		} else {
			warning("Bad MIDI command %02X", (int)event);
			trackPos[bestTrack] = nullptr;
		}

		// Update all tracks' deltas
		if (write) {
			for (i = 0; i < numTracks; ++i) {
				if (trackPos[i] && i != bestTrack)
					trackTimer[i] -= trackTimer[bestTrack];
			}
		}

		if (trackPos[bestTrack]) {
			if (write) {
				trackTimer[bestTrack] = 0;

				// Write VLQ delta
				while (delta & 0x80) {
					*output++ = (byte)(delta & 0xFF);
					delta >>= 8;
				}
				*output++ = (byte)(delta & 0xFF);

				// Write MIDI data
				if (!implicitEvent)
					++trackPos[bestTrack];
				--copyBytes;
				*output++ = runningStatus[bestTrack];
				memcpy(output, trackPos[bestTrack], copyBytes);
				output += copyBytes;
			}

			// Fetch new VLQ delta for winning track
			trackPos[bestTrack] += copyBytes;
			if (activeTracks)
				trackTimer[bestTrack] += readVLQ(trackPos[bestTrack]);
		}
	}

	*output++ = 0x00;

	return output - buffer;
}

MidiParser *MidiParser::createParser_SMF(int8 source) { return new MidiParser_SMF(source); }
