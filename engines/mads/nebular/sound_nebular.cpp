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
#include "common/md5.h"
#include "mads/nebular/sound_nebular.h"

namespace Audio {
class Mixer;
}

namespace MADS {

namespace Nebular {

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
		_driver = new Nebular::ASound1(_mixer, _opl);
		break;
	case 2:
		_driver = new Nebular::ASound2(_mixer, _opl);
		break;
	case 3:
		_driver = new Nebular::ASound3(_mixer, _opl);
		break;
	case 4:
		_driver = new Nebular::ASound4(_mixer, _opl);
		break;
	case 5:
		_driver = new Nebular::ASound5(_mixer, _opl);
		break;
	case 6:
		_driver = new Nebular::ASound6(_mixer, _opl);
		break;
	case 7:
		_driver = new Nebular::ASound7(_mixer, _opl);
		break;
	case 8:
		_driver = new Nebular::ASound8(_mixer, _opl);
		break;
	case 9:
		_driver = new Nebular::ASound9(_mixer, _opl);
		break;
	default:
		_driver = nullptr;
		return;
	}
}

/*-----------------------------------------------------------------------*/

RexASound::RexASound(Audio::Mixer *mixer, OPL::OPL *opl,
		const Common::Path &filename, int dataOffset) :
		ASound(mixer, opl, filename, dataOffset) {
	_chanCommandCount = 15;
}

void RexASound::channelCommand(byte *&pSrc, bool &updateFlag) {
	AdlibChannel *chan = _activeChannelPtr;
	int cmdNum = 255 - *pSrc;

	switch (cmdNum) {
	case 0:
		if (!chan->_field17) {
			if (*++pSrc == 0) {
				chan->_pSrc += 2;
				chan->_ptr3 = chan->_pSrc;
				chan->_field17 = 0;
			} else {
				chan->_field17 = *pSrc;
				chan->_pSrc = chan->_ptr3;
			}
		} else if (--chan->_field17) {
			chan->_pSrc = chan->_ptr3;
		} else {
			chan->_pSrc += 2;
			chan->_ptr3 = chan->_pSrc;
		}
		break;

	case 1:
		if (!chan->_field19) {
			if (*++pSrc == 0) {
				chan->_pSrc += 2;
				chan->_ptr4 = chan->_pSrc;
				chan->_ptr3 = chan->_pSrc;
				chan->_field17 = 0;
				chan->_field19 = 0;
			} else {
				chan->_field19 = *pSrc;
				chan->_pSrc = chan->_ptr4;
				chan->_ptr3 = chan->_ptr4;
			}
		} else if (--chan->_field19) {
			chan->_ptr4 = chan->_pSrc;
			chan->_ptr3 = chan->_pSrc;
		} else {
			chan->_pSrc += 2;
			chan->_ptr4 = chan->_pSrc;
			chan->_ptr3 = chan->_pSrc;
		}
		break;

	case 2:
		// Loop sound data
		chan->_field1 = 0;
		chan->_field2 = chan->_field3 = 0;
		chan->_volume = chan->_field7 = 0;
		chan->_field1D = chan->_volumeOffset = 0;
		chan->_field8 = 0;
		chan->_field9 = 0;
		chan->_fieldB = 0;
		chan->_field17 = 0;
		chan->_field19 = 0;
		chan->_fieldD = 0x40;
		chan->_ptr1 = chan->_soundData;
		chan->_pSrc = chan->_soundData;
		chan->_ptr3 = chan->_soundData;
		chan->_ptr4 = chan->_soundData;

		chan->_pSrc += 2;
		break;

	case 3:
		chan->_sampleIndex = *++pSrc;
		chan->_pSrc += 2;
		loadSample(chan->_sampleIndex);
		break;

	case 4:
		chan->_field7 = *++pSrc;
		chan->_pSrc += 2;
		break;

	case 5:
		chan->_field1 = *++pSrc;
		chan->_pSrc += 2;
		break;

	case 6:
		++pSrc;
		if (chan->_fieldE) {
			chan->_pSrc += 2;
		} else {
			chan->_volume = *pSrc >> 1;
			updateFlag = true;
			chan->_pSrc += 2;
		}
		break;

	case 7:
		++pSrc;
		if (!chan->_fieldE) {
			chan->_fieldA = *pSrc;
			chan->_field2 = *++pSrc;
			chan->_field9 = 1;
		}

		chan->_pSrc += 3;
		break;

	case 8:
		chan->_field1D = (int8) * ++pSrc;
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
		if (chan->_fieldE) {
			chan->_pSrc += 2;
		} else {
			chan->_volumeOffset = *pSrc >> 1;
			updateFlag = true;
			chan->_pSrc += 2;
		}
		break;

	case 11:
		chan->_fieldD = *++pSrc;
		updateFlag = true;
		chan->_pSrc += 2;
		break;

	case 12:
		chan->_fieldC = *++pSrc;
		chan->_field3 = *++pSrc;
		chan->_fieldB = 1;
		chan->_pSrc += 2;
		break;

	case 13:
		++pSrc;
		chan->_pSrc += 2;
		break;

	case 14:
		chan->_field1F = *++pSrc;
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
	: RexASound(mixer, opl, "asound.001", 0x1520) {
	_cmd23Toggle = false;

	// Load sound samples
	_soundFile.seek(_dataOffset + 0x12C);
	for (int i = 0; i < 98; ++i)
		_samples.push_back(AdlibSample(_soundFile));
}

int ASound1::command(int commandId, int param) {
	if (commandId > 41)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound1::command9() {
	playSound(0xC68, 12);
	return 0;
}

int ASound1::command10() {
	byte *pData1 = loadData(0x130E, 48);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x133E, 392));
		_channels[2].load(loadData(0x14C6, 46));
		_channels[3].load(loadData(0x14F4, 48));
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
	playSound(0x1216, 248);
	return 0;
}

int ASound1::command15() {
	byte *pData1 = loadData(0x1524, 152);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[4].load(pData1);
		_channels[5].load(loadData(0x15BC, 94));
		_channels[6].load(loadData(0x161A, 94));
		_channels[7].load(loadData(0x1678, 42));
		_channels[8].load(loadData(0x16A2, 42));
	}

	return 0;
}

int ASound1::command16() {
	playSound(0xC74, 14);
	return 0;
}

int ASound1::command17() {
	playSound(0xE9A, 10);
	return 0;
}

int ASound1::command18() {
	command1();
	playSound(0xCA6, 20);
	return 0;
}

int ASound1::command19() {
	command1();
	playSound(0xCBA, 74);
	return 0;
}

int ASound1::command20() {
	byte *pData = loadData(0xD18, 28);
	if (!isSoundActive(pData))
		playSoundData(pData);
	return 0;
}

int ASound1::command21() {
	playSound(0xD04, 20);
	return 0;
}

int ASound1::command22() {
	byte *pData = loadData(0xD34, 10);
	pData[6] = (getRandomNumber() & 7) + 85;

	if (!isSoundActive(pData))
		playSoundData(pData);

	return 0;
}

int ASound1::command23() {
	_cmd23Toggle = !_cmd23Toggle;
	playSound(_cmd23Toggle ? 0xD3E : 0xD46, 8);
	return 0;
}

int ASound1::command24() {
	playSound(0xD4E, 18);
	playSound(0xD60, 20);
	playSound(0xD74, 14);
	return 0;
}

int ASound1::command25() {
	byte *pData = loadData(0xD82, 16);
	if (!isSoundActive(pData))
		playSoundData(pData);

	return 0;
}

int ASound1::command26() {
	byte *pData = loadData(0xEEC, 10);
	pData[5] = (command2627293032() + 0x7F) & 0xFF;

	if (!isSoundActive(pData))
		_channels[6].load(pData);

	return 0;
}

int ASound1::command27() {
	byte *pData = loadData(0xEE2, 10);
	pData[5] = (command2627293032() + 0x40) & 0xFF;

	if (!isSoundActive(pData))
		_channels[7].load(pData);

	return 0;
}

int ASound1::command28() {
	playSound(0xD92, 28);
	return 0;
}

int ASound1::command29() {
	byte *pData = loadData(0xC82, 36);
	byte v = (command2627293032() + 0x40) & 0xFF;
	pData[7] = pData[13] = pData[21] = pData[27] = v;

	if (!isSoundActive(pData))
		playSoundData(pData, 0);

	return 0;
}

int ASound1::command30() {
	byte *pData = loadData(0xEA6, 16);
	pData[7] = (command2627293032() + 0x40) & 0xFF;

	if (!isSoundActive(pData))
		playSoundData(pData, 0);

	return 0;
}

int ASound1::command31() {
	byte *pData = loadData(0xDAE, 14);
	if (!isSoundActive(pData))
		playSoundData(pData);

	return 0;
}

int ASound1::command32() {
	byte *pData = loadData(0xEB4, 46);
	int v = command2627293032() + 0x40;
	pData[9] = pData[17] = pData[25] = pData[33] = v & 0xFF;
	pData[11] = pData[19] = pData[27] = pData[35] = v >> 8;

	if (!isSoundActive(pData))
		playSoundData(pData, 0);

	return 0;
}

int ASound1::command33() {
	playSound(0xDBC, 10);
	playSound(0xDC6, 10);
	return 0;
}

int ASound1::command34() {
	int v = getRandomNumber() & 0x20;
	if (!v)
		v = 0x60;

	byte *pData = loadData(0xDD0, 22);
	pData[8] = pData[15] = v;
	playSoundData(pData);
	return 0;
}

int ASound1::command35() {
	playSound(0xDE6, 16);
	return 0;
}

int ASound1::command36() {
	playSound(0xE10, 10);
	command34();

	return 0;
}

int ASound1::command37() {
	playSound(0xE1A, 14);
	return 0;
}

int ASound1::command38() {
	playSound(0xE28, 114);
	return 0;
}

int ASound1::command39() {
	byte *pData1 = loadData(0x16CC, 82);
	if (!isSoundActive(pData1)) {
		_channels[5].load(pData1);
		_channels[6].load(loadData(0x171E, 30));
		_channels[7].load(loadData(0x173C, 40));
		_channels[8].load(loadData(0x1764, 64));
	}
	return 0;
}

int ASound1::command40() {
	playSound(0xDF6, 26);
	return 0;
}

int ASound1::command41() {
	playSound(0xC32, 34);
	playSound(0xC54, 20);
	return 0;
}

void ASound1::command111213() {
	byte *pData1 = loadData(0xEF6, 408);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x108E, 266));
		_channels[2].load(loadData(0x1198, 66));
		_channels[2].load(loadData(0x11DA, 60));
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

ASound2::ASound2(Audio::Mixer *mixer, OPL::OPL *opl) : RexASound(mixer, opl, "asound.002", 0x15E0) {
	_command12Param = 0xFD;

	// Load sound samples
	_soundFile.seek(_dataOffset + 0x144);
	for (int i = 0; i < 164; ++i)
		_samples.push_back(AdlibSample(_soundFile));
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
	byte *pData1 = loadData(0x1094, 376);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[2].load(loadData(0x123E, 130));

		command9Randomize();
		_channels[1].load(loadData(0x120C, 50));
	}
	return 0;
}

void ASound2::command9Randomize() {
	// Randomization
	int v;
	while (((v = getRandomNumber()) & 0x3F) > 36)
		;

	byte *pData = loadData(0x120C, 50);
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
	byte *pData1 = loadData(0x12C0, 60);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x12FC, 318));
		_channels[2].load(loadData(0x143A, 110));
	}

	return 0;
}

int ASound2::command11() {
	byte *pData = loadData(0x14A8, 170);
	if (!isSoundActive(pData)) {
		playSoundData(pData);
		playSoundData(loadData(0x1552, 1802));
		playSoundData(loadData(0x1C5C, 716));
		playSoundData(loadData(0x1F28, 106));
	}

	return 0;
}

int ASound2::command12() {
	_command12Param += 26;
	byte v = _command12Param & 0x7f;

	byte *pData = loadData(0x4A5E, 38);
	pData[5] = pData[20] = v;
	playSoundData(pData);

	pData = loadData(0x4A84, 30);
	pData[5] = pData[18] = v;
	playSoundData(pData);

	return 0;
}

int ASound2::command13() {
	playSoundData(loadData(0x4AA2, 20));
	playSoundData(loadData(0x4AB6, 20));

	return 0;
}

int ASound2::command14() {
	playSound(0x4ACA, 40);
	playSound(0x4AF2, 42);

	return 0;
}

int ASound2::command15() {
	byte *pData1 = loadData(0x1F92, 1074);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x23C4, 1050);
		playSound(0x27DE, 58);
		playSound(0x2818, 712);
		playSound(0x2AE0, 256);
	}

	return 0;
}

int ASound2::command16() {
	byte *pData1 = loadData(0x3960, 280);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x3A78, 266);
		playSound(0x3B72, 322);
		playSound(0x3CC4, 488);
		playSound(0x3EAC, 104);
		playSound(0x3F14, 104);
	}

	return 0;
}

int ASound2::command17() {
	byte *pData1 = loadData(0x3F7C, 432);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x412C, 422);
		playSound(0x42D2, 424);
		playSound(0x447A, 418);
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
		byte *pData = loadData(command18_list[idx][0], command18_list[idx][1]);
		_channels[3].load(pData);
	}

	return 0;
}

int ASound2::command19() {
	byte *pData1 = loadData(0x2BE0, 366);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x2D4E, 460);
		playSound(0x2F1A, 266);
		playSound(0x3024, 328);
		playSound(0x316C, 162);
		playSound(0x320E, 366);
	}

	return 0;
}

int ASound2::command20() {
	playSound(0x4A36, 40);

	return 0;
}

int ASound2::command21() {
	playSound(0x49DE, 16);
	playSound(0x49EE, 16);
	playSound(0x49FF, 16);

	return 0;
}

int ASound2::command22() {
	playSound(0x4A0E, 24);
	playSound(0x4A26, 16);

	return 0;
}

int ASound2::command23() {
	playSound(0x49B6, 16);

	return 0;
}

int ASound2::command24() {
	playSound(0x49C6, 24);

	return 0;
}

int ASound2::command25() {
	playSound(0x49AC, 10);

	return 0;
}

int ASound2::command26() {
	playSound(0x498A, 14);
	playSound(0x4998, 20);

	return 0;
}

int ASound2::command27() {
	playSound(0x4912, 80);
	playSound(0x4962, 40);

	return 0;
}

int ASound2::command28() {
	playSound(0x48E8, 28);
	playSound(0x4904, 14);

	return 0;
}

int ASound2::command29() {
	playSound(0x48B2, 22);

	return 0;
}

int ASound2::command30() {
	playSound(0x4870, 22);
	playSound(0x4886, 22);
	playSound(0x489C, 22);

	return 0;
}

int ASound2::command31() {
	playSound(0x482E, 22);
	playSound(0x4844, 22);
	playSound(0x489C, 22);

	return 0;
}

int ASound2::command32() {
	playSound(0x46E8, 10);

	return 0;
}

int ASound2::command33() {
	playSound(0x46D8, 16);

	return 0;
}

int ASound2::command34() {
	playSound(0x46C8, 16);

	return 0;
}

int ASound2::command35() {
	playSound(0x46B2, 22);

	return 0;
}

int ASound2::command36() {
	playSound(0x4624, 16);

	return 0;
}

int ASound2::command37() {
	playSound(0x4674, 20);
	playSound(0x4688, 32);
	playSound(0x46A8, 10);

	return 0;
}

int ASound2::command38() {
	byte *pData1 = loadData(0x359E, 202);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x3668, 220);
		playSound(0x3744, 124);
		playSound(0x37C0, 162);
		playSound(0x3862, 78);
		playSound(0x38B0, 176);
	}

	return 0;
}

int ASound2::command39() {
	byte *pData = loadData(0x466A, 10);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound2::command40() {
	playSound(0x4634, 34);
	playSound(0x4656, 20);

	return 0;
}

int ASound2::command41() {
	playSound(0x48C8, 32);

	return 0;
}

int ASound2::command42() {
	playSound(0x46F2, 156);
	playSound(0x478E, 160);

	return 0;
}

int ASound2::command43() {
	playSound(0x4B1C, 40);
	playSound(0x4B44, 41);

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

ASound3::ASound3(Audio::Mixer *mixer, OPL::OPL *opl) : RexASound(mixer, opl, "asound.003", 0x15B0) {
	_command39Flag = false;

	// Load sound samples
	_soundFile.seek(_dataOffset + 0x122);
	for (int i = 0; i < 192; ++i)
		_samples.push_back(AdlibSample(_soundFile));
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
	byte *pData1 = loadData(0x13EA, 254);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0X14E8, 452));
		_channels[2].load(loadData(0x16AC, 396));
		_channels[3].load(loadData(0x1838, 118));
		_channels[4].load(loadData(0x18AE, 74));
	}

	return 0;
}

int ASound3::command11() {
	byte *pData1 = loadData(0x2B84, 596);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x2DD8, 562));
		_channels[2].load(loadData(0x300A, 1694));
		_channels[3].load(loadData(0x36A8, 1100));
		_channels[4].load(loadData(0x3AF4, 420));
		_channels[5].load(loadData(0x3C98, 1516));
	}

	return 0;
}

int ASound3::command13() {
	byte *pData1 = loadData(0x4470, 64);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x44B0, 64);
		playSound(0x44F0, 64);
		playSound(0x4530, 64);
		playSound(0x4570, 64);
		playSound(0X45b0, 64);
	}

	return 0;
}

int ASound3::command14() {
	byte *pData1 = loadData(0X45F0, 36);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x4614, 36);
		playSound(0x4638, 36);
		playSound(0x465C, 32);
		playSound(0x467C, 76);
		playSound(0x46C8, 74);
	}

	return 0;
}

int ASound3::command15() {
	_channels[3].load(loadData(0x36A8, 1100));
	_channels[4].load(loadData(0x3AF4, 420));
	_channels[5].load(loadData(0x3C98, 1516));

	_channels[3]._field20 = 0xDD;
	_channels[4]._field20 = 0xDD;
	_channels[5]._field20 = 0xDD;

	return 0;
}

int ASound3::command16() {
	byte *pData1 = loadData(0x4712, 398);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x48A0, 354));
		_channels[2].load(loadData(0x4A02, 410));
		_channels[3].load(loadData(0x4B9C, 392));
	}

	return 0;
}

int ASound3::command17() {
	byte *pData1 = loadData(0x18F8, 400);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x1A88, 680));
		_channels[2].load(loadData(0x1D30, 478));
		_channels[3].load(loadData(0x1F0E, 1146));
		_channels[4].load(loadData(0x2388, 1006));
		_channels[5].load(loadData(0x2776, 1038));
	}

	return 0;
}

int ASound3::command18() {
	byte *pData1 = loadData(0x4284, 142);
	if (!isSoundActive(pData1)) {
		command1();
		playSoundData(pData1);
		playSound(0x4312, 172);
		playSound(0x43BE, 88);
		playSound(0x4416, 90);
	}

	return 0;
}

int ASound3::command19() {
	playSound(0x4F6, 8);

	return 0;
}

int ASound3::command20() {
	playSound(0x4F1C, 10);

	return 0;
}

int ASound3::command21() {
	playSound(0x4F2E, 8);

	return 0;
}

int ASound3::command22() {
	playSound(0x4F36, 16);

	return 0;
}

int ASound3::command23() {
	playSound(0x4F50, 10);
	playSound(0x4F46, 10);

	return 0;
}

int ASound3::command24() {
	// WORKAROUND: Original calls isSoundActive without loading data pointer
	byte *pData = loadData(0x4EFC, 12);
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
	playSound(0x4EE6, 22);

	return 0;
}

int ASound3::command26() {
	playSound(0x4F5A, 8);

	return 0;
}

int ASound3::command27() {
	playSound(0x4DA2, 34);
	playSound(0x4DC4, 20);

	return 0;
}

int ASound3::command28() {
	playSound(0x4F72, 10);
	playSound(0x4F72, 10);

	return 0;
}

int ASound3::command29() {
	playSound(0x4F72, 10);
	playSound(0x4F72, 10);

	return 0;
}

int ASound3::command30() {
	playSound(0x4E5A, 22);
	playSound(0x4E70, 22);
	playSound(0x4E86, 22);

	return 0;
}

int ASound3::command31() {
	playSound(0x4F7C, 40);

	return 0;
}

int ASound3::command32() {
	playSound(0x4ED2, 10);

	return 0;
}

int ASound3::command33() {
	playSound(0x4EC2, 16);

	return 0;
}

int ASound3::command34() {
	playSound(0x4EB2, 16);

	return 0;
}

int ASound3::command35() {
	playSound(0x4E9C, 22);

	return 0;
}

int ASound3::command36() {
	playSound(0x4D2C, 16);

	return 0;
}

int ASound3::command37() {
	playSound(0x4E1E, 20);
	playSound(0x4E32, 30);
	playSound(0x4E50, 10);

	return 0;
}

int ASound3::command38() {
	playSound(0x4FAC, 10);

	return 0;
}

int ASound3::command39() {
	_command39Flag = !_command39Flag;
	if (_command39Flag) {
		playSound(0x4FD0, 8);
	} else {
		playSound(0x4FD8, 8);
	}

	return 0;
}

int ASound3::command40() {
	_command39Flag = !_command39Flag;
	if (_command39Flag) {
		playSound(0x4EE0, 8);
	} else {
		playSound(0x4EE8, 8);
	}

	return 0;
}

int ASound3::command41() {
	playSound(0x4F08, 20);

	return 0;
}

int ASound3::command42() {
	playSound(0x4DD8, 28);
	playSound(0x4DF4, 42);

	return 0;
}

int ASound3::command43() {
	playSound(0x4FB6, 12);
	playSound(0x4FC2, 14);

	return 0;
}

int ASound3::command44() {
	playSound(0x4FFE, 14);

	return 0;
}

int ASound3::command45() {
	playSound(0x500C, 14);

	return 0;
}

int ASound3::command46() {
	playSound(0x4D78, 14);
	playSound(0x4D86, 14);
	playSound(0x4D94, 14);

	return 0;
}

int ASound3::command47() {
	playSound(0x4D62, 8);
	playSound(0x4D6A, 14);

	return 0;
}

int ASound3::command49() {
	playSound(0x4D62, 8);
	playSound(0x4D6A, 14);

	return 0;
}

int ASound3::command50() {
	playSound(0x4D3C, 14);
	playSound(0x4D4A, 14);
	playSound(0x4D58, 10);

	return 0;
}

int ASound3::command51() {
	playSound(0x4FF0, 14);

	return 0;
}

int ASound3::command57() {
	byte *pData = loadData(0x4EDC, 10);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound3::command59() {
	playSound(0x4F62, 16);

	return 0;
}

int ASound3::command60() {
	playSound(0x4FA4, 8);

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

ASound4::ASound4(Audio::Mixer *mixer, OPL::OPL *opl) : RexASound(mixer, opl, "asound.004", 0x14F0) {
	// Load sound samples
	_soundFile.seek(_dataOffset + 0x122);
	for (int i = 0; i < 210; ++i)
		_samples.push_back(AdlibSample(_soundFile));
}

int ASound4::command(int commandId, int param) {
	if (commandId > 60)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound4::command10() {
	byte *pData = loadData(0x22AA, 254);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x23A8, 452));
		_channels[2].load(loadData(0x256C, 396));
		_channels[3].load(loadData(0x26F8, 118));
		_channels[4].load(loadData(0x276E, 74));
	}

	return 0;
}

int ASound4::command12() {
	byte *pData = loadData(0x16A8, 550);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x18CE, 442));
		_channels[2].load(loadData(0x1A88, 298));
		_channels[3].load(loadData(0x1BB2, 354));
		_channels[4].load(loadData(0x1D14, 572));
		_channels[4].load(loadData(0x1F50, 560));
	}

	int v = (_commandParam > 0x40) ? _commandParam - 0x40 : 0;
	v += 0xB5;
	for (int channelNum = 0; channelNum < 6; ++channelNum)
		_channels[channelNum]._field20 = v;

	return 0;
}

int ASound4::command19() {
	playSound(0x28EC, 8);

	return 0;
}

int ASound4::command20() {
	playSound(0x28E2, 10);

	return 0;
}

int ASound4::command21() {
	playSound(0x27C0, 8);

	return 0;
}

int ASound4::command24() {
	int v;
	while ((v = (getRandomNumber() & 0x3F)) > 45)
		;

	byte *pData = loadData(0x28D6, 12);
	pData[6] = v + 19;
	playSoundData(pData);

	return 0;
}

int ASound4::command27() {
	playSound(0x27D8, 34);
	playSound(0x27FA, 20);

	return 0;
}

int ASound4::command30() {
	playSound(0x284A, 22);
	playSound(0x2860, 22);
	playSound(0x2876, 22);

	return 0;
}

int ASound4::command32() {
	playSound(0x28C2, 10);

	return 0;
}

int ASound4::command33() {
	playSound(0x28B2, 16);

	return 0;
}

int ASound4::command34() {
	playSound(0x28A2, 16);

	return 0;
}

int ASound4::command35() {
	playSound(0x288C, 22);

	return 0;
}

int ASound4::command36() {
	playSound(0x27C8, 16);

	return 0;
}

int ASound4::command37() {
	playSound(0x280E, 20);
	playSound(0x2822, 30);
	playSound(0x2840, 10);

	return 0;
}

int ASound4::command38() {
	playSound(0x2904, 10);

	return 0;
}

int ASound4::command43() {
	playSound(0x290E, 12);
	playSound(0x291A, 14);

	return 0;
}

int ASound4::command52() {
	byte *pData = loadData(0x23A8, 452);
	if (_channels[1]._ptr1 == pData) {
		pData = loadData(0x146E, 570);
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
	byte *pData = loadData(0x28CC, 10);
	pData[6] = v;
	playSoundData(pData);

	return 0;
}

int ASound4::command58() {
	byte *pData = loadData(0x146E, 570);
	if (_channels[1]._ptr1 == pData) {
		_channels[0].load(loadData(0x22AA, 254));
		_channels[1]._field20 = 0;
		_channels[2]._field20 = 0;
	}

	return 0;
}

int ASound4::command59() {
	playSound(0x28F4, 8);

	return 0;
}

int ASound4::command60() {
	playSound(0x28FC, 8);

	return 0;
}

void ASound4::method1() {
	byte *pData = loadData(0x2180, 58);
	if (!isSoundActive(pData)) {
		command1();

		_channels[0].load(pData);
		_channels[1].load(loadData(0x21BA, 48));
		_channels[2].load(loadData(0x21EA, 50));
		_channels[3].load(loadData(0x221C, 40));
		_channels[4].load(loadData(0x2244, 28));
		_channels[5].load(loadData(0x2260, 74));

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

ASound5::ASound5(Audio::Mixer *mixer, OPL::OPL *opl) : RexASound(mixer, opl, "asound.002", 0x15E0) {
	// Load sound samples
	_soundFile.seek(_dataOffset + 0x144);
	for (int i = 0; i < 164; ++i)
		_samples.push_back(AdlibSample(_soundFile));
}

int ASound5::command(int commandId, int param) {
	if (commandId > 41)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound5::command9() {
	byte *pData = loadData(0x2114, 10);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound5::command10() {
	playSound(0x211E, 10);

	return 0;
}

int ASound5::command11() {
	playSound(0x2016, 10);

	return 0;
}

int ASound5::command13() {
	playSound(0x2154, 10);

	return 0;
}

int ASound5::command14() {
	playSound(0x21DC, 22);

	return 0;
}

int ASound5::command15() {
	byte *pData = loadData(0x21DC, 22);
	if (_channels[0]._ptr1 == pData) {
		pData = loadData(0x1F2, 12);
		_channels[0]._soundData = pData;
		_channels[0]._field17 = 1;
		_channels[0]._field19 = 1;
	}

	return 0;
}

int ASound5::command16() {
	playSound(0x214C, 8);

	return 0;
}

int ASound5::command17() {
	playSound(0x2142, 10);

	return 0;
}

int ASound5::command18() {
	playSound(0x21A2, 22);

	return 0;
}

int ASound5::command19() {
	playSound(0x2190, 18);

	return 0;
}

int ASound5::command20() {
	playSound(0x2170, 16);

	return 0;
}

int ASound5::command21() {
	playSound(0x2180, 16);

	return 0;
}

int ASound5::command22() {
	playSound(0x2168, 8);

	return 0;
}

int ASound5::command23() {
	playSound(0x215E, 10);

	return 0;
}

int ASound5::command26() {
	playSound(0x21B8, 12);

	return 0;
}

int ASound5::command27() {
	playSound(0x21C4, 24);

	return 0;
}

int ASound5::command28() {
	playSound(0x2020, 34);
	playSound(0x4904, 20);

	return 0;
}

int ASound5::command29() {
	byte *pData = loadData(0x17C, 312);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1864, 304));
		_channels[2].load(loadData(0x1994, 222));
		_channels[3].load(loadData(0x1864, 304));
		_channels[4].load(loadData(0x1994, 222));
	}

	return 0;
}

int ASound5::command30() {
	playSound(0x2092, 22);
	playSound(0x20A8, 22);
	playSound(0x20BE, 22);

	return 0;
}

int ASound5::command31() {
	playSound(0x2128, 22);
	playSound(0x2134, 14);

	return 0;
}

int ASound5::command32() {
	playSound(0x210A, 10);

	return 0;
}

int ASound5::command33() {
	playSound(0x20FA, 16);

	return 0;
}

int ASound5::command34() {
	playSound(0x20EA, 16);

	return 0;
}

int ASound5::command35() {
	playSound(0x20D4, 22);

	return 0;
}

int ASound5::command36() {
	playSound(0x2006, 16);

	return 0;
}

int ASound5::command37() {
	playSound(0x2056, 20);
	playSound(0x206A, 30);
	playSound(0x2088, 10);

	return 0;
}

int ASound5::command38() {
	byte *pData1 = loadData(0x14F2, 570);
	if (_channels[3]._ptr1 == pData1) {
		_channels[3].load(loadData(0x1A72, 522));
		_channels[3].load(loadData(0x1C7C, 874));
	}

	return 0;
}

int ASound5::command39() {
	playSound(0x1FEE, 8);

	return 0;
}

int ASound5::command40() {
	playSound(0x1FF6, 16);

	return 0;
}

int ASound5::command41() {
	byte *pData1 = loadData(0x14F2, 570);
	if (!isSoundActive(pData1)) {
		byte *pData2 = loadData(0x1A72, 522);
		if (_channels[3]._ptr1 == pData2) {
			_channels[3].load(pData1);
			_channels[4].load(loadData(0x1FE6, 8));
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

ASound6::ASound6(Audio::Mixer *mixer, OPL::OPL *opl) : RexASound(mixer, opl, "asound.006", 0x1390) {
	// Load sound samples
	_soundFile.seek(_dataOffset + 0x122);
	for (int i = 0; i < 200; ++i)
		_samples.push_back(AdlibSample(_soundFile));
}

int ASound6::command(int commandId, int param) {
	if (commandId > 29)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound6::command9() {
	byte *pData = loadData(0x2194, 10);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound6::command10() {
	playSound(0x2224, 24);

	return 0;
}

int ASound6::command11() {
	playSound(0x2202, 34);

	return 0;
}

int ASound6::command12() {
	playSound(0x2246, 8);

	return 0;
}

int ASound6::command13() {
	playSound(0x2298, 28);

	return 0;
}

int ASound6::command14() {
	playSound(0x22B4, 27);

	return 0;
}

int ASound6::command15() {
	playSound(0x219E, 12);

	return 0;
}

int ASound6::command16() {
	playSound(0x21AA, 22);
	playSound(0x21C0, 12);

	return 0;
}

int ASound6::command17() {
	playSound(0x21CC, 54);

	return 0;
}

int ASound6::command18() {
	playSound(0x2270, 16);

	return 0;
}

int ASound6::command19() {
	playSound(0x2280, 16);

	return 0;
}

int ASound6::command20() {
	playSound(0x223C, 10);

	return 0;
}

int ASound6::command21() {
	playSound(0x224E, 34);

	return 0;
}

int ASound6::command22() {
	playSound(0x2290, 8);

	return 0;
}

int ASound6::command23() {
	playSound(0x215E, 34);
	playSound(0x2180, 20);

	return 0;
}

int ASound6::command24() {
	byte *pData1 = loadData(0x1D54, 540);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x1F70, 52));
		_channels[2].load(loadData(0x1FA4, 430));
	}

	return 0;
}

int ASound6::command25() {
	playSound(0x2152, 12);

	return 0;
}

int ASound6::command29() {
	byte *pData1 = loadData(0x149A, 312);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x15D2, 304));
		_channels[2].load(loadData(0x1702, 222));
		_channels[3].load(loadData(0x17E0, 522));
		_channels[4].load(loadData(0x19EA, 874));
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

ASound7::ASound7(Audio::Mixer *mixer, OPL::OPL *opl) : RexASound(mixer, opl, "asound.007", 0x1460) {
	// Load sound samples
	_soundFile.seek(_dataOffset + 0x122);
	for (int i = 0; i < 214; ++i)
		_samples.push_back(AdlibSample(_soundFile));
}

int ASound7::command(int commandId, int param) {
	if (commandId > 37)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound7::command9() {
	byte *pData1 = loadData(0x2992, 122);
	if (!isSoundActive(pData1)) {
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x2A0C, 76));
		_channels[2].load(loadData(0x2A58, 122));
		_channels[3].load(loadData(0x2AD2, 38));
	}

	return 0;
}

int ASound7::command15() {
	byte *pData = loadData(0x2B3E, 10);
	pData[6] = (getRandomNumber() & 7) + 85;

	return 0;
}

int ASound7::command16() {
	playSound(0x2CE2, 8);

	return 0;
}

int ASound7::command18() {
	playSound(0x2C94, 22);

	return 0;
}

int ASound7::command19() {
	byte *pData1 = loadData(0x2C94, 22);
	byte *pData2 = loadData(0x2CAA, 16);
	if (_channels[8]._ptr1 == pData1 || _channels[8]._ptr1 == pData2) {
		_channels[8]._soundData = loadData(0x2CBA, 12);
		_channels[8]._field17 = 1;
		_channels[8]._field19 = 1;
	}

	return 0;
}

int ASound7::command20() {
	playSound(0x2CD0, 18);

	return 0;
}

int ASound7::command21() {
	playSound(0x2CC6, 10);

	return 0;
}

int ASound7::command22() {
	playSound(0x2C08, 140);

	return 0;
}

int ASound7::command23() {
	playSound(0x2B08, 34);
	playSound(0x2B2A, 20);

	return 0;
}

int ASound7::command24() {
	byte *pData1 = loadData(0x14C6, 144);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x1556, 146));
		_channels[2].load(loadData(0x15E8, 138));
		_channels[3].load(loadData(0x1672, 122));
		_channels[4].load(loadData(0x16EC, 74));
	}

	return 0;
}

int ASound7::command25() {
	byte *pData1 = loadData(0x1DBE, 182);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x1E74, 182));
		_channels[2].load(loadData(0x1F2A, 186));
		_channels[3].load(loadData(0x1FE4, 244));
	}

	return 0;
}

int ASound7::command26() {
	byte *pData1 = loadData(0x20D8, 312);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x2210, 304));
		_channels[2].load(loadData(0x2340, 222));
		_channels[3].load(loadData(0x241E, 522));
		_channels[4].load(loadData(0x2628, 874));
	}

	return 0;
}

int ASound7::command27() {
	byte *pData1 = loadData(0x1736, 158);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x17D4, 288));
		_channels[2].load(loadData(0x18F4, 290));
		_channels[3].load(loadData(0x1A16, 396));
		_channels[4].load(loadData(0x1BA2, 380));
		_channels[5].load(loadData(0x1D1E, 160));
	}

	return 0;
}

int ASound7::command28() {
	playSound(0x2CAA, 16);

	return 0;
}

int ASound7::command30() {
	playSound(0x2B86, 22);
	playSound(0x2B9C, 22);
	playSound(0x2BB2, 22);

	return 0;
}

int ASound7::command32() {
	playSound(0x2BFE, 10);

	return 0;
}

int ASound7::command33() {
	playSound(0x2BEE, 16);

	return 0;
}

int ASound7::command34() {
	playSound(0x2BDE, 16);

	return 0;
}

int ASound7::command35() {
	playSound(0x2BC8, 22);

	return 0;
}

int ASound7::command36() {
	playSound(0x2AF8, 16);

	return 0;
}

int ASound7::command37() {
	playSound(0x2B48, 20);
	playSound(0x2B5C, 32);
	playSound(0x2B7C, 10);

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

ASound8::ASound8(Audio::Mixer *mixer, OPL::OPL *opl) : RexASound(mixer, opl, "asound.008", 0x1490) {
	// Load sound samples
	_soundFile.seek(_dataOffset + 0x122);
	for (int i = 0; i < 174; ++i)
		_samples.push_back(AdlibSample(_soundFile));
}

int ASound8::command(int commandId, int param) {
	if (commandId > 37)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound8::command9() {
	byte *pData = loadData(0x15BE, 10);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound8::command10() {
	byte *pData = loadData(0x2B3E, 10);
	pData[6] = (getRandomNumber() & 7) + 85;
	playSoundData(pData);

	return 0;
}

int ASound8::command11() {
	playSound(0x17CA, 12);

	return 0;
}

int ASound8::command12() {
	playSound(0x17D6, 12);

	return 0;
}

int ASound8::command13() {
	playSound(0x1694, 10);

	return 0;
}

int ASound8::command14() {
	playSound(0x169E, 24);

	return 0;
}

int ASound8::command15() {
	byte *pData = loadData(0x169E, 24);
	if (_channels[8]._ptr1 == pData) {
		_channels[8]._soundData = loadData(0x16B6, 12);
		_channels[8]._field17 = 1;
		_channels[8]._field19 = 1;
	}

	return 0;
}

int ASound8::command16() {
	playSound(0x1686, 14);

	return 0;
}

int ASound8::command17() {
	playSound(0x17EC, 12);

	return 0;
}

int ASound8::command18() {
	playSound(0x17F8, 12);

	return 0;
}

int ASound8::command19() {
	playSound(0x16D8, 8);

	return 0;
}

int ASound8::command20() {
	playSound(0x16E0, 8);

	return 0;
}

int ASound8::command21() {
	playSound(0x17E2, 10);

	return 0;
}

int ASound8::command22() {
	playSound(0x178C, 14);
	playSound(0x179A, 14);
	playSound(0x17A8, 14);

	return 0;
}

int ASound8::command23() {
	playSound(0x2B08, 34);
	playSound(0x2B2A, 20);

	return 0;
}

int ASound8::command24() {
	playSound(0x17B6, 8);

	return 0;
}

int ASound8::command25() {
	playSound(0x17BE, 12);

	return 0;
}

int ASound8::command26() {
	playSound(0x16C2, 22);

	return 0;
}

int ASound8::command27() {
	playSound(0x1588, 34);
	playSound(0x15AA, 20);

	return 0;
}

int ASound8::command28() {
	byte *pData1 = loadData(0x114E, 376);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[2].load(loadData(0x12F8, 130));

		byte *pData = loadData(0x12C6, 50);
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
	byte *pData1 = loadData(0x137A, 60);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x13B6, 318));
		_channels[2].load(loadData(0x14F4, 118));
	}

	return 0;
}

int ASound8::command30() {
	playSound(0x1644, 22);
	playSound(0x165A, 22);
	playSound(0x1670, 22);

	return 0;
}

int ASound8::command31() {
	playSound(0x156A, 14);

	return 0;
}

int ASound8::command32() {
	playSound(0x163A, 10);

	return 0;
}

int ASound8::command33() {
	playSound(0x162A, 16);

	return 0;
}

int ASound8::command34() {
	playSound(0x161A, 16);

	return 0;
}

int ASound8::command35() {
	playSound(0x1604, 22);

	return 0;
}

int ASound8::command36() {
	playSound(0x1578, 16);

	return 0;
}

int ASound8::command37() {
	playSound(0x15C8, 20);
	playSound(0x15DC, 30);
	playSound(0X15FA, 10);

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

ASound9::ASound9(Audio::Mixer *mixer, OPL::OPL *opl) : RexASound(mixer, opl, "asound.009", 0x16F0) {
	_v1 = _v2 = 0;
	_soundPtr = nullptr;

	// Load sound samples
	_soundFile.seek(_dataOffset + 0x50);
	for (int i = 0; i < 94; ++i)
		_samples.push_back(AdlibSample(_soundFile));
}

int ASound9::command(int commandId, int param) {
	if (commandId > 51)
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound9::command9() {
	_v1 = 1848;
	_v2 = 84;
	_channels[0].load(loadData(0xAA4, 470));
	_channels[1].load(loadData(0xE4C, 450));
	_channels[2].load(loadData(0x1466, 702));
	_channels[3].load(loadData(0x137E, 232));
	_channels[4].load(loadData(0x1014, 65));
	_channels[5].load(loadData(0x11C4, 44));
	_channels[6].load(loadData(0XC7A, 466));
	return 0;
}

int ASound9::command10() {
	_channels[0].load(loadData(0x1724, 24));
	_channels[1].load(loadData(0x173C, 24));
	_channels[2].load(loadData(0x1754, 20));
	_channels[3].load(loadData(0x1768, 20));
	_channels[4].load(loadData(0x177C, 20));
	_channels[5].load(loadData(0x1790, 20));
	return 0;
}

int ASound9::command11() {
	playSound(0x8232, 168);
	playSound(0x82DA, 170);
	return 0;
}

int ASound9::command12() {
	playSound(0x80DA, 12);
	playSound(0x80E6, 12);
	return 0;
}

int ASound9::command13() {
	playSound(0x80F2, 38);
	playSound(0x8118, 42);
	return 0;
}

int ASound9::command14() {
	playSound(0x81F6, 22);
	return 0;
}

int ASound9::command15() {
	playSound(0x818A, 32);
	playSound(0x81AA, 32);
	return 0;
}

int ASound9::command16() {
	playSound(0x8022, 36);
	playSound(0x8046, 42);
	return 0;
}

int ASound9::command17() {
	command29();
	playSound(0x858C, 11);
	return 0;
}

int ASound9::command18() {
	playSound(0x80C2, 24);
	return 0;
}

int ASound9::command19() {
	playSound(0x80A0, 34);
	return 0;
}

int ASound9::command20() {
	int v = (getRandomNumber() & 0x10) | 0x4D;
	byte *pData = loadData(0x8142, 8);
	pData[4] = v & 0x7F;
	playSoundData(pData);
	return 0;
}

int ASound9::command21() {
	playSound(0x815A, 16);
	return 0;
}

int ASound9::command22() {
	playSound(0x816A, 16);
	return 0;
}

int ASound9::command23() {
	playSound(0x814A, 16);
	return 0;
}

int ASound9::command24() {
	playSound(0x7FE2, 34);
	return 0;
}

int ASound9::command25() {
	playSound(0x8004, 30);
	return 0;
}

int ASound9::command26() {
	_channels[6].load(loadData(0x8384, 156));
	_channels[7].load(loadData(0x8420, 160));
	return 0;
}

int ASound9::command27() {
	playSound(0x84C0, 140);
	return 0;
}

int ASound9::command28() {
	playSound(0x81CA, 10);
	return 0;
}

int ASound9::command29() {
	playSound(0x81D4, 10);
	return 0;
}

int ASound9::command30() {
	playSound(0x817A, 16);
	return 0;
}

int ASound9::command31() {
	playSound(0x820C, 14);
	playSound(0x821A, 24);
	return 0;
}

int ASound9::command32() {
	playSound(0x8070, 8);
	return 0;
}

int ASound9::command33() {
	playSound(0x8078, 16);
	playSound(0x8088, 16);
	return 0;
}

int ASound9::command34() {
	// Skipped stuff in original
	_channels[0].load(loadData(0x17A4, 24));
	_channels[1].load(loadData(0x1CDE, 62));
	_channels[2].load(loadData(0x2672, 980));
	_channels[3].load(loadData(0x3336, 1000));
	_channels[4].load(loadData(0x469E, 176));
	_channels[5].load(loadData(0x57F2, 138));

	return 0;
}

int ASound9::command35() {
	playSound(0x854C, 64);
	return 0;
}

int ASound9::command36() {
	playSound(0x81DE, 10);
	playSound(0x81E8, 14);
	return 0;
}

int ASound9::command37() {
	byte *pData = loadData(0x8098, 8);
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
	playSound(0x100E, 6);
	return 0;
}

int ASound9::command39() {
	_soundPtr = loadData(0x1055, 128);
	return 0;
}

int ASound9::command40() {
	_soundPtr = loadData(0x118C, 50);
	return 0;
}

int ASound9::command41() {
	_soundPtr = loadData(0x11BE, 6);
	return 0;
}

int ASound9::command42() {
	_soundPtr = loadData(0x11F0, 50);
	return 0;
}

int ASound9::command43() {
	_v1 = _v2 = 80;
	_channels[0].load(loadData(0x626A, 90));
	_channels[1].load(loadData(0x67F2, 92));
	_channels[2].load(loadData(0x6CFE, 232));
	_channels[3].load(loadData(0x7146, 236));

	return 0;
}

int ASound9::command44_46() {
	_soundPtr = loadData(0x10D5, 38);
	return 0;
}

int ASound9::command45() {
	_soundPtr = loadData(0x10FB, 38);
	return 0;
}

int ASound9::command47() {
	_soundPtr = loadData(0x1121, 107);
	return 0;
}

int ASound9::command48() {
	playSound(0x7FD0, 8);
	playSound(0x7FD8, 10);
	return 0;
}

int ASound9::command49() {
	_channels[0].load(loadData(0x7AD6, 92));
	_channels[1].load(loadData(0x7B32, 90));
	_channels[2].load(loadData(0x7B8C, 738));
	_channels[3].load(loadData(0x7E6E, 28));
	_channels[4].load(loadData(0x7E8A, 30));
	_channels[5].load(loadData(0x7EA8, 30));
	_channels[6].load(loadData(0x7EC6, 195));
	return 0;
}

int ASound9::command50() {
	_soundPtr = loadData(0x1222, 348);
	return 0;
}

int ASound9::command51() {
	// Skipped stuff in original
	_channels[0].load(loadData(0x17BC, 1282));
	_channels[1].load(loadData(0x1CFC, 2422));
	_channels[2].load(loadData(0x2A46, 2288));
	_channels[3].load(loadData(0x371E, 3964));
	_channels[4].load(loadData(0x474E, 1863));
	_channels[5].load(loadData(0x587C, 2538));
	return 0;
}


} // namespace Nebular

} // namespace MADS
