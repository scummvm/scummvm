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

// CPU_CLOCK according to AppleWin
static const double CPU_CLOCK = 1020484.5; // ~ 1.02 MHz

namespace Scumm {

Player_AppleII::Player_AppleII(ScummEngine *scumm, Audio::Mixer *mixer) {
	_speakerState = 0;
	_soundNr = 0;

	_mixer = mixer;
	_sampleRate = _mixer->getOutputRate();
	_vm = scumm;

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

	_soundNr = nr;
	_buffer.clear();

	byte *data = _vm->getResourceAddress(rtSound, nr);
	assert(data);

	byte *ptr1 = data + 4;

	int type = ptr1[0];
	if (type == 0)
		return;
	int loop = ptr1[1];
	assert(loop > 0);
	ptr1 += 2;

	debug(4, "startSound %d: type %d, loop %d",
		  nr, type, loop);
	
	do {
		switch (type) {
			case 1: // freq up/down
				soundFunc1(ptr1);
				break;
			case 2: // symmetric wave (~)
				soundFunc2(ptr1);
				break;
			case 3: // asymmetric wave (__-)
				soundFunc3(ptr1);
				break;
			case 4: // polyphone (2 voices)
				soundFunc4(ptr1);
				break;
			case 5:	// periodic noise
				soundFunc5(ptr1);
				break;
		}
		--loop;
	} while (loop > 0);
}

void Player_AppleII::stopAllSounds() {
	Common::StackLock lock(_mutex);
	_buffer.clear();
}

void Player_AppleII::stopSound(int nr) {
	Common::StackLock lock(_mutex);
	if (_soundNr == nr) {
		_buffer.clear();
	}
}

int Player_AppleII::getSoundStatus(int nr) const {
	Common::StackLock lock(_mutex);
	return (_buffer.availableSize() > 0 ? 1 : 0);
}

int Player_AppleII::getMusicTimer() {
	/* Apple-II sounds are synchronous -> no music timer */
	return 0;
}

int Player_AppleII::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	return _buffer.read((byte*)buffer, numSamples * 2) / 2;
}

/************************************
 * Apple-II sound-resource parser
 ************************************/

// toggle speaker on/off
void Player_AppleII::speakerToggle() {
	_speakerState ^= 0x1; 
}

void Player_AppleII::generateSamples(int cycles) {
	// sampleDiff is used to compensate fractional samples
	static double sampleDiff = 0;
	double fSamples = (double)cycles / CPU_CLOCK * _sampleRate + sampleDiff;
	int samples = (int)(fSamples + 0.5);
	sampleDiff = fSamples - samples;

	float vol = (float)_maxvol / 255;
	int16 value = vol * (_speakerState ? 32767 :  -32767);
	for (int i = 0; i < samples; ++i)
		_buffer.write(&value, sizeof(value));
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

void Player_AppleII::soundFunc1(const byte *params) { // D085
	int delta = params[0];
	int count = params[1];
	byte interval = params[2]; // must be byte ("interval < delta" possible)
	int limit = params[3];
	bool decInterval = (params[4] >= 0x40);

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

void Player_AppleII::soundFunc2(const byte *params) { // D0D6
	for (int pos = 1; pos < 256; ++pos) {
		byte interval = params[pos];
		if (interval == 0xFF)
			return;
		_soundFunc2(interval, params[0] /*, LD12F=interval*/);
	}
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

void Player_AppleII::soundFunc3(const byte *params) { // D132
	for (int pos = 1; pos < 256; ++pos) {
		byte interval = params[pos];
		if (interval == 0xFF)
			return;
		_soundFunc3(interval, params[0]);
	}
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
	static byte updateRemain1 = 80;
	static byte updateRemain2 = 10;

	while (true) {
		--updateRemain1;
		if (updateRemain1 == 0) {
			updateRemain1 = updateInterval1;
			speakerShiftReg ^= bitmask1;
		}

		--updateRemain2;
		if (updateRemain2 == 0) {
			updateRemain2 = updateInterval2;
			if (updateRemain1 != 0) {
				speakerShiftReg ^= bitmask2;		
			}
		}

		if (speakerShiftReg & 0x1)
			speakerToggle();
		speakerShiftReg >>= 1;
		generateSamples(40);

		++count;
		if (count == 0) {
			return;
		}
	}
}

void Player_AppleII::soundFunc4(const byte *params) { // D170
	while (params[0] != 0x01) {
		_soundFunc4(params[0], params[1], params[2]);
		params += 3;
	}
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

void Player_AppleII::soundFunc5(const byte *params) { // D222
	const byte noiseMask[] = {
		0x3F, 0x3F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F
	};

	int param0 = params[0];
	assert(param0 > 0);
	for (int i = 0; i < 10; ++i) {
		int count = param0;
		do {
			_soundFunc5(noise() & noiseMask[i], 1);
			--count;
		} while (count > 0);
	}
}

} // End of namespace Scumm
