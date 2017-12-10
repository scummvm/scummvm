/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGDS_PROCESS_H
#define AGDS_PROCESS_H

#include "agds/object.h"
#include "agds/processExitCode.h"
#include "common/scummsys.h"
#include "common/stack.h"
#include "common/debug.h"

namespace AGDS {

class AGDSEngine;
class Process {
public:
	enum Status { kStatusActive, kStatusPassive, kStatusDone, kStatusError };

private:
	typedef Common::Stack<int32> StackType;

	AGDSEngine *	_engine;
	Object *		_object;
	StackType		_stack;
	unsigned		_ip;
	Status			_status;
	Common::String	_exitValue;
	ProcessExitCode	_exitCode;

private:
	uint8 next() {
		const Object::CodeType & code = _object->getCode();
		if (_ip < code.size()) {
			return code[_ip++];
		} else {
			_status = kStatusError;
			return 0;
		}
	}

	uint16 next16() {
		uint8 l = next();
		uint16 h = next();
		return (h << 8) | l;
	}

	void push(int32 value);
	int32 pop();

	void jump(int delta)
	{ _ip += delta; }

	void jumpz(int delta)
	{
		int value = pop();
		if (value == 0)
			jump(delta);
	}

	Common::String getString(int id);
	Common::String popString() {
		return getString(pop());
	}

	void enter(uint16 magic, uint16 size);
	void exitProcess();
	void suspendProcess();
	void exitScreen();
	void call(uint16 addr);

	void clearScreen();
	void loadPicture();
	void loadMouse();
	void loadScreenObject();
	void loadFont();
	void removeScreenObject();
	void changeScreenPatch();
	void setScreenHeight();
	void updateScreenHeightToDisplay();
	void findObjectInMouseArea();
	void loadRegionFromObject();
	void loadPictureFromObject();
	void loadAnimationFromObject();
	void loadAnimation();
	void loadSample();
	void setCounter();
	void getRegionWidth();
	void getRegionHeight();
	void fadeObject();

	void setSystemVariable();
	void getIntegerSystemVariable();
	void setIntegerVariable();
	void getGlobal(unsigned index);
	void setGlobal();
	void hasGlobal();
	void incrementGlobal();
	void appendToSharedStorage();

	void enableUser();
	void onKey(unsigned size);
	void onUse(unsigned size);

	void loadMouseStub66();
	void stub98();
	void stub128();
	void stub129();
	void stub130();
	void stub133();
	void stub134();
	void stub136();
	void stub165();
	void stub182();
	void stub188();
	void stub190();
	void stub195();
	void stub196();
	void stub202(unsigned size);
	void stub203();
	void stub206();
	void exitProcessSetNextScreen();

#define UNARY_OP(NAME, OP) void NAME () { int arg = pop(); debug(#NAME " %d", arg); push( OP arg ); }
#define BINARY_OP(NAME, OP) void NAME () { int arg2 = pop(); int arg1 = pop(); debug(#NAME " %d " #OP " %d", arg1, arg2); push(arg1 OP arg2); }

	UNARY_OP(boolNot, !)
	UNARY_OP(bitNot, ~)
	BINARY_OP(boolOr, ||)
	BINARY_OP(boolAnd, &&)
	BINARY_OP(equals, ==)
	BINARY_OP(notEquals, !=)
	BINARY_OP(greater, >)
	BINARY_OP(greaterOrEquals, >=)
	BINARY_OP(less, <)
	BINARY_OP(lessOrEquals, <=)
	BINARY_OP(add, +)
	BINARY_OP(sub, -)
	BINARY_OP(mul, *)
	BINARY_OP(div, /)

#undef UNARY_OP
#undef BINARY_OP


	void suspend(ProcessExitCode exitCode) {
		if (_status == kStatusActive)
			_status = kStatusPassive;
		_exitCode = exitCode;
	}

public:
	Process(AGDSEngine *engine, Object *object, unsigned ip = 0);

	void activate();

	ProcessExitCode execute();

	ProcessExitCode getExitCode() const {
		return _exitCode;
	}

	const Common::String & getExitValue() const {
		return _exitValue;
	}
};


} // End of namespace AGDS

#endif /* AGDS_PROCESS_H */
