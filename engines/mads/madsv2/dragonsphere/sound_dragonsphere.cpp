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
	static const char *const MD5_DEMO[] = {
		"c4fc9e9d7e2392c69ea7b3ca997e832d",
		nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr,
		"21432c2dd055d0d505ede8fecc77b29a"
	};

	for (int i = 1; i <= 9; ++i) {
		if (i == 7 || i == 8)
			continue;
		if (_isDemo && i != 1 && i != 9)
			continue;

		Common::Path filename(Common::String::format("asound.dr%d", i));
		if (!f.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		Common::String md5str = Common::computeStreamMD5AsString(f, 8192);
		f.close();

		if (md5str != (_isDemo ? MD5_DEMO[i - 1] : MD5[i - 1]))
			error("Invalid sound file - %s", filename.toString().c_str());
	}
}

void DragonSoundManager::loadDriver(int sectionNumber) {
	switch (sectionNumber) {
	case 1:
		if (_isDemo)
			_driver = new ASoundDemo1(_mixer, _opl);
		else
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
		_driver = new ASound6(_mixer, _opl);
		break;
	case 9:
		if (_isDemo)
			_driver = new ASoundDemo9(_mixer, _opl);
		else
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
	// commands 0-8  (off_11A14)
	&ASound3::command0,  &ASound3::command1,  &ASound3::command2,  &ASound3::command3,
	&ASound3::command4,  &ASound3::command5,  &ASound3::command6,  &ASound3::command7,
	&ASound3::command8,
	// 9-15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 16-19  (off_11A26; slot 19 = no-op)
	&ASound3::command16, &ASound3::command17, &ASound3::command18, nullptr,
	// 20-23 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24-31  (off_11A2E; slot at 32 = no-op/unreachable)
	&ASound3::command24, &ASound3::command25, &ASound3::command26, &ASound3::command27,
	&ASound3::command28, &ASound3::command29, &ASound3::command30, &ASound3::command31,
	// commands 32-33  (funcs_11C61; slot 34 = no-op)
	&ASound3::command32, &ASound3::command33,
	// 34-63 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,
	// commands 64-73  (off_11A46); slots 74-76 = no-ops
	&ASound3::command64, &ASound3::command65, &ASound3::command66, &ASound3::command67,
	&ASound3::command68, &ASound3::command69, &ASound3::command70, &ASound3::command71,
	&ASound3::command72, &ASound3::command73,
	nullptr,             nullptr,             nullptr
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
	if (commandId >= 32 && commandId < 64)
		setMusicIndex(commandId);
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
// command16 (Pattern B deferred): timer=0xA8, musicIndex=0x10, ch0-6
// ---------------------------------------------------------------------------
void ASound3::loadCommand16() {
	resetCallbackTimer(0xA8);
	setMusicIndex(0x10);
	ASound::command1();
	_channels[0]->load(loadData(0x1192));
	_channels[1]->load(loadData(0x1279));
	_channels[2]->load(loadData(0x1307));
	_channels[3]->load(loadData(0x149E));
	_channels[4]->load(loadData(0x153C));
	_channels[5]->load(loadData(0x1624));
	_channels[6]->load(loadData(0x1755));
}

int ASound3::command16() {
	byte *pData = loadData(0x1192);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound3, loadCommand16));
		else
			loadCommand16();
	}
	return 0;
}

// command17: isSoundActive guard, timer=0x60, command1, ch0-6
int ASound3::command17() {
	byte *pData = loadData(0x213A);
	if (!isSoundActive(pData)) {
		resetCallbackTimer(0x60);
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x216F));
		_channels[2]->load(loadData(0x21A2));
		_channels[3]->load(loadData(0x21BB));
		_channels[4]->load(loadData(0x21EE));
		_channels[5]->load(loadData(0x21FC));
		_channels[6]->load(loadData(0x2214));
	}
	return 0;
}

// command18: re-launch last music piece (command1 + re-dispatch via musicIndex)
int ASound3::command18() {
	ASound::command1();
	if (getMusicIndex() <= 0x12)
		return command16();
	return (this->*_commandList[getMusicIndex()])();
}

// ---------------------------------------------------------------------------
// commands 24-31 (off_11A2E)
// ---------------------------------------------------------------------------

int ASound3::command24() {
	playSound(0x224D);
	playSound(0x2282);
	return 0;
}

int ASound3::command25() {
	playSound(0x22B2);
	playSound(0x22E0);
	return 0;
}

int ASound3::command26() {
	playSound(0x2310);
	return 0;
}

int ASound3::command27() {
	playSound(0x231C);
	return 0;
}

int ASound3::command28() {
	playSound(0x243B);
	return 0;
}

int ASound3::command29() {
	playSound(0x2350);
	return 0;
}

int ASound3::command30() {
	playSound(0x2388);
	return 0;
}

// command31: patch pitch byte then play (0x23EC = 0x23E9+3)
int ASound3::command31() {
	*loadData(0x23EC) = 0x67;
	playSound(0x23E9);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32-33 (funcs_11C61, Pattern B deferred)
// ---------------------------------------------------------------------------
void ASound3::loadCommand32() {
	resetCallbackTimer(0xA8);
	ASound::command1();
	_channels[0]->load(loadData(0x17BA));
	_channels[1]->load(loadData(0x1884));
	_channels[2]->load(loadData(0x1954));
	_channels[3]->load(loadData(0x1A03));
	_channels[4]->load(loadData(0x1A40));
	_channels[5]->load(loadData(0x1B17));
	_channels[6]->load(loadData(0x1B99));
}

int ASound3::command32() {
	byte *pData = loadData(0x17BA);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound3, loadCommand32));
		else
			loadCommand32();
	}
	return 0;
}

void ASound3::loadCommand33() {
	resetCallbackTimer(0xA8);
	ASound::command1();
	_channels[0]->load(loadData(0x1D68));
	_channels[1]->load(loadData(0x1E5D));
	_channels[2]->load(loadData(0x1ECE));
	_channels[3]->load(loadData(0x1F05));
	_channels[4]->load(loadData(0x2006));
	_channels[5]->load(loadData(0x203C));
	_channels[6]->load(loadData(0x209B));
}

int ASound3::command33() {
	byte *pData = loadData(0x1D68);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound3, loadCommand33));
		else
			loadCommand33();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64-73 (off_11A46)
// ---------------------------------------------------------------------------

// command64: patch pitch byte (0x78) then play — same block as command31 (0x78 vs 0x67)
int ASound3::command64() {
	*loadData(0x23EC) = 0x78;
	playSound(0x23E9);
	return 0;
}

int ASound3::command65() {
	playSound(0x1C5E);
	return 0;
}

int ASound3::command66() {
	playSound(0x1C74);
	playSound(0x1C7C);
	return 0;
}

int ASound3::command67() {
	_channels[8]->load(loadData(0x1C8E));
	return 0;
}

int ASound3::command68() {
	_channels[7]->load(loadData(0x1CA2));
	_channels[8]->load(loadData(0x1CAF));
	return 0;
}

int ASound3::command69() {
	playSound(0x1CE9);
	return 0;
}

int ASound3::command70() {
	playSound(0x1CFB);
	return 0;
}

int ASound3::command71() {
	playSound(0x1D13);
	return 0;
}

int ASound3::command72() {
	_channels[7]->load(loadData(0x1D20));
	_channels[8]->load(loadData(0x1D2B));
	return 0;
}

int ASound3::command73() {
	_channels[7]->load(loadData(0x1D57));
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound4  (asound.dr4)                                                  *
 *-----------------------------------------------------------------------*/

const ASound4::CommandPtr ASound4::_commandList[82] = {
	// commands 0-8 (off_11A14; slot 5 = no-op)
	&ASound4::command0,  &ASound4::command1,  &ASound4::command2,  &ASound4::command3,
	&ASound4::command4,  nullptr,             &ASound4::command6,  &ASound4::command7,
	&ASound4::command8,
	// 9-15 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,
	// commands 16-19 (off_11A26; slot 19 = no-op)
	&ASound4::command16, &ASound4::command17, &ASound4::command18, nullptr,
	// 20-23 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24-31 (off_11A2E; slot 32 unreachable via off_11A2E)
	&ASound4::command24, &ASound4::command25, &ASound4::command26, &ASound4::command27,
	&ASound4::command28, &ASound4::command29, &ASound4::command30, &ASound4::command31,
	// commands 32-41 (funcs_11E51; slots 34 and 41 = no-ops)
	&ASound4::command32, &ASound4::command33, nullptr,             &ASound4::command35,
	&ASound4::command36, &ASound4::command37, &ASound4::command38, &ASound4::command39,
	&ASound4::command40, nullptr,
	// 42-63 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,
	// commands 64-81 (off_11A54; slot 79 = no-op; slots 80-81 = stubs)
	&ASound4::command64, &ASound4::command65, &ASound4::command66, &ASound4::command67,
	&ASound4::command68, &ASound4::command69, &ASound4::command70, &ASound4::command71,
	&ASound4::command72, &ASound4::command73, &ASound4::command74, &ASound4::command75,
	&ASound4::command76, &ASound4::command77, &ASound4::command78, nullptr,
	&ASound4::command80, &ASound4::command81
};

ASound4::ASound4(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.dr4", 0x2120, 0x31d0) {
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASound4::command(int commandId, int param) {
	if (commandId > 81 || !_commandList[commandId])
		return 0;
	if (commandId >= 32 && commandId < 64)
		setMusicIndex(commandId);
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound (command5 slot = nullptr, not dispatched)
int ASound4::command0() { return ASound::command0(); }
int ASound4::command1() { return ASound::command1(); }
int ASound4::command2() { return ASound::command2(); }
int ASound4::command3() { return ASound::command3(); }
int ASound4::command4() { return ASound::command4(); }
int ASound4::command6() { return ASound::command6(); }
int ASound4::command7() { return ASound::command7(); }
int ASound4::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// command16 - Pattern B deferred music, timer=0xC0, ASound::command3, ch0-6
// ---------------------------------------------------------------------------
void ASound4::loadCommand16() {
	resetCallbackTimer(0xC0);
	setMusicIndex(0x10);
	ASound::command3();
	_channels[0]->load(loadData(0x1F1A));
	_channels[1]->load(loadData(0x1F77));
	_channels[2]->load(loadData(0x1FBA));
	_channels[3]->load(loadData(0x2010));
	_channels[4]->load(loadData(0x20F5));
	_channels[5]->load(loadData(0x2117));
	_channels[6]->load(loadData(0x213D));
}

int ASound4::command16() {
	byte *pData = loadData(0x1F1A);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound4, loadCommand16));
		else
			loadCommand16();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command17 - Pattern A with timer reset, ASound::command1, ch0-6
// ---------------------------------------------------------------------------
int ASound4::command17() {
	byte *pData = loadData(0x2BC0);
	if (!isSoundActive(pData)) {
		resetCallbackTimer(0x60);
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x2BF5));
		_channels[2]->load(loadData(0x2C28));
		_channels[3]->load(loadData(0x2C41));
		_channels[4]->load(loadData(0x2C74));
		_channels[5]->load(loadData(0x2C82));
		_channels[6]->load(loadData(0x2C9A));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command18 - re-dispatch to last music command
// ---------------------------------------------------------------------------
int ASound4::command18() {
	ASound::command1();
	if (getMusicIndex() <= 0x12)
		return command16();
	return (this->*_commandList[getMusicIndex()])();
}

// ---------------------------------------------------------------------------
// commands 24-31 (off_11A2E) - SFX
// ---------------------------------------------------------------------------
int ASound4::command24() { playSound(0x2CD3); playSound(0x2D08); return 0; }
int ASound4::command25() { playSound(0x2D38); playSound(0x2D66); return 0; }
int ASound4::command26() { playSound(0x2D96); return 0; }
int ASound4::command27() { playSound(0x2DA2); return 0; }
int ASound4::command28() { playSound(0x2EC1); return 0; }
int ASound4::command29() { playSound(0x2DD6); return 0; }
int ASound4::command30() { playSound(0x2E0E); return 0; }
int ASound4::command31() {
	*loadData(0x2E72) = 0x67;
	playSound(0x2E6F);
	return 0;
}

// ---------------------------------------------------------------------------
// command32 - Pattern B deferred music, timer=0x60, ASound::command3, ch0-6
// ---------------------------------------------------------------------------
void ASound4::loadCommand32() {
	resetCallbackTimer(0x60);
	ASound::command3();
	_channels[0]->load(loadData(0x1192));
	_channels[1]->load(loadData(0x128E));
	_channels[2]->load(loadData(0x14A8));
	_channels[3]->load(loadData(0x1575));
	_channels[4]->load(loadData(0x15B5));
	_channels[5]->load(loadData(0x1638));
	_channels[6]->load(loadData(0x16C8));
}

int ASound4::command32() {
	byte *pData = loadData(0x1192);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound4, loadCommand32));
		else
			loadCommand32();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command33 - Pattern B deferred music, timer=0xC0, ASound::command3, ch0-6
// ---------------------------------------------------------------------------
void ASound4::loadCommand33() {
	resetCallbackTimer(0xC0);
	ASound::command3();
	_channels[0]->load(loadData(0x16DE));
	_channels[1]->load(loadData(0x1701));
	_channels[2]->load(loadData(0x172B));
	_channels[3]->load(loadData(0x176F));
	_channels[4]->load(loadData(0x18E8));
	_channels[5]->load(loadData(0x18F8));
	_channels[6]->load(loadData(0x183F));
}

int ASound4::command33() {
	byte *pData = loadData(0x16DE);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound4, loadCommand33));
		else
			loadCommand33();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command35 - Pattern B deferred music, timer=0x54, ASound::command1, ch0-2
// ---------------------------------------------------------------------------
void ASound4::loadCommand35() {
	resetCallbackTimer(0x54);
	ASound::command1();
	_channels[0]->load(loadData(0x1912));
	_channels[1]->load(loadData(0x1965));
	_channels[2]->load(loadData(0x1975));
}

int ASound4::command35() {
	byte *pData = loadData(0x1912);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound4, loadCommand35));
		else
			loadCommand35();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command36 - Pattern B deferred music, timer=84, ASound::command1, ch0-6
// ---------------------------------------------------------------------------
void ASound4::loadCommand36() {
	resetCallbackTimer(84);
	ASound::command1();
	_channels[0]->load(loadData(0x26CE));
	_channels[1]->load(loadData(0x2935));
	_channels[2]->load(loadData(0x296E));
	_channels[3]->load(loadData(0x2992));
	_channels[4]->load(loadData(0x2B8D));
	_channels[5]->load(loadData(0x2B99));
	_channels[6]->load(loadData(0x2BB2));
}

int ASound4::command36() {
	byte *pData = loadData(0x26CE);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound4, loadCommand36));
		else
			loadCommand36();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command37 - Pattern B deferred music, timer=0x40, ASound::command1, ch0-6
// ---------------------------------------------------------------------------
void ASound4::loadCommand37() {
	resetCallbackTimer(0x40);
	ASound::command1();
	_channels[0]->load(loadData(0x1986));
	_channels[1]->load(loadData(0x19CB));
	_channels[2]->load(loadData(0x19FA));
	_channels[3]->load(loadData(0x1A33));
	_channels[4]->load(loadData(0x1A77));
	_channels[5]->load(loadData(0x1AAA));
	_channels[6]->load(loadData(0x1ABA));
}

int ASound4::command37() {
	byte *pData = loadData(0x1986);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound4, loadCommand37));
		else
			loadCommand37();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command38 - Pattern B deferred music, timer=0x40, ASound::command1, ch0-6
// ---------------------------------------------------------------------------
void ASound4::loadCommand38() {
	resetCallbackTimer(0x40);
	ASound::command1();
	_channels[0]->load(loadData(0x1AC8));
	_channels[1]->load(loadData(0x1B28));
	_channels[2]->load(loadData(0x1B76));
	_channels[3]->load(loadData(0x1BBC));
	_channels[4]->load(loadData(0x1EC3));
	_channels[5]->load(loadData(0x1ED8));
	_channels[6]->load(loadData(0x1C16));
}

int ASound4::command38() {
	byte *pData = loadData(0x1AC8);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound4, loadCommand38));
		else
			loadCommand38();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command39 - Pattern B deferred music, timer=0x48, ASound::command1, ch0-5
// (isSoundActive check uses offset 0x1D29; channels loaded from 0x2176+)
// ---------------------------------------------------------------------------
void ASound4::loadCommand39() {
	resetCallbackTimer(0x48);
	ASound::command1();
	_channels[0]->load(loadData(0x2176));
	_channels[1]->load(loadData(0x21F3));
	_channels[2]->load(loadData(0x2269));
	_channels[3]->load(loadData(0x232B));
	_channels[4]->load(loadData(0x23CF));
	_channels[5]->load(loadData(0x24A1));
}

int ASound4::command39() {
	byte *pData = loadData(0x1D29);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound4, loadCommand39));
		else
			loadCommand39();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command40 - Pattern B deferred music, timer=0xC0, ASound::command1, ch0-6
// ---------------------------------------------------------------------------
void ASound4::loadCommand40() {
	resetCallbackTimer(0xC0);
	ASound::command1();
	_channels[0]->load(loadData(0x1F06));
	_channels[1]->load(loadData(0x1F68));
	_channels[2]->load(loadData(0x1FAB));
	_channels[3]->load(loadData(0x2010));
	_channels[4]->load(loadData(0x2106));
	_channels[5]->load(loadData(0x212A));
	_channels[6]->load(loadData(0x2159));
}

int ASound4::command40() {
	byte *pData = loadData(0x1F06);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound4, loadCommand40));
		else
			loadCommand40();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64-81 (off_11A54) - SFX (command73 = empty, 80-81 = stubs)
// ---------------------------------------------------------------------------
int ASound4::command64() { playSound(0x25B4); return 0; }
int ASound4::command65() { playSound(0x25F7); return 0; }
int ASound4::command66() { playSound(0x2601); return 0; }
int ASound4::command67() { playSound(0x260B); return 0; }
int ASound4::command68() { playSound(0x2629); return 0; }
int ASound4::command69() { playSound(0x25C0); return 0; }
int ASound4::command70() { playSound(0x2643); playSound(0x264F); return 0; }
int ASound4::command71() { playSound(0x265D); return 0; }
int ASound4::command72() { playSound(0x2667); return 0; }
int ASound4::command73() { return 0; }
int ASound4::command74() { playSound(0x2643); playSound(0x2643); return 0; }
int ASound4::command75() { ASound::command5(); playSound(0x2676); playSound(0x2683); return 0; }
int ASound4::command76() { playSound(0x2E47); playSound(0x2E5B); return 0; }
int ASound4::command77() { playSound(0x26AD); return 0; }
int ASound4::command78() { playSound(0x26BC); return 0; }
int ASound4::command80() { return 0; }
int ASound4::command81() { return 0; }

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound5  (asound.dr5)                                                  *
 *-----------------------------------------------------------------------*/

const ASound5::CommandPtr ASound5::_commandList[82] = {
	// commands 0-8 (off_11A14)
	&ASound5::command0,  &ASound5::command1,  &ASound5::command2,  &ASound5::command3,
	&ASound5::command4,  &ASound5::command5,  &ASound5::command6,  &ASound5::command7,
	&ASound5::command8,
	// 9-15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 16-19 (off_11A26; slot 19 = no-op)
	&ASound5::command16, &ASound5::command17, &ASound5::command18, nullptr,
	// 20-23 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24-31 (off_11A2E; slot 32 handled by funcs_11E05)
	&ASound5::command24, &ASound5::command25, &ASound5::command26, &ASound5::command27,
	&ASound5::command28, &ASound5::command29, &ASound5::command30, &ASound5::command31,
	// commands 32-39 (funcs_11E05; slot 39 = no-op)
	&ASound5::command32, &ASound5::command33, &ASound5::command34, &ASound5::command35,
	&ASound5::command36, &ASound5::command37, &ASound5::command38, nullptr,
	// 40-63 absent
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr,
	// commands 64-81 (off_11A50; slot 79 = no-op; 80-81 = stubs)
	&ASound5::command64, &ASound5::command65, &ASound5::command66, &ASound5::command67,
	&ASound5::command68, &ASound5::command69, &ASound5::command70, &ASound5::command71,
	&ASound5::command72, &ASound5::command73, &ASound5::command74, &ASound5::command75,
	&ASound5::command76, &ASound5::command77, &ASound5::command78, nullptr,
	&ASound5::command80, &ASound5::command81
};

ASound5::ASound5(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.dr5", 0x20d0, 0x2ee0) {
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
		_samples.push_back(AdlibSample(samplesStream));
	_lastParam = 0;
}

int ASound5::command(int commandId, int param) {
	_lastParam = param;
	if (commandId > 81 || !_commandList[commandId])
		return 0;
	if (commandId >= 32 && commandId < 64)
		setMusicIndex(commandId);
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
// command16 - Pattern B deferred music, timer=0xC0, ch0-6
// isSoundActive check uses 0x168A (not ch0's load offset 0x167C)
// ---------------------------------------------------------------------------
void ASound5::loadCommand16() {
	resetCallbackTimer(0xC0);
	setMusicIndex(0x10);
	ASound::command1();
	_channels[0]->load(loadData(0x167C));
	_channels[1]->load(loadData(0x17ED));
	_channels[2]->load(loadData(0x18A8));
	_channels[3]->load(loadData(0x1995));
	_channels[4]->load(loadData(0x1A4B));
	_channels[5]->load(loadData(0x1C77));
	_channels[6]->load(loadData(0x1CA0));
}

int ASound5::command16() {
	byte *pData = loadData(0x168A);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound5, loadCommand16));
		else
			loadCommand16();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command17 - isSoundActive guard, inline resetCallbackTimer(0x60), ch0-6
// (no deferred path; timer is reset and channels loaded directly)
// ---------------------------------------------------------------------------
int ASound5::command17() {
	byte *pData = loadData(0x28CC);
	if (!isSoundActive(pData)) {
		resetCallbackTimer(0x60);
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x2901));
		_channels[2]->load(loadData(0x2934));
		_channels[3]->load(loadData(0x294D));
		_channels[4]->load(loadData(0x2980));
		_channels[5]->load(loadData(0x298E));
		_channels[6]->load(loadData(0x29A6));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command18 - re-entry dispatcher: fade + re-launch last music piece
// ---------------------------------------------------------------------------
int ASound5::command18() {
	ASound::command1();
	int musicIndex = getMusicIndex();
	if (musicIndex <= 0x12)
		return command16();
	if (_commandList[musicIndex])
		return (this->*_commandList[musicIndex])();
	return 0;
}

// ---------------------------------------------------------------------------
// commands 24-31 (off_11A2E) - SFX
// ---------------------------------------------------------------------------
int ASound5::command24() { playSound(0x29DF); playSound(0x2A14); return 0; }
int ASound5::command25() { playSound(0x2A44); playSound(0x2A72); return 0; }
int ASound5::command26() { playSound(0x2AA2); return 0; }
int ASound5::command27() { playSound(0x2AAE); return 0; }
int ASound5::command28() { playSound(0x2BCD); return 0; }
int ASound5::command29() { playSound(0x2AE2); return 0; }
int ASound5::command30() { playSound(0x2B1A); return 0; }
int ASound5::command31() { *loadData(0x2B7E) = 0x67; playSound(0x2B7B); return 0; }

// ---------------------------------------------------------------------------
// command32 - Pattern B deferred music, timer=0x48, ch0-5
// ---------------------------------------------------------------------------
void ASound5::loadCommand32() {
	resetCallbackTimer(0x48);
	setMusicIndex(0x10);
	ASound::command1();
	_channels[0]->load(loadData(0x26C0));
	_channels[1]->load(loadData(0x2705));
	_channels[2]->load(loadData(0x2735));
	_channels[3]->load(loadData(0x276D));
	_channels[4]->load(loadData(0x27A1));
	_channels[5]->load(loadData(0x282E));
}

int ASound5::command32() {
	byte *pData = loadData(0x26C0);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound5, loadCommand32));
		else
			loadCommand32();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command33 - Pattern B deferred music, timer=0x60, ch0-5
// ---------------------------------------------------------------------------
void ASound5::loadCommand33() {
	resetCallbackTimer(0x60);
	setMusicIndex(0x10);
	ASound::command1();
	_channels[0]->load(loadData(0x1192));
	_channels[1]->load(loadData(0x1258));
	_channels[2]->load(loadData(0x1309));
	_channels[3]->load(loadData(0x1602));
	_channels[4]->load(loadData(0x1518));
	_channels[5]->load(loadData(0x161B));
}

int ASound5::command33() {
	byte *pData = loadData(0x1192);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound5, loadCommand33));
		else
			loadCommand33();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command34 - Pattern B deferred music, timer=0xC0, ch0-5
// ---------------------------------------------------------------------------
void ASound5::loadCommand34() {
	resetCallbackTimer(0xC0);
	ASound::command1();
	_channels[0]->load(loadData(0x1E44));
	_channels[1]->load(loadData(0x1EFA));
	_channels[2]->load(loadData(0x1FEE));
	_channels[3]->load(loadData(0x210E));
	_channels[4]->load(loadData(0x21CF));
	_channels[5]->load(loadData(0x2378));
}

int ASound5::command34() {
	byte *pData = loadData(0x1E44);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound5, loadCommand34));
		else
			loadCommand34();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command35 - Pattern B deferred music, timer=0xC0, ch0-5
// ---------------------------------------------------------------------------
void ASound5::loadCommand35() {
	resetCallbackTimer(0xC0);
	ASound::command1();
	_channels[0]->load(loadData(0x2402));
	_channels[1]->load(loadData(0x2414));
	_channels[2]->load(loadData(0x2446));
	_channels[3]->load(loadData(0x2494));
	_channels[4]->load(loadData(0x24B6));
	_channels[5]->load(loadData(0x24CE));
}

int ASound5::command35() {
	byte *pData = loadData(0x2402);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound5, loadCommand35));
		else
			loadCommand35();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// command36 - direct: setMusicIndex(0x29), playSound (no channel loads)
// ---------------------------------------------------------------------------
int ASound5::command36() {
	setMusicIndex(0x29);
	playSound(0x24F9);
	return 0;
}

// ---------------------------------------------------------------------------
// command37 - direct: command1 + findFreeChannel (lower pool) ×6
// ---------------------------------------------------------------------------
int ASound5::command37() {
	ASound::command1();
	findFreeChannel(loadData(0x1D68));
	findFreeChannel(loadData(0x1DA5));
	findFreeChannel(loadData(0x1DDD));
	findFreeChannel(loadData(0x1E16));
	findFreeChannel(loadData(0x1E2D));
	findFreeChannel(loadData(0x1E38));
	return 0;
}

// ---------------------------------------------------------------------------
// command38 - Pattern B deferred music, timer=0xC0, ch0-6
// ---------------------------------------------------------------------------
void ASound5::loadCommand38() {
	resetCallbackTimer(0xC0);
	setMusicIndex(0x26);
	ASound::command1();
	_channels[0]->load(loadData(0x1EA9));
	_channels[1]->load(loadData(0x1FB0));
	_channels[2]->load(loadData(0x204C));
	_channels[3]->load(loadData(0x21B7));
	_channels[4]->load(loadData(0x2262));
	_channels[5]->load(loadData(0x23CD));
	_channels[6]->load(loadData(0x23F6));
}

int ASound5::command38() {
	byte *pData = loadData(0x1EA9);
	if (!isSoundActive(pData)) {
		if (isMusicChannelsActive())
			scheduleCallback(MAKE_CALLBACK(ASound5, loadCommand38));
		else
			loadCommand38();
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64-78 (off_11A50) - SFX
// ---------------------------------------------------------------------------
int ASound5::command64() { playSound(0x252C); return 0; }
int ASound5::command65() { playSound(0x253B); playSound(0x254C); return 0; }
int ASound5::command66() { playSound(0x255E); return 0; }
int ASound5::command67() { playSound(0x2585); playSound(0x2572); return 0; }
int ASound5::command68() { playSound(0x25AD); playSound(0x25AB); return 0; }
int ASound5::command69() { playSound(0x25D4); return 0; }
int ASound5::command70() { playSound(0x25FF); playSound(0x25FF); return 0; }
int ASound5::command71() { playSound(0x260F); playSound(0x262A); return 0; }
int ASound5::command72() { playSound(0x2645); return 0; }
int ASound5::command73() { playSound(0x2656); return 0; }
int ASound5::command74() { playSound(0x266A); playSound(0x267E); return 0; }
int ASound5::command75() { playSound(0x2692); return 0; }
int ASound5::command76() { playSound(0x26A3); playSound(0x26AF); return 0; }

// ---------------------------------------------------------------------------
// command77 - param-based: param=0 writes loop counters at DS:0x20/0x22;
// param≠0 sets velocity byte and conditionally plays sound 0x2B40.
// _lastParam is stored by command() dispatcher before the near call.
// ---------------------------------------------------------------------------
int ASound5::command77() {
	if (_lastParam == 0) {
		if (!isSoundActive(loadData(0x2B40))) {
			*(uint16 *)loadData(0x22) = 1;
			*(uint16 *)loadData(0x20) = 1;
		}
	} else {
		*loadData(0x2B4B) = _lastParam & 0x7F;
		if (!isSoundActive(loadData(0x2B40)))
			playSound(0x2B40);
	}
	return 0;
}

int ASound5::command78() { *loadData(0x2B7E) = 0x7F; playSound(0x2B7B); return 0; }
int ASound5::command80() { return 0; }
int ASound5::command81() { return 0; }

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound6  (asound.dr6)                                                  *
 *-----------------------------------------------------------------------*/

const ASound6::CommandPtr ASound6::_commandList[102] = {
	// commands 0-8  (off_11A14)
	&ASound6::command0,  &ASound6::command1,  &ASound6::command2,  &ASound6::command3,
	&ASound6::command4,  &ASound6::command5,  &ASound6::command6,  &ASound6::command7,
	&ASound6::command8,
	// 9-15 absent
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	// commands 16-19 (off_11A26; slot 19 = no-op)
	&ASound6::command16, &ASound6::command17, &ASound6::command18, nullptr,
	// 20-23 absent
	nullptr, nullptr, nullptr, nullptr,
	// commands 24-31 (off_11A2E)
	&ASound6::command24, &ASound6::command25, &ASound6::command26, &ASound6::command27,
	&ASound6::command28, &ASound6::command29, &ASound6::command30, &ASound6::command31,
	// commands 32-47 (funcs_1204D; slots 41-43 and 46-47 = no-op)
	&ASound6::command32, &ASound6::command33, &ASound6::command34, &ASound6::command35,
	&ASound6::command36, &ASound6::command37, &ASound6::command38, &ASound6::command39,
	&ASound6::command40, nullptr, nullptr, nullptr,
	&ASound6::command44, &ASound6::command45, nullptr, nullptr,
	// 48-63 absent
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	// commands 64-101 (off_11A60; slot 99 = no-op)
	&ASound6::command64, &ASound6::command65, &ASound6::command66, &ASound6::command67,
	&ASound6::command68, &ASound6::command69, &ASound6::command70, &ASound6::command71,
	&ASound6::command72, &ASound6::command73, &ASound6::command74, &ASound6::command75,
	&ASound6::command76, &ASound6::command77, &ASound6::command78, &ASound6::command79,
	&ASound6::command80, &ASound6::command81, &ASound6::command82, &ASound6::command83,
	&ASound6::command84, &ASound6::command85, &ASound6::command86, &ASound6::command87,
	&ASound6::command88, &ASound6::command89, &ASound6::command90, &ASound6::command91,
	&ASound6::command92, &ASound6::command93, &ASound6::command94, &ASound6::command95,
	&ASound6::command96, &ASound6::command97, &ASound6::command98, nullptr,
	&ASound6::command100, &ASound6::command101
};

ASound6::ASound6(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.dr6", 0x2370, 0x3870) {
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
		_samples.push_back(AdlibSample(samplesStream));
	_cmd33Flag = 0;
	_lastParam = 0;
}

int ASound6::command(int commandId, int param) {
	_lastParam = param;
	if (commandId > 101 || !_commandList[commandId])
		return 0;
	if (commandId >= 32 && commandId < 64)
		setMusicIndex(commandId);
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base
int ASound6::command0() { return ASound::command0(); }
int ASound6::command1() { return ASound::command1(); }
int ASound6::command2() { return ASound::command2(); }
int ASound6::command3() { return ASound::command3(); }
int ASound6::command4() { return ASound::command4(); }
int ASound6::command5() { return ASound::command5(); }
int ASound6::command6() { return ASound::command6(); }
int ASound6::command7() { return ASound::command7(); }
int ASound6::command8() { return ASound::command8(); }

// command16 — Pattern B with 5 isSoundActive guards; setMusicIndex(0x10)
void ASound6::loadCommand16() {
	resetCallbackTimer(0xC8);
	setMusicIndex(0x10);
	ASound::command1();
	_channels[0]->load(loadData(0x2AA0));
	_channels[1]->load(loadData(0x2B2B));
	_channels[2]->load(loadData(0x2B70));
	_channels[3]->load(loadData(0x2BC9));
	_channels[4]->load(loadData(0x2BDA));
	_channels[5]->load(loadData(0x2BED));
}
int ASound6::command16() {
	if (isSoundActive(loadData(0x1542))) return 0;
	if (isSoundActive(loadData(0x16F4))) return 0;
	if (isSoundActive(loadData(0x1A10))) return 0;
	if (isSoundActive(loadData(0x1E44))) return 0;
	if (isSoundActive(loadData(0x2AA0))) return 0;
	if (!isMusicChannelsActive())
		loadCommand16();
	else
		scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand16));
	return 0;
}

// command17 — inline (non-deferred): resetCallbackTimer(0x60), ch0-ch6
int ASound6::command17() {
	byte *pData = loadData(0x3166);
	if (!isSoundActive(pData)) {
		resetCallbackTimer(0x60);
		ASound::command1();
		_channels[0]->load(pData);
		_channels[1]->load(loadData(0x319B));
		_channels[2]->load(loadData(0x31CE));
		_channels[3]->load(loadData(0x31E7));
		_channels[4]->load(loadData(0x321A));
		_channels[5]->load(loadData(0x3228));
		_channels[6]->load(loadData(0x3240));
	}
	return 0;
}

// command18 — re-entry dispatcher: command1() + musicIndex dispatch
int ASound6::command18() {
	ASound::command1();
	int musicIndex = getMusicIndex();
	if (musicIndex <= 0x12)
		return command16();
	if (_commandList[musicIndex])
		return (this->*_commandList[musicIndex])();
	return 0;
}

// commands 24-31 — SFX via playSound (loc_103DC = findFreeChannelFull)
int ASound6::command24() { playSound(0x3279); playSound(0x32AE); return 0; }
int ASound6::command25() { playSound(0x32DE); playSound(0x330C); return 0; }
int ASound6::command26() { playSound(0x333C); return 0; }
int ASound6::command27() { playSound(0x3348); return 0; }
int ASound6::command28() { playSound(0x3354); return 0; }
int ASound6::command29() { playSound(0x337C); return 0; }
int ASound6::command30() { playSound(0x33B4); return 0; }
int ASound6::command31() { *loadData(0x3418) = 0x67; playSound(0x3415); return 0; }

// command32 — complex: optionally modifies active channel; Pattern B for ch0-ch5
// Shared deferred loader with command33 (loc_11B29)
void ASound6::loadCommand32() {
	resetCallbackTimer(0x3C);
	ASound::command1();
	_channels[0]->load(loadData(0x1192));
	_channels[1]->load(loadData(0x11D8));
	_channels[2]->load(loadData(0x122C));
	_channels[3]->load(loadData(0x1255));
	_channels[4]->load(loadData(0x1317));
	_channels[5]->load(loadData(0x134D));
	if (_cmd33Flag == 0xFF) {
		_cmd33Flag = 0;
		_channels[6]->load(loadData(0x12F4));
	}
}
int ASound6::command32() {
	// If sound 0x12F4 is already active, set pending-stop and redirect its data
	byte *p12F4 = loadData(0x12F4);
	for (int ch = 0; ch < 9; ++ch) {
		if (_channels[ch]->_activeCount && _channels[ch]->_soundData == p12F4) {
			_channels[ch]->_pendingStop = 0xFF;
			_channels[ch]->_soundData = loadData(0x3252);
			break;
		}
	}
	if (!isSoundActive(loadData(0x1192))) {
		if (!isMusicChannelsActive())
			loadCommand32();
		else
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand32));
	}
	return 0;
}

// command33 — sets _cmd33Flag=0xFF then shares loadCommand32 loader
int ASound6::command33() {
	if (!isSoundActive(loadData(0x12F4))) {
		_cmd33Flag = 0xFF;
		if (!isMusicChannelsActive()) {
			loadCommand32();
		} else if (isSoundActive(loadData(0x1192))) {
			// loc_11B69: sound 0x1192 already running — only load ch6
			_cmd33Flag = 0;
			_channels[6]->load(loadData(0x12F4));
		} else {
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand32));
		}
	}
	return 0;
}

// command34 — Pattern B with shortcut: if ch0 already playing 0x1362, load extra only
void ASound6::loadCommand34() {
	resetCallbackTimer(0x1E);
	ASound::command1();
	_channels[0]->load(loadData(0x1362));
	_channels[4]->load(loadData(0x14F1));
	_channels[1]->load(loadData(0x13C5));
	_channels[5]->load(loadData(0x1510));
	_channels[3]->load(loadData(0x14A7));
	_channels[6]->load(loadData(0x152A));
	_channels[2]->load(loadData(0x1418));
}
int ASound6::command34() {
	if (!isSoundActive(loadData(0x13C5))) {
		if (_channels[0]->_loopStartPtr == loadData(0x1362) && _channels[0]->_activeCount != 0) {
			// Shortcut: ch0 already running 0x1362 — only load extra channels (loc_11C15)
			_channels[1]->load(loadData(0x13C5));
			_channels[5]->load(loadData(0x1510));
			_channels[3]->load(loadData(0x14A7));
			_channels[6]->load(loadData(0x152A));
			_channels[2]->load(loadData(0x1418));
		} else if (!isMusicChannelsActive()) {
			loadCommand34();
		} else {
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand34));
		}
	}
	return 0;
}

// command35 — Pattern B, timer=0x1E, ch0+ch4 only
void ASound6::loadCommand35() {
	resetCallbackTimer(0x1E);
	ASound::command1();
	_channels[0]->load(loadData(0x1362));
	_channels[4]->load(loadData(0x14F1));
}
int ASound6::command35() {
	if (!isSoundActive(loadData(0x1362))) {
		if (!isMusicChannelsActive())
			loadCommand35();
		else
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand35));
	}
	return 0;
}

// command36 — triple guard (0x16F4, 0x1A10, 0x1E44); Pattern B; ch0-ch5
void ASound6::loadCommand36() {
	resetCallbackTimer(0xC8);
	ASound::command1();
	_channels[0]->load(loadData(0x1542));
	_channels[1]->load(loadData(0x1594));
	_channels[2]->load(loadData(0x162A));
	_channels[3]->load(loadData(0x16A5));
	_channels[4]->load(loadData(0x16BF));
	_channels[5]->load(loadData(0x16D9));
}
int ASound6::command36() {
	if (isSoundActive(loadData(0x16F4))) return 0;
	if (isSoundActive(loadData(0x1A10))) return 0;
	if (isSoundActive(loadData(0x1E44))) return 0;
	if (!isMusicChannelsActive())
		loadCommand36();
	else
		scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand36));
	return 0;
}

// command37 — triple guard (0x1A10, 0x1E44, 0x16F4); Pattern B; ch0-ch5
void ASound6::loadCommand37() {
	resetCallbackTimer(0xC8);
	ASound::command1();
	_channels[0]->load(loadData(0x16F4));
	_channels[1]->load(loadData(0x1757));
	_channels[2]->load(loadData(0x17BF));
	_channels[3]->load(loadData(0x18C0));
	_channels[4]->load(loadData(0x19C7));
	_channels[5]->load(loadData(0x19F7));
}
int ASound6::command37() {
	if (isSoundActive(loadData(0x1A10))) return 0;
	if (isSoundActive(loadData(0x1E44))) return 0;
	if (isSoundActive(loadData(0x16F4))) return 0;
	if (!isMusicChannelsActive())
		loadCommand37();
	else
		scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand37));
	return 0;
}

// command38 — single guard (0x1A10); Pattern B; ch0-ch6
void ASound6::loadCommand38() {
	resetCallbackTimer(0xC8);
	ASound::command1();
	_channels[0]->load(loadData(0x1A10));
	_channels[1]->load(loadData(0x1A76));
	_channels[2]->load(loadData(0x1ADC));
	_channels[3]->load(loadData(0x1B6C));
	_channels[4]->load(loadData(0x1C29));
	_channels[5]->load(loadData(0x1E14));
	_channels[6]->load(loadData(0x1E2C));
}
int ASound6::command38() {
	if (!isSoundActive(loadData(0x1A10))) {
		if (!isMusicChannelsActive())
			loadCommand38();
		else
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand38));
	}
	return 0;
}

// command39 — single guard (0x1E44); Pattern B; ch0-ch6
void ASound6::loadCommand39() {
	resetCallbackTimer(0xC8);
	ASound::command1();
	_channels[0]->load(loadData(0x1E44));
	_channels[1]->load(loadData(0x1FA2));
	_channels[2]->load(loadData(0x21AD));
	_channels[3]->load(loadData(0x2249));
	_channels[4]->load(loadData(0x2298));
	_channels[5]->load(loadData(0x23D0));
	_channels[6]->load(loadData(0x2461));
}
int ASound6::command39() {
	if (!isSoundActive(loadData(0x1E44))) {
		if (!isMusicChannelsActive())
			loadCommand39();
		else
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand39));
	}
	return 0;
}

// command40 — single guard (0x2476); Pattern B; ch0-ch6
void ASound6::loadCommand40() {
	resetCallbackTimer(0xC8);
	ASound::command1();
	_channels[0]->load(loadData(0x2476));
	_channels[1]->load(loadData(0x259C));
	_channels[2]->load(loadData(0x2691));
	_channels[3]->load(loadData(0x2786));
	_channels[4]->load(loadData(0x28C1));
	_channels[5]->load(loadData(0x29CB));
	_channels[6]->load(loadData(0x2A7A));
}
int ASound6::command40() {
	if (!isSoundActive(loadData(0x2476))) {
		if (!isMusicChannelsActive())
			loadCommand40();
		else
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand40));
	}
	return 0;
}

// command44 — Pattern B; setMusicIndex(0x25); all 9 channels; timer=0x30
void ASound6::loadCommand44() {
	resetCallbackTimer(0x30);
	setMusicIndex(0x25);
	ASound::command1();
	_channels[0]->load(loadData(0x2C06));
	_channels[1]->load(loadData(0x2C4D));
	_channels[2]->load(loadData(0x2C71));
	_channels[3]->load(loadData(0x2CA0));
	_channels[4]->load(loadData(0x2CD1));
	_channels[5]->load(loadData(0x2D7D));
	_channels[6]->load(loadData(0x2D88));
	_channels[7]->load(loadData(0x2D95));
	_channels[8]->load(loadData(0x2D2E));
}
int ASound6::command44() {
	if (!isSoundActive(loadData(0x2C06))) {
		if (!isMusicChannelsActive())
			loadCommand44();
		else
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand44));
	}
	return 0;
}

// command45 — Pattern B; timer=0x1E; 3×findFreeChannel (lower pool, sub_10388)
void ASound6::loadCommand45() {
	resetCallbackTimer(0x1E);
	ASound::command1();
	findFreeChannel(loadData(0x2DB0));
	findFreeChannel(loadData(0x2DE7));
	findFreeChannel(loadData(0x2DF2));
}
int ASound6::command45() {
	if (!isSoundActive(loadData(0x2DB0))) {
		if (!isMusicChannelsActive())
			loadCommand45();
		else
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand45));
	}
	return 0;
}

// command64 — param-based: param=0 sets global flags; param!=0 sets pitch byte and plays sfx
int ASound6::command64() {
	if (_lastParam == 0) {
		if (!isSoundActive(loadData(0x33DA))) {
			*(uint16 *)loadData(0x22) = 1;
			*(uint16 *)loadData(0x20) = 1;
		}
	} else {
		*loadData(0x33E5) = _lastParam & 0x7F;
		if (!isSoundActive(loadData(0x33DA)))
			playSound(0x33DA);
	}
	return 0;
}

// commands 65-98 — SFX
int ASound6::command65() { playSound(0x30F6); return 0; }
int ASound6::command66() { playSound(0x2DFE); return 0; }
int ASound6::command67() { playSound(0x3104); playSound(0x3128); return 0; }
int ASound6::command68() { playSound(0x2E10); return 0; }
int ASound6::command69() { findFreeChannel(loadData(0x2E13)); return 0; }
int ASound6::command70() { playSound(0x2E29); return 0; }
int ASound6::command71() { playSound(0x2E41); return 0; }
int ASound6::command72() { playSound(0x2E51); playSound(0x2E76); return 0; }
int ASound6::command73() { playSound(0x2ED9); return 0; }
int ASound6::command74() { playSound(0x2EFF); return 0; }
int ASound6::command75() { playSound(0x2F0B); playSound(0x2F17); return 0; }
int ASound6::command76() { playSound(0x2F26); return 0; }
int ASound6::command77() { playSound(0x2F3E); return 0; }
int ASound6::command78() { playSound(0x2F63); return 0; }
int ASound6::command79() { playSound(0x2F75); return 0; }

// command80 — if sound 0x2F75 is active, set its _outerLoopCount to 1
int ASound6::command80() {
	byte *pData = loadData(0x2F75);
	for (int ch = 0; ch < 9; ++ch) {
		if (_channels[ch]->_activeCount && _channels[ch]->_soundData == pData) {
			_channels[ch]->_outerLoopCount = 1;
			break;
		}
	}
	return 0;
}

int ASound6::command81() { playSound(0x2F85); playSound(0x2F90); return 0; }
int ASound6::command82() { playSound(0x2FAE); return 0; }
int ASound6::command83() { playSound(0x2E9D); playSound(0x2EBA); return 0; }
int ASound6::command84() { playSound(0x2FB8); return 0; }
int ASound6::command85() { playSound(0x2FC5); return 0; }
int ASound6::command86() { playSound(0x2FCF); playSound(0x2FDA); return 0; }
int ASound6::command87() { playSound(0x2FF8); return 0; }
int ASound6::command88() { playSound(0x300C); return 0; }
int ASound6::command89() { playSound(0x3027); return 0; }
int ASound6::command90() { playSound(0x3041); playSound(0x3053); return 0; }
int ASound6::command91() { playSound(0x3062); return 0; }
int ASound6::command92() { _channels[8]->load(loadData(0x3074)); return 0; }
int ASound6::command93() { findFreeChannel(loadData(0x3088)); return 0; }
int ASound6::command94() { findFreeChannel(loadData(0x3094)); return 0; }
int ASound6::command95() { findFreeChannel(loadData(0x30A3)); return 0; }

// command96 — Pattern B; timer=0x5A; ch0-ch6
void ASound6::loadCommand96() {
	resetCallbackTimer(0x5A);
	ASound::command1();
	_channels[0]->load(loadData(0x3482));
	_channels[1]->load(loadData(0x34C8));
	_channels[2]->load(loadData(0x350E));
	_channels[3]->load(loadData(0x3554));
	_channels[4]->load(loadData(0x34B3));
	_channels[5]->load(loadData(0x34F9));
	_channels[6]->load(loadData(0x353F));
}
int ASound6::command96() {
	if (!isSoundActive(loadData(0x3482))) {
		if (!isMusicChannelsActive())
			loadCommand96();
		else
			scheduleCallback(MAKE_CALLBACK(ASound6, loadCommand96));
	}
	return 0;
}

int ASound6::command97() { playSound(0x30CA); playSound(0x30E0); return 0; }
int ASound6::command98() { playSound(0x314C); playSound(0x314E); return 0; }
int ASound6::command100() { return 0; }
int ASound6::command101() { return 0; }

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

/*-----------------------------------------------------------------------*/
/* ASoundDemo1  (asound.dr1 [demo])                                       *
 *-----------------------------------------------------------------------*/

const ASoundDemo1::CommandPtr ASoundDemo1::_commandList[93] = {
	// commands 0-8  (commands0)
	&ASoundDemo1::command0,  &ASoundDemo1::command1,  &ASoundDemo1::command2,  &ASoundDemo1::command3,
	&ASoundDemo1::command4,  &ASoundDemo1::command5,  &ASoundDemo1::command6,  &ASoundDemo1::command7,
	&ASoundDemo1::command8,
	// 9-15 absent
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	// command 16  (commands16)
	&ASoundDemo1::command16,
	// 17-23 absent
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	// commands 24-27, 28-29 = no-op, commands 30-43, 44 = no-op  (commands24)
	&ASoundDemo1::command24, &ASoundDemo1::command25, &ASoundDemo1::command26, &ASoundDemo1::command27,
	nullptr,                 nullptr,
	&ASoundDemo1::command30, &ASoundDemo1::command31, &ASoundDemo1::command32, &ASoundDemo1::command33,
	&ASoundDemo1::command34, &ASoundDemo1::command35, &ASoundDemo1::command36, &ASoundDemo1::command37,
	&ASoundDemo1::command38, &ASoundDemo1::command39, &ASoundDemo1::command40, &ASoundDemo1::command41,
	&ASoundDemo1::command42, &ASoundDemo1::command43,
	nullptr,
	// 45-63 absent
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr,
	// commands 64-89 = no-op, commands 90-92  (commands64)
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr,
	&ASoundDemo1::command90, &ASoundDemo1::command91, &ASoundDemo1::command92
};

ASoundDemo1::ASoundDemo1(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.dr1", 0x23e0, 0x4900) {
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASoundDemo1::command(int commandId, int param) {
	if (commandId > 92 || !_commandList[commandId])
		return 0;
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASoundDemo1::command0() { return ASound::command0(); }
int ASoundDemo1::command1() { return ASound::command1(); }
int ASoundDemo1::command2() { return ASound::command2(); }
int ASoundDemo1::command3() { return ASound::command3(); }
int ASoundDemo1::command4() { return ASound::command4(); }
int ASoundDemo1::command5() { return ASound::command5(); }
int ASoundDemo1::command6() { return ASound::command6(); }
int ASoundDemo1::command7() { return ASound::command7(); }
int ASoundDemo1::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// command16 - no isSoundActive guard; if music channels are busy, defer via
// callback, otherwise load immediately. Loads channels 0-6.
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand16() {
	resetCallbackTimer(0x90);
	ASound::command1();
	_channels[0]->load(loadData(0x2BC6));
	_channels[1]->load(loadData(0x2C74));
	_channels[2]->load(loadData(0x2CEC));
	_channels[3]->load(loadData(0x2D68));
	_channels[4]->load(loadData(0x2DC0));
	_channels[5]->load(loadData(0x2DE7));
	_channels[6]->load(loadData(0x2E07));
}

int ASoundDemo1::command16() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand16));
	else
		loadCommand16();
	return 0;
}

// ---------------------------------------------------------------------------
// commands 25-27 - sound effects via findFreeChannelFull
// ---------------------------------------------------------------------------
int ASoundDemo1::command25() {
	findFreeChannelFull(loadData(0x4575));
	findFreeChannelFull(loadData(0x45A1));
	return 0;
}

int ASoundDemo1::command26() {
	findFreeChannelFull(loadData(0x45CF));
	return 0;
}

int ASoundDemo1::command27() {
	findFreeChannelFull(loadData(0x45DB));
	return 0;
}

// ---------------------------------------------------------------------------
// command30 - isMusicChannelsActive guard, deferred callback, load ch0-5
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand30() {
	resetCallbackTimer(0xB0);
	ASound::command1();
	_channels[0]->load(loadData(0x2F2A));
	_channels[1]->load(loadData(0x2F73));
	_channels[2]->load(loadData(0x2FD5));
	_channels[3]->load(loadData(0x2FFF));
	_channels[4]->load(loadData(0x302F));
	_channels[5]->load(loadData(0x303E));
}

int ASoundDemo1::command30() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand30));
	else
		loadCommand30();
	return 0;
}

// ---------------------------------------------------------------------------
// command31 - isMusicChannelsActive guard, deferred callback, load ch0-5.
// Note: command1() is called before resetCallbackTimer here (reversed order
// vs command30/command16).
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand31() {
	ASound::command1();
	resetCallbackTimer(0xB0);
	_channels[0]->load(loadData(0x304E));
	_channels[1]->load(loadData(0x30F4));
	_channels[2]->load(loadData(0x3156));
	_channels[3]->load(loadData(0x3191));
	_channels[4]->load(loadData(0x3210));
	_channels[5]->load(loadData(0x3228));
}

int ASoundDemo1::command31() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand31));
	else
		loadCommand31();
	return 0;
}

// ---------------------------------------------------------------------------
// command32 - isMusicChannelsActive guard, deferred callback, load ch0-5
// (channels 3-5 load from non-sequential data offsets)
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand32() {
	resetCallbackTimer(0x50);
	ASound::command1();
	_channels[0]->load(loadData(0x32AC));
	_channels[1]->load(loadData(0x32FE));
	_channels[2]->load(loadData(0x3387));
	_channels[3]->load(loadData(0x32EF));
	_channels[4]->load(loadData(0x3371));
	_channels[5]->load(loadData(0x3448));
}

int ASoundDemo1::command32() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand32));
	else
		loadCommand32();
	return 0;
}

// ---------------------------------------------------------------------------
// command33 - isMusicChannelsActive guard, deferred callback, load ch0-5
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand33() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x3A7E));
	_channels[1]->load(loadData(0x3B16));
	_channels[2]->load(loadData(0x3B9C));
	_channels[3]->load(loadData(0x3BE7));
	_channels[4]->load(loadData(0x3C1E));
	_channels[5]->load(loadData(0x3C34));
}

int ASoundDemo1::command33() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand33));
	else
		loadCommand33();
	return 0;
}

// ---------------------------------------------------------------------------
// command34 - isMusicChannelsActive guard, deferred callback, load ch0-5
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand34() {
	resetCallbackTimer(0x80);
	ASound::command1();
	_channels[0]->load(loadData(0x40FE));
	_channels[1]->load(loadData(0x41C2));
	_channels[2]->load(loadData(0x42B1));
	_channels[3]->load(loadData(0x4345));
	_channels[4]->load(loadData(0x43B5));
	_channels[5]->load(loadData(0x43C4));
}

int ASoundDemo1::command34() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand34));
	else
		loadCommand34();
	return 0;
}

// ---------------------------------------------------------------------------
// command35 - isMusicChannelsActive guard, deferred callback, load ch0-5
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand35() {
	resetCallbackTimer(0xC0);
	ASound::command1();
	_channels[0]->load(loadData(0x43E8));
	_channels[1]->load(loadData(0x444B));
	_channels[2]->load(loadData(0x44B6));
	_channels[3]->load(loadData(0x44C5));
	_channels[4]->load(loadData(0x44D6));
	_channels[5]->load(loadData(0x44E0));
}

int ASoundDemo1::command35() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand35));
	else
		loadCommand35();
	return 0;
}

// ---------------------------------------------------------------------------
// command36 - isMusicChannelsActive guard, deferred callback, load ch0-5.
// Note: command1() is called before resetCallbackTimer here (reversed order
// vs command33-35).
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand36() {
	ASound::command1();
	resetCallbackTimer(0x60);
	_channels[0]->load(loadData(0x1906));
	_channels[1]->load(loadData(0x19ED));
	_channels[2]->load(loadData(0x1AC8));
	_channels[3]->load(loadData(0x1BF8));
	_channels[4]->load(loadData(0x1CE4));
	_channels[5]->load(loadData(0x1CFC));
}

int ASoundDemo1::command36() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand36));
	else
		loadCommand36();
	return 0;
}

// ---------------------------------------------------------------------------
// command37 - isMusicChannelsActive guard, deferred callback, load ch0-5
// (command1() before resetCallbackTimer, as in command36)
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand37() {
	ASound::command1();
	resetCallbackTimer(0xB0);
	_channels[0]->load(loadData(0x1D40));
	_channels[1]->load(loadData(0x1D95));
	_channels[2]->load(loadData(0x1DF7));
	_channels[3]->load(loadData(0x1E51));
	_channels[4]->load(loadData(0x1E60));
	_channels[5]->load(loadData(0x1E6F));
}

int ASoundDemo1::command37() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand37));
	else
		loadCommand37();
	return 0;
}

// ---------------------------------------------------------------------------
// command38 - isMusicChannelsActive guard, deferred callback, load ch0-5
// (resetCallbackTimer before command1(), as in command33-35)
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand38() {
	resetCallbackTimer(0xA8);
	ASound::command1();
	_channels[0]->load(loadData(0x1E7E));
	_channels[1]->load(loadData(0x1FF7));
	_channels[2]->load(loadData(0x21AD));
	_channels[3]->load(loadData(0x2476));
	_channels[4]->load(loadData(0x269E));
	_channels[5]->load(loadData(0x293B));
}

int ASoundDemo1::command38() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand38));
	else
		loadCommand38();
	return 0;
}

// ---------------------------------------------------------------------------
// command39 - isMusicChannelsActive guard, deferred callback, load ch0-5
// (channels 2-5 load from non-sequential data offsets)
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand39() {
	resetCallbackTimer(0x90);
	ASound::command1();
	_channels[0]->load(loadData(0x2972));
	_channels[1]->load(loadData(0x29DF));
	_channels[2]->load(loadData(0x2BB7));
	_channels[3]->load(loadData(0x2A56));
	_channels[4]->load(loadData(0x2B36));
	_channels[5]->load(loadData(0x2B8A));
}

int ASoundDemo1::command39() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand39));
	else
		loadCommand39();
	return 0;
}

// ---------------------------------------------------------------------------
// command40 - isMusicChannelsActive guard, deferred callback, load ch0-6
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand40() {
	resetCallbackTimer(0x90);
	ASound::command1();
	_channels[0]->load(loadData(0x17D6));
	_channels[1]->load(loadData(0x180C));
	_channels[2]->load(loadData(0x183F));
	_channels[3]->load(loadData(0x1880));
	_channels[4]->load(loadData(0x18B2));
	_channels[5]->load(loadData(0x18CD));
	_channels[6]->load(loadData(0x18E3));
}

int ASoundDemo1::command40() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand40));
	else
		loadCommand40();
	return 0;
}

// ---------------------------------------------------------------------------
// command41 - isMusicChannelsActive guard, deferred callback, load ch0-5
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand41() {
	resetCallbackTimer(0x50);
	ASound::command1();
	_channels[0]->load(loadData(0x3C44));
	_channels[1]->load(loadData(0x3CB1));
	_channels[2]->load(loadData(0x3CCD));
	_channels[3]->load(loadData(0x3CE9));
	_channels[4]->load(loadData(0x3CF6));
	_channels[5]->load(loadData(0x3D03));
}

int ASoundDemo1::command41() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand41));
	else
		loadCommand41();
	return 0;
}

// ---------------------------------------------------------------------------
// command42 - isMusicChannelsActive guard, deferred callback, load ch0-5.
// Uses resetCallbackTimerEx: counter=0x60, period=0xE0 (asymmetric).
// (channels 2 and 5 load from non-sequential data offsets)
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand42() {
	resetCallbackTimerEx(0x60, 0xE0);
	ASound::command1();
	_channels[0]->load(loadData(0x3D10));
	_channels[1]->load(loadData(0x3D66));
	_channels[2]->load(loadData(0x40EE));
	_channels[3]->load(loadData(0x3DED));
	_channels[4]->load(loadData(0x3F7C));
	_channels[5]->load(loadData(0x4053));
}

int ASoundDemo1::command42() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand42));
	else
		loadCommand42();
	return 0;
}

// ---------------------------------------------------------------------------
// command43 - isMusicChannelsActive guard, deferred callback, load ch0-5
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand43() {
	resetCallbackTimer(0x60);
	ASound::command1();
	_channels[0]->load(loadData(0x347A));
	_channels[1]->load(loadData(0x35BB));
	_channels[2]->load(loadData(0x36A8));
	_channels[3]->load(loadData(0x37B5));
	_channels[4]->load(loadData(0x38A5));
	_channels[5]->load(loadData(0x3A4D));
}

int ASoundDemo1::command43() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand43));
	else
		loadCommand43();
	return 0;
}

// ---------------------------------------------------------------------------
// command24 - sound effects via findFreeChannelFull
// ---------------------------------------------------------------------------
int ASoundDemo1::command24() {
	findFreeChannelFull(loadData(0x4514));
	findFreeChannelFull(loadData(0x4547));
	return 0;
}

// ---------------------------------------------------------------------------
// commands 90-91 - sound effects via findFreeChannelFull
// ---------------------------------------------------------------------------
int ASoundDemo1::command90() {
	findFreeChannelFull(loadData(0x3236));
	findFreeChannelFull(loadData(0x3260));
	return 0;
}

int ASoundDemo1::command91() {
	findFreeChannelFull(loadData(0x326B));
	findFreeChannelFull(loadData(0x32A1));
	return 0;
}

// ---------------------------------------------------------------------------
// command92 - isMusicChannelsActive guard, deferred callback, load ch0-5
// ---------------------------------------------------------------------------
void ASoundDemo1::loadCommand92() {
	resetCallbackTimer(0x54);
	ASound::command1();
	_channels[0]->load(loadData(0x2E16));
	_channels[1]->load(loadData(0x2E62));
	_channels[2]->load(loadData(0x2EA9));
	_channels[3]->load(loadData(0x2ED2));
	_channels[4]->load(loadData(0x2EF5));
	_channels[5]->load(loadData(0x2F1C));
}

int ASoundDemo1::command92() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo1, loadCommand92));
	else
		loadCommand92();
	return 0;
}

/*-----------------------------------------------------------------------*/
/* ASoundDemo9  (asound.dr9 [demo])                                       *
 *-----------------------------------------------------------------------*/

const ASoundDemo9::CommandPtr ASoundDemo9::_commandList[51] = {
	// commands 0-8  (commands0)
	&ASoundDemo9::command0,  &ASoundDemo9::command1,  &ASoundDemo9::command2,  &ASoundDemo9::command3,
	&ASoundDemo9::command4,  &ASoundDemo9::command5,  &ASoundDemo9::command6,  &ASoundDemo9::command7,
	&ASoundDemo9::command8,
	// 9-15 absent
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	// command 16  (commands16_24)
	&ASoundDemo9::command16,
	// 17-23 absent
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	// command 24 = shared with command16
	&ASoundDemo9::command16,
	// commands 25-29  (commands24)
	&ASoundDemo9::command25, &ASoundDemo9::command26, &ASoundDemo9::command27,
	&ASoundDemo9::command28_32, &ASoundDemo9::command29_33,
	// 30-31 absent
	nullptr, nullptr,
	// commands 32-33 = shared with commands 28/29
	&ASoundDemo9::command28_32, &ASoundDemo9::command29_33,
	// commands 34-50
	&ASoundDemo9::command34, &ASoundDemo9::command35, &ASoundDemo9::command36, &ASoundDemo9::command37,
	&ASoundDemo9::command38, &ASoundDemo9::command39, &ASoundDemo9::command40, &ASoundDemo9::command41,
	&ASoundDemo9::command42, &ASoundDemo9::command43, &ASoundDemo9::command44, &ASoundDemo9::command45,
	&ASoundDemo9::command46, &ASoundDemo9::command47, &ASoundDemo9::command48, &ASoundDemo9::command49,
	&ASoundDemo9::command50
};

ASoundDemo9::ASoundDemo9(Audio::Mixer *mixer, OPL::OPL *opl)
		: ASound(mixer, opl, "asound.dr9", 0x23a0, 0x62b0) {
	auto samplesStream = getDataStream(0x1dc);
	for (int i = 0; i < 182; ++i)
		_samples.push_back(AdlibSample(samplesStream));
}

int ASoundDemo9::command(int commandId, int param) {
	if (commandId > 50 || !_commandList[commandId])
		return 0;
	return (this->*_commandList[commandId])();
}

// commands 0-8: delegate to base ASound
int ASoundDemo9::command0() { return ASound::command0(); }
int ASoundDemo9::command1() { return ASound::command1(); }
int ASoundDemo9::command2() { return ASound::command2(); }
int ASoundDemo9::command3() { return ASound::command3(); }
int ASoundDemo9::command4() { return ASound::command4(); }
int ASoundDemo9::command5() { return ASound::command5(); }
int ASoundDemo9::command6() { return ASound::command6(); }
int ASoundDemo9::command7() { return ASound::command7(); }
int ASoundDemo9::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// commands 16, 25-27 - sound effects via findFreeChannelFull
// ---------------------------------------------------------------------------
int ASoundDemo9::command16() {
	findFreeChannelFull(loadData(0x5EC6));
	findFreeChannelFull(loadData(0x5EF9));
	return 0;
}

int ASoundDemo9::command25() {
	findFreeChannelFull(loadData(0x5F27));
	findFreeChannelFull(loadData(0x5F53));
	return 0;
}

int ASoundDemo9::command26() { findFreeChannelFull(loadData(0x5F81)); return 0; }
int ASoundDemo9::command27() { findFreeChannelFull(loadData(0x5F8D)); return 0; }

// ---------------------------------------------------------------------------
// command28_32 - isMusicChannelsActive guard, deferred callback, load ch0-5,8
// (shared handler for commands 28 and 32)
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand28() {
	ASound::command1();
	resetCallbackTimerEx(0x62, 0x54);
	_channels[0]->load(loadData(0x1938));
	_channels[1]->load(loadData(0x1972));
	_channels[2]->load(loadData(0x1A2C));
	_channels[3]->load(loadData(0x1A64));
	_channels[4]->load(loadData(0x1B9E));
	_channels[5]->load(loadData(0x1D5C));
	_channels[8]->load(loadData(0x1C90));
}

int ASoundDemo9::command28_32() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand28));
	else
		loadCommand28();
	return 0;
}

// ---------------------------------------------------------------------------
// command29_33 - isMusicChannelsActive guard, deferred callback, load ch0-8
// (shared handler for commands 29 and 33; non-sequential channel order)
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand29() {
	ASound::command1();
	resetCallbackTimerEx(0x62, 0x54);
	_channels[0]->load(loadData(0x2B10));
	_channels[1]->load(loadData(0x2B7F));
	_channels[2]->load(loadData(0x2BC5));
	_channels[4]->load(loadData(0x2D3F));
	_channels[5]->load(loadData(0x2D9F));
	_channels[6]->load(loadData(0x2F0A));
	_channels[3]->load(loadData(0x2FF3));
	_channels[7]->load(loadData(0x3000));
	_channels[8]->load(loadData(0x2C83));
}

int ASoundDemo9::command29_33() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand29));
	else
		loadCommand29();
	return 0;
}

// ---------------------------------------------------------------------------
// command34 - isMusicChannelsActive guard, deferred callback, load ch0-2,8,4-6
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand34() {
	ASound::command1();
	resetCallbackTimer(0x38);
	_channels[0]->load(loadData(0x300E));
	_channels[1]->load(loadData(0x3204));
	_channels[2]->load(loadData(0x3409));
	_channels[8]->load(loadData(0x360A));
	_channels[4]->load(loadData(0x3789));
	_channels[5]->load(loadData(0x388C));
	_channels[6]->load(loadData(0x38D7));
}

int ASoundDemo9::command34() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand34));
	else
		loadCommand34();
	return 0;
}

// ---------------------------------------------------------------------------
// command35 - isMusicChannelsActive guard, deferred callback, load ch0-3,8,5-6
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand35() {
	ASound::command1();
	resetCallbackTimer(0x50);
	_channels[0]->load(loadData(0x3924));
	_channels[1]->load(loadData(0x396F));
	_channels[2]->load(loadData(0x39B6));
	_channels[3]->load(loadData(0x3A7A));
	_channels[8]->load(loadData(0x3AEA));
	_channels[5]->load(loadData(0x3C01));
	_channels[6]->load(loadData(0x3C97));
}

int ASoundDemo9::command35() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand35));
	else
		loadCommand35();
	return 0;
}

// ---------------------------------------------------------------------------
// command36 - isMusicChannelsActive guard, deferred callback, load ch0-5,8
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand36() {
	ASound::command1();
	resetCallbackTimer(0x28);
	_channels[0]->load(loadData(0x3DFE));
	_channels[1]->load(loadData(0x3E8F));
	_channels[2]->load(loadData(0x3F0B));
	_channels[3]->load(loadData(0x3F41));
	_channels[4]->load(loadData(0x422F));
	_channels[5]->load(loadData(0x42F1));
	_channels[8]->load(loadData(0x4299));
}

int ASoundDemo9::command36() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand36));
	else
		loadCommand36();
	return 0;
}

// ---------------------------------------------------------------------------
// command37 - isMusicChannelsActive guard, deferred callback, load ch0-4,8,5
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand37() {
	ASound::command1();
	resetCallbackTimer(0x50);
	_channels[0]->load(loadData(0x4334));
	_channels[1]->load(loadData(0x43C3));
	_channels[2]->load(loadData(0x4453));
	_channels[3]->load(loadData(0x449A));
	_channels[4]->load(loadData(0x452E));
	_channels[8]->load(loadData(0x453C));
	_channels[5]->load(loadData(0x4618));
}

int ASoundDemo9::command37() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand37));
	else
		loadCommand37();
	return 0;
}

// ---------------------------------------------------------------------------
// command38 - isMusicChannelsActive guard, deferred callback, load ch0-4,8,6
// (ch3 reuses the same data offset as command36's ch3)
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand38() {
	ASound::command1();
	resetCallbackTimer(0x28);
	_channels[0]->load(loadData(0x476C));
	_channels[1]->load(loadData(0x47F9));
	_channels[2]->load(loadData(0x4879));
	_channels[3]->load(loadData(0x3F41));
	_channels[4]->load(loadData(0x48AF));
	_channels[8]->load(loadData(0x4911));
	_channels[6]->load(loadData(0x495D));
}

int ASoundDemo9::command38() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand38));
	else
		loadCommand38();
	return 0;
}

// ---------------------------------------------------------------------------
// command39 - isMusicChannelsActive guard, deferred callback, load ch0-5,8
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand39() {
	ASound::command1();
	resetCallbackTimer(0x28);
	_channels[0]->load(loadData(0x499A));
	_channels[1]->load(loadData(0x4A2F));
	_channels[2]->load(loadData(0x4AC3));
	_channels[3]->load(loadData(0x4BBF));
	_channels[4]->load(loadData(0x5059));
	_channels[5]->load(loadData(0x511B));
	_channels[8]->load(loadData(0x50F1));
}

int ASoundDemo9::command39() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand39));
	else
		loadCommand39();
	return 0;
}

// ---------------------------------------------------------------------------
// command40 - isMusicChannelsActive guard, deferred callback, load ch0-2,8,4-6
// (ch0-2 reuse the same data offsets as command34's ch0-2)
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand40() {
	ASound::command1();
	resetCallbackTimer(0x38);
	_channels[0]->load(loadData(0x300E));
	_channels[1]->load(loadData(0x3204));
	_channels[2]->load(loadData(0x3409));
	_channels[8]->load(loadData(0x521A));
	_channels[4]->load(loadData(0x5327));
	_channels[5]->load(loadData(0x53DF));
	_channels[6]->load(loadData(0x5439));
}

int ASoundDemo9::command40() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand40));
	else
		loadCommand40();
	return 0;
}

// ---------------------------------------------------------------------------
// command41 - isMusicChannelsActive guard, deferred callback, load ch0-2,8,4-6
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand41() {
	ASound::command1();
	resetCallbackTimer(0x54);
	_channels[0]->load(loadData(0x1D6A));
	_channels[1]->load(loadData(0x1E74));
	_channels[2]->load(loadData(0x22ED));
	_channels[8]->load(loadData(0x2465));
	_channels[4]->load(loadData(0x25C2));
	_channels[5]->load(loadData(0x26A2));
	_channels[6]->load(loadData(0x292C));
}

int ASoundDemo9::command41() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand41));
	else
		loadCommand41();
	return 0;
}

// ---------------------------------------------------------------------------
// command42 - isMusicChannelsActive guard, deferred callback, load ch0-3,8,4-6
// ---------------------------------------------------------------------------
void ASoundDemo9::loadCommand42() {
	ASound::command1();
	resetCallbackTimerEx(0xA8, 0x50);
	_channels[0]->load(loadData(0x1DBC));
	_channels[1]->load(loadData(0x1EBF));
	_channels[2]->load(loadData(0x2366));
	_channels[3]->load(loadData(0x1E1E));
	_channels[8]->load(loadData(0x250E));
	_channels[4]->load(loadData(0x25F9));
	_channels[5]->load(loadData(0x2785));
	_channels[6]->load(loadData(0x29C9));
}

int ASoundDemo9::command42() {
	if (isMusicChannelsActive())
		scheduleCallback(MAKE_CALLBACK(ASoundDemo9, loadCommand42));
	else
		loadCommand42();
	return 0;
}

// ---------------------------------------------------------------------------
// command44 - cancels any pending deferred callback (no loads, no fade)
// ---------------------------------------------------------------------------
int ASoundDemo9::command44() {
	scheduleCallback(nullptr);
	return 0;
}

// ---------------------------------------------------------------------------
// command43 - no isMusicChannelsActive guard; always loads immediately.
// load ch0-7 (no ch8)
// ---------------------------------------------------------------------------
int ASoundDemo9::command43() {
	ASound::command1();
	resetCallbackTimer(0x60);
	_channels[0]->load(loadData(0x55EE));
	_channels[1]->load(loadData(0x564D));
	_channels[2]->load(loadData(0x5699));
	_channels[3]->load(loadData(0x596B));
	_channels[4]->load(loadData(0x5981));
	_channels[5]->load(loadData(0x598E));
	_channels[6]->load(loadData(0x599B));
	_channels[7]->load(loadData(0x59A6));
	return 0;
}

// ---------------------------------------------------------------------------
// commands 45-46 - sound effects via findFreeChannel (lower pool)
// ---------------------------------------------------------------------------
int ASoundDemo9::command45() {
	findFreeChannel(loadData(0x5498));
	findFreeChannel(loadData(0x54F8));
	return 0;
}

int ASoundDemo9::command46() {
	findFreeChannel(loadData(0x5543));
	findFreeChannel(loadData(0x5598));
	return 0;
}

// ---------------------------------------------------------------------------
// command47 - no guard, no callback reset; load ch0-8 (all 9 channels)
// ---------------------------------------------------------------------------
int ASoundDemo9::command47() {
	ASound::command1();
	_channels[0]->load(loadData(0x59B4));
	_channels[1]->load(loadData(0x5A3B));
	_channels[2]->load(loadData(0x5A87));
	_channels[3]->load(loadData(0x5BAE));
	_channels[4]->load(loadData(0x5C95));
	_channels[5]->load(loadData(0x5D57));
	_channels[6]->load(loadData(0x5E69));
	_channels[7]->load(loadData(0x5E76));
	_channels[8]->load(loadData(0x5DB5));
	return 0;
}

// ---------------------------------------------------------------------------
// commands 48-50 - sound effects via findFreeChannel (lower pool)
// ---------------------------------------------------------------------------
int ASoundDemo9::command48() { findFreeChannel(loadData(0x5E84)); return 0; }
int ASoundDemo9::command49() { findFreeChannel(loadData(0x5E8E)); return 0; }
int ASoundDemo9::command50() { findFreeChannel(loadData(0x5E98)); return 0; }

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
