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

#ifndef MADS_NEBULAR_RSOUND_NEBULAR_H
#define MADS_NEBULAR_RSOUND_NEBULAR_H

#include "mads/nebular/rsound.h"

namespace MADS {
namespace RexNebular {

class RSound9 : public RSound {
private:
	/**
	 * Deferred sound-loader callback state (g_callbackCounter/
	 * g_callbackPeriod/_soundPtr in the original disassembly). Mirrors
	 * ASound9's identical mechanism: arms a recurring timer that
	 * re-invokes a scheduled loader function every _callbackPeriod
	 * ticks, without ever clearing the pointer itself (the loader body
	 * clears it if it wants the recurrence to stop).
	 */
	typedef void (RSound9:: *CallbackFunction)();
	int _callbackCounter;
	int _callbackPeriod;
	CallbackFunction _callbackFnPtr;

	typedef int (RSound9:: *CommandPtr)();
	static const CommandPtr _commandList[52];

	void tickCallback() override;
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
	RSound9(Audio::Mixer *mixer);

	int command(int commandId, int param) override;
};

} // namespace RexNebular
} // namespace MADS

#endif
