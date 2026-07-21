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

#include "mads/dragonsphere/asound.h"

namespace MADS {
namespace Dragonsphere {

class DragonSoundManager : public SoundManager {
private:
	bool _isDemo;

protected:
	void loadDriver(int sectionNum) override;

public:
	DragonSoundManager(Audio::Mixer *mixer, bool &soundFlag, bool isDemo) :
		SoundManager(mixer, soundFlag), _isDemo(isDemo) {
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
 * ASound3  (asound.dr3, _dataOffset = 0x1F30, _dataSize = 0x2750)
 *
 * Dispatch table layout (five tables collapsed to flat [77]):
 *   off_11A14:   commands  0–8   (base=0,    max=8)
 *   off_11A26:   commands 16–19  (base=0x10, max=0x13; slot 19 = no-op)
 *   off_11A2E:   commands 24–32  (base=0x18, max=0x20; slot 32 = no-op/unreachable)
 *   funcs_11C61: commands 32–34  (base=0x20, max=0x22; slot 34 = no-op)
 *   off_11A46:   commands 64–76  (base=0x40, max=0x4C; slots 74–76 = no-ops)
 *
 * command16 sets _musicIndex = 0x10 for command18 re-entry.
 * commands 32–33: _musicIndex saved by dispatcher for command18 re-entry.
 *
 * command31 and command64 both target the same sound block (0x23E9) but
 * patch byte 0x23EC to 0x67 vs 0x78 for different pitch variants.
 */
class ASound3 : public ASound {
private:
	typedef int (ASound3::*CommandPtr)();
	static const CommandPtr _commandList[77];

	// Deferred loader callbacks (void, Pattern B)
	void loadCommand16();
	void loadCommand32();
	void loadCommand33();

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

public:
	ASound3(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound3() override {}
	int command(int commandId, int param) override;
};

/**
 * ASound4  (asound.dr4, _dataOffset = 0x2120, _dataSize = 0x31D0)
 *
 * Dispatch table layout (five tables collapsed to flat [82]):
 *   off_11A14:   commands  0–8   (base=0,    max=8;    slot 5 = no-op)
 *   off_11A26:   commands 16–19  (base=0x10, max=0x13; slot 19 = no-op)
 *   off_11A2E:   commands 24–32  (base=0x18, max=0x20; slot 32 = no-op/unreachable)
 *   funcs_11E51: commands 32–41  (base=0x20, max=0x29; slots 34,41 = no-ops)
 *   off_11A54:   commands 64–81  (base=0x40, max=0x51; slot 79 = no-op; slots 80,81 = stubs)
 *
 * command16 sets _musicIndex = 0x10 for command18 re-entry.
 * commands 32–33 and 35–40: _musicIndex saved by dispatcher for command18 re-entry.
 *
 * command31 patches _soundData[0x2E72] to 0x67 before playSound.
 * commands 32–33 use ASound::command3() (partial fade) rather than command1().
 * command39 loads only channels 0–5 (no channel 6).
 */
class ASound4 : public ASound {
private:
	typedef int (ASound4::*CommandPtr)();
	static const CommandPtr _commandList[82];

	// Deferred loader callbacks (void, Pattern B)
	void loadCommand16();
	void loadCommand32();
	void loadCommand33();
	void loadCommand35();
	void loadCommand36();
	void loadCommand37();
	void loadCommand38();
	void loadCommand39();
	void loadCommand40();

	int command0(); int command1(); int command2(); int command3();
	int command4(); int command6(); int command7(); int command8();

	int command16(); int command17(); int command18();

	int command24(); int command25(); int command26(); int command27();
	int command28(); int command29(); int command30(); int command31();

	int command32(); int command33(); int command35(); int command36();
	int command37(); int command38(); int command39(); int command40();

	int command64(); int command65(); int command66(); int command67();
	int command68(); int command69(); int command70(); int command71();
	int command72(); int command73(); int command74(); int command75();
	int command76(); int command77(); int command78();
	int command80(); int command81();

public:
	ASound4(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound4() override {}
	int command(int commandId, int param) override;
};

/**
 * ASound5  (asound.dr5, _dataOffset = 0x20D0, _dataSize = 0x2EE0)
 *
 * Dispatch table layout (five tables collapsed to flat [82]):
 *   off_11A14:   commands  0–8   (base=0,    max=8)
 *   off_11A26:   commands 16–19  (base=0x10, max=0x13; slot 19 = no-op)
 *   off_11A2E:   commands 24–32  (base=0x18, max=0x20; slot 32 = no-op/unreachable)
 *   funcs_11E05: commands 32–39  (base=0x20, max=0x27; slot 39 = no-op)
 *   off_11A50:   commands 64–81  (base=0x40, max=0x51; slot 79 = no-op; slots 80–81 = stubs)
 *
 * command16 and command32/33 set _musicIndex = 0x10 for command18 re-entry.
 * command38 sets _musicIndex = 0x26; command36 sets it to 0x29 (beyond table).
 * command17 uses a direct (non-deferred) load with inline resetCallbackTimer(0x60).
 * command36 calls setMusicIndex(0x29) then playSound (no channel loads).
 * command37 calls ASound::command1() then findFreeChannel (lower pool) ×6.
 * command77 accesses _lastParam (stored by dispatcher) to branch on param value.
 *
 * _soundData[0x2B7E] — pitch-bend byte (command31 = 0x67, command78 = 0x7F)
 * _soundData[0x2B4B] — velocity byte set by command77 (param≠0 path)
 */
class ASound5 : public ASound {
private:
	typedef int (ASound5::*CommandPtr)();
	static const CommandPtr _commandList[82];

	// Deferred loader callbacks (void, Pattern B)
	void loadCommand16();
	void loadCommand32();
	void loadCommand33();
	void loadCommand34();
	void loadCommand35();
	void loadCommand38();

	int _lastParam;    // param from command() stored for use by command77

	int command0(); int command1(); int command2(); int command3();
	int command4(); int command5(); int command6(); int command7();
	int command8();

	int command16(); int command17(); int command18();

	int command24(); int command25(); int command26(); int command27();
	int command28(); int command29(); int command30(); int command31();

	int command32(); int command33(); int command34(); int command35();
	int command36(); int command37(); int command38();

	int command64(); int command65(); int command66(); int command67();
	int command68(); int command69(); int command70(); int command71();
	int command72(); int command73(); int command74(); int command75();
	int command76(); int command77(); int command78();
	int command80(); int command81();

public:
	ASound5(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound5() override {}
	int command(int commandId, int param) override;
};


/**
 * ASound6  (asound.dr6, _dataOffset = 0x2370, _dataSize = 0x3870)
 * Dispatch table layout (five tables collapsed to flat [102]):
 *   off_11A14:   commands  0– 8  (base=0,    max=8)
 *   off_11A26:   commands 16–19  (base=0x10, max=0x13; slot 19 = no-op)
 *   off_11A2E:   commands 24–31  (base=0x18, max=0x1F)
 *   funcs_1204D: commands 32–47  (base=0x20, max=0x2F; slots 41–43, 46–47 = no-op)
 *   off_11A60:   commands 64–101 (base=0x40, max=0x65; slot 99 = no-op)
 */
class ASound6 : public ASound {
private:
	typedef int (ASound6::*CommandPtr)();
	static const CommandPtr _commandList[102];

	void loadCommand16();
	void loadCommand32();   // shared deferred loader for commands 32 & 33
	void loadCommand34();
	void loadCommand35();
	void loadCommand36();
	void loadCommand37();
	void loadCommand38();
	void loadCommand39();
	void loadCommand40();
	void loadCommand44();
	void loadCommand45();
	void loadCommand96();

	uint8 _cmd33Flag;   // byte_134D1: set by command33 to extend loadCommand32
	int   _lastParam;   // param stored by command() for use by command64

	int command0();  int command1();  int command2();  int command3();
	int command4();  int command5();  int command6();  int command7();
	int command8();

	int command16(); int command17(); int command18();

	int command24(); int command25(); int command26(); int command27();
	int command28(); int command29(); int command30(); int command31();

	int command32(); int command33(); int command34(); int command35();
	int command36(); int command37(); int command38(); int command39();
	int command40(); int command44(); int command45();

	int command64(); int command65(); int command66(); int command67();
	int command68(); int command69(); int command70(); int command71();
	int command72(); int command73(); int command74(); int command75();
	int command76(); int command77(); int command78(); int command79();
	int command80(); int command81(); int command82(); int command83();
	int command84(); int command85(); int command86(); int command87();
	int command88(); int command89(); int command90(); int command91();
	int command92(); int command93(); int command94(); int command95();
	int command96(); int command97(); int command98();
	int command100(); int command101();

public:
	ASound6(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound6() override {}
	int command(int commandId, int param) override;
};

/**
 * ASoundDemo1  (asound.dr1 [demo], _dataOffset = 0x23e0, _dataSize = 0x4900)
 *
 * Dispatch table layout (four tables collapsed to flat [93]):
 *   commands0:  commands  0– 8  (base=0,    max=8)
 *   commands16: command   16    (base=0x10, max=0x10, 1 entry)
 *   commands24: commands 24–43  (base=0x18, max=0x2B; slots 28,29,44 = no-op)
 *   commands64: commands 64–92  (base=0x40, max=0x5C; slots 64–89 = no-op)
 */
class ASoundDemo1 : public ASound {
private:
	typedef int (ASoundDemo1::*CommandPtr)();
	static const CommandPtr _commandList[93];

	void loadCommand16();
	void loadCommand30();
	void loadCommand31();
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
	void loadCommand43();
	void loadCommand92();

	int command0(); int command1(); int command2(); int command3();
	int command4(); int command5(); int command6(); int command7();
	int command8();

	int command16();

	int command24(); int command25(); int command26(); int command27();
	int command30(); int command31(); int command32(); int command33();
	int command34(); int command35(); int command36(); int command37();
	int command38(); int command39(); int command40(); int command41();
	int command42(); int command43();

	int command90(); int command91(); int command92();

public:
	ASoundDemo1(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASoundDemo1() override {}
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

/**
 * ASoundDemo9  (asound.dr9 [demo], _dataOffset = 0x23a0, _dataSize = 0x62b0)
 *
 * Dispatch table layout (flat [51]):
 *   commands0:     commands  0– 8  (base=0,    max=8)
 *   commands16_24: command   16    (base=0x10, max=0x10, 1 entry; also
 *                  reused directly for command 24 - shared handler)
 *   commands24:    commands 25–29, 34–50 (base=0x18/0x22; slots 30,31 = no-op)
 *     - command28_32 is shared between commands 28 and 32
 *     - command29_33 is shared between commands 29 and 33
 *     (commands 32/33 are redirected to 28/29 by index-correction code
 *     outside the array, not via separate array slots)
 *   commands 64+ are unreachable (dispatcher upper bound is 0 for that range)
 */
class ASoundDemo9 : public ASound {
private:
	typedef int (ASoundDemo9::*CommandPtr)();
	static const CommandPtr _commandList[51];

	// Deferred loader callbacks (void, Pattern B)
	void loadCommand28();
	void loadCommand29();
	void loadCommand34();
	void loadCommand35();
	void loadCommand36();
	void loadCommand37();
	void loadCommand38();
	void loadCommand39();
	void loadCommand40();
	void loadCommand41();
	void loadCommand42();

	int command0(); int command1(); int command2(); int command3();
	int command4(); int command5(); int command6(); int command7();
	int command8();

	int command16();

	int command25(); int command26(); int command27();
	int command28_32();
	int command29_33();

	int command34(); int command35(); int command36(); int command37();
	int command38(); int command39(); int command40(); int command41();
	int command42(); int command43(); int command44(); int command45();
	int command46(); int command47(); int command48(); int command49();
	int command50();

public:
	ASoundDemo9(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASoundDemo9() override {}
	int command(int commandId, int param) override;
};

} // namespace Dragonsphere
} // namespace MADS

#endif
