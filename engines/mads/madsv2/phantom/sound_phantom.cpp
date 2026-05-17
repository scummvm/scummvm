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

#include "common/endian.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/textconsole.h"
#include "mads/madsv2/phantom/sound_phantom.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

void PhantomSoundManager::validate() {
	Common::File f;
	static const char *const MD5[] = {
		"8edcb79a8c3514eac0835496326a72ae",
		"4b81a46440f8404d9eda1ce5ae2c5579",
		"11d8d441e47ad1ccd8faafd6572a17d0",
		"4cd5c4d45126e60ca701690489ab8afa",
		"588357d711bbcdabdf7d7e5d96013ce5",
		nullptr,
		nullptr,
		nullptr,
		"3d4843074c1dcbfd7919179c58aec9bc"
	};

	for (int i = 1; i <= 9; ++i) {
		if (i >= 6 && i <= 8)
			continue;
		Common::Path filename(Common::String::format("asound.ph%d", i));
		if (!f.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		Common::String md5str = Common::computeStreamMD5AsString(f, 8192);
		f.close();

		if (md5str != MD5[i - 1])
			error("Invalid sound file - %s", filename.toString().c_str());
	}
}

void PhantomSoundManager::loadDriver(int sectionNumber) {
	switch (sectionNumber) {
	case 1:
		_driver = new ASound1(_mixer, _opl);
		break;
	case 2:
		_driver = new ASound2(_mixer, _opl);
		break;
	case 3:
		_driver = new ASound3(_mixer, _opl);
		break;
	case 4:
		_driver = new ASound4(_mixer, _opl);
		break;
	case 5:
		_driver = new ASound5(_mixer, _opl);
		break;
	case 9:
		_driver = new ASound9(_mixer, _opl);
		break;
	default:
		_driver = nullptr;
		return;
	}
}

/*-----------------------------------------------------------------------*/
/* ASound1  (asound.ph1)                                                  *
 *-----------------------------------------------------------------------*/

const ASound1::CommandPtr ASound1::_commandList[40] = {
	&ASound1::command0,  &ASound1::command1,  &ASound1::command2,  &ASound1::command3,
	&ASound1::command4,  &ASound1::command5,  &ASound1::command6,  &ASound1::command7,
	&ASound1::command8,  nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound1::command16, nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound1::command24, &ASound1::command25, &ASound1::command26, &ASound1::command27,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound1::command32, &ASound1::command33, &ASound1::command34, &ASound1::command35,
	&ASound1::command36, &ASound1::command37, &ASound1::command38, &ASound1::command39
};

ASound1::ASound1(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.ph1", 0x21e0, 0x4d20) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1d4);
	for (int i = 0; i < 120; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound1::command(int commandId, int param) {
	if (commandId > 39 || !_commandList[commandId])
		return 0;
	
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASound1::command0() { return ASound::command0(); }
int ASound1::command1() { return ASound::command1(); }
int ASound1::command2() { return ASound::command2(); }
int ASound1::command3() { return ASound::command3(); }
int ASound1::command4() { return ASound::command4(); }
int ASound1::command5() { return ASound::command5(); }
int ASound1::command6() { return ASound::command6(); }
int ASound1::command7() { return ASound::command7(); }
int ASound1::command8() { return ASound::command8(); }

int ASound1::commandMusic0() {
	ASound::command1();
	_channels[0]->load(loadData(0x1ECA));
	_channels[1]->load(loadData(0x1FBF));
	_channels[2]->load(loadData(0x2037));
	_channels[3]->load(loadData(0x20EE));
	_channels[4]->load(loadData(0x219B));
	_channels[5]->load(loadData(0x21AF));
	return 0;
}

int ASound1::commandMusic1() {
	ASound::command1();
	_channels[0]->load(loadData(0x3418));
	_channels[1]->load(loadData(0x34EB));
	_channels[2]->load(loadData(0x359B));
	_channels[3]->load(loadData(0x3658));
	_channels[4]->load(loadData(0x3667));
	_channels[5]->load(loadData(0x3677));
	return 0;
}

int ASound1::commandMusic2() {
	ASound::command1();
	_channels[0]->load(loadData(0x3688));
	_channels[1]->load(loadData(0x387B));
	_channels[2]->load(loadData(0x3A01));
	_channels[3]->load(loadData(0x3BC6));
	_channels[4]->load(loadData(0x3D31));
	_channels[5]->load(loadData(0x3D41));
	return 0;
}

int ASound1::commandMusic3() {
	ASound::command1();
	_channels[0]->load(loadData(0x3D52));
	_channels[1]->load(loadData(0x3FD3));
	_channels[2]->load(loadData(0x41FF));
	_channels[3]->load(loadData(0x420C));
	_channels[4]->load(loadData(0x4219));
	_channels[5]->load(loadData(0x4229));
	return 0;
}

// ---------------------------------------------------------------------------
// command16 - random background music
//
// If channel 0 is active and already playing one of the five known music
// pieces (identified by their starting offset in field_17), leave it alone.
// Otherwise pick a piece at random: the original uses getRandomNumber() & 7,
// discarding 0 and indexing a four-entry table repeated twice (entries 1-3
// -> pieces 1-3, entries 4-7 -> same pieces again with entry 4 wrapping to
// piece 0).  We reproduce this with a modulo-4 on a non-zero value.
// ---------------------------------------------------------------------------
int ASound1::command16() {
	if (_channels[0]->_activeCount) {
		// Special offset checks
		int f = _channels[0]->_loopStartPtr - &_soundData[0];

		if (f == 0 || f == 0x1ECA || f == 0x21C4 ||
		    f == 0x3418 || f == 0x3688 || f == 0x3D52)
			return 0;
	}

	int idx;
	do {
		idx = getRandomNumber() & 3;
	} while (idx == 0);
	_musicIndex = idx;

	typedef int (ASound1::*MusicPtr)();
	static const MusicPtr musicTable[4] = {
		&ASound1::commandMusic0,
		&ASound1::commandMusic1,
		&ASound1::commandMusic2,
		&ASound1::commandMusic3
	};
	return (this->*musicTable[idx])();
}

// ---------------------------------------------------------------------------
// commands 24-27 - upper channel pool
// ---------------------------------------------------------------------------

int ASound1::command24() {
	playSound(0x173A);
	playSound(0x176D);
	return 0;
}

int ASound1::command25() {
	playSound(0x179B);
	playSound(0x17C7);
	return 0;
}

int ASound1::command26() {
	playSound(0x17F5);
	return 0;
}

int ASound1::command27() {
	playSound(0x1801);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32-39
// ---------------------------------------------------------------------------

// command32 - no guard, no fade, load ch0-5
int ASound1::command32() {
	_channels[0]->load(loadData(0x2522));
	_channels[1]->load(loadData(0x255D));
	_channels[2]->load(loadData(0x2591));
	_channels[3]->load(loadData(0x25BB));
	_channels[4]->load(loadData(0x25E7));
	_channels[5]->load(loadData(0x2613));
	return 0;
}

// command33 - isSoundActive guard, command1, load ch0-5
int ASound1::command33() {
	byte *pData = loadData(0x266C);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x2929));
		_channels[2]->load(loadData(0x2B1D));
		_channels[3]->load(loadData(0x2D37));
		_channels[4]->load(loadData(0x2EC3));
		_channels[5]->load(loadData(0x3033));
	}
	return 0;
}

// command34 - isSoundActive guard, stop(), load ch0-5
int ASound1::command34() {
	byte *pData = loadData(0x1852);
	if (!isSoundActive(pData)) {
		stop();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x1AA9));
		_channels[2]->load(loadData(0x1BC4));
		_channels[3]->load(loadData(0x1CF1));
		_channels[4]->load(loadData(0x1DF2));
		_channels[5]->load(loadData(0x1EBE));
	}
	return 0;
}

// command35 - isSoundActive guard, command2 (lower-bank fade),
// load ch0-5
int ASound1::command35() {
	byte *pData = loadData(0x0C36);
	if (!isSoundActive(pData)) {
		ASound::command2();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x0D7F));
		_channels[2]->load(loadData(0x0E48));
		_channels[3]->load(loadData(0x0F10));
		_channels[4]->load(loadData(0x0FB2));
		_channels[5]->load(loadData(0x1096));
	}
	return 0;
}

// command36 - isSoundActive guard, command2 (lower-bank fade),
// load ch0-5
int ASound1::command36() {
	byte *pData = loadData(0x1190);
	if (!isSoundActive(pData)) {
		ASound::command2();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x12D7));
		_channels[2]->load(loadData(0x13AA));
		_channels[3]->load(loadData(0x1476));
		_channels[4]->load(loadData(0x1528));
		_channels[5]->load(loadData(0x1614));
	}
	return 0;
}

// command37 - isSoundActive guard, command1, four loadAny
// calls starting from channel 0
int ASound1::command37() {
	byte *pData = loadData(0x3220);
	if (!isSoundActive(pData)) {
		ASound::command1();
		findFreeChannel(pData);
		findFreeChannel(loadData(0x326A));
		findFreeChannel(loadData(0x3293));
		findFreeChannel(loadData(0x32AC));
	}
	return 0;
}

// command38 - alias for commandMusic0; also the direct dispatch
// target for command 38.
int ASound1::command38() {
	return commandMusic0();
}

// command39 - isSoundActive guard, command1, load ch0-5
int ASound1::command39() {
	byte *pData = loadData(0x423A);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x43DF));
		_channels[2]->load(loadData(0x44F7));
		_channels[3]->load(loadData(0x45ED));
		_channels[4]->load(loadData(0x46F9));
		_channels[5]->load(loadData(0x48AF));
	}
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound2  (asound.ph2)                                                  *
 *-----------------------------------------------------------------------*/

const ASound2::CommandPtr ASound2::_commandList[73] = {
	// commands 0-8  (asound_commands1)
	&ASound2::command0,  &ASound2::command1,  &ASound2::command2,  &ASound2::command3,
	&ASound2::command4,  &ASound2::command5,  &ASound2::command6,  &ASound2::command7,
	&ASound2::command8,
	// 9-15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// command 16  (asound_commands2)
	&ASound2::command16,
	// 17-23 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24-27  (asound_commands3)
	&ASound2::command24, &ASound2::command25, &ASound2::command26, &ASound2::command27,
	// 28-31 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 32-35  (asound_commands4)
	&ASound2::command32, &ASound2::command33, &ASound2::command34, &ASound2::command35,
	// 36-63 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 64-72  (asound_commands5)
	&ASound2::command64, &ASound2::command65, &ASound2::command66, &ASound2::command67,
	&ASound2::command68, &ASound2::command69, &ASound2::command70, &ASound2::command71,
	&ASound2::command72
};

ASound2::ASound2(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.ph2", 0x2040, 0x2300) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1d4);
	for (int i = 0; i < 120; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound2::command(int commandId, int param) {
	if (commandId > 72 || !_commandList[commandId])
		return 0;

	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASound2::command0() { return ASound::command0(); }
int ASound2::command1() { return ASound::command1(); }
int ASound2::command2() { return ASound::command2(); }
int ASound2::command3() { return ASound::command3(); }
int ASound2::command4() { return ASound::command4(); }
int ASound2::command5() { return ASound::command5(); }
int ASound2::command6() { return ASound::command6(); }
int ASound2::command7() { return ASound::command7(); }
int ASound2::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// command16 - isSoundActive guard, command1, load ch0-5
// ---------------------------------------------------------------------------
int ASound2::command16() {
	byte *pData = loadData(0x0C36);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x0C8E));
		_channels[2]->load(loadData(0x0CF4));
		_channels[3]->load(loadData(0x0D4E));
		_channels[4]->load(loadData(0x0DA3));
		_channels[5]->load(loadData(0x0DB1));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 24-27 (asound_commands3) - upper channel pool
// ---------------------------------------------------------------------------

int ASound2::command24() {
	playSound(0x1A4A);
	playSound(0x1A7D);
	return 0;
}

int ASound2::command25() {
	playSound(0x1AAB);
	playSound(0x1AD7);
	return 0;
}

int ASound2::command26() {
	playSound(0x1B05);
	return 0;
}

int ASound2::command27() {
	playSound(0x1B11);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32-35 (asound_commands4)
// ---------------------------------------------------------------------------

// command32 - command1, six loadAny calls from channel 0
int ASound2::command32() {
	ASound::command1();
	findFreeChannel(loadData(0x1BE4));
	findFreeChannel(loadData(0x1CB7));
	findFreeChannel(loadData(0x1E1E));
	findFreeChannel(loadData(0x1EC8));
	findFreeChannel(loadData(0x1ED8));
	findFreeChannel(loadData(0x1EEF));
	return 0;
}

// command33 - isSoundActive guard, command1, load ch0-7
int ASound2::command33() {
	byte *pData = loadData(0x1B62);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x1B97));
		_channels[2]->load(loadData(0x1BA5));
		_channels[3]->load(loadData(0x1BB3));
		_channels[4]->load(loadData(0x1BC1));
		_channels[5]->load(loadData(0x1BC5));
		_channels[6]->load(loadData(0x1BC9));
		_channels[7]->load(loadData(0x1BD5));
	}
	return 0;
}

// command34 - isSoundActive guard, command1, load ch0-6
int ASound2::command34() {
	byte *pData = loadData(0x0DC0);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x0FAF));
		_channels[2]->load(loadData(0x1206));
		_channels[3]->load(loadData(0x139A));
		_channels[4]->load(loadData(0x1565));
		_channels[5]->load(loadData(0x1833));
		_channels[6]->load(loadData(0x18CD));
	}
	return 0;
}

// command35 - isSoundActive guard, command1, load ch0-6
int ASound2::command35() {
	byte *pData = loadData(0x1F02);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x1F66));
		_channels[2]->load(loadData(0x1F70));
		_channels[3]->load(loadData(0x1F8D));
		_channels[4]->load(loadData(0x1FCE));
		_channels[5]->load(loadData(0x1FF7));
		_channels[6]->load(loadData(0x202E));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64-72 (asound_commands5) - upper channel pool
// ---------------------------------------------------------------------------

int ASound2::command64() {
	playSound(0x1928);
	return 0;
}

int ASound2::command65() {
	playSound(0x193C);
	return 0;
}

int ASound2::command66() {
	playSound(0x1946);
	playSound(0x195C);
	return 0;
}

int ASound2::command67() {
	playSound(0x196D);
	return 0;
}

// no-op
int ASound2::command68() {
	return 0;
}

int ASound2::command69() {
	playSound(0x197F);
	playSound(0x19A5);
	playSound(0x19CB);
	return 0;
}

int ASound2::command70() {
	playSound(0x19E5);
	playSound(0x19F1);
	return 0;
}

int ASound2::command71() {
	playSound(0x19FF);
	return 0;
}

int ASound2::command72() {
	playSound(0x1A0D);
	playSound(0x1A10);
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound3  (asound.ph3)                                                  *
 *-----------------------------------------------------------------------*/

const ASound3::CommandPtr ASound3::_commandList[77] = {
	// commands 0-8  (asound_commands1)
	&ASound3::command0,  &ASound3::command1,  &ASound3::command2,  &ASound3::command3,
	&ASound3::command4,  &ASound3::command5,  &ASound3::command6,  &ASound3::command7,
	&ASound3::command8,
	// 9-15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// command 16  (asound_commands2)
	&ASound3::command16,
	// 17-23 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24-27  (asound_commands3)
	&ASound3::command24, &ASound3::command25, &ASound3::command26, &ASound3::command27,
	// 28-31 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 32-37  (asound_commands4)
	&ASound3::command32, &ASound3::command33, &ASound3::command34, &ASound3::command35,
	&ASound3::command36, &ASound3::command37,
	// 38-63 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,
	// commands 64-75  (asound_commands5)
	&ASound3::command64, &ASound3::command65, &ASound3::command66, &ASound3::command67,
	&ASound3::command68, &ASound3::command69, &ASound3::command70, &ASound3::command71,
	&ASound3::command72, &ASound3::command73, &ASound3::command74, &ASound3::command75,
	// command 76 = nullsub_8
	nullptr
};

ASound3::ASound3(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.ph3", 0x20c0, 0x31a0) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1d4);
	for (int i = 0; i < 120; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound3::command(int commandId, int param) {
	if (commandId > 76 || !_commandList[commandId])
		return 0;
	
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASound3::command0() { return ASound::command0(); }
int ASound3::command1() { return ASound::command1(); }
int ASound3::command2() { return ASound::command2(); }
int ASound3::command3() { return ASound::command3(); }
int ASound3::command4() { return ASound::command4(); }
int ASound3::command5() { return ASound::command5(); }
int ASound3::command6() { return ASound::command6(); }
int ASound3::command7() { return ASound::command7(); }
int ASound3::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// sub_11CC6 - shared helper used by command34.
// isSoundActive guard on 0xC36; if not active: command1, load ch0-7.
// (The tiny ch6 and ch7 blocks - 4 bytes each - are very short sub-blocks
// within the same composite sound.)
// ---------------------------------------------------------------------------
void ASound3::sub11CC6() {
	byte *pData = loadData(0x0C36);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x0C6B));
		_channels[2]->load(loadData(0x0C79));
		_channels[3]->load(loadData(0x0C87));
		_channels[4]->load(loadData(0x0C95));
		_channels[5]->load(loadData(0x0C99));
		_channels[6]->load(loadData(0x0C9D));
		_channels[7]->load(loadData(0x0CA9));
	}
}

// ---------------------------------------------------------------------------
// command16 - isSoundActive guard, command1, load ch0-5
// ---------------------------------------------------------------------------
int ASound3::command16() {
	byte *pData = loadData(0x24F2);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x259E));
		_channels[2]->load(loadData(0x2627));
		_channels[3]->load(loadData(0x26AE));
		_channels[4]->load(loadData(0x2761));
		_channels[5]->load(loadData(0x2810));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 24-27 (asound_commands3) - upper channel pool
// ---------------------------------------------------------------------------

int ASound3::command24() {
	playSound(0x2A7C);
	playSound(0x2AAF);
	return 0;
}

int ASound3::command25() {
	playSound(0x2ADD);
	playSound(0x2B09);
	return 0;
}

int ASound3::command26() {
	playSound(0x2B37);
	return 0;
}

int ASound3::command27() {
	playSound(0x2B43);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32-37 (asound_commands4)
// ---------------------------------------------------------------------------

// command32 - isSoundActive guard, command1, load ch0-7
int ASound3::command32() {
	byte *pData = loadData(0x2B94);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x2C00));
		_channels[2]->load(loadData(0x2C49));
		_channels[3]->load(loadData(0x2C8C));
		_channels[4]->load(loadData(0x2D23));
		_channels[5]->load(loadData(0x2DCE));
		_channels[6]->load(loadData(0x2E25));
		_channels[7]->load(loadData(0x2E84));
	}
	return 0;
}

// command33 - isSoundActive guard, command1, load ch0-6
int ASound3::command33() {
	byte *pData = loadData(0x149E);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x16AB));
		_channels[2]->load(loadData(0x1933));
		_channels[3]->load(loadData(0x1A2F));
		_channels[4]->load(loadData(0x1C25));
		_channels[5]->load(loadData(0x1ECD));
		_channels[6]->load(loadData(0x206F));
	}
	return 0;
}

// command34 - calls sub11CC6 (loads ch0-7 if not active),
// then unconditionally loads ch8 at 0x298E
int ASound3::command34() {
	sub11CC6();
	_channels[8]->load(loadData(0x298E));
	return 0;
}

// command35 - isSoundActive guard, command1, load ch0-5
int ASound3::command35() {
	byte *pData = loadData(0x0CB8);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x0E55));
		_channels[2]->load(loadData(0x0F63));
		_channels[3]->load(loadData(0x1051));
		_channels[4]->load(loadData(0x1159));
		_channels[5]->load(loadData(0x130B));
	}
	return 0;
}

// command36 - isSoundActive guard, command1, load ch0-5
int ASound3::command36() {
	byte *pData = loadData(0x2072);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x2136));
		_channels[2]->load(loadData(0x23EA));
		_channels[3]->load(loadData(0x243D));
		_channels[4]->load(loadData(0x2455));
		_channels[5]->load(loadData(0x24A3));
	}
	return 0;
}

// command37 - single upper-pool voice
int ASound3::command37() {
	playSound(0x298E);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64-75 (asound_commands5)
// ---------------------------------------------------------------------------

int ASound3::command64() {
	_channels[6]->load(loadData(0x28CA));
	_channels[8]->load(loadData(0x28FC));
	return 0;
}

int ASound3::command65() {
	_channels[6]->load(loadData(0x2919));
	_channels[8]->load(loadData(0x292A));
	return 0;
}

int ASound3::command66() {
	_channels[6]->load(loadData(0x2937));
	_channels[7]->load(loadData(0x2956));
	_channels[8]->load(loadData(0x2965));
	return 0;
}

int ASound3::command67() {
	playSound(0x2984);
	return 0;
}

int ASound3::command68() {
	playSound(0x2998);
	playSound(0x29AE);
	playSound(0x29C2);
	return 0;
}

int ASound3::command69() {
	playSound(0x29D8);
	return 0;
}

int ASound3::command70() {
	playSound(0x2B4F);
	return 0;
}

int ASound3::command71() {
	playSound(0x2B5E);
	return 0;
}

int ASound3::command72() {
	_channels[7]->load(loadData(0x29EA));
	_channels[8]->load(loadData(0x2A18));
	return 0;
}

int ASound3::command73() {
	playSound(0x2A44);
	return 0;
}

int ASound3::command74() {
	playSound(0x2A4E);
	return 0;
}

int ASound3::command75() {
	_channels[7]->load(loadData(0x29FB));
	_channels[8]->load(loadData(0x2A29));
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound4  (asound.ph4)                                                  *
 *-----------------------------------------------------------------------*/

const ASound4::CommandPtr ASound4::_commandList[71] = {
	// commands 0-8  (asound_commands1)
	&ASound4::command0,  &ASound4::command1,  &ASound4::command2,  &ASound4::command3,
	&ASound4::command4,  &ASound4::command5,  &ASound4::command6,  &ASound4::command7,
	&ASound4::command8,
	// 9-15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// command 16  (asound_commands2)
	&ASound4::command16,
	// 17-23 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24-27  (asound_commands3)
	&ASound4::command24, &ASound4::command25, &ASound4::command26, &ASound4::command27,
	// 28-63 absent (the 0x20-range table has max=0, unreachable)
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 64-70  (asound_commands4)
	&ASound4::command64, &ASound4::command65, &ASound4::command66, &ASound4::command67,
	&ASound4::command68, &ASound4::command69, &ASound4::command70
};

ASound4::ASound4(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.ph4", 0x1f90, 0x14d0) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1d4);
	for (int i = 0; i < 120; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound4::command(int commandId, int param) {
	if (commandId > 70 || !_commandList[commandId])
		return 0;
	
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASound4::command0() { return ASound::command0(); }
int ASound4::command1() { return ASound::command1(); }
int ASound4::command2() { return ASound::command2(); }
int ASound4::command3() { return ASound::command3(); }
int ASound4::command4() { return ASound::command4(); }
int ASound4::command5() { return ASound::command5(); }
int ASound4::command6() { return ASound::command6(); }
int ASound4::command7() { return ASound::command7(); }
int ASound4::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// command16 - isSoundActive guard, command1, load ch0-6
// ---------------------------------------------------------------------------
int ASound4::command16() {
	byte *pData = loadData(0x0C36);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x0C75));
		_channels[2]->load(loadData(0x0EF1));
		_channels[3]->load(loadData(0x0F19));
		_channels[4]->load(loadData(0x0F41));
		_channels[5]->load(loadData(0x0F67));
		_channels[6]->load(loadData(0x0F90));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 24-27 (asound_commands3) - upper pool
//
// commands 24 and 25 are both wired to the same handler,
// which loads two upper-pool sounds.
// ---------------------------------------------------------------------------

int ASound4::command24() {
	playSound(0x0FFA);
	playSound(0x100C);
	return 0;
}

int ASound4::command25() {
	return command24();
}

int ASound4::command26() {
	playSound(0x119D);
	return 0;
}

int ASound4::command27() {
	playSound(0x11A9);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64-70 (asound_commands4, base 0x40)
// All entries use the upper pool
// ---------------------------------------------------------------------------

int ASound4::command64() {
	playSound(0x1017);
	playSound(0x1031);
	return 0;
}

int ASound4::command65() {
	playSound(0x1042);
	playSound(0x104B);
	return 0;
}

int ASound4::command66() {
	playSound(0x105F);
	playSound(0x1068);
	return 0;
}

int ASound4::command67() {
	playSound(0x1078);
	playSound(0x1081);
	return 0;
}

int ASound4::command68() {
	playSound(0x108F);
	return 0;
}

int ASound4::command69() {
	playSound(0x109B);
	return 0;
}

int ASound4::command70() {
	playSound(0x10A5);
	playSound(0x10A8);
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound5  (asound.ph5)                                                  *
 *-----------------------------------------------------------------------*/

const ASound5::CommandPtr ASound5::_commandList[79] = {
	// commands 0-8  (asound_commands1)
	&ASound5::command0,  &ASound5::command1,  &ASound5::command2,  &ASound5::command3,
	&ASound5::command4,  &ASound5::command5,  &ASound5::command6,  &ASound5::command7,
	&ASound5::command8,
	// 9-15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// command 16  (asound_commands2)
	&ASound5::command16,
	// 17-23 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24-27  (asound_commands3)
	&ASound5::command24, &ASound5::command25, &ASound5::command26, &ASound5::command27,
	// 28-31 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 32-39  (asound_commands4)
	&ASound5::command32, &ASound5::command33, &ASound5::command34, &ASound5::command35,
	&ASound5::command36, &ASound5::command37, &ASound5::command38, &ASound5::command39,
	// 40-63 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 64-78  (asound_commands5)
	&ASound5::command64, &ASound5::command65, &ASound5::command66, &ASound5::command67,
	&ASound5::command68, &ASound5::command69, &ASound5::command70, &ASound5::command71,
	&ASound5::command72, &ASound5::command73, &ASound5::command74, &ASound5::command75,
	&ASound5::command76, &ASound5::command77, &ASound5::command78
};

ASound5::ASound5(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.ph5", 0x2140, 0x5cd0) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1d4);
	for (int i = 0; i < 120; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound5::command(int commandId, int param) {
	if (commandId > 78 || !_commandList[commandId])
		return 0;
	
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASound5::command0() { return ASound::command0(); }
int ASound5::command1() { return ASound::command1(); }
int ASound5::command2() { return ASound::command2(); }
int ASound5::command3() { return ASound::command3(); }
int ASound5::command4() { return ASound::command4(); }
int ASound5::command5() { return ASound::command5(); }
int ASound5::command6() { return ASound::command6(); }
int ASound5::command7() { return ASound::command7(); }
int ASound5::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// command16 - isSoundActive guard, command1, load ch0-5
// ---------------------------------------------------------------------------
int ASound5::command16() {
	byte *pData = loadData(0x4142);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x41BA));
		_channels[2]->load(loadData(0x424C));
		_channels[3]->load(loadData(0x42D1));
		_channels[4]->load(loadData(0x4316));
		_channels[5]->load(loadData(0x43AE));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 24-27 (asound_commands3) - upper pool
// ---------------------------------------------------------------------------

int ASound5::command24() {
	playSound(0x51FA);
	playSound(0x522D);
	return 0;
}

int ASound5::command25() {
	playSound(0x525B);
	playSound(0x5287);
	return 0;
}

int ASound5::command26() {
	playSound(0x52B5);
	return 0;
}

// three upper-pool voices
int ASound5::command27() {
	playSound(0x4040);
	playSound(0x404A);
	playSound(0x4061);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32-39 (asound_commands4)
// ---------------------------------------------------------------------------

// command1, eight loadAny (lower pool, AdlibChannel_loadAny)
int ASound5::command32() {
	ASound::command1();
	findFreeChannel(loadData(0x43BC));
	findFreeChannel(loadData(0x466D));
	findFreeChannel(loadData(0x4773));
	findFreeChannel(loadData(0x4953));
	findFreeChannel(loadData(0x4B4B));
	findFreeChannel(loadData(0x4D93));
	findFreeChannel(loadData(0x4EC7));
	findFreeChannel(loadData(0x5071));
	return 0;
}

// isSoundActive guard, command1, load ch0-6
int ASound5::command33() {
	byte *pData = loadData(0x21C6);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x2427));
		_channels[2]->load(loadData(0x26B3));
		_channels[3]->load(loadData(0x27C3));
		_channels[4]->load(loadData(0x29F1));
		_channels[5]->load(loadData(0x2CB9));
		_channels[6]->load(loadData(0x2E89));
	}
	return 0;
}

// isSoundActive guard, command1, load ch0-5 (non-sequential)
// ch1 and ch3 use data from the 0x4000 region; ch0/ch2/ch4 from 0x2E9E region
int ASound5::command34() {
	byte *pData = loadData(0x2E9E);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x4003));
		_channels[2]->load(loadData(0x348F));
		_channels[3]->load(loadData(0x400A));
		_channels[4]->load(loadData(0x3A1F));
		_channels[5]->load(loadData(0x4011));
	}
	return 0;
}

// isSoundActive guard, command1, load ch0-5 (non-sequential)
// ch1/ch3/ch5 use data from the 0x2196 region interleaved with ch0/ch2/ch4
int ASound5::command35() {
	byte *pData = loadData(0x1D0A);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x2196));
		_channels[2]->load(loadData(0x1E4A));
		_channels[3]->load(loadData(0x21A8));
		_channels[4]->load(loadData(0x1F7A));
		_channels[5]->load(loadData(0x21BA));
	}
	return 0;
}

// isSoundActive guard, command1, load ch0-5 (non-sequential)
// ch4 and ch5 reuse blocks at 0x15EC and 0x18D9 that precede ch0's data
int ASound5::command36() {
	byte *pData = loadData(0x15F5);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x18E2));
		_channels[2]->load(loadData(0x1A28));
		_channels[3]->load(loadData(0x1C59));
		_channels[4]->load(loadData(0x15EC));
		_channels[5]->load(loadData(0x18D9));
	}
	return 0;
}

// isSoundActive guard, command1, load ch0-8
int ASound5::command37() {
	byte *pData = loadData(0x1190);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x131D));
		_channels[2]->load(loadData(0x1497));
		_channels[3]->load(loadData(0x14D3));
		_channels[4]->load(loadData(0x1513));
		_channels[5]->load(loadData(0x153F));
		_channels[6]->load(loadData(0x156B));
		_channels[7]->load(loadData(0x159D));
		_channels[8]->load(loadData(0x15D1));
	}
	return 0;
}

// isSoundActive guard, command2 (lower-bank fade), load ch0-5
int ASound5::command38() {
	byte *pData = loadData(0x0C36);
	if (!isSoundActive(pData)) {
		ASound::command2();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x0D7F));
		_channels[2]->load(loadData(0x0E48));
		_channels[3]->load(loadData(0x0F10));
		_channels[4]->load(loadData(0x0FB2));
		_channels[5]->load(loadData(0x1096));
	}
	return 0;
}

// isSoundActive guard, command3 (lower-bank fade only), load ch0-5
int ASound5::command39() {
	byte *pData = loadData(0x5312);
	if (!isSoundActive(pData)) {
		ASound::command3();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x5569));
		_channels[2]->load(loadData(0x567C));
		_channels[3]->load(loadData(0x579D));
		_channels[4]->load(loadData(0x5890));
		_channels[5]->load(loadData(0x5954));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64-78 (asound_commands5) - upper pool unless noted
// ---------------------------------------------------------------------------

int ASound5::command64() {
	playSound(0x4101);
	return 0;
}

int ASound5::command65() {
	playSound(0x401A);
	return 0;
}

int ASound5::command66() {
	playSound(0x402C);
	return 0;
}

int ASound5::command67() {
	playSound(0x4036);
	return 0;
}

int ASound5::command68() {
	playSound(0x407A);
	return 0;
}

int ASound5::command69() {
	playSound(0x408C);
	return 0;
}

// also shared by command77 and command78
int ASound5::command70() {
	playSound(0x40BA);
	return 0;
}

int ASound5::command71() {
	playSound(0x40C8);
	return 0;
}

int ASound5::command72() {
	playSound(0x40D2);
	return 0;
}

int ASound5::command73() {
	playSound(0x40DC);
	playSound(0x40E7);
	return 0;
}

int ASound5::command74() {
	playSound(0x410B);
	return 0;
}

int ASound5::command75() {
	playSound(0x4129);
	playSound(0x4134);
	return 0;
}

// same block as command70
int ASound5::command76() {
	return command70();
}

// same block as command70
int ASound5::command77() {
	return command70();
}

int ASound5::command78() {
	playSound(0x411F);
	return 0;
}

/*-----------------------------------------------------------------------*/

const ASound9::CommandPtr ASound9::_commandList[72] = {
	&ASound9::command0,  &ASound9::command1,  &ASound9::command2,  &ASound9::command3,
	&ASound9::command4,  &ASound9::command5,  &ASound9::command6,  &ASound9::command7,
	&ASound9::command8,  nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound9::command16, nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound9::command24, &ASound9::command25, &ASound9::command26, &ASound9::command27,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound9::command32, &ASound9::command33, &ASound9::command34, &ASound9::command35,
	&ASound9::command36, &ASound9::command37, &ASound9::command38, &ASound9::command39,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound9::command64, &ASound9::command65, &ASound9::command66, &ASound9::command67,
	&ASound9::command68, &ASound9::command69, &ASound9::command70, &ASound9::command71
};

ASound9::ASound9(Audio::Mixer *mixer, OPL::OPL *opl) :
		ASound(mixer, opl, "asound.ph9", 0x20c0, 0x3690) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1d4);
	for (int i = 0; i < 120; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound9::command(int commandId, int param) {
	if (commandId > 71 || !_commandList[commandId])
		return 0;
	
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASound9::command0() {
	return ASound::command0();
}
int ASound9::command1() {
	return ASound::command1();
}
int ASound9::command2() {
	return ASound::command2();
}
int ASound9::command3() {
	return ASound::command3();
}
int ASound9::command4() {
	return ASound::command4();
}
int ASound9::command5() {
	return ASound::command5();
}
int ASound9::command6() {
	return ASound::command6();
}
int ASound9::command7() {
	return ASound::command7();
}
int ASound9::command8() {
	return ASound::command8();
}

int ASound9::command24() {
	playSound(0x203E);
	playSound(0x2071);
	return 0;
}

int ASound9::command25() {
	playSound(0x209F);
	playSound(0x20CB);
	return 0;
}

int ASound9::command26() {
	playSound(0x20F9);
	return 0;
}

int ASound9::command27() {
	playSound(0x2105);
	return 0;
}

int ASound9::command32() {
	ASound::command1();
	findFreeChannel(loadData(0x2B16));
	findFreeChannel(loadData(0x2B6C));
	findFreeChannel(loadData(0x2BB6));
	findFreeChannel(loadData(0x2E88));
	findFreeChannel(loadData(0x2E98));
	findFreeChannel(loadData(0x2EA3));
	findFreeChannel(loadData(0x2EAE));
	findFreeChannel(loadData(0x2EB7));
	return 0;
}

int ASound9::command33() {
	return 0;
}

int ASound9::command34() {
	ASound::command1();
	_channels[0]->load(loadData(0x31D0));
	_channels[1]->load(loadData(0x3221));
	_channels[2]->load(loadData(0x3282));
	_channels[3]->load(loadData(0x32CB));
	_channels[4]->load(loadData(0x331A));
	_channels[5]->load(loadData(0x3369));
	_channels[6]->load(loadData(0x33B0));
	return 0;
}

int ASound9::command35() {
	ASound::command1();
	_channels[0]->load(loadData(0x295E));
	_channels[1]->load(loadData(0x299E));
	_channels[2]->load(loadData(0x29C3));
	_channels[3]->load(loadData(0x29E8));
	_channels[4]->load(loadData(0x2A46));
	_channels[5]->load(loadData(0x2AA5));
	_channels[6]->load(loadData(0x2AE0));
	return 0;
}

int ASound9::command36() {
	ASound::command1();
	_channels[0]->load(loadData(0x30AA));
	_channels[1]->load(loadData(0x30DD));
	_channels[2]->load(loadData(0x3109));
	_channels[3]->load(loadData(0x313D));
	_channels[4]->load(loadData(0x3175));
	_channels[5]->load(loadData(0x319B));
	return 0;
}

int ASound9::command37() {
	ASound::command1();
	_channels[0]->load(loadData(0x2156));
	_channels[1]->load(loadData(0x21A6));
	_channels[2]->load(loadData(0x228E));
	_channels[3]->load(loadData(0x22F7));
	_channels[4]->load(loadData(0x2351));
	_channels[5]->load(loadData(0x25A8));
	_channels[6]->load(loadData(0x28BF));
	return 0;
}

int ASound9::command38() {
	byte *pData = loadData(0x11BC);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x1477));
		_channels[2]->load(loadData(0x158D));
		_channels[3]->load(loadData(0x1777));
		_channels[4]->load(loadData(0x1977));
		_channels[5]->load(loadData(0x1BC5));
		_channels[6]->load(loadData(0x1CFF));
		_channels[7]->load(loadData(0x1EAF));
	}
	return 0;
}

int ASound9::command39() {
	byte *pData = loadData(0x0C36);
	if (!isSoundActive(pData)) {
		ASound::command0();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x0D7D));
		_channels[2]->load(loadData(0x0E50));
		_channels[3]->load(loadData(0x0F1C));
		_channels[4]->load(loadData(0x0FCE));
		_channels[5]->load(loadData(0x10BA));
	}
	return 0;
}

int ASound9::command64() {
	playSound(0x2EC6);
	return 0;
}

int ASound9::command65() {
	playSound(0x2EDA);
	return 0;
}

int ASound9::command66() {
	_channels[0]->load(loadData(0x2EE4));
	_channels[1]->load(loadData(0x2F0E));
	_channels[2]->load(loadData(0x2F3E));
	_channels[3]->load(loadData(0x2F6E));
	_channels[4]->load(loadData(0x2EE4));
	_channels[5]->load(loadData(0x2F0E));
	_channels[6]->load(loadData(0x2F3E));
	_channels[7]->load(loadData(0x2F6E));
	return 0;
}

int ASound9::command67() {
	_channels[6]->load(loadData(0x2F9E));
	_channels[7]->load(loadData(0x2FBD));
	_channels[8]->load(loadData(0x2FCC));
	return 0;
}

int ASound9::command68() {
	playSound(0x2FEB);
	return 0;
}

int ASound9::command69() {
	playSound(0x2FF5);
	playSound(0x301B);
	playSound(0x3041);
	return 0;
}

int ASound9::command70() {
	playSound(0x305B);
	playSound(0x3064);
	return 0;
}

int ASound9::command71() {
	playSound(0x306D);
	playSound(0x308A);
	return 0;
}

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
