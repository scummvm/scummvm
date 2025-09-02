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

MidiParser_SMF::MidiParser_SMF(int8 source) : MidiParser(source) {
}

void MidiParser_SMF::parseNextEvent(EventInfo &info) {
	uint8 subtrack = info.subtrack;
	const byte *playPos = _position._subtracks[subtrack]._playPos;
	info.start = playPos;
	info.delta = readVLQ(playPos);

	// Process the next info.
	if ((playPos[0] & 0xF0) >= 0x80)
		info.event = *(playPos++);
	else
		info.event = _position._subtracks[subtrack]._runningStatus;
	if (info.event < 0x80) {
		_position._subtracks[subtrack]._playPos = playPos;
		return;
	}

	_position._subtracks[subtrack]._runningStatus = info.event;
	switch (info.command()) {
	case 0x9: // Note On
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);
		if (info.basic.param2 == 0)
			info.event = info.channel() | 0x80;
		info.length = 0;
		break;

	case 0xC:
	case 0xD:
		info.basic.param1 = *(playPos++);
		info.basic.param2 = 0;
		break;

	case 0x8:
	case 0xA:
	case 0xB:
	case 0xE:
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);
		info.length = 0;
		break;

	case 0xF: // System Common, Meta or SysEx event
		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer
			info.basic.param1 = *(playPos++);
			info.basic.param2 = *(playPos++);
			break;

		case 0x3: // Song Select
			info.basic.param1 = *(playPos++);
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
			info.length = readVLQ(playPos);
			info.ext.data = playPos;
			playPos += info.length;
			break;

		case 0xF: // META event
			info.ext.type = *(playPos++);
			info.length = readVLQ(playPos);
			info.ext.data = playPos;
			playPos += info.length;
			break;

		default:
			warning("MidiParser_SMF::parseNextEvent: Unsupported event code %x", info.event);
			break;
		}
		break;

	default:
		break;
	}

	_position._subtracks[subtrack]._playPos = playPos;
}

bool MidiParser_SMF::loadMusic(const byte *data, uint32 size) {
	uint32 len;
	byte midiType;
	byte numTrackChunks;

	unloadMusic();
	const byte *pos = data;

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

		numTrackChunks = pos[2] << 8 | pos[3];
		midiType = pos[1];
		if (midiType > 2) {
			warning("Invalid MIDI type %d", (int)midiType);
			return false;
		}
		_numTracks = (midiType == 1 ? 1 : numTrackChunks);
		_ppqn = pos[4] << 8 | pos[5];
		pos += len;
	} else {
		warning("Expected MThd header but found '%c%c%c%c' instead", pos[0], pos[1], pos[2], pos[3]);
		return false;
	}

	// Now we identify and store the location for each track.
	if (_numTracks > MAXIMUM_TRACKS) {
		warning("Can only handle %d tracks but was handed %d", (int)MAXIMUM_TRACKS, (int)_numTracks);
		return false;
	}
	if (midiType == 1 && numTrackChunks > MAXIMUM_SUBTRACKS) {
		warning("Can only handle MIDI type 1 with %d subtracks but was handed %d", (int)MAXIMUM_SUBTRACKS, (int)numTrackChunks);
		return false;
	}

	int tracksRead = 0;
	while (tracksRead < numTrackChunks) {
		if (memcmp(pos, "MTrk", 4)) {
			warning("Position: %p ('%c')", (const void *)pos, *pos);
			warning("Hit invalid block '%c%c%c%c' while scanning for track locations", pos[0], pos[1], pos[2], pos[3]);
			return false;
		}

		// Skip the MTrk and length bytes
		if (midiType == 1) {
			_tracks[0][tracksRead] = pos + 8;
		}
		else {
			_tracks[tracksRead][0] = pos + 8;
			_numSubtracks[tracksRead] = 1;
		}
		pos += 4;
		len = read4high(pos);
		pos += len;
		++tracksRead;
	}
	if (midiType == 1) {
		_numSubtracks[0] = tracksRead;
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

MidiParser *MidiParser::createParser_SMF(int8 source) { return new MidiParser_SMF(source); }
