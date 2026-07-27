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

#include "mads/phantom/rsound_phantom.h"

namespace MADS {
namespace Phantom {

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
		// TODO: command 37 not yet implemented - disassembly not yet
		// provided. There is no command 17 (see the class comment).
		return 0;
	}
}

void RSound1::checkRandomAmbianceTrigger() {
	// Matches sub_1222E exactly. CONFIRMED: the only code that ever arms
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
	// through to the base class's reset-channels-6-9 + full GM-reset.
	if (isSoundActive(loadData(0x3D98)))
		return 0;
	return RSound::command4();
}

int RSound1::command5() {
	// Confirmed: same isSoundActive(0x3D98) gate as command4() above.
	if (isSoundActive(loadData(0x3D98)))
		return 0;
	return RSound::command5();
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
		playSoundAny(0x2044);
		playSoundAny(0x206B);
		playSoundAny(0x208B);
		playSoundAny(0x20AB);
		playSoundAny(0x20C9);
		playSoundAny(0x20E3);
	}
	return 0;
}

int RSound1::command33() {
	byte *pData = loadData(0x530);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x530);
		playSoundAny(0x766);
		playSoundAny(0x929);
		playSoundAny(0xB37);
		playSoundAny(0xCBA);
		_channels[5].load(loadData(0xE20));
	}
	return 0;
}

int RSound1::command34() {
	byte *pData = loadData(0x1014);
	if (!isSoundActive(pData)) {
		command1();
		playSoundAny(0x1014);
		playSoundAny(0x126B);
		playSoundAny(0x137C);
		playSoundAny(0x149B);
		playSoundAny(0x158C);
		playSoundAny(0x164E);
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
		playSoundChannels1To8(0x3D98);
		playSoundChannels1To8(0x3F39);
		playSoundChannels1To8(0x404D);
		playSoundChannels1To8(0x413F);
		playSoundChannels1To8(0x4247);
		playSoundChannels1To8(0x43FD);
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

} // namespace Phantom
} // namespace MADS
