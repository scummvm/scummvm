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

#ifndef MADS_NEBULAR_SOUND_NEBULAR_H
#define MADS_NEBULAR_SOUND_NEBULAR_H

#include "mads/nebular/sound/asound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

class ASound1 : public ASound {
private:
	typedef int (ASound1:: *CommandPtr)();
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
	ASound1(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASoundDemo1 : public ASound {
private:
	typedef int (ASoundDemo1:: *CommandPtr)();
	static const CommandPtr _commandList[41];
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

	void command111213();
	int command2627293032();
public:
	ASoundDemo1(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASound2 : public ASound {
private:
	byte _command12Param;
private:
	typedef int (ASound2:: *CommandPtr)();
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
	ASound2(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASound3 : public ASound {
private:
	bool _command39Flag;

	typedef int (ASound3:: *CommandPtr)();
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
	ASound3(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASound4 : public ASound {
private:
	typedef int (ASound4:: *CommandPtr)();
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
	ASound4(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASound5 : public ASound {
private:
	typedef int (ASound5:: *CommandPtr)();
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
	ASound5(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASound6 : public ASound {
private:
	typedef int (ASound6:: *CommandPtr)();
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
	ASound6(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASound7 : public ASound {
private:
	typedef int (ASound7:: *CommandPtr)();
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
	ASound7(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASound8 : public ASound {
private:
	typedef int (ASound8:: *CommandPtr)();
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
	ASound8(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASoundDemo9 : public ASound {
private:
	typedef int (ASoundDemo9:: *CommandPtr)();
	static const CommandPtr _commandList[39];

	int command11();
	int command14();
	int command17();
	int command20();
	int command21();
	int command22();
	int command23();
	int command26();
	int command27();
	int command28();
	int command29();
	int command30();
	int command31();
	int command34();
	int command35();
	int command36();
	int command38();
public:
	ASoundDemo9(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class ASound9 : public ASound {
private:
	/**
	 * Deferred sound-loader callback state. Unlike every other Rex Nebular driver,
	 * ASound9 arms a recurring timer that re-invokes a scheduled loader
	 * function every _callbackPeriod ticks, without ever clearing the
	 * pointer itself (the loader body clears it if it wants the recurrence
	 * to stop).
	 */
	typedef void (ASound9:: *CallbackFunction)();
	int _callbackCounter;
	int _callbackPeriod;
	CallbackFunction _callbackFnPtr;

	typedef int (ASound9:: *CommandPtr)();
	static const CommandPtr _commandList[52];

	void tickCallback() override;

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

	// Deferred loader bodies, scheduled via _callbackFnPtr by the commands above
	void loadCommand38();
	void loadCommand39();
	void loadCommand40();
	void loadCommand41();
	void loadCommand42();
	void loadCommand44_46();
	void loadCommand45();
	void loadCommand47();
	void loadCommand50();
public:
	ASound9(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif
