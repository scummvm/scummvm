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

#ifndef MADS_PHANTOM_SOUND_H
#define MADS_PHANTOM_SOUND_H

#include "mads/core/sound.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

class PhantomSoundManager : public SoundManager {
protected:
	void loadDriver(int sectionNum) override;

public:
	PhantomSoundManager(Audio::Mixer *mixer, bool &soundFlag) : SoundManager(mixer, soundFlag) {
	}
	~PhantomSoundManager() override {
	}

	void validate() override;
};

class PhantomASound : public ASound {
protected:
	// Per-driver scripting register file (256 byte-sized registers)
	byte _scratchArr[256];

	// Music synchronisation state variables (mirrors word_11F32 etc.)
	int _w11F32 = 0, _w11F42 = 0, _w11F44 = 0;
	int _w11F46 = 0, _w11F48 = 0, _w11F4A = 0;
	int _w11F4C = 0, _w11F4E = 0, _w11F50 = 0;

	void channelCommand(byte *&pSrc, bool &updateFlag) override;

public:
	/**
	 * Constructor
	 * @param mixer			Mixer
	 * @param opl			OPL
	 * @param filename		Specifies the adlib sound player file to use
	 * @param dataOffset	Offset in the file of the data segment
	 */
	PhantomASound(Audio::Mixer *mixer, OPL::OPL *opl, const Common::Path &filename, int dataOffset);
};

/**
 * ASound1  (asound.ph1, _dataOffset = 0x21e0)
 *
 * Dispatch table layout:
 *   off_11C32: commands  0–8   (max=8,    base=0)
 *   off_11C44: command   16    (max=0x10, base=0x10, 1 entry)
 *   off_11C46: commands 24–27  (max=0x1B, base=0x18, 4 entries)
 *   off_11C4E: commands 32–39  (max=0x27, base=0x20, 8 entries)
 *
 * A fifth table (unk_13C3E, commands 64–76) exists but is encoded as raw
 * sound data bytes used as near-pointers — not reconstructible without the
 * binary.  Those commands are silently ignored.
 *
 * command16 (sub_11F70): random background-music selector.  Checks whether
 * channel 0 is already playing one of the five known music pieces; if not,
 * randomly picks from four music loaders and plays it, storing the choice
 * in _musicIndex (mirrors word_11F5E in the original).
 */
class ASound1 : public PhantomASound {
private:
	typedef int (ASound1::*CommandPtr)();
	static const CommandPtr _commandList[40];

	// Mirrors word_11F5E: tracks which music piece was last selected.
	int _musicIndex = 0;

	// Background-music loaders (targets of the CS:0x1F60 indirect table).
	int commandMusic0();   // sub_11D84  – starts at 0x1ECA
	int commandMusic1();   // sub_11EE6  – starts at 0x3418
	int commandMusic2();   // sub_11F0E  – starts at 0x3688
	int commandMusic3();   // sub_11F36  – starts at 0x3D52

	int command0() override;
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

public:
	ASound1(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound1() override {}
	int command(int commandId, int param) override;
};

/**
 * ASound2  (asound.ph2, _dataOffset = 0x2040)
 *
 * Dispatch table layout:
 *   asound_commands1: commands  0–8   (max=8,    base=0)
 *   asound_commands2: command   16    (max=0x10, base=0x10, 1 entry)
 *   asound_commands3: commands 24–27  (max=0x1B, base=0x18, 4 entries)
 *   asound_commands4: commands 32–35  (max=0x23, base=0x20, 4 entries)
 *   asound_commands5: commands 64–72  (max=0x48, base=0x40, 9 entries)
 */
class ASound2 : public PhantomASound {
private:
	typedef int (ASound2::*CommandPtr)();
	static const CommandPtr _commandList[73];

	int command0() override;
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

	int command64();
	int command65();
	int command66();
	int command67();
	int command68();
	int command69();
	int command70();
	int command71();
	int command72();

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
class ASound3 : public PhantomASound {
private:
	typedef int (ASound3::*CommandPtr)();
	static const CommandPtr _commandList[77];

	// Internal helper: isSoundActive guard on 0xC36, load ch0–7.
	// Called by command34 which then adds ch8.
	void sub11CC6();

	int command0() override;
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
class ASound4 : public PhantomASound {
private:
	typedef int (ASound4::*CommandPtr)();
	static const CommandPtr _commandList[71];

	int command0() override;
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
class ASound5 : public PhantomASound {
private:
	typedef int (ASound5::*CommandPtr)();
	static const CommandPtr _commandList[79];

	int command0() override;
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

class ASound9 : public PhantomASound {
private:
	typedef int (ASound9:: *CommandPtr)();
	int command0() override;
	int command1();
	int command2();
	int command3();
	int command4();
	int command5();
	int command6();
	int command7();
	int command8();

	int command16() {
		return command24();
	}

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

	static const CommandPtr _commandList[72];

public:
	ASound9(Audio::Mixer *mixer, OPL::OPL *opl);
	~ASound9() override {
	}
	int command(int commandId, int param) override;
};

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
