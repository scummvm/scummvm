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

#include "common/md5.h"
#include "xeen/music.h"
#include "xeen/xeen.h"
#include "xeen/files.h"

namespace Xeen {

#define CALLBACKS_PER_SECOND 72

Music::Music(Audio::Mixer *mixer) : _mixer(mixer), _effectsData(nullptr),
		_musicPtr1(nullptr), _musicPtr2(nullptr), _dataPtr(nullptr),
		_lowMusicIgnored(false),
		_fieldF(false), _field1C(false), _field1E(false), _field109(0),
		_field10B(0), _field114(0), _field115(0), _field116(0), _field117(0) {
	_channels.resize(ADLIB_CHANNEL_COUNT);
	Common::fill(&_fieldFB[0], &_fieldFB[7], 0);
	Common::fill(&_field10D[0], &_field10D[7], 0);

	_mixer = mixer;
	_opl = OPL::Config::create();
	_opl->init();
	_opl->start(new Common::Functor0Mem<void, Music>(this, &Music::onTimer), CALLBACKS_PER_SECOND);
	initialize();

	loadEffectsData();
}

Music::~Music() {
	_opl->stop();
	delete _opl;
	delete[] _effectsData;
}

void Music::initialize() {
	write(1, 0x20);
	write(8, 0);
	write(0xBD, 0);

	resetFrequencies();
	reset();
}

void Music::loadEffectsData() {
	File file("admus");
	Common::String md5str = Common::computeStreamMD5AsString(file, 8192);
	
	if (md5str != "be8989a5e868913f0e53963046e3ea13")
		error("Unknown music driver encountered");

	// Load in the driver data
	const int EFFECTS_OFFSET = 0x91D;
	byte *effectsData = new byte[file.size() - EFFECTS_OFFSET];
	file.seek(EFFECTS_OFFSET);
	file.read(effectsData, file.size() - EFFECTS_OFFSET);
	file.close();
	_effectsData = effectsData;

	// Extract the effects offsets
	_effectsOffsets.resize(180);
	for (int idx = 0; idx < 180; ++idx)
		_effectsOffsets[idx] = READ_LE_UINT16(&effectsData[idx * 2]) - EFFECTS_OFFSET;
}

void Music::onTimer() {
	Common::StackLock slock(_driverMutex);
	update();
	flush();
}

void Music::write(int reg, int val) {
	_queue.push(RegisterValue(reg, val));
}

void Music::flush() {
	Common::StackLock slock(_driverMutex);

	while (!_queue.empty()) {
		RegisterValue v = _queue.pop();
		_opl->writeReg(v._regNum, v._value);
	}
}

void Music::update() {
	const byte *srcP = _dataPtr;

	bool flag = !_field1E;
	if (!flag) {
		_field1C = 0;
		if (_field116 && --_field116 == 0)
			flag = true;
	}
	if (flag && _lowMusicIgnored) {
		srcP = _musicPtr1;
		_field1C = 1;
		if (!_field117 || --_field117 == 0)
			flag = false;
	}

	if (flag) {
		postProcess();
		return;
	}

	// Main loop
	bool breakFlag = false;
	while (!breakFlag) {
		byte nextByte = *srcP++;
		int cmd = (nextByte >> 3) & 15;

		CommandFn fn = (_field1C == 1) ? COMMAND_TABLE2[cmd] : COMMAND_TABLE1[cmd];
		breakFlag = (this->*fn)(srcP, nextByte);
	}
}

void Music::playEffect(uint effectId) {
	if (!_lowMusicIgnored || effectId < 7 || effectId >= 11) {
		if (effectId < _effectsOffsets.size()) {
			_musicPtr1 = _musicPtr2 = &_effectsData[_effectsOffsets[effectId]];
			_field117 = 0;
			_field115 = 0;
			_field114 = 0;
			reset();
			_lowMusicIgnored = true;
		}
	}
}

void Music::reset() {
	if (!_fieldF) {
		_field109 = 0;
		setFrequency(7, 0);
		_channels[7]._outputLevel = 63;
		setOutputLevel(7, 63);
	}

	_field10B = 0;
	setFrequency(8, 0);
	_channels[8]._outputLevel = 63;
	setOutputLevel(8, 63);
}

void Music::resetFrequencies() {
	for (int opNum = 6; opNum >= 0; --opNum) {
		_fieldFB[opNum] = 0;
		setFrequency(opNum, 0);
	}
}

void Music::setFrequency(byte operatorNum, uint frequency) {
	write(0xA0 + operatorNum, frequency & 0xff);
	write(0xB0 + operatorNum, (frequency >> 8));
}

void Music::postProcess() {
	// TODO
}

bool Music::cmd1(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd2(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd3(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd4(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd5(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd6(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd7(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd8(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd9(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd10(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd11(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd12(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd13(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd14(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd15(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd16(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd17(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd18(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd19(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd20(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd21(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd22(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd23(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

bool Music::cmd24(const byte *&srcP, byte nextByte) {
	return false;  // TODO
}

const CommandFn Music::COMMAND_TABLE1[16] = {
	&Music::cmd1,  &Music::cmd2,  &Music::cmd3,  &Music::cmd4,
	&Music::cmd5,  &Music::cmd5,  &Music::cmd6,  &Music::cmd4,
	&Music::cmd7,  &Music::cmd8,  &Music::cmd9,  &Music::cmd10,
	&Music::cmd11, &Music::cmd12, &Music::cmd13, &Music::cmd14
};

const CommandFn Music::COMMAND_TABLE2[16] = {
	&Music::cmd15, &Music::cmd16, &Music::cmd17, &Music::cmd18,
	&Music::cmd4,  &Music::cmd4,  &Music::cmd19, &Music::cmd20,
	&Music::cmd21, &Music::cmd22, &Music::cmd4,  &Music::cmd4,
	&Music::cmd23, &Music::cmd12, &Music::cmd13, &Music::cmd24
};

void Music::setOutputLevel(byte channelNum, uint level) {
	write(0x40 + OPERATOR2_INDEXES[channelNum], level |
		(_channels[channelNum]._scalingValue & 0xC0));
}

const byte Music::OPERATOR1_INDEXES[ADLIB_CHANNEL_COUNT] = {
	0, 1, 2, 8, 9, 0xA, 0x10, 0x11, 0x12
};

const byte Music::OPERATOR2_INDEXES[ADLIB_CHANNEL_COUNT] = {
	3, 4, 5, 0xB, 0xC, 0xD, 0x13, 0x14, 0x15
};

} // End of namespace Xeen
