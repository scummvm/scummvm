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

#include "common/util.h"
#include "mads/nebular/sound/psound_nebular.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

// -------------------------------------------------------------------------
// Retail section 1
// -------------------------------------------------------------------------

const PSound1::CommandPtr PSound1::_commandList[42] = {
	&PSound1::command0, &PSound1::command1, &PSound1::command2, &PSound1::command3,
	&PSound1::command4, &PSound1::command5, &PSound1::command6, &PSound1::command7,
	&PSound1::command8, &PSound1::command9, &PSound1::command10, &PSound1::command11,
	&PSound1::command12, &PSound1::command13, &PSound1::command14, &PSound1::command15,
	&PSound1::command16, &PSound1::command17, &PSound1::command18, &PSound1::command19,
	&PSound1::command20, &PSound1::command21, &PSound1::command22, &PSound1::command23,
	&PSound1::command24, &PSound1::command25, &PSound1::command26, &PSound1::command27,
	&PSound1::command28, &PSound1::command29, &PSound1::command30, &PSound1::command31,
	&PSound1::command32, &PSound1::command33, &PSound1::command34, &PSound1::command35,
	&PSound1::command36, &PSound1::command37, &PSound1::command38, &PSound1::command39,
	&PSound1::command40, &PSound1::command41
};

static const PSoundDriverData kPSound1Data = {
	"PSOUND.001",
	0x23c0, 0x13c6, 0x17a0,
	0x0934, 0x0134, 32, PSound::kMusicChannelCount,
	{ 0x0060, 0x00e0, 0x00fc, 0x0106, 0x0110 }
};

PSound1::PSound1(Audio::Mixer *mixer) :
		PSound(mixer, kPSound1Data),
		_command23Toggle(false) {
}

void PSound1::loadCommand11Music() {
	if (isSoundActive(0x0b68))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x0b68);
	loadChannel(1, 0x0c7a);
	loadChannel(2, 0x0d8a);
	loadChannel(3, 0x0dd8);
	loadChannel(4, 0x0d86);

	// These values are set by the command handler/shared loader, not encoded
	// in the streams themselves.
	_channels[2].panning = 0;
	_channels[4].panning = 0x7f;
}

int PSound1::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSound1::command0() {
	_command23Toggle = false;
	return PSound::command0();
}

int PSound1::command9() {
	if (!isSoundActive(0x11a4))
		playSound(0x11a4);
	return 0;
}

int PSound1::command10() {
	if (!isSoundActive(0x093e)) {
		requestStop(0, kChannelCount);
		loadChannel(0, 0x093e);
		loadChannel(1, 0x097e);
		loadChannel(2, 0x0b04);
		loadChannel(3, 0x0b36);
	}
	return 0;
}

int PSound1::command11() {
	loadCommand11Music();
	_channels[0].volumeOffset = (int8)0xc1;
	_channels[1].volumeOffset = (int8)0xc1;
	return 0;
}

int PSound1::command12() {
	loadCommand11Music();
	_channels[0].volumeOffset = 0;
	_channels[1].volumeOffset = (int8)0xc1;
	return 0;
}

int PSound1::command13() {
	loadCommand11Music();
	_channels[0].volumeOffset = 0;
	_channels[1].volumeOffset = 0;
	return 0;
}

int PSound1::command14() {
	playSoundAny(0x0e18);
	return 0;
}

int PSound1::command15() {
	if (!isSoundActive(0x0e9c)) {
		requestStop(0, kChannelCount);
		loadChannel(0, 0x0e9c);
		loadChannel(1, 0x0f66);
		loadChannel(2, 0x0fca);
		loadChannel(3, 0x102e);
		loadChannel(4, 0x105c);
	}
	return 0;
}

int PSound1::command16() {
	playSound(0x11ac);
	return 0;
}

int PSound1::command17() {
	playSound(0x133e);
	return 0;
}

int PSound1::command18() {
	playSound(0x11e2);
	return 0;
}

int PSound1::command19() {
	requestStop(0, kChannelCount);
	playSound(0x11f4);
	return 0;
}

int PSound1::command20() {
	playSound(0x1250);
	return 0;
}

int PSound1::command21() {
	playSound(0x123e);
	return 0;
}

int PSound1::command22() {
	writeDataByte(0x126e, (nextRandom() & 7) + 0x55);
	playSound(0x1268);
	return 0;
}

int PSound1::command23() {
	_command23Toggle = !_command23Toggle;
	playSound(_command23Toggle ? 0x1270 : 0x1278);
	return 0;
}

int PSound1::command24() {
	playSound(0x1280);
	playSound(0x1290);
	playSound(0x12a2);
	return 0;
}

int PSound1::command25() {
	playSound(0x12ac);
	return 0;
}

int PSound1::command26() {
	const byte scaledParam = scaledCommandParameter(_commandParam);
	writeDataByte(0x1399, (scaledParam >> 1) + 0x28);
	if (!isSoundActive(0x1394))
		loadChannel(8, 0x1394);
	return 0;
}

int PSound1::command27() {
	const byte scaledParam = scaledCommandParameter(_commandParam);
	writeDataByte(0x138d, (scaledParam >> 1) + 0x32);
	if (!isSoundActive(0x1388))
		loadChannel(8, 0x1388);
	return 0;
}

int PSound1::command28() {
	playSound(0x12bc);
	return 0;
}

int PSound1::command29() {
	const byte scaledParam = scaledCommandParameter(_commandParam);
	const byte value = (scaledParam >> 1) + 0x2d;
	writeDataByte(0x11c5, value);
	writeDataByte(0x11cb, value);
	writeDataByte(0x11d3, value);
	writeDataByte(0x11d9, value);
	if (!isSoundActive(0x11be))
		playSoundAny(0x11be);
	return 0;
}

int PSound1::command30() {
	const byte scaledParam = scaledCommandParameter(_commandParam);
	writeDataByte(0x134f, (scaledParam >> 1) + 0x23);
	if (!isSoundActive(0x1346))
		playSoundAny(0x1346);
	return 0;
}

int PSound1::command31() {
	playSound(0x131a);
	loadChannel(5, 0x12d6);
	return 0;
}

int PSound1::command32() {
	const byte scaledParam = scaledCommandParameter(_commandParam);
	writeDataByte(0x1367, (scaledParam >> 1) + 0x41);
	if (!isSoundActive(0x1360))
		playSoundAny(0x1360);
	return 0;
}

int PSound1::command33() {
	playSound(0x12e2);
	playSound(0x12ea);
	return 0;
}

int PSound1::command34() {
	const byte value = (nextRandom() & 0x0c) + 0x23;
	writeDataByte(0x13a9, value);
	writeDataByte(0x13ae, value + 0x24);
	playSound(0x13a0);
	return 0;
}

int PSound1::command35() {
	playSound(0x12f2);
	return 0;
}

int PSound1::command36() {
	playSound(0x1310);
	return 0;
}

int PSound1::command37() {
	playSound(0x1328);
	return 0;
}

int PSound1::command38() {
	playSound(0x1330);
	return 0;
}

int PSound1::command39() {
	if (!isSoundActive(0x108a)) {
		loadChannel(0, 0x108a);
		loadChannel(1, 0x1108);
		loadChannel(2, 0x1124);
		loadChannel(3, 0x1152);
		loadChannel(4, 0x10da);
	}
	return 0;
}

int PSound1::command40() {
	playSound(0x1300);
	return 0;
}

int PSound1::command41() {
	playSoundAny(0x13b4);
	return 0;
}


// -------------------------------------------------------------------------
// Retail section 2
// -------------------------------------------------------------------------

const PSound2::CommandPtr PSound2::_commandList[44] = {
	&PSound2::command0, &PSound2::command1, &PSound2::command2, &PSound2::command3,
	&PSound2::command4, &PSound2::command5, &PSound2::command6, &PSound2::command7,
	&PSound2::command8, &PSound2::command9, &PSound2::command10, &PSound2::command11,
	&PSound2::command12, &PSound2::command13, &PSound2::command14, &PSound2::command15,
	&PSound2::command16, &PSound2::command17, &PSound2::command18, &PSound2::command19,
	&PSound2::command20, &PSound2::command21, &PSound2::command22, &PSound2::command23,
	&PSound2::command24, &PSound2::command25, &PSound2::command26, &PSound2::command27,
	&PSound2::command28, &PSound2::command29, &PSound2::command30, &PSound2::command31,
	&PSound2::command32, &PSound2::command33, &PSound2::command34, &PSound2::command35,
	&PSound2::command36, &PSound2::command37, &PSound2::command38, &PSound2::command39,
	&PSound2::command40, &PSound2::command41, &PSound2::command42, &PSound2::command43
};

static const PSoundDriverData kPSound2Data = {
	"PSOUND.002",
	0x2470, 0x3d30, 0x4100,
	0x0fc4, 0x3530, 32, PSound::kMusicChannelCount,
	{ 0x0082, 0x0102, 0x011e, 0x0128, 0x0132 }
};

PSound2::PSound2(Audio::Mixer *mixer) :
		PSound(mixer, kPSound2Data),
		_command12Phase(0x50) {
}

void PSound2::mutateCommand9Sequence() {
	// Generate two interleaved ten-byte ramps.
	uint16 value;
	do {
		value = nextRandom() & 0x3f;
	} while (value > 0x24);

	byte ramp = (byte)(value + 0x14);
	for (uint i = 0; i < 10; ++i)
		writeDataByte(0x0207 + i * 2, ramp - i);

	writeDataByte(0x01ff, nextRandom() & 0xff);

	ramp = (byte)(10 - ((value + 1) / 6));
	for (uint i = 0; i < 10; ++i)
		writeDataByte(0x0208 + i * 2, ramp + i);
}

void PSound2::loadCommand9Music() {
	if (isSoundActive(0x0156))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x0156);
	loadChannel(2, 0x0222);
	mutateCommand9Sequence();
	loadChannel(1, 0x01fc);
}

void PSound2::loadCommand10Music() {
	if (isSoundActive(0x02ca))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x02ca);
	loadChannel(1, 0x031e);
	loadChannel(2, 0x0584);
}

void PSound2::loadCommand11Music() {
	if (isSoundActive(0x169c))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x169c);
	loadChannel(1, 0x1748);
	loadChannel(2, 0x1ece);
	loadChannel(3, 0x20c0);
}

void PSound2::loadCommand15Music() {
	if (isSoundActive(0x211e))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x211e);
	loadChannel(1, 0x2550);
	loadChannel(2, 0x2968);
	loadChannel(3, 0x2af2);
	loadChannel(4, 0x2d58);
}

void PSound2::loadCommand16Music() {
	if (isSoundActive(0x11ee))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x11ee);
	loadChannel(1, 0x1298);
	loadChannel(2, 0x1354);
	loadChannel(3, 0x1438);
	loadChannel(4, 0x159e);
	loadChannel(5, 0x161c);
}

void PSound2::loadCommand17Music() {
	if (isSoundActive(0x2e76))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x2e76);
	loadChannel(1, 0x302e);
	loadChannel(2, 0x31da);
	loadChannel(3, 0x3388);
}

void PSound2::loadCommand19Music() {
	if (isSoundActive(0x063a))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x063a);
	loadChannel(1, 0x07ae);
	loadChannel(2, 0x0976);
	loadChannel(3, 0x0a8c);
	loadChannel(4, 0x0bda);
	loadChannel(5, 0x0c7e);
}

void PSound2::loadCommand38Music() {
	if (isSoundActive(0x06e0))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x06e0);
	loadChannel(1, 0x089a);
	loadChannel(2, 0x0a0c);
	loadChannel(3, 0x0b36);
	loadChannel(4, 0x0c2e);
	loadChannel(5, 0x0d3e);
}

int PSound2::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSound2::command0() {
	_command12Phase = 0x50;
	return PSound::command0();
}

int PSound2::command9() {
	loadCommand9Music();
	return 0;
}

int PSound2::command10() {
	loadCommand10Music();
	return 0;
}

int PSound2::command11() {
	loadCommand11Music();
	return 0;
}

int PSound2::command12() {
	_command12Phase = (_command12Phase + 8) & 0x7f;
	writeDataByte(0x0fcd, _command12Phase);
	playSound(0x0fcc);
	writeDataByte(0x0fe3, _command12Phase);
	playSound(0x0fe2);
	return 0;
}

int PSound2::command13() {
	playSound(0x0ff0);
	playSound(0x1004);
	return 0;
}

int PSound2::command14() {
	playSound(0x1026);
	playSound(0x1050);
	return 0;
}

int PSound2::command15() {
	loadCommand15Music();
	return 0;
}

int PSound2::command16() {
	loadCommand16Music();
	return 0;
}

int PSound2::command17() {
	loadCommand17Music();
	return 0;
}

int PSound2::command18() {
	if (!_channels[5].activeCount) {
		const uint16 tableByteOffset = nextRandom() & 0x1e;
		loadChannel(5, readDataUint16(0x005e + tableByteOffset));
	}
	return 0;
}

int PSound2::command19() {
	loadCommand19Music();
	return 0;
}

int PSound2::command20() {
	playSound(0x1086);
	return 0;
}

int PSound2::command21() {
	playSound(0x10a0);
	return 0;
}

int PSound2::command22() {
	playSound(0x107a);
	return 0;
}

int PSound2::command23() {
	playSound(0x10aa);
	return 0;
}

int PSound2::command24() {
	playSound(0x10b4);
	return 0;
}

int PSound2::command25() {
	playSound(0x10bc);
	return 0;
}

int PSound2::command26() {
	playSound(0x10c8);
	return 0;
}

int PSound2::command27() {
	playSound(0x10d4);
	return 0;
}

int PSound2::command28() {
	const uint16 random = nextRandom();
	writeDataByte(0x10f7, random & 0x7f);
	const byte base = (random & 0x0f) + 0x2c;
	writeDataByte(0x10f8, base);
	writeDataByte(0x10fa, base + 0x0c);
	playSound(0x10f0);
	return 0;
}

int PSound2::command29() {
	playSound(0x10fe);
	return 0;
}

int PSound2::command30() {
	playSound(0x1136);
	return 0;
}

int PSound2::command31() {
	playSound(0x113e);
	return 0;
}

int PSound2::command32() {
	playSound(0x1152);
	return 0;
}

int PSound2::command33() {
	playSound(0x115c);
	return 0;
}

int PSound2::command34() {
	playSound(0x1164);
	return 0;
}

int PSound2::command35() {
	playSound(0x1174);
	return 0;
}

int PSound2::command36() {
	playSound(0x11ca);
	return 0;
}

int PSound2::command37() {
	playSound(0x1188);
	return 0;
}

int PSound2::command38() {
	loadCommand38Music();
	return 0;
}

int PSound2::command39() {
	writeDataByte(0x119c, (nextRandom() & 7) + 0x55);
	playSound(0x1196);
	return 0;
}

int PSound2::command40() {
	playSound(0x119e);
	return 0;
}

int PSound2::command41() {
	playSound(0x1120);
	return 0;
}

int PSound2::command42() {
	playSound(0x1146);
	return 0;
}

int PSound2::command43() {
	playSound(0x11b0);
	return 0;
}


// -------------------------------------------------------------------------
// Retail section 3
// -------------------------------------------------------------------------

const PSound3::CommandPtr PSound3::_commandList[61] = {
	&PSound3::command0, &PSound3::command1, &PSound3::command2, &PSound3::command3,
	&PSound3::command4, &PSound3::command5, &PSound3::command6, &PSound3::command7,
	&PSound3::command8, &PSound3::command9, &PSound3::command10, &PSound3::command11,
	&PSound3::nullCommand, &PSound3::command13, &PSound3::command14, &PSound3::command15,
	&PSound3::command16, &PSound3::command17, &PSound3::command18, &PSound3::command19,
	&PSound3::command20, &PSound3::command21, &PSound3::command22, &PSound3::command23,
	&PSound3::command24, &PSound3::command25, &PSound3::command26, &PSound3::command27,
	&PSound3::command28, &PSound3::command29, &PSound3::command30, &PSound3::command31,
	&PSound3::command32, &PSound3::command33, &PSound3::command34, &PSound3::command35,
	&PSound3::command36, &PSound3::command37, &PSound3::command38, &PSound3::command39,
	&PSound3::command40, &PSound3::command41, &PSound3::command42, &PSound3::command43,
	&PSound3::command44, &PSound3::command45, &PSound3::nullCommand, &PSound3::nullCommand,
	&PSound3::nullCommand, &PSound3::nullCommand, &PSound3::nullCommand, &PSound3::command51,
	&PSound3::nullCommand, &PSound3::nullCommand, &PSound3::nullCommand, &PSound3::nullCommand,
	&PSound3::nullCommand, &PSound3::command57, &PSound3::nullCommand, &PSound3::command59,
	&PSound3::command60
};

static const PSoundDriverData kPSound3Data = {
	"PSOUND.003",
	0x2450, 0x44f2, 0x48d0,
	0x2336, 0x3972, 32, PSound::kMusicChannelCount,
	{ 0x0062, 0x00e2, 0x00fe, 0x0108, 0x0112 }
};

PSound3::PSound3(Audio::Mixer *mixer) :
		PSound(mixer, kPSound3Data),
		_command39Toggle(false),
		_stopFadeReload(4) {
}

void PSound3::loadCommand10Music() {
	if (isSoundActive(0x181e))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x181e);
	loadChannel(1, 0x191c);
	loadChannel(2, 0x1ae0);
	loadChannel(3, 0x1c6c);
	loadChannel(4, 0x1ca6);
	loadChannel(5, 0x1ce2);
}

void PSound3::loadCommand11Music() {
	requestStop(0, kChannelCount);
	loadChannel(0, 0x0136);
	loadChannel(1, 0x038a);
	loadChannel(2, 0x05aa);
	loadChannel(3, 0x0c48);
	loadChannel(4, 0x1090);
	loadChannel(5, 0x1234);
}

void PSound3::loadCommand13Music() {
	if (isSoundActive(0x34e4))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x34e4);
	loadChannel(1, 0x3524);
	loadChannel(2, 0x3564);
	loadChannel(3, 0x35a4);
	loadChannel(4, 0x35e4);
	loadChannel(5, 0x3624);
}

void PSound3::loadCommand14Music() {
	if (isSoundActive(0x3664))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x3664);
	loadChannel(1, 0x3688);
	loadChannel(2, 0x36ac);
	loadChannel(3, 0x36d0);
	loadChannel(4, 0x36f0);
	loadChannel(5, 0x373c);
}

void PSound3::loadCommand16Music() {
	requestStop(0, kChannelCount);
	playSoundAny(0x1d2c);
	playSoundAny(0x1eb6);
	playSoundAny(0x2014);
	playSoundAny(0x21ae);
}

void PSound3::loadCommand17Music() {
	if (isSoundActive(0x2514))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x2514);
	loadChannel(1, 0x26a6);
	loadChannel(2, 0x28f4);
	loadChannel(3, 0x2aa0);
	loadChannel(4, 0x2df6);
	loadChannel(5, 0x31cc);
}

void PSound3::loadCommand18Music() {
	if (isSoundActive(0x3786))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x3786);
	loadChannel(1, 0x3814);
	loadChannel(2, 0x38c0);
	loadChannel(3, 0x3918);
}

int PSound3::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSound3::command1() {
	_stopFadeReload = 1;
	return PSound::command1();
}

int PSound3::command3() {
	_stopFadeReload = 1;
	return PSound::command3();
}

int PSound3::command9() {
	// Section 3 adds a parameterized global stop. It marks every channel
	// pending and stores twice the low command byte as the fade cadence.
	requestStop(0, kChannelCount);
	_stopFadeReload = (byte)((byte)_commandParam << 1);
	return 0;
}

int PSound3::command10() {
	loadCommand10Music();
	return 0;
}

int PSound3::command11() {
	loadCommand11Music();
	return 0;
}

int PSound3::command13() {
	loadCommand13Music();
	return 0;
}

int PSound3::command14() {
	loadCommand14Music();
	return 0;
}

int PSound3::command15() {
	loadCommand11Music();
	for (uint i = 0; i < kMusicChannelCount; ++i)
		_channels[i].patchAttenuation = (int8)0xf4;
	return 0;
}

int PSound3::command16() {
	loadCommand16Music();
	return 0;
}

int PSound3::command17() {
	loadCommand17Music();
	return 0;
}

int PSound3::command18() {
	loadCommand18Music();
	return 0;
}

int PSound3::command19() {
	playSound(0x2398);
	return 0;
}

int PSound3::command20() {
	playSound(0x238e);
	return 0;
}

int PSound3::command21() {
	loadChannel(8, 0x23a0);
	return 0;
}

int PSound3::command22() {
	loadChannel(8, 0x23a8);
	return 0;
}

int PSound3::command23() {
	loadChannel(7, 0x23c2);
	loadChannel(8, 0x23b8);
	return 0;
}

int PSound3::command24() {
	playSound(0x2350);
	return 0;
}

int PSound3::command25() {
	playSound(0x2338);
	return 0;
}

int PSound3::command26() {
	playSound(0x23cc);
	return 0;
}

int PSound3::command27() {
	playSound(0x248c);
	return 0;
}

int PSound3::command28() {
	playSound(0x23dc);
	return 0;
}

int PSound3::command29() {
	playSound(0x23e4);
	return 0;
}

int PSound3::command30() {
	playSound(0x2378);
	return 0;
}

int PSound3::command31() {
	playSound(0x23ee);
	return 0;
}

int PSound3::command32() {
	playSound(0x249e);
	return 0;
}

int PSound3::command33() {
	playSound(0x24a8);
	return 0;
}

int PSound3::command34() {
	playSound(0x24b0);
	return 0;
}

int PSound3::command35() {
	playSound(0x24c0);
	return 0;
}

int PSound3::command36() {
	playSound(0x24f6);
	return 0;
}

int PSound3::command37() {
	playSound(0x24d4);
	return 0;
}

int PSound3::command38() {
	playSound(0x241e);
	return 0;
}

int PSound3::command39() {
	_command39Toggle = !_command39Toggle;
	playSound(_command39Toggle ? 0x2446 : 0x243e);
	return 0;
}

int PSound3::command40() {
	_command39Toggle = !_command39Toggle;
	playSound(_command39Toggle ? 0x2456 : 0x244e);
	return 0;
}

int PSound3::command41() {
	playSound(0x2380);
	return 0;
}

int PSound3::command42() {
	playSound(0x24e2);
	return 0;
}

int PSound3::command43() {
	playSound(0x2428);
	playSound(0x2432);
	return 0;
}

int PSound3::command44() {
	playSound(0x2468);
	return 0;
}

int PSound3::command45() {
	playSound(0x2476);
	return 0;
}

int PSound3::command51() {
	playSound(0x245e);
	return 0;
}

int PSound3::command57() {
	writeDataByte(0x248a, (nextRandom() & 7) + 0x55);
	playSound(0x2484);
	return 0;
}

int PSound3::command59() {
	playSound(0x23d4);
	return 0;
}

int PSound3::command60() {
	playSound(0x2416);
	return 0;
}



// -------------------------------------------------------------------------
// Retail section 4
// -------------------------------------------------------------------------

const PSound4::CommandPtr PSound4::_commandList[60] = {
	&PSound4::command0, &PSound4::command1, &PSound4::command2, &PSound4::command3,
	&PSound4::command4, &PSound4::command5, &PSound4::command6, &PSound4::command7,
	&PSound4::command8, &PSound4::command9, &PSound4::command10, &PSound4::nullCommand,
	&PSound4::command12, &PSound4::nullCommand, &PSound4::nullCommand, &PSound4::nullCommand,
	&PSound4::nullCommand, &PSound4::nullCommand, &PSound4::nullCommand, &PSound4::command19,
	&PSound4::command20, &PSound4::command21, &PSound4::command22, &PSound4::command23,
	&PSound4::nullCommand, &PSound4::nullCommand, &PSound4::nullCommand, &PSound4::command27,
	&PSound4::nullCommand, &PSound4::nullCommand, &PSound4::command30, &PSound4::nullCommand,
	&PSound4::command32, &PSound4::command33, &PSound4::command34, &PSound4::command35,
	&PSound4::command36, &PSound4::command37, &PSound4::command38, &PSound4::nullCommand,
	&PSound4::nullCommand, &PSound4::nullCommand, &PSound4::nullCommand, &PSound4::nullCommand,
	&PSound4::nullCommand, &PSound4::nullCommand, &PSound4::nullCommand, &PSound4::nullCommand,
	&PSound4::nullCommand, &PSound4::nullCommand, &PSound4::nullCommand, &PSound4::nullCommand,
	&PSound4::command52, &PSound4::command53, &PSound4::command54, &PSound4::command55,
	&PSound4::command56, &PSound4::command57, &PSound4::command58, &PSound4::command59
};

static const PSoundDriverData kPSound4Data = {
	"PSOUND.004",
	0x23a0, 0x1caf, 0x2080,
	0x08b6, 0x0136, 30, PSound::kMusicChannelCount,
	{ 0x0062, 0x00e2, 0x00fe, 0x0108, 0x0112 }
};

PSound4::PSound4(Audio::Mixer *mixer) :
		PSound(mixer, kPSound4Data),
		_stopFadeReload(4) {
}

void PSound4::loadCommand10Music() {
	if (isSoundActive(0x175a))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x175a);
	loadChannel(1, 0x1876);
	loadChannel(2, 0x1a5a);
	loadChannel(3, 0x1be4);
	loadChannel(4, 0x1c24);
	loadChannel(5, 0x1c66);
}

void PSound4::loadCommand12Music(int param) {
	// The executable has a fast path when channel 0 already owns the primary
	// stream. If the same cue is active elsewhere, the handler returns without
	// changing attenuation.
	const bool primaryOnChannel0 = _channels[0].activeCount &&
			_channels[0].sequenceStart == 0x0be2;
	if (!primaryOnChannel0) {
		if (isSoundActive(0x0be2))
			return;

		requestStop(0, kChannelCount);
		loadChannel(0, 0x0be2);
		loadChannel(1, 0x0e08);
		loadChannel(2, 0x0f98);
		loadChannel(3, 0x1072);
		loadChannel(4, 0x11cc);
		loadChannel(5, 0x1402);
	}

	const uint16 value = (uint16)param;
	const int8 attenuation = (int8)((value >> 3) + (value >> 5) - 0x12);
	for (uint i = 0; i < kMusicChannelCount; ++i)
		_channels[i].patchAttenuation = attenuation;
}

bool PSound4::loadCommand53Music() {
	if (isSoundActive(0x1632))
		return false;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x1632);
	loadChannel(1, 0x166c);
	loadChannel(2, 0x169c);
	loadChannel(3, 0x16ce);
	loadChannel(4, 0x16f6);
	loadChannel(5, 0x1712);

	for (uint i = 0; i < kMusicChannelCount; ++i)
		_channels[i].patchAttenuation = (int8)0xd8;
	return true;
}

int PSound4::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSound4::command1() {
	_stopFadeReload = 1;
	return PSound::command1();
}

int PSound4::command3() {
	_stopFadeReload = 1;
	return PSound::command3();
}

int PSound4::command9() {
	requestStop(0, kChannelCount);
	_stopFadeReload = (byte)_commandParam;
	return 0;
}

int PSound4::command10() {
	loadCommand10Music();
	return 0;
}

int PSound4::command12() {
	loadCommand12Music(_commandParam);
	return 0;
}

int PSound4::command19() {
	playSound(0x08d0);
	return 0;
}

int PSound4::command20() {
	playSound(0x08d8);
	return 0;
}

int PSound4::command21() {
	loadChannel(8, 0x0978);
	return 0;
}

int PSound4::command22() {
	loadChannel(8, 0x0980);
	return 0;
}

int PSound4::command23() {
	loadChannel(7, 0x099a);
	loadChannel(8, 0x0990);
	return 0;
}

int PSound4::command27() {
	playSound(0x0904);
	return 0;
}

int PSound4::command30() {
	playSound(0x08c8);
	return 0;
}

int PSound4::command32() {
	playSound(0x0916);
	return 0;
}

int PSound4::command33() {
	playSound(0x0920);
	return 0;
}

int PSound4::command34() {
	playSound(0x0928);
	return 0;
}

int PSound4::command35() {
	playSound(0x0938);
	return 0;
}

int PSound4::command36() {
	playSound(0x095a);
	return 0;
}

int PSound4::command37() {
	playSound(0x094c);
	return 0;
}

int PSound4::command38() {
	playSound(0x08f2);
	return 0;
}

int PSound4::command52() {
	if (_channels[1].sequenceStart == 0x1876 &&
			!isSoundActive(0x09a4)) {
		loadChannel(0, 0x09a4);
		_channels[1].patchAttenuation = (int8)0xd8;
		_channels[2].patchAttenuation = (int8)0xd8;
	}
	return 0;
}

int PSound4::command53() {
	if (loadCommand53Music())
		_channels[0].patchAttenuation = 0;
	return 0;
}

int PSound4::command54() {
	if (loadCommand53Music()) {
		_channels[1].patchAttenuation = 0;
		_channels[2].patchAttenuation = 0;
	}
	return 0;
}

int PSound4::command55() {
	if (loadCommand53Music()) {
		_channels[3].patchAttenuation = 0;
		_channels[4].patchAttenuation = 0;
	}
	return 0;
}

int PSound4::command56() {
	if (loadCommand53Music())
		_channels[5].patchAttenuation = 0;
	return 0;
}

int PSound4::command57() {
	writeDataByte(0x0902, (nextRandom() & 7) + 0x55);
	playSound(0x08fc);
	return 0;
}

int PSound4::command58() {
	if (_channels[0].sequenceStart == 0x09a4) {
		loadChannel(0, 0x175a);
		_channels[1].patchAttenuation = 0;
		_channels[2].patchAttenuation = 0;
	}
	return 0;
}

int PSound4::command59() {
	playSound(0x08e2);
	return 0;
}



// -------------------------------------------------------------------------
// Retail section 5
// -------------------------------------------------------------------------

const PSound5::CommandPtr PSound5::_commandList[42] = {
	&PSound5::command0, &PSound5::command1, &PSound5::command2, &PSound5::command3,
	&PSound5::command4, &PSound5::command5, &PSound5::command6, &PSound5::command7,
	&PSound5::command8, &PSound5::command9, &PSound5::command10, &PSound5::command11122425,
	&PSound5::command11122425, &PSound5::command13, &PSound5::command14, &PSound5::command15,
	&PSound5::command16, &PSound5::command17, &PSound5::command18, &PSound5::command1921,
	&PSound5::command20, &PSound5::command1921, &PSound5::command22, &PSound5::command23,
	&PSound5::command11122425, &PSound5::command11122425, &PSound5::command26, &PSound5::command27,
	&PSound5::command28, &PSound5::command29, &PSound5::command30, &PSound5::command31,
	&PSound5::command32, &PSound5::command33, &PSound5::command34, &PSound5::command35,
	&PSound5::command36, &PSound5::command37, &PSound5::command38, &PSound5::command39,
	&PSound5::command40, &PSound5::command41
};

static const PSoundDriverData kPSound5Data = {
	"PSOUND.005",
	0x22f0, 0x15f2, 0x19d0,
	0x09b6, 0x0136, 34, PSound::kMusicChannelCount,
	{ 0x0062, 0x00e2, 0x00fe, 0x0108, 0x0112 }
};

PSound5::PSound5(Audio::Mixer *mixer) :
		PSound(mixer, kPSound5Data) {
}

void PSound5::loadCommand29Music() {
	if (isSoundActive(0x0d8a))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x0d8a);
	loadChannel(1, 0x0eaa);
	loadChannel(2, 0x0fce);
	loadChannel(3, 0x10a2);
	loadChannel(4, 0x12a6);
}

int PSound5::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSound5::command9() {
	writeDataByte(0x09f0, (nextRandom() & 7) + 0x55);
	playSound(0x09ea);
	return 0;
}

int PSound5::command10() {
	playSound(0x0a48);
	return 0;
}

int PSound5::command11122425() {
	playSound(0x09d0);
	return 0;
}

int PSound5::command13() {
	playSound(0x0a60);
	return 0;
}

int PSound5::command14() {
	loadChannel(8, 0x0af6);
	return 0;
}

int PSound5::command15() {
	if (_channels[8].sequenceStart == 0x0af6) {
		// Change the restart target without reloading the channel.
		_channels[8].originalSequence = 0x0b0c;
		_channels[8].innerLoopCount = 1;
		_channels[8].outerLoopCount = 1;
	}
	return 0;
}

int PSound5::command16() {
	playSound(0x0a58);
	return 0;
}

int PSound5::command17() {
	playSound(0x0a50);
	return 0;
}

int PSound5::command18() {
	playSound(0x0ad4);
	return 0;
}

int PSound5::command1921() {
	playSound(0x0a8c);
	return 0;
}

int PSound5::command20() {
	playSound(0x0a7c);
	return 0;
}

int PSound5::command22() {
	playSound(0x0a74);
	return 0;
}

int PSound5::command23() {
	playSound(0x0a6a);
	return 0;
}

int PSound5::command26() {
	playSound(0x0a9e);
	return 0;
}

int PSound5::command27() {
	playSound(0x0ab0);
	return 0;
}

int PSound5::command28() {
	playSound(0x09f2);
	return 0;
}

int PSound5::command29() {
	loadCommand29Music();
	return 0;
}

int PSound5::command30() {
	playSound(0x09c8);
	return 0;
}

int PSound5::command31() {
	playSound(0x0aea);
	return 0;
}

int PSound5::command32() {
	playSound(0x0a04);
	return 0;
}

int PSound5::command33() {
	playSound(0x0a0e);
	return 0;
}

int PSound5::command34() {
	playSound(0x0a16);
	return 0;
}

int PSound5::command35() {
	playSound(0x0a26);
	return 0;
}

int PSound5::command36() {
	playSound(0x0b18);
	return 0;
}

int PSound5::command37() {
	playSound(0x0a3a);
	return 0;
}

int PSound5::command38() {
	if (_channels[3].sequenceStart == 0x0b4c) {
		loadChannel(3, 0x10a2);
		loadChannel(4, 0x12a6);
	}
	return 0;
}

int PSound5::command39() {
	loadChannel(6, 0x0b38);
	return 0;
}

int PSound5::command40() {
	loadChannel(6, 0x0b40);
	return 0;
}

int PSound5::command41() {
	if (!isSoundActive(0x0b4c) &&
			_channels[3].sequenceStart == 0x10a2) {
		loadChannel(3, 0x0b4c);
		loadChannel(4, _nullSequenceOffset);
	}
	return 0;
}


// -------------------------------------------------------------------------
// Retail section 6
// -------------------------------------------------------------------------

const PSound6::CommandPtr PSound6::_commandList[30] = {
	&PSound6::command0, &PSound6::command1, &PSound6::command2, &PSound6::command3,
	&PSound6::command4, &PSound6::command5, &PSound6::command6, &PSound6::command7,
	&PSound6::command8, &PSound6::command9, &PSound6::command10, &PSound6::command11,
	&PSound6::command12, &PSound6::command13, &PSound6::command14, &PSound6::command15,
	&PSound6::command16, &PSound6::command17, &PSound6::command18, &PSound6::command19,
	&PSound6::command20, &PSound6::command21, &PSound6::command22, &PSound6::command23,
	&PSound6::command24, &PSound6::command25, &PSound6::nullCommand, &PSound6::nullCommand,
	&PSound6::nullCommand, &PSound6::command29
};

static const PSoundDriverData kPSound6Data = {
	"PSOUND.006",
	0x2250, 0x17c8, 0x1ba0,
	0x0936, 0x0136, 32, PSound::kMusicChannelCount,
	{ 0x0062, 0x00e2, 0x00fe, 0x0108, 0x0112 }
};

PSound6::PSound6(Audio::Mixer *mixer) :
		PSound(mixer, kPSound6Data) {
}

void PSound6::loadCommand24Music() {
	if (isSoundActive(0x0abe))
		return;

	requestStop(0, kChannelCount);
	playSoundAny(0x0abe);
	playSoundAny(0x0d8e);
	playSoundAny(0x0f28);
}

void PSound6::loadCommand29Music() {
	if (isSoundActive(0x0f60))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x0f60);
	loadChannel(1, 0x1080);
	loadChannel(2, 0x11a4);
	loadChannel(3, 0x1278);
	loadChannel(4, 0x147c);
}

int PSound6::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSound6::command9() {
	writeDataByte(0x0946, (nextRandom() & 7) + 0x55);
	playSound(0x0940);
	return 0;
}

int PSound6::command10() {
	playSound(0x09e2);
	return 0;
}

int PSound6::command11() {
	playSound(0x09c0);
	return 0;
}

int PSound6::command12() {
	playSound(0x0a0e);
	return 0;
}

int PSound6::command13() {
	playSound(0x0a74);
	return 0;
}

int PSound6::command14() {
	playSound(0x0a90);
	return 0;
}

int PSound6::command15() {
	playSound(0x0968);
	return 0;
}

int PSound6::command16() {
	playSound(0x0974);
	playSound(0x0968);
	return 0;
}

int PSound6::command17() {
	playSound(0x098a);
	return 0;
}

int PSound6::command18() {
	playSound(0x0a42);
	return 0;
}

int PSound6::command19() {
	playSound(0x0a52);
	return 0;
}

int PSound6::command20() {
	playSound(0x09fa);
	return 0;
}

int PSound6::command21() {
	playSound(0x0a20);
	return 0;
}

int PSound6::command22() {
	playSound(0x0a62);
	return 0;
}

int PSound6::command23() {
	playSound(0x0948);
	return 0;
}

int PSound6::command24() {
	loadCommand24Music();
	return 0;
}

int PSound6::command25() {
	playSound(0x0aac);
	return 0;
}

int PSound6::command29() {
	loadCommand29Music();
	return 0;
}



// -------------------------------------------------------------------------
// Retail section 7
// -------------------------------------------------------------------------

const PSound7::CommandPtr PSound7::_commandList[38] = {
	&PSound7::command0, &PSound7::command1, &PSound7::command2, &PSound7::command3,
	&PSound7::command4, &PSound7::command5, &PSound7::command6, &PSound7::command7,
	&PSound7::command8, &PSound7::command9, &PSound7::nullCommand, &PSound7::nullCommand,
	&PSound7::nullCommand, &PSound7::nullCommand, &PSound7::nullCommand, &PSound7::command15,
	&PSound7::command1617, &PSound7::command1617, &PSound7::command18, &PSound7::command19,
	&PSound7::command20, &PSound7::command21, &PSound7::command22, &PSound7::command23,
	&PSound7::command24, &PSound7::command25, &PSound7::command26, &PSound7::command27,
	&PSound7::command28, &PSound7::nullCommand, &PSound7::command30, &PSound7::nullCommand,
	&PSound7::command32, &PSound7::command33, &PSound7::command34, &PSound7::command35,
	&PSound7::nullCommand, &PSound7::command37
};

static const PSoundDriverData kPSound7Data = {
	"PSOUND.007",
	0x2300, 0x2039, 0x2410,
	0x09f6, 0x0136, 35, PSound::kMusicChannelCount,
	{ 0x0062, 0x00e2, 0x00fe, 0x0108, 0x0112 }
};

PSound7::PSound7(Audio::Mixer *mixer) :
		PSound(mixer, kPSound7Data) {
}

void PSound7::loadCommand9Music() {
	if (isSoundActive(0x1ef4))
		return;

	requestStop(0, kChannelCount);
	playSoundAny(0x1ef4);
	playSoundAny(0x1f4e);
	playSoundAny(0x1fa6);
	playSoundAny(0x200c);
}

void PSound7::loadCommand24Music() {
	if (isSoundActive(0x1658))
		return;

	requestStop(0, kChannelCount);
	playSoundAny(0x1658);
	playSoundAny(0x171e);
	playSoundAny(0x17cc);
	playSoundAny(0x1872);
	playSoundAny(0x190a);
}

void PSound7::loadCommand25Music() {
	if (isSoundActive(0x0ac6))
		return;

	requestStop(0, kChannelCount);
	playSoundAny(0x0ac6);
	playSoundAny(0x0b7a);
	playSoundAny(0x0c30);
	playSoundAny(0x0cec);
}

void PSound7::loadCommand26Music() {
	if (isSoundActive(0x0dda))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x0dda);
	loadChannel(1, 0x0efa);
	loadChannel(2, 0x101e);
	loadChannel(3, 0x10f2);
	loadChannel(4, 0x1302);
}

void PSound7::loadCommand27Music() {
	if (isSoundActive(0x195e))
		return;

	requestStop(0, kChannelCount);
	playSoundAny(0x195e);
	playSoundAny(0x1a32);
	playSoundAny(0x1b3e);
	playSoundAny(0x1c60);
	playSoundAny(0x1dac);
	playSoundAny(0x1ef2);
}

int PSound7::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSound7::command9() {
	loadCommand9Music();
	return 0;
}

int PSound7::command15() {
	writeDataByte(0x0a06, (nextRandom() & 7) + 0x55);
	playSound(0x0a00);
	return 0;
}

int PSound7::command1617() {
	playSound(0x0abe);
	return 0;
}

int PSound7::command18() {
	loadChannel(8, 0x0a70);
	return 0;
}

int PSound7::command19() {
	if (_channels[8].sequenceStart == 0x0a70) {
		// The handler changes the eventual restart target and both nested
		// loop counters without reloading or repositioning channel 8.
		_channels[8].originalSequence = 0x0a86;
		_channels[8].innerLoopCount = 1;
		_channels[8].outerLoopCount = 1;
	}
	return 0;
}

int PSound7::command20() {
	playSound(0x0aac);
	return 0;
}

int PSound7::command21() {
	playSound(0x0aa2);
	return 0;
}

int PSound7::command22() {
	playSound(0x0a64);
	return 0;
}

int PSound7::command23() {
	playSound(0x0a08);
	return 0;
}

int PSound7::command24() {
	loadCommand24Music();
	return 0;
}

int PSound7::command25() {
	loadCommand25Music();
	return 0;
}

int PSound7::command26() {
	loadCommand26Music();
	return 0;
}

int PSound7::command27() {
	loadCommand27Music();
	return 0;
}

int PSound7::command28() {
	loadChannel(8, 0x0a92);
	return 0;
}

int PSound7::command30() {
	playSound(0x0a28);
	return 0;
}

int PSound7::command32() {
	playSound(0x0a30);
	return 0;
}

int PSound7::command33() {
	playSound(0x0a3a);
	return 0;
}

int PSound7::command34() {
	playSound(0x0a42);
	return 0;
}

int PSound7::command35() {
	playSound(0x0a52);
	return 0;
}

int PSound7::command37() {
	playSound(0x0a1a);
	return 0;
}



// -------------------------------------------------------------------------
// Retail section 8
// -------------------------------------------------------------------------

const PSound8::CommandPtr PSound8::_commandList[38] = {
	&PSound8::command0, &PSound8::command1, &PSound8::command2, &PSound8::command3,
	&PSound8::command4, &PSound8::command5, &PSound8::command6, &PSound8::command7,
	&PSound8::command8, &PSound8::command9, &PSound8::nullCommand, &PSound8::command11,
	&PSound8::command12, &PSound8::command13, &PSound8::command14, &PSound8::command15,
	&PSound8::command16, &PSound8::command17, &PSound8::command18, &PSound8::command19,
	&PSound8::command20, &PSound8::command21, &PSound8::command22, &PSound8::nullCommand,
	&PSound8::command24, &PSound8::command25, &PSound8::command26, &PSound8::command27,
	&PSound8::command28, &PSound8::command29, &PSound8::command30, &PSound8::command31,
	&PSound8::command32, &PSound8::command33, &PSound8::command34, &PSound8::command35,
	&PSound8::nullCommand, &PSound8::command37
};

static const PSoundDriverData kPSound8Data = {
	"PSOUND.008",
	0x22e0, 0x0f7a, 0x1350,
	0x061a, 0x077a, 32, PSound::kMusicChannelCount,
	{ 0x0062, 0x00e2, 0x00fe, 0x0108, 0x0112 }
};

PSound8::PSound8(Audio::Mixer *mixer) :
		PSound(mixer, kPSound8Data) {
}

void PSound8::mutateCommand28Sequence() {
	uint16 random;
	do {
		random = nextRandom() & 0x3f;
	} while (random > 0x24);

	// The executable generates ten descending note bytes in the odd byte
	// positions of the mutable stream body.
	for (uint i = 0; i < 10; ++i)
		writeDataByte(0x01e7 + i * 2, (byte)(random + 0x14 - i));

	writeDataByte(0x01df, (byte)nextRandom());

	// The matching duration/control bytes rise from a value derived from the
	// accepted random note range. The 8086 DIV is an unsigned quotient.
	const byte firstValue = 10 - (byte)((random + 1) / 6);
	for (uint i = 0; i < 10; ++i)
		writeDataByte(0x01e8 + i * 2, firstValue + i);
}

void PSound8::loadCommand28Music() {
	if (isSoundActive(0x0136))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x0136);
	loadChannel(1, 0x0202);
	mutateCommand28Sequence();
	loadChannel(2, 0x01dc);
}

void PSound8::loadCommand29Music() {
	if (isSoundActive(0x02aa))
		return;

	requestStop(0, kChannelCount);
	loadChannel(0, 0x02aa);
	loadChannel(1, 0x02fe);
	loadChannel(2, 0x0564);
}

int PSound8::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSound8::command9() {
	writeDataByte(0x069a, (nextRandom() & 7) + 0x55);
	playSound(0x0694);
	return 0;
}

int PSound8::command11() {
	playSound(0x0742);
	return 0;
}

int PSound8::command12() {
	playSound(0x074e);
	return 0;
}

int PSound8::command13() {
	playSound(0x0636);
	return 0;
}

int PSound8::command14() {
	loadChannel(8, 0x06b4);
	return 0;
}

int PSound8::command15() {
	loadChannel(8, 0x06cc);
	return 0;
}

int PSound8::command16() {
	playSound(0x062a);
	return 0;
}

int PSound8::command17() {
	playSound(0x0762);
	return 0;
}

int PSound8::command18() {
	playSound(0x076e);
	return 0;
}

int PSound8::command19() {
	playSound(0x06f4);
	return 0;
}

int PSound8::command20() {
	playSound(0x06fc);
	return 0;
}

int PSound8::command21() {
	playSound(0x075a);
	return 0;
}

int PSound8::command22() {
	loadChannel(6, 0x0704);
	loadChannel(7, 0x0712);
	loadChannel(8, 0x0720);
	return 0;
}

int PSound8::command24() {
	playSound(0x072e);
	return 0;
}

int PSound8::command25() {
	playSound(0x0736);
	return 0;
}

int PSound8::command26() {
	playSound(0x06de);
	return 0;
}

int PSound8::command27() {
	playSound(0x069c);
	return 0;
}

int PSound8::command28() {
	loadCommand28Music();
	return 0;
}

int PSound8::command29() {
	loadCommand29Music();
	return 0;
}

int PSound8::command30() {
	playSound(0x0640);
	return 0;
}

int PSound8::command31() {
	playSound(0x0622);
	return 0;
}

int PSound8::command32() {
	playSound(0x0650);
	return 0;
}

int PSound8::command33() {
	playSound(0x065a);
	return 0;
}

int PSound8::command34() {
	playSound(0x0662);
	return 0;
}

int PSound8::command35() {
	playSound(0x0672);
	return 0;
}

int PSound8::command37() {
	playSound(0x0686);
	return 0;
}



// -------------------------------------------------------------------------
// Retail section 9
// -------------------------------------------------------------------------

const PSound9::CommandPtr PSound9::_commandList[52] = {
	&PSound9::command0, &PSound9::command1, &PSound9::command2, &PSound9::command3,
	&PSound9::command4, &PSound9::command5, &PSound9::command6, &PSound9::command7,
	&PSound9::command8, &PSound9::command9, &PSound9::command10, &PSound9::command11,
	&PSound9::command12, &PSound9::command13, &PSound9::command14, &PSound9::command15,
	&PSound9::command16, &PSound9::command17, &PSound9::command18, &PSound9::command19,
	&PSound9::command20, &PSound9::command21, &PSound9::command22, &PSound9::command23,
	&PSound9::command24, &PSound9::command25, &PSound9::command26, &PSound9::command27,
	&PSound9::command28, &PSound9::command29, &PSound9::command30, &PSound9::command31,
	&PSound9::command32, &PSound9::command33, &PSound9::command34, &PSound9::command35,
	&PSound9::command36, &PSound9::command37, &PSound9::command38, &PSound9::command39,
	&PSound9::command40, &PSound9::command41, &PSound9::command42, &PSound9::command43,
	&PSound9::command4446, &PSound9::command45, &PSound9::command4446, &PSound9::command47,
	&PSound9::command48, &PSound9::command49, &PSound9::command50, &PSound9::command51
};

static const PSoundDriverData kPSound9Data = {
	"PSOUND.009",
	0x2630, 0x6d3c, 0x7110,
	0x5f3c, 0x62bc, 42, 7,
	{ 0x0068, 0x00e8, 0x0104, 0x010e, 0x0118 }
};

PSound9::PSound9(Audio::Mixer *mixer) :
		PSound(mixer, kPSound9Data),
		_scheduledCallbackOffset(0) {
}

void PSound9::scheduleCallback(uint16 callbackOffset) {
	// Store the callback offset used by the section timer.
	_scheduledCallbackOffset = callbackOffset;
	writeDataUint16(0x0064, callbackOffset);
}

void PSound9::tickCallback() {
	uint16 period = readDataUint16(0x0062);
	if (!period)
		return;

	uint16 counter = readDataUint16(0x0060);
	if (!counter)
		counter = period;
	if (--counter) {
		writeDataUint16(0x0060, counter);
		return;
	}

	writeDataUint16(0x0060, period);
	if (_scheduledCallbackOffset)
		runScheduledCallback();
}

void PSound9::runScheduledCallback() {
	const uint16 callbackOffset = _scheduledCallbackOffset;
	_scheduledCallbackOffset = 0;
	writeDataUint16(0x0064, 0);

	switch (callbackOffset) {
	case 0x0851:
		loadChannel(0, 0x0198);
		loadChannel(1, 0x081a);
		loadChannel(2, 0x0e64);
		loadChannel(3, 0x13b0);
		break;
	case 0x087c:
		loadChannel(0, 0x041e);
		loadChannel(1, 0x0a54);
		loadChannel(2, 0x0ffa);
		loadChannel(3, 0x155c);
		break;
	case 0x08a7:
		loadChannel(0, 0x06a8);
		loadChannel(1, 0x0c8a);
		loadChannel(2, 0x1190);
		loadChannel(3, 0x170a);
		break;
	case 0x096e:
		loadChannel(0, 0x19ee);
		loadChannel(1, 0x2012);
		loadChannel(2, 0x298e);
		loadChannel(3, 0x34d4);
		loadChannel(4, 0x3fb6);
		loadChannel(5, 0x49dc);
		break;
	case 0x09a7:
		loadChannel(0, 0x1a84);
		loadChannel(1, 0x20d8);
		loadChannel(2, 0x2a56);
		loadChannel(3, 0x3668);
		loadChannel(4, 0x4312);
		loadChannel(5, 0x4c46);
		break;
	case 0x09e0:
		loadChannel(0, 0x1bda);
		loadChannel(1, 0x2438);
		loadChannel(2, 0x2dbe);
		loadChannel(3, 0x39e0);
		loadChannel(4, 0x43b6);
		loadChannel(5, 0x4d5c);
		break;
	case 0x0a19:
		writeDataByte(0x3c2b, 0);
		writeDataByte(0x45fd, 0);
		writeDataByte(0x4f19, 0);
		loadChannel(0, 0x1cf0);
		loadChannel(1, 0x252c);
		loadChannel(2, 0x2ec8);
		loadChannel(3, 0x3bb0);
		loadChannel(4, 0x4552);
		loadChannel(5, 0x4e40);
		break;
	case 0x0b16:
		loadChannel(0, 0x5368);
		loadChannel(1, 0x56e2);
		loadChannel(2, 0x5580);
		loadChannel(3, 0x5bdc);
		loadChannel(4, 0x58ca);
		loadChannel(5, 0x5a7a);
		loadChannel(6, 0x5d52);
		break;
	case 0x0b56:
		loadChannel(0, 0x53e0);
		loadChannel(1, 0x5778);
		loadChannel(2, 0x55cc);
		loadChannel(3, 0x5c06);
		loadChannel(4, 0x5976);
		loadChannel(5, 0x5b20);
		loadChannel(6, 0x5e4a);
		break;
	default:
		break;
	}
}

void PSound9::loadCommand9Music() {
	if (isSoundActive(0x528c))
		return;

	requestStop(0, _musicChannelCount);
	writeDataUint16(0x0060, 0x0738);
	writeDataUint16(0x0062, 0x0054);
	loadChannel(0, 0x528c);
	loadChannel(1, 0x543e);
	loadChannel(2, 0x5622);
	loadChannel(3, 0x5b22);
	loadChannel(4, 0x57d6);
	loadChannel(5, 0x5978);
	loadChannel(6, 0x5c08);
}

void PSound9::loadCommand10Music() {
	if (isSoundActive(0x5eaa))
		return;

	requestStop(0, _musicChannelCount);
	loadChannel(0, 0x5eaa);
	loadChannel(1, 0x5ec6);
	loadChannel(2, 0x5ee4);
	loadChannel(3, 0x5efa);
	loadChannel(4, 0x5f10);
	loadChannel(5, 0x5f26);
}

void PSound9::loadCommand34Music() {
	writeDataByte(0x3c2b, 2);
	writeDataByte(0x45fd, 2);
	writeDataByte(0x4f19, 2);
	writeDataUint16(0x0060, 0x0060);
	writeDataUint16(0x0062, 0x0060);
	loadChannel(0, 0x184c);
	loadChannel(1, 0x1d6e);
	loadChannel(2, 0x25ac);
	loadChannel(3, 0x2f6e);
	loadChannel(4, 0x3c32);
	loadChannel(5, 0x4604);
}

void PSound9::loadCommand43Music() {
	if (isSoundActive(0x013c))
		return;

	requestStop(0, _musicChannelCount);
	playSoundAny(0x013c);
	playSoundAny(0x07bc);
	playSoundAny(0x0d9e);
	playSoundAny(0x12e4);
}

void PSound9::loadCommand49Music() {
	loadChannel(0, 0x4f20);
	loadChannel(1, 0x4f88);
	loadChannel(2, 0x4fec);
	loadChannel(3, 0x525e);
	loadChannel(4, 0x5210);
	loadChannel(5, 0x5230);
	loadChannel(6, 0x51f2);
}

void PSound9::loadCommand51Music() {
	writeDataByte(0x3c2b, 2);
	writeDataByte(0x45fd, 2);
	writeDataByte(0x4f19, 2);
	writeDataUint16(0x0060, 0x0060);
	writeDataUint16(0x0062, 0x0060);
	loadChannel(0, 0x1880);
	loadChannel(1, 0x1db2);
	loadChannel(2, 0x27f8);
	loadChannel(3, 0x31c6);
	loadChannel(4, 0x3c98);
	loadChannel(5, 0x4698);
}

int PSound9::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSound9::command0() {
	_scheduledCallbackOffset = 0;
	writeDataUint16(0x0060, 0);
	writeDataUint16(0x0062, 0);
	writeDataUint16(0x0064, 0);
	return PSound::command0();
}

int PSound9::command9() {
	loadCommand9Music();
	return 0;
}

int PSound9::command10() {
	loadCommand10Music();
	return 0;
}

int PSound9::command11() {
	loadChannel(7, 0x600e);
	loadChannel(8, 0x60a2);
	return 0;
}

int PSound9::command12() {
	loadChannel(8, 0x6184);
	return 0;
}

int PSound9::command13() {
	loadChannel(7, 0x61a0);
	return 0;
}

int PSound9::command14() {
	loadChannel(7, 0x5fd2);
	return 0;
}

int PSound9::command15() {
	loadChannel(7, 0x61ba);
	return 0;
}

int PSound9::command16() {
	loadChannel(7, 0x61d8);
	return 0;
}

int PSound9::command17() {
	loadChannel(7, 0x614a);
	return 0;
}

int PSound9::command18() {
	playSound(0x6292);
	return 0;
}

int PSound9::command19() {
	playSound(0x6260);
	return 0;
}

int PSound9::command20() {
	writeDataByte(0x5f60, (nextRandom() & 0x10) | 0x4d);
	loadChannel(7, 0x5f5c);
	return 0;
}

int PSound9::command21() {
	loadChannel(8, 0x5f74);
	return 0;
}

int PSound9::command22() {
	loadChannel(8, 0x5f84);
	return 0;
}

int PSound9::command23() {
	loadChannel(8, 0x5f64);
	return 0;
}

int PSound9::command24() {
	loadChannel(8, 0x6224);
	return 0;
}

int PSound9::command25() {
	loadChannel(7, 0x6248);
	return 0;
}

int PSound9::command26() {
	loadChannel(8, 0x6138);
	return 0;
}

int PSound9::command27() {
	loadChannel(8, 0x61f0);
	return 0;
}

int PSound9::command28() {
	loadChannel(7, 0x5fa2);
	return 0;
}

int PSound9::command29() {
	loadChannel(7, 0x5fae);
	return 0;
}

int PSound9::command30() {
	loadChannel(8, 0x5f94);
	return 0;
}

int PSound9::command31() {
	loadChannel(7, 0x5fe8);
	loadChannel(8, 0x5ff2);
	return 0;
}

int PSound9::command32() {
	loadChannel(7, 0x6202);
	return 0;
}

int PSound9::command33() {
	loadChannel(8, 0x620a);
	return 0;
}

int PSound9::command34() {
	loadCommand34Music();
	return 0;
}

int PSound9::command35() {
	loadChannel(8, 0x615c);
	return 0;
}

int PSound9::command36() {
	loadChannel(7, 0x5fba);
	loadChannel(8, 0x5fc2);
	return 0;
}

int PSound9::command37() {
	loadChannel(7, 0x621a);
	return 0;
}

int PSound9::command38() {
	scheduleCallback(0x0b16);
	return 0;
}

int PSound9::command39() {
	scheduleCallback(0x0b56);
	return 0;
}

int PSound9::command40() {
	scheduleCallback(0x096e);
	return 0;
}

int PSound9::command41() {
	scheduleCallback(0x09a7);
	return 0;
}

int PSound9::command42() {
	scheduleCallback(0x09e0);
	return 0;
}

int PSound9::command43() {
	loadCommand43Music();
	return 0;
}

int PSound9::command4446() {
	scheduleCallback(0x0851);
	return 0;
}

int PSound9::command45() {
	scheduleCallback(0x087c);
	return 0;
}

int PSound9::command47() {
	scheduleCallback(0x08a7);
	return 0;
}

int PSound9::command48() {
	loadChannel(7, 0x62b4);
	return 0;
}

int PSound9::command49() {
	loadCommand49Music();
	return 0;
}

int PSound9::command50() {
	scheduleCallback(0x0a19);
	return 0;
}

int PSound9::command51() {
	loadCommand51Music();
	return 0;
}


// -------------------------------------------------------------------------
// Demo section 1
// -------------------------------------------------------------------------

const PSoundDemo1::CommandPtr PSoundDemo1::_commandList[39] = {
	&PSoundDemo1::command0, &PSoundDemo1::command1, &PSoundDemo1::command2, &PSoundDemo1::command3,
	&PSoundDemo1::command4, &PSoundDemo1::command5, &PSoundDemo1::command6, &PSoundDemo1::command7,
	&PSoundDemo1::command8, &PSoundDemo1::nullCommand, &PSoundDemo1::command101112, &PSoundDemo1::command101112,
	&PSoundDemo1::command101112, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand,
	&PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand,
	&PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand,
	&PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand,
	&PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand,
	&PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand,
	&PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand, &PSoundDemo1::nullCommand
};

static const PSoundDriverData kPSoundDemo1Data = {
	"PSOUND.001",
	0x2130, 0x236a, 0x2740,
	0x2134, 0x0134, 32, PSound::kMusicChannelCount,
	{ 0x0060, 0x00e0, 0x00fc, 0x0106, 0x0110 }
};

PSoundDemo1::PSoundDemo1(Audio::Mixer *mixer) :
		PSound(mixer, kPSoundDemo1Data) {
}

void PSoundDemo1::loadDemoMusic() {
	if (isSoundActive(0x2140))
		return;

	requestStop(0, _musicChannelCount);
	playSoundAny(0x2140);
	playSoundAny(0x2180);
	playSoundAny(0x2306);
	playSoundAny(0x2338);
}

int PSoundDemo1::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSoundDemo1::command101112() {
	loadDemoMusic();
	return 0;
}


// -------------------------------------------------------------------------
// Demo section 9
// -------------------------------------------------------------------------

const PSoundDemo9::CommandPtr PSoundDemo9::_commandList[39] = {
	&PSoundDemo9::command0, &PSoundDemo9::command1, &PSoundDemo9::command2, &PSoundDemo9::command3,
	&PSoundDemo9::command4, &PSoundDemo9::command5, &PSoundDemo9::command6, &PSoundDemo9::command7,
	&PSoundDemo9::command8, &PSoundDemo9::nullCommand, &PSoundDemo9::nullCommand, &PSoundDemo9::command11,
	&PSoundDemo9::nullCommand, &PSoundDemo9::nullCommand, &PSoundDemo9::command14, &PSoundDemo9::nullCommand,
	&PSoundDemo9::nullCommand, &PSoundDemo9::command17, &PSoundDemo9::nullCommand, &PSoundDemo9::nullCommand,
	&PSoundDemo9::command20, &PSoundDemo9::command21, &PSoundDemo9::command22, &PSoundDemo9::command23,
	&PSoundDemo9::nullCommand, &PSoundDemo9::nullCommand, &PSoundDemo9::command26, &PSoundDemo9::nullCommand,
	&PSoundDemo9::command28, &PSoundDemo9::command29, &PSoundDemo9::command30, &PSoundDemo9::command31,
	&PSoundDemo9::nullCommand, &PSoundDemo9::nullCommand, &PSoundDemo9::command34, &PSoundDemo9::command35,
	&PSoundDemo9::command36, &PSoundDemo9::nullCommand, &PSoundDemo9::command38
};

static const PSoundDriverData kPSoundDemo9Data = {
	"PSOUND.009",
	0x21f0, 0x4486, 0x4860,
	0x2356, 0x0134, 32, PSound::kMusicChannelCount,
	{ 0x0060, 0x00e0, 0x00fc, 0x0106, 0x0110 }
};

PSoundDemo9::PSoundDemo9(Audio::Mixer *mixer) :
		PSound(mixer, kPSoundDemo9Data) {
}

void PSoundDemo9::loadCommand34Music() {
	if (isSoundActive(0x2428))
		return;

	requestStop(0, _musicChannelCount);
	playSoundAny(0x2428);
	playSoundAny(0x263a);
	playSoundAny(0x2952);
	playSoundAny(0x2ef8);
	playSoundAny(0x385c);
	playSoundAny(0x3e48);
}

void PSoundDemo9::loadCommand38Music() {
	if (isSoundActive(0x2362))
		return;

	requestStop(0, _musicChannelCount);
	playSoundAny(0x2362);
	playSoundAny(0x2374);
	playSoundAny(0x23a4);
	playSoundAny(0x23d6);
	playSoundAny(0x2402);
	playSoundAny(0x2416);
}

int PSoundDemo9::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	if (commandId < 0 || static_cast<uint>(commandId) >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int PSoundDemo9::command11() {
	loadChannel(7, 0x21e2);
	loadChannel(8, 0x2276);
	return 0;
}

int PSoundDemo9::command14() {
	loadChannel(6, 0x21a6);
	return 0;
}

int PSoundDemo9::command17() {
	loadChannel(7, 0x231c);
	return 0;
}

int PSoundDemo9::command20() {
	writeDataByte(0x2138, (nextRandom() & 0x10) | 0x4d);
	loadChannel(6, 0x2134);
	return 0;
}

int PSoundDemo9::command21() {
	loadChannel(6, 0x214c);
	return 0;
}

int PSoundDemo9::command22() {
	loadChannel(6, 0x215c);
	return 0;
}

int PSoundDemo9::command23() {
	loadChannel(6, 0x213c);
	return 0;
}

int PSoundDemo9::command26() {
	loadChannel(6, 0x230c);
	return 0;
}

int PSoundDemo9::command28() {
	loadChannel(8, 0x217a);
	return 0;
}

int PSoundDemo9::command29() {
	loadChannel(8, 0x2184);
	return 0;
}

int PSoundDemo9::command30() {
	loadChannel(7, 0x216c);
	return 0;
}

int PSoundDemo9::command31() {
	loadChannel(7, 0x21bc);
	loadChannel(8, 0x21c6);
	return 0;
}

int PSoundDemo9::command34() {
	loadCommand34Music();
	return 0;
}

int PSoundDemo9::command35() {
	loadChannel(6, 0x232e);
	return 0;
}

int PSoundDemo9::command36() {
	playSoundAny(0x218e);
	playSoundAny(0x2196);
	return 0;
}

int PSoundDemo9::command38() {
	loadCommand38Music();
	return 0;
}


} // namespace Sound
} // namespace RexNebular
} // namespace MADS
