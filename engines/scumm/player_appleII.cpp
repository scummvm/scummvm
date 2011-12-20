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
 */

#include <assert.h>
#include "engines/engine.h"
#include "scumm/player_appleII.h"
#include "scumm/scumm.h"

namespace Scumm {

Player_AppleII::Player_AppleII(ScummEngine *scumm, Audio::Mixer *mixer) {
	_mixer = mixer;
	_vm = scumm;
	
	resetState();

	setSampleRate(_mixer->getOutputRate());

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_AppleII::~Player_AppleII() {
	_mixer->stopHandle(_soundHandle);
}

/*
void logSounds() {
	for (nr = 1; nr < 70; ++nr) {
		switch (nr) {
		// non-existing or invalid
		case 3: case 4: case 5:  case 49: case 51: case 68:
			continue;
		}

		byte *data = _vm->getResourceAddress(rtSound, nr);
		if (data) {
			size_t size = (data[1] << 8) | data[0];

			std::stringstream s;
			s << "d:/msound/sound" << nr << ".snd";
			FILE *f = fopen(s.str().c_str(), "wb");
			fwrite(data, 1, size, f);
			fclose(f);
			printf("sound: %d\n", nr);
		}
	}
}
*/

void Player_AppleII::startSound(int nr) {
	Common::StackLock lock(_mutex);

	resetState();

	byte *data = _vm->getResourceAddress(rtSound, nr);
	assert(data);
	byte *ptr1 = data + 4;

	_state.type = ptr1[0];
	if (_state.type == 0) {
		// nothing to play
		resetState();
		return;
	}

	_state.soundNr = nr;
	_state.finished = false;
	initFuncState();

	_state.loop = ptr1[1];
	assert(_state.loop > 0);

	_state.params = &ptr1[2];

	debug(4, "startSound %d: type %d, loop %d",
		  nr, _state.type, _state.loop);
}

void Player_AppleII::initFuncState() {
	switch (_state.type) {
		case 2:	case 3:
			_state.func23.pos = 1;
			break;
		case 4:
			_state.func4.updateRemain1 = 80;
			_state.func4.updateRemain2 = 10;
			break;
		case 5:
			_state.func5.index = 0;
			break;
	}
}

bool Player_AppleII::updateSound() {
	if (!_state.soundNr || _state.finished)
		return false;

	bool done = false;
	switch (_state.type) {
		case 1: // freq up/down
			done = soundFunc1();
			break;
		case 2: // symmetric wave (~)
			done = soundFunc2();
			break;
		case 3: // asymmetric wave (__-)
			done = soundFunc3();
			break;
		case 4: // polyphone (2 voices)
			done = soundFunc4();
			break;
		case 5:	// periodic noise
			done = soundFunc5();
			break;
	}

	if (done) {
		--_state.loop;
		if (_state.loop <= 0) {
			_state.finished = true;
		} else {
			// reset function state on each loop
			initFuncState();
		}
	}

	return true;
}

void Player_AppleII::resetState() {
	_state.soundNr = 0;
	_state.type = 0;
	_state.loop = 0;
	_state.params = NULL;
	_state.localParams = NULL;
	_state.speakerState = 0;
	_state.finished = true;

	_sampleConverter.reset();
}

void Player_AppleII::stopAllSounds() {
	Common::StackLock lock(_mutex);
	resetState();
}

void Player_AppleII::stopSound(int nr) {
	Common::StackLock lock(_mutex);
	if (_state.soundNr == nr) {
		resetState();
	}
}

int Player_AppleII::getSoundStatus(int nr) const {
	Common::StackLock lock(_mutex);
	return (_state.soundNr == nr);
}

int Player_AppleII::getMusicTimer() {
	/* Apple-II sounds are synchronous -> no music timer */
	return 0;
}

int Player_AppleII::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	if (!_state.soundNr)
		return 0;

	int samplesLeft = numSamples;
	do {
		int nSamplesRead = _sampleConverter.readSamples(buffer, samplesLeft);
		samplesLeft -= nSamplesRead;
		buffer += nSamplesRead;
	} while ((samplesLeft > 0) && updateSound());

	// reset state if sound is played completely
	if (_state.finished && (_sampleConverter.availableSize() == 0))
		resetState();

	return numSamples - samplesLeft;
}

/************************************
 * Apple-II sound-resource parser
 ************************************/

// toggle speaker on/off
void Player_AppleII::speakerToggle() {
	_state.speakerState ^= 0x1; 
}

void Player_AppleII::generateSamples(int cycles) {
	_sampleConverter.addCycles(_state.speakerState, cycles);
}

void Player_AppleII::wait(int interval, int count /*y*/) {
	assert(count > 0); // 0 == 256? 
	assert(interval > 0); // 0 == 256? 
	generateSamples(11 + count*(8 + 5 * interval));
}

void Player_AppleII::_soundFunc1(int interval /*a*/, int count /*y*/) { // D076
	assert(interval > 0); // 0 == 256? 
	assert(count > 0); // 0 == 256? 

	for (; count >= 0; --count) {
		speakerToggle();
		generateSamples(17 + 5 * interval);
	}
}

bool Player_AppleII::soundFunc1() { // D085
	const int delta = _state.params[0];
	const int count = _state.params[1];
	byte interval = _state.params[2]; // must be byte ("interval < delta" possible)
	const int limit = _state.params[3];
	const bool decInterval = (_state.params[4] >= 0x40);

	if (decInterval) {
		do {
			_soundFunc1(interval, count);
			interval -= delta;
		} while (interval >= limit);
	} else {
		do {
			_soundFunc1(interval, count);
			interval += delta;
		} while (interval < limit);
	}

	return true;
}

void Player_AppleII::_soundFunc2(int interval /*a*/, int count) { // D0EF
	if (interval == 0xFE) {
		wait(interval, 10);
	} else {
		assert(count > 0); // 0 == 256?
		assert(interval > 0); // 0 == 256?

		int a = (interval >> 3) + count;
		for (int y = a; y > 0; --y) { 
			generateSamples(1292 - 5*interval);
			speakerToggle();

			generateSamples(1287 - 5*interval);
			speakerToggle();
		}
	}
}

bool Player_AppleII::soundFunc2() { // D0D6
	// while (pos = 1; pos < 256; ++pos)
	if (_state.func23.pos < 256) {
		byte interval = _state.params[_state.func23.pos];
		if (interval == 0xFF)
			return true;
		_soundFunc2(interval, _state.params[0] /*, LD12F=interval*/);

		++_state.func23.pos;
		return false;
	}	
	return true;
}

void Player_AppleII::_soundFunc3(int interval /*a*/, int count /*LD12D*/) { // D14B
	if (interval == 0xFE) {
		wait(interval, 70);
	} else {
		assert(interval > 0); // 0 == 256?
		assert(count > 0); // 0 == 256?

		for (int y = count; y > 0; --y) {
			generateSamples(1289 - 5*interval);
			speakerToggle();
		}
	}
}

bool Player_AppleII::soundFunc3() { // D132
	// while (pos = 1; pos < 256; ++pos)
	if (_state.func23.pos < 256) {
		byte interval = _state.params[_state.func23.pos];
		if (interval == 0xFF)
			return true;
		_soundFunc3(interval, _state.params[0]);

		++_state.func23.pos;
		return false;
	}
	return true;
}

void Player_AppleII::_soundFunc4(byte param0, byte param1, byte param2) { // D1A2
	uint16 count = (-param2 << 8) | 0x3;
	byte bitmask1 = 0x3;
	byte bitmask2 = 0x3;
	
	byte updateInterval2 = param0;
	if (updateInterval2 == 0)
		bitmask2 = 0x0;

	byte updateInterval1 = param1;
	if (updateInterval1 == 0) {
		bitmask1 = 0x0;
		if (bitmask2 != 0) {
			bitmask1 = bitmask2;
			bitmask2 = 0;
			updateInterval1 = updateInterval2;
		}
	}

	byte speakerShiftReg = 0;
	byte updateRemain1 = _state.func4.updateRemain1;
	byte updateRemain2 = _state.func4.updateRemain2;

	while (true) {
		--updateRemain1;
		--updateRemain2;

		if (updateRemain2 == 0) {
			updateRemain2 = updateInterval2;
			// use only first voice's data (bitmask1) if both voices are triggered 
			if (updateRemain1 != 0) {
				speakerShiftReg ^= bitmask2;		
			}
		}

		if (updateRemain1 == 0) {
			updateRemain1 = updateInterval1;
			speakerShiftReg ^= bitmask1;
		}

		if (speakerShiftReg & 0x1)
			speakerToggle();
		speakerShiftReg >>= 1;
		generateSamples(42); /* actually 42.5 */

		++count;
		if (count == 0) {
			_state.func4.updateRemain1 = updateRemain1;
			_state.func4.updateRemain2 = updateRemain2;
			return;
		}
	}
}

bool Player_AppleII::soundFunc4() { // D170
	if (!_state.localParams)
		_state.localParams = _state.params;

	// while (_state.params[0] != 0x01)
	if (_state.localParams[0] != 0x01) {
		_soundFunc4(_state.localParams[0], _state.localParams[1], _state.localParams[2]);
		_state.localParams += 3;
		return false;
	}

	_state.localParams = NULL;
	return true;
}

void Player_AppleII::_soundFunc5(int interval /*a*/, int count) { // D270
	assert(count > 0); // 0 == 256?
	if (interval == 0)
		interval = 256;

	for (int i = count; i > 0; --i) {
		generateSamples(10 + 5*interval);
		speakerToggle();
		
		generateSamples(5 + 5*interval);
		speakerToggle();
	}
}

// LD000[loc] ^ LD00A[loc]
static const byte noiseTable[256] = {
	0x65, 0x1b, 0xda, 0x11, 0x61, 0xe5, 0x77, 0x57, 0x92, 0xc8, 0x51, 0x1c, 0xd4, 0x91, 0x62, 0x63, 
	0x00, 0x38, 0x57, 0xd5, 0x18, 0xd8, 0xdc, 0x40, 0x03, 0x86, 0xd3, 0x2f, 0x10, 0x11, 0xd8, 0x3c, 
	0xbe, 0x00, 0x19, 0xc5, 0xd2, 0xc3, 0xca, 0x34, 0x00, 0x28, 0xbf, 0xb9, 0x18, 0x20, 0x01, 0xcc, 
	0xda, 0x08, 0xbc, 0x75, 0x7c, 0xb0, 0x8d, 0xe0, 0x09, 0x18, 0xbf, 0x5d, 0xe9, 0x8c, 0x75, 0x64, 
	0xe5, 0xb5, 0x5d, 0xe0, 0xb7, 0x7d, 0xe9, 0x8c, 0x55, 0x65, 0xc5, 0xb5, 0x5d, 0xd8, 0x09, 0x0d,
	0x64, 0xf0, 0xf0, 0x08, 0x63, 0x03, 0x00, 0x55, 0x35, 0xc0, 0x00, 0x20, 0x74, 0xa5, 0x1e, 0xe3, 
	0x00, 0x06, 0x3c, 0x52, 0xd1, 0x70, 0xd0, 0x57, 0x02, 0xf0, 0x00, 0xb6, 0xfc, 0x02, 0x11, 0x9a, 
	0x3b, 0xc8, 0x38, 0xdf, 0x1a, 0xb0, 0xd1, 0xb8, 0xd0, 0x18, 0x8a, 0x4a, 0xea, 0x1b, 0x12, 0x5d, 
	0x29, 0x58, 0xd8, 0x43, 0xb8, 0x2d, 0xd2, 0x61, 0x10, 0x3c, 0x0c, 0x5d, 0x1b, 0x61, 0x10, 0x3c, 
	0x0a, 0x5d, 0x1d, 0x61, 0x10, 0x3c, 0x0b, 0x19, 0x88, 0x21, 0xc0, 0x21, 0x07, 0x00, 0x65, 0x62,
	0x08, 0xe9, 0x36, 0x40, 0x20, 0x41, 0x06, 0x00, 0x20, 0x00, 0x00, 0xed, 0xa3, 0x00, 0x88, 0x06, 
	0x98, 0x01, 0x5d, 0x7f, 0x02, 0x1d, 0x78, 0x03, 0x60, 0xcb, 0x3a, 0x01, 0xbd, 0x78, 0x02, 0x5d, 
	0x7e, 0x03, 0x1d, 0xf5, 0xa6, 0x40, 0x81, 0xb4, 0xd0, 0x8d, 0xd3, 0xd0, 0x6d, 0xd5, 0x61, 0x48, 
	0x61, 0x4d, 0xd1, 0xc8, 0xb1, 0xd8, 0x69, 0xff, 0x61, 0xd9, 0xed, 0xa0, 0xfe, 0x19, 0x91, 0x37, 
	0x19, 0x37, 0x00, 0xf1, 0x00, 0x01, 0x1f, 0x00, 0xad, 0xc1, 0x01, 0x01, 0x2e, 0x00, 0x40, 0xc6,
	0x7a, 0x9b, 0x95, 0x43, 0xfc, 0x18, 0xd2, 0x9e, 0x2a, 0x5a, 0x4b, 0x2a, 0xb6, 0x87, 0x30, 0x6c
};

byte /*a*/ Player_AppleII::noise() { // D261
	static int pos = 0; // initial value?
	byte result = noiseTable[pos];
	pos = (pos + 1) % 256;
	return result;
}

bool Player_AppleII::soundFunc5() { // D222
	const byte noiseMask[] = {
		0x3F, 0x3F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F
	};

	int param0 = _state.params[0];
	assert(param0 > 0);

	// while (i = 0; i < 10; ++i)
	if (_state.func5.index < 10) {
		int count = param0;
		do {
			_soundFunc5(noise() & noiseMask[_state.func5.index], 1);
			--count;
		} while (count > 0);

		++_state.func5.index;
		return false;
	}

	return true;
}

} // End of namespace Scumm
