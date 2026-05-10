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

#ifndef PROCESS_H
#define PROCESS_H

#include "agds/object.h"
#include "agds/opcode.h"
#include "agds/processExitCode.h"
#include "common/debug.h"
#include "common/scummsys.h"
#include "common/stack.h"

namespace AGDS {

class AGDSEngine;
class Screen;
class Animation;
using AnimationPtr = Common::SharedPtr<Animation>;

class Process {
public:
	enum Status { kStatusActive,
				  kStatusPassive,
				  kStatusDone,
				  kStatusError };

private:
	using StackType = Common::Stack<int32>;

	AGDSEngine *_engine;
	Common::String _parentScreen;
	ObjectPtr _object;
	StackType _stack;
	unsigned _entryPoint;
	unsigned _ip, _lastIp;
	Status _status;
	bool _exited;
	ProcessExitCode _exitCode;
	Common::String _exitArg1, _exitArg2;
	int _exitIntArg1, _exitIntArg2;
	int _tileWidth, _tileHeight;
	int _tileResource;
	int _tileIndex;
	Common::String _phaseVar;
	int _timer;
	int _animationCycles;
	bool _animationLoop;
	Common::Point _animationPosition;
	int _animationZ;
	int _animationDelay;
	int _animationRandom;
	bool _phaseVarControlled;
	int _animationSpeed;
	bool _samplePeriodic;
	bool _sampleAmbient;
	int32 _sampleVolume;
	Common::Point _mousePosition;
	int _filmSubtitlesResource;
	AnimationPtr _processAnimation;
	int _version;

private:
	void debug(const char *str, ...);
	void error(const char *str, ...);

	void push(bool);

	uint8 next();
	uint16 next16() {
		uint8 l = next();
		uint16 h = next();
		return (h << 8) | l;
	}
	uint16 nextOpcode();

	int32 pop();
	int32 top();

	Common::String getString(int id);
	Common::String popString() {
		return getString(pop());
	}
	Common::String popText();
	uint32 popColor();

#define AGDS_PROCESS_METHOD(opcode, method) \
	void method();
#define AGDS_PROCESS_METHOD_C(opcode, method) \
	void method(int8);
#define AGDS_PROCESS_METHOD_B(opcode, method) \
	void method(uint8);
#define AGDS_PROCESS_METHOD_W(opcode, method) \
	void method(int16);
#define AGDS_PROCESS_METHOD_U(opcode, method) \
	void method(uint16);
#define AGDS_PROCESS_METHOD_UD(opcode, method) \
	void method(int32);
#define AGDS_PROCESS_METHOD_UU(opcode, method) \
	void method(uint16, uint16);

	AGDS_OPCODE_LIST(AGDS_PROCESS_METHOD,
					 AGDS_PROCESS_METHOD_C, AGDS_PROCESS_METHOD_B, AGDS_PROCESS_METHOD_W,
					 AGDS_PROCESS_METHOD_U, AGDS_PROCESS_METHOD_UD, AGDS_PROCESS_METHOD_UU)

	void moveCharacter(bool usermove);
	void tell(bool npc, const Common::String &sound);
	void tell(bool npc) { tell(npc, Common::String()); }

	Common::String getCloneVarName(const Common::String &arg1, const Common::String &arg2);

	void suspend(ProcessExitCode exitCode, const Common::String &arg1, const Common::String &arg2 = Common::String());
	void suspend(ProcessExitCode exitCode, int arg1 = 0, int arg2 = 0);

	ProcessExitCode resume();
	void suspendIfPassive();

	void setupAnimation(const AnimationPtr &animation);
	void attachInventoryObjectToMouse(bool flag);
	void leaveCharacter(const Common::String &name, const Common::String &regionName, int dir);
	void removeScreenObject(const Common::String &name);

public:
	Process(AGDSEngine *engine, const ObjectPtr &object, unsigned ip, int version);
	unsigned entryPoint() const {
		return _entryPoint;
	}

	static Common::String disassemble(const ObjectPtr &object, int version);

	ObjectPtr getObject() const {
		return _object;
	}

	const Common::String &getName() const {
		return _object->getName();
	}

	const Common::String &parentScreenName() const {
		return _parentScreen;
	}

	Status status() const {
		return _status;
	}

	bool active() const {
		return _status == kStatusActive;
	}
	bool passive() const {
		return _status == kStatusPassive;
	}
	void activate();
	void deactivate();
	void done();
	void fail();

	bool finished() const {
		return _status == kStatusDone || _status == kStatusError;
	}

	void run();

	ProcessExitCode getExitCode() const {
		return _exitCode;
	}

	const Common::String &getExitArg1() const {
		return _exitArg1;
	}

	int getExitIntArg1() const {
		return _exitIntArg1;
	}

	const Common::String &getExitArg2() const {
		return _exitArg2;
	}

	int getExitIntArg2() const {
		return _exitIntArg2;
	}

	void setMousePosition(Common::Point mousePosition) {
		_mousePosition = mousePosition;
	}
	void updateWithCurrentMousePosition();

	const Common::String &phaseVar() const {
		return _phaseVar;
	}

	const AnimationPtr &processAnimation() const {
		return _processAnimation;
	}

	void removeProcessAnimation();
};

} // End of namespace AGDS

#endif /* AGDS_PROCESS_H */
