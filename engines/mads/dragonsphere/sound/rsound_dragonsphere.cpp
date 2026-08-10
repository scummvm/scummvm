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
#include "mads/dragonsphere/sound/rsound_dragonsphere.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

#define MAKE_CALLBACK(cls, fn) reinterpret_cast<RSound::CallbackFunction>(&cls::fn)

RSound1::RSound1(Audio::Mixer *mixer) : RSound(mixer, "rsound.dr1", 0x2C00, 0x3840, 0x9C) {
}

const RSound1::CommandPtr RSound1::_commandList[102] = {
	&RSound1::command0, &RSound1::command1, &RSound1::command2, &RSound1::command3,
	&RSound1::command4, &RSound1::command5, &RSound1::command6, &RSound1::command7,
	&RSound1::command8, &RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand,
	&RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand,
	&RSound1::command16, &RSound1::command17, &RSound1::command18, &RSound1::nullCommand,
	&RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand,
	&RSound1::command24, &RSound1::command25, &RSound1::command26, &RSound1::command27,
	&RSound1::command28, &RSound1::nullCommand, &RSound1::command30, &RSound1::command31,
	&RSound1::command32, &RSound1::command33, &RSound1::command34, &RSound1::command35,
	&RSound1::command36, &RSound1::command37, &RSound1::command38, &RSound1::command39,
	&RSound1::command40, &RSound1::command41, &RSound1::command42, &RSound1::command43,
	&RSound1::command44, &RSound1::command45, &RSound1::command46, &RSound1::command47,
	&RSound1::command48, &RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand,
	&RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand,
	&RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand,
	&RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand, &RSound1::nullCommand,
	&RSound1::command64, &RSound1::command65, &RSound1::command66, &RSound1::command67,
	&RSound1::command68, &RSound1::command69, &RSound1::command70, &RSound1::command71,
	&RSound1::nullCommand, &RSound1::command73, &RSound1::command74, &RSound1::command75,
	&RSound1::command76, &RSound1::command77, &RSound1::command78, &RSound1::command79,
	&RSound1::command80, &RSound1::command81, &RSound1::command82, &RSound1::command83,
	&RSound1::command84, &RSound1::command85, &RSound1::command86, &RSound1::command87,
	&RSound1::command88, &RSound1::command89, &RSound1::command90, &RSound1::command91,
	&RSound1::nullCommand, &RSound1::command93, &RSound1::command94, &RSound1::command95,
	&RSound1::command96, &RSound1::command97, &RSound1::nullCommand, &RSound1::command99,
	&RSound1::command100, &RSound1::command101
};

int RSound1::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	return (this->*_commandList[commandId])();
}

/*-----------------------------------------------------------------------*/
// Bucket 2 (16-18): background-music dispatcher / direct play / re-entrant
// launcher. Matches the sibling ASound1 driver's identically-purposed
// command16/17/18 trio.

int RSound1::command16() {
	byte *pData = loadData(0x7BA);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x90);
		loadCommand16();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand16));
	return 0;
}

void RSound1::loadCommand16() {
	setMusicIndex(0x10);
	command3();
	_channels[0].load(loadData(0x7BA));
	_channels[1].load(loadData(0x8C3));
	_channels[2].load(loadData(0x939));
	_channels[3].load(loadData(0x9AF));
}

int RSound1::command17() {
	// Ungated scheduling (Pattern A): no isMusicChannelsActive() check,
	// always loads immediately once the isSoundActive() gate passes.
	if (isSoundActive(loadData(0x2F54)))
		return 0;
	command3();
	resetCallbackTimerEx(0xC0, 0x60);
	_channels[0].load(loadData(0x2F54));
	_channels[1].load(loadData(0x2F84));
	_channels[2].load(loadData(0x2FB5));
	_channels[3].load(loadData(0x2FCC));
	return 0;
}

int RSound1::command18() {
	// Matches rsound_command18: command3() then re-dispatch by
	// _musicIndex. The original branches into one of two separate
	// physical dispatch tables (bucket 2 or bucket 4) depending on
	// whether the index is <= 0x12; since this port uses a single flat
	// _commandList[] covering every index, both branches collapse to the
	// same array lookup.
	command3();
	return (this->*_commandList[getMusicIndex()])();
}

/*-----------------------------------------------------------------------*/
// Bucket 3 (24-31): plain SFX, channels 6-8.

int RSound1::command24() {
	playSoundChannels6to8(0x3474);
	playSoundChannels6to8(0x348B);
	return 0;
}

int RSound1::command25() {
	playSoundChannels6to8(0x349F);
	playSoundChannels6to8(0x34B6);
	return 0;
}

int RSound1::command26() {
	playSoundChannels6to8(0x34CA);
	return 0;
}

int RSound1::command27() {
	playSoundChannels6to8(0x34D4);
	return 0;
}

int RSound1::command28() {
	playSoundChannels6to8(0x358D);
	playSoundChannels6to8(0x3597);
	return 0;
}

int RSound1::command30() {
	playSoundChannels6to8(0x34ED);
	return 0;
}

int RSound1::command31() {
	playSoundChannels6to8(0x35DF);
	return 0;
}

/*-----------------------------------------------------------------------*/
// Bucket 4 (32-48): music-piece loaders, mostly gated by isSoundActive()
// then either an immediate load or a deferred one (scheduled once the
// lower/music channel group frees up), matching the sibling ASound1
// driver's Pattern B idiom.

int RSound1::command32() {
	byte *pData = loadData(0xA14);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand32();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand32));
	return 0;
}

void RSound1::loadCommand32() {
	setMusicIndex(0x20);
	command3();
	_channels[0].load(loadData(0xA14));
	_channels[1].load(loadData(0xA5A));
	_channels[2].load(loadData(0xAB9));
	_channels[3].load(loadData(0xAD6));
}

int RSound1::command33() {
	byte *pData = loadData(0xAFC);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand33();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand33));
	return 0;
}

void RSound1::loadCommand33() {
	command3();
	_channels[0].load(loadData(0xAFC));
	_channels[1].load(loadData(0xB9E));
	_channels[2].load(loadData(0xBFD));
	_channels[3].load(loadData(0xC20));
}

int RSound1::command34() {
	// Ungated scheduling (Pattern A), unlike every other bucket-4
	// command: no isMusicChannelsActive() check, and uses
	// playSoundChannels1To5() (free-channel search) rather than direct
	// Channel_loadN targets. Also the only asymmetric timer in this
	// bucket (counter=0x50, period=5).
	if (isSoundActive(loadData(0x25F2)))
		return 0;
	resetCallbackTimerEx(0x50, 5);
	command3();
	playSoundChannels1To5(0x25F2);
	playSoundChannels1To5(0x2660);
	playSoundChannels1To5(0x2676);
	return 0;
}

int RSound1::command35() {
	byte *pData = loadData(0xDDA);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x60);
		loadCommand35();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand35));
	return 0;
}

void RSound1::loadCommand35() {
	command3();
	_channels[0].load(loadData(0xDDA));
	_channels[1].load(loadData(0xE6F));
	_channels[2].load(loadData(0xEF4));
	_channels[3].load(loadData(0xF3F));
}

int RSound1::command36() {
	byte *pData = loadData(0xF66);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x80);
		loadCommand36();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand36));
	return 0;
}

void RSound1::loadCommand36() {
	command3();
	_channels[0].load(loadData(0xF66));
	_channels[1].load(loadData(0x1028));
	_channels[2].load(loadData(0x1113));
	_channels[3].load(loadData(0x11A6));
}

int RSound1::command37() {
	byte *pData = loadData(0x120C);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xC0);
		loadCommand37();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand37));
	return 0;
}

void RSound1::loadCommand37() {
	command3();
	_channels[0].load(loadData(0x120C));
	_channels[1].load(loadData(0x1267));
}

int RSound1::command38() {
	// NOTE: channel-load order is 5,3,4,2 - not sequential. Preserved
	// exactly.
	byte *pData = loadData(0x12CE);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x60);
		loadCommand38();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand38));
	return 0;
}

void RSound1::loadCommand38() {
	command3();
	_channels[4].load(loadData(0x12CE));
	_channels[2].load(loadData(0x1373));
	_channels[3].load(loadData(0x1449));
	_channels[1].load(loadData(0x1534));
}

int RSound1::command39() {
	byte *pData = loadData(0x1622);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand39();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand39));
	return 0;
}

void RSound1::loadCommand39() {
	command3();
	_channels[0].load(loadData(0x1622));
	_channels[1].load(loadData(0x1678));
	_channels[2].load(loadData(0x16D7));
}

int RSound1::command40() {
	byte *pData = loadData(0x172E);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xA8);
		loadCommand40();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand40));
	return 0;
}

void RSound1::loadCommand40() {
	command3();
	_channels[0].load(loadData(0x172E));
	_channels[1].load(loadData(0x189C));
	_channels[2].load(loadData(0x1A47));
	_channels[3].load(loadData(0x1D0A));
	_channels[4].load(loadData(0x1F05));
}

int RSound1::command41() {
	// NOTE: uses channel 9 in place of channel 3 - confirmed directly
	// from the disassembly (Channel_load9, not Channel_load3).
	byte *pData = loadData(0x219E);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x90);
		loadCommand41();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand41));
	return 0;
}

void RSound1::loadCommand41() {
	command3();
	_channels[0].load(loadData(0x219E));
	_channels[1].load(loadData(0x220C));
	_channels[8].load(loadData(0x2283));
	_channels[3].load(loadData(0x22D8));
	_channels[4].load(loadData(0x23B1));
}

int RSound1::command42() {
	byte *pData = loadData(0x2400);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x90);
		loadCommand42();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand42));
	return 0;
}

void RSound1::loadCommand42() {
	setMusicIndex(0x29);
	command3();
	_channels[0].load(loadData(0x2400));
	_channels[1].load(loadData(0x242B));
	_channels[2].load(loadData(0x245C));
	_channels[3].load(loadData(0x2499));
	_channels[4].load(loadData(0x24D0));
}

void RSound1::command43_48Tail(byte variant) {
	// Matches the shared command43/48 tail: writes `variant` into the
	// sound data one byte into the block command2's load targets
	// (offset 0x2539), then gates on isSoundActive(0x24EC) with NO
	// isMusicChannelsActive() branching at all (unlike every other
	// bucket-4 loader) - always proceeds unconditionally once the gate
	// passes.
	*loadData(0x2539) = variant;
	if (isSoundActive(loadData(0x24EC)))
		return;
	loadCommand43_48();
}

int RSound1::command43() {
	command43_48Tail(0x5D);
	return 0;
}

int RSound1::command48() {
	command43_48Tail(0x31);
	return 0;
}

void RSound1::loadCommand43_48() {
	setMusicIndex(0x28);
	command3();
	_channels[0].load(loadData(0x24EC));
	_channels[1].load(loadData(0x2538));
	_channels[2].load(loadData(0x257D));
	_channels[3].load(loadData(0x25A4));
	_channels[4].load(loadData(0x25CD));
}

int RSound1::command44() {
	byte *pData = loadData(0x268C);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		// Asymmetric timer: counter=0x60, period=0xE0.
		resetCallbackTimerEx(0x60, 0xE0);
		loadCommand44();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand44));
	return 0;
}

void RSound1::loadCommand44() {
	command3();
	_channels[0].load(loadData(0x268C));
	_channels[1].load(loadData(0x26D2));
	_channels[2].load(loadData(0x2717));
	_channels[3].load(loadData(0x273F));
	_channels[4].load(loadData(0x2801));
}

int RSound1::command45() {
	byte *pData = loadData(0x28F6);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x60);
		loadCommand45();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand45));
	return 0;
}

void RSound1::loadCommand45() {
	command3();
	_channels[0].load(loadData(0x28F6));
	_channels[1].load(loadData(0x2A60));
	_channels[2].load(loadData(0x2B46));
	_channels[3].load(loadData(0x2C41));
	_channels[4].load(loadData(0x2D2F));
}

int RSound1::command46() {
	byte *pData = loadData(0x2EC6);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x90);
		loadCommand46();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand46));
	return 0;
}

void RSound1::loadCommand46() {
	command3();
	_channels[0].load(loadData(0x2EC6));
	_channels[1].load(loadData(0x2F1B));
}

int RSound1::command47() {
	// NOTE: channel-load order is 5,3,4,2 - not sequential, same shape
	// as command38. Preserved exactly.
	byte *pData = loadData(0x1342);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x60);
		loadCommand47();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound1, loadCommand47));
	return 0;
}

void RSound1::loadCommand47() {
	command3();
	_channels[4].load(loadData(0x1342));
	_channels[2].load(loadData(0x13EC));
	_channels[3].load(loadData(0x1503));
	_channels[1].load(loadData(0x15BE));
}

/*-----------------------------------------------------------------------*/
// Bucket 5 (64-101): plain SFX, channels 6-8 (playSound), except
// command101 which loads channel 9 directly.

int RSound1::command64() {
	playSoundChannels6to8(0x303A);
	playSoundChannels6to8(0x304C);
	playSoundChannels6to8(0x3052);
	return 0;
}

int RSound1::command65() {
	playSoundChannels6to8(0x3077);
	return 0;
}

int RSound1::command66() {
	playSoundChannels6to8(0x3093);
	return 0;
}

int RSound1::command67() {
	playSoundChannels6to8(0x30D9);
	return 0;
}

int RSound1::command68() {
	playSoundChannels6to8(0x3105);
	return 0;
}

int RSound1::command69() {
	playSoundChannels6to8(0x3111);
	return 0;
}

int RSound1::command70() {
	playSoundChannels6to8(0x3133);
	return 0;
}

int RSound1::command71() {
	playSoundChannels6to8(0x3149);
	return 0;
}

int RSound1::command73() {
	playSoundChannels6to8(0x3199);
	return 0;
}

int RSound1::command74() {
	playSoundChannels6to8(0x31CF);
	return 0;
}

int RSound1::command75() {
	playSoundChannels6to8(0x31F3);
	return 0;
}

int RSound1::command76() {
	playSoundChannels6to8(0x320F);
	return 0;
}

int RSound1::command77() {
	playSoundChannels6to8(0x321B);
	playSoundChannels6to8(0x3227);
	playSoundChannels6to8(0x3233);
	return 0;
}

int RSound1::command78() {
	playSoundChannels6to8(0x323F);
	return 0;
}

int RSound1::command79() {
	playSoundChannels6to8(0x3286);
	return 0;
}

int RSound1::command80() {
	playSoundChannels6to8(0x32A7);
	return 0;
}

int RSound1::command81() {
	// 0x32DC is called three times in a row - a genuine quirk, preserved
	// exactly rather than collapsed to a single call.
	playSoundChannels6to8(0x32DC);
	playSoundChannels6to8(0x32DC);
	playSoundChannels6to8(0x32DC);
	return 0;
}

int RSound1::command82() {
	playSoundChannels6to8(0x3303);
	return 0;
}

int RSound1::command83() {
	playSoundChannels6to8(0x3367);
	return 0;
}

int RSound1::command84() {
	playSoundChannels6to8(0x3383);
	return 0;
}

int RSound1::command85() {
	playSoundChannels6to8(0x338D);
	return 0;
}

int RSound1::command86() {
	playSoundChannels6to8(0x33A4);
	return 0;
}

int RSound1::command87() {
	playSoundChannels6to8(0x33C9);
	return 0;
}

int RSound1::command88() {
	playSoundChannels6to8(0x33E3);
	return 0;
}

int RSound1::command89() {
	playSoundChannels6to8(0x33FC);
	return 0;
}

int RSound1::command90() {
	playSoundChannels6to8(0x340E);
	return 0;
}

int RSound1::command91() {
	playSoundChannels6to8(0x343B);
	return 0;
}

int RSound1::command93() {
	playSoundChannels6to8(0x3004);
	playSoundChannels6to8(0x301F);
	return 0;
}

int RSound1::command94() {
	playSoundChannels6to8(0x30B7);
	return 0;
}

int RSound1::command95() {
	playSoundChannels6to8(0x3517);
	return 0;
}

int RSound1::command96() {
	// 0x357C is called twice in a row - a genuine quirk, preserved
	// exactly rather than collapsed to a single call.
	playSoundChannels6to8(0x352F);
	playSoundChannels6to8(0x357C);
	playSoundChannels6to8(0x357C);
	return 0;
}

int RSound1::command97() {
	playSoundChannels6to8(0x3581);
	return 0;
}

int RSound1::command99() {
	playSoundChannels6to8(0x35A1);
	return 0;
}

int RSound1::command100() {
	playSoundChannels6to8(0x35B3);
	playSoundChannels6to8(0x35C0);
	playSoundChannels6to8(0x35CD);
	return 0;
}

int RSound1::command101() {
	_channels[8].load(loadData(0x35D7));
	return 0;
}

/*-----------------------------------------------------------------------*/

RSound2::RSound2(Audio::Mixer *mixer) : RSound(mixer, "rsound.dr2", 0x27B0, 0x1A60, 0xAC) {
}

void RSound2::resetChannels1to6() {
	// Matches resetChannels1to6: zeroes channels 1-6 (0-based 0-5).
	_isDisabled = true;
	resetChannelRange(0, 5);
	resetHeldNotes();
	_isDisabled = false;
}

int RSound2::command2() {
	resetChannels1to6();
	sendGmReset(4);
	return 0;
}

int RSound2::command1() {
	// Must call THIS driver's own command3() (not virtual in the base -
	// see class comment).
	command3();
	command5();
	return 0;
}

int RSound2::command3() {
	// Confirmed: enables channels 1-6 (six channels) - channel 9 is
	// untouched here, unlike the base class's default (1-5,9).
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[0].enable(0xFF);
	_channels[1].enable(0xFF);
	_channels[2].enable(0xFF);
	_channels[3].enable(0xFF);
	_channels[4].enable(0xFF);
	_channels[5].enable(0xFF);
	return 0;
}

int RSound2::command5() {
	// Confirmed: enables channels 7,8 only (two channels) - the base
	// class's default enables 3 (6,7,8).
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[6].enable(0xFF);
	_channels[7].enable(0xFF);
	return 0;
}

/*-----------------------------------------------------------------------*/

int RSound2::command16() {
	byte *pData = loadData(0xEE0);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x90);
		loadCommand16();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound2, loadCommand16));
	return 0;
}

void RSound2::loadCommand16() {
	command3();
	_channels[0].load(loadData(0xEE0));
	_channels[1].load(loadData(0xF2F));
}

int RSound2::command17() {
	// Ungated scheduling (Pattern A), matching RSound1's command17 shape.
	if (isSoundActive(loadData(0x155C)))
		return 0;
	command3();
	resetCallbackTimerEx(192, 96);
	_channels[0].load(loadData(0x155C));
	_channels[1].load(loadData(0x158C));
	_channels[2].load(loadData(0x15BD));
	_channels[3].load(loadData(0x15D4));
	return 0;
}

int RSound2::command18() {
	// See RSound1::command18 - the flat _commandList[] collapses the
	// original's two-table branch into a single array lookup.
	command3();
	return (this->*_commandList[getMusicIndex()])();
}

int RSound2::command24() {
	playSoundChannels7to8(0x160C);
	playSoundChannels7to8(0x1620);
	return 0;
}

int RSound2::command25() {
	playSoundChannels7to8(0x1632);
	playSoundChannels7to8(0x1646);
	return 0;
}

int RSound2::command26() {
	playSoundChannels7to8(0x1658);
	return 0;
}

int RSound2::command27() {
	playSoundChannels7to8(0x1662);
	return 0;
}

int RSound2::command28() {
	playSoundChannels7to8(0x167B);
	playSoundChannels7to8(0x1685);
	return 0;
}

int RSound2::command30() {
	playSoundChannels7to8(0x16FC);
	return 0;
}

int RSound2::command31() {
	playSoundChannels7to8(0x17F9);
	return 0;
}

int RSound2::command32() {
	// WORKAROUND: this fixes a bug in the original that incorrectly
	// called isSoundActive instead - it passed _callbackFnPtr's own
	// data-segment offset (0x2379) to isSoundActive() as if it were
	// sound data, when it almost certainly meant to check whether a
	// callback was already scheduled before proceeding.
	if (isCallbackScheduled())
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand32();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound2, loadCommand32));
	return 0;
}

void RSound2::loadCommand32() {
	command3();
	_channels[0].load(loadData(0x7BA));
	_channels[1].load(loadData(0x80B));
	_channels[2].load(loadData(0x8A3));
	_channels[3].load(loadData(0x95F));
	_channels[4].load(loadData(0x9E3));
	_channels[5].load(loadData(0xA2D));
}

int RSound2::command33() {
	byte *pData = loadData(0xA54);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand33();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound2, loadCommand33));
	return 0;
}

void RSound2::loadCommand33() {
	command3();
	_channels[0].load(loadData(0xA54));
	_channels[1].load(loadData(0xAF3));
	_channels[2].load(loadData(0xB93));
	_channels[3].load(loadData(0xC3F));
	_channels[4].load(loadData(0xD17));
	_channels[5].load(loadData(0xD9D));
}

int RSound2::command34() {
	byte *pData = loadData(0xE4A);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(96);
		loadCommand34();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound2, loadCommand34));
	return 0;
}

void RSound2::loadCommand34() {
	command3();
	_channels[0].load(loadData(0xE4A));
	_channels[1].load(loadData(0xEE2));
	_channels[2].load(loadData(0xF2F));
	_channels[3].load(loadData(0xF82));
	_channels[4].load(loadData(0xFCD));
	_channels[5].load(loadData(0x1024));
}

int RSound2::command35() {
	// WORKAROUND: this fixes a bug in the original that incorrectly
	// called isSoundActive instead - same shape as command32's bug (see
	// its comment).
	if (isCallbackScheduled())
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x60);
		loadCommand35();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound2, loadCommand35));
	return 0;
}

void RSound2::loadCommand35() {
	// Asymmetric timer: counter=192, period=80.
	resetCallbackTimerEx(192, 80);
	command3();
	_channels[0].load(loadData(0x107A));
	_channels[1].load(loadData(0x11BA));
	_channels[2].load(loadData(0x1246));
	_channels[3].load(loadData(0x1320));
	_channels[4].load(loadData(0x13FA));
	_channels[5].load(loadData(0x149C));
}

int RSound2::command64() {
	playSoundChannels7to8(0x168F);
	return 0;
}

int RSound2::command65() {
	playSoundChannels7to8(0x1699);
	playSoundChannels7to8(0x16B9);
	playSoundChannels7to8(0x16D2);
	return 0;
}

int RSound2::command66() {
	playSoundChannels7to8(0x1726);
	return 0;
}

int RSound2::command67() {
	playSoundChannels7to8(0x1745);
	return 0;
}

int RSound2::command68() {
	playSoundChannels7to8(0x175B);
	return 0;
}

int RSound2::command69() {
	playSoundChannels7to8(0x176B);
	playSoundChannels7to8(0x1775);
	return 0;
}

int RSound2::command70() {
	playSoundChannels7to8(0x1781);
	playSoundChannels7to8(0x1791);
	return 0;
}

int RSound2::command71() {
	playSoundChannels7to8(0x17A1);
	playSoundChannels7to8(0x17E0);
	return 0;
}

int RSound2::command72() {
	playSoundChannels1To5(0x184B);
	playSoundChannels1To5(0x185F);
	return 0;
}

const RSound2::CommandPtr RSound2::_commandList[73] = {
	&RSound2::command0, &RSound2::command1, &RSound2::command2, &RSound2::command3,
	&RSound2::command4, &RSound2::command5, &RSound2::command6, &RSound2::command7,
	&RSound2::command8, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::command16, &RSound2::command17, &RSound2::command18, &RSound2::nullCommand,
	&RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand, &RSound2::nullCommand,
	&RSound2::command24, &RSound2::command25, &RSound2::command26, &RSound2::command27,
	&RSound2::command28, &RSound2::nullCommand, &RSound2::command30, &RSound2::command31,
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
	&RSound2::command72
};

int RSound2::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	return (this->*_commandList[commandId])();
}

/*-----------------------------------------------------------------------*/

RSound3::RSound3(Audio::Mixer *mixer) : RSound(mixer, "rsound.dr3", 0x2750, 0x1780, 0xAC) {
}

int RSound3::command1() {
	// Must call THIS driver's own command3() (not virtual in the base -
	// see class comment).
	command3();
	command5();
	return 0;
}

int RSound3::command3() {
	// Confirmed: enables channels 1-6 AND 9 (seven channels) - overlaps
	// with command5's channels 5,6.
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[0].enable(0xFF);
	_channels[1].enable(0xFF);
	_channels[2].enable(0xFF);
	_channels[3].enable(0xFF);
	_channels[4].enable(0xFF);
	_channels[5].enable(0xFF);
	_channels[8].enable(0xFF);
	return 0;
}

int RSound3::command5() {
	// Confirmed: enables channels 5,6,7,8 (four channels) - overlaps
	// with command3's channels 5,6.
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[4].enable(0xFF);
	_channels[5].enable(0xFF);
	_channels[6].enable(0xFF);
	_channels[7].enable(0xFF);
	return 0;
}

int RSound3::command16() {
	// Gate uses the disassembly's own "isSoundPlaying" name - treated as
	// equivalent to isSoundActive() (confirmed identical for Phantom's
	// RSound4; not independently re-confirmed here).
	byte *pData = loadData(0x7BF);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x90);
		loadCommand16();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound3, loadCommand16));
	return 0;
}

void RSound3::loadCommand16() {
	// Asymmetric timer: counter=28, period=84.
	resetCallbackTimerEx(28, 84);
	command3();
	_channels[0].load(loadData(0x7BF));
	_channels[1].load(loadData(0x88C));
	_channels[2].load(loadData(0x8EE));
	_channels[3].load(loadData(0xA72));
	_channels[4].load(loadData(0xAE6));
	_channels[8].load(loadData(0xBBF));
}

int RSound3::command17() {
	// Ungated scheduling (Pattern A), matching RSound1/2's command17 shape.
	if (isSoundActive(loadData(0x12CC)))
		return 0;
	command3();
	resetCallbackTimerEx(0xC0, 0x60);
	_channels[0].load(loadData(0x12CC));
	_channels[1].load(loadData(0x12FC));
	_channels[2].load(loadData(0x132D));
	_channels[3].load(loadData(0x1344));
	return 0;
}

int RSound3::command18() {
	// See RSound1::command18 - the flat _commandList[] collapses the
	// original's two-table branch into a single array lookup.
	command3();
	return (this->*_commandList[getMusicIndex()])();
}

int RSound3::command24() {
	playSoundChannels6to8(0x137C);
	playSoundChannels6to8(0x1390);
	return 0;
}

int RSound3::command25() {
	playSoundChannels6to8(0x13A2);
	playSoundChannels6to8(0x13B6);
	return 0;
}

int RSound3::command26() {
	playSoundChannels6to8(0x13C8);
	return 0;
}

int RSound3::command27() {
	playSoundChannels6to8(0x13D2);
	return 0;
}

int RSound3::command28() {
	playSoundChannels6to8(0x13EB);
	playSoundChannels6to8(0x13F5);
	return 0;
}

int RSound3::command30() {
	playSoundChannels6to8(0x149A);
	return 0;
}

void RSound3::command31_64Tail(byte variant) {
	*loadData(0x1402) = variant;
	playSoundChannels6to8(0x13FF);
}

int RSound3::command31() {
	command31_64Tail(0x5A);
	return 0;
}

int RSound3::command64() {
	command31_64Tail(0x78);
	return 0;
}

int RSound3::command32() {
	byte *pData = loadData(0xCAE);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand32();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound3, loadCommand32));
	return 0;
}

void RSound3::loadCommand32() {
	command3();
	_channels[0].load(loadData(0xCAE));
	_channels[1].load(loadData(0xD68));
	_channels[2].load(loadData(0xE17));
	_channels[3].load(loadData(0xEC8));
	_channels[4].load(loadData(0xEED));
	_channels[5].load(loadData(0xFAE));
}

int RSound3::command33() {
	byte *pData = loadData(0x102A);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand33();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound3, loadCommand33));
	return 0;
}

void RSound3::loadCommand33() {
	command3();
	_channels[0].load(loadData(0x102A));
	_channels[1].load(loadData(0x10FE));
	_channels[2].load(loadData(0x1160));
	_channels[3].load(loadData(0x116F));
	_channels[4].load(loadData(0x1264));
	_channels[8].load(loadData(0x1284));
}

int RSound3::command65() {
	playSoundChannels6to8(0x1451);
	return 0;
}

int RSound3::command67() {
	playSoundChannels6to8(0x14C4);
	return 0;
}

int RSound3::command68() {
	playSoundChannels6to8(0x14E0);
	return 0;
}

int RSound3::command69() {
	playSoundChannels6to8(0x1552);
	return 0;
}

int RSound3::command70() {
	playSoundChannels6to8(0x1560);
	playSoundChannels6to8(0x157E);
	return 0;
}

int RSound3::command71() {
	playSoundChannels6to8(0x148D);
	return 0;
}

int RSound3::command72() {
	playSoundChannels6to8(0x14FB);
	playSoundChannels6to8(0x1518);
	playSoundChannels6to8(0x1535);
	return 0;
}

int RSound3::command73() {
	playSoundChannels6to8(0x1598);
	return 0;
}

const RSound3::CommandPtr RSound3::_commandList[74] = {
	&RSound3::command0, &RSound3::command1, &RSound3::command2, &RSound3::command3,
	&RSound3::command4, &RSound3::command5, &RSound3::command6, &RSound3::command7,
	&RSound3::command8, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::command16, &RSound3::command17, &RSound3::command18, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::command24, &RSound3::command25, &RSound3::command26, &RSound3::command27,
	&RSound3::command28, &RSound3::nullCommand, &RSound3::command30, &RSound3::command31,
	&RSound3::command32, &RSound3::command33, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand, &RSound3::nullCommand,
	&RSound3::command64, &RSound3::command65, &RSound3::nullCommand, &RSound3::command67,
	&RSound3::command68, &RSound3::command69, &RSound3::command70, &RSound3::command71,
	&RSound3::command72, &RSound3::command73
};

int RSound3::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	return (this->*_commandList[commandId])();
}

/*-----------------------------------------------------------------------*/

RSound4::RSound4(Audio::Mixer *mixer) : RSound(mixer, "rsound.dr4", 0x2930, 0x2370, 0xAC) {
}

int RSound4::command1() {
	// Must call THIS driver's own command3() (not virtual in the base -
	// see class comment).
	command3();
	if (isSoundActive(loadData(0x1F4F)))
		return 0;
	_fadeCheckPeriod = 1; // armFadeCheck
	disableChannelTo(5, 0xFF); // channel 6 (redundant re-disable if command3's own gate already did it - harmless)
	disableChannelTo(6, 0xFF); // channel 7
	disableChannelTo(7, 0xFF); // channel 8
	return 0;
}

int RSound4::command2() {
	resetChannels1to5(); // this driver's own 7-channel version (1-6,9)
	sendGmReset(4);
	return 0;
}

int RSound4::command3() {
	// Confirmed: unconditionally disables channels 1,2,3,4,5,9 (the
	// familiar "lower" group), then conditionally disables channel 6 too -
	// only if loadData(0x1F4F) isn't already playing.
	_fadeCheckPeriod = 1; // armFadeCheck
	disableChannelTo(0, 0xFF); // ch1
	disableChannelTo(1, 0xFF); // ch2
	disableChannelTo(2, 0xFF); // ch3
	disableChannelTo(3, 0xFF); // ch4
	disableChannelTo(4, 0xFF); // ch5
	disableChannelTo(8, 0xFF); // ch9
	if (isSoundActive(loadData(0x1F4F)))
		return 0;
	disableChannelTo(5, 0xFF); // ch6 (conditional)
	return 0;
}

int RSound4::command4() {
	resetChannels6to8(); // this driver's own conditional version
	sendGmReset(RSOUND_CHANNEL_COUNT);
	return 0;
}

void RSound4::resetChannels1to5() {
	// Matches this driver's own resetChannels1to5: channels 1-6 AND 9
	// (seven channels) - one more than the base class's default (1-5,9).
	_isDisabled = true;
	resetChannelRange(0, 5);
	_channels[8]._activeCount = 0;
	_channels[8]._pitchBendFadeStep = 0;
	_channels[8]._volumeFadeStep = 0;
	_channels[8]._panFadeStep = 0;
	resetHeldNotes();
	_isDisabled = false;
}

void RSound4::resetChannels6to8() {
	// Matches this driver's own resetChannels6to8: channels 7,8
	// unconditionally, channel 6 only if it's currently active AND
	// playing loadData(0x1F4F).
	_isDisabled = true;
	resetChannelRange(6, 7);
	if (_channels[5]._activeCount && _channels[5]._soundData == loadData(0x1F4F))
		resetChannelRange(5, 5);
	_isDisabled = false;
}

const RSound4::CommandPtr RSound4::_commandList[79] = {
	&RSound4::command0, &RSound4::command1, &RSound4::command2, &RSound4::command3,
	&RSound4::command4, &RSound4::nullCommand, &RSound4::command6, &RSound4::command7,
	&RSound4::command8, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::command16, &RSound4::command17, &RSound4::command18, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::command24, &RSound4::command25, &RSound4::command26, &RSound4::command27,
	&RSound4::command28, &RSound4::command29, &RSound4::command30, &RSound4::command31,
	&RSound4::command32, &RSound4::command33, &RSound4::nullCommand, &RSound4::command35,
	&RSound4::command36, &RSound4::command37, &RSound4::command38, &RSound4::command39,
	&RSound4::command40, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand, &RSound4::nullCommand,
	&RSound4::command64, &RSound4::command65, &RSound4::command66, &RSound4::command67,
	&RSound4::command68, &RSound4::command69, &RSound4::command70, &RSound4::command71,
	&RSound4::command72, &RSound4::command73, &RSound4::command74, &RSound4::command75,
	&RSound4::command76, &RSound4::command77, &RSound4::command78
};

int RSound4::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	return (this->*_commandList[commandId])();
}

int RSound4::command16() {
	// Genuinely different gate order - see class comment: checks
	// isMusicChannelsActive() first, with no isSoundActive() gate at all
	// on the immediate path.
	if (!isMusicChannelsActive()) {
		loadCommand16();
	} else {
		if (isSoundActive(loadData(0x7C4)))
			return 0;
		scheduleCallback(MAKE_CALLBACK(RSound4, loadCommand16));
	}
	return 0;
}

void RSound4::loadCommand16() {
	command3();
	_channels[0].load(loadData(0x7C4));
	_channels[1].load(loadData(0x804));
	_channels[2].load(loadData(0x838));
	_channels[8].load(loadData(0x887));
}

int RSound4::command17() {
	// Ungated scheduling (Pattern A), matching every other driver's
	// command17 shape.
	if (isSoundActive(loadData(0x1E1C)))
		return 0;
	command3();
	resetCallbackTimerEx(0xC0, 0x60);
	_channels[0].load(loadData(0x1E1C));
	_channels[1].load(loadData(0x1E4C));
	_channels[2].load(loadData(0x1E7D));
	_channels[3].load(loadData(0x1E94));
	return 0;
}

int RSound4::command18() {
	// See RSound1::command18 - the flat _commandList[] collapses the
	// original's two-table branch into a single array lookup.
	command3();
	return (this->*_commandList[getMusicIndex()])();
}

int RSound4::command24() {
	playSoundChannels6to8(0x1ECC);
	playSoundChannels6to8(0x1EE0);
	return 0;
}

int RSound4::command25() {
	playSoundChannels6to8(0x1EF2);
	playSoundChannels6to8(0x1F06);
	return 0;
}

int RSound4::command26() {
	playSoundChannels6to8(0x1F18);
	return 0;
}

int RSound4::command27() {
	playSoundChannels6to8(0x1F22);
	return 0;
}

int RSound4::command28() {
	playSoundChannels6to8(0x1F3B);
	playSoundChannels6to8(0x1F45);
	return 0;
}

int RSound4::command29() {
	// Reuses 0x1F4F both as the isSoundActive gate and the play target -
	// confirmed directly from the disassembly.
	if (isSoundActive(loadData(0x1F4F)))
		return 0;
	playSoundChannels6to8(0x1F4F);
	return 0;
}

int RSound4::command30() {
	playSoundChannels6to8(0x205A);
	return 0;
}

int RSound4::command31() {
	playSoundChannels6to8(0x20FD);
	return 0;
}

int RSound4::command32() {
	byte *pData = loadData(0x964);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand32();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound4, loadCommand32));
	return 0;
}

void RSound4::loadCommand32() {
	command3();
	_channels[0].load(loadData(0x964));
	_channels[1].load(loadData(0xA44));
	_channels[2].load(loadData(0xC50));
	_channels[3].load(loadData(0xCFC));
	_channels[4].load(loadData(0xD18));
	_channels[5].load(loadData(0xD92));
}

int RSound4::command33() {
	byte *pData = loadData(0xE10);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand33();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound4, loadCommand33));
	return 0;
}

void RSound4::loadCommand33() {
	command3();
	_channels[0].load(loadData(0xE10));
	_channels[1].load(loadData(0xE2E));
	_channels[2].load(loadData(0xE56));
	_channels[3].load(loadData(0xE97));
	_channels[8].load(loadData(0xF5E));
}

int RSound4::command35() {
	byte *pData = loadData(0xFFA);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x60);
		loadCommand35();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound4, loadCommand35));
	return 0;
}

void RSound4::loadCommand35() {
	// NOTE: calls command1(), not command3() - unlike every other
	// bucket-4 command in this driver, confirmed directly from the
	// disassembly.
	command1();
	_channels[0].load(loadData(0xFFA));
}

int RSound4::command36() {
	// NOTE: the isSoundActive() gate (0x25F1) doesn't match the first
	// load offset (0x1950) - same kind of confirmed gate/load mismatch
	// seen in RSound3's command32/command35.
	byte *pData = loadData(0x25F1);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x80);
		loadCommand36();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound4, loadCommand36));
	return 0;
}

void RSound4::loadCommand36() {
	command3();
	_channels[0].load(loadData(0x1950));
	_channels[1].load(loadData(0x1BB6));
	_channels[2].load(loadData(0x1BFF));
	_channels[3].load(loadData(0x1C1A));
	_channels[4].load(loadData(0x1E15));
}

int RSound4::command37() {
	byte *pData = loadData(0x1062);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xC0);
		loadCommand37();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound4, loadCommand37));
	return 0;
}

void RSound4::loadCommand37() {
	command3();
	_channels[0].load(loadData(0x1062));
	_channels[1].load(loadData(0x10A0));
	_channels[2].load(loadData(0x10CB));
	_channels[3].load(loadData(0x1100));
	_channels[4].load(loadData(0x1141));
}

int RSound4::command38() {
	byte *pData = loadData(0x1172);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0x60);
		loadCommand38();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound4, loadCommand38));
	return 0;
}

void RSound4::loadCommand38() {
	command3();
	_channels[0].load(loadData(0x1172));
	_channels[1].load(loadData(0x11C9));
	_channels[2].load(loadData(0x1202));
	_channels[3].load(loadData(0x122F));
	_channels[8].load(loadData(0x126E));
}

int RSound4::command39() {
	byte *pData = loadData(0x14DE);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xB0);
		loadCommand39();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound4, loadCommand39));
	return 0;
}

void RSound4::loadCommand39() {
	command3();
	_channels[0].load(loadData(0x14DE));
	_channels[1].load(loadData(0x1550));
	_channels[2].load(loadData(0x15BB));
	_channels[3].load(loadData(0x1672));
	_channels[4].load(loadData(0x170D));
	_channels[8].load(loadData(0x17C8));
}

int RSound4::command40() {
	byte *pData = loadData(0x7BF);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(0xA8);
		loadCommand40();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound4, loadCommand40));
	return 0;
}

void RSound4::loadCommand40() {
	command3();
	_channels[0].load(loadData(0x7BF));
	_channels[1].load(loadData(0x7FF));
	_channels[2].load(loadData(0x833));
	_channels[3].load(loadData(0x7BA));
	_channels[8].load(loadData(0x887));
}

int RSound4::command64() {
	playSoundChannels6to8(0x1F8D);
	return 0;
}

int RSound4::command65() {
	playSoundChannels6to8(0x1FD9);
	return 0;
}

int RSound4::command66() {
	playSoundChannels6to8(0x1FE3);
	return 0;
}

int RSound4::command67() {
	playSoundChannels6to8(0x2037);
	return 0;
}

int RSound4::command68() {
	// 0x1FF8 is called twice in a row - a genuine quirk, preserved
	// exactly rather than collapsed to a single call.
	playSoundChannels6to8(0x1FF8);
	playSoundChannels6to8(0x1FF8);
	return 0;
}

int RSound4::command69() {
	playSoundChannels6to8(0x1FB3);
	return 0;
}

int RSound4::command70() {
	playSoundChannels6to8(0x201B);
	return 0;
}

int RSound4::command71() {
	playSoundChannels6to8(0x202D);
	return 0;
}

int RSound4::command72() {
	playSoundChannels6to8(0x2084);
	return 0;
}

int RSound4::command73() {
	playSoundChannels6to8(0x20CB);
	return 0;
}

int RSound4::command74() {
	playSoundChannels6to8(0x2099);
	return 0;
}

int RSound4::command75() {
	playSoundChannels6to8(0x20B0);
	return 0;
}

int RSound4::command76() {
	playSoundChannels6to8(0x20DD);
	playSoundChannels6to8(0x20ED);
	return 0;
}

int RSound4::command77() {
	playSoundChannels6to8(0x214F);
	return 0;
}

int RSound4::command78() {
	playSoundChannels6to8(0x216A);
	return 0;
}

/*-----------------------------------------------------------------------*/

RSound5::RSound5(Audio::Mixer *mixer) : RSound(mixer, "rsound.dr5", 0x2910, 0x2530, 0x9C) {
}

int RSound5::command1() {
	// Must call THIS driver's own command5() (not virtual in the base -
	// see class comment).
	command3();
	command5();
	return 0;
}

int RSound5::command5() {
	// Same disableChannelTo mechanic as RSound4's
	// command5, for channels 6,7,8.
	_fadeCheckPeriod = 1; // armFadeCheck
	disableChannelTo(5, 0xFF);
	disableChannelTo(6, 0xFF);
	disableChannelTo(7, 0xFF);
	return 0;
}

const RSound5::CommandPtr RSound5::_commandList[79] = {
	&RSound5::command0, &RSound5::command1, &RSound5::command2, &RSound5::command3,
	&RSound5::command4, &RSound5::command5, &RSound5::command6, &RSound5::command7,
	&RSound5::command8, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::command16, &RSound5::command17, &RSound5::command18, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::command24, &RSound5::command25, &RSound5::command26, &RSound5::command27,
	&RSound5::command28, &RSound5::command29, &RSound5::command30, &RSound5::command31,
	&RSound5::command32, &RSound5::command33, &RSound5::command34, &RSound5::command35,
	&RSound5::command36, &RSound5::command37, &RSound5::command38, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand, &RSound5::nullCommand,
	&RSound5::command64, &RSound5::command65, &RSound5::command66, &RSound5::command67,
	&RSound5::command68, &RSound5::command69, &RSound5::command70, &RSound5::command71,
	&RSound5::command72, &RSound5::command73, &RSound5::command74, &RSound5::command75,
	&RSound5::command76, &RSound5::command77, &RSound5::command78
};

int RSound5::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	return (this->*_commandList[commandId])();
}

int RSound5::command16() {
	// The first-time-vs-every-other-time selection -
	// see class comment. Both variants check the SAME two offsets
	// (0x7DC, 0x7E9) as an OR-gate before proceeding, just in reversed
	// order.
	if (_command16Played) {
		if (isSoundActive(loadData(0x7E9)))
			return 0;
		if (isSoundActive(loadData(0x7DC)))
			return 0;
		if (!isMusicChannelsActive()) {
			resetCallbackTimer(192);
			loadCommand16B();
		} else
			scheduleCallback(MAKE_CALLBACK(RSound5, loadCommand16B));
	} else {
		_command16Played = true;
		if (isSoundActive(loadData(0x7DC)))
			return 0;
		if (isSoundActive(loadData(0x7E9)))
			return 0;
		if (!isMusicChannelsActive()) {
			resetCallbackTimer(192);
			loadCommand16A();
		} else
			scheduleCallback(MAKE_CALLBACK(RSound5, loadCommand16A));
	}
	return 0;
}

void RSound5::loadCommand16A() {
	command3();
	_channels[0].load(loadData(0x7DC));
	_channels[1].load(loadData(0x935));
	_channels[2].load(loadData(0x9B8));
	_channels[3].load(loadData(0xA85));
	_channels[4].load(loadData(0xAFA));
	_channels[8].load(loadData(0xD15));
}

void RSound5::loadCommand16B() {
	setMusicIndex(0x10);
	command3();
	_channels[0].load(loadData(0x7E9));
	_channels[1].load(loadData(0x93B));
	_channels[2].load(loadData(0x9BE));
	_channels[3].load(loadData(0xA8B));
	_channels[4].load(loadData(0xB08));
	_channels[8].load(loadData(0xD1B));
}

int RSound5::command17() {
	// Ungated scheduling (Pattern A), matching every other driver's
	// command17 shape.
	if (isSoundActive(loadData(0x1F5A)))
		return 0;
	command3();
	resetCallbackTimerEx(0xC0, 0x60);
	_channels[0].load(loadData(0x1F5A));
	_channels[1].load(loadData(0x1F8A));
	_channels[2].load(loadData(0x1FBB));
	_channels[3].load(loadData(0x1FD2));
	return 0;
}

int RSound5::command18() {
	// See RSound1::command18 - the flat _commandList[] collapses the
	// original's two-table branch into a single array lookup.
	command3();
	return (this->*_commandList[getMusicIndex()])();
}

int RSound5::command24() {
	playSoundChannels6to8(0x200A);
	playSoundChannels6to8(0x201E);
	return 0;
}

int RSound5::command25() {
	playSoundChannels6to8(0x2030);
	playSoundChannels6to8(0x2044);
	return 0;
}

int RSound5::command26() {
	playSoundChannels6to8(0x2056);
	return 0;
}

int RSound5::command27() {
	playSoundChannels6to8(0x2060);
	return 0;
}

int RSound5::command28() {
	playSoundChannels6to8(0x2079);
	playSoundChannels6to8(0x2083);
	return 0;
}

int RSound5::command29() {
	// Reuses 0x208D both as the isSoundActive gate and the play target,
	// matching RSound4's command29 exactly.
	if (isSoundActive(loadData(0x208D)))
		return 0;
	playSoundChannels6to8(0x208D);
	return 0;
}

int RSound5::command30() {
	playSoundChannels6to8(0x22D4);
	return 0;
}

void RSound5::command31_78Tail(byte variant) {
	*loadData(0x2301) = variant;
	playSoundChannels6to8(0x22FE);
}

int RSound5::command31() {
	command31_78Tail(0x5A);
	return 0;
}

int RSound5::command32() {
	// NOTE: calls command1(), not command3(), unlike most other bucket-4
	// commands in this driver.
	byte *pData = loadData(0xDCA);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(72);
		loadCommand32();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound5, loadCommand32));
	return 0;
}

void RSound5::loadCommand32() {
	setMusicIndex(0x10);
	command1();
	_channels[0].load(loadData(0xDCA));
	_channels[1].load(loadData(0xE0B));
	_channels[2].load(loadData(0xE37));
	_channels[3].load(loadData(0xE5F));
	_channels[8].load(loadData(0xEE4));
}

int RSound5::command33() {
	byte *pData = loadData(0xFDA);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(96);
		loadCommand33();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound5, loadCommand33));
	return 0;
}

void RSound5::loadCommand33() {
	setMusicIndex(0x10);
	command3();
	_channels[0].load(loadData(0xFDA));
	_channels[1].load(loadData(0x1080));
	_channels[2].load(loadData(0x1112));
	_channels[3].load(loadData(0x129E));
	_channels[4].load(loadData(0x136A));
	_channels[8].load(loadData(0x1408));
}

int RSound5::command34() {
	byte *pData = loadData(0x16EA);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(192);
		loadCommand34();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound5, loadCommand34));
	return 0;
}

void RSound5::loadCommand34() {
	setMusicIndex(0x10);
	command1();
	_channels[0].load(loadData(0x16EA));
	_channels[1].load(loadData(0x1785));
	_channels[2].load(loadData(0x1832));
	_channels[3].load(loadData(0x1915));
	_channels[4].load(loadData(0x19A9));
	_channels[8].load(loadData(0x1B30));
}

int RSound5::command35() {
	byte *pData = loadData(0x1B54);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(96);
		loadCommand35();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound5, loadCommand35));
	return 0;
}

void RSound5::loadCommand35() {
	setMusicIndex(0x10);
	command1();
	_channels[0].load(loadData(0x1B54));
	_channels[1].load(loadData(0x1B86));
	_channels[2].load(loadData(0x1BCB));
	_channels[3].load(loadData(0x1BE6));
}

int RSound5::command36() {
	// No gate at all - direct single-channel load.
	_channels[3].load(loadData(0x1BFD));
	return 0;
}

int RSound5::command37() {
	// No gate at all (unlike every other bucket-4 command here): just
	// clears _callbackFnPtr directly (counter/period untouched), calls
	// command1(), then 4x playSoundChannels1To5().
	scheduleCallback(nullptr);
	command1();
	playSoundChannels1To5(0x1C2C);
	playSoundChannels1To5(0x1C67);
	playSoundChannels1To5(0x1C99);
	playSoundChannels1To5(0x1CCB);
	return 0;
}

int RSound5::command38() {
	byte *pData = loadData(0x1CDC);
	if (isSoundActive(pData))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(192);
		loadCommand38();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound5, loadCommand38));
	return 0;
}

void RSound5::loadCommand38() {
	setMusicIndex(0x10);
	command1();
	_channels[0].load(loadData(0x1CDC));
	_channels[1].load(loadData(0x1D26));
	_channels[2].load(loadData(0x1D62));
	_channels[3].load(loadData(0x1E1C));
	_channels[4].load(loadData(0x1E34));
	_channels[8].load(loadData(0x1F44));
}

int RSound5::command64() {
	playSoundChannels6to8(0x2150);
	return 0;
}

int RSound5::command65() {
	playSoundChannels6to8(0x212E);
	return 0;
}

int RSound5::command66() {
	playSoundChannels6to8(0x216A);
	return 0;
}

int RSound5::command67() {
	playSoundChannels6to8(0x2260);
	playSoundChannels6to8(0x223C);
	playSoundChannels6to8(0x2213);
	return 0;
}

int RSound5::command68() {
	playSoundChannels6to8(0x22AD);
	playSoundChannels6to8(0x22AD);
	return 0;
}

int RSound5::command69() {
	playSoundChannels6to8(0x2280);
	return 0;
}

int RSound5::command70() {
	// Confirmed bug in the original (missing retn/jmp after the second,
	// redundant call) - treated as a single play, matching the intended
	// final action.
	playSoundChannels6to8(0x21C4);
	return 0;
}

int RSound5::command71() {
	playSoundChannels6to8(0x217E);
	playSoundChannels6to8(0x219D);
	return 0;
}

int RSound5::command72() {
	playSoundChannels6to8(0x21E4);
	return 0;
}

int RSound5::command73() {
	playSoundChannels6to8(0x20DE);
	return 0;
}

int RSound5::command74() {
	playSoundChannels6to8(0x210E);
	playSoundChannels6to8(0x211E);
	return 0;
}

int RSound5::command75() {
	playSoundChannels6to8(0x20F2);
	return 0;
}

int RSound5::command76() {
	playSoundChannels6to8(0x21D4);
	playSoundChannels6to8(0x21B8);
	return 0;
}

int RSound5::command77() {
	// Uses _commandParam directly.
	int param = _commandParam;
	if (param == 0) {
		if (_channels[7]._loopStartPtr == loadData(0x20CB))
			_channels[7]._innerLoopPtr = loadData(0x20C9);
		return 0;
	}

	if ((uint16)param >= 0x8000)
		param = 0;
	*loadData(0x20D6) = (byte)(param & 0x7F);

	if (isSoundActive(loadData(0x20CB)))
		return 0;
	_channels[7].load(loadData(0x20CB));
	return 0;
}

int RSound5::command78() {
	command31_78Tail(0x78);
	return 0;
}

/*-----------------------------------------------------------------------*/

RSound6::RSound6(Audio::Mixer *mixer) : RSound(mixer, "rsound.dr6", 0x2B60, 0x2840, 0xAC) {
}

int RSound6::command1() {
	// Must call THIS driver's own command3() (not virtual in the base -
	// see class comment).
	command3();
	command5();
	return 0;
}

int RSound6::command2() {
	// Matches this driver's own resetChannels1to5: channels 1-6 AND 9
	// (seven channels) - one more than the base class's default (1-5,9).
	_isDisabled = true;
	resetChannelRange(0, 5);
	_channels[8]._activeCount = 0;
	_channels[8]._pitchBendFadeStep = 0;
	_channels[8]._volumeFadeStep = 0;
	_channels[8]._panFadeStep = 0;
	resetHeldNotes();
	_isDisabled = false;
	sendGmReset(4);
	return 0;
}

int RSound6::command3() {
	// Confirmed via symbolic names: enables channels 1,2,3,4,5,6 AND 9
	// (seven channels) - one more than the base class's default (1-5,9).
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[0].enable(0xFF);
	_channels[1].enable(0xFF);
	_channels[2].enable(0xFF);
	_channels[3].enable(0xFF);
	_channels[4].enable(0xFF);
	_channels[5].enable(0xFF);
	_channels[8].enable(0xFF);
	return 0;
}

void RSound6::resetChannels6to8() {
	// Confirmed to match the base class's exact shape (channels 6,7,8
	// unconditionally).
	_isDisabled = true;
	resetChannelRange(5, 7);
	_isDisabled = false;
}

int RSound6::command4() {
	// Custom inline GM-reset loop bounded 8 down to 6 (three channels),
	// not the shared sendGmReset(count) - see class comment.
	resetChannels6to8();
	sendGmResetRange(8, 6);
	return 0;
}

int RSound6::command5() {
	// Confirmed via address arithmetic against command3's range: enables
	// only channels 7,8 (two channels) - one fewer than the base class's
	// default (6,7,8).
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[6].enable(0xFF);
	_channels[7].enable(0xFF);
	return 0;
}

Channel *RSound6::playSoundAny(int offset) {
	// Matches this driver's own playSoundAny exactly: channels 1-8
	// (0-based 0-7), symmetric free/fallback scan.
	return playSoundData(loadData(offset), 0, 7, 7);
}

Channel *RSound6::playSoundChannels1To6(int offset) {
	// Matches playSoundChannesl1to6 (sic): channels 1-6 (0-based 0-5),
	// symmetric free/fallback scan.
	return playSoundData(loadData(offset), 0, 5, 5);
}

int RSound6::isMusicChannelsActive() {
	// Channels 1-6 AND 9 (seven channels), matching
	// command3()'s own wider group.
	return _channels[0]._activeCount || _channels[1]._activeCount ||
		_channels[2]._activeCount || _channels[3]._activeCount ||
		_channels[4]._activeCount || _channels[5]._activeCount ||
		_channels[8]._activeCount;
}

const RSound6::CommandPtr RSound6::_commandList[99] = {
	&RSound6::command0, &RSound6::command1, &RSound6::command2, &RSound6::command3,
	&RSound6::command4, &RSound6::command5, &RSound6::command6, &RSound6::command7,
	&RSound6::command8, &RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::command16, &RSound6::command17, &RSound6::command18, &RSound6::nullCommand,
	&RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::command24, &RSound6::command25, &RSound6::command26, &RSound6::command27,
	&RSound6::command28, &RSound6::nullCommand, &RSound6::command30, &RSound6::command31,
	&RSound6::command32, &RSound6::command33, &RSound6::command34, &RSound6::command35,
	&RSound6::command36, &RSound6::command37, &RSound6::command38, &RSound6::command39,
	&RSound6::command40, &RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::command44, &RSound6::command45, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand, &RSound6::nullCommand,
	&RSound6::command64, &RSound6::command65, &RSound6::command66, &RSound6::command67,
	&RSound6::command68, &RSound6::command69, &RSound6::command70, &RSound6::command71,
	&RSound6::command72, &RSound6::command73, &RSound6::command74, &RSound6::command75,
	&RSound6::command76, &RSound6::command77, &RSound6::command78, &RSound6::command79,
	&RSound6::command80, &RSound6::command81, &RSound6::command82, &RSound6::command83,
	&RSound6::command84, &RSound6::command85, &RSound6::command86, &RSound6::command87,
	&RSound6::command88, &RSound6::command89, &RSound6::command90, &RSound6::command91,
	&RSound6::command92, &RSound6::command93, &RSound6::command94, &RSound6::command95,
	&RSound6::command96, &RSound6::command97, &RSound6::command98
};

int RSound6::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	return (this->*_commandList[commandId])();
}

int RSound6::command16() {
	// Three chained isSoundActive gates (an OR: any match early-exits)
	// before the isMusicChannelsActive() branch.
	if (isSoundActive(loadData(0x7BA)))
		return 0;
	if (isSoundActive(loadData(0x34C0)))
		return 0;
	if (isSoundActive(loadData(0x3636)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(200);
		loadCommand16();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, loadCommand16));
	return 0;
}

void RSound6::loadCommand16() {
	setMusicIndex(0x10);
	command3();
	_channels[0].load(loadData(0x7BA));
	_channels[1].load(loadData(0x842));
	_channels[2].load(loadData(0x881));
}

int RSound6::command17() {
	// Ungated scheduling (Pattern A), matching every other driver's
	// command17 shape.
	if (isSoundActive(loadData(0x1E98)))
		return 0;
	command3();
	resetCallbackTimerEx(0xC0, 0x60);
	_channels[0].load(loadData(0x1E98));
	_channels[1].load(loadData(0x1EC8));
	_channels[2].load(loadData(0x1EF9));
	_channels[3].load(loadData(0x1F10));
	return 0;
}

int RSound6::command18() {
	// See RSound1::command18 - the flat _commandList[] collapses the
	// original's two-table branch into a single array lookup.
	command3();
	return (this->*_commandList[getMusicIndex()])();
}

int RSound6::command24() {
	playSoundAny(0x20E4);
	playSoundAny(0x20F8);
	return 0;
}

int RSound6::command25() {
	playSoundAny(0x210A);
	playSoundAny(0x211E);
	return 0;
}

int RSound6::command26() {
	playSoundAny(0x2130);
	return 0;
}

int RSound6::command27() {
	playSoundAny(0x213A);
	return 0;
}

int RSound6::command28() {
	playSoundChannels7to8(0x2153);
	playSoundChannels7to8(0x215D);
	return 0;
}

int RSound6::command30() {
	playSoundAny(0x23DF);
	return 0;
}

int RSound6::command31() {
	playSoundChannels7to8(0x25E1);
	return 0;
}

/*-----------------------------------------------------------------------*/
// command32/command33 share an intricate tail - see the class comment.

void RSound6::command32_33LoadCh4() {
	_command33Flag = 0;
	_channels[3].load(loadData(0x9FB));
}

void RSound6::command32_33Load() {
	// Falls through to the shared flag check at its end.
	command3();
	_channels[0].load(loadData(0x8CE));
	_channels[1].load(loadData(0x914));
	_channels[2].load(loadData(0x953));
	_channels[8].load(loadData(0x96E));
	if (_command33Flag == 0xFF)
		command32_33LoadCh4();
}

int RSound6::command32() {
	// Matches rsound_command32's own entry point.
	if (_channels[3]._soundData == loadData(0x9FB)) {
		_channels[3]._pendingStop = 0xFF;
		_channels[3]._soundData = loadData(0x250C);
	}
	if (_channels[0]._activeCount) {
		if (_channels[0]._soundData == loadData(0x8CE))
			return 0;
		scheduleCallback(MAKE_CALLBACK(RSound6, command32_33Load));
		return 0;
	}
	resetCallbackTimer(60);
	command32_33Load();
	return 0;
}

int RSound6::command33() {
	// Matches rsound_command33's own entry point: gates, sets the shared
	// flag, then either falls into command32's immediate
	// setup or command32's "load channel 4" tail directly,
	// or - if neither condition holds - falls all the way through into
	// command32's own body from the top (equivalent to calling it).
	if (isSoundActive(loadData(0x9FB)))
		return 0;
	_command33Flag = 0xFF;
	if (!_channels[0]._activeCount) {
		resetCallbackTimer(60);
		command32_33Load();
		return 0;
	}
	if (_channels[0]._soundData == loadData(0x8CE)) {
		command32_33LoadCh4();
		return 0;
	}
	return command32();
}

/*-----------------------------------------------------------------------*/

void RSound6::command34LoadCh1AndRest() {
	// Falls through to command34LoadRestOnly().
	command3();
	_channels[0].load(loadData(0xA1A));
	_channels[1].load(loadData(0xA5C));
	_channels[2].load(loadData(0xA95));
	_channels[3].load(loadData(0xB22));
}

void RSound6::command34LoadRestOnly() {
	// Reached directly (already-looping skip path).
	_channels[1].load(loadData(0xA5C));
	_channels[2].load(loadData(0xA95));
	_channels[3].load(loadData(0xB22));
}

int RSound6::command34() {
	if (isSoundActive(loadData(0xA5C)))
		return 0;
	if (_channels[0]._loopStartPtr == loadData(0xA1A) && _channels[0]._activeCount != 0) {
		command34LoadRestOnly();
		return 0;
	}
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(30);
		command34LoadCh1AndRest();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, command34LoadCh1AndRest));
	return 0;
}

int RSound6::command35() {
	if (isSoundActive(loadData(0xA1A)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(30);
		command3();
		_channels[0].load(loadData(0xA1A));
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, command35));
	return 0;
}

int RSound6::command36() {
	// Two chained isSoundActive gates (an OR).
	if (isSoundActive(loadData(0xB60)))
		return 0;
	if (isSoundActive(loadData(0xCD6)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(200);
		command3();
		_channels[0].load(loadData(0xB60));
		_channels[1].load(loadData(0xBA8));
		_channels[2].load(loadData(0xC42));
		_channels[3].load(loadData(0xCB3));
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, command36));
	return 0;
}

int RSound6::command37() {
	if (isSoundActive(loadData(0xCD6)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(200);
		command3();
		_channels[0].load(loadData(0xCD6));
		_channels[1].load(loadData(0xD26));
		_channels[2].load(loadData(0xD82));
		_channels[3].load(loadData(0xE7E));
		_channels[4].load(loadData(0xF7B));
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, command37));
	return 0;
}

int RSound6::command38() {
	if (isSoundActive(loadData(0xFC0)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(200);
		command3();
		_channels[0].load(loadData(0xFC0));
		_channels[1].load(loadData(0x1018));
		_channels[2].load(loadData(0x1070));
		_channels[3].load(loadData(0x10FA));
		_channels[4].load(loadData(0x11B3));
		_channels[5].load(loadData(0x139C));
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, command38));
	return 0;
}

int RSound6::command39() {
	if (isSoundActive(loadData(0x13EC)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(100);
		command3();
		_channels[0].load(loadData(0x13EC));
		_channels[1].load(loadData(0x1515));
		_channels[2].load(loadData(0x1717));
		_channels[3].load(loadData(0x179F));
		_channels[4].load(loadData(0x17C3));
		_channels[8].load(loadData(0x18DD));
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, command39));
	return 0;
}

int RSound6::command40() {
	if (isSoundActive(loadData(0x195A)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(100);
		command3();
		_channels[0].load(loadData(0x195A));
		_channels[1].load(loadData(0x1A2E));
		_channels[2].load(loadData(0x1B0C));
		_channels[3].load(loadData(0x1BF6));
		_channels[4].load(loadData(0x1D12));
		_channels[5].load(loadData(0x1E0E));
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, command40));
	return 0;
}

int RSound6::command44() {
	if (isSoundActive(loadData(0x1F48)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(48);
		loadCommand44();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, loadCommand44));
	return 0;
}

void RSound6::loadCommand44() {
	setMusicIndex(0x10);
	command3();
	_channels[0].load(loadData(0x1F48));
	_channels[1].load(loadData(0x1F9D));
	_channels[2].load(loadData(0x1FC1));
	_channels[3].load(loadData(0x1FEF));
	_channels[4].load(loadData(0x201D));
	_channels[8].load(loadData(0x2073));
}

int RSound6::command45() {
	if (isSoundActive(loadData(0x20AA)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(30);
		command3();
		playSoundChannels1To6(0x20AA);
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, command45));
	return 0;
}

int RSound6::command64() {
	// Uses _commandParam directly: masks the low 7 bits into the sound
	// data at offset 0x23D7 (11 bytes into the block about to be played)
	// before the gated play.
	*loadData(0x23D7) = (byte)(_commandParam & 0x7F);
	if (isSoundActive(loadData(0x23CC)))
		return 0;
	playSoundChannels7to8(0x23CC);
	return 0;
}

int RSound6::command65() {
	// Complements (and re-masks to 7 bits) a persistent byte within the
	// sound data at offset 0x2592 on every call, before playing 0x2584.
	byte *pByte = loadData(0x2592);
	*pByte = (~*pByte) & 0x7F;
	playSoundAny(0x2584);
	return 0;
}

int RSound6::command66() {
	playSoundAny(0x2357);
	return 0;
}

int RSound6::command67() {
	playSoundAny(0x25A9);
	playSoundAny(0x25C5);
	return 0;
}

int RSound6::command68() {
	playSoundAny(0x2167);
	playSoundAny(0x2176);
	playSoundAny(0x218D);
	return 0;
}

int RSound6::command69() {
	playSoundAny(0x23B7);
	return 0;
}

int RSound6::command70() {
	playSoundAny(0x21E1);
	return 0;
}

int RSound6::command71() {
	playSoundAny(0x239B);
	return 0;
}

int RSound6::command72() {
	_channels[6].load(loadData(0x22A4));
	_channels[7].load(loadData(0x22D8));
	return 0;
}

int RSound6::command73() {
	playSoundAny(0x223D);
	return 0;
}

int RSound6::command74() {
	playSoundAny(0x2258);
	playSoundAny(0x2264);
	return 0;
}

int RSound6::command75() {
	playSoundAny(0x21D1);
	return 0;
}

int RSound6::command76() {
	playSoundAny(0x232C);
	return 0;
}

int RSound6::command77() {
	playSoundChannels7to8(0x230C);
	return 0;
}

int RSound6::command78() {
	playSoundAny(0x21FF);
	playSoundAny(0x2209);
	return 0;
}

int RSound6::command79() {
	_channels[6].load(loadData(0x2219));
	_channels[7].load(loadData(0x222B));
	return 0;
}

int RSound6::command80() {
	// Confirmed: jmp rsound_command4.
	return command4();
}

int RSound6::command81() {
	playSoundAny(0x219C);
	return 0;
}

int RSound6::command82() {
	playSoundAny(0x21AF);
	return 0;
}

int RSound6::command83() {
	playSoundAny(0x2278);
	playSoundAny(0x228F);
	return 0;
}

int RSound6::command84() {
	playSoundAny(0x2365);
	return 0;
}

int RSound6::command85() {
	playSoundAny(0x2380);
	return 0;
}

int RSound6::command86() {
	playSoundAny(0x2409);
	playSoundAny(0x2423);
	return 0;
}

int RSound6::command87() {
	playSoundAny(0x247D);
	return 0;
}

int RSound6::command88() {
	playSoundAny(0x243D);
	playSoundAny(0x245D);
	return 0;
}

int RSound6::command89() {
	playSoundAny(0x2489);
	return 0;
}

int RSound6::command90() {
	// 0x249F is called three times in a row - a genuine quirk, preserved
	// exactly rather than collapsed to a single call.
	playSoundAny(0x249C);
	playSoundAny(0x249F);
	playSoundAny(0x249F);
	playSoundAny(0x249F);
	return 0;
}

int RSound6::command91() {
	playSoundAny(0x24C0);
	return 0;
}

int RSound6::command92() {
	playSoundAny(0x24DC);
	playSoundAny(0x24EB);
	return 0;
}

int RSound6::command93() {
	playSoundAny(0x24FA);
	return 0;
}

int RSound6::command94() {
	playSoundAny(0x250E);
	return 0;
}

int RSound6::command95() {
	playSoundAny(0x2539);
	return 0;
}

int RSound6::command96() {
	if (isSoundActive(loadData(0x26DF)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(90);
		command1();
		// 0x1E70 is played four times in a row - a genuine quirk,
		// preserved exactly rather than collapsed to a single call.
		playSoundChannels1To6(0x1E70);
		playSoundChannels1To6(0x1E70);
		playSoundChannels1To6(0x1E70);
		playSoundChannels1To6(0x1E70);
	} else
		scheduleCallback(MAKE_CALLBACK(RSound6, command96));
	return 0;
}

int RSound6::command97() {
	// 0x2563 is called three times in a row - same quirk shape as
	// command90.
	playSoundAny(0x2560);
	playSoundAny(0x2563);
	playSoundAny(0x2563);
	playSoundAny(0x2563);
	return 0;
}

int RSound6::command98() {
	_channels[3].load(loadData(0x2633));
	_channels[4].load(loadData(0x2646));
	return 0;
}

/*-----------------------------------------------------------------------*/

RSound9::RSound9(Audio::Mixer *mixer) : RSound(mixer, "rsound.dr9", 0x2BC0, 0x52D0, 0x9A) {
}

int RSound9::command1() {
	// Must call THIS driver's own command3()/command5() (not virtual in
	// the base - see class comment).
	command3();
	command5();
	return 0;
}

int RSound9::command3() {
	// Confirmed via symbolic names: enables only channels 1,2,3,4,9 (five
	// channels) - channel 5 has moved to command5()'s group, unlike the
	// base class's default (1-5,9).
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[0].enable(0xFF);
	_channels[1].enable(0xFF);
	_channels[2].enable(0xFF);
	_channels[3].enable(0xFF);
	_channels[8].enable(0xFF);
	return 0;
}

int RSound9::command5() {
	// Confirmed via explicit Channel_enable calls: enables channels
	// 5,6,7,8 (four channels) - one more than the base class's default
	// (6,7,8).
	_fadeCheckPeriod = 1; // armFadeCheck
	_channels[4].enable(0xFF);
	_channels[5].enable(0xFF);
	_channels[6].enable(0xFF);
	_channels[7].enable(0xFF);
	return 0;
}

int RSound9::command32() {
	// No gate at all (unlike every other bucket-4 command in this
	// driver): unconditionally arms the timer and loads.
	resetCallbackTimerEx(98, 84);
	command1();
	_channels[0].load(loadData(0x572));
	_channels[1].load(loadData(0x597));
	_channels[2].load(loadData(0x64B));
	_channels[3].load(loadData(0x67E));
	_channels[4].load(loadData(0x7B1));
	_channels[8].load(loadData(0x898));
	return 0;
}

void RSound9::variantSetupA() {
	*loadData(0x97E) = 0x29;
	*loadData(0x990) = 0x29;
	*loadData(0xA89) = 0x29;
	*loadData(0xA8F) = 0x29;
	*loadData(0x8EC) = 0x41;
	*loadData(0x902) = 0x41;
	*loadData(0x948) = 0x3A;
	*loadData(0x95E) = 0x3A;
}

void RSound9::variantSetupB() {
	*loadData(0x97E) = 0x2A;
	*loadData(0x990) = 0x2A;
	*loadData(0xA89) = 0x2A;
	*loadData(0xA8F) = 0x2A;
	*loadData(0x8EC) = 0x42;
	*loadData(0x902) = 0x42;
	*loadData(0x948) = 0x3B;
	*loadData(0x95E) = 0x3B;
}

void RSound9::command33_47_60Load() {
	command1();
	_channels[0].load(loadData(0x8E2));
	_channels[1].load(loadData(0x940));
	_channels[2].load(loadData(0x976));
	_channels[8].load(loadData(0xA37));
	_channels[4].load(loadData(0xA83));
	_channels[5].load(loadData(0xAE1));
	_channels[6].load(loadData(0xC43));
}

int RSound9::command60() {
	variantSetupA();
	if (!isMusicChannelsActive()) {
		resetCallbackTimerEx(98, 84);
		command33_47_60Load();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, command33_47_60Load));
	return 0;
}

int RSound9::command33Or47() {
	variantSetupB();
	if (!isMusicChannelsActive()) {
		resetCallbackTimerEx(98, 84);
		command33_47_60Load();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, command33_47_60Load));
	return 0;
}

int RSound9::command34Or54() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(56);
		loadCommand34Or54();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand34Or54));
	return 0;
}

void RSound9::loadCommand34Or54() {
	command1();
	_channels[0].load(loadData(0xD1E));
	_channels[1].load(loadData(0xF0D));
	_channels[2].load(loadData(0x110C));
	_channels[4].load(loadData(0x1470));
	_channels[5].load(loadData(0x15A5));
	_channels[6].load(loadData(0x15F0));
	_channels[8].load(loadData(0x130B));
}

int RSound9::command35() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(80);
		loadCommand35();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand35));
	return 0;
}

void RSound9::loadCommand35() {
	command1();
	_channels[0].load(loadData(0x1644));
	_channels[1].load(loadData(0x16B0));
	_channels[2].load(loadData(0x16DC));
	_channels[3].load(loadData(0x17A0));
	_channels[8].load(loadData(0x180E));
	_channels[5].load(loadData(0x1830));
	_channels[6].load(loadData(0x18AC));
}

int RSound9::command36() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(40);
		loadCommand36();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand36));
	return 0;
}

void RSound9::loadCommand36() {
	command1();
	_channels[0].load(loadData(0x1A0E));
	_channels[1].load(loadData(0x1A84));
	_channels[2].load(loadData(0x1B00));
	_channels[3].load(loadData(0x1B34));
	_channels[4].load(loadData(0x1DEA));
	_channels[5].load(loadData(0x1E56));
	_channels[6].load(loadData(0x1EC2));
}

int RSound9::command37() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(80);
		loadCommand37();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand37));
	return 0;
}

void RSound9::loadCommand37() {
	command1();
	_channels[0].load(loadData(0x1F04));
	_channels[1].load(loadData(0x1F94));
	_channels[2].load(loadData(0x2028));
	_channels[3].load(loadData(0x206A));
	_channels[8].load(loadData(0x213C));
	_channels[5].load(loadData(0x219E));
}

int RSound9::command38() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(40);
		loadCommand38();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand38));
	return 0;
}

void RSound9::loadCommand38() {
	command1();
	_channels[0].load(loadData(0x22D2));
	_channels[1].load(loadData(0x234A));
	_channels[2].load(loadData(0x23C8));
	_channels[3].load(loadData(0x1B34));
	_channels[4].load(loadData(0x23FA));
	_channels[8].load(loadData(0x245C));
	_channels[6].load(loadData(0x24BC));
}

int RSound9::command39() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(40);
		loadCommand39();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand39));
	return 0;
}

void RSound9::loadCommand39() {
	command1();
	_channels[0].load(loadData(0x24F8));
	_channels[1].load(loadData(0x2584));
	_channels[2].load(loadData(0x2616));
	_channels[3].load(loadData(0x2710));
	_channels[4].load(loadData(0x2B8A));
	_channels[8].load(loadData(0x2C22));
	_channels[6].load(loadData(0x2C66));
}

int RSound9::command40() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(56);
		loadCommand40();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand40));
	return 0;
}

void RSound9::loadCommand40() {
	command1();
	_channels[0].load(loadData(0xD1E));
	_channels[1].load(loadData(0xF0D));
	_channels[2].load(loadData(0x110C));
	_channels[8].load(loadData(0x2D60));
	_channels[4].load(loadData(0x2E54));
	_channels[5].load(loadData(0x2F33));
	_channels[6].load(loadData(0x2F6B));
}

int RSound9::command41() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(84);
		loadCommand41();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand41));
	return 0;
}

void RSound9::loadCommand41() {
	command1();
	_channels[0].load(loadData(0x2F94));
	_channels[2].load(loadData(0x3460));
	_channels[8].load(loadData(0x3605));
	_channels[4].load(loadData(0x369A));
	_channels[5].load(loadData(0x3773));
	_channels[6].load(loadData(0x39F6));
}

int RSound9::command53() {
	// See class comment: this is the unlabeled function directly
	// following command41() in the disassembly, assigned to index 53 by
	// elimination.
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(84);
		loadCommand53();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand53));
	return 0;
}

void RSound9::loadCommand53() {
	command1();
	_channels[1].load(loadData(0x33C0));
	_channels[2].load(loadData(0x3575));
	_channels[8].load(loadData(0x3686));
	_channels[4].load(loadData(0x36FF));
	_channels[5].load(loadData(0x3966));
	_channels[6].load(loadData(0x3B89));
}

int RSound9::command42() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimerEx(168, 80);
		loadCommand42();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand42));
	return 0;
}

void RSound9::loadCommand42() {
	command1();
	_channels[0].load(loadData(0x2FDD));
	_channels[1].load(loadData(0x3031));
	_channels[2].load(loadData(0x34D9));
	_channels[8].load(loadData(0x3634));
	_channels[4].load(loadData(0x36D1));
	_channels[5].load(loadData(0x3856));
	_channels[6].load(loadData(0x3A93));
}

int RSound9::command43() {
	// No gate at all: unconditionally arms the timer and loads.
	resetCallbackTimer(20);
	command1();
	_channels[0].load(loadData(0x3BF0));
	_channels[1].load(loadData(0x3C4A));
	_channels[2].load(loadData(0x3C92));
	_channels[3].load(loadData(0x3F64));
	return 0;
}

int RSound9::command45() {
	// No gate, no timer setup, no command1() call - just two direct
	// channel loads.
	_channels[4].load(loadData(0x3F70));
	_channels[5].load(loadData(0x3FD0));
	return 0;
}

int RSound9::command46() {
	_channels[4].load(loadData(0x401B));
	_channels[5].load(loadData(0x4070));
	return 0;
}

int RSound9::command48() {
	playSoundChannels6to8(0x507A);
	return 0;
}

int RSound9::command49() {
	playSoundChannels6to8(0x50B0);
	return 0;
}

int RSound9::command50() {
	playSoundChannels6to8(0x50A4);
	return 0;
}

int RSound9::command51() {
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(96);
		loadCommand51();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand51));
	return 0;
}

void RSound9::loadCommand51() {
	command1();
	_channels[0].load(loadData(0x458E));
	_channels[1].load(loadData(0x4623));
	_channels[2].load(loadData(0x46A8));
	_channels[3].load(loadData(0x46F3));
}

int RSound9::command52() {
	// The original's spurious lea bx/call isSoundActive on the
	// deferred-schedule branch (dead/vestigial - see class comment) is
	// intentionally omitted here.
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(84);
		loadCommand52();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand52));
	return 0;
}

void RSound9::loadCommand52() {
	command1();
	_channels[0].load(loadData(0x40C6));
	_channels[1].load(loadData(0x432C));
	_channels[2].load(loadData(0x4375));
	_channels[3].load(loadData(0x438E));
	_channels[4].load(loadData(0x4587));
}

int RSound9::command55() {
	if (isSoundActive(loadData(0x471A)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(96);
		loadCommand55();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand55));
	return 0;
}

void RSound9::loadCommand55() {
	command1();
	_channels[0].load(loadData(0x471A));
	_channels[1].load(loadData(0x47EE));
	_channels[2].load(loadData(0x4A21));
	_channels[3].load(loadData(0x4ABF));
	_channels[4].load(loadData(0x4ADB));
	_channels[5].load(loadData(0x4B35));
}

int RSound9::command57() {
	if (isSoundActive(loadData(0x4B96)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(48);
		loadCommand57();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand57));
	return 0;
}

void RSound9::loadCommand57() {
	command1();
	_channels[0].load(loadData(0x4B96));
	_channels[1].load(loadData(0x4BF0));
	_channels[2].load(loadData(0x4C20));
	_channels[3].load(loadData(0x4C8A));
	_channels[4].load(loadData(0x4CBC));
	_channels[8].load(loadData(0x4D16));
}

int RSound9::command58() {
	if (isSoundActive(loadData(0x4D94)))
		return 0;
	if (!isMusicChannelsActive()) {
		resetCallbackTimer(144);
		loadCommand58();
	} else
		scheduleCallback(MAKE_CALLBACK(RSound9, loadCommand58));
	return 0;
}

void RSound9::loadCommand58() {
	// Calls the full command0() reset (not command1(), unlike every
	// other command in this batch) before loading.
	command0();
	_channels[0].load(loadData(0x4D94));
	_channels[1].load(loadData(0x4DD1));
	_channels[2].load(loadData(0x4DEB));
	_channels[3].load(loadData(0x4E2C));
}

int RSound9::command59() {
	command1();
	_channels[6].load(loadData(0x4D52));
	_channels[7].load(loadData(0x4D78));
	return 0;
}

int RSound9::command61() {
	_channels[8].load(loadData(0x50C0));
	return 0;
}

int RSound9::command62() {
	// No gate at all: unconditionally arms the (asymmetric) timer and loads.
	resetCallbackTimerEx(112, 96);
	command1();
	_channels[0].load(loadData(0x4E44));
	_channels[1].load(loadData(0x4E61));
	_channels[2].load(loadData(0x4F92));
	return 0;
}

int RSound9::command63() {
	playSoundChannels6to8(0x50CF);
	return 0;
}

const RSound9::CommandPtr RSound9::_commandList[96] = {
	&RSound9::command0, &RSound9::command1, &RSound9::command2, &RSound9::command3,
	&RSound9::command4, &RSound9::command5, &RSound9::command6, &RSound9::command7,
	&RSound9::command8, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand, &RSound9::nullCommand,
	&RSound9::command32, &RSound9::command33Or47, &RSound9::command34Or54, &RSound9::command35,
	&RSound9::command36, &RSound9::command37, &RSound9::command38, &RSound9::command39,
	&RSound9::command40, &RSound9::command41, &RSound9::command42, &RSound9::command43,
	&RSound9::nullCommand, &RSound9::command45, &RSound9::command46, &RSound9::command33Or47,
	&RSound9::command48, &RSound9::command49, &RSound9::command50, &RSound9::command51,
	&RSound9::command52, &RSound9::command53, &RSound9::command34Or54, &RSound9::command55,
	&RSound9::nullCommand, &RSound9::command57, &RSound9::command58, &RSound9::command59,
	&RSound9::command60, &RSound9::command61, &RSound9::command62, &RSound9::command63,
	&RSound9::command32, &RSound9::command33Or47, &RSound9::command34Or54, &RSound9::command35,
	&RSound9::command36, &RSound9::command37, &RSound9::command38, &RSound9::command39,
	&RSound9::command40, &RSound9::command41, &RSound9::command42, &RSound9::command43,
	&RSound9::nullCommand, &RSound9::command45, &RSound9::command46, &RSound9::command33Or47,
	&RSound9::command48, &RSound9::command49, &RSound9::command50, &RSound9::command51,
	&RSound9::command52, &RSound9::command53, &RSound9::command34Or54, &RSound9::command55,
	&RSound9::nullCommand, &RSound9::command57, &RSound9::command58, &RSound9::command59,
	&RSound9::command60, &RSound9::command61, &RSound9::command62, &RSound9::command63
};

int RSound9::command(int commandId, int param) {
	if (commandId < 0 || commandId >= ARRAYSIZE(_commandList))
		return 0;

	_commandParam = param;
	return (this->*_commandList[commandId])();
}

} // namespace Sound
} // namespace Dragonsphere
} // namespace MADS
