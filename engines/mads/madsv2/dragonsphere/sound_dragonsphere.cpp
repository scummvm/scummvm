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
#include "mads/madsv2/dragonsphere/sound_dragonsphere.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {

void DragonSoundManager::validate() {
	Common::File f;
	static const char *const MD5[] = {
		"cac84f53ccf18ca56f4c03352037790f",
		"2dcdbe18ca5225384cdb97ceb7f5642a",
		"c6001b0dfe32cb9399ab60742b631c2e",
		"1596b657c6171e13714eaf114bf94641",
		"ecbb8bdf1e2e36fcacedce79761e625b",
		"29639869e5faed1378dd2cdc1d132889",
		nullptr,
		nullptr,
		"379fcc9af2142f15a3e7166eee6dd49d"
	};

	for (int i = 1; i <= 9; ++i) {
		if (i == 7 || i == 8)
			continue;
		Common::Path filename(Common::String::format("asound.dr%d", i));
		if (!f.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		Common::String md5str = Common::computeStreamMD5AsString(f, 8192);
		f.close();

		if (md5str != MD5[i - 1])
			error("Invalid sound file - %s", filename.toString().c_str());
	}
}

void DragonSoundManager::loadDriver(int sectionNumber) {
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
	case 6:
		// TODO
	case 9:
		_driver = new ASound9(_mixer, _opl);
		break;
	default:
		_driver = nullptr;
		return;
	}
}

/*-----------------------------------------------------------------------*/
/* ASound1  (asound.dr1)                                                  *
 *-----------------------------------------------------------------------*/

// Cast a derived-class void loader to the base CallbackFunction type.
// Safe under single inheritance: same function address, only static type changes.
#define MAKE_CALLBACK(cls, fn) \
	reinterpret_cast<ASound::CallbackFunction>(&cls::fn)

const ASound1::CommandPtr ASound1::_commandList[102] = {
	// commands 0-8  (off_11A14)
	&ASound1::command0,   &ASound1::command1,   &ASound1::command2,   &ASound1::command3,
	&ASound1::command4,   &ASound1::command5,   &ASound1::command6,   &ASound1::command7,
	&ASound1::command8,
	// 9-15 absent
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	// commands 16-18, 19=no-op  (off_11A26)
	&ASound1::command16,  &ASound1::command17,  &ASound1::command18,  nullptr,
	// 20-23 absent
	nullptr, nullptr, nullptr, nullptr,
	// commands 24-31, then no-op for slot 32  (off_11A2E)
	&ASound1::command24,  &ASound1::command25,  &ASound1::command26,  &ASound1::command27,
	&ASound1::command28,  &ASound1::command29,  &ASound1::command30,  &ASound1::command31,
	// commands 32-48, 49=no-op  (funcs_12251)
	&ASound1::command32,  &ASound1::command33,  &ASound1::command34,  &ASound1::command35,
	&ASound1::command36,  &ASound1::command37,  &ASound1::command38,  &ASound1::command39,
	&ASound1::command40,  &ASound1::command41,  &ASound1::command42,  &ASound1::command43,
	&ASound1::command44,  &ASound1::command45,  &ASound1::command46,  &ASound1::command47,
	&ASound1::command48,  nullptr,
	// 50-63 absent
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	// commands 64-101  (off_11A64); slot 92 and slot 98 are no-ops
	&ASound1::command64,  &ASound1::command65,  &ASound1::command66,  &ASound1::command67,
	&ASound1::command68,  &ASound1::command69,  &ASound1::command70,  &ASound1::command71,
	&ASound1::command72,  &ASound1::command73,  &ASound1::command74,  &ASound1::command75,
	&ASound1::command76,  &ASound1::command77,  &ASound1::command78,  &ASound1::command79,
	&ASound1::command80,  &ASound1::command81,  &ASound1::command82,  &ASound1::command83,
	&ASound1::command84,  &ASound1::command85,  &ASound1::command86,  &ASound1::command87,
	&ASound1::command88,  &ASound1::command89,  &ASound1::command90,  &ASound1::command91,
	nullptr,              &ASound1::command93,  &ASound1::command94,  &ASound1::command95,
	&ASound1::command96,  &ASound1::command97,  nullptr,              &ASound1::command99,
	&ASound1::command100, &ASound1::command101
};

ASound1::ASound1(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.dr1", 0x2520, 0x49e0) {
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound1::command(int commandId, int param) {
	if (commandId > 101 || !_commandList[commandId])
		return 0;
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASound1::command0()  { return ASound::command0(); }
int ASound1::command1()  { return ASound::command1(); }
int ASound1::command2()  { return ASound::command2(); }
int ASound1::command3()  { return ASound::command3(); }
int ASound1::command4()  { return ASound::command4(); }
int ASound1::command5()  { return ASound::command5(); }
int ASound1::command6()  { return ASound::command6(); }
int ASound1::command7()  { return ASound::command7(); }
int ASound1::command8()  { return ASound::command8(); }

// ---------------------------------------------------------------------------
// command16 - music piece A (castle interior theme)
// isSoundActive guard + isMusicChannelsActive deferred-callback pattern.
// counter = period = 0x90; sets musicIndex = 0x10.
// ---------------------------------------------------------------------------
void ASound1::loadCommand16() {
	resetCallbackTimer(0x90);
	setMusicIndex(0x10);
	ASound::command1();
	_channels[0]->load(loadData(0x262C));
	_channels[1]->load(loadData(0x26DA));
	_channels[2]->load(loadData(0x2752));
	_channels[3]->load(loadData(0x27CE));
	_channels[4]->load(loadData(0x2826));
	_channels[5]->load(loadData(0x284D));
	_channels[6]->load(loadData(0x286D));
}

int ASound1::command16() {
	byte *pData = loadData(0x262C);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand16();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand16));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command17 - special music piece (7 channels)
// Uses isAnyChannelActive: if any channel busy, skip entirely.
// No isSoundActive guard; no deferred callback.
// counter = period = 0x60; no musicIndex update.
// ---------------------------------------------------------------------------
int ASound1::command17() {
	if (isAnyChannelActive())
		return 0;
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x43CE));
	_channels[1]->load(loadData(0x4403));
	_channels[2]->load(loadData(0x4436));
	_channels[3]->load(loadData(0x444F));
	_channels[4]->load(loadData(0x4482));
	_channels[5]->load(loadData(0x4490));
	_channels[6]->load(loadData(0x44A8));
	return 0;
}

// ---------------------------------------------------------------------------
// command18 - re-entrant music launcher (delegate to base ASound::command18)
// ---------------------------------------------------------------------------
int ASound1::command18() { return ASound::command18(); }

// ---------------------------------------------------------------------------
// commands 24-31 - sound effects via findFreeChannelFull
// ---------------------------------------------------------------------------
int ASound1::command24() {
	findFreeChannelFull(loadData(0x44E1));
	findFreeChannelFull(loadData(0x4516));
	return 0;
}

int ASound1::command25() {
	findFreeChannelFull(loadData(0x4546));
	findFreeChannelFull(loadData(0x4574));
	return 0;
}

int ASound1::command26() { findFreeChannelFull(loadData(0x45A4)); return 0; }
int ASound1::command27() { findFreeChannelFull(loadData(0x45B0)); return 0; }
int ASound1::command28() { findFreeChannelFull(loadData(0x46CF)); return 0; }
int ASound1::command29() { findFreeChannelFull(loadData(0x45E4)); return 0; }
int ASound1::command30() { findFreeChannelFull(loadData(0x461C)); return 0; }

// command31 - writes transposition byte 0x67 into the sound block before loading
int ASound1::command31() {
	*loadData(0x4680) = 0x67;
	findFreeChannelFull(loadData(0x467D));
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32-48 - music piece loaders
// All use: isSoundActive guard → isMusicChannelsActive → defer or load.
// Exceptions: command35 uses command3 (not command1); command43/48 are
// special (no deferred callback; modify sound data bytes directly);
// command44 uses asymmetric counter/period; command46 uses isAnyChannelActive
// as the outer guard instead of isSoundActive.
// ---------------------------------------------------------------------------

void ASound1::loadCommand32() {
	resetCallbackTimer(0xB0);
	ASound::command1();
	_channels[0]->load(loadData(0x299A));
	_channels[1]->load(loadData(0x29E3));
	_channels[2]->load(loadData(0x2A45));
	_channels[3]->load(loadData(0x2A6F));
	_channels[4]->load(loadData(0x2A9F));
	_channels[5]->load(loadData(0x2AAE));
}

int ASound1::command32() {
	byte *pData = loadData(0x299A);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand32();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand32));
	}
	return 0;
}

void ASound1::loadCommand33() {
	resetCallbackTimer(0xB0);
	setMusicIndex(0x21);
	ASound::command1();
	_channels[0]->load(loadData(0x2ABE));
	_channels[1]->load(loadData(0x2B64));
	_channels[2]->load(loadData(0x2BC8));
	_channels[3]->load(loadData(0x2C03));
	_channels[4]->load(loadData(0x2C82));
	_channels[5]->load(loadData(0x2C9A));
}

int ASound1::command33() {
	byte *pData = loadData(0x2ABE);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand33();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand33));
	}
	return 0;
}

void ASound1::loadCommand34() {
	resetCallbackTimer(0x50);
	ASound::command1();
	_channels[0]->load(loadData(0x3A74));
	_channels[1]->load(loadData(0x3AE1));
	_channels[2]->load(loadData(0x3AFD));
	_channels[3]->load(loadData(0x3B19));
	_channels[4]->load(loadData(0x3B26));
	_channels[5]->load(loadData(0x3B33));
}

int ASound1::command34() {
	byte *pData = loadData(0x3A74);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand34();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand34));
	}
	return 0;
}

// command35 uses command3 (pending-stop fade) instead of command1
void ASound1::loadCommand35() {
	resetCallbackTimer(0x60);
	ASound::command3();
	_channels[0]->load(loadData(0x38A4));
	_channels[1]->load(loadData(0x393C));
	_channels[2]->load(loadData(0x39C2));
	_channels[3]->load(loadData(0x3A0E));
	_channels[4]->load(loadData(0x3A3E));
	_channels[5]->load(loadData(0x3A54));
	_channels[6]->load(loadData(0x3A63));
}

int ASound1::command35() {
	byte *pData = loadData(0x393C);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand35();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand35));
	}
	return 0;
}

void ASound1::loadCommand36() {
	resetCallbackTimer(0x80);
	ASound::command1();
	_channels[0]->load(loadData(0x3F36));
	_channels[1]->load(loadData(0x3FFA));
	_channels[2]->load(loadData(0x40E9));
	_channels[3]->load(loadData(0x417D));
	_channels[4]->load(loadData(0x41ED));
	_channels[5]->load(loadData(0x41FC));
}

int ASound1::command36() {
	byte *pData = loadData(0x3F36);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand36();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand36));
	}
	return 0;
}

void ASound1::loadCommand37() {
	resetCallbackTimer(0xC0);
	ASound::command1();
	_channels[0]->load(loadData(0x4220));
	_channels[1]->load(loadData(0x4285));
	_channels[2]->load(loadData(0x42F0));
	_channels[3]->load(loadData(0x42FF));
	_channels[4]->load(loadData(0x4310));
	_channels[5]->load(loadData(0x431C));
}

int ASound1::command37() {
	byte *pData = loadData(0x4220);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand37();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand37));
	}
	return 0;
}

void ASound1::loadCommand38() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x12CA));
	_channels[1]->load(loadData(0x13BF));
	_channels[2]->load(loadData(0x14A2));
	_channels[3]->load(loadData(0x15DB));
	_channels[4]->load(loadData(0x16D3));
	_channels[5]->load(loadData(0x1715));
}

int ASound1::command38() {
	byte *pData = loadData(0x12CA);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand38();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand38));
	}
	return 0;
}

void ASound1::loadCommand39() {
	resetCallbackTimer(0xB0);
	ASound::command1();
	_channels[0]->load(loadData(0x179C));
	_channels[1]->load(loadData(0x17F1));
	_channels[2]->load(loadData(0x1857));
	_channels[3]->load(loadData(0x18B1));
	_channels[4]->load(loadData(0x18C0));
	_channels[5]->load(loadData(0x18CF));
}

int ASound1::command39() {
	byte *pData = loadData(0x179C);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand39();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand39));
	}
	return 0;
}

void ASound1::loadCommand40() {
	resetCallbackTimer(0xA8);
	ASound::command1();
	_channels[0]->load(loadData(0x18DE));
	_channels[1]->load(loadData(0x1A57));
	_channels[2]->load(loadData(0x1C0D));
	_channels[3]->load(loadData(0x1ED6));
	_channels[4]->load(loadData(0x20FE));
	_channels[5]->load(loadData(0x239B));
}

int ASound1::command40() {
	byte *pData = loadData(0x18DE);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand40();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand40));
	}
	return 0;
}

void ASound1::loadCommand41() {
	resetCallbackTimer(0x90);
	ASound::command1();
	_channels[0]->load(loadData(0x23D2));
	_channels[1]->load(loadData(0x2440));
	_channels[2]->load(loadData(0x261C));
	_channels[3]->load(loadData(0x24BA));
	_channels[4]->load(loadData(0x259A));
	_channels[5]->load(loadData(0x25EF));
}

int ASound1::command41() {
	byte *pData = loadData(0x23D2);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand41();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand41));
	}
	return 0;
}

void ASound1::loadCommand42() {
	resetCallbackTimer(144);
	setMusicIndex(0x29);
	ASound::command1();
	_channels[0]->load(loadData(0x1192));
	_channels[1]->load(loadData(0x11C8));
	_channels[2]->load(loadData(0x11FB));
	_channels[3]->load(loadData(0x123C));
	_channels[4]->load(loadData(0x126E));
	_channels[5]->load(loadData(0x128B));
	_channels[6]->load(loadData(0x12A1));
}

int ASound1::command42() {
	byte *pData = loadData(0x1192);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand42();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand42));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// loadMusicPitchBend - shared loader for command43 and command48
// Resets timer to 0x54, fades current music, loads the pitch-bend piece
// (6 channels starting at 0x287C).
// ---------------------------------------------------------------------------
void ASound1::loadMusicPitchBend() {
	resetCallbackTimer(0x54);
	ASound::command1();
	_channels[0]->load(loadData(0x287C));
	_channels[1]->load(loadData(0x28C8));
	_channels[2]->load(loadData(0x290F));
	_channels[3]->load(loadData(0x2938));
	_channels[4]->load(loadData(0x295B));
	_channels[5]->load(loadData(0x298C));
}

// command43 - set pitch-bend variant byte to 0x2F (bent), mark script var,
// then start the pitch-bend piece only if it is not already playing.
int ASound1::command43() {
	*loadData(0x28C9) = 0x2F;
	setScriptVar(14, 1);
	if (!isSoundActive(loadData(0x287C))) {
		loadMusicPitchBend();
		setMusicIndex(0x10);
	}
	return 0;
}

// command44 - asymmetric timer: counter=0x60 but period=0xE0
void ASound1::loadCommand44() {
	resetCallbackTimerEx(0x60, 0xE0);
	ASound::command1();
	_channels[0]->load(loadData(0x3B40));
	_channels[1]->load(loadData(0x3B9E));
	_channels[2]->load(loadData(0x3F26));
	_channels[3]->load(loadData(0x3C25));
	_channels[4]->load(loadData(0x3DB4));
	_channels[5]->load(loadData(0x3E8B));
}

int ASound1::command44() {
	byte *pData = loadData(0x3B40);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand44();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand44));
	}
	return 0;
}

void ASound1::loadCommand45() {
	resetCallbackTimer(0x60);
	setMusicIndex(0x2D);
	ASound::command1();
	_channels[0]->load(loadData(0x32A0));
	_channels[1]->load(loadData(0x33E1));
	_channels[2]->load(loadData(0x34CE));
	_channels[3]->load(loadData(0x35DB));
	_channels[4]->load(loadData(0x36CB));
	_channels[5]->load(loadData(0x3873));
}

int ASound1::command45() {
	byte *pData = loadData(0x32A0);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand45();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand45));
	}
	return 0;
}

// command46 - uses isAnyChannelActive as the outer guard (not isSoundActive),
// then isMusicChannelsActive for the defer/immediate decision.
void ASound1::loadCommand46() {
	resetCallbackTimer(0x90);
	ASound::command1();
	_channels[0]->load(loadData(0x432E));
	_channels[1]->load(loadData(0x4385));
	_channels[2]->load(loadData(0x43B8));
	_channels[3]->load(loadData(0x43C3));
}

int ASound1::command46() {
	if (isAnyChannelActive())
		return 0;
	if (!isMusicChannelsActive())
		loadCommand46();
	else
		scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand46));
	return 0;
}

void ASound1::loadCommand47() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x12C4));
	_channels[1]->load(loadData(0x13B2));
	_channels[2]->load(loadData(0x149C));
	_channels[3]->load(loadData(0x15D5));
	_channels[4]->load(loadData(0x16F4));
	_channels[5]->load(loadData(0x1759));
}

int ASound1::command47() {
	byte *pData = loadData(0x12C4);
	if (!isSoundActive(pData)) {
		if (!isMusicChannelsActive())
			loadCommand47();
		else
			scheduleCallback(MAKE_CALLBACK(ASound1, loadCommand47));
	}
	return 0;
}

// command48 - set pitch-bend variant byte to 0x1B (normal), then load if not active
int ASound1::command48() {
	*loadData(0x28C9) = 0x1B;
	setScriptVar(14, 0);
	if (!isSoundActive(loadData(0x287C))) {
		loadMusicPitchBend();
		setMusicIndex(0x28);
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64-101 - SFX loaders via findFreeChannelFull / findFreeChannel /
// direct channel load.
// ---------------------------------------------------------------------------
int ASound1::command64()  { findFreeChannelFull(loadData(0x2D20)); return 0; }
int ASound1::command65()  { findFreeChannelFull(loadData(0x2D35)); return 0; }
int ASound1::command66()  { findFreeChannelFull(loadData(0x2D3F)); return 0; }
int ASound1::command67()  { findFreeChannelFull(loadData(0x2D4B)); return 0; }
int ASound1::command68()  { findFreeChannelFull(loadData(0x2D59)); return 0; }

int ASound1::command69() {
	findFreeChannelFull(loadData(0x2D78));
	findFreeChannelFull(loadData(0x2D8E));
	return 0;
}

int ASound1::command70()  { findFreeChannelFull(loadData(0x2DA0)); return 0; }

// command71 - load directly into channel 8
int ASound1::command71() { _channels[8]->load(loadData(0x2DB9)); return 0; }

int ASound1::command72()  { findFreeChannelFull(loadData(0x2DC7)); return 0; }
int ASound1::command73()  { findFreeChannelFull(loadData(0x2DAC)); return 0; }

int ASound1::command74() {
	findFreeChannelFull(loadData(0x2DD5));
	findFreeChannelFull(loadData(0x2DDE));
	return 0;
}

int ASound1::command75()  { findFreeChannelFull(loadData(0x2DF2)); return 0; }
int ASound1::command76()  { findFreeChannelFull(loadData(0x2E0D)); return 0; }

int ASound1::command77() {
	findFreeChannelFull(loadData(0x2E1C));
	findFreeChannelFull(loadData(0x2E28));
	return 0;
}

int ASound1::command78()  { findFreeChannelFull(loadData(0x2E4C)); return 0; }
int ASound1::command79()  { findFreeChannelFull(loadData(0x2E34)); return 0; }
int ASound1::command80()  { findFreeChannelFull(loadData(0x2E5E)); return 0; }

// command81 - loads the same data block twice (as in the original)
int ASound1::command81() {
	findFreeChannelFull(loadData(0x2EA4));
	findFreeChannelFull(loadData(0x2EA4));
	return 0;
}

int ASound1::command82()  { findFreeChannelFull(loadData(0x2ECB)); return 0; }
int ASound1::command83()  { findFreeChannelFull(loadData(0x2EE3)); return 0; }
int ASound1::command84()  { findFreeChannelFull(loadData(0x2EF5)); return 0; }
int ASound1::command85()  { findFreeChannelFull(loadData(0x2EFF)); return 0; }

// command86 - load directly into channel 7
int ASound1::command86() { _channels[7]->load(loadData(0x2F0C)); return 0; }

int ASound1::command87() {
	findFreeChannelFull(loadData(0x2D67));
	findFreeChannelFull(loadData(0x2D89));
	return 0;
}

int ASound1::command88()  { findFreeChannelFull(loadData(0x2F24)); return 0; }
int ASound1::command89()  { findFreeChannelFull(loadData(0x2F2E)); return 0; }

int ASound1::command90() {
	findFreeChannelFull(loadData(0x2CAA));
	findFreeChannelFull(loadData(0x2CD4));
	return 0;
}

int ASound1::command91() {
	findFreeChannelFull(loadData(0x2CDF));
	findFreeChannelFull(loadData(0x2D15));
	return 0;
}

// command92 = nullptr (no-op, slot mapped to null in _commandList)

int ASound1::command93() {
	findFreeChannelFull(loadData(0x2F3C));
	findFreeChannelFull(loadData(0x2F45));
	return 0;
}

int ASound1::command94()  { findFreeChannelFull(loadData(0x2F57)); return 0; }

// command95 - four-voice piece using findFreeChannel (not Full)
int ASound1::command95() {
	findFreeChannel(loadData(0x2F6B));
	findFreeChannel(loadData(0x2FD7));
	findFreeChannel(loadData(0x302E));
	findFreeChannel(loadData(0x3045));
	return 0;
}

int ASound1::command96()  { findFreeChannelFull(loadData(0x305C)); return 0; }

// command97 = sub_11F98 in original
int ASound1::command97() {
	findFreeChannelFull(loadData(0x307E));
	findFreeChannelFull(loadData(0x3094));
	return 0;
}

// command98 = nullptr (no-op, slot mapped to null in _commandList)

int ASound1::command99()  { findFreeChannelFull(loadData(0x30B0)); return 0; }
int ASound1::command100() { findFreeChannelFull(loadData(0x30BA)); return 0; }
int ASound1::command101() { findFreeChannelFull(loadData(0x30C4)); return 0; }

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound2  (asound.dr2)                                                  *
 *-----------------------------------------------------------------------*/

const ASound2::CommandPtr ASound2::_commandList[76] = {
	// 0-8: delegate to base
	&ASound2::command0,  &ASound2::command1,  &ASound2::command2,  &ASound2::command3,
	&ASound2::command4,  &ASound2::command5,  &ASound2::command6,  &ASound2::command7,
	&ASound2::command8,
	// 9-15: nullptr
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// 16-19: off_11A26
	&ASound2::command16, &ASound2::command17, &ASound2::command18, nullptr,
	// 20-23: nullptr
	nullptr,             nullptr,             nullptr,             nullptr,
	// 24-31: off_11A2E
	&ASound2::command24, &ASound2::command25, &ASound2::command26, &ASound2::command27,
	&ASound2::command28, &ASound2::command29, &ASound2::command30, &ASound2::command31,
	// 32-36: funcs_11C87 (slot 36 = no-op)
	&ASound2::command32, &ASound2::command33, &ASound2::command34, &ASound2::command35,
	nullptr,
	// 37-63: nullptr
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,
	// 64-75: off_11A4A (slots 73-75 = no-ops)
	&ASound2::command64,    &ASound2::command65, &ASound2::command66,    &ASound2::command67,
	&ASound2::command68,    &ASound2::command69_70, &ASound2::command69_70, &ASound2::command71,
	&ASound2::command72,    nullptr,             nullptr,             nullptr
};

ASound2::ASound2(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.dr2", 0x1fa0, 0x2950) {
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound2::command(int commandId, int param) {
	if (commandId > 75 || !_commandList[commandId])
		return 0;
	if (commandId >= 32 && commandId < 64)
		setMusicIndex(commandId);
	return (this->*_commandList[commandId])();
}

int ASound2::command0() { return ASound::command0(); }
int ASound2::command1() { return ASound::command1(); }
int ASound2::command2() { return ASound::command2(); }
int ASound2::command3() { return ASound::command3(); }
int ASound2::command4() { return ASound::command4(); }
int ASound2::command5() { return ASound::command5(); }
int ASound2::command6() { return ASound::command6(); }
int ASound2::command7() { return ASound::command7(); }
int ASound2::command8() { return ASound::command8(); }

void ASound2::loadCommand16() {
	resetCallbackTimer(0x60);
	setMusicIndex(0x10);
	ASound::command1();
	_channels[0]->load(loadData(0x2584));
	_channels[1]->load(loadData(0x25E0));
	_channels[2]->load(loadData(0x2630));
	_channels[3]->load(loadData(0x2640));
}

int ASound2::command16() {
	byte *pData = loadData(0x2584);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound2, loadCommand16));
		else
			loadCommand16();
	}
	return 0;
}

int ASound2::command17() {
	byte *pData = loadData(0x2268);
	if (!isSoundActive(pData)) {
		resetCallbackTimer(0x60);
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x229D));
		_channels[2]->load(loadData(0x22D0));
		_channels[3]->load(loadData(0x22E9));
		_channels[4]->load(loadData(0x231C));
		_channels[5]->load(loadData(0x232A));
		_channels[8]->load(loadData(0x2342));
	}
	return 0;
}

int ASound2::command18() {
	ASound::command1();
	if (getMusicIndex() <= 18)
		return command16();
	return (this->*_commandList[getMusicIndex()])();
}

int ASound2::command24() {
	playSound(0x237B);
	playSound(0x23B0);
	return 0;
}

int ASound2::command25() {
	playSound(0x23E0);
	playSound(0x240E);
	return 0;
}

int ASound2::command26() {
	playSound(0x243E);
	return 0;
}

int ASound2::command27() {
	playSound(0x244A);
	return 0;
}

int ASound2::command28() {
	playSound(0x2569);
	return 0;
}

int ASound2::command29() {
	playSound(0x247E);
	return 0;
}

int ASound2::command30() {
	playSound(0x24B6);
	return 0;
}

int ASound2::command31() {
	*loadData(0x251A) = 0x67;
	playSound(0x2517);
	return 0;
}

void ASound2::loadCommand32() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x1192));
	_channels[1]->load(loadData(0x1201));
	_channels[2]->load(loadData(0x127A));
	_channels[3]->load(loadData(0x1360));
	_channels[4]->load(loadData(0x144A));
	_channels[5]->load(loadData(0x1514));
}

int ASound2::command32() {
	byte *pData = loadData(0x1192);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound2, loadCommand32));
		else
			loadCommand32();
	}
	return 0;
}

void ASound2::loadCommand33() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x155E));
	_channels[1]->load(loadData(0x162B));
	_channels[2]->load(loadData(0x16E9));
	_channels[3]->load(loadData(0x17B2));
	_channels[4]->load(loadData(0x189C));
	_channels[5]->load(loadData(0x1921));
	_channels[8]->load(loadData(0x19DF));
}

int ASound2::command33() {
	byte *pData = loadData(0x155E);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound2, loadCommand33));
		else
			loadCommand33();
	}
	return 0;
}

void ASound2::loadCommand34() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x1FDA));
	_channels[1]->load(loadData(0x2071));
	_channels[2]->load(loadData(0x20C0));
	_channels[3]->load(loadData(0x2116));
	_channels[4]->load(loadData(0x2126));
	_channels[5]->load(loadData(0x2180));
	_channels[8]->load(loadData(0x2190));
}

int ASound2::command34() {
	byte *pData = loadData(0x1FDA);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound2, loadCommand34));
		else
			loadCommand34();
	}
	return 0;
}

void ASound2::loadCommand35() {
	resetCallbackTimerEx(0xC0, 0x50);
	ASound::command1();
	_channels[0]->load(loadData(0x1A2C));
	_channels[1]->load(loadData(0x1B74));
	_channels[2]->load(loadData(0x1C06));
	_channels[3]->load(loadData(0x1CE3));
	_channels[4]->load(loadData(0x1DDE));
	_channels[5]->load(loadData(0x1E91));
	_channels[8]->load(loadData(0x1F62));
}

int ASound2::command35() {
	byte *pData = loadData(0x1A2C);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound2, loadCommand35));
		else
			loadCommand35();
	}
	return 0;
}

int ASound2::command64() {
	playSound(0x219E);
	playSound(0x21A8);
	return 0;
}

int ASound2::command65() {
	playSound(0x1F92);
	playSound(0x1FAB);
	playSound(0x1FBE);
	playSound(0x1FC9);
	return 0;
}

int ASound2::command66() {
	playSound(0x21B2);
	return 0;
}

int ASound2::command67() {
	playSound(0x21BC);
	playSound(0x21C5);
	return 0;
}

int ASound2::command68() {
	playSound(0x21E7);
	return 0;
}

int ASound2::command69_70() {
	playSound(0x24EF);
	playSound(0x2503);
	return 0;
}

int ASound2::command71() {
	playSound(0x21FC);
	return 0;
}

int ASound2::command72() {
	playSound(0x2242);
	playSound(0x224F);
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound3  (asound.dr3)                                                  *
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
		: ASound(mixer, opl, "asound.dr3", 0x1f30, 0x2750) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
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
/* ASound4  (asound.dr4)                                                  *
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
		: ASound(mixer, opl, "asound.dr4", 0x2120, 0x31d0) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
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
/* ASound5  (asound.dr5)                                                  *
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
		: ASound(mixer, opl, "asound.dr5", 0x20d0, 0x2ee0) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
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

/*-----------------------------------------------------------------------*/
/* ASound5  (asound.dr6)                                                  *
 *-----------------------------------------------------------------------*/

const ASound6::CommandPtr ASound6::_commandList[98] = {
	// commands 0-8  (asound_commands1)
	&ASound6::command0,  &ASound6::command1,  &ASound6::command2,  &ASound6::command3,
	&ASound6::command4,  &ASound6::command5,  &ASound6::command6,  &ASound6::command7,
	&ASound6::command8
};

ASound6::ASound6(Audio::Mixer *mixer, OPL::OPL *opl)
	: ASound(mixer, opl, "asound.dr6", 0x2370, 0x3870) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound6::command(int commandId, int param) {
	if (commandId > 71 || !_commandList[commandId])
		return 0;

	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASound6::command0() {
	return ASound::command0();
}
int ASound6::command1() {
	return ASound::command1();
}
int ASound6::command2() {
	return ASound::command2();
}
int ASound6::command3() {
	return ASound::command3();
}
int ASound6::command4() {
	return ASound::command4();
}
int ASound6::command5() {
	return ASound::command5();
}
int ASound6::command6() {
	return ASound::command6();
}
int ASound6::command7() {
	return ASound::command7();
}
int ASound6::command8() {
	return ASound::command8();
}

/*-----------------------------------------------------------------------*/

const ASound9::CommandPtr ASound9::_commandList[65] = {
	// 0-8: delegate to base
	&ASound9::command0,    &ASound9::command1,    &ASound9::command2,    &ASound9::command3,
	&ASound9::command4,    &ASound9::command5,    &ASound9::command6,    &ASound9::command7,
	&ASound9::command8,
	// 9-15: nullptr (not in any dispatch range)
	nullptr,               nullptr,               nullptr,               nullptr,
	nullptr,               nullptr,               nullptr,
	// 16-19: nullptr (off_11A26, all null)
	nullptr,               nullptr,               nullptr,               nullptr,
	// 20-23: nullptr
	nullptr,               nullptr,               nullptr,               nullptr,
	// 24-31: nullptr (off_11A2E, all null)
	nullptr,               nullptr,               nullptr,               nullptr,
	nullptr,               nullptr,               nullptr,               nullptr,
	// 32-63: music/SFX (off_11A3E)
	&ASound9::command32,   &ASound9::command33_47, &ASound9::command34,  &ASound9::command35,
	&ASound9::command36,   &ASound9::command37,   &ASound9::command38,   &ASound9::command39,
	&ASound9::command40,   &ASound9::command41,   &ASound9::command42,   &ASound9::command43,
	nullptr,               &ASound9::command45,   &ASound9::command46,   &ASound9::command33_47,
	&ASound9::command48,   &ASound9::command49,   &ASound9::command50,   &ASound9::command51,
	&ASound9::command52,   &ASound9::command53,   &ASound9::command54,   &ASound9::command55,
	nullptr,               &ASound9::command57,   &ASound9::command58,   &ASound9::command59,
	nullptr,               &ASound9::command61,   &ASound9::command62,   &ASound9::command63,
	// 64: nullptr (off_11A80, single null entry)
	nullptr,
};

ASound9::ASound9(Audio::Mixer *mixer, OPL::OPL *opl) :
		ASound(mixer, opl, "asound.dr9", 0x23a0, 0x6a10) {
	// Load sound samples
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound9::command(int commandId, int param) {
	if (commandId > 64 || !_commandList[commandId])
		return 0;
	return (this->*_commandList[commandId])();
}

// Commands 0-8: delegate to base
int ASound9::command0() { return ASound::command0(); }
int ASound9::command1() { return ASound::command1(); }
int ASound9::command2() { return ASound::command2(); }
int ASound9::command3() { return ASound::command3(); }
int ASound9::command4() { return ASound::command4(); }
int ASound9::command5() { return ASound::command5(); }
int ASound9::command6() { return ASound::command6(); }
int ASound9::command7() { return ASound::command7(); }
int ASound9::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// Music command 32 — deferred loader (isMusicChannelsActive pattern B)
// ---------------------------------------------------------------------------

void ASound9::loadCommand32() {
	ASound::command1();
	resetCallbackTimerEx(0x62, 0x54);
	_channels[0]->load(loadData(0x1192));
	_channels[1]->load(loadData(0x11DA));
	_channels[2]->load(loadData(0x128F));
	_channels[3]->load(loadData(0x12C2));
	_channels[4]->load(loadData(0x13FB));
	_channels[5]->load(loadData(0x156B));
	_channels[8]->load(loadData(0x14EA));
}

int ASound9::command32() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand32));
	else
		loadCommand32();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 33/47 — shared deferred loader
// ---------------------------------------------------------------------------

void ASound9::loadCommand33_47() {
	ASound::command1();
	resetCallbackTimerEx(0x54, 0x46);
	_channels[0]->load(loadData(0x21A6));
	_channels[1]->load(loadData(0x2219));
	_channels[2]->load(loadData(0x225F));
	_channels[4]->load(loadData(0x239A));
	_channels[5]->load(loadData(0x23FC));
	_channels[6]->load(loadData(0x2563));
	_channels[3]->load(loadData(0x264A));
	_channels[7]->load(loadData(0x2657));
	_channels[8]->load(loadData(0x231E));
}

int ASound9::command33_47() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand33_47));
	else
		loadCommand33_47();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 34
// ---------------------------------------------------------------------------

void ASound9::loadCommand34() {
	ASound::command1();
	resetCallbackTimer(0x38);
	_channels[0]->load(loadData(0x2664));
	_channels[1]->load(loadData(0x285C));
	_channels[2]->load(loadData(0x2A61));
	_channels[8]->load(loadData(0x2C64));
	_channels[4]->load(loadData(0x2DE3));
	_channels[5]->load(loadData(0x2EE6));
	_channels[6]->load(loadData(0x2F31));
}

int ASound9::command34() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand34));
	else
		loadCommand34();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 35
// ---------------------------------------------------------------------------

void ASound9::loadCommand35() {
	ASound::command1();
	resetCallbackTimer(0x50);
	_channels[0]->load(loadData(0x2F7E));
	_channels[1]->load(loadData(0x2FC7));
	_channels[2]->load(loadData(0x300E));
	_channels[3]->load(loadData(0x30D2));
	_channels[8]->load(loadData(0x3148));
	_channels[5]->load(loadData(0x31D2));
	_channels[6]->load(loadData(0x326A));
}

int ASound9::command35() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand35));
	else
		loadCommand35();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 36
// ---------------------------------------------------------------------------

void ASound9::loadCommand36() {
	ASound::command1();
	resetCallbackTimer(0x28);
	_channels[0]->load(loadData(0x33D4));
	_channels[1]->load(loadData(0x345F));
	_channels[2]->load(loadData(0x34D5));
	_channels[3]->load(loadData(0x3505));
	_channels[4]->load(loadData(0x37D1));
	_channels[5]->load(loadData(0x3895));
	_channels[8]->load(loadData(0x383D));
}

int ASound9::command36() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand36));
	else
		loadCommand36();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 37
// ---------------------------------------------------------------------------

void ASound9::loadCommand37() {
	ASound::command1();
	resetCallbackTimer(0x50);
	_channels[0]->load(loadData(0x38D8));
	_channels[1]->load(loadData(0x393D));
	_channels[2]->load(loadData(0x39A1));
	_channels[3]->load(loadData(0x3A05));
	_channels[4]->load(loadData(0x3A89));
	_channels[8]->load(loadData(0x3A97));
	_channels[5]->load(loadData(0x3AF3));
}

int ASound9::command37() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand37));
	else
		loadCommand37();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 38
// ---------------------------------------------------------------------------

void ASound9::loadCommand38() {
	ASound::command1();
	resetCallbackTimer(0x28);
	_channels[0]->load(loadData(0x3C5C));
	_channels[1]->load(loadData(0x3CE9));
	_channels[2]->load(loadData(0x3D69));
	_channels[3]->load(loadData(0x3505));
	_channels[4]->load(loadData(0x3D95));
	_channels[8]->load(loadData(0x3DF9));
	_channels[6]->load(loadData(0x3E45));
}

int ASound9::command38() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand38));
	else
		loadCommand38();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 39
// ---------------------------------------------------------------------------

void ASound9::loadCommand39() {
	ASound::command1();
	resetCallbackTimer(0x28);
	_channels[0]->load(loadData(0x3E6E));
	_channels[1]->load(loadData(0x3EE3));
	_channels[2]->load(loadData(0x3F55));
	_channels[3]->load(loadData(0x4049));
	_channels[4]->load(loadData(0x44E3));
	_channels[5]->load(loadData(0x459B));
	_channels[8]->load(loadData(0x4571));
}

int ASound9::command39() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand39));
	else
		loadCommand39();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 40
// ---------------------------------------------------------------------------

void ASound9::loadCommand40() {
	ASound::command1();
	resetCallbackTimer(0x38);
	_channels[0]->load(loadData(0x2664));
	_channels[1]->load(loadData(0x285C));
	_channels[2]->load(loadData(0x2A61));
	_channels[8]->load(loadData(0x4692));
	_channels[4]->load(loadData(0x479F));
	_channels[5]->load(loadData(0x4841));
	_channels[6]->load(loadData(0x4875));
}

int ASound9::command40() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand40));
	else
		loadCommand40();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 41
// ---------------------------------------------------------------------------

void ASound9::loadCommand41() {
	ASound::command1();
	resetCallbackTimer(0x54);
	_channels[0]->load(loadData(0x157A));
	_channels[1]->load(loadData(0x1629));
	_channels[2]->load(loadData(0x1A68));
	_channels[8]->load(loadData(0x1BA2));
	_channels[4]->load(loadData(0x1C58));
	_channels[5]->load(loadData(0x1D38));
	_channels[6]->load(loadData(0x1FC2));
}

int ASound9::command41() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand41));
	else
		loadCommand41();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 42
// ---------------------------------------------------------------------------

void ASound9::loadCommand42() {
	ASound::command1();
	resetCallbackTimerEx(0xA8, 0x50);
	_channels[0]->load(loadData(0x15B3));
	_channels[1]->load(loadData(0x163A));
	_channels[2]->load(loadData(0x1AE1));
	_channels[3]->load(loadData(0x1613));
	_channels[8]->load(loadData(0x1BE7));
	_channels[4]->load(loadData(0x1C8F));
	_channels[5]->load(loadData(0x1E1B));
	_channels[6]->load(loadData(0x205F));
}

int ASound9::command42() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand42));
	else
		loadCommand42();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 43 — direct load (no deferred path)
// ---------------------------------------------------------------------------

int ASound9::command43() {
	ASound::command1();
	resetCallbackTimer(0x60);
	_channels[0]->load(loadData(0x4A00));
	_channels[1]->load(loadData(0x4A5F));
	_channels[2]->load(loadData(0x4AAB));
	_channels[3]->load(loadData(0x4D7D));
	_channels[4]->load(loadData(0x4D93));
	_channels[5]->load(loadData(0x4D9E));
	_channels[6]->load(loadData(0x4DA9));
	_channels[7]->load(loadData(0x4DB2));
	return 0;
}

// ---------------------------------------------------------------------------
// SFX commands 45, 46
// ---------------------------------------------------------------------------

int ASound9::command45() {
	playSound(0x48AA);
	playSound(0x490A);
	return 0;
}

int ASound9::command46() {
	playSound(0x4955);
	playSound(0x49AA);
	return 0;
}

// ---------------------------------------------------------------------------
// SFX commands 48, 49, 50
// ---------------------------------------------------------------------------

int ASound9::command48() { playSound(0x54C3); return 0; }
int ASound9::command49() { playSound(0x54CD); return 0; }
int ASound9::command50() { playSound(0x54D7); return 0; }

// ---------------------------------------------------------------------------
// Music command 51 — deferred loader (timer set before command1)
// ---------------------------------------------------------------------------

void ASound9::loadCommand51() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x4DBE));
	_channels[1]->load(loadData(0x4E56));
	_channels[2]->load(loadData(0x4EDC));
	_channels[3]->load(loadData(0x4F32));
	_channels[4]->load(loadData(0x4F62));
	_channels[5]->load(loadData(0x4F75));
	_channels[6]->load(loadData(0x4F84));
	_channels[7]->load(loadData(0x4F95));
}

int ASound9::command51() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand51));
	else
		loadCommand51();
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 52 — isSoundActive guard + deferred loader
// ---------------------------------------------------------------------------

void ASound9::loadCommand52() {
	resetCallbackTimer(0x54);
	ASound::command1();
	_channels[0]->load(loadData(0x4FAA));
	_channels[1]->load(loadData(0x5211));
	_channels[2]->load(loadData(0x524A));
	_channels[3]->load(loadData(0x526E));
	_channels[4]->load(loadData(0x5469));
	_channels[5]->load(loadData(0x5475));
	_channels[6]->load(loadData(0x548E));
}

int ASound9::command52() {
	byte *pData = loadData(0x4FAA);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand52));
		else
			loadCommand52();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Command 53 — delayed fade: arms a one-shot timer to call command1
// ---------------------------------------------------------------------------

void ASound9::command53_loader() {
	ASound::command1();
}

int ASound9::command53() {
	resetCallbackTimer(0x708);
	scheduleCallback(MAKE_CALLBACK(ASound9, command53_loader));
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 54 — isSoundActive guard + deferred loader
// ---------------------------------------------------------------------------

void ASound9::loadCommand54() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x5548));
	_channels[1]->load(loadData(0x555E));
	_channels[2]->load(loadData(0x556F));
	_channels[3]->load(loadData(0x558F));
}

int ASound9::command54() {
	byte *pData = loadData(0x5548);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand54));
		else
			loadCommand54();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 55 — isSoundActive guard + deferred loader
// ---------------------------------------------------------------------------

void ASound9::loadCommand55() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x5712));
	_channels[1]->load(loadData(0x57EA));
	_channels[2]->load(loadData(0x5A1E));
	_channels[3]->load(loadData(0x5ACA));
	_channels[4]->load(loadData(0x5AEE));
	_channels[5]->load(loadData(0x5B4E));
	_channels[6]->load(loadData(0x5BBB));
	_channels[7]->load(loadData(0x5BC8));
	_channels[8]->load(loadData(0x5BE0));
}

int ASound9::command55() {
	byte *pData = loadData(0x5712);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand55));
		else
			loadCommand55();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 57 — isSoundActive guard + deferred loader
// ---------------------------------------------------------------------------

void ASound9::loadCommand57() {
	resetCallbackTimer(0x30);
	ASound::command1();
	_channels[0]->load(loadData(0x5CD6));
	_channels[1]->load(loadData(0x5D5E));
	_channels[2]->load(loadData(0x5DD1));
	_channels[3]->load(loadData(0x5E0F));
	_channels[4]->load(loadData(0x5E54));
	_channels[5]->load(loadData(0x5F36));
	_channels[6]->load(loadData(0x5F97));
	_channels[7]->load(loadData(0x5FEC));
	_channels[8]->load(loadData(0x5ECD));
}

int ASound9::command57() {
	byte *pData = loadData(0x5CD6);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand57));
		else
			loadCommand57();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 58 — music-channel isSoundActive guard + deferred loader
// ---------------------------------------------------------------------------

void ASound9::loadCommand58() {
	resetCallbackTimer(0x90);
	ASound::command1();
	_channels[0]->load(loadData(0x5BEE));
	_channels[1]->load(loadData(0x5C23));
	_channels[2]->load(loadData(0x5C3E));
	_channels[3]->load(loadData(0x5C7E));
	_channels[4]->load(loadData(0x5C9D));
	_channels[5]->load(loadData(0x5CA8));
	_channels[6]->load(loadData(0x5CC8));
}

int ASound9::command58() {
	byte *pData = loadData(0x5BEE);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand58));
		else
			loadCommand58();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 59 — direct load (no deferred, no timer)
// ---------------------------------------------------------------------------

int ASound9::command59() {
	ASound::command1();
	_channels[0]->load(loadData(0x55AE));
	_channels[1]->load(loadData(0x5637));
	_channels[2]->load(loadData(0x5621));
	_channels[3]->load(loadData(0x56DF));
	return 0;
}

// ---------------------------------------------------------------------------
// Command 61 — stop SFX channels then play three SFX voices
// ---------------------------------------------------------------------------

int ASound9::command61() {
	ASound::command5();
	playSound(0x54E1);
	playSound(0x5507);
	playSound(0x552D);
	return 0;
}

// ---------------------------------------------------------------------------
// Music command 62 — deferred loader (channels 0-2, 5-6 only)
// ---------------------------------------------------------------------------

void ASound9::loadCommand62() {
	ASound::command1();
	resetCallbackTimer(0x40);
	_channels[0]->load(loadData(0x6006));
	_channels[1]->load(loadData(0x6204));
	_channels[2]->load(loadData(0x6409));
	_channels[5]->load(loadData(0x660C));
	_channels[6]->load(loadData(0x667F));
}

int ASound9::command62() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASound9, loadCommand62));
	else
		loadCommand62();
	return 0;
}

// ---------------------------------------------------------------------------
// SFX command 63
// ---------------------------------------------------------------------------

int ASound9::command63() {
	playSound(0x66EC);
	return 0;
}

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
