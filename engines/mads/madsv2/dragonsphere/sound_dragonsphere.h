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

#ifndef MADS_DRAGONSPHERE_SOUND_H
#define MADS_DRAGONSPHERE_SOUND_H

#include "mads/madsv2/dragonsphere/asound.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {

class DragonSoundManager : public SoundManager {
protected:
	void loadDriver(int sectionNum) override;

public:
	DragonSoundManager(Audio::Mixer *mixer, bool &soundFlag) : SoundManager(mixer, soundFlag) {
	}
	~DragonSoundManager() override {
	}

	void validate() override;
};

/**
 * ASound1  (asound.dr1, _dataOffset = 0x2520, _dataSize = 0x49e0)
 *
 * Dispatch table layout (five tables collapsed to flat [102]):
 *   off_11A14:   commands  0– 8  (base=0,    max=8)
 *   off_11A26:   commands 16–19  (base=0x10, max=0x13; slot 19 = no-op)
 *   off_11A2E:   commands 24–32  (base=0x18, max=0x20; slot 32 = no-op)
 *   funcs_12251: commands 32–49  (base=0x20, max=0x31; slot 49 = no-op)
 *   off_11A64:   commands 64–101 (base=0x40, max=0x65)
 *     Slot 92 and slot 98 are no-ops (command98); slots 102–103 are
 *     nullsub_1/nullsub_4, both beyond the [102] array.
 *
 * word_12370 (_musicIndex in base): tracks the last music-piece launched
 * via command18 for re-entry.  Values <=0x12 use off_11A26; >0x12 use
 * funcs_12251 with index = musicIndex - 0x20.
 *
 * Mutable sound-data bytes (modified before channel loads):
 *   _soundData[0x28C9] — pitch-bend variant byte (command43 / command48)
 *   _soundData[0x4680] — transposition byte for command31's block
 */
class ASound1 : public ASound {
private:
	typedef int (ASound1::*CommandPtr)();
	static const CommandPtr _commandList[102];

	// --------------- deferred-load callbacks (void, no return) ------------
	// Stored via reinterpret_cast<CallbackFunction> and fired by tickCallback.
	void loadCommand16();
	void loadCommand32();
	void loadCommand33();
	void loadCommand34();
	void loadCommand35();
	void loadCommand36();
	void loadCommand37();
	void loadCommand38();
	void loadCommand39();
	void loadCommand40();
	void loadCommand41();
	void loadCommand42();
	void loadCommand44();
	void loadCommand45();
	void loadCommand46();
	void loadCommand47();

	// Shared pitch-bend loader called directly from command43 and command48.
	void loadMusicPitchBend();

	// --------------- command handlers (int, return 0) ----------------------
	int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	int command16();
	int command17();
	int command18();

	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();

	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();
	int command40();
	int command41();
	int command42();
	int command43();
	int command44();
	int command45();
	int command46();
	int command47();
	int command48();

	int command64();
	int command65();
	int command66();
	int command67();
	int command68();
	int command69();
	int command70();
	int command71();
	int command72();
	int command73();
	int command74();
	int command75();
	int command76();
	int command77();
	int command78();
	int command79();
	int command80();
	int command81();
	int command82();
	int command83();
	int command84();
	int command85();
	int command86();
	int command87();
	int command88();
	int command89();
	int command90();
	int command91();
	int command93();
	int command94();
	int command95();
	int command96();
	int command97();
	int command99();
	int command100();
	int command101();

public:
	ASound1(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound1() override {}
	int command(int commandId, int param) override;
};

/**
 * ASound2  (asound.dr2, _dataOffset = 0x1FA0, _dataSize = 0x2950)
 *
 * Dispatch table layout (five tables collapsed to flat [76]):
 *   off_11A14:   commands  0–8   (base=0,    max=8)
 *   off_11A26:   commands 16–19  (base=0x10, max=0x13; slot 19 = no-op)
 *   off_11A2E:   commands 24–31  (base=0x18, max=0x1F; slot at cmd32 unreachable)
 *   funcs_11C87: commands 32–36  (base=0x20, max=0x24; slot 36 = no-op)
 *   off_11A4A:   commands 64–75  (base=0x40, max=0x4B; slots 73–75 = no-ops)
 *
 * command16 sets _musicIndex = 0x10 for command18 re-entry.
 * commands 32–35: _musicIndex saved by dispatcher for command18 re-entry.
 */
class ASound2 : public ASound {
private:
	typedef int (ASound2::*CommandPtr)();
	static const CommandPtr _commandList[76];

	// Deferred loader callbacks (void, Pattern B)
	void loadCommand16();
	void loadCommand32();
	void loadCommand33();
	void loadCommand34();
	void loadCommand35();

	int command0(); int command1(); int command2(); int command3();
	int command4(); int command5(); int command6(); int command7();
	int command8();

	int command16();
	int command17();
	int command18();

	int command24(); int command25(); int command26(); int command27();
	int command28(); int command29(); int command30(); int command31();

	int command32(); int command33(); int command34(); int command35();

	int command64(); int command65(); int command66(); int command67();
	int command68(); int command69_70(); int command71(); int command72();

public:
	ASound2(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound2() override {}
	int command(int commandId, int param) override;
};

/**
 * ASound3  (asound.ph3, _dataOffset = 0x20c0)
 *
 * Dispatch table layout:
 *   asound_commands1: commands  0–8   (max=8,    base=0)
 *   asound_commands2: command   16    (max=0x10, base=0x10, 1 entry)
 *   asound_commands3: commands 24–27  (max=0x1B, base=0x18, 4 entries)
 *   asound_commands4: commands 32–37  (max=0x25, base=0x20, 6 entries)
 *   asound_commands5: commands 64–75  (max=0x4B, base=0x40, 12 entries)
 *     (command 76 = nullsub_8, silently ignored by bounds check)
 */
class ASound3 : public ASound {
private:
	typedef int (ASound3::*CommandPtr)();
	static const CommandPtr _commandList[77];

	// Internal helper: isSoundActive guard on 0xC36, load ch0–7.
	// Called by command34 which then adds ch8.
	void sub11CC6();

	int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	int command16();

	int command24();
	int command25();
	int command26();
	int command27();

	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();

	int command64();
	int command65();
	int command66();
	int command67();
	int command68();
	int command69();
	int command70();
	int command71();
	int command72();
	int command73();
	int command74();
	int command75();

public:
	ASound3(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound3() override {}
	int command(int commandId, int param) override;
};

/**
 * ASound4  (asound.ph4, _dataOffset = 0x1f90)
 *
 * Dispatch table layout:
 *   asound_commands1: commands  0–8   (max=8,    base=0)
 *   asound_commands2: command   16    (max=0x10, base=0x10, 1 entry)
 *   asound_commands3: commands 24–27  (max=0x1B, base=0x18, 4 entries)
 *   asound_commands4: commands 64–70  (max=0x46, base=0x40, 7 entries)
 *     (Commands 32–63 are unreachable: the 0x20-range max constant = 0)
 *
 * commands 24 and 25 share the same handler (sub_11D0A).
 */
class ASound4 : public ASound {
private:
	typedef int (ASound4::*CommandPtr)();
	static const CommandPtr _commandList[71];

	int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	int command16();

	int command24();
	int command25();
	int command26();
	int command27();

	int command64();
	int command65();
	int command66();
	int command67();
	int command68();
	int command69();
	int command70();

public:
	ASound4(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound4() override {}
	int command(int commandId, int param) override;
};

/**
 * ASound5  (asound.ph5, _dataOffset = 0x2140)
 *
 * Dispatch table layout:
 *   asound_commands1: commands  0–8   (max=8,    base=0)
 *   asound_commands2: command   16    (max=0x10, base=0x10, 1 entry)
 *   asound_commands3: commands 24–27  (max=0x1B, base=0x18, 4 entries)
 *   asound_commands4: commands 32–39  (max=0x27, base=0x20, 8 entries)
 *   asound_commands5: commands 64–78  (max=0x4E, base=0x40, 15 entries)
 *     (command 79 = nullsub_8, silently ignored by bounds check)
 *
 * commands 36/35/34 load channels in non-sequential data order.
 * commands 70, 77, and 78 all play the same 0x40BA sound block.
 */
class ASound5 : public ASound {
private:
	typedef int (ASound5::*CommandPtr)();
	static const CommandPtr _commandList[79];

	int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	int command16();

	int command24();
	int command25();
	int command26();
	int command27();

	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command39();

	int command64();
	int command65();
	int command66();
	int command67();
	int command68();
	int command69();
	int command70();
	int command71();
	int command72();
	int command73();
	int command74();
	int command75();
	int command76();
	int command77();
	int command78();

public:
	ASound5(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound5() override {}
	int command(int commandId, int param) override;
};


class ASound6 : public ASound {
private:
	typedef int (ASound6:: *CommandPtr)();
	int command0();
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	static const CommandPtr _commandList[98];

public:
	ASound6(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound6() override {
	}
	int command(int commandId, int param) override;
};

class ASound9 : public ASound {
private:
	typedef int (ASound9:: *CommandPtr)();

	// Deferred loader helpers (void callbacks, Pattern B)
	void loadCommand32();
	void loadCommand33_47();
	void loadCommand34();
	void loadCommand35();
	void loadCommand36();
	void loadCommand37();
	void loadCommand38();
	void loadCommand39();
	void loadCommand40();
	void loadCommand41();
	void loadCommand42();
	void loadCommand51();
	void loadCommand52();
	void command53_loader();
	void loadCommand54();
	void loadCommand55();
	void loadCommand57();
	void loadCommand58();
	void loadCommand62();

	// Commands 0-8: delegate to base
	int command0(); int command1(); int command2(); int command3();
	int command4(); int command5(); int command6(); int command7();
	int command8();

	// Music commands 32-63
	int command32(); int command33_47(); int command34(); int command35();
	int command36(); int command37(); int command38(); int command39();
	int command40(); int command41(); int command42(); int command43();
	int command45(); int command46();
	int command48(); int command49(); int command50();
	int command51(); int command52(); int command53();
	int command54(); int command55();
	int command57(); int command58(); int command59();
	int command61(); int command62(); int command63();

	static const CommandPtr _commandList[65];

public:
	ASound9(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound9() override {}
	int command(int commandId, int param) override;
};

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS

#endif
