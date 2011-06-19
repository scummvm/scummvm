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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The MADS game logic is all hard-coded into the games, although for Rex at least
 * it seems to use only a fairly basic set of instructions and function calls, so it should be
 * possible
 */

#ifndef M4_MADS_LOGIC_H
#define M4_MADS_LOGIC_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"
#include "m4/mads_views.h"

namespace M4 {

union ScriptVarValue {
	const char *strValue;
	uint32 intValue;
};

/**
 * Specifies a script variable that either be a 32-bit unsigned integer or a string pointer
 */
class ScriptVar {
private:
	ScriptVarValue _value;
	bool _isInt;
public:
	ScriptVar(uint32 v = 0) { _value.intValue = v; _isInt = true; }
	ScriptVar(const char *s) { _value.strValue = s; _isInt = false; }

	void set(uint32 v) { _value.intValue = v; _isInt = true; }
	void set(const char *s) { _value.strValue = s; _isInt = false; }
	const char *getStr() const { assert(!_isInt); return _value.strValue; }
	uint32 get() const { assert(_isInt); return _value.intValue; }
	bool isInt() const { return _isInt; }

	operator int() { return get(); }
};

class MadsSceneLogic {
private:
	// Library interface methods
	uint16 loadSpriteSet(uint16 suffixNum, uint16 sepChar);
	uint16 startReversibleSpriteSequence(uint16 srcSpriteIdx, bool flipped, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks);
	uint16 startCycledSpriteSequence(uint16 srcSpriteIdx, bool flipped, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks);
	uint16 startSpriteSequence3(uint16 srcSpriteIdx, bool flipped, int numTicks, int triggerCountdown, int timeoutTicks, int extraTicks);
	void activateHotspot(int idx, bool active);
	void getPlayerSpritesPrefix();
	void getPlayerSpritesPrefix2();
private:
	int _sceneNumber;
	int16 _spriteIndexes[50];
	byte *_scriptsData;
	int _scriptsSize;
	Common::HashMap<Common::String, uint32> _subroutines;
	Common::Array<uint32> _subroutineOffsets;

	enum SubFormatIndex {SUBFORMAT_ENTER, SUBFORMAT_STEP, SUBFORMAT_PREACTIONS, SUBFORMAT_ACTIONS};
	static const char *subFormatList[];
	static const char *_opcodeStrings[];

	// Support functions
	const char *formAnimName(char sepChar, int16 suffixNum);
	void getSceneSpriteSet();
	void getAnimName();

	uint32 getDataValue(int dataId);
	void setDataValue(int dataId, uint16 dataValue);
	void getCallParameters(int numParams, Common::Stack<ScriptVar> &stack, ScriptVar *callParams);
public:
	MadsSceneLogic() { _scriptsData = NULL; }
	~MadsSceneLogic() { delete _scriptsData; }

	void initializeScripts();
	void selectScene(int sceneNum);

	void setupScene();
	void doEnterScene();
	void doPreactions();
	void doAction();
	void doSceneStep();

	void execute(const Common::String &scriptName);
	void execute(uint32 scriptOffset);
	uint32 getParam(uint32 &scriptOffset, int opcode);
	void callSubroutine(int subIndex, Common::Stack<ScriptVar> &stack);
};

class MadsGameLogic {
public:
	static void initializeGlobals();
};

}

#endif
