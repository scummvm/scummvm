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

#ifndef MADS_NEBULAR_SOUND_PSOUND_NEBULAR_H
#define MADS_NEBULAR_SOUND_PSOUND_NEBULAR_H

#include "mads/nebular/sound/psound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

class PSound1 : public PSound {
private:
	typedef int (PSound1:: *CommandPtr)();
	static const CommandPtr _commandList[42];
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

	bool _command23Toggle;

	void loadCommand11Music();

public:
	explicit PSound1(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSound2 : public PSound {
private:
	typedef int (PSound2:: *CommandPtr)();
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

	byte _command12Phase;

	void mutateCommand9Sequence();
	void loadCommand9Music();
	void loadCommand10Music();
	void loadCommand11Music();
	void loadCommand15Music();
	void loadCommand16Music();
	void loadCommand17Music();
	void loadCommand19Music();
	void loadCommand38Music();

public:
	explicit PSound2(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSound3 : public PSound {
private:
	typedef int (PSound3:: *CommandPtr)();
	static const CommandPtr _commandList[61];
	int command1();
	int command3();
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
	int command51();
	int command57();
	int command59();
	int command60();

	bool _command39Toggle;
	byte _stopFadeReload;
	byte getStopFadeReload() const override { return _stopFadeReload; }

	void loadCommand10Music();
	void loadCommand11Music();
	void loadCommand13Music();
	void loadCommand14Music();
	void loadCommand16Music();
	void loadCommand17Music();
	void loadCommand18Music();

public:
	explicit PSound3(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSound4 : public PSound {
private:
	typedef int (PSound4:: *CommandPtr)();
	static const CommandPtr _commandList[60];
	int command1();
	int command3();
	int command9();
	int command10();
	int command12();
	int command19();
	int command20();
	int command21();
	int command22();
	int command23();
	int command27();
	int command30();
	int command32();
	int command33();
	int command34();
	int command35();
	int command36();
	int command37();
	int command38();
	int command52();
	int command53();
	int command54();
	int command55();
	int command56();
	int command57();
	int command58();
	int command59();

	byte _stopFadeReload;
	byte getStopFadeReload() const override { return _stopFadeReload; }

	void loadCommand10Music();
	void loadCommand12Music(int param);
	bool loadCommand53Music();

public:
	explicit PSound4(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSound5 : public PSound {
private:
	typedef int (PSound5:: *CommandPtr)();
	static const CommandPtr _commandList[42];
	int command9();
	int command10();
	int command11122425();
	int command13();
	int command14();
	int command15();
	int command16();
	int command17();
	int command18();
	int command1921();
	int command20();
	int command22();
	int command23();
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

	void loadCommand29Music();

public:
	explicit PSound5(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSound6 : public PSound {
private:
	typedef int (PSound6:: *CommandPtr)();
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

	void loadCommand24Music();
	void loadCommand29Music();

public:
	explicit PSound6(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSound7 : public PSound {
private:
	typedef int (PSound7:: *CommandPtr)();
	static const CommandPtr _commandList[38];
	int command9();
	int command15();
	int command1617();
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
	int command37();

	void loadCommand9Music();
	void loadCommand24Music();
	void loadCommand25Music();
	void loadCommand26Music();
	void loadCommand27Music();

public:
	explicit PSound7(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSound8 : public PSound {
private:
	typedef int (PSound8:: *CommandPtr)();
	static const CommandPtr _commandList[38];
	int command9();
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
	int command37();

	void mutateCommand28Sequence();
	void loadCommand28Music();
	void loadCommand29Music();

public:
	explicit PSound8(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSound9 : public PSound {
private:
	typedef int (PSound9:: *CommandPtr)();
	static const CommandPtr _commandList[52];
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
	int command4446();
	int command45();
	int command47();
	int command48();
	int command49();
	int command50();
	int command51();

	uint16 _scheduledCallbackOffset;

	void scheduleCallback(uint16 callbackOffset);
	void runScheduledCallback();
	void tickCallback() override;
	void loadCommand9Music();
	void loadCommand10Music();
	void loadCommand34Music();
	void loadCommand43Music();
	void loadCommand49Music();
	void loadCommand51Music();

public:
	explicit PSound9(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSoundDemo1 : public PSound {
private:
	typedef int (PSoundDemo1:: *CommandPtr)();
	static const CommandPtr _commandList[39];
	int command101112();

	void loadDemoMusic();

public:
	explicit PSoundDemo1(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

class PSoundDemo9 : public PSound {
private:
	typedef int (PSoundDemo9:: *CommandPtr)();
	static const CommandPtr _commandList[39];
	int command11();
	int command14();
	int command17();
	int command20();
	int command21();
	int command22();
	int command23();
	int command26();
	int command28();
	int command29();
	int command30();
	int command31();
	int command34();
	int command35();
	int command36();
	int command38();

	void loadCommand34Music();
	void loadCommand38Music();

public:
	explicit PSoundDemo9(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

} // namespace Sound
} // namespace RexNebular
} // namespace MADS

#endif // MADS_NEBULAR_SOUND_PSOUND_NEBULAR_H
