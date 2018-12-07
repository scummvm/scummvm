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
class Screen;

class Process {
public:
	enum Status { kStatusActive, kStatusPassive, kStatusDone, kStatusError };

private:
	typedef Common::Stack<int32> StackType;

	AGDSEngine *	_engine;
	Common::String	_parentScreen;
	ObjectPtr		_object;
	StackType		_stack;
	unsigned		_ip, _lastIp;
	Status			_status;
	ProcessExitCode	_exitCode;
	Common::String	_exitArg1, _exitArg2;
	int				_exitIntArg1, _exitIntArg2;
	int				_glyphWidth, _glyphHeight;
	Common::String	_phaseVar;

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
	int32 top();

	void dup() {
		push(top());
	}

	void jump(int delta)
	{ debug("jump %+d", delta); _ip += delta; }

	void jumpz(int delta)
	{
		int value = pop();
		if (value == 0) {
			debug("jumpz %d %+d", value, delta);
			_ip += delta;
		}
	}

	Common::String getString(int id);
	Common::String popString() {
		return getString(pop());
	}
	Common::String popText();

	void enter(uint16 magic, uint16 size);
	void exitProcess();
	void exitProcessCreatePatch();
	void setNextScreen();
	void setNextScreenSaveInHistory();
	void loadPreviousScreen();
	void call(uint16 addr);

	void disableInventory();
	void enableInventory();
	void inventoryClear();
	void inventoryAddObject();
	void inventoryHasObject();
	void getMaxInventorySize();
	void getInventoryFreeSpace();
	void appendInventoryObjectNameToSharedSpace();

	void getObjectId();
	void clearScreen();
	void loadPicture();
	void loadMouse();
	void loadMouseCursorFromObject();
	void loadScreenRegion();
	void loadScreenObject();
	void loadFont();
	void removeScreenObject();
	void changeScreenPatch();
	void setObjectHeight();
	void setScreenHeight();
	void updateScreenHeightToDisplay();
	void addMouseArea();
	void loadRegionFromObject();
	void generateRegion();
	void loadPictureFromObject();
	void loadAnimationFromObject();
	void loadTextFromObject();
	void loadAnimation();
	void setAnimationPosition();
	void loadSample();
	void playSound();
	void playFilm();
	void getSampleVolume();
	void setSampleVolumeAndPan();
	void updatePhaseVarOr2();
	void updatePhaseVarOr4();
	void cloneObject();
	void setTimer();
	void getRegionCenterX();
	void getRegionCenterY();
	void fadeObject();
	void moveScreenObject();
	void setFontGlyphSize();
	void getObjectPictureWidth();
	void getObjectPictureHeight();
	void loadCharacter();
	void enableCharacter();
	void moveCharacter(bool usermove);
	void showCharacter();
	void fogOnCharacter();
	void leaveCharacter();
	void setCharacter();
	void pointCharacter();
	void animateCharacter();
	void getCharacterAnimationPhase();
	void stopCharacter();
	void quit();

	void setDialogForNextFilm();
	void npcSay();
	void playerSay();
	void runDialog();
	void setObjectText();

	void getRandomNumber();
	void setStringSystemVariable();
	void getIntegerSystemVariable();
	void setIntegerSystemVariable();
	void getGlobal(unsigned index);
	void setGlobal();
	void resetPhaseVar();
	void hasGlobal();
	void postIncrementGlobal();
	void postDecrementGlobal();
	void incrementGlobal(int value);
	void incrementGlobalByTop() { incrementGlobal(top()); }
	void decrementGlobal(int value);
	void decrementGlobalByTop() { decrementGlobal(top()); }
	void multiplyGlobalByTop();
	void divideGlobalByTop();
	void modGlobalByTop();
	void shlGlobalByTop();
	void shrGlobalByTop();
	void andGlobalByTop();
	void orGlobalByTop();
	void xorGlobalByTop();

	void appendToSharedStorage();
	void appendNameToSharedStorage();
	Common::String getCloneVarName(const Common::String & arg1, const Common::String & arg2);
	void getCloneVar();
	void setCloneVar();
	void cloneName();
	void setDelay();

	void disableUser();
	void enableUser();
	void disableMouseAreas();
	void modifyMouseArea();
	void onKey(unsigned size);
	void onUse(unsigned size);
	void onLook(unsigned size);
	void onScreenBD(unsigned size);

	void stub63(unsigned size);
	void stub82();
	void stub83();
	void stub102();
	void stub119();
	void stub128();
	void stub129();
	void setCycles();
	void setRandom();
	void stub133();
	void stub136();
	void stub137();
	void stub138();
	void stub152();
	void stub153();
	void stub154();
	void stub155();
	void stub160();
	void stub166();
	void stub172();
	void stub173();
	void stub174();
	void stub184();
	void stub190();
	void stub192();
	void stub193();
	void stub194();
	void stub199();
	void stub200();
	void stub201(unsigned size);
	void stub202(unsigned size);
	void stub209(unsigned size);
	void stub215();
	void stub216();
	void stub217();
	void stub221();
	void stub223();
	void stub225();
	void stub231();
	void stub233();
	void stub235();
	void debug(const char *str, ...);

#define UNARY_OP(NAME, OP) void NAME () { int arg = pop(); debug(#NAME " %d", arg); push( OP arg ); }
#define BINARY_OP(NAME, OP) void NAME () { int arg2 = pop(); int arg1 = pop(); debug(#NAME " %d " #OP " %d", arg1, arg2); push(arg1 OP arg2); }

	UNARY_OP(boolNot, !)
	UNARY_OP(bitNot, ~)
	UNARY_OP(negate, -)
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
	BINARY_OP(bitAnd, &)
	BINARY_OP(bitOr, |)
	BINARY_OP(bitXor, ^)

#undef UNARY_OP
#undef BINARY_OP

	void suspend(ProcessExitCode exitCode, const Common::String &arg1, const Common::String &arg2 = Common::String()) {
		debug("suspend %d", exitCode);
		if (_status == kStatusActive)
			_status = kStatusPassive;
		_exitCode = exitCode;
		_exitIntArg1 = 0;
		_exitIntArg2 = 0;
		_exitArg1 = arg1;
		_exitArg2 = arg2;
	}

	void suspend(ProcessExitCode exitCode = kExitCodeSuspend, int arg1 = 0, int arg2 = 0) {
		debug("suspend %d", exitCode);
		if (_status == kStatusActive)
			_status = kStatusPassive;
		_exitCode = exitCode;
		_exitIntArg1 = arg1;
		_exitIntArg2 = arg2;
		_exitArg1.clear();
		_exitArg2.clear();
	}

public:
	Process(AGDSEngine *engine, ObjectPtr object, unsigned ip = 0);

	const Common::String & getName() const {
		return _object->getName();
	}

	const Common::String & parentScreenName() const {
		return _parentScreen;
	}

	Status getStatus() const {
		return _status;
	}

	void activate();

	ProcessExitCode execute();

	ProcessExitCode getExitCode() const {
		return _exitCode;
	}

	const Common::String & getExitArg1() const {
		return _exitArg1;
	}

	int getExitIntArg1() const {
		return _exitIntArg1;
	}

	const Common::String & getExitArg2() const {
		return _exitArg2;
	}

	int getExitIntArg2() const {
		return _exitIntArg2;
	}

};


} // End of namespace AGDS

#endif /* AGDS_PROCESS_H */
