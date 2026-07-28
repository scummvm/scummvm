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

#include "mads/nebular/sound/rsound_nebular.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

const RSound1::CommandPtr RSound1::_commandList[42] = {
	&RSound1::command0, &RSound1::command1, &RSound1::command2, &RSound1::command3,
	&RSound1::command4, &RSound1::command5, &RSound1::command6, &RSound1::command7,
	&RSound1::command8, &RSound1::command9, &RSound1::command10, &RSound1::command11,
	&RSound1::command12, &RSound1::command13, &RSound1::command14, &RSound1::command15,
	&RSound1::command16, &RSound1::command17, &RSound1::command18, &RSound1::command19,
	&RSound1::command20, &RSound1::command21, &RSound1::command22, &RSound1::command23,
	&RSound1::command24, &RSound1::command25, &RSound1::command26, &RSound1::command27,
	&RSound1::command28, &RSound1::command29, &RSound1::command30, &RSound1::command31,
	&RSound1::command32, &RSound1::command33, &RSound1::command34, &RSound1::command35,
	&RSound1::command36, &RSound1::command37, &RSound1::command38, &RSound1::command39,
	&RSound1::command40, &RSound1::command41
};

RSound1::RSound1(Audio::Mixer *mixer) : RSound(mixer, "rsound.001", 0x1350, 0x1A90, 0x67) {
}

int RSound1::command(int commandId, int param) {
	if (commandId > 41)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound1::clampParam() {
	return (_commandParam > 0x40) ? _commandParam - 0x40 : 0;
}

void RSound1::method1() {
	byte *pData = loadData(0x1166);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x13BC));
		_channels[2].load(loadData(0x155C));
		_channels[3].load(loadData(0x15D8));
	}
}

int RSound1::command9() {
	playSound(0xAD4);
	return 0;
}

int RSound1::command10() {
	byte *pData = loadData(0xCE4);
	if (!isSoundActive(pData)) {
		_channels[0].load(pData);
		_channels[1].load(loadData(0xD18));
		_channels[2].load(loadData(0xE9C));
		_channels[3].load(loadData(0xEE8));
	}
	return 0;
}

int RSound1::command11() {
	method1();
	_channels[0]._volume = 0;
	sendVolume(1, _channels[0]._volume);
	_channels[1]._volume = 0;
	sendVolume(2, _channels[1]._volume);
	return 0;
}

int RSound1::command12() {
	method1();
	_channels[0]._volume = 80;
	sendVolume(1, _channels[0]._volume);
	_channels[1]._volume = 0;
	sendVolume(2, _channels[1]._volume);
	return 0;
}

int RSound1::command13() {
	method1();
	_channels[0]._volume = 80;
	sendVolume(1, _channels[0]._volume);
	_channels[1]._volume = 80;
	sendVolume(2, _channels[1]._volume);
	return 0;
}

int RSound1::command14() {
	playSound(0x16C2);
	return 0;
}

int RSound1::command15() {
	byte *pData = loadData(0xF3A);
	if (!isSoundActive(pData)) {
		command1();
		_channels[4].load(pData);
		_channels[5].load(loadData(0x102A));
		_channels[6].load(loadData(0x110E));
	}
	return 0;
}

int RSound1::command16() {
	playSound(0xADE);
	return 0;
}

int RSound1::command17() {
	playSound(0xAE8);
	return 0;
}

int RSound1::command18() {
	playSound(0xAF2);
	return 0;
}

int RSound1::command19() {
	command1();
	playSound(0xB04);
	return 0;
}

int RSound1::command20() {
	playSound(0xB5E);
	return 0;
}

int RSound1::command21() {
	playSound(0xB4C);
	return 0;
}

int RSound1::command22() {
	playSound(0xB6E);
	return 0;
}

int RSound1::command23() {
	byte *pData = loadData(0xB7A);
	pData[6] ^= 0x1F;
	playSound(0xB7A);
	return 0;
}

int RSound1::command24() {
	playSound(0xB84);
	return 0;
}

int RSound1::command25() {
	playSound(0xB8E);
	return 0;
}

int RSound1::command26() {
	byte *pData = loadData(0xCCA);
	int v1 = (getRandomNumber() & 24) + 45;
	pData[8] = v1;
	int v2 = clampParam() + 64;
	pData[5] = v2;
	_channels[7].load(pData);
	return 0;
}

int RSound1::command27() {
	byte *pData = loadData(0xCBE);
	int v1 = (getRandomNumber() & 24) + 45;
	pData[8] = v1;
	int v2 = clampParam() + 64;
	pData[5] = v2;
	_channels[7].load(pData);
	return 0;
}

int RSound1::command28() {
	playSound(0xB9E);
	return 0;
}

int RSound1::command29() {
	byte *pData = loadData(0xC6C);
	int v = (clampParam() >> 1) + 32;
	pData[0xB] = v;
	if (!isSoundActive(pData))
		playSound(0xC6C);
	return 0;
}

int RSound1::command30() {
	byte *pData = loadData(0xC80);
	int v = clampParam() + 63;
	pData[0xB] = v;
	if (!isSoundActive(pData))
		playSoundAny(0xC80);
	return 0;
}

int RSound1::command31() {
	playSound(0xBBE);
	return 0;
}

int RSound1::command32() {
	byte *pData = loadData(0xC96);
	int half = clampParam() >> 1;
	pData[0xB] = pData[0x17] = half + 68;
	pData[0x11] = pData[0x1D] = half + 20;
	if (!isSoundActive(pData))
		playSoundAny(0xC96);
	return 0;
}

int RSound1::command33() {
	playSound(0xBD0);
	playSound(0xBDA);
	return 0;
}

int RSound1::command34() {
	byte *pData = loadData(0xBE8);
	int v = (getRandomNumber() & 12) + 45;
	pData[9] = v;
	pData[0x10] = v + 36;
	playSound(0xBE8);
	return 0;
}

int RSound1::command35() {
	playSound(0xBFC);
	playSound(0xC0E);
	playSound(0xC20);
	return 0;
}

int RSound1::command36() {
	playSound(0xC3E);
	return 0;
}

int RSound1::command37() {
	byte *pData = loadData(0xC4C);
	int r = getRandomNumber() & 15;
	pData[6] = r + 42;
	pData[3] = 62 - (r << 1);
	playSound(0xC4C);
	return 0;
}

int RSound1::command38() {
	playSoundAny(0xC56);
	playSound(0xC60);
	return 0;
}

int RSound1::command39() {
	byte *pData = loadData(0x1818);
	if (!isSoundActive(pData)) {
		_channels[4].load(pData);
		_channels[5].load(loadData(0x1848));
		_channels[6].load(loadData(0x1874));
		_channels[7].load(loadData(0x18B4));
		_channels[8].load(loadData(0x18CE));
	}
	return 0;
}

int RSound1::command40() {
	playSound(0xC34);
	return 0;
}

int RSound1::command41() {
	playSound(0xCD6);
	return 0;
}

/*-----------------------------------------------------------------------*/

const uint16 RSound2::_table1[16] = {
	0x3234, 0x3250, 0x326A, 0x3284, 0x329E, 0x32D6, 0x3304, 0x333C,
	0x3352, 0x3378, 0x33B6, 0x33D0, 0x33EA, 0x3404, 0x341E, 0x343E
};

const RSound2::CommandPtr RSound2::_commandList[44] = {
	&RSound2::command0, &RSound2::command1, &RSound2::command2, &RSound2::command3,
	&RSound2::command4, &RSound2::command5, &RSound2::command6, &RSound2::command7,
	&RSound2::command8, &RSound2::command9, &RSound2::command10, &RSound2::command11,
	&RSound2::command12, &RSound2::command13, &RSound2::command14, &RSound2::command15,
	&RSound2::command16, &RSound2::command17, &RSound2::command18, &RSound2::command19,
	&RSound2::command20, &RSound2::command21, &RSound2::command22, &RSound2::command23,
	&RSound2::command24, &RSound2::command25, &RSound2::command26, &RSound2::command27,
	&RSound2::command28, &RSound2::command29, &RSound2::command30, &RSound2::command31,
	&RSound2::command32, &RSound2::command33, &RSound2::command34, &RSound2::command35,
	&RSound2::command36, &RSound2::command37, &RSound2::command38, &RSound2::command39,
	&RSound2::command40, &RSound2::command41, &RSound2::command42, &RSound2::command43
};

RSound2::RSound2(Audio::Mixer *mixer) : RSound(mixer, "rsound.002", 0x1390, 0x42F0, 0x87) {
}

int RSound2::command(int commandId, int param) {
	if (commandId > 43)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound2::command5() {
	_pitchCycleCounter = 47;
	_channels[3].enable(0xFF);
	_channels[4].enable(0xFF);
	_channels[5].enable(0xFF);
	_channels[6].enable(0xFF);
	_channels[7].enable(0xFF);
	return 0;
}

int RSound2::command9() {
	byte *pData = loadData(0x103C);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x11B2));
		_channels[8].load(loadData(0x127E));
	}
	return 0;
}

int RSound2::command10() {
	byte *pData = loadData(0x132E);
	if (!isSoundActive(pData)) {
		command1();
		_channels[2].load(pData);
		_channels[8].load(loadData(0x1384));
	}
	return 0;
}

int RSound2::command11() {
	byte *pData = loadData(0x1548);
	if (!isSoundActive(pData)) {
		command1();
		_channels[2].load(pData);
		_channels[8].load(loadData(0x1648));
	}
	return 0;
}

int RSound2::command12() {
	byte *pData = loadData(0xE52);
	_pitchCycleCounter += 16;
	pData[3] = _pitchCycleCounter & 0x7F;
	playSound(0xE52);
	return 0;
}

int RSound2::command13() {
	playSound(0xE5C);
	playSound(0xE66);
	return 0;
}

int RSound2::command14() {
	playSound(0xE70);
	playSound(0xE8A);
	return 0;
}

int RSound2::command15() {
	byte *pData = loadData(0x1DFC);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x1DFC);
		playSoundAny(0x222E);
		playSoundAny(0x2648);
		_channels[8].load(loadData(0x26A2));
	}
	return 0;
}

int RSound2::command16() {
	byte *pData = loadData(0x3456);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x3456);
		playSoundAny(0x3572);
		playSoundAny(0x367E);
		playSoundAny(0x37C6);
		playSoundAny(0x39AE);
		playSoundAny(0x3A3A);
	}
	return 0;
}

int RSound2::command17() {
	byte *pData = loadData(0x3AC0);
	if (!isSoundActive(pData)) {
		playSound(0x3AC0);
		playSound(0x3C70);
		playSound(0x3E16);
		playSound(0x3FBE);
	}
	return 0;
}

int RSound2::command18() {
	if (_channels[7]._activeCount)
		return 0;

	int idx = (getRandomNumber() & 30) >> 1;
	_channels[7].load(loadData(_table1[idx]));
	return 0;
}

int RSound2::command19() {
	byte *pData = loadData(0x2A64);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x2A64);
		playSoundAny(0x2BE2);
		playSoundAny(0x2DAC);
		playSoundAny(0x2ECE);
		playSoundAny(0x3026);
		playSoundAny(0x30C2);
	}
	return 0;
}

int RSound2::command20() {
	playSound(0xF1E);
	playSound(0xF1E);
	playSound(0xF1E);
	playSound(0xF1E);
	return 0;
}

int RSound2::command21() {
	playSound(0xF58);
	return 0;
}

int RSound2::command22() {
	playSound(0xF44);
	return 0;
}

int RSound2::command23() {
	playSound(0xEBA);
	return 0;
}

int RSound2::command24() {
	playSound(0xEB0);
	return 0;
}

int RSound2::command25() {
	playSound(0xEA6);
	return 0;
}

int RSound2::command26() {
	playSound(0xF4E);
	return 0;
}

int RSound2::command27() {
	Channel *ch = playSound(0xFAC);
	if (ch)
		ch->_innerLoopPtr = loadData(0xFB8);

	ch = playSound(0xFB2);
	if (ch)
		ch->_innerLoopPtr = loadData(0xFB8);

	playSound(0xFB8);
	return 0;
}

int RSound2::command28() {
	byte *pData = loadData(0xEDA);
	int r = getRandomNumber();
	int v1 = r & 0x7F;
	pData[7] = v1;
	int v2 = (v1 & 0x0F) + 0x43;
	pData[8] = v2;
	int v3 = v2 + 0x0C;
	pData[0xA] = v3;
	playSound(0xEDA);
	return 0;
}

int RSound2::command29() {
	playSoundAny(0xF80);
	return 0;
}

int RSound2::command30() {
	playSound(0xF14);
	byte *pData = loadData(0xF0A);
	pData[3] = 40;
	playSound(0xF0A);
	return 0;
}

int RSound2::command31() {
	byte *pData = loadData(0xF0A);
	pData[3] = 0x18;
	playSound(0xF0A);
	return 0;
}

int RSound2::command32() {
	playSound(0xEC4);
	return 0;
}

int RSound2::command33() {
	playSound(0xED0);
	return 0;
}

int RSound2::command34() {
	playSound(0xEE8);
	return 0;
}

int RSound2::command35() {
	playSound(0xEF8);
	return 0;
}

int RSound2::command36() {
	playSound(0xFF4);
	playSound(0x1008);
	return 0;
}

int RSound2::command37() {
	playSound(0xFCC);
	return 0;
}

int RSound2::command38() {
	byte *pData = loadData(0x2B0E);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x2B0E);
		playSoundAny(0x2CD0);
		playSoundAny(0x2E46);
		playSoundAny(0x2F7C);
		playSoundAny(0x3074);
		playSoundAny(0x317E);
	}
	return 0;
}

int RSound2::command39() {
	playSound(0xFDA);
	return 0;
}

int RSound2::command40() {
	playSound(0xFE6);
	return 0;
}

int RSound2::command41() {
	playSoundAny(0xF62);
	return 0;
}

int RSound2::command42() {
	playSound(0xF92);
	return 0;
}

int RSound2::command43() {
	playSound(0x1018);
	playSound(0x102A);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound3::CommandPtr RSound3::_commandList[61] = {
	&RSound3::command0, &RSound3::command1, &RSound3::command2, &RSound3::command3,
	&RSound3::command4, &RSound3::command5, &RSound3::command6, &RSound3::command7,
	&RSound3::command8, &RSound3::command9, &RSound3::command10, &RSound3::command11,
	&RSound3::nullCommand, &RSound3::command13, &RSound3::command14, &RSound3::command15,
	&RSound3::command16, &RSound3::command17, &RSound3::command18, &RSound3::command19,
	&RSound3::command20, &RSound3::command21, &RSound3::command22, &RSound3::command23,
	&RSound3::command24, &RSound3::command25, &RSound3::command26, &RSound3::command27x42,
	&RSound3::command28, &RSound3::command29, &RSound3::command30, &RSound3::command31,
	&RSound3::command32, &RSound3::command33, &RSound3::command34, &RSound3::command35,
	&RSound3::command36, &RSound3::command37, &RSound3::command38, &RSound3::command39,
	&RSound3::command40, &RSound3::command41, &RSound3::command27x42, &RSound3::command43,
	&RSound3::command44, &RSound3::command45, &RSound3::command46, &RSound3::command47x49,
	&RSound3::command48, &RSound3::command47x49, &RSound3::command50, &RSound3::command51,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::command57, &RSound3::nullCommand, &RSound3::command59,
	&RSound3::command60
};

RSound3::RSound3(Audio::Mixer *mixer) : RSound(mixer, "rsound.003", 0x14E0, 0x4C60, 0x67) {
}

int RSound3::command(int commandId, int param) {
	if (commandId > 60)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound3::notImplemented() {
	warning("RSound3::command: not yet implemented (missing disassembly)");
	return 0;
}

Channel *RSound3::method1(int offset, byte value) {
	byte *pData = loadData(offset);
	pData[5] = value;
	return playSound(offset);
}

void RSound3::sub1074E() {
	_byte10742 = 1;
}

void RSound3::resetUpperChannelsTail() {
	sub1074E();
	_channels[4].enable(0xFF);
	_channels[5].enable(0xFF);
	_channels[6].enable(0xFF);
	_channels[7].enable(0xFF);
}

int RSound3::command1() {
	command3();
	resetUpperChannelsTail();
	return 0;
}

int RSound3::command5() {
	if (!isSoundActive(loadData(0x1AE6)))
		resetUpperChannelsTail();
	return 0;
}

int RSound3::command9() {
	command1();
	_byte10742 = (byte)_commandParam;
	return 0;
}

int RSound3::command10() {
	byte *pData = loadData(0x14FE);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1630));
		_channels[2].load(loadData(0x186E));
		_channels[3].load(loadData(0x1A68));
		_channels[8].load(loadData(0x1AA6));
	}
	return 0;
}

void RSound3::setVariantByte(byte value) {
	loadData(0x204A)[1] = value;
	loadData(0x229C)[1] = value;
	loadData(0x2748)[1] = value;
	loadData(0x2C56)[1] = value;
}

int RSound3::command11() {
	if (!isSoundActive(loadData(0x1AE6))) {
		setVariantByte(0x64);
		_channels[0].load(loadData(0x1AE6));
		_channels[1].load(loadData(0x1E00));
		_channels[2].load(loadData(0x1E66));
		_channels[3].load(loadData(0x204A));
		_channels[4].load(loadData(0x229C));
		_channels[5].load(loadData(0x2748));
		_channels[6].load(loadData(0x2C56));
	}
	return 0;
}

int RSound3::command13() {
	command1();
	playSoundAny(0x1364);
	playSoundAny(0x1364);
	playSoundAny(0x1364);
	playSoundAny(0x1364);
	playSoundAny(0x1364);
	return 0;
}

int RSound3::command14() {
	_channels[0].load(loadData(0x32DC));
	_channels[1].load(loadData(0x32FC));
	_channels[2].load(loadData(0x331C));
	_channels[3].load(loadData(0x333E));
	_channels[4].load(loadData(0x335C));
	_channels[5].load(loadData(0x339E));
	_channels[6].load(loadData(0x33DE));
	_channels[7].load(loadData(0x341E));
	return 0;
}

void RSound3::sendDualVolume(byte volume) {
	_channels[0]._volume = volume;
	sendVolume(1, volume);
	_channels[0]._volume = volume;
	sendVolume(2, volume);
}

int RSound3::command15() {
	setVariantByte(0x60);
	sendDualVolume(0x60);

	if (_channels[3]._activeCount && _channels[3]._soundData == loadData(0x204A)) {
		_channels[2]._pendingStop = 0xFF;
		_channels[3]._pendingStop = 0xFF;
		_channels[4]._pendingStop = 0xFF;
		_channels[5]._pendingStop = 0xFF;
		_channels[6]._pendingStop = 0xFF;
		_channels[7]._pendingStop = 0xFF;
		sub1074E();
		return 0;
	}

	command1();
	_channels[0].load(loadData(0x1AE6));
	_channels[1].load(loadData(0x1E00));
	return 0;
}

int RSound3::command16() {
	_command16AltFlag = !_command16AltFlag;

	if (_command16AltFlag) {
		byte *pData = loadData(0x345E);
		if (!isSoundActive(pData)) {
			_channels[0].load(pData);
			_channels[1].load(loadData(0x364C));
			_channels[2].load(loadData(0x3806));
			_channels[3].load(loadData(0x399E));
		}
	} else {
		byte *pData = loadData(0x3B26);
		if (!isSoundActive(pData)) {
			command1();
			_channels[0].load(pData);
			_channels[1].load(loadData(0x3BD8));
			_channels[2].load(loadData(0x3CF8));
			_channels[3].load(loadData(0x3E46));
		}
	}
	return 0;
}

int RSound3::command17() {
	byte *pData = loadData(0x3F5C);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x4022));
		_channels[2].load(loadData(0x41F0));
		_channels[3].load(loadData(0x42F8));
	}
	return 0;
}

int RSound3::command18() {
	command1();
	_channels[0].load(loadData(0x4492));
	_channels[1].load(loadData(0x45A4));
	_channels[2].load(loadData(0x46A2));
	_channels[3].load(loadData(0x47DC));
	_channels[4].load(loadData(0x49C0));
	_channels[5].load(loadData(0x4A4E));
	return 0;
}

int RSound3::command19() {
	if (!isSoundActive(loadData(0x1AE6)))
		playSound(0x12B4);
	return 0;
}

int RSound3::command20() {
	if (!isSoundActive(loadData(0x1AE6)))
		playSound(0x1246);
	return 0;
}

int RSound3::command21() {
	if (!isSoundActive(loadData(0x1AE6))) {
		playSound(0x1232);
		playSound(0x123C);
	}
	return 0;
}

int RSound3::command22() {
	playSound(0x126E);
	return 0;
}

int RSound3::command23() {
	if (!isSoundActive(loadData(0x1AE6))) {
		playSound(0x12D2);
		playSound(0x12E0);
	}
	return 0;
}

int RSound3::command24() {
	if (!isSoundActive(loadData(0x1AE6))) {
		playSound(0x11E2);
		playSound(0x120A);
	}
	return 0;
}

int RSound3::command25() {
	method1(0x11A6, 42);
	method1(0x11C4, 42);
	return 0;
}

int RSound3::command26() {
	playSound(0x1252);
	return 0;
}

int RSound3::command27x42() {
	playSound(0x14BE);
	return 0;
}

int RSound3::command28() {
	byte *pData = loadData(0x12EE);
	pData[3] = 0x4D;
	playSound(0x12EE);
	return 0;
}

int RSound3::command29() {
	byte *pData = loadData(0x12EE);
	pData[3] = 0x7F;
	playSound(0x12EE);
	return 0;
}

int RSound3::command30() {
	playSound(0x14B4);
	playSound(0x14AA);
	return 0;
}

int RSound3::command31() {
	playSound(0x12F8);
	playSound(0x130E);
	return 0;
}

int RSound3::command32() {
	playSound(0x1472);
	return 0;
}

int RSound3::command33() {
	playSound(0x147E);
	return 0;
}

int RSound3::command34() {
	playSound(0x1488);
	return 0;
}

int RSound3::command35() {
	playSound(0x1498);
	return 0;
}

int RSound3::command36() {
	playSound(0x14DA);
	playSound(0x14EE);
	return 0;
}

int RSound3::command37() {
	playSound(0x14CC);
	return 0;
}

int RSound3::command38() {
	playSound(0x133A);
	return 0;
}

int RSound3::command39() {
	byte *pData = loadData(0x1346);
	pData[3] = 77;
	_command3940Toggle ^= 4;
	pData[6] = _command3940Toggle + 0x28;
	playSound(0x1346);
	return 0;
}

int RSound3::command40() {
	byte *pData = loadData(0x1346);
	pData[3] = 47;
	_command3940Toggle ^= 4;
	pData[6] = _command3940Toggle + 0x28;
	playSound(0x1346);
	return 0;
}

int RSound3::command41() {
	playSound(0x1184);
	return 0;
}

int RSound3::command43() {
	playSound(0x1350);
	playSound(0x135A);
	return 0;
}

int RSound3::command44() {
	playSound(0x12A0);
	return 0;
}

int RSound3::command45() {
	playSound(0x12AA);
	return 0;
}

int RSound3::command46() {
	playSound(0x13AA);
	playSound(0x13C6);
	return 0;
}

int RSound3::command47x49() {
	playSound(0x13E6);
	playSound(0x13FE);
	return 0;
}

int RSound3::command48() {
	playSound(0x141E);
	return 0;
}

int RSound3::command50() {
	playSound(0x1436);
	playSound(0x144C);
	return 0;
}

int RSound3::command51() {
	playSound(0x125C);
	return 0;
}

int RSound3::command57() {
	playSound(0x1466);
	return 0;
}

int RSound3::command59() {
	playSound(0x1324);
	return 0;
}

int RSound3::command60() {
	playSound(0x132E);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound4::CommandPtr RSound4::_commandList[60] = {
	&RSound4::command0, &RSound4::command1, &RSound4::command2, &RSound4::command3,
	&RSound4::command4, &RSound4::command5, &RSound4::command6, &RSound4::command7,
	&RSound4::command8, &RSound4::command9, &RSound4::command10, &RSound4::nullCommand,
	&RSound4::command12, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::command19,
	&RSound4::command20, &RSound4::command21, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::command27,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::command30, &RSound4::nullCommand,
	&RSound4::command32, &RSound4::command33, &RSound4::command34, &RSound4::command35,
	&RSound4::command36, &RSound4::command37, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::command52, &RSound4::command53, &RSound4::command54, &RSound4::command55,
	&RSound4::command56, &RSound4::command57, &RSound4::command58, &RSound4::command59
};

RSound4::RSound4(Audio::Mixer *mixer) : RSound(mixer, "rsound.004", 0x1340, 0x2E20, 0x67) {
}

int RSound4::command(int commandId, int param) {
	if (commandId > 59)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

void RSound4::tickCallback() {
	if (!_callbackPeriod)
		return;
	if (--_callbackCounter)
		return;

	_callbackCounter = _callbackPeriod;
	if (_callbackFnPtr)
		(this->*_callbackFnPtr)();
}

int RSound4::command9() {
	command1();
	_byte10745 = (byte)_commandParam;
	return 0;
}

byte RSound4::paramToVariant() {
	return (byte)((_commandParam >> 1) + 36);
}

void RSound4::setCommand12Variant() {
	byte value = paramToVariant();
	loadData(0x1966)[1] = value;
	loadData(0x1DC8)[1] = value;
	loadData(0x1FBA)[1] = value;
	loadData(0x211E)[1] = value;
	loadData(0x24A8)[1] = value;
}

int RSound4::command12() {
	if (_channels[4]._soundData == loadData(0x1966) && _channels[4]._activeCount) {
		setCommand12Variant();
		return 0;
	}

	setCommand12Variant();
	command1();
	_channels[4].load(loadData(0x1966));
	_channels[5].load(loadData(0x1DC8));
	_channels[6].load(loadData(0x1FBA));
	_channels[7].load(loadData(0x211E));
	_channels[8].load(loadData(0x24A8));
	return 0;
}

void RSound4::loadIntroChannels() {
	_channels[0].load(loadData(0x1274));
	_channels[1].load(loadData(0x13A6));
	_channels[2].load(loadData(0x15E4));
}

int RSound4::command10() {
	command1();
	_channels[3].load(loadData(0x17DE));
	_channels[8].load(loadData(0x181C));
	loadIntroChannels();
	return 0;
}

int RSound4::command19() {
	playSound(0x1196);
	return 0;
}

int RSound4::command20() {
	playSound(0x118A);
	return 0;
}

int RSound4::command21() {
	playSound(0x1260);
	playSound(0x126A);
	return 0;
}

int RSound4::command27() {
	playSound(0x1220);
	return 0;
}

int RSound4::command30() {
	playSound(0x1216);
	playSound(0x120C);
	return 0;
}

int RSound4::command32() {
	playSound(0x11D4);
	return 0;
}

int RSound4::command33() {
	playSound(0x11E0);
	return 0;
}

int RSound4::command34() {
	playSound(0x11EA);
	return 0;
}

int RSound4::command35() {
	playSound(0x11FA);
	return 0;
}

int RSound4::command36() {
	playSound(0x123C);
	playSound(0x1250);
	return 0;
}

int RSound4::command37() {
	playSound(0x122E);
	return 0;
}

int RSound4::command52() {
	_channels[0]._pSrc = loadData(0x1188);
	_channels[1]._pSrc = loadData(0x1188);
	_channels[2]._pSrc = loadData(0x1188);
	_channels[4].load(loadData(0x2A0C));
	return 0;
}

int RSound4::command53() {
	command1();
	_callbackCounter = 56;
	_callbackPeriod = 56;
	playSoundAny(0x1888);
	playSoundAny(0x18DE);
	return 0;
}

void RSound4::loadCommand54() {
	_callbackFnPtr = nullptr;
	playSoundAny(0x18B2);
	playSoundAny(0x1904);
}

int RSound4::command54() {
	_callbackFnPtr = &RSound4::loadCommand54;
	return 0;
}

void RSound4::loadCommand55() {
	_callbackFnPtr = nullptr;
	playSoundAny(0x191E);
}

int RSound4::command55() {
	_callbackFnPtr = &RSound4::loadCommand55;
	return 0;
}

void RSound4::loadCommand56() {
	_callbackFnPtr = nullptr;
	playSoundAny(0x185C);
}

int RSound4::command56() {
	_callbackFnPtr = &RSound4::loadCommand56;
	return 0;
}

int RSound4::command57() {
	playSound(0x11C8);
	return 0;
}

int RSound4::command58() {
	_channels[4]._pSrc = loadData(0x1188);
	loadIntroChannels();
	return 0;
}

int RSound4::command59() {
	playSound(0x11BE);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound5::CommandPtr RSound5::_commandList[42] = {
	&RSound5::command0, &RSound5::command1, &RSound5::command2, &RSound5::command3,
	&RSound5::command4, &RSound5::command5, &RSound5::command6, &RSound5::command7,
	&RSound5::command8, &RSound5::command9, &RSound5::command10, &RSound5::command11x24,
	&RSound5::command12x25, &RSound5::command13, &RSound5::command14, &RSound5::command15,
	&RSound5::command16, &RSound5::command17, &RSound5::command18, &RSound5::command19,
	&RSound5::command20, &RSound5::command21, &RSound5::command22, &RSound5::command23,
	&RSound5::command11x24, &RSound5::command12x25, &RSound5::command26, &RSound5::command27,
	&RSound5::command28, &RSound5::command29, &RSound5::command30, &RSound5::command31,
	&RSound5::command32, &RSound5::command33, &RSound5::command34, &RSound5::command35,
	&RSound5::command36, &RSound5::command37, &RSound5::command38, &RSound5::command39,
	&RSound5::command40, &RSound5::command41
};

RSound5::RSound5(Audio::Mixer *mixer) : RSound(mixer, "rsound.005", 0x12A0, 0x1FD0, 0x67) {
}

int RSound5::command(int commandId, int param) {
	if (commandId > 41)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound5::command9() {
	playSound(0x11C4);
	return 0;
}

int RSound5::command10() {
	playSound(0x1238);
	return 0;
}

int RSound5::command11x24() {
	playSound(0x1196);
	return 0;
}

int RSound5::command12x25() {
	playSound(0x1242);
	return 0;
}

int RSound5::command13() {
	playSound(0x125E);
	playSound(0x1268);
	return 0;
}

int RSound5::command14() {
	_channels[7].load(loadData(0x1272));
	return 0;
}

int RSound5::command15() {
	if (_channels[7]._soundData == loadData(0x1272)) {
		byte *pData = loadData(0x1288);
		_channels[7]._innerLoopPtr = pData;
		_channels[7]._outerLoopPtr = pData;
		_channels[7]._activeCount = 1;
	}
	return 0;
}

int RSound5::command16() {
	playSound(0x129A);
	playSound(0x129A);
	playSound(0x129A);
	playSound(0x129A);
	return 0;
}

int RSound5::command17() {
	playSound(0x11B4);
	return 0;
}

int RSound5::command18() {
	playSound(0x12E4);
	playSound(0x12F6);
	playSound(0x1308);
	playSound(0x131A);
	return 0;
}

int RSound5::command19() {
	playSound(0x132C);
	return 0;
}

int RSound5::command20() {
	playSound(0x1368);
	return 0;
}

int RSound5::command21() {
	playSound(0x1388);
	return 0;
}

int RSound5::command22() {
	playSound(0x139A);
	return 0;
}

int RSound5::command23() {
	playSound(0x13AA);
	playSound(0x13AA);
	playSound(0x13AA);
	playSound(0x13AA);
	return 0;
}

int RSound5::command26() {
	playSound(0x13D6);
	return 0;
}

int RSound5::command27() {
	playSound(0x13F0);
	return 0;
}

int RSound5::command28() {
	playSound(0x121C);
	return 0;
}

void RSound5::loadTailChannels() {
	_channels[3].load(loadData(0x1688));
	_channels[8].load(loadData(0x1882));
}

int RSound5::command29() {
	byte *pData = loadData(0x1488);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1534));
		_channels[2].load(loadData(0x15EA));
		loadTailChannels();
	}
	return 0;
}

int RSound5::command30() {
	playSound(0x1212);
	playSound(0x1208);
	return 0;
}

int RSound5::command31() {
	playSound(0x140A);
	return 0;
}

int RSound5::command32() {
	playSound(0x11D0);
	return 0;
}

int RSound5::command33() {
	playSound(0x11DC);
	return 0;
}

int RSound5::command34() {
	playSound(0x11E6);
	return 0;
}

int RSound5::command35() {
	playSound(0x11F6);
	return 0;
}

int RSound5::command36() {
	playSound(0x1464);
	playSound(0x1478);
	return 0;
}

int RSound5::command37() {
	playSound(0x122A);
	return 0;
}

int RSound5::command38() {
	_channels[4]._pSrc = loadData(0x1182);
	loadTailChannels();
	return 0;
}

int RSound5::command39() {
	playSound(0x141E);
	playSound(0x1428);
	return 0;
}

int RSound5::command40() {
	playSound(0x1432);
	return 0;
}

int RSound5::command41() {
	_channels[8]._pSrc = loadData(0x1182);
	_channels[3].load(loadData(0x1BB6));
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound6::CommandPtr RSound6::_commandList[30] = {
	&RSound6::command0, &RSound6::command1, &RSound6::command2, &RSound6::command3,
	&RSound6::command4, &RSound6::command5, &RSound6::command6, &RSound6::command7,
	&RSound6::command8, &RSound6::command9, &RSound6::command10, &RSound6::command11,
	&RSound6::command12, &RSound6::command13x14, &RSound6::command13x14, &RSound6::command15,
	&RSound6::command16, &RSound6::command17, &RSound6::command18, &RSound6::command19,
	&RSound6::command20, &RSound6::command21, &RSound6::command22, &RSound6::command23,
	&RSound6::command24, &RSound6::command25, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::nullCommand, &RSound6::command28
};

RSound6::RSound6(Audio::Mixer *mixer) : RSound(mixer, "rsound.006", 0x12D0, 0x1EF0, 0x67) {
}

int RSound6::command(int commandId, int param) {
	if (commandId > 29)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

void RSound6::tickCallback() {
	if (!_callbackPeriod)
		return;
	if (--_callbackCounter)
		return;

	_callbackCounter = _callbackPeriod;
	if (_callbackFnPtr)
		(this->*_callbackFnPtr)();
}

void RSound6::reloadCommand24() {
	_callbackFnPtr = nullptr;
	command1();
	_callbackCounter = 84;
	_callbackPeriod = 84;
	_channels[0].load(loadData(0x1A38));
	_channels[1].load(loadData(0x1BBE));
	_channels[8].load(loadData(0x1B90));
}

void RSound6::reloadCommand28() {
	_callbackFnPtr = nullptr;
	command1();
	_callbackCounter = 84;
	_callbackPeriod = 84;
	_channels[0].load(loadData(0x130A));
	_channels[1].load(loadData(0x13B6));
	_channels[2].load(loadData(0x146C));
	_channels[3].load(loadData(0x150A));
	_channels[8].load(loadData(0x1704));
}

int RSound6::command9() {
	playSound(0x111C);
	return 0;
}

int RSound6::command10() {
	playSound(0x1190);
	return 0;
}

int RSound6::command11() {
	playSound(0x11A4);
	playSound(0x11A4);
	playSound(0x11A4);
	playSound(0x11A4);
	return 0;
}

int RSound6::command12() {
	playSound(0x11C8);
	return 0;
}

int RSound6::command13x14() {
	playSound(0x11E8);
	return 0;
}

int RSound6::command15() {
	playSound(0x11F2);
	playSound(0x11F2);
	playSound(0x11F2);
	playSound(0x11F2);
	return 0;
}

int RSound6::command16() {
	playSound(0x121E);
	return 0;
}

int RSound6::command17() {
	playSound(0x1228);
	playSound(0x1228);
	playSound(0x1228);
	playSound(0x1228);
	return 0;
}

int RSound6::command18() {
	playSound(0x1254);
	return 0;
}

int RSound6::command19() {
	playSound(0x1266);
	return 0;
}

int RSound6::command20() {
	playSound(0x1278);
	return 0;
}

int RSound6::command21() {
	_channels[4].load(loadData(0x1282));
	playSound(0x1282);
	playSound(0x1282);
	playSound(0x12AA);
	return 0;
}

int RSound6::command22() {
	_channels[4].load(loadData(0x12CE));
	_channels[5].load(loadData(0x12CE));
	_channels[6].load(loadData(0x12CE));
	_channels[7].load(loadData(0x12CE));
	return 0;
}

int RSound6::command23() {
	playSound(0x1174);
	return 0;
}

int RSound6::command24() {
	if (_channels[0]._activeCount && _channels[0]._soundData == loadData(0x130A)) {
		_callbackFnPtr = &RSound6::reloadCommand24;
		return 0;
	}

	reloadCommand24();
	return 0;
}

int RSound6::command25() {
	_channels[4].load(loadData(0x12FE));
	return 0;
}

int RSound6::command28() {
	if (isSoundActive(loadData(0x130A)))
		return 0;

	if (_channels[0]._activeCount && _channels[0]._soundData == loadData(0x1A38)) {
		_callbackFnPtr = &RSound6::reloadCommand28;
		return 0;
	}

	reloadCommand28();
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound7::CommandPtr RSound7::_commandList[38] = {
	&RSound7::command0, &RSound7::command1, &RSound7::command2, &RSound7::command3,
	&RSound7::command4, &RSound7::command5, &RSound7::command6, &RSound7::command7,
	&RSound7::command8, &RSound7::command9, &RSound7::nullCommand, &RSound7::nullCommand,
	&RSound7::nullCommand, &RSound7::nullCommand, &RSound7::nullCommand, &RSound7::command15,
	&RSound7::command16, &RSound7::command17, &RSound7::command18, &RSound7::command19,
	&RSound7::command20, &RSound7::command21, &RSound7::command22, &RSound7::command23,
	&RSound7::command24, &RSound7::command25, &RSound7::nullCommand, &RSound7::command27,
	&RSound7::nullCommand, &RSound7::nullCommand, &RSound7::command30, &RSound7::nullCommand,
	&RSound7::command32, &RSound7::command33, &RSound7::command34, &RSound7::command35,
	&RSound7::command36, &RSound7::command37
};

RSound7::RSound7(Audio::Mixer *mixer) : RSound(mixer, "rsound.007", 0x1240, 0x1EF0, 0x67) {
}

int RSound7::command(int commandId, int param) {
	if (commandId > 37)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound7::command9() {
	command1();
	_channels[0].load(loadData(0x1C0E));
	_channels[1].load(loadData(0x1C88));
	_channels[2].load(loadData(0x1CD4));
	_channels[3].load(loadData(0x1D4E));
	return 0;
}

int RSound7::command15() {
	playSound(0x125C);
	return 0;
}

int RSound7::command16() {
	playSound(0x12DE);
	return 0;
}

int RSound7::command17() {
	playSound(0x12C2);
	return 0;
}

int RSound7::command18() {
	_channels[7].load(loadData(0x12FC));
	return 0;
}

int RSound7::command19() {
	if (_channels[7]._soundData == loadData(0x12FC)) {
		byte *pData = loadData(0x1312);
		_channels[7]._innerLoopPtr = pData;
		_channels[7]._outerLoopPtr = pData;
		_channels[7]._activeCount = 1;
	}
	return 0;
}

int RSound7::command20() {
	playSound(0x1324);
	playSound(0x1324);
	return 0;
}

int RSound7::command21() {
	playSound(0x1336);
	return 0;
}

int RSound7::command22() {
	playSound(0x1340);
	return 0;
}

int RSound7::command23() {
	playSound(0x12B4);
	return 0;
}

int RSound7::command24() {
	_channels[0].load(loadData(0x137C));
	_channels[1].load(loadData(0x1406));
	_channels[2].load(loadData(0x1492));
	_channels[3].load(loadData(0x1516));
	_channels[4].load(loadData(0x1588));
	return 0;
}

int RSound7::command25() {
	command1();
	_channels[0].load(loadData(0x1612));
	_channels[1].load(loadData(0x16C8));
	_channels[2].load(loadData(0x177E));
	_channels[3].load(loadData(0x1838));
	return 0;
}

int RSound7::command27() {
	_channels[0].load(loadData(0x1932));
	_channels[1].load(loadData(0x1986));
	_channels[2].load(loadData(0x19EC));
	_channels[3].load(loadData(0x1A66));
	_channels[4].load(loadData(0x1B3C));
	return 0;
}

int RSound7::command30() {
	playSound(0x12AA);
	playSound(0x12A0);
	return 0;
}

int RSound7::command32() {
	playSound(0x1268);
	return 0;
}

int RSound7::command33() {
	playSound(0x1274);
	return 0;
}

int RSound7::command34() {
	playSound(0x127E);
	return 0;
}

int RSound7::command35() {
	playSound(0x128E);
	return 0;
}

int RSound7::command36() {
	playSound(0x1358);
	playSound(0x136C);
	return 0;
}

int RSound7::command37() {
	playSound(0x134A);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound8::CommandPtr RSound8::_commandList[38] = {
	&RSound8::command0, &RSound8::command1, &RSound8::command2, &RSound8::command3,
	&RSound8::command4, &RSound8::command5, &RSound8::command6, &RSound8::command7,
	&RSound8::command8, &RSound8::command9, &RSound8::command10, &RSound8::command11,
	&RSound8::command12, &RSound8::command13, &RSound8::command14, &RSound8::command15,
	&RSound8::command16, &RSound8::command17, &RSound8::command18, &RSound8::command19,
	&RSound8::command20, &RSound8::command21, &RSound8::command22, &RSound8::command23,
	&RSound8::command24, &RSound8::command25, &RSound8::command26, &RSound8::command27,
	&RSound8::command28, &RSound8::command29, &RSound8::command30, &RSound8::command31,
	&RSound8::command32, &RSound8::command33, &RSound8::command34, &RSound8::command35,
	&RSound8::command36, &RSound8::command37
};

RSound8::RSound8(Audio::Mixer *mixer) : RSound(mixer, "rsound.008", 0x1290, 0x19A0, 0x67) {
}

int RSound8::command(int commandId, int param) {
	if (commandId > 37)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound8::command9() {
	playSound(0x10BA);
	return 0;
}

int RSound8::command10() {
	_channels[0].load(loadData(0x115A));
	_channels[1].load(loadData(0x115A));
	_channels[2].load(loadData(0x115A));
	_channels[3].load(loadData(0x1150));
	return 0;
}

int RSound8::command11() {
	playSound(0x1194);
	return 0;
}

int RSound8::command12() {
	playSound(0x11B2);
	return 0;
}

int RSound8::command13() {
	playSound(0x11D0);
	playSound(0x11D0);
	playSound(0x11D0);
	playSound(0x11D0);
	return 0;
}

void RSound8::setCommand1415Variant(byte v1, byte v2) {
	byte *pData = loadData(0x1204);
	pData[3] = v1;
	pData[6] = v2;
	pData[9] = v2;
	playSound(0x1204);
	playSound(0x1204);
	playSound(0x1204);
	playSound(0x1204);
}

int RSound8::command14() {
	setCommand1415Variant(40, 1);
	return 0;
}

int RSound8::command15() {
	setCommand1415Variant(100, 255);
	return 0;
}

int RSound8::command16() {
	playSound(0x112E);
	playSound(0x112E);
	return 0;
}

int RSound8::command17() {
	playSound(0x1234);
	return 0;
}

int RSound8::command18() {
	playSound(0x1244);
	return 0;
}

int RSound8::command19() {
	playSound(0x1254);
	return 0;
}

int RSound8::command20() {
	playSound(0x125E);
	return 0;
}

int RSound8::command21() {
	playSound(0x126E);
	return 0;
}

int RSound8::command22() {
	playSound(0x1278);
	return 0;
}

int RSound8::command23() {
	_channels[0].load(loadData(0x128E));
	_channels[1].load(loadData(0x128E));
	_channels[2].load(loadData(0x128E));
	_channels[3].load(loadData(0x128E));
	return 0;
}

int RSound8::command24() {
	playSound(0x12B4);
	return 0;
}

int RSound8::command25() {
	playSound(0x12CA);
	return 0;
}

int RSound8::command26() {
	playSound(0x12DC);
	return 0;
}

int RSound8::command27() {
	playSound(0x1112);
	return 0;
}

int RSound8::command28() {
	byte *pData = loadData(0x130A);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1480));
		_channels[8].load(loadData(0x154C));
	}
	return 0;
}

int RSound8::command29() {
	byte *pData = loadData(0x15FC);
	if (!isSoundActive(pData)) {
		command1();
		_channels[2].load(pData);
		_channels[8].load(loadData(0x1652));
	}
	return 0;
}

int RSound8::command30() {
	playSound(0x1108);
	playSound(0x10FE);
	return 0;
}

int RSound8::command31() {
	playSound(0x1140);
	return 0;
}

int RSound8::command32() {
	playSound(0x10C6);
	return 0;
}

int RSound8::command33() {
	playSound(0x10D2);
	return 0;
}

int RSound8::command34() {
	playSound(0x10DC);
	return 0;
}

int RSound8::command35() {
	playSound(0x10EC);
	return 0;
}

int RSound8::command36() {
	playSound(0x12E6);
	playSound(0x12FA);
	return 0;
}

int RSound8::command37() {
	playSound(0x1120);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound9::CommandPtr RSound9::_commandList[52] = {
	&RSound9::command0, &RSound9::command1, &RSound9::command2, &RSound9::command3,
	&RSound9::command4, &RSound9::command5, &RSound9::command6, &RSound9::command7,
	&RSound9::command8, &RSound9::command9, &RSound9::command10, &RSound9::command11,
	&RSound9::command12, &RSound9::command13, &RSound9::command14, &RSound9::command15,
	&RSound9::command16, &RSound9::command17, &RSound9::command18, &RSound9::command19,
	&RSound9::command20, &RSound9::command21, &RSound9::command22, &RSound9::command23,
	&RSound9::command24, &RSound9::command25, &RSound9::command26, &RSound9::command27,
	&RSound9::command28, &RSound9::command29, &RSound9::command30, &RSound9::command31,
	&RSound9::command32, &RSound9::command33, &RSound9::command34, &RSound9::command35,
	&RSound9::command36, &RSound9::command37, &RSound9::command38, &RSound9::command39,
	&RSound9::command40, &RSound9::command41, &RSound9::command42, &RSound9::command43,
	&RSound9::command44_46, &RSound9::command45, &RSound9::command44_46, &RSound9::command47,
	&RSound9::command48, &RSound9::command49, &RSound9::command50, &RSound9::command51
};

RSound9::RSound9(Audio::Mixer *mixer) : RSound(mixer, "rsound.009", 0x1520, 0x8920, 0x6F) {
	_callbackCounter = 0;
	_callbackPeriod = 0;
	_callbackFnPtr = nullptr;
}

int RSound9::command(int commandId, int param) {
	if (commandId > 51)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

void RSound9::tickCallback() {
	if (!_callbackPeriod)
		return;
	if (--_callbackCounter)
		return;

	_callbackCounter = _callbackPeriod;
	if (_callbackFnPtr)
		(this->*_callbackFnPtr)();
}

int RSound9::command0() {
	_callbackCounter = 0;
	_callbackPeriod = 0;
	_callbackFnPtr = nullptr;
	return RSound::command0();
}

int RSound9::command9() {
	_callbackCounter = 1848;
	_callbackPeriod = 84;
	_channels[0].load(loadData(0x16E4));
	_channels[1].load(loadData(0x1E9E));
	_channels[2].load(loadData(0x2F9C));
	_channels[3].load(loadData(0x2644));
	_channels[4].load(loadData(0x1FF4));
	_channels[5].load(loadData(0x2382));
	_channels[6].load(loadData(0x1AAE));
	return 0;
}

int RSound9::command10() {
	_channels[0].load(loadData(0x31E2));
	_channels[1].load(loadData(0x31FA));
	_channels[2].load(loadData(0x3212));
	_channels[3].load(loadData(0x322C));
	return 0;
}

int RSound9::command11() {
	_channels[7].load(loadData(0x33E2));
	return 0;
}

int RSound9::command12() {
	_channels[7].load(loadData(0x342E));
	return 0;
}

int RSound9::command13() {
	_channels[7].load(loadData(0x343A));
	return 0;
}

int RSound9::command14() {
	_channels[7].load(loadData(0x3442));
	return 0;
}

int RSound9::command15() {
	_channels[7].load(loadData(0x3462));
	return 0;
}

int RSound9::command16() {
	_channels[7].load(loadData(0x347A));
	return 0;
}

int RSound9::command17() {
	_channels[7].load(loadData(0x3470));
	return 0;
}

int RSound9::command18() {
	playSound(0x3248);
	return 0;
}

int RSound9::command19() {
	playSound(0x3262);
	return 0;
}

int RSound9::command20() {
	int v = (getRandomNumber() & 24) + 77;
	byte *pData = loadData(0x3284);
	pData[6] = v & 0x7F;
	playSound(0x3284);
	return 0;
}

int RSound9::command21() {
	byte *pData = loadData(0x3298);
	pData[9] = 70;
	if (!isSoundActive(pData))
		playSound(0x3298);
	return 0;
}

int RSound9::command22() {
	byte *pData = loadData(0x3298);
	pData[9] = 45;
	if (!isSoundActive(pData))
		playSound(0x3298);
	return 0;
}

int RSound9::command23() {
	Channel *chan = playSound(0x32B0);
	if (chan)
		chan->_innerLoopPtr = loadData(0x32CE);

	chan = playSound(0x32B6);
	if (chan)
		chan->_innerLoopPtr = loadData(0x32CE);

	chan = playSound(0x32C8);
	if (chan)
		chan->_innerLoopPtr = loadData(0x32CE);
	return 0;
}

int RSound9::command24() {
	playSound(0x32E0);
	return 0;
}

int RSound9::command25() {
	playSound(0x32F6);
	return 0;
}

int RSound9::command26() {
	playSound(0x331A);
	return 0;
}

int RSound9::command27() {
	playSound(0x3332);
	return 0;
}

int RSound9::command28() {
	int v = (getRandomNumber() & 28) + 15;
	byte *pData = loadData(0x334A);
	pData[6] = v & 0x7F;
	_channels[7].load(pData);
	return 0;
}

int RSound9::command29() {
	int v = (getRandomNumber() & 12) + 33;
	byte *pData = loadData(0x335E);
	pData[6] = v & 0x7F;
	playSound(0x335E);
	return 0;
}

int RSound9::command30() {
	playSound(0x3386);
	return 0;
}

int RSound9::command31() {
	playSound(0x3396);
	playSound(0x33A4);
	playSound(0x33B2);
	return 0;
}

int RSound9::command32() {
	playSound(0x33C0);
	return 0;
}

int RSound9::command33() {
	playSound(0x33CA);
	return 0;
}

int RSound9::command34() {
	command1();
	_callbackCounter = 96;
	_callbackPeriod = 96;

	*loadData(0x6841) = 2;
	*loadData(0x7DCD) = 2;
	*loadData(0x8791) = 2;

	_channels[0].load(loadData(0x4D2A));
	_channels[1].load(loadData(0x51AA));
	_channels[2].load(loadData(0x5634));
	_channels[3].load(loadData(0x6844));
	_channels[4].load(loadData(0x7DD0));
	return 0;
}

int RSound9::command35() {
	playSound(0x344C);
	return 0;
}

int RSound9::command36() {
	playSound(0x334A);

	Channel *chan = playSound(0x32C2);
	if (chan)
		chan->_innerLoopPtr = loadData(0x3378);

	chan = playSound(0x32BC);
	if (chan)
		chan->_innerLoopPtr = loadData(0x3368);
	return 0;
}

int RSound9::command37() {
	int v = (getRandomNumber() & 2) + 72;
	byte *pData = loadData(0x349C);
	pData[6] = v & 0x7F;
	playSound(0x349C);
	return 0;
}

int RSound9::command38() {
	_callbackFnPtr = &RSound9::loadCommand38;
	return 0;
}

void RSound9::loadCommand38() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x1878));
	_channels[1].load(loadData(0x1F64));
	_channels[2].load(loadData(0x308E));
	_channels[3].load(loadData(0x2A10));
	_channels[4].load(loadData(0x21C8));
	_channels[5].load(loadData(0x2558));
	_channels[6].load(loadData(0x1E9A));
}

int RSound9::command39() {
	_callbackFnPtr = &RSound9::loadCommand39;
	return 0;
}

void RSound9::loadCommand39() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x1A24));
	_channels[1].load(loadData(0x1FD0));
	_channels[2].load(loadData(0x318A));
	_channels[3].load(loadData(0x2E4A));
	_channels[4].load(loadData(0x2380));
	_channels[5].load(loadData(0x2642));
	_channels[6].load(loadData(0x1E9C));
}

int RSound9::command40() {
	_callbackFnPtr = &RSound9::loadCommand40;
	return 0;
}

void RSound9::loadCommand40() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x4F00));
	_channels[1].load(loadData(0x534A));
	_channels[2].load(loadData(0x5CDE));
	_channels[3].load(loadData(0x6F8E));
	_channels[4].load(loadData(0x8110));
}

int RSound9::command41() {
	_callbackFnPtr = &RSound9::loadCommand41;
	return 0;
}

void RSound9::loadCommand41() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x4F26));
	_channels[1].load(loadData(0x53BC));
	_channels[2].load(loadData(0x5DFE));
	_channels[3].load(loadData(0x747E));
	_channels[4].load(loadData(0x8340));
}

int RSound9::command42() {
	_callbackFnPtr = &RSound9::loadCommand42;
	return 0;
}

void RSound9::loadCommand42() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x4F30));
	_channels[1].load(loadData(0x555C));
	_channels[2].load(loadData(0x6582));
	_channels[3].load(loadData(0x7A2E));
	_channels[4].load(loadData(0x8480));
}

int RSound9::command43() {
	_callbackCounter = 80;
	_callbackPeriod = 80;
	_channels[0].load(loadData(0x34BE));
	_channels[1].load(loadData(0x3A46));
	_channels[2].load(loadData(0x3F52));
	_channels[3].load(loadData(0x439A));
	return 0;
}

int RSound9::command44_46() {
	_callbackFnPtr = &RSound9::loadCommand44_46;
	return 0;
}

void RSound9::loadCommand44_46() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x3518));
	_channels[1].load(loadData(0x3AA2));
	_channels[2].load(loadData(0x403A));
	_channels[3].load(loadData(0x4486));
}

int RSound9::command45() {
	_callbackFnPtr = &RSound9::loadCommand45;
	return 0;
}

void RSound9::loadCommand45() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x37AC));
	_channels[1].load(loadData(0x3CF8));
	_channels[2].load(loadData(0x41E6));
	_channels[3].load(loadData(0x4630));
}

int RSound9::command47() {
	_callbackFnPtr = &RSound9::loadCommand47;
	return 0;
}

void RSound9::loadCommand47() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x47D6));
	_channels[1].load(loadData(0x48FA));
	_channels[2].load(loadData(0x4A20));
	_channels[3].load(loadData(0x4BAE));
}

int RSound9::command48() {
	byte *pData = loadData(0x34B0);
	pData[6] ^= 0x1F;
	pData[0xA] ^= 0x1F;
	playSound(0x34B0);
	return 0;
}

int RSound9::command49() {
	_channels[0].load(loadData(0x12CA));
	_channels[1].load(loadData(0x132A));
	_channels[2].load(loadData(0x1384));
	_channels[3].load(loadData(0x1666));
	_channels[4].load(loadData(0x1682));
	_channels[5].load(loadData(0x16A0));
	_channels[6].load(loadData(0x16BE));
	return 0;
}

int RSound9::command50() {
	_callbackFnPtr = &RSound9::loadCommand50;
	return 0;
}

void RSound9::loadCommand50() {
	_callbackFnPtr = nullptr;

	*loadData(0x6841) = 0;
	*loadData(0x7DCD) = 0;
	*loadData(0x8791) = 0;

	_channels[0].load(loadData(0x50B8));
	_channels[1].load(loadData(0x7DCE));
	_channels[2].load(loadData(0x676A));
	_channels[3].load(loadData(0x7D08));
	_channels[4].load(loadData(0x85C4));
}

int RSound9::command51() {
	command1();
	_callbackCounter = 96;
	_callbackPeriod = 96;

	*loadData(0x6841) = 2;
	*loadData(0x7DCD) = 2;
	*loadData(0x8791) = 2;

	_channels[0].load(loadData(0x4D3C));
	_channels[1].load(loadData(0x51EE));
	_channels[2].load(loadData(0x5A62));
	_channels[3].load(loadData(0x6986));
	_channels[4].load(loadData(0x7E5C));
	return 0;
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
