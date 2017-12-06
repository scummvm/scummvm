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
	void clearScreen();
	void setSystemVariable();
	void getGlobal(unsigned index);
	void setGlobal();
	void loadPicture();
	void loadMouse();
	void appendToSharedStorage();
	void loadScreenObject();
	void removeScreenObject();
	void loadFont();
	void setIntegerVariable();
	void exitProcess();
	void onKey(unsigned size);

	void stub98();
	void stub128();
	void changeScreenPatch();
	void stub182();
	void stub202(unsigned size);
	void stub203();
	void enableUser();

	void suspend(ProcessExitCode exitCode) {
		if (_status == kStatusActive)
			_status = kStatusPassive;
		_exitCode = exitCode;
	}

public:
	Process(AGDSEngine *engine, Object *object);

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
