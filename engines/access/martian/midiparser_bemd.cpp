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

#include "access/martian/midiparser_bemd.h"

#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/file.h"

namespace Access {

MidiParser_BEmd::MidiParser_BEmd(): _tickData(nullptr), _tickDataEnd(nullptr), _trackDataEnd(nullptr) {
}

bool MidiParser_BEmd::loadMusic(const byte *data, uint32 size) {
	unloadMusic();
	const byte *pos = data;

	//
	// 'BEmd' MIDI format from Martian Memorandum.
	//
	// A simple single-track format that splits note and timing data.
	//
	// Header is:
	//     'BEmd' (magic number)
	//     0xC0 0x00 (unknown, always 0xC0?
	//     16-bit offset to timing data block
	//     16-bit size of timing data block
	//     6 bytes unk (normally 0)
	// Header is followed by the track data block, then timing delta data
	// block.
	//
	// Track data mostly follows other MIDI formats with a few differences:
	//  * Fixed length arguments
	//  * 0xFF 0x51 (tempo meta event) is followed by a uint16 which is fed
	//      directly to the PIT as a delay
	//  * Deltas are assumed to be 0. On 0xF8, a 16 bit int is read from
	//      the timing block and sent to the PIT as a timing delay.
	//
	if (!memcmp(pos, "BEmd", 4)) {
		pos += 4;
		if (size <= 16)
			error("Wrong BEmd music resource size");

		/*uint16 unk1 = */ READ_LE_UINT16(pos);  // Normally 0xC0?
		uint16 secondBlockOffset = READ_LE_UINT16(pos + 2);
		if (secondBlockOffset < 16 || secondBlockOffset >= size)
			error("Bad second block offset in BEmd file");
		uint16 secondBlockSize = READ_LE_UINT16(pos + 4);
		if (static_cast<uint32>(secondBlockOffset + secondBlockSize) != size)
			error("Bad second block offset+size in BEmd file");

		_trackDataEnd = data + secondBlockOffset;
		_tickData = _trackDataEnd;
		_tickDataEnd = data + size;

		// Only one track
		_numTracks = 1;
		_numSubtracks[0] = 1;
		_autoLoop = false;
		_ppqn = 1;
		_tracks[0][0] = data + 16;

		resetTracking();
		setTempo(16667);
		setTrack(0);
		return true;
	} else {
		warning("Expected BEmd header but found '%c%c%c%c' instead", pos[0], pos[1], pos[2], pos[3]);
		return false;
	}

	return false;
}


void MidiParser_BEmd::parseNextEvent(EventInfo &info) {
	uint8 subtrack = info.subtrack;
	const byte *playPos = _position._subtracks[subtrack]._playPos;
	info.start = playPos;

	info.delta = 0;

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

	case 0xF:
		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer
			info.basic.param1 = *(playPos++);
			info.basic.param2 = *(playPos++);
			break;

		case 0x3: // Song Select
			info.basic.param1 = *(playPos++);
			info.basic.param2 = 0;
			break;

		case 0x8: // Timing data
			// Tick data is stored separately.
			info.delta = READ_LE_UINT16(_tickData);
			_tickData += 2;
			// FALL THROUGH
		case 0x6:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xE:
			info.basic.param1 = info.basic.param2 = 0;
			break;

		case 0x0: // SysEx
			error("MidiParser_BEmd::parseNextEvent: Unexpected SysEx event");
			break;

		case 0xF: // META event
			info.ext.type = *(playPos++);
			if (info.ext.type == 0x51) {
				// Set Tempo - 2 bytes and interpreted as direct input for the PIT
				// as ticks to next note (0.8381uS/tick)
				setTempo(READ_LE_UINT16(playPos) * 0.8381);
			}
			info.length = (info.ext.type == 0x2f ? 0 : 2);
			info.ext.data = playPos;
			playPos += info.length;
			break;

		default:
			error("MidiParser_BEmd::parseNextEvent: Unsupported event code %x", info.event);
			break;
		}

	default:
		break;
	}

	_position._subtracks[subtrack]._playPos = playPos;

	assert(playPos < _trackDataEnd);
	assert(_tickData < _tickDataEnd);
}

bool MidiParser_BEmd::processEvent(const EventInfo &info, bool fireEvents) {
	// Ignore timer events we handled already.
	if ((info.event == 0xF8) || (info.event == 0xFF && info.ext.type == 0x51))
		return true;
	return MidiParser::processEvent(info, fireEvents);
}

void MidiParser_BEmd::resetTracking() {
	_tickData = _trackDataEnd;
	MidiParser::resetTracking();
}

} // end namespace Access
