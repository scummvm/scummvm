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

#include "audio/fmopl.h"
#include "common/algorithm.h"
#include "common/file.h"
#include "common/md5.h"
#include "mads/nebular/sound_nebular.h"

namespace Audio {
class Mixer;
}

namespace MADS {
namespace RexNebular {

void RexSoundManager::validate() {
	Common::File f;
	static const char *const MD5[] = {
		"205398468de2c8873b7d4d73d5be8ddc",
		"f9b2d944a2fb782b1af5c0ad592306d3",
		"7431f8dad77d6ddfc24e6f3c0c4ac7df",
		"eb1f3f5a4673d3e73d8ac1818c957cf4",
		"f936dd853073fa44f3daac512e91c476",
		"3dc139d3e02437a6d9b732072407c366",
		"af0edab2934947982e9a405476702e03",
		"8cbc25570b50ba41c9b5361cad4fbedc",
		"a31e4783e098f633cbb6689adb41dd4f"
	};

	for (int i = 1; i <= 9; ++i) {
		Common::Path filename(Common::String::format("ASOUND.00%d", i));
		if (!f.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		Common::String md5str = Common::computeStreamMD5AsString(f, 8192);
		f.close();

		if (md5str != MD5[i - 1])
			error("Invalid sound file - %s", filename.toString().c_str());
	}
}

void RexSoundManager::loadDriver(int sectionNumber) {
	switch (sectionNumber) {
	case 1:
		_driver = new RexNebular::ASound1(_mixer, _opl);
		break;
	case 2:
		_driver = new RexNebular::ASound2(_mixer, _opl);
		break;
	case 3:
		_driver = new RexNebular::ASound3(_mixer, _opl);
		break;
	case 4:
		_driver = new RexNebular::ASound4(_mixer, _opl);
		break;
	case 5:
		_driver = new RexNebular::ASound5(_mixer, _opl);
		break;
	case 6:
		_driver = new RexNebular::ASound6(_mixer, _opl);
		break;
	case 7:
		_driver = new RexNebular::ASound7(_mixer, _opl);
		break;
	case 8:
		_driver = new RexNebular::ASound8(_mixer, _opl);
		break;
	case 9:
		_driver = new RexNebular::ASound9(_mixer, _opl);
		break;
	default:
		_driver = nullptr;
		return;
	}
}

/*-----------------------------------------------------------------------*/

RexASound::RexASound(Audio::Mixer *mixer, OPL::OPL *opl,
		const Common::Path &filename, int dataOffset, int dataSize) :
		ASound(mixer, opl, filename, dataOffset, dataSize) {
	_chanCommandCount = 15;
}

void RexASound::channelCommand(byte *&pSrc, bool &updateFlag) {
	AdlibChannel *chan = _activeChannelPtr;
	int cmdNum = 255 - *pSrc;

	switch (cmdNum) {
	case 0:
		if (!chan->_innerLoopCount) {
			if (*++pSrc == 0) {
				chan->_pSrc += 2;
				chan->_innerLoopPtr = chan->_pSrc;
				chan->_innerLoopCount = 0;
			} else {
				chan->_innerLoopCount = *pSrc;
				chan->_pSrc = chan->_innerLoopPtr;
			}
		} else if (--chan->_innerLoopCount) {
			chan->_pSrc = chan->_innerLoopPtr;
		} else {
			chan->_pSrc += 2;
			chan->_innerLoopPtr = chan->_pSrc;
		}
		break;

	case 1:
		if (!chan->_outerLoopCount) {
			if (*++pSrc == 0) {
				chan->_pSrc += 2;
				chan->_outerLoopPtr = chan->_pSrc;
				chan->_innerLoopPtr = chan->_pSrc;
				chan->_innerLoopCount = 0;
				chan->_outerLoopCount = 0;
			} else {
				chan->_outerLoopCount = *pSrc;
				chan->_pSrc = chan->_outerLoopPtr;
				chan->_innerLoopPtr = chan->_outerLoopPtr;
			}
		} else if (--chan->_outerLoopCount) {
			chan->_outerLoopPtr = chan->_pSrc;
			chan->_innerLoopPtr = chan->_pSrc;
		} else {
			chan->_pSrc += 2;
			chan->_outerLoopPtr = chan->_pSrc;
			chan->_innerLoopPtr = chan->_pSrc;
		}
		break;

	case 2:
		// Loop sound data
		chan->_pitchBend = 0;
		chan->_volumeFadeStep = chan->_attenFadeStep = 0;
		chan->_volume = chan->_noteOffset = 0;
		chan->_transpose = chan->_volumeOffset = 0;
		chan->_keyOnDelay = 0;
		chan->_volumeFadeCounter = 0;
		chan->_attenFadeCounter = 0;
		chan->_innerLoopCount = 0;
		chan->_outerLoopCount = 0;
		chan->_patchAttenuation = 0x40;
		chan->_ptr1 = chan->_soundData;
		chan->_pSrc = chan->_soundData;
		chan->_innerLoopPtr = chan->_soundData;
		chan->_outerLoopPtr = chan->_soundData;

		chan->_pSrc += 2;
		break;

	case 3:
		chan->_sampleIndex = *++pSrc;
		chan->_pSrc += 2;
		loadSample(chan->_sampleIndex);
		break;

	case 4:
		chan->_noteOffset = *++pSrc;
		chan->_pSrc += 2;
		break;

	case 5:
		chan->_pitchBend = *++pSrc;
		chan->_pSrc += 2;
		break;

	case 6:
		++pSrc;
		if (chan->_pendingStop) {
			chan->_pSrc += 2;
		} else {
			chan->_volume = *pSrc >> 1;
			updateFlag = true;
			chan->_pSrc += 2;
		}
		break;

	case 7:
		++pSrc;
		if (!chan->_pendingStop) {
			chan->_volumeFadeReload = *pSrc;
			chan->_volumeFadeStep = *++pSrc;
			chan->_volumeFadeCounter = 1;
		}

		chan->_pSrc += 3;
		break;

	case 8:
		chan->_transpose = (int8) * ++pSrc;
		chan->_pSrc += 2;
		break;

	case 9:
	{
		int v1 = *++pSrc;
		++pSrc;
		int v2 = (v1 - 1) & getRandomNumber();
		int v3 = pSrc[v2];
		int v4 = pSrc[v1];

		pSrc[v4 + v1 + 1] = v3;
		chan->_pSrc += v1 + 3;
		break;
	}

	case 10:
		++pSrc;
		if (chan->_pendingStop) {
			chan->_pSrc += 2;
		} else {
			chan->_volumeOffset = *pSrc >> 1;
			updateFlag = true;
			chan->_pSrc += 2;
		}
		break;

	case 11:
		chan->_patchAttenuation = *++pSrc;
		updateFlag = true;
		chan->_pSrc += 2;
		break;

	case 12:
		chan->_attenFadeReload = *++pSrc;
		chan->_attenFadeStep = *++pSrc;
		chan->_attenFadeCounter = 1;
		chan->_pSrc += 2;
		break;

	case 13:
		++pSrc;
		chan->_pSrc += 2;
		break;

	case 14:
		chan->_octaveTranspose = *++pSrc;
		chan->_pSrc += 2;
		break;

	default:
		break;
	}
}

/*-----------------------------------------------------------------------*/

const ASound1::CommandPtr ASound1::_commandList[42] = {
	&ASound1::command0, &ASound1::command1, &ASound1::command2, &ASound1::command3,
	&ASound1::command4, &ASound1::command5, &ASound1::command6, &ASound1::command7,
	&ASound1::command8, &ASound1::command9, &ASound1::command10, &ASound1::command11,
	&ASound1::command12, &ASound1::command13, &ASound1::command14, &ASound1::command15,
	&ASound1::command16, &ASound1::command17, &ASound1::command18, &ASound1::command19,
	&ASound1::command20, &ASound1::command21, &ASound1::command22, &ASound1::command23,
	&ASound1::command24, &ASound1::command25, &ASound1::command26, &ASound1::command27,
	&ASound1::command28, &ASound1::command29, &ASound1::command30, &ASound1::command31,
	&ASound1::command32, &ASound1::command33, &ASound1::command34, &ASound1::command35,
	&ASound1::command36, &ASound1::command37, &ASound1::command38, &ASound1::command39,
	&ASound1::command40, &ASound1::command41
};

ASound1::ASound1(Audio::Mixer *mixer, OPL::OPL *opl)
	: RexASound(mixer, opl, "asound.001", 0x1520, 0x17b0) {
	_cmd23Toggle = false;

	// Load sound samples
	auto samplesStream = getDataStream(0x12C);
	for (int i = 0; i < 98; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound1::command(int commandId, int param) {
	if (commandId > 41)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound1::command9() {
	playSound(0xC68);
	return 0;
}

int ASound1::command10() {
	byte *pData1 = loadData(0x130E);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x133E));
		_channels[2].load(loadData(0x14C6));
		_channels[3].load(loadData(0x14F4));
	}

	return 0;
}

int ASound1::command11() {
	command111213();
	_channels[0]._volumeOffset = 0;
	_channels[1]._volumeOffset = 0;
	return 0;
}

int ASound1::command12() {
	command111213();
	_channels[0]._volumeOffset = 40;
	_channels[1]._volumeOffset = 0;
	return 0;
}

int ASound1::command13() {
	command111213();
	_channels[0]._volumeOffset = 40;
	_channels[1]._volumeOffset = 50;
	return 0;
}

int ASound1::command14() {
	playSound(0x1216);
	return 0;
}

int ASound1::command15() {
	byte *pData1 = loadData(0x1524);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[4].load(pData1);
		_channels[5].load(loadData(0x15BC));
		_channels[6].load(loadData(0x161A));
		_channels[7].load(loadData(0x1678));
		_channels[8].load(loadData(0x16A2));
	}

	return 0;
}

int ASound1::command16() {
	playSound(0xC74);
	return 0;
}

int ASound1::command17() {
	playSound(0xE9A);
	return 0;
}

int ASound1::command18() {
	command1();
	playSound(0xCA6);
	return 0;
}

int ASound1::command19() {
	command1();
	playSound(0xCBA);
	return 0;
}

int ASound1::command20() {
	byte *pData = loadData(0xD18);
	if (!isSoundActive(pData))
		playSoundData(pData);
	return 0;
}

int ASound1::command21() {
	playSound(0xD04);
	return 0;
}

int ASound1::command22() {
	byte *pData = loadData(0xD34);
	pData[6] = (getRandomNumber() & 7) + 85;

	if (!isSoundActive(pData))
		playSoundData(pData);

	return 0;
}

int ASound1::command23() {
	_cmd23Toggle = !_cmd23Toggle;
	playSound(_cmd23Toggle ? 0xD3E : 0xD46);
	return 0;
}

int ASound1::command24() {
	playSound(0xD4E);
	playSound(0xD60);
	playSound(0xD74);
	return 0;
}

int ASound1::command25() {
	byte *pData = loadData(0xD82);
	if (!isSoundActive(pData))
		playSoundData(pData);

	return 0;
}

int ASound1::command26() {
	byte *pData = loadData(0xEEC);
	pData[5] = (command2627293032() + 0x7F) & 0xFF;

	if (!isSoundActive(pData))
		_channels[6].load(pData);

	return 0;
}

int ASound1::command27() {
	byte *pData = loadData(0xEE2);
	pData[5] = (command2627293032() + 0x40) & 0xFF;

	if (!isSoundActive(pData))
		_channels[7].load(pData);

	return 0;
}

int ASound1::command28() {
	playSound(0xD92);
	return 0;
}

int ASound1::command29() {
	byte *pData = loadData(0xC82);
	byte v = (command2627293032() + 0x40) & 0xFF;
	pData[7] = pData[13] = pData[21] = pData[27] = v;

	if (!isSoundActive(pData))
		playSoundData(pData, 0);

	return 0;
}

int ASound1::command30() {
	byte *pData = loadData(0xEA6);
	pData[7] = (command2627293032() + 0x40) & 0xFF;

	if (!isSoundActive(pData))
		playSoundData(pData, 0);

	return 0;
}

int ASound1::command31() {
	byte *pData = loadData(0xDAE);
	if (!isSoundActive(pData))
		playSoundData(pData);

	return 0;
}

int ASound1::command32() {
	byte *pData = loadData(0xEB4);
	int v = command2627293032() + 0x40;
	pData[9] = pData[17] = pData[25] = pData[33] = v & 0xFF;
	pData[11] = pData[19] = pData[27] = pData[35] = v >> 8;

	if (!isSoundActive(pData))
		playSoundData(pData, 0);

	return 0;
}

int ASound1::command33() {
	playSound(0xDBC);
	playSound(0xDC6);
	return 0;
}

int ASound1::command34() {
	int v = getRandomNumber() & 0x20;
	if (!v)
		v = 0x60;

	byte *pData = loadData(0xDD0);
	pData[8] = pData[15] = v;
	playSoundData(pData);
	return 0;
}

int ASound1::command35() {
	playSound(0xDE6);
	return 0;
}

int ASound1::command36() {
	playSound(0xE10);
	command34();

	return 0;
}

int ASound1::command37() {
	playSound(0xE1A);
	return 0;
}

int ASound1::command38() {
	playSound(0xE28);
	return 0;
}

int ASound1::command39() {
	byte *pData1 = loadData(0x16CC);
	if (!isSoundActive(pData1)) {
		_channels[5].load(pData1);
		_channels[6].load(loadData(0x171E));
		_channels[7].load(loadData(0x173C));
		_channels[8].load(loadData(0x1764));
	}
	return 0;
}

int ASound1::command40() {
	playSound(0xDF6);
	return 0;
}

int ASound1::command41() {
	playSound(0xC32);
	playSound(0xC54);
	return 0;
}

void ASound1::command111213() {
	byte *pData1 = loadData(0xEF6);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x108E));
		_channels[2].load(loadData(0x1198));
		_channels[2].load(loadData(0x11DA));
	}
}

int ASound1::command2627293032() {
	return (_commandParam > 0x40) ? _commandParam - 0x40 : _commandParam & 0xff00;
}


/*-----------------------------------------------------------------------*/

const ASound2::CommandPtr ASound2::_commandList[44] = {
	&ASound2::command0, &ASound2::command1, &ASound2::command2, &ASound2::command3,
	&ASound2::command4, &ASound2::command5, &ASound2::command6, &ASound2::command7,
	&ASound2::command8, &ASound2::command9, &ASound2::command10, &ASound2::command11,
	&ASound2::command12, &ASound2::command13, &ASound2::command14, &ASound2::command15,
	&ASound2::command16, &ASound2::command17, &ASound2::command18, &ASound2::command19,
	&ASound2::command20, &ASound2::command21, &ASound2::command22, &ASound2::command23,
	&ASound2::command24, &ASound2::command25, &ASound2::command26, &ASound2::command27,
	&ASound2::command28, &ASound2::command29, &ASound2::command30, &ASound2::command31,
	&ASound2::command32, &ASound2::command33, &ASound2::command34, &ASound2::command35,
	&ASound2::command36, &ASound2::command37, &ASound2::command38, &ASound2::command39,
	&ASound2::command40, &ASound2::command41, &ASound2::command42, &ASound2::command43
};

ASound2::ASound2(Audio::Mixer *mixer, OPL::OPL *opl) :
		RexASound(mixer, opl, "asound.002", 0x15E0, 0x4b70) {
	_command12Param = 0xFD;

	// Load sound samples
	auto samplesStream = getDataStream(0x144);
	for (int i = 0; i < 164; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound2::command(int commandId, int param) {
	if (commandId > 43)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound2::command0() {
	_command12Param = 0xFD;
	return ASound::command0();
}

int ASound2::command9() {
	byte *pData1 = loadData(0x1094);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[2].load(loadData(0x123E));

		command9Randomize();
		_channels[1].load(loadData(0x120C));
	}
	return 0;
}

void ASound2::command9Randomize() {
	// Randomization
	int v;
	while (((v = getRandomNumber()) & 0x3F) > 36)
		;

	byte *pData = loadData(0x120C);
	command9Apply(pData, v + 20, -1);
	command9Apply(pData + 1, 10 - ((v + 1) / 6), 1);
}

void ASound2::command9Apply(byte *data, int val, int incr) {
	data += 8;
	for (int ctr = 0; ctr < 10; ++ctr, data += 4, val += incr) {
		*data = val;
	}
}

int ASound2::command10() {
	byte *pData1 = loadData(0x12C0);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x12FC));
		_channels[2].load(loadData(0x143A));
	}

	return 0;
}

int ASound2::command11() {
	byte *pData = loadData(0x14A8);
	if (!isSoundActive(pData)) {
		playSoundData(pData);
		playSoundData(loadData(0x1552));
		playSoundData(loadData(0x1C5C));
		playSoundData(loadData(0x1F28));
	}

	return 0;
}

int ASound2::command12() {
	_command12Param += 26;
	byte v = _command12Param & 0x7f;

	byte *pData = loadData(0x4A5E);
	pData[5] = pData[20] = v;
	playSoundData(pData);

	pData = loadData(0x4A84);
	pData[5] = pData[18] = v;
	playSoundData(pData);

	return 0;
}

int ASound2::command13() {
	playSoundData(loadData(0x4AA2));
	playSoundData(loadData(0x4AB6));

	return 0;
}

int ASound2::command14() {
	playSound(0x4ACA);
	playSound(0x4AF2);

	return 0;
}

int ASound2::command15() {
	byte *pData1 = loadData(0x1F92);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x23C4);
		playSound(0x27DE);
		playSound(0x2818);
		playSound(0x2AE0);
	}

	return 0;
}

int ASound2::command16() {
	byte *pData1 = loadData(0x3960);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x3A78);
		playSound(0x3B72);
		playSound(0x3CC4);
		playSound(0x3EAC);
		playSound(0x3F14);
	}

	return 0;
}

int ASound2::command17() {
	byte *pData1 = loadData(0x3F7C);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x412C);
		playSound(0x42D2);
		playSound(0x447A);
	}

	return 0;
}

static const int command18_list[16][2] = {
	{ 0x337C, 28 }, { 0x3398, 26 }, { 0x33B2, 26 }, { 0x33CC, 26 },
	{ 0x33E6, 56 }, { 0x341E, 46 }, { 0x344C, 56 }, { 0x3484, 22 },
	{ 0x349A, 38 }, { 0x34C0, 62 }, { 0x34FE, 26 }, { 0x3518, 26 },
	{ 0x3532, 26 }, { 0x354C, 26 }, { 0x3566, 32 }, { 0x3586, 24 }
};

int ASound2::command18() {
	if (_channels[3]._activeCount == 0) {
		int idx = (getRandomNumber() & 0x1E) >> 1;
		byte *pData = loadData(command18_list[idx][0]);
		_channels[3].load(pData);
	}

	return 0;
}

int ASound2::command19() {
	byte *pData1 = loadData(0x2BE0);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x2D4E);
		playSound(0x2F1A);
		playSound(0x3024);
		playSound(0x316C);
		playSound(0x320E);
	}

	return 0;
}

int ASound2::command20() {
	playSound(0x4A36);

	return 0;
}

int ASound2::command21() {
	playSound(0x49DE);
	playSound(0x49EE);
	playSound(0x49FF);

	return 0;
}

int ASound2::command22() {
	playSound(0x4A0E);
	playSound(0x4A26);

	return 0;
}

int ASound2::command23() {
	playSound(0x49B6);

	return 0;
}

int ASound2::command24() {
	playSound(0x49C6);

	return 0;
}

int ASound2::command25() {
	playSound(0x49AC);

	return 0;
}

int ASound2::command26() {
	playSound(0x498A);
	playSound(0x4998);

	return 0;
}

int ASound2::command27() {
	playSound(0x4912);
	playSound(0x4962);

	return 0;
}

int ASound2::command28() {
	playSound(0x48E8);
	playSound(0x4904);

	return 0;
}

int ASound2::command29() {
	playSound(0x48B2);

	return 0;
}

int ASound2::command30() {
	playSound(0x4870);
	playSound(0x4886);
	playSound(0x489C);

	return 0;
}

int ASound2::command31() {
	playSound(0x482E);
	playSound(0x4844);
	playSound(0x489C);

	return 0;
}

int ASound2::command32() {
	playSound(0x46E8);

	return 0;
}

int ASound2::command33() {
	playSound(0x46D8);

	return 0;
}

int ASound2::command34() {
	playSound(0x46C8);

	return 0;
}

int ASound2::command35() {
	playSound(0x46B2);

	return 0;
}

int ASound2::command36() {
	playSound(0x4624);

	return 0;
}

int ASound2::command37() {
	playSound(0x4674);
	playSound(0x4688);
	playSound(0x46A8);

	return 0;
}

int ASound2::command38() {
	byte *pData1 = loadData(0x359E);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x3668);
		playSound(0x3744);
		playSound(0x37C0);
		playSound(0x3862);
		playSound(0x38B0);
	}

	return 0;
}

int ASound2::command39() {
	byte *pData = loadData(0x466A);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound2::command40() {
	playSound(0x4634);
	playSound(0x4656);

	return 0;
}

int ASound2::command41() {
	playSound(0x48C8);

	return 0;
}

int ASound2::command42() {
	playSound(0x46F2);
	playSound(0x478E);

	return 0;
}

int ASound2::command43() {
	playSound(0x4B1C);
	playSound(0x4B44);

	return 0;
}

/*-----------------------------------------------------------------------*/

const ASound3::CommandPtr ASound3::_commandList[61] = {
	&ASound3::command0, &ASound3::command1, &ASound3::command2, &ASound3::command3,
	&ASound3::command4, &ASound3::command5, &ASound3::command6, &ASound3::command7,
	&ASound3::command8, &ASound3::command9, &ASound3::command10, &ASound3::command11,
	&ASound3::nullCommand, &ASound3::command13, &ASound3::command14, &ASound3::command15,
	&ASound3::command16, &ASound3::command17, &ASound3::command18, &ASound3::command19,
	&ASound3::command20, &ASound3::command21, &ASound3::command22, &ASound3::command23,
	&ASound3::command24, &ASound3::command25, &ASound3::command26, &ASound3::command27,
	&ASound3::command28, &ASound3::command29, &ASound3::command30, &ASound3::command31,
	&ASound3::command32, &ASound3::command33, &ASound3::command34, &ASound3::command35,
	&ASound3::command36, &ASound3::command37, &ASound3::command38, &ASound3::command39,
	&ASound3::command40, &ASound3::command41, &ASound3::command42, &ASound3::command43,
	&ASound3::command44, &ASound3::command45, &ASound3::command46, &ASound3::command47,
	&ASound3::nullCommand, &ASound3::command49, &ASound3::command50, &ASound3::command51,
	&ASound3::nullCommand, &ASound3::nullCommand, &ASound3::nullCommand, &ASound3::nullCommand,
	&ASound3::nullCommand, &ASound3::command57, &ASound3::nullCommand, &ASound3::command59,
	&ASound3::command60
};

ASound3::ASound3(Audio::Mixer *mixer, OPL::OPL *opl) :
		RexASound(mixer, opl, "asound.003", 0x15B0, 0x5020) {
	_command39Flag = false;

	// Load sound samples
	auto samplesStream = getDataStream(0x122);
	for (int i = 0; i < 192; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound3::command(int commandId, int param) {
	if (commandId > 60)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound3::command9() {
	AdlibChannel::_channelsEnabled = _commandParam != 0;

	return 0;
}

int ASound3::command10() {
	byte *pData1 = loadData(0x13EA);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0X14E8));
		_channels[2].load(loadData(0x16AC));
		_channels[3].load(loadData(0x1838));
		_channels[4].load(loadData(0x18AE));
	}

	return 0;
}

int ASound3::command11() {
	byte *pData1 = loadData(0x2B84);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x2DD8));
		_channels[2].load(loadData(0x300A));
		_channels[3].load(loadData(0x36A8));
		_channels[4].load(loadData(0x3AF4));
		_channels[5].load(loadData(0x3C98));
	}

	return 0;
}

int ASound3::command13() {
	byte *pData1 = loadData(0x4470);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x44B0);
		playSound(0x44F0);
		playSound(0x4530);
		playSound(0x4570);
		playSound(0X45b0);
	}

	return 0;
}

int ASound3::command14() {
	byte *pData1 = loadData(0X45F0);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x4614);
		playSound(0x4638);
		playSound(0x465C);
		playSound(0x467C);
		playSound(0x46C8);
	}

	return 0;
}

int ASound3::command15() {
	_channels[3].load(loadData(0x36A8));
	_channels[4].load(loadData(0x3AF4));
	_channels[5].load(loadData(0x3C98));

	_channels[3]._field20 = 0xDD;
	_channels[4]._field20 = 0xDD;
	_channels[5]._field20 = 0xDD;

	return 0;
}

int ASound3::command16() {
	byte *pData1 = loadData(0x4712);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x48A0));
		_channels[2].load(loadData(0x4A02));
		_channels[3].load(loadData(0x4B9C));
	}

	return 0;
}

int ASound3::command17() {
	byte *pData1 = loadData(0x18F8);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x1A88));
		_channels[2].load(loadData(0x1D30));
		_channels[3].load(loadData(0x1F0E));
		_channels[4].load(loadData(0x2388));
		_channels[5].load(loadData(0x2776));
	}

	return 0;
}

int ASound3::command18() {
	byte *pData1 = loadData(0x4284);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x4312);
		playSound(0x43BE);
		playSound(0x4416);
	}

	return 0;
}

int ASound3::command19() {
	playSound(0x4F6);

	return 0;
}

int ASound3::command20() {
	playSound(0x4F1C);

	return 0;
}

int ASound3::command21() {
	playSound(0x4F2E);

	return 0;
}

int ASound3::command22() {
	playSound(0x4F36);

	return 0;
}

int ASound3::command23() {
	playSound(0x4F50);
	playSound(0x4F46);

	return 0;
}

int ASound3::command24() {
	// WORKAROUND: Original calls isSoundActive without loading data pointer
	byte *pData = loadData(0x4EFC);
	if (!isSoundActive(pData)) {
		int v;
		while ((v = (getRandomNumber() & 0x3F)) > 45)
			;

		pData[6] = v + 19;
		playSoundData(pData);
	}

	return 0;
}

int ASound3::command25() {
	playSound(0x4EE6);

	return 0;
}

int ASound3::command26() {
	playSound(0x4F5A);

	return 0;
}

int ASound3::command27() {
	playSound(0x4DA2);
	playSound(0x4DC4);

	return 0;
}

int ASound3::command28() {
	playSound(0x4F72);
	playSound(0x4F72);

	return 0;
}

int ASound3::command29() {
	playSound(0x4F72);
	playSound(0x4F72);

	return 0;
}

int ASound3::command30() {
	playSound(0x4E5A);
	playSound(0x4E70);
	playSound(0x4E86);

	return 0;
}

int ASound3::command31() {
	playSound(0x4F7C);

	return 0;
}

int ASound3::command32() {
	playSound(0x4ED2);

	return 0;
}

int ASound3::command33() {
	playSound(0x4EC2);

	return 0;
}

int ASound3::command34() {
	playSound(0x4EB2);

	return 0;
}

int ASound3::command35() {
	playSound(0x4E9C);

	return 0;
}

int ASound3::command36() {
	playSound(0x4D2C);

	return 0;
}

int ASound3::command37() {
	playSound(0x4E1E);
	playSound(0x4E32);
	playSound(0x4E50);

	return 0;
}

int ASound3::command38() {
	playSound(0x4FAC);

	return 0;
}

int ASound3::command39() {
	_command39Flag = !_command39Flag;
	if (_command39Flag) {
		playSound(0x4FD0);
	} else {
		playSound(0x4FD8);
	}

	return 0;
}

int ASound3::command40() {
	_command39Flag = !_command39Flag;
	if (_command39Flag) {
		playSound(0x4EE0);
	} else {
		playSound(0x4EE8);
	}

	return 0;
}

int ASound3::command41() {
	playSound(0x4F08);

	return 0;
}

int ASound3::command42() {
	playSound(0x4DD8);
	playSound(0x4DF4);

	return 0;
}

int ASound3::command43() {
	playSound(0x4FB6);
	playSound(0x4FC2);

	return 0;
}

int ASound3::command44() {
	playSound(0x4FFE);

	return 0;
}

int ASound3::command45() {
	playSound(0x500C);

	return 0;
}

int ASound3::command46() {
	playSound(0x4D78);
	playSound(0x4D86);
	playSound(0x4D94);

	return 0;
}

int ASound3::command47() {
	playSound(0x4D62);
	playSound(0x4D6A);

	return 0;
}

int ASound3::command49() {
	playSound(0x4D62);
	playSound(0x4D6A);

	return 0;
}

int ASound3::command50() {
	playSound(0x4D3C);
	playSound(0x4D4A);
	playSound(0x4D58);

	return 0;
}

int ASound3::command51() {
	playSound(0x4FF0);

	return 0;
}

int ASound3::command57() {
	byte *pData = loadData(0x4EDC);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound3::command59() {
	playSound(0x4F62);

	return 0;
}

int ASound3::command60() {
	playSound(0x4FA4);

	return 0;
}

/*-----------------------------------------------------------------------*/

const ASound4::CommandPtr ASound4::_commandList[61] = {
	&ASound4::command0, &ASound4::command1, &ASound4::command2, &ASound4::command3,
	&ASound4::command4, &ASound4::command5, &ASound4::command6, &ASound4::command7,
	&ASound4::command8, &ASound4::nullCommand, &ASound4::command10, &ASound4::nullCommand,
	&ASound4::command12, &ASound4::nullCommand, &ASound4::nullCommand, &ASound4::nullCommand,
	&ASound4::nullCommand, &ASound4::nullCommand, &ASound4::nullCommand, &ASound4::command19,
	&ASound4::command20, &ASound4::command21, &ASound4::nullCommand, &ASound4::nullCommand,
	&ASound4::command24, &ASound4::nullCommand, &ASound4::nullCommand, &ASound4::command27,
	&ASound4::nullCommand, &ASound4::nullCommand, &ASound4::command30, &ASound4::nullCommand,
	&ASound4::command32, &ASound4::command33, &ASound4::command34, &ASound4::command35,
	&ASound4::command36, &ASound4::command37, &ASound4::command38, &ASound4::nullCommand,
	&ASound4::nullCommand, &ASound4::nullCommand, &ASound4::nullCommand, &ASound4::command43,
	&ASound4::nullCommand, &ASound4::nullCommand, &ASound4::nullCommand, &ASound4::nullCommand,
	&ASound4::nullCommand, &ASound4::nullCommand, &ASound4::nullCommand, &ASound4::nullCommand,
	&ASound4::command52, &ASound4::command53, &ASound4::command54, &ASound4::command55,
	&ASound4::command56, &ASound4::command57, &ASound4::command58, &ASound4::command59,
	&ASound4::command60
};

ASound4::ASound4(Audio::Mixer *mixer, OPL::OPL *opl) :
		RexASound(mixer, opl, "asound.004", 0x14F0, 0x2930) {
	// Load sound samples
	auto samplesStream = getDataStream(0x122);
	for (int i = 0; i < 210; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound4::command(int commandId, int param) {
	if (commandId > 60)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound4::command10() {
	byte *pData = loadData(0x22AA);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x23A8));
		_channels[2].load(loadData(0x256C));
		_channels[3].load(loadData(0x26F8));
		_channels[4].load(loadData(0x276E));
	}

	return 0;
}

int ASound4::command12() {
	byte *pData = loadData(0x16A8);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x18CE));
		_channels[2].load(loadData(0x1A88));
		_channels[3].load(loadData(0x1BB2));
		_channels[4].load(loadData(0x1D14));
		_channels[4].load(loadData(0x1F50));
	}

	int v = (_commandParam > 0x40) ? _commandParam - 0x40 : 0;
	v += 0xB5;
	for (int channelNum = 0; channelNum < 6; ++channelNum)
		_channels[channelNum]._field20 = v;

	return 0;
}

int ASound4::command19() {
	playSound(0x28EC);

	return 0;
}

int ASound4::command20() {
	playSound(0x28E2);

	return 0;
}

int ASound4::command21() {
	playSound(0x27C0);

	return 0;
}

int ASound4::command24() {
	int v;
	while ((v = (getRandomNumber() & 0x3F)) > 45)
		;

	byte *pData = loadData(0x28D6);
	pData[6] = v + 19;
	playSoundData(pData);

	return 0;
}

int ASound4::command27() {
	playSound(0x27D8);
	playSound(0x27FA);

	return 0;
}

int ASound4::command30() {
	playSound(0x284A);
	playSound(0x2860);
	playSound(0x2876);

	return 0;
}

int ASound4::command32() {
	playSound(0x28C2);

	return 0;
}

int ASound4::command33() {
	playSound(0x28B2);

	return 0;
}

int ASound4::command34() {
	playSound(0x28A2);

	return 0;
}

int ASound4::command35() {
	playSound(0x288C);

	return 0;
}

int ASound4::command36() {
	playSound(0x27C8);

	return 0;
}

int ASound4::command37() {
	playSound(0x280E);
	playSound(0x2822);
	playSound(0x2840);

	return 0;
}

int ASound4::command38() {
	playSound(0x2904);

	return 0;
}

int ASound4::command43() {
	playSound(0x290E);
	playSound(0x291A);

	return 0;
}

int ASound4::command52() {
	byte *pData = loadData(0x23A8);
	if (_channels[1]._ptr1 == pData) {
		pData = loadData(0x146E);
		if (!isSoundActive(pData)) {
			_channels[0].load(pData);
			_channels[1]._field20 = 0xD8;
			_channels[2]._field20 = 0xD8;
		}
	}

	return 0;
}

int ASound4::command53() {
	method1();
	_channels[0]._field20 = 0;

	return 0;
}

int ASound4::command54() {
	method1();
	_channels[1]._field20 = 0;
	_channels[2]._field20 = 0;

	return 0;
}

int ASound4::command55() {
	method1();
	_channels[3]._field20 = 0;
	_channels[4]._field20 = 0;

	return 0;
}

int ASound4::command56() {
	method1();
	_channels[5]._field20 = 0;

	return 0;
}

int ASound4::command57() {
	int v = (getRandomNumber() & 7) + 85;
	byte *pData = loadData(0x28CC);
	pData[6] = v;
	playSoundData(pData);

	return 0;
}

int ASound4::command58() {
	byte *pData = loadData(0x146E);
	if (_channels[1]._ptr1 == pData) {
		_channels[0].load(loadData(0x22AA));
		_channels[1]._field20 = 0;
		_channels[2]._field20 = 0;
	}

	return 0;
}

int ASound4::command59() {
	playSound(0x28F4);

	return 0;
}

int ASound4::command60() {
	playSound(0x28FC);

	return 0;
}

void ASound4::method1() {
	byte *pData = loadData(0x2180);
	if (!isSoundActive(pData)) {
		command1();

		_channels[0].load(pData);
		_channels[1].load(loadData(0x21BA));
		_channels[2].load(loadData(0x21EA));
		_channels[3].load(loadData(0x221C));
		_channels[4].load(loadData(0x2244));
		_channels[5].load(loadData(0x2260));

		for (int channel = 0; channel < 6; ++channel)
			_channels[channel]._field20 = 0xB5;
	}
}


/*-----------------------------------------------------------------------*/

const ASound5::CommandPtr ASound5::_commandList[42] = {
	&ASound5::command0, &ASound5::command1, &ASound5::command2, &ASound5::command3,
	&ASound5::command4, &ASound5::command5, &ASound5::command6, &ASound5::command7,
	&ASound5::command8, &ASound5::command9, &ASound5::command10, &ASound5::command11,
	&ASound5::command11, &ASound5::command13, &ASound5::command14, &ASound5::command15,
	&ASound5::command16, &ASound5::command17, &ASound5::command18, &ASound5::command19,
	&ASound5::command20, &ASound5::command21, &ASound5::command22, &ASound5::command23,
	&ASound5::command11, &ASound5::command11, &ASound5::command26, &ASound5::command27,
	&ASound5::command28, &ASound5::command29, &ASound5::command30, &ASound5::command31,
	&ASound5::command32, &ASound5::command33, &ASound5::command34, &ASound5::command35,
	&ASound5::command36, &ASound5::command37, &ASound5::command38, &ASound5::command39,
	&ASound5::command40, &ASound5::command41
};

ASound5::ASound5(Audio::Mixer *mixer, OPL::OPL *opl) :
		RexASound(mixer, opl, "asound.005", 0x15E0, 0x2200) {
	// Load sound samples
	auto samplesStream = getDataStream(0x144);
	for (int i = 0; i < 164; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound5::command(int commandId, int param) {
	if (commandId > 41)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound5::command9() {
	byte *pData = loadData(0x2114);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound5::command10() {
	playSound(0x211E);

	return 0;
}

int ASound5::command11() {
	playSound(0x2016);

	return 0;
}

int ASound5::command13() {
	playSound(0x2154);

	return 0;
}

int ASound5::command14() {
	playSound(0x21DC);

	return 0;
}

int ASound5::command15() {
	byte *pData = loadData(0x21DC);
	if (_channels[0]._ptr1 == pData) {
		pData = loadData(0x1F2);
		_channels[0]._soundData = pData;
		_channels[0]._innerLoopCount = 1;
		_channels[0]._outerLoopCount = 1;
	}

	return 0;
}

int ASound5::command16() {
	playSound(0x214C);

	return 0;
}

int ASound5::command17() {
	playSound(0x2142);

	return 0;
}

int ASound5::command18() {
	playSound(0x21A2);

	return 0;
}

int ASound5::command19() {
	playSound(0x2190);

	return 0;
}

int ASound5::command20() {
	playSound(0x2170);

	return 0;
}

int ASound5::command21() {
	playSound(0x2180);

	return 0;
}

int ASound5::command22() {
	playSound(0x2168);

	return 0;
}

int ASound5::command23() {
	playSound(0x215E);

	return 0;
}

int ASound5::command26() {
	playSound(0x21B8);

	return 0;
}

int ASound5::command27() {
	playSound(0x21C4);

	return 0;
}

int ASound5::command28() {
	playSound(0x2020);
	playSound(0x4904);

	return 0;
}

int ASound5::command29() {
	byte *pData = loadData(0x17C);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1864));
		_channels[2].load(loadData(0x1994));
		_channels[3].load(loadData(0x1864));
		_channels[4].load(loadData(0x1994));
	}

	return 0;
}

int ASound5::command30() {
	playSound(0x2092);
	playSound(0x20A8);
	playSound(0x20BE);

	return 0;
}

int ASound5::command31() {
	playSound(0x2128);
	playSound(0x2134);

	return 0;
}

int ASound5::command32() {
	playSound(0x210A);

	return 0;
}

int ASound5::command33() {
	playSound(0x20FA);

	return 0;
}

int ASound5::command34() {
	playSound(0x20EA);

	return 0;
}

int ASound5::command35() {
	playSound(0x20D4);

	return 0;
}

int ASound5::command36() {
	playSound(0x2006);

	return 0;
}

int ASound5::command37() {
	playSound(0x2056);
	playSound(0x206A);
	playSound(0x2088);

	return 0;
}

int ASound5::command38() {
	byte *pData1 = loadData(0x14F2);
	if (_channels[3]._ptr1 == pData1) {
		_channels[3].load(loadData(0x1A72));
		_channels[3].load(loadData(0x1C7C));
	}

	return 0;
}

int ASound5::command39() {
	playSound(0x1FEE);

	return 0;
}

int ASound5::command40() {
	playSound(0x1FF6);

	return 0;
}

int ASound5::command41() {
	byte *pData1 = loadData(0x14F2);
	if (!isSoundActive(pData1)) {
		byte *pData2 = loadData(0x1A72);
		if (_channels[3]._ptr1 == pData2) {
			_channels[3].load(pData1);
			_channels[4].load(loadData(0x1FE6));
		}
	}

	return 0;
}

/*-----------------------------------------------------------------------*/

const ASound6::CommandPtr ASound6::_commandList[30] = {
	&ASound6::command0, &ASound6::command1, &ASound6::command2, &ASound6::command3,
	&ASound6::command4, &ASound6::command5, &ASound6::command6, &ASound6::command7,
	&ASound6::command8, &ASound6::command9, &ASound6::command10, &ASound6::command11,
	&ASound6::command11, &ASound6::command13, &ASound6::command14, &ASound6::command15,
	&ASound6::command16, &ASound6::command17, &ASound6::command18, &ASound6::command19,
	&ASound6::command20, &ASound6::command21, &ASound6::command22, &ASound6::command23,
	&ASound6::command11, &ASound6::command11, &ASound6::nullCommand, &ASound6::nullCommand,
	&ASound6::nullCommand, &ASound6::command29
};

ASound6::ASound6(Audio::Mixer *mixer, OPL::OPL *opl) :
		RexASound(mixer, opl, "asound.006", 0x1390, 0x22d0) {
	// Load sound samples
	auto samplesStream = getDataStream(0x122);
	for (int i = 0; i < 200; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound6::command(int commandId, int param) {
	if (commandId > 29)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound6::command9() {
	byte *pData = loadData(0x2194);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound6::command10() {
	playSound(0x2224);

	return 0;
}

int ASound6::command11() {
	playSound(0x2202);

	return 0;
}

int ASound6::command12() {
	playSound(0x2246);

	return 0;
}

int ASound6::command13() {
	playSound(0x2298);

	return 0;
}

int ASound6::command14() {
	playSound(0x22B4);

	return 0;
}

int ASound6::command15() {
	playSound(0x219E);

	return 0;
}

int ASound6::command16() {
	playSound(0x21AA);
	playSound(0x21C0);

	return 0;
}

int ASound6::command17() {
	playSound(0x21CC);

	return 0;
}

int ASound6::command18() {
	playSound(0x2270);

	return 0;
}

int ASound6::command19() {
	playSound(0x2280);

	return 0;
}

int ASound6::command20() {
	playSound(0x223C);

	return 0;
}

int ASound6::command21() {
	playSound(0x224E);

	return 0;
}

int ASound6::command22() {
	playSound(0x2290);

	return 0;
}

int ASound6::command23() {
	playSound(0x215E);
	playSound(0x2180);

	return 0;
}

int ASound6::command24() {
	byte *pData1 = loadData(0x1D54);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x1F70));
		_channels[2].load(loadData(0x1FA4));
	}

	return 0;
}

int ASound6::command25() {
	playSound(0x2152);

	return 0;
}

int ASound6::command29() {
	byte *pData1 = loadData(0x149A);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x15D2));
		_channels[2].load(loadData(0x1702));
		_channels[3].load(loadData(0x17E0));
		_channels[4].load(loadData(0x19EA));
	}

	return 0;
}

/*-----------------------------------------------------------------------*/

const ASound7::CommandPtr ASound7::_commandList[38] = {
	&ASound7::command0, &ASound7::command1, &ASound7::command2, &ASound7::command3,
	&ASound7::command4, &ASound7::command5, &ASound7::command6, &ASound7::command7,
	&ASound7::command8, &ASound7::command9, &ASound7::nullCommand, &ASound7::nullCommand,
	&ASound7::nullCommand, &ASound7::nullCommand, &ASound7::nullCommand, &ASound7::command15,
	&ASound7::command16, &ASound7::command16, &ASound7::command18, &ASound7::command19,
	&ASound7::command20, &ASound7::command21, &ASound7::command22, &ASound7::command23,
	&ASound7::command24, &ASound7::command25, &ASound7::command26, &ASound7::command27,
	&ASound7::command28, &ASound7::nullCommand, &ASound7::command30, &ASound7::nullCommand,
	&ASound7::command32, &ASound7::command33, &ASound7::command34, &ASound7::command35,
	&ASound7::command36, &ASound7::command37
};

ASound7::ASound7(Audio::Mixer *mixer, OPL::OPL *opl) :
		RexASound(mixer, opl, "asound.007", 0x1460, 0x2cf0) {
	// Load sound samples
	auto samplesStream = getDataStream(0x122);
	for (int i = 0; i < 214; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound7::command(int commandId, int param) {
	if (commandId > 37)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound7::command9() {
	byte *pData1 = loadData(0x2992);
	if (!isSoundActive(pData1)) {
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x2A0C));
		_channels[2].load(loadData(0x2A58));
		_channels[3].load(loadData(0x2AD2));
	}

	return 0;
}

int ASound7::command15() {
	byte *pData = loadData(0x2B3E);
	pData[6] = (getRandomNumber() & 7) + 85;

	return 0;
}

int ASound7::command16() {
	playSound(0x2CE2);

	return 0;
}

int ASound7::command18() {
	playSound(0x2C94);

	return 0;
}

int ASound7::command19() {
	byte *pData1 = loadData(0x2C94);
	byte *pData2 = loadData(0x2CAA);
	if (_channels[8]._ptr1 == pData1 || _channels[8]._ptr1 == pData2) {
		_channels[8]._soundData = loadData(0x2CBA);
		_channels[8]._innerLoopCount = 1;
		_channels[8]._outerLoopCount = 1;
	}

	return 0;
}

int ASound7::command20() {
	playSound(0x2CD0);

	return 0;
}

int ASound7::command21() {
	playSound(0x2CC6);

	return 0;
}

int ASound7::command22() {
	playSound(0x2C08);

	return 0;
}

int ASound7::command23() {
	playSound(0x2B08);
	playSound(0x2B2A);

	return 0;
}

int ASound7::command24() {
	byte *pData1 = loadData(0x14C6);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x1556));
		_channels[2].load(loadData(0x15E8));
		_channels[3].load(loadData(0x1672));
		_channels[4].load(loadData(0x16EC));
	}

	return 0;
}

int ASound7::command25() {
	byte *pData1 = loadData(0x1DBE);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x1E74));
		_channels[2].load(loadData(0x1F2A));
		_channels[3].load(loadData(0x1FE4));
	}

	return 0;
}

int ASound7::command26() {
	byte *pData1 = loadData(0x20D8);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x2210));
		_channels[2].load(loadData(0x2340));
		_channels[3].load(loadData(0x241E));
		_channels[4].load(loadData(0x2628));
	}

	return 0;
}

int ASound7::command27() {
	byte *pData1 = loadData(0x1736);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x17D4));
		_channels[2].load(loadData(0x18F4));
		_channels[3].load(loadData(0x1A16));
		_channels[4].load(loadData(0x1BA2));
		_channels[5].load(loadData(0x1D1E));
	}

	return 0;
}

int ASound7::command28() {
	playSound(0x2CAA);

	return 0;
}

int ASound7::command30() {
	playSound(0x2B86);
	playSound(0x2B9C);
	playSound(0x2BB2);

	return 0;
}

int ASound7::command32() {
	playSound(0x2BFE);

	return 0;
}

int ASound7::command33() {
	playSound(0x2BEE);

	return 0;
}

int ASound7::command34() {
	playSound(0x2BDE);

	return 0;
}

int ASound7::command35() {
	playSound(0x2BC8);

	return 0;
}

int ASound7::command36() {
	playSound(0x2AF8);

	return 0;
}

int ASound7::command37() {
	playSound(0x2B48);
	playSound(0x2B5C);
	playSound(0x2B7C);

	return 0;
}

/*-----------------------------------------------------------------------*/

const ASound8::CommandPtr ASound8::_commandList[38] = {
	&ASound8::command0, &ASound8::command1, &ASound8::command2, &ASound8::command3,
	&ASound8::command4, &ASound8::command5, &ASound8::command6, &ASound8::command7,
	&ASound8::command8, &ASound8::command9, &ASound8::command10, &ASound8::command11,
	&ASound8::command12, &ASound8::command13, &ASound8::command14, &ASound8::command15,
	&ASound8::command16, &ASound8::command16, &ASound8::command18, &ASound8::command19,
	&ASound8::command20, &ASound8::command21, &ASound8::command22, &ASound8::command23,
	&ASound8::command24, &ASound8::command25, &ASound8::command26, &ASound8::command27,
	&ASound8::command28, &ASound8::command29, &ASound8::command30, &ASound8::command31,
	&ASound8::command32, &ASound8::command33, &ASound8::command34, &ASound8::command35,
	&ASound8::command36, &ASound8::command37
};

ASound8::ASound8(Audio::Mixer *mixer, OPL::OPL *opl) :
		RexASound(mixer, opl, "asound.008", 0x1490, 0x1810) {
	// Load sound samples
	auto samplesStream = getDataStream(0x122);
	for (int i = 0; i < 174; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound8::command(int commandId, int param) {
	if (commandId > 37)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound8::command9() {
	byte *pData = loadData(0x15BE);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound8::command10() {
	_channels[7].load(loadData(0x171c));
	_channels[8].load(loadData(0x1754));

	return 0;
}

int ASound8::command11() {
	playSound(0x17CA);

	return 0;
}

int ASound8::command12() {
	playSound(0x17D6);

	return 0;
}

int ASound8::command13() {
	playSound(0x1694);

	return 0;
}

int ASound8::command14() {
	_channels[8].load(loadData(0x169e));

	return 0;
}

int ASound8::command15() {
	byte *pData = loadData(0x169E);
	if (_channels[8]._ptr1 == pData) {
		_channels[8]._soundData = loadData(0x16B6);
		_channels[8]._innerLoopCount = 1;
		_channels[8]._outerLoopCount = 1;
	}

	return 0;
}

int ASound8::command16() {
	playSound(0x1686);

	return 0;
}

int ASound8::command17() {
	playSound(0x17EC);

	return 0;
}

int ASound8::command18() {
	playSound(0x17F8);

	return 0;
}

int ASound8::command19() {
	playSound(0x16D8);

	return 0;
}

int ASound8::command20() {
	playSound(0x16E0);

	return 0;
}

int ASound8::command21() {
	playSound(0x17E2);

	return 0;
}

int ASound8::command22() {
	_channels[6].load(loadData(0x178C));
	_channels[7].load(loadData(0x179A));
	_channels[8].load(loadData(0x17A8));

	return 0;
}

int ASound8::command23() {
	_channels[7].load(loadData(0x16E8));
	_channels[8].load(loadData(0x170A));

	return 0;
}

int ASound8::command24() {
	playSound(0x17B6);

	return 0;
}

int ASound8::command25() {
	playSound(0x17BE);

	return 0;
}

int ASound8::command26() {
	playSound(0x16C2);

	return 0;
}

int ASound8::command27() {
	playSound(0x1588);
	playSound(0x15AA);

	return 0;
}

int ASound8::command28() {
	byte *pData1 = loadData(0x114E);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[2].load(loadData(0x12F8));

		byte *pData = loadData(0x12C6);
		method1(pData);
		_channels[1].load(pData);
	}

	return 0;
}

void ASound8::method1(byte *pData) {
	int v;
	while ((v = (getRandomNumber() & 0x3F)) > 36)
		;

	adjustRange(pData, v + 20, -1);
	adjustRange(pData + 1, 10 - ((v + 1) / 6), 1);
}

void ASound8::adjustRange(byte *pData, byte v, int incr) {
	pData += 8;

	for (int i = 0; i < 10; ++i, pData += 4, v += incr) {
		*pData = v;
	}
}

int ASound8::command29() {
	byte *pData1 = loadData(0x137A);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x13B6));
		_channels[2].load(loadData(0x14F4));
	}

	return 0;
}

int ASound8::command30() {
	playSound(0x1644);
	playSound(0x165A);
	playSound(0x1670);

	return 0;
}

int ASound8::command31() {
	_channels[7].load(loadData(0x156A));

	return 0;
}

int ASound8::command32() {
	playSound(0x163A);

	return 0;
}

int ASound8::command33() {
	playSound(0x162A);

	return 0;
}

int ASound8::command34() {
	playSound(0x161A);

	return 0;
}

int ASound8::command35() {
	playSound(0x1604);

	return 0;
}

int ASound8::command36() {
	playSound(0x1578);

	return 0;
}

int ASound8::command37() {
	playSound(0x15C8);
	playSound(0x15DC);
	playSound(0X15FA);

	return 0;
}

/*-----------------------------------------------------------------------*/

const ASound9::CommandPtr ASound9::_commandList[52] = {
	&ASound9::command0, &ASound9::command1, &ASound9::command2, &ASound9::command3,
	&ASound9::command4, &ASound9::command5, &ASound9::command6, &ASound9::command7,
	&ASound9::command8, &ASound9::command9, &ASound9::command10, &ASound9::command11,
	&ASound9::command12, &ASound9::command13, &ASound9::command14, &ASound9::command15,
	&ASound9::command16, &ASound9::command17, &ASound9::command18, &ASound9::command19,
	&ASound9::command20, &ASound9::command21, &ASound9::command22, &ASound9::command23,
	&ASound9::command24, &ASound9::command25, &ASound9::command26, &ASound9::command27,
	&ASound9::command28, &ASound9::command29, &ASound9::command30, &ASound9::command31,
	&ASound9::command32, &ASound9::command33, &ASound9::command34, &ASound9::command35,
	&ASound9::command36, &ASound9::command37, &ASound9::command38, &ASound9::command39,
	&ASound9::command40, &ASound9::command41, &ASound9::command42, &ASound9::command43,
	&ASound9::command44_46, &ASound9::command45, &ASound9::command44_46, &ASound9::command47,
	&ASound9::command48, &ASound9::command49, &ASound9::command50, &ASound9::command51
};

ASound9::ASound9(Audio::Mixer *mixer, OPL::OPL *opl) :
		RexASound(mixer, opl, "asound.009", 0x16F0, 0x85a0) {
	_callbackCounter = _callbackPeriod = 0;
	_callbackFnPtr = nullptr;

	// Load sound samples
	auto samplesStream = getDataStream(0x50);
	for (int i = 0; i < 94; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound9::command(int commandId, int param) {
	if (commandId > 51)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

void ASound9::tickCallback() {
	if (!_callbackPeriod)
		return;
	if (--_callbackCounter)
		return;

	_callbackCounter = _callbackPeriod;
	if (_callbackFnPtr)
		(this->*_callbackFnPtr)();
}

int ASound9::command9() {
	_callbackCounter = 1848;
	_callbackPeriod = 84;
	_channels[0].load(loadData(0xAA4));
	_channels[1].load(loadData(0xE4C));
	_channels[2].load(loadData(0x1466));
	_channels[3].load(loadData(0x137E));
	_channels[4].load(loadData(0x1014));
	_channels[5].load(loadData(0x11C4));
	_channels[6].load(loadData(0XC7A));
	return 0;
}

int ASound9::command10() {
	_channels[0].load(loadData(0x1724));
	_channels[1].load(loadData(0x173C));
	_channels[2].load(loadData(0x1754));
	_channels[3].load(loadData(0x1768));
	_channels[4].load(loadData(0x177C));
	_channels[5].load(loadData(0x1790));
	return 0;
}

int ASound9::command11() {
	playSound(0x8232);
	playSound(0x82DA);
	return 0;
}

int ASound9::command12() {
	playSound(0x80DA);
	playSound(0x80E6);
	return 0;
}

int ASound9::command13() {
	playSound(0x80F2);
	playSound(0x8118);
	return 0;
}

int ASound9::command14() {
	playSound(0x81F6);
	return 0;
}

int ASound9::command15() {
	playSound(0x818A);
	playSound(0x81AA);
	return 0;
}

int ASound9::command16() {
	playSound(0x8022);
	playSound(0x8046);
	return 0;
}

int ASound9::command17() {
	command29();
	playSound(0x858C);
	return 0;
}

int ASound9::command18() {
	playSound(0x80C2);
	return 0;
}

int ASound9::command19() {
	playSound(0x80A0);
	return 0;
}

int ASound9::command20() {
	int v = (getRandomNumber() & 0x10) | 0x4D;
	byte *pData = loadData(0x8142);
	pData[4] = v & 0x7F;
	playSoundData(pData);
	return 0;
}

int ASound9::command21() {
	playSound(0x815A);
	return 0;
}

int ASound9::command22() {
	playSound(0x816A);
	return 0;
}

int ASound9::command23() {
	playSound(0x814A);
	return 0;
}

int ASound9::command24() {
	playSound(0x7FE2);
	return 0;
}

int ASound9::command25() {
	playSound(0x8004);
	return 0;
}

int ASound9::command26() {
	_channels[6].load(loadData(0x8384));
	_channels[7].load(loadData(0x8420));
	return 0;
}

int ASound9::command27() {
	playSound(0x84C0);
	return 0;
}

int ASound9::command28() {
	playSound(0x81CA);
	return 0;
}

int ASound9::command29() {
	playSound(0x81D4);
	return 0;
}

int ASound9::command30() {
	playSound(0x817A);
	return 0;
}

int ASound9::command31() {
	playSound(0x820C);
	playSound(0x821A);
	return 0;
}

int ASound9::command32() {
	playSound(0x8070);
	return 0;
}

int ASound9::command33() {
	playSound(0x8078);
	playSound(0x8088);
	return 0;
}

int ASound9::command34() {
	_callbackCounter = _callbackPeriod = 96;

	*loadData(0x469B) = 2;
	*loadData(0x57EF) = 2;
	*loadData(0x6267) = 2;

	_channels[0].load(loadData(0x17A4));
	_channels[1].load(loadData(0x1CBE));
	_channels[2].load(loadData(0x2672));
	_channels[3].load(loadData(0x3336));
	_channels[4].load(loadData(0x469E));
	_channels[5].load(loadData(0x57F2));

	return 0;
}

int ASound9::command35() {
	playSound(0x854C);
	return 0;
}

int ASound9::command36() {
	playSound(0x81DE);
	playSound(0x81E8);
	return 0;
}

int ASound9::command37() {
	byte *pData = loadData(0x8098);
	int v = getRandomNumber();
	if ((v &= 0x40) != 0)
		v |= 8;
	else
		v += 0x4A;

	pData[6] = v;
	playSoundData(pData);
	return 0;
}

int ASound9::command38() {
	_callbackFnPtr = &ASound9::loadCommand38;
	return 0;
}

void ASound9::loadCommand38() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0xBA4));
	_channels[1].load(loadData(0xF1E));
	_channels[2].load(loadData(0x15C4));
	_channels[3].load(loadData(0x1442));
	_channels[4].load(loadData(0x111A));
	_channels[5].load(loadData(0x12D6));
	_channels[6].load(loadData(0xDBA));
}

int ASound9::command39() {
	_callbackFnPtr = &ASound9::loadCommand39;
	return 0;
}

void ASound9::loadCommand39() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0xC1C));
	_channels[1].load(loadData(0xFB6));
	_channels[2].load(loadData(0x16C4));
	_channels[3].load(loadData(0x1464));
	_channels[4].load(loadData(0x11C2));
	_channels[5].load(loadData(0x137C));
	_channels[6].load(loadData(0xE00));
}

int ASound9::command40() {
	_callbackFnPtr = &ASound9::loadCommand40;
	return 0;
}

void ASound9::loadCommand40() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x1958));
	_channels[1].load(loadData(0x1F52));
	_channels[2].load(loadData(0x2C02));
	_channels[3].load(loadData(0x3BE0));
	_channels[4].load(loadData(0x4BCA));
	_channels[5].load(loadData(0x5BA6));
}

int ASound9::command41() {
	_callbackFnPtr = &ASound9::loadCommand41;
	return 0;
}

void ASound9::loadCommand41() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x19E4));
	_channels[1].load(loadData(0x1FFE));
	_channels[2].load(loadData(0x2CB8));
	_channels[3].load(loadData(0x3DC4));
	_channels[4].load(loadData(0x4F6C));
	_channels[5].load(loadData(0x5F20));
}

int ASound9::command42() {
	_callbackFnPtr = &ASound9::loadCommand42;
	return 0;
}

void ASound9::loadCommand42() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x1B3A));
	_channels[1].load(loadData(0x2506));
	_channels[2].load(loadData(0x3194));
	_channels[3].load(loadData(0x42E2));
	_channels[4].load(loadData(0x548A));
	_channels[5].load(loadData(0x6036));
}

int ASound9::command43() {
	_callbackCounter = _callbackPeriod = 80;
	_channels[0].load(loadData(0x626A));
	_channels[1].load(loadData(0x67F2));
	_channels[2].load(loadData(0x6CFE));
	_channels[3].load(loadData(0x7146));

	return 0;
}

int ASound9::command44_46() {
	_callbackFnPtr = &ASound9::loadCommand44_46;
	return 0;
}

void ASound9::loadCommand44_46() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x62C4));
	_channels[1].load(loadData(0x684E));
	_channels[2].load(loadData(0x6DE6));
	_channels[3].load(loadData(0x7232));
}

int ASound9::command45() {
	_callbackFnPtr = &ASound9::loadCommand45;
	return 0;
}

void ASound9::loadCommand45() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x6558));
	_channels[1].load(loadData(0x6AA4));
	_channels[2].load(loadData(0x6F92));
	_channels[3].load(loadData(0x73DC));
}

int ASound9::command47() {
	_callbackFnPtr = &ASound9::loadCommand47;
	return 0;
}

void ASound9::loadCommand47() {
	_callbackFnPtr = nullptr;
	_channels[0].load(loadData(0x7582));
	_channels[1].load(loadData(0x76A6));
	_channels[2].load(loadData(0x77CC));
	_channels[3].load(loadData(0x795A));
}

int ASound9::command48() {
	playSound(0x7FD0);
	playSound(0x7FD8);
	return 0;
}

int ASound9::command49() {
	_channels[0].load(loadData(0x7AD6));
	_channels[1].load(loadData(0x7B32));
	_channels[2].load(loadData(0x7B8C));
	_channels[3].load(loadData(0x7E6E));
	_channels[4].load(loadData(0x7E8A));
	_channels[5].load(loadData(0x7EA8));
	_channels[6].load(loadData(0x7EC6));
	return 0;
}

int ASound9::command50() {
	_callbackFnPtr = &ASound9::loadCommand50;
	return 0;
}

void ASound9::loadCommand50() {
	_callbackFnPtr = nullptr;

	*loadData(0x469B) = 0;
	*loadData(0x57EF) = 0;
	*loadData(0x6267) = 0;

	_channels[0].load(loadData(0x1C30));
	_channels[1].load(loadData(0x25E4));
	_channels[2].load(loadData(0x32A8));
	_channels[3].load(loadData(0x4596));
	_channels[4].load(loadData(0x5718));
	_channels[5].load(loadData(0x6140));
}

int ASound9::command51() {
	_callbackCounter = _callbackPeriod = 96;

	*loadData(0x469B) = 2;
	*loadData(0x57EF) = 2;
	*loadData(0x6267) = 2;

	_channels[0].load(loadData(0x17BC));
	_channels[1].load(loadData(0x1CFC));
	_channels[2].load(loadData(0x2A46));
	_channels[3].load(loadData(0x371E));
	_channels[4].load(loadData(0x474E));
	_channels[5].load(loadData(0x587C));
	return 0;
}


} // namespace RexNebular
} // namespace MADS
