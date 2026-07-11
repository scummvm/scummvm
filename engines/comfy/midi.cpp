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

#include "comfy/comfy.h"
#include "comfy/midiplyr/midiplyr.h"

#include "common/ptr.h"

namespace Comfy {

uint16 ComfyEngine::midiTick() {
	if (!_midiPlyrDriver)
		return 0;

	uint32 previousCounter = _midiInstanceEventTime;
	if (_engineVersion == kEngineVersion3) {
		int16 adjustment = midiGetCounterAdjustment();
		if (adjustment) {
			_midiPlyrDriver->setVocCounter(uint32(int32(_midiPlyrDriver->getVocCounter()) + adjustment));
			_midiPlyrDriver->setTimeCounter(uint32(int32(_midiPlyrDriver->getTimeCounter()) + adjustment));
		}
	}

	do {
		_midiTimeCounter = _midiPlyrDriver->getTimeCounter();
		if ((_engineVersion == kEngineVersion3 && _midiTimeCounter <= previousCounter) ||
				(_engineVersion != kEngineVersion3 && _midiTimeCounter == previousCounter))
			_system->delayMillis(1);
	} while ((_engineVersion == kEngineVersion3 && _midiTimeCounter <= previousCounter) ||
		(_engineVersion != kEngineVersion3 && _midiTimeCounter == previousCounter));

	_midiInstanceEventTime = _midiTimeCounter;
	_midiEventBaseTime = int32(_midiInstanceEventTime);

	int16 delta = int16(uint16(_midiInstanceEventTime - previousCounter));
	if (delta < 1)
		delta = 1;

	if (_midiTimeScale != 0x400) {
		_midiTimeDelta = int16((int32(delta) * _midiTimeScale) >> 10);
		if (!_midiTimeDelta)
			_midiTimeDelta = 1;

		if (_midiTimeDelta != delta) {
			_midiTimeCounter = uint32(int32(_midiTimeCounter) + _midiTimeDelta - delta);
			_midiPlyrDriver->setTimeCounter(_midiTimeCounter);

			_midiInstanceEventTime = uint32(int32(_midiInstanceEventTime) + _midiTimeDelta - delta);
			_midiEventBaseTime = int32(_midiInstanceEventTime);
			delta = _midiTimeDelta;
		}
	}

	return uint16(delta);
}

bool ComfyEngine::midiPlyrStart() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(pathFOpen(Common::Path("MIDIFILE.DAT"), true));
	if (!stream)
		return false;

	byte signature[2];
	if (stream->read(signature, sizeof(signature)) != sizeof(signature))
		return false;

	byte flags;
	if (signature[0] == 'C' && signature[1] == 'M')
		flags = 0;
	else if (signature[0] == 'C' && signature[1] == 'm')
		flags = 1;
	else
		return false;

	midiPlyrStop();
	MidiPlyrDriver::DriverVersion version = _engineVersion == kEngineVersion3 ?
		MidiPlyrDriver::kDriverVersion2 : MidiPlyrDriver::kDriverVersion1;
	_midiPlyrDriver = new MidiPlyrDriver(version);
	if (!_midiPlyrDriver->musicStart(flags)) {
		delete _midiPlyrDriver;
		_midiPlyrDriver = nullptr;
		return false;
	}

	_midiTimeCounter = 0;
	_midiInstanceEventTime = 0;
	_midiPlyrDriver->setTimeCounter(0);
	return true;
}

void ComfyEngine::midiPlyrStop() {
	if (!_midiPlyrDriver)
		return;

	_midiPlyrDriver->musicEnd();
	delete _midiPlyrDriver;
	_midiPlyrDriver = nullptr;
}

int16 ComfyEngine::midiGetCounterAdjustment() {
	if (!_midiCounterAdjustment)
		return 0;

	int16 adjustment = _midiCounterAdjustment > 0 ? 1 : -1;
	int16 magnitude = _midiCounterAdjustment < 0 ? -_midiCounterAdjustment : _midiCounterAdjustment;
	if (magnitude > 5)
		adjustment *= 2;

	_midiCounterAdjustment -= adjustment;
	return adjustment;
}

void ComfyEngine::midiSetTimeScale(int16 delta) {
	if (!delta) {
		_midiTimeScale = 0x400;
		return;
	}

	_midiTimeScale = (int32(delta) * _midiTimeScale) >> 10;
	if (_midiTimeScale < 100)
		_midiTimeScale = 100;

	if (_midiTimeScale > 10000)
		_midiTimeScale = 10000;
}

} // End of namespace Comfy
