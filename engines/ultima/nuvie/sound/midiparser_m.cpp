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

#include "midiparser_m.h"

#include "audio/mididrv.h"
#include "audio/midiparser.h"

namespace Ultima {
namespace Nuvie {

MidiParser_M::MidiParser_M(int8 source) : MidiParser(source) {
	// M uses a fixed timer frequency of 60 Hz, or 16.667 ms per tick.
	_psecPerTick = 16667;

	_trackLength = 0;

	_loopPoint = nullptr;
	_loopStack = new Common::FixedStack<LoopData, 16>();
}

MidiParser_M::~MidiParser_M() {
	delete _loopStack;
}

bool MidiParser_M::loadMusic(byte* data, uint32 size) {
	unloadMusic();

	// M uses only 1 track.
	_tracks[0] = data;
	_numTracks = 1;
	_trackLength = size;

	// The global loop defaults to the start of the M data.
	_loopPoint = data;

	resetTracking();
	setTrack(0);

	return true;
}

void MidiParser_M::unloadMusic() {
	MidiParser::unloadMusic();
	_trackLength = 0;

	_loopPoint = nullptr;
	_loopStack->clear();
}

// MidiParser::onTimer does some checks based on MIDI note on/off command bytes
// which have a different meaning in M, so those checks are removed here.
void MidiParser_M::onTimer() {
	uint32 endTime;
	uint32 eventTime;

	if (!_position._playPos || !_driver || !_doParse || _pause || !_driver->isReady(_source))
		return;

	_abortParse = false;
	endTime = _position._playTime + _timerRate;

	bool loopEvent = false;
	while (!_abortParse) {
		EventInfo &info = _nextEvent;

		eventTime = _position._lastEventTime + info.delta * _psecPerTick;
		if (eventTime > endTime)
			break;

		if (!info.noop) {
			// Process the next info.
			bool ret = processEvent(info);
			if (!ret)
				return;
		}

		loopEvent |= info.loop;

		if (!_abortParse) {
			_position._lastEventTime = eventTime;
			_position._lastEventTick += info.delta;
			parseNextEvent(_nextEvent);
		}
	}

	if (!_abortParse) {
		_position._playTime = endTime;
		_position._playTick = (_position._playTime - _position._lastEventTime) / _psecPerTick + _position._lastEventTick;
		if (loopEvent) {
			// One of the processed events has looped (part of) the MIDI data.
			// Infinite looping will cause the tracker to overflow eventually.
			// Reset the tracker positions to prevent this from happening.
			_position._playTime -= _position._lastEventTime;
			_position._lastEventTime = 0;
			_position._playTick -= _position._lastEventTick;
			_position._lastEventTick = 0;
		}
	}
}

bool MidiParser_M::processEvent(const EventInfo& info, bool fireEvents) {
	if (info.command() == 0x8 && info.channel() == 0x1) {
		// Call subroutine
		LoopData loopData { };
		loopData.returnPos = _position._playPos;
		loopData.numLoops = info.ext.data[0];
		uint16 startOffset = READ_LE_UINT16(info.ext.data + 1);
		assert(startOffset < _trackLength);
		loopData.startPos = _tracks[0] + startOffset;
		_loopStack->push(loopData);
		_position._playPos = loopData.startPos;
	} else if (info.command() == 0xE) {
		// Set loop point
		_loopPoint = _position._playPos;
	} else if (info.command() == 0xF) {
		// Return
		if (_loopStack->empty()) {
			// Global loop: return to the global loop point
			_position._playPos = _loopPoint;
		} else {
			// Subroutine loop
			LoopData *loopData = &_loopStack->top();
			if (loopData->numLoops > 1) {
				// Return to the start of the subroutine data
				loopData->numLoops--;
				_position._playPos = loopData->startPos;
			} else {
				// Return to the call position
				_position._playPos = loopData->returnPos;
				_loopStack->pop();
			}
		}
	} else if (info.command() == 0x8 && info.channel() == 0x3) {
		// Load instrument
		if (fireEvents) {
			// Send the instrument data as a meta event
			sendMetaEventToDriver(info.ext.type, info.ext.data, (uint16)info.length);
		}
	} else if (fireEvents) {
		// Other events are handled by the driver
		sendToDriver(info.event, info.basic.param1, info.basic.param2);
	}

	return true;
}

void MidiParser_M::parseNextEvent(EventInfo &info) {
	assert(_position._playPos - _tracks[0] < _trackLength);
	info.start = _position._playPos;
	info.event = *(_position._playPos++);
	info.delta = 0;
	info.basic.param1 = 0;
	info.basic.param2 = 0;
	info.noop = false;
	info.loop = false;

	switch (info.command()) {
	case 0x0: // Note off
	case 0x1: // Note on
	case 0x2: // Set pitch
	case 0x3: // Set level
	case 0x4: // Set modulation
	case 0x5: // Set slide
	case 0x6: // Set vibrato
	case 0x7: // Program change
		// These commands all have 1 data byte.
		info.basic.param1 = *(_position._playPos++);
		break;

	case 0x8: // Subcommand
		switch (info.channel()) {
		case 0x1: // Call subroutine
			// This command specifies the number of loops (1 byte) and an
			// offset in the M data to jump to (2 bytes).
			info.ext.type = info.channel();
			info.length = 3;
			info.ext.data = _position._playPos;
			_position._playPos += info.length;
			break;
		case 0x2: // Delay
			// This command is used to specify a delta time between the
			// previous and the next event. It does nothing otherwise.
			info.delta = *(_position._playPos++);
			info.noop = true;
			break;
		case 0x3: // Load instrument
			// This command specifies the instrument bank slot in which the
			// instrument should be loaded (1 byte) plus an OPL instrument
			// definition (11 bytes).
			info.ext.type = info.channel();
			info.length = 12;
			info.ext.data = _position._playPos;
			_position._playPos += info.length;
			break;
		case 0x5: // Fade out
		case 0x6: // Fade in
			// These commands have 1 data byte.
			info.basic.param1 = *(_position._playPos++);
			break;
		default:
			info.noop = true;
			break;
		}
		break;

	case 0xE: // Set loop point
		// This command does not have any data bytes.
		break;

	case 0xF: // Return
		// This command does not have any data bytes.
		info.loop = true;
		break;

	default:
		info.noop = true;
		break;
	}
}

void MidiParser_M::allNotesOff() {
	if (_driver) {
		_driver->stopAllNotes();
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
