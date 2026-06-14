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

#include "engines/macs2/midiparser_macs2.h"
#include "common/endian.h"
#include "common/textconsole.h"

namespace Macs2 {

MidiParser_Macs2::MidiParser_Macs2() : MidiParser(), _dataStart(nullptr), _timerFrequency(0) {
}

bool MidiParser_Macs2::loadMusic(const byte *data, uint32 size) {
	unloadMusic();

	if (!data || size < 0x26)
		return false;

	// Parse song header
	// Offset 0x06: instrument data offset (relative to file start)
	// Offset 0x08: song data offset (relative to file start)
	// Offset 0x0C: timer frequency
	uint16 dataOffset = READ_LE_UINT16(data + 0x08);
	_timerFrequency = READ_LE_UINT16(data + 0x0C);

	if (dataOffset >= size)
		return false;

	_dataStart = data + dataOffset;

	_numTracks = 1;
	_numSubtracks[0] = 1;
	_tracks[0][0] = _dataStart;

	// The original decrements _nextEventTimer on EVERY timer callback (120 Hz),
	// not just on subdivision ticks. So delta=1 means 1/120th of a second.
	// MidiParser: µs per tick = tempo / ppqn. With ppqn=1, tempo = µs per tick.
	// At 120 callbacks/sec: µs per tick = 1000000/120 = 8333.
	// Since our timer rate is also set to 1000000/120, setting tempo=8333 with
	// ppqn=1 means exactly 1 tick per callback, matching the original behavior.
	_ppqn = 1;

	resetTracking();
	setTempo(8333);
	setTrack(0);
	return true;
}

void MidiParser_Macs2::parseNextEvent(EventInfo &info) {
	const byte *playPos = _position._subtracks[0]._playPos;
	info.start = playPos;
	info.length = 0;
	info.loop = false;
	info.noop = false;

	// Read VLQ delta time (standard MIDI format)
	info.delta = readVLQ(playPos);

	// Read event byte; handle running status
	if (playPos[0] & 0x80) {
		info.event = *(playPos++);
		_position._subtracks[0]._runningStatus = info.event;
	} else {
		info.event = _position._subtracks[0]._runningStatus;
	}

	if (info.event < 0x80) {
		// Invalid state - treat as end of track
		info.event = 0xFF;
		info.ext.type = 0x2F;
		info.length = 0;
		_position._subtracks[0]._playPos = playPos;
		return;
	}

	switch (info.command()) {
	case 0x9: // Note On
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);
		if (info.basic.param2 == 0) {
			// Velocity 0 = note off
			info.event = info.channel() | 0x80;
		}
		break;

	case 0x8: // Note Off
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);
		break;

	case 0xA: // Aftertouch (polyphonic key pressure) - consumed but ignored
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);
		info.noop = true;
		break;

	case 0xB: // Control Change
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);
		// Custom CCs 0x66-0x69 are game-specific; pass them through
		// and let the MidiDriver handle them (or ignore them).
		break;

	case 0xC: // Program Change
		info.basic.param1 = *(playPos++);
		info.basic.param2 = 0;
		break;

	case 0xD: // Channel Pressure
		info.basic.param1 = *(playPos++);
		info.basic.param2 = 0;
		info.noop = true;
		break;

	case 0xE: // Pitch Wheel - consumed but treated as noop
		info.basic.param1 = *(playPos++);
		info.basic.param2 = *(playPos++);
		info.noop = true;
		break;

	case 0xF: // System/Meta events - signal loop to start
		// The macs2 format uses any 0xFx event to loop back to the beginning
		info.event = 0xFF;
		info.ext.type = 0x2F; // End of track (parser will handle looping via _autoLoop)
		info.length = 0;
		info.loop = true;
		break;

	default:
		warning("MidiParser_Macs2: unknown event %02x", info.event);
		info.noop = true;
		break;
	}

	_position._subtracks[0]._playPos = playPos;
}

void MidiParser_Macs2::resetTracking() {
	MidiParser::resetTracking();
}

} // End of namespace Macs2
