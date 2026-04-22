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

#ifndef MADS_SOUND_NEBULAR_H
#define MADS_SOUND_NEBULAR_H

#include "mads/core/sound.h"

namespace MADS {
namespace Nebular {

class RexSoundManager : public SoundManager {
protected:
	void loadDriver(int sectionNum) override;

public:
	RexSoundManager(Audio::Mixer *mixer, bool &soundFlag) : SoundManager(mixer, soundFlag) {
	}
	~RexSoundManager() override {
	}

	void validate() override;
};

class RexASound : public ASound {
protected:
	void channelCommand(byte *&pSrc, bool &updateFlag) override;

public:
	RexASound(Audio::Mixer *mixer, OPL::OPL *opl,
		const Common::Path &filename, int dataOffset);
};

class ASound1 : public RexASound {
private:
	typedef int (ASound1::*CommandPtr)();
	static const CommandPtr _commandList[42];
	bool _cmd23Toggle;

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
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

	void command111213();
	int command2627293032();
public:
	ASound1(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound2 : public RexASound {
private:
	byte _command12Param;
private:
	typedef int (ASound2::*CommandPtr)();
	static const CommandPtr _commandList[44];

	int command0() override;
	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
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

	void command9Randomize();
	void command9Apply(byte *data, int val, int incr);
public:
	ASound2(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound3 : public RexASound {
private:
	bool _command39Flag;

	typedef int (ASound3::*CommandPtr)();
	static const CommandPtr _commandList[61];

	int command9();
	int command10();
	int command11();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
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
	int command49();
	int command50();
	int command51();
	int command57();
	int command59();
	int command60();

	void command9Randomize();
	void command9Apply(byte *data, int val, int incr);
public:
	ASound3(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound4 : public RexASound {
private:
	typedef int (ASound4::*CommandPtr)();
	static const CommandPtr _commandList[61];

	int command10();
	int command12();
	int command19();
	int command20();
	int command21();
	int command24();
	int command27();
	int command30();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command43();
	int command52();
	int command53();
	int command54();
	int command55();
	int command56();
	int command57();
	int command58();
	int command59();
	int command60();

	void method1();
public:
	ASound4(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound5 : public RexASound {
private:
	typedef int (ASound5::*CommandPtr)();
	static const CommandPtr _commandList[42];

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
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
public:
	ASound5(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound6 : public RexASound {
private:
	typedef int (ASound6::*CommandPtr)();
	static const CommandPtr _commandList[30];

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command29();
public:
	ASound6(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound7 : public RexASound {
private:
	typedef int (ASound7::*CommandPtr)();
	static const CommandPtr _commandList[38];

	int command9();
	int command15();
	int command16();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command24();
	int command25();
	int command26();
	int command27();
	int command28();
	int command30();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
public:
	ASound7(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound8 : public RexASound {
private:
	typedef int (ASound8::*CommandPtr)();
	static const CommandPtr _commandList[38];

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
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

	void method1(byte *pData);
	void adjustRange(byte *pData, byte v, int incr);
public:
	ASound8(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

class ASound9 : public RexASound {
private:
	int _v1, _v2;
	byte *_soundPtr;

	typedef int (ASound9::*CommandPtr)();
	static const CommandPtr _commandList[52];

	int command9();
	int command10();
	int command11();
	int command12();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
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
	int command44_46();
	int command45();
	int command47();
	int command48();
	int command49();
	int command50();
	int command51();
	int command57();
	int command59();
	int command60();
public:
	ASound9(Audio::Mixer *mixer, OPL::OPL *opl);

	int command(int commandId, int param) override;
};

} // namespace Nebular

} // namespace MADS

#endif /* MADS_SOUND_NEBULAR_H */
