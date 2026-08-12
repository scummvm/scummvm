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

#include "common/file.h"
#include "common/md5.h"
#include "common/util.h"
#include "mads/phantom/sound/rsound_phantom.h"

namespace MADS {
namespace Phantom {
namespace Sound {

RSound1::RSound1(Audio::Mixer *mixer) : RSound(mixer, "rsound.ph1", 0x2D20, 0x4B30, 0xDC) {
}

int RSound1::command(int commandId, int param) {
	_commandParam = param;
	_frameCounter = 0;

	switch (commandId) {
	case 0: return command0();
	case 1: return command1();
	case 2: return command2();
	case 3: return command3();
	case 4: return command4();
	case 5: return command5();
	case 6: return command6();
	case 7: return command7();
	case 8: return command8();
	case 16: return command16();
	case 24: return command24();
	case 25: return command25();
	case 26: return command26();
	case 27: return command27();
	case 32: return command32();
	case 33: return command33();
	case 34: return command34();
	case 35: return command35();
	case 36: return command36();
	case 37: return command37();
	case 38: return command38();
	case 39: return command39();
	case 64: return command64();
	case 65: return command65();
	case 66: return command66();
	case 67: return command67();
	case 68: return command68();
	case 69: return command69();
	case 70: return command70();
	case 71: return command71();
	case 72: return command72();
	case 73: return command73();
	case 74: return command74();
	case 75: return command75();
	case 76: return command76();
	default:
		// There is no command 17 (see the class comment).
		return 0;
	}
}

void RSound1::checkRandomAmbianceTrigger() {
	// CONFIRMED: the only code that ever arms
	// this (setting _randomAmbianceTriggerFlag to 0xFF) is itself
	// unreachable/dead code, so in practice this check always fails and
	// command16() never fires via this path - only command37's clear
	// (setting it to 0) is live. Kept faithful to the original regardless.
	if (_randomAmbianceTriggerFlag != 0xFF)
		return;
	_randomAmbianceTriggerFlag = 0;
	command16();
}

int RSound1::command16() {
	// Matches rsound_command16. CONFIRMED: "ds:4982h" is
	// rsound_channel1._activeCount, not a loadData()-relative data byte -
	// i.e. this gate is "only bother checking whether one of the five
	// known pieces is already playing if channel 1 is actually busy;
	// if it's idle, nothing could conflict, so skip straight to picking."
	if (_channels[0]._activeCount) {
		if (isSoundActive(loadData(0x2172))) return 0;
		if (isSoundActive(loadData(0x2448))) return 0;
		if (isSoundActive(loadData(0x3826))) return 0;
		if (isSoundActive(loadData(0x2780))) return 0;
		if (isSoundActive(loadData(0x2E26))) return 0;
	}

	int idx;
	do {
		idx = getRandomNumber() & 7;
	} while (idx > 4 || idx == _lastRandomAmbianceIndex);
	_lastRandomAmbianceIndex = idx;

	typedef int (RSound1:: *AmbiancePtr)();
	static const AmbiancePtr targets[5] = {
		&RSound1::sound1, &RSound1::sound2, &RSound1::sound3,
		&RSound1::sound4, &RSound1::sound5
	};
	return (this->*targets[idx])();
}

int RSound1::command4() {
	// Confirmed: rsound_command4 is gated by isSoundActive(0x3D98) -
	// which is also command39's own target sound - before falling
	// through to the shared reset-channels-4-9 + full GM-reset tail.
	if (isSoundActive(loadData(0x3D98)))
		return 0;
	resetAndGmResetUpperChannels();
	return 0;
}

int RSound1::command5() {
	// Confirmed: same isSoundActive(0x3D98) gate as command4() above.
	if (isSoundActive(loadData(0x3D98)))
		return 0;
	enableUpperChannels();
	return 0;
}

int RSound1::command24() {
	playSound(0x4596);
	playSound(0x45AA);
	return 0;
}

int RSound1::command25() {
	playSound(0x45BC);
	playSound(0x45D0);
	return 0;
}

int RSound1::command26() {
	playSound(0x45E2);
	return 0;
}

int RSound1::command27() {
	playSound(0x45EC);
	return 0;
}

int RSound1::command32() {
	byte *pData = loadData(0x2044);
	if (!isSoundActive(pData)) {
		command1();
		playSoundChannels1To5(0x2044);
		playSoundChannels1To5(0x206B);
		playSoundChannels1To5(0x208B);
		playSoundChannels1To5(0x20AB);
		playSoundChannels1To5(0x20C9);
		playSoundChannels1To5(0x20E3);
	}
	return 0;
}

int RSound1::command33() {
	byte *pData = loadData(0x530);
	if (!isSoundActive(pData)) {
		command1();
		playSoundChannels1To5(0x530);
		playSoundChannels1To5(0x766);
		playSoundChannels1To5(0x929);
		playSoundChannels1To5(0xB37);
		playSoundChannels1To5(0xCBA);
		_channels[5].load(loadData(0xE20));
	}
	return 0;
}

int RSound1::command34() {
	byte *pData = loadData(0x1014);
	if (!isSoundActive(pData)) {
		command1();
		playSoundChannels1To5(0x1014);
		playSoundChannels1To5(0x126B);
		playSoundChannels1To5(0x137C);
		playSoundChannels1To5(0x149B);
		playSoundChannels1To5(0x158C);
		playSoundChannels1To5(0x164E);
	}
	return 0;
}

int RSound1::command35() {
	// isSoundActive(0x1BD4) is checked twice in a row in the disassembly
	// (a genuine, harmless duplicate - both checks are on the same data,
	// so the second is always redundant with the first) - preserved
	// exactly rather than collapsed to a single check.
	byte *pData = loadData(0x1BD4);
	if (!isSoundActive(pData) && !isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1D46));
		_channels[2].load(loadData(0x1DDA));
		_channels[3].load(loadData(0x1E91));
		_channels[4].load(loadData(0x1F1B));
	}
	return 0;
}

int RSound1::command36() {
	byte *pData = loadData(0x1734);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1882));
		_channels[2].load(loadData(0x1949));
		_channels[3].load(loadData(0x1A08));
		_channels[4].load(loadData(0x1A9E));
	}
	return 0;
}

int RSound1::command38() {
	// Also independently reachable via command16's random picker.
	byte *pData = loadData(0x2172);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x2268));
		_channels[2].load(loadData(0x22E1));
		_channels[3].load(loadData(0x239A));
	}
	return 0;
}

int RSound1::command39() {
	byte *pData = loadData(0x3D98);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x3D98);
		playSoundAny(0x3F39);
		playSoundAny(0x404D);
		playSoundAny(0x413F);
		playSoundAny(0x4247);
		playSoundAny(0x43FD);
	}
	return 0;
}

int RSound1::sound1() {
	// Not a real disassembly function - index 0 of funcs_122AD dispatches
	// straight to command38 (see the class comment).
	return command38();
}

int RSound1::sound2() {
	byte *pData = loadData(0x2448);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x25B8));
		_channels[2].load(loadData(0x26C9));
		_channels[3].load(loadData(0x2772));
	}
	return 0;
}

int RSound1::sound3() {
	byte *pData = loadData(0x3826);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x3AAA));
		_channels[2].load(loadData(0x3CE6));
		_channels[3].load(loadData(0x3CD7));
	}
	return 0;
}

int RSound1::sound4() {
	byte *pData = loadData(0x2780);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x2972));
		_channels[2].load(loadData(0x2AF5));
		_channels[3].load(loadData(0x2CBA));
	}
	return 0;
}

int RSound1::sound5() {
	byte *pData = loadData(0x35D8);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x36A8));
		_channels[2].load(loadData(0x3757));
		_channels[3].load(loadData(0x3814));
	}
	return 0;
}

int RSound1::command37() {
	// The disassembly's first "call isSoundActive" has no preceding
	// "lea cx" - unlike every other command, which always reloads cx
	// immediately before an isSoundActive check. The master dispatcher
	// (rsound_command) never touches cx before invoking any command, so
	// cx here holds whatever the game engine's own call into command()
	// left it as - genuinely external state with no reproducible value,
	// not something this port can derive from the disassembly. Treating
	// this check as inert (unpredictable external data will essentially
	// never match a real loaded Channel._soundData) rather than
	// inventing a target offset for it, so command1() and the loads
	// below run unconditionally.
	_randomAmbianceTriggerFlag = 0;
	command1();
	_randomAmbianceTriggerFlag = 0;
	_channels[2].load(loadData(0x3CF6));
	_channels[3].load(loadData(0x3D3E));
	_channels[4].load(loadData(0x3D75));
	return 0;
}

int RSound1::command64() {
	playSound(0x460B);
	return 0;
}

int RSound1::command65() {
	playSound(0x461D);
	playSound(0x4631);
	return 0;
}

int RSound1::command66() {
	playSound(0x4647);
	return 0;
}

int RSound1::command67() {
	playSound(0x465D);
	_channels[8].load(loadData(0x466E));
	return 0;
}

int RSound1::command68() {
	playSound(0x4725);
	return 0;
}

int RSound1::command69() {
	playSound(0x4676);
	playSound(0x4693);
	return 0;
}

int RSound1::command70() {
	playSound(0x46AC);
	playSound(0x46B8);
	playSound(0x46C4);
	playSound(0x47B7);
	return 0;
}

int RSound1::command71() {
	playSound(0x470B);
	return 0;
}

int RSound1::command72() {
	playSound(0x4747);
	return 0;
}

int RSound1::command73() {
	playSound(0x4761);
	return 0;
}

int RSound1::command74() {
	playSound(0x47A7);
	playSound(0x47B7);
	return 0;
}

int RSound1::command75() {
	// Plays the same offset twice - a genuine quirk, preserved exactly.
	playSound(0x4783);
	playSound(0x4783);
	return 0;
}

int RSound1::command76() {
	playSound(0x46D0);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound2::CommandPtr RSound2::_commandList[74] = {
	&RSound2::command0, &RSound2::command1, &RSound2::command2, &RSound2::command3,
	&RSound2::command4, &RSound2::command5, &RSound2::command6, &RSound2::command7,
	&RSound2::command8, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::command16, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::command24, &RSound2::command25, &RSound2::command26, &RSound2::command27,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::command32, &RSound2::command33, &RSound2::command34, &RSound2::command35,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::command64, &RSound2::command65, &RSound2::command66, &RSound2::command67,
	&RSound2::command68, &RSound2::command69, &RSound2::command70, &RSound2::command71,
	&RSound2::command72, &RSound2::nullCommand
};

RSound2::RSound2(Audio::Mixer *mixer) : RSound(mixer, "rsound.ph2", 0x2B40, 0x1C40, 0xDC) {
}

int RSound2::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound2::command4() {
	// Confirmed: same isSoundActive(0x3D98)-gated shape as RSound1's
	// command4(), though this driver has no equivalent to RSound1's
	// command39 (its own bucket structure stops at 35, then jumps to
	// 64+), so this offset can't be "reused ambient-piece target" here -
	// just a coincidentally identical offset NUMBER, not assumed to hold
	// the same data.
	if (isSoundActive(loadData(0x3D98)))
		return 0;
	resetAndGmResetUpperChannels();
	return 0;
}

int RSound2::command5() {
	// Confirmed: same isSoundActive(0x3D98) gate as command4() above.
	if (isSoundActive(loadData(0x3D98)))
		return 0;
	enableUpperChannels();
	return 0;
}

int RSound2::command16() {
	// Unlike RSound1's command16 (a random-ambiance picker), this is a
	// plain gated 3-channel load - confirms command16 is genuinely
	// driver-specific, not shared behavior.
	byte *pData = loadData(0x1188);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x11E1));
		_channels[2].load(loadData(0x1246));
	}
	return 0;
}

int RSound2::command24() {
	playSound(0x16AC);
	playSound(0x16C0);
	return 0;
}

int RSound2::command25() {
	playSound(0x16D2);
	playSound(0x16E6);
	return 0;
}

int RSound2::command26() {
	playSound(0x16F8);
	return 0;
}

int RSound2::command27() {
	playSound(0x1702);
	return 0;
}

int RSound2::command32() {
	// No isSoundActive gate at all - unlike every other multi-load
	// command seen so far.
	command1();
	playSoundChannels1To5(0x129E);
	playSoundChannels1To5(0x136C);
	playSoundChannels1To5(0x14D1);
	return 0;
}

int RSound2::command33() {
	byte *pData = loadData(0x52E);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x52E);
		playSoundAny(0x561);
		playSoundAny(0x56F);
		playSoundAny(0x57D);
		playSoundAny(0x58B);
		playSoundAny(0x597);
	}
	return 0;
}

int RSound2::command34() {
	byte *pData = loadData(0x5A6);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x5A6);
		playSoundAny(0x776);
		playSoundAny(0xA3B);
		playSoundAny(0xBC5);
		playSoundAny(0xD87);
		playSoundAny(0x1086);
		playSoundAny(0x112E);
	}
	return 0;
}

int RSound2::command35() {
	byte *pData = loadData(0x157C);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x157C);
		playSoundAny(0x15EA);
		playSoundAny(0x1615);
		playSoundAny(0x1640);
	}
	return 0;
}

int RSound2::command64() {
	_channels[8].load(loadData(0x17B6));
	return 0;
}

int RSound2::command65() {
	_channels[8].load(loadData(0x17AE));
	return 0;
}

int RSound2::command66() {
	playSound(0x17A0);
	return 0;
}

int RSound2::command67() {
	playSound(0x1732);
	playSound(0x1748);
	playSound(0x175E);
	playSound(0x1774);
	playSound(0x178A);
	return 0;
}

int RSound2::command68() {
	// Same 4 offsets called twice each - a genuine quirk, preserved
	// exactly rather than collapsed to 4 calls.
	playSoundAny(0x17C2);
	playSoundAny(0x17F2);
	playSoundAny(0x1828);
	playSoundAny(0x185E);
	playSoundAny(0x17C2);
	playSoundAny(0x17F2);
	playSoundAny(0x1828);
	playSoundAny(0x185E);
	return 0;
}

int RSound2::command69() {
	_channels[8].load(loadData(0x1721));
	return 0;
}

int RSound2::command70() {
	playSound(0x1894);
	playSound(0x18B1);
	return 0;
}

int RSound2::command71() {
	playSound(0x18CA);
	return 0;
}

int RSound2::command72() {
	playSound(0x168A);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound3::CommandPtr RSound3::_commandList[77] = {
	&RSound3::command0, &RSound3::command1, &RSound3::command2,
	// Index 3 is a plain "jmp rsound_command1" in the disassembly - the
	// dispatch slot for command index 3 aliases directly to command1's
	// FULL handler (lower channels 1-4,9 AND upper channels 5-8), not
	// to the "inner" lower-only logic that the base class's own
	// command3() implements. Mapping it to &RSound3::command1 (rather
	// than &RSound3::command3) preserves that exactly.
	&RSound3::command1,
	&RSound3::command4, &RSound3::command5, &RSound3::command6, &RSound3::command7,
	&RSound3::command8, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::command16, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::command24, &RSound3::command25, &RSound3::command26, &RSound3::command27,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::command32, &RSound3::command33, &RSound3::command34, &RSound3::command35,
	&RSound3::command36, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::command64, &RSound3::command65, &RSound3::command66, &RSound3::command67,
	&RSound3::command68, &RSound3::command69, &RSound3::command70, &RSound3::command71,
	&RSound3::command72, &RSound3::command73, &RSound3::command74, &RSound3::command75,
	&RSound3::nullCommand
};

RSound3::RSound3(Audio::Mixer *mixer) : RSound(mixer, "rsound.ph3", 0x2BA0, 0x37A0, 0xEC) {
}

int RSound3::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound3::command4() {
	// Confirmed: TWO chained isSoundActive() gates (unlike RSound1/
	// RSound2's single 0x3D98 gate), then a tail that
	// resets only channels 5-9 (indices 4-8) - narrower than the
	// shared resetChannels4to9()'s 4-9 (indices 3-8) - so it can't
	// reuse resetAndGmResetUpperChannels() and calls resetChannelRange()
	// directly instead.
	if (isSoundActive(loadData(0x2AA6)))
		return 0;
	if (isSoundActive(loadData(0x1E30)))
		return 0;

	_isDisabled = true;
	resetChannelRange(4, 8);
	_isDisabled = false;
	sendGmReset(RSOUND_CHANNEL_COUNT);
	return 0;
}

int RSound3::command5() {
	// Same two-gate shape as command4() above, but the plain
	// enableUpperChannels() tail (shared with command1()).
	if (isSoundActive(loadData(0x2AA6)))
		return 0;
	if (isSoundActive(loadData(0x1E30)))
		return 0;
	enableUpperChannels();
	return 0;
}

int RSound3::command16() {
	byte *pData = loadData(0x1BFA);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1CAF));
		_channels[2].load(loadData(0x1D24));
		_channels[3].load(loadData(0x1D8B));
	}
	return 0;
}

int RSound3::command24() {
	playSoundChannels5To8(0x32E9);
	playSoundChannels5To8(0x32FD);
	return 0;
}

int RSound3::command25() {
	playSoundChannels5To8(0x330F);
	playSoundChannels5To8(0x3323);
	return 0;
}

int RSound3::command26() {
	playSoundChannels5To8(0x3335);
	return 0;
}

int RSound3::command27() {
	playSoundChannels5To8(0x333F);
	return 0;
}

int RSound3::command32() {
	byte *pData = loadData(0x14F2);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x14F2);
		playSoundAny(0x154A);
		playSoundAny(0x1589);
		playSoundAny(0x15C8);
		playSoundAny(0x1638);
		playSoundAny(0x16BF);
		playSoundAny(0x1704);
	}
	return 0;
}

int RSound3::command33() {
	byte *pData = loadData(0x1E30);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x1E30);
		playSoundAny(0x2094);
		playSoundAny(0x2326);
		playSoundAny(0x2432);
		playSoundAny(0x2632);
		playSoundAny(0x28FF);
	}
	return 0;
}

int RSound3::command34() {
	// No isSoundActive gate at all - unconditional, unlike 32/33/35/36.
	command1();
	playSoundAny(0x6AE);
	playSoundAny(0x95F);
	playSoundAny(0xA67);
	playSoundAny(0xC49);
	playSoundAny(0xE43);
	playSoundAny(0x10AA);
	playSoundAny(0x11E0);
	playSoundAny(0x138C);
	return 0;
}

int RSound3::command35() {
	byte *pData = loadData(0x2AA6);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x2AA6);
		playSoundAny(0x2C43);
		playSoundAny(0x2D53);
		playSoundAny(0x2E43);
		playSoundAny(0x2F4B);
		playSoundAny(0x30FD);
	}
	return 0;
}

int RSound3::command36() {
	byte *pData = loadData(0x1778);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x1778);
		playSoundAny(0x1832);
		playSoundAny(0x18EC);
		playSoundAny(0x1B9D);
	}
	return 0;
}

int RSound3::command64() {
	playSoundChannels5To8(0x335E);
	return 0;
}

int RSound3::command65() {
	playSoundChannels5To8(0x337E);
	playSoundChannels5To8(0x3391);
	return 0;
}

int RSound3::command66() {
	// 0x33AE is called three times in a row - a genuine quirk, preserved
	// exactly rather than collapsed to a single call.
	playSoundChannels5To8(0x33A4);
	playSoundChannels5To8(0x33AE);
	playSoundChannels5To8(0x33AE);
	playSoundChannels5To8(0x33AE);
	return 0;
}

int RSound3::command67() {
	playSoundChannels5To8(0x33BC);
	return 0;
}

int RSound3::command68() {
	playSoundChannels5To8(0x33C9);
	return 0;
}

int RSound3::command69() {
	playSoundChannels5To8(0x33DA);
	return 0;
}

int RSound3::command70() {
	playSoundChannels5To8(0x32CF);
	return 0;
}

int RSound3::command71() {
	playSoundChannels5To8(0x3294);
	return 0;
}

int RSound3::command72() {
	playSoundChannels5To8(0x3406);
	return 0;
}

int RSound3::command73() {
	playSoundChannels5To8(0x3414);
	return 0;
}

int RSound3::command74() {
	_channels[8].load(loadData(0x3429));
	return 0;
}

int RSound3::command75() {
	playSoundChannels5To8(0x33EB);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound4::CommandPtr RSound4::_commandList[72] = {
	&RSound4::command0, &RSound4::command1, &RSound4::command2, &RSound4::command3,
	&RSound4::command4, &RSound4::command5, &RSound4::command6, &RSound4::command7,
	&RSound4::command8, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::command16, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::command24, &RSound4::command25, &RSound4::command26, &RSound4::command27,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::command64, &RSound4::command65, &RSound4::command66, &RSound4::command67,
	&RSound4::command68, &RSound4::command69, &RSound4::command70, &RSound4::nullCommand
};

RSound4::RSound4(Audio::Mixer *mixer) : RSound(mixer, "rsound.ph4", 0x2A40, 0xE00, 0xEC) {
}

bool RSound4::callFunction(uint16 targetOffset) {
	if (targetOffset != 0x229c)
		return false;

	const uint tableOffset = 0x0984 + (getRandomNumber() & 0x0f) * 2;
	const byte *source = loadData(tableOffset);
	byte *destination = loadData(0x0666);
	destination[0] = source[0];
	destination[1] = source[1];
	return true;
}

int RSound4::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound4::command4() {
	// Confirmed: NO isSoundActive() gate in this driver, unlike every
	// other driver seen so far - unconditionally resets channels 5-8,
	// then calls sendGmReset(9).
	_isDisabled = true;
	resetChannelRange(5, 8);
	_isDisabled = false;
	sendGmReset(RSOUND_CHANNEL_COUNT);
	return 0;
}

int RSound4::command5() {
	// Confirmed: _fadeCheckPeriod = 1 + channels 5-8 -
	// identical to the shared enableUpperChannels(), no gate at all.
	enableUpperChannels();
	return 0;
}

int RSound4::command16() {
	// The disassembly's "isSoundPlaying" helper is confirmed functionally identical to
	// RSound::isSoundActive() - same channel 1-8 scan, same
	// _activeCount && _soundData==pData test, same pop-return-address
	// early-exit trick.
	if (!isSoundActive(loadData(0x8F0))) {
		command1();
		_channels[0].load(loadData(0x63A));
		_channels[1].load(loadData(0x8F0));
		_channels[2].load(loadData(0x916));
		_channels[3].load(loadData(0x93C));
		_channels[4].load(loadData(0x962));
	}
	return 0;
}

int RSound4::command24() {
	playSound(0x9C6);
	playSound(0x9D6);
	return 0;
}

int RSound4::command25() {
	playSound(0x9EC);
	playSound(0xA00);
	return 0;
}

int RSound4::command26() {
	playSound(0xA12);
	return 0;
}

int RSound4::command27() {
	playSound(0xA1C);
	return 0;
}

int RSound4::command64() {
	playSound(0xA38);
	return 0;
}

int RSound4::command65() {
	playSound(0xA4E);
	return 0;
}

int RSound4::command66() {
	playSound(0xA60);
	return 0;
}

int RSound4::command67() {
	playSound(0xA76);
	return 0;
}

int RSound4::command68() {
	playSound(0xA80);
	return 0;
}

int RSound4::command69() {
	playSound(0xA8C);
	return 0;
}

int RSound4::command70() {
	playSound(0x9A4);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound5::CommandPtr RSound5::_commandList[80] = {
	&RSound5::command0, &RSound5::command1, &RSound5::command2, &RSound5::command3,
	&RSound5::command4, &RSound5::command5, &RSound5::command6, &RSound5::command7,
	&RSound5::command8, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::command16, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::command24, &RSound5::command25, &RSound5::command26, &RSound5::command27,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::command33, &RSound5::command34, &RSound5::command35,
	&RSound5::command36, &RSound5::command37, &RSound5::command38, &RSound5::command39,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::command64, &RSound5::command65, &RSound5::command66, &RSound5::command67,
	&RSound5::command68, &RSound5::command69, &RSound5::command70, &RSound5::command71,
	&RSound5::command72, &RSound5::command73, &RSound5::command74, &RSound5::command75,
	&RSound5::command76, &RSound5::command77, &RSound5::command78, &RSound5::nullCommand
};

RSound5::RSound5(Audio::Mixer *mixer) : RSound(mixer, "rsound.ph5", 0x2BD0, 0x4870, 0xEC) {
}

int RSound5::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound5::command3() {
	// Confirmed: enables channels 1-5 AND 9 (six channels) - channel 5
	// has moved into the "lower" group here, unlike every prior driver's
	// command3() (channels 1-4,9 only, matching RSound::command3()).
	_fadeCheckPeriod = 1;
	_channels[0].enable(0xFF);
	_channels[1].enable(0xFF);
	_channels[2].enable(0xFF);
	_channels[3].enable(0xFF);
	_channels[4].enable(0xFF);
	_channels[8].enable(0xFF);
	return 0;
}

void RSound5::enableChannels678() {
	_fadeCheckPeriod = 1;
	_channels[5].enable(0xFF);
	_channels[6].enable(0xFF);
	_channels[7].enable(0xFF);
}

int RSound5::command1() {
	// Must call THIS driver's own command3() (see class comment - not
	// virtual in the base, so RSound::command1() would otherwise call
	// the base's command3() even with this override present).
	command3();
	enableChannels678();
	return 0;
}

int RSound5::command4() {
	// No isSoundActive() gate, matching RSound4's command4() shape.
	_isDisabled = true;
	resetChannelRange(5, 8);
	_isDisabled = false;
	sendGmReset(RSOUND_CHANNEL_COUNT);
	return 0;
}

int RSound5::command5() {
	enableChannels678();
	return 0;
}

int RSound5::command16() {
	byte *pData = loadData(0xBBE);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0xC60));
		_channels[2].load(loadData(0xD04));
		_channels[3].load(loadData(0xD97));
		_channels[4].load(loadData(0xDDA));
	}
	return 0;
}

int RSound5::command24() {
	playSound(0x437C);
	playSound(0x4390);
	return 0;
}

int RSound5::command25() {
	playSound(0x43A2);
	playSound(0x43B6);
	return 0;
}

int RSound5::command26() {
	playSound(0x43C8);
	return 0;
}

int RSound5::command27() {
	playSound(0x43D2);
	return 0;
}

int RSound5::command33() {
	byte *pData = loadData(0xE66);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x110E));
		_channels[2].load(loadData(0x13A4));
		_channels[3].load(loadData(0x14B4));
		_channels[4].load(loadData(0x16E4));
		_channels[5].load(loadData(0x19CB));
	}
	return 0;
}

int RSound5::command34() {
	byte *pData = loadData(0x3190);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x3330));
		_channels[2].load(loadData(0x34BA));
		_channels[3].load(loadData(0x34E8));
		_channels[4].load(loadData(0x3518));
		_channels[5].load(loadData(0x3546));
		_channels[6].load(loadData(0x3574));
		_channels[7].load(loadData(0x35A6));
	}
	return 0;
}

int RSound5::command35() {
	byte *pData = loadData(0x1B9A);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x218B));
		_channels[2].load(loadData(0x271D));
	}
	return 0;
}

int RSound5::command36() {
	byte *pData = loadData(0x2D04);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x2E43));
		_channels[2].load(loadData(0x2F73));
	}
	return 0;
}

int RSound5::command37() {
	byte *pData = loadData(0x35D8);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x3884));
		_channels[2].load(loadData(0x39A6));
		_channels[3].load(loadData(0x3BC9));
	}
	return 0;
}

int RSound5::command38() {
	byte *pData = loadData(0x73A);
	if (!isSoundActive(pData)) {
		command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x87A));
		_channels[2].load(loadData(0x938));
		_channels[3].load(loadData(0x9F9));
		_channels[4].load(loadData(0xA8A));
	}
	return 0;
}

int RSound5::command39() {
	// Gate leads to command3() (the lower-group enable), not command1() -
	// see class comment. Only playSoundChannels1To5() is used here, never
	// the upper 3 channels.
	byte *pData = loadData(0x3C5C);
	if (!isSoundActive(pData)) {
		command3();
		playSoundChannels1To5(0x3C5C);
		playSoundChannels1To5(0x3EB3);
		playSoundChannels1To5(0x3FC4);
		playSoundChannels1To5(0x40E3);
		playSoundChannels1To5(0x41D4);
		playSoundChannels1To5(0x4296);
	}
	return 0;
}

int RSound5::command64() {
	playSound(0x447B);
	return 0;
}

int RSound5::command65() {
	playSound(0x43EE);
	return 0;
}

int RSound5::command66() {
	playSound(0x43FC);
	return 0;
}

int RSound5::command67() {
	_channels[6].load(loadData(0x4422));
	return 0;
}

int RSound5::command68() {
	playSound(0x4406);
	return 0;
}

int RSound5::command69() {
	playSound(0x442C);
	return 0;
}

int RSound5::command70() {
	playSound(0x4418);
	return 0;
}

int RSound5::command71() {
	playSound(0x444A);
	playSound(0x4459);
	return 0;
}

int RSound5::command72() {
	playSound(0x4467);
	return 0;
}

int RSound5::command73() {
	_channels[6].load(loadData(0x4471));
	return 0;
}

int RSound5::command74() {
	playSound(0x4488);
	return 0;
}

int RSound5::command75() {
	playSound(0x449D);
	return 0;
}

int RSound5::command76() {
	playSound(0x44B9);
	return 0;
}

int RSound5::command77() {
	playSound(0x44D1);
	return 0;
}

int RSound5::command78() {
	playSound(0x44EB);
	return 0;
}

/*-----------------------------------------------------------------------*/

const RSound9::CommandPtr RSound9::_commandList[73] = {
	&RSound9::command0, &RSound9::command1, &RSound9::command2, &RSound9::command3,
	&RSound9::command4, &RSound9::command5, &RSound9::command6, &RSound9::command7,
	&RSound9::command8, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::command16Or24, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::command16Or24, &RSound9::command25, &RSound9::command26, &RSound9::command27,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::command32, &RSound9::nullCommand, &RSound9::command34, &RSound9::command35,
	&RSound9::command36, &RSound9::command37, &RSound9::command38, &RSound9::command39,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::command64, &RSound9::command65Or66, &RSound9::command65Or66, &RSound9::command67,
	&RSound9::command68, &RSound9::command69, &RSound9::command70, &RSound9::command71,
	&RSound9::nullCommand
};

RSound9::RSound9(Audio::Mixer *mixer) : RSound(mixer, "rsound.ph9", 0x2B70, 0x2D40, 0xEC) {
}

int RSound9::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int RSound9::command4() {
	// Resets channels 7,8,9 (0-based indices 6-8) - a fourth
	// distinct range for this reset tail (RSound1/2: 4-9, RSound3: 5-9,
	// RSound4/5: 6-9). No isSoundActive() gate, matching RSound4/5.
	_isDisabled = true;
	resetChannelRange(6, 8);
	_isDisabled = false;
	sendGmReset(RSOUND_CHANNEL_COUNT);
	return 0;
}

int RSound9::command5() {
	// Confirmed to match the shared enableUpperChannels() exactly
	// (channels 5-8, armFadeCheck).
	enableUpperChannels();
	return 0;
}

int RSound9::command16Or24() {
	playSoundChannels5To8(0x295A);
	return 0;
}

int RSound9::command25() {
	playSoundChannels5To8(0x295C);
	return 0;
}

int RSound9::command26() {
	playSoundChannels5To8(0x295E);
	return 0;
}

int RSound9::command27() {
	playSoundChannels5To8(0x2960);
	return 0;
}

int RSound9::command32() {
	// No isSoundActive gate at all - unconditional.
	command1();
	playSoundAny(0x628);
	playSoundAny(0x682);
	playSoundAny(0x6CA);
	playSoundAny(0x99C);
	return 0;
}

int RSound9::command34() {
	// True dispatch index 34 - this is the disassembly's
	// "rsound_command33" body (see class comment on the naming shift).
	command1();
	playSoundAny(0x9A8);
	playSoundAny(0x9E4);
	playSoundAny(0xA53);
	playSoundAny(0xA96);
	playSoundAny(0xAFB);
	playSoundAny(0xB34);
	return 0;
}

int RSound9::command35() {
	// True dispatch index 35 - disassembly's "rsound_command34" body.
	command1();
	playSoundAny(0x1008);
	playSoundAny(0x103C);
	playSoundAny(0x105D);
	playSoundAny(0x107E);
	playSoundAny(0x10D6);
	playSoundAny(0x1131);
	playSoundAny(0x116C);
	return 0;
}

int RSound9::command36() {
	// True dispatch index 36 - disassembly's "rsound_command35" body.
	command1();
	playSoundAny(0x1198);
	playSoundAny(0x11C9);
	playSoundAny(0x11F5);
	playSoundAny(0x1229);
	playSoundAny(0x125D);
	playSoundAny(0x1283);
	return 0;
}

int RSound9::command37() {
	// True dispatch index 37 - disassembly's "rsound_command36" body.
	command1();
	playSoundAny(0x12B8);
	playSoundAny(0x1306);
	playSoundAny(0x1408);
	playSoundAny(0x1486);
	playSoundAny(0x14E0);
	playSoundAny(0x1733);
	playSoundAny(0x1A44);
	return 0;
}

int RSound9::command38() {
	// True dispatch index 38 - disassembly's "rsound_command37" body.
	command1();
	playSoundAny(0x1B30);
	playSoundAny(0x1DE1);
	playSoundAny(0x1EE9);
	playSoundAny(0x20CB);
	playSoundAny(0x22C5);
	playSoundAny(0x250F);
	playSoundAny(0x2645);
	playSoundAny(0x27F1);
	return 0;
}

int RSound9::command39() {
	// True dispatch index 39 - disassembly's "rsound_command38" body.
	command1();
	playSoundChannels1To6(0xB76);
	playSoundChannels1To6(0xCB4);
	playSoundChannels1To6(0xD7B);
	playSoundChannels1To6(0xE3E);
	playSoundChannels1To6(0xED2);
	return 0;
}

int RSound9::command64() {
	_channels[7].load(loadData(0x296A));
	return 0;
}

int RSound9::command65Or66() {
	_channels[8].load(loadData(0x2962));
	return 0;
}

int RSound9::command67() {
	// 0x298A is called three times in a row - a genuine quirk, preserved
	// exactly rather than collapsed to a single call.
	playSoundChannels5To8(0x2980);
	playSoundChannels5To8(0x298A);
	playSoundChannels5To8(0x298A);
	playSoundChannels5To8(0x298A);
	return 0;
}

int RSound9::command68() {
	_channels[7].load(loadData(0x29C4));
	return 0;
}

int RSound9::command69() {
	_channels[8].load(loadData(0x29B3));
	return 0;
}

int RSound9::command70() {
	// Shares a fallthrough tail with command71: writes a variant byte
	// into the sound data at 0x299B
	// before playing 0x2998 - twice, matching the disassembly's call +
	// tail-jmp pair exactly.
	*loadData(0x299B) = 80;
	playSoundChannels5To8(0x2998);
	playSoundChannels5To8(0x2998);
	return 0;
}

int RSound9::command71() {
	*loadData(0x299B) = 99;
	playSoundChannels5To8(0x2998);
	playSoundChannels5To8(0x2998);
	return 0;
}

namespace {

enum {
	kDemoFileSize = 35413,
	kDemoDataOffset = 0x2cd0,
	kDemoInitializedDataSize = 0x5d85,
	kDemoDeclaredDataSize = 0x5f60,
	kDemoSysExOffset = 0x00e2
};

const char *const kDemoFilename = "rsound.pha";
const char *const kDemoFirst8192Md5 = "8fca9087bfe5897dd8079d0513eed377";

} // namespace

RSoundDemo::RSoundDemo(Audio::Mixer *mixer, const Common::Path &filename,
					   int dataOffset, int dataSize, int sysExOffset) :
		RSound(mixer, filename, dataOffset, dataSize, sysExOffset) {
}

int RSoundDemo::dispatchCommonCommand(int commandId) {
	switch (commandId) {
	case 0: return command0();
	case 1: return command1();
	case 2: return command2();
	case 3: return command3();
	case 4: return command4();
	case 5: return command5();
	case 6: return command6();
	case 7: return command7();
	case 8: return command8();
	default: return 0;
	}
}

int RSoundDemo::command4() {
	// PHA's command 4 has no retail-style sound-active guard.
	resetAndGmResetUpperChannels();
	return 0;
}

int RSoundDemo::command5() {
	// PHA's command 5 enters the shared upper-channel enable tail directly.
	enableUpperChannels();
	return 0;
}

RSoundDemoPHA::RSoundDemoPHA(Audio::Mixer *mixer) :
		RSoundDemo(mixer, kDemoFilename, kDemoDataOffset,
				   kDemoDeclaredDataSize, kDemoSysExOffset) {
	// The MZ file omits the zero-initialized tail declared by the overlay
	// descriptor. Supply it explicitly rather than depending on container
	// growth semantics in the common file loader.
	for (uint offset = kDemoInitializedDataSize; offset < kDemoDeclaredDataSize; ++offset)
		_soundData[offset] = 0;
}

bool RSoundDemoPHA::validate(Common::String *reason) {
	Common::File file;
	if (!file.open(kDemoFilename)) {
		if (reason)
			*reason = "file is missing";
		return false;
	}
	if (file.size() != kDemoFileSize) {
		if (reason)
			*reason = "file size does not match";
		return false;
	}
	if (kDemoDataOffset + kDemoInitializedDataSize != file.size() ||
		kDemoInitializedDataSize > kDemoDeclaredDataSize) {
		if (reason)
			*reason = "declared data segment is inconsistent";
		return false;
	}

	file.seek(0);
	if (Common::computeStreamMD5AsString(file, 8192) != kDemoFirst8192Md5) {
		if (reason)
			*reason = "first-8192-byte signature does not match";
		return false;
	}
	return true;
}

int RSoundDemoPHA::command(int commandId, int param) {
	Common::StackLock lock(_driverMutex);
	_commandParam = param;
	_frameCounter = 0;

	if (commandId >= 0 && commandId <= 8)
		return dispatchCommonCommand(commandId);
	if (commandId < 9 || commandId > 27)
		return 0;

	// Each presentation handler calls command 1 and then invokes the native
	// channels-1-to-8 allocator once for every root in its row.
	static const uint16 roots[19][8] = {
		{0x0602, 0x0745, 0x0793, 0x086f, 0x0911, 0x09b3, 0x0a01, 0x0a89},
		{0x01e0, 0x023a, 0x0282, 0x0554},
		{0x0560, 0x0583, 0x05a4, 0x05c0, 0x05e1},
		{0x1c20, 0x1e77, 0x1f88, 0x20a7, 0x2198, 0x225a},
		{0x135c, 0x1494, 0x154e, 0x15fa, 0x1683},
		{0x2f0e, 0x2fc8, 0x3080, 0x3331},
		{0x338c, 0x355c, 0x3821, 0x39ab, 0x3b6d, 0x3e6c, 0x3f14},
		{0x2340, 0x25fd, 0x27f1, 0x2a29, 0x2bb3, 0x2d21},
		{0x3f6e, 0x3fcd, 0x40b0, 0x40f5, 0x4115},
		{0x4136, 0x418c, 0x41f1},
		{0x4248, 0x42cb, 0x434e, 0x43be, 0x4446, 0x44e2, 0x4514},
		{0x455e, 0x45e5, 0x461d, 0x4671},
		{0x46dc, 0x477d, 0x4845, 0x48f7, 0x493b, 0x4983, 0x49a9},
		{0x0aea, 0x0b38, 0x0c38, 0x0cb6, 0x0d0e, 0x0f61, 0x1272},
		{0x17b2, 0x17e6, 0x1807, 0x1828, 0x1880, 0x18db, 0x1916},
		{0x1942, 0x1973, 0x199f, 0x19d3, 0x1a07, 0x1a2d},
		{0x1a62, 0x1a9e, 0x1afd, 0x1b40, 0x1ba5, 0x1bde},
		{0x49d0, 0x4ac2, 0x4b35, 0x4bea},
		{0x4c94, 0x4e00, 0x4f0d, 0x4fb2}
	};

	command1();
	const uint16 *commandRoots = roots[commandId - 9];
	for (uint index = 0; index < ARRAYSIZE(roots[0]) && commandRoots[index]; ++index)
		playSoundAny(commandRoots[index]);
	return 0;
}

void RSoundDemoPHA::writeRandomizedPair(uint16 firstLowOffset,
		uint16 secondLowOffset, uint16 firstHighOffset, uint16 secondHighOffset,
		byte firstLow, byte secondLow, byte firstHigh, byte secondHigh) {
	if ((getRandomNumber() >> 8) <= 0x80) {
		SWAP(firstLow, secondLow);
		SWAP(firstHigh, secondHigh);
	}
	*loadData(firstLowOffset) = firstLow;
	*loadData(secondLowOffset) = secondLow;
	*loadData(firstHighOffset) = firstHigh;
	*loadData(secondHighOffset) = secondHigh;
}

bool RSoundDemoPHA::callFunction(uint16 targetOffset) {
	switch (targetOffset) {
	case 0x240e:
		writeRandomizedPair(0x3fd5, 0x404a, 0x3fd7, 0x404c,
							0x14, 0x03, 0x32, 0x64);
		return true;
	case 0x243b:
		writeRandomizedPair(0x3ffb, 0x4065, 0x3ffd, 0x4067,
							0x0d, 0x0b, 0x4b, 0x5a);
		return true;
	default:
		return false;
	}
}

} // namespace Sound
} // namespace Phantom
} // namespace MADS
