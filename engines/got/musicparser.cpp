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

#include "got/musicparser.h"

#include "common/endian.h"
#include "common/textconsole.h"

namespace Got {

MusicParser_Got::MusicParser_Got() :
	_pause(false),
	_abortParse(false),
	_currentDelta(0),
	_driver(nullptr),
	_track(nullptr),
	_trackLength(0),
	_playPos(nullptr),
	_loopStart(-1) { }

MusicParser_Got::~MusicParser_Got() {
	stopPlaying();
}

void MusicParser_Got::sendToDriver(uint16 b) {
	if (_driver == nullptr)
		return;

	_driver->send(b);
}

void MusicParser_Got::sendToDriver(byte reg, byte value) {
	// OPL register and value are sent as a uint16 to the driver.
	sendToDriver((reg << 8) | value);
}

void MusicParser_Got::allNotesOff() {
	if (_driver == nullptr)
		return;

	_driver->stopAllNotes();
}

void MusicParser_Got::resetTracking() {
	_playPos = nullptr;
}

bool MusicParser_Got::startPlaying() {
	if (_track == nullptr || _pause)
		return false;

	if (_playPos == nullptr) {
		_playPos = _track;
	}

	return true;
}

void MusicParser_Got::pausePlaying() {
	if (isPlaying() && !_pause) {
		_pause = true;
		allNotesOff();
	}
}

void MusicParser_Got::resumePlaying() {
	_pause = false;
}

void MusicParser_Got::stopPlaying() {
	if (isPlaying())
		allNotesOff();
	resetTracking();
	_pause = false;
}

bool MusicParser_Got::isPlaying() {
	return _playPos != nullptr;
}

void MusicParser_Got::setMusicDriver(MusicDriver_Got *driver) {
	_driver = driver;
}

bool MusicParser_Got::loadMusic(byte* data, uint32 size) {
	assert(size >= 5);

	// Data starts with a word defining the loop point.
	uint16 loopHeader = READ_LE_UINT16(data);
	data += 2;

	// Rest of the data is music events.
	_track = data;
	_trackLength = size - 2;

	if (loopHeader == 0) {
		// No loop point defined.
		_loopStart = -1;
	} else {
		// Loop point is defined as the number of words from the start of the
		// data (including the header).
		_loopStart = (loopHeader - 1) * 2;
		if ((uint32)_loopStart >= _trackLength) {
			warning("MusicParser_Got::loadMusic - Music data has loop start point after track end");
			_loopStart = -1;
		}
	}

	return true;
}

void MusicParser_Got::unloadMusic() {
	if (_track == nullptr)
		// No music data loaded
		return;

	stopPlaying();
	_abortParse = true;
	_track = nullptr;
}

void MusicParser_Got::onTimer() {
	if (_playPos == nullptr || !_driver || _pause)
		return;

	if (_currentDelta > 0) {
		_currentDelta--;
		if (_currentDelta > 0)
			// More ticks to go to the next event.
			return;
	}

	_abortParse = false;
	while (!_abortParse && isPlaying() && _currentDelta == 0) {
		assert((_playPos + 3) < (_track + _trackLength));

		// An event consists of a delta, followed by an OPL register / value pair.

		// A delta consists of 1 or 2 bytes. Bit 7 in the first byte indicates
		// if a second byte is used.
		uint16 newDelta = *_playPos++;
		if ((newDelta & 0x80) > 0) {
			assert((_playPos + 3) < (_track + _trackLength));
			newDelta &= 0x7F;
			newDelta <<= 8;
			newDelta |= *_playPos++;
		}
		// The delta is the number of ticks from this event to the next event,
		// not the number of ticks preceding this event.
		_currentDelta = newDelta;

		// Next, read the OPL register / value pair.
		byte oplRegister = *_playPos++;
		byte oplRegValue = *_playPos++;

		if (oplRegister == 0 && oplRegValue == 0) {
			// End of track is indicated by an event with OPL register and value 0.
			if (_loopStart >= 0) {
				// Continue playback at the loop point.
				_playPos = _track + _loopStart;
			}
			else {
				stopPlaying();
			}
		}
		else {
			// Write the specified OPL register value.
			sendToDriver(oplRegister, oplRegValue);
		}
	}
}

void MusicParser_Got::timerCallback(void *data) {
	((MusicParser_Got *)data)->onTimer();
}

} // namespace Got
