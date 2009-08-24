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
 * $URL$
 * $Id$
 *
 */

#ifndef M4_SCRIPT_H
#define M4_SCRIPT_H

#include "common/file.h"
#include "common/stream.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/stack.h"

#include "m4/woodscript.h"

namespace M4 {

const unsigned long kScriptFileMagic = 0x5845344D;
const unsigned long kScriptFileVersion = 1;

enum ScriptValueType {
	kInteger,
	kConstString,
	kLogicVar,
	kLogicVarRef,
	kGameVar,
	kKernelVar,
	kDataRef,
	kRegister,
	kStackVar
};

enum ScriptDataType {
	kStreamBreakSeries,
	kStreamPlaySeries,
	kSaidArray,
	kParserArray,
	kSpeechArray,
	kCreditsArray,
	kInvObj,
	kMineRoom,
	kButtonItem
};

class ScriptInterpreter;

class StringTable {
public:
	StringTable();
	~StringTable();
	void load(Common::File *fd);
	int size() { return _strings.size(); }
	const char *operator[](uint32 index) const {
		assert(index < _strings.size() );
		return _strings[index];
	}
protected:
	Common::Array<const char*> _strings;
	char *_stringsData;
};

struct ScriptValue {

	ScriptValueType type;

	union {
		int value;
	};

	ScriptValue() : type(kInteger), value(0) {};
	ScriptValue(ScriptValueType itype, int ivalue) : type(itype), value(ivalue) {};

	ScriptValue(const int intValue) : type(kInteger), value(intValue) {};

	ScriptValue& operator=(const int intValue) {
		type = kInteger;
		value = intValue;
		return *this;
	}

};

class ScriptDataItem {
public:
	ScriptDataItem() : _inter(NULL) {}
	ScriptDataItem(ScriptInterpreter *inter) : _inter(inter) {}
	virtual ~ScriptDataItem() {}
	virtual void load(Common::File *fd) = 0;
	static int type() { return -1; }
protected:
	ScriptInterpreter *_inter;
};

class ScriptDataCache {
public:
	ScriptDataCache(ScriptInterpreter *inter) : _inter(inter) {
	}
	~ScriptDataCache() {
		clear();
	}

	// WORKAROUND: The old prototype for this function was:
	// template<class T> T *load(Common::File *fd, uint32 ofs);
	// that caused a parser error in g++ 3.3.6 used by our
	// "motoezx" target of our buildbot. The actual parser
	// error happended, when calling the function like this:
	// "T *result = _dataCache->load<T>(_scriptFile, _data[value.value]->offset);"
	// in ScriptInterpreter::toData. To work around this
	// we moved the return value as parameter instead.
	template<class T>
	void load(Common::File *fd, uint32 ofs, T *&item) {
		if (_cache.contains(ofs)) {
			item = (T*)(_cache[ofs]);
		} else {
			item = new T(_inter);
			fd->seek(ofs + 4); // "+4" skips the data size
			item->load(fd);
			_cache[ofs] = item;
		}
	}
	void clear() {
		// TODO: Free all cached items
	}
protected:
	typedef Common::HashMap<uint32, ScriptDataItem*> CacheMap;
	CacheMap _cache;
	ScriptInterpreter *_inter;
};

struct SeriesStreamBreakItem {
	int frameNum;
	const char *digiName;
	int digiChannel;
	int digiVolume;
	int trigger;
	int flags;
	ScriptValue variable;
	int value;
};

class SeriesStreamBreakList : public ScriptDataItem {
public:
	SeriesStreamBreakList(ScriptInterpreter *inter) : ScriptDataItem(inter) {}
	~SeriesStreamBreakList();
	void load(Common::File *fd);
	int size() const { return _items.size(); }
	SeriesStreamBreakItem *operator[](int index) const { return _items[index]; }
	static int type() { return 0; }
protected:
	Common::Array<SeriesStreamBreakItem*> _items;
};

struct SaidArrayItem {
	const char *itemName;
	const char *digiNameLook;
	const char *digiNameTake;
	const char *digiNameGear;
};

class SaidArray : public ScriptDataItem {
public:
	SaidArray(ScriptInterpreter *inter) : ScriptDataItem(inter) {}
	~SaidArray();
	void load(Common::File *fd);
	int size() const { return _items.size(); }
	SaidArrayItem *operator[](int index) const { return _items[index]; }
	static int type() { return 2; }
protected:
	Common::Array<SaidArrayItem*> _items;
};

struct ParserArrayItem {
	const char *w0;
	const char *w1;
	int trigger;
	ScriptValue testVariable;
	int testValue;
	ScriptValue variable;
	int value;
};

class ParserArray : public ScriptDataItem {
public:
	ParserArray(ScriptInterpreter *inter) : ScriptDataItem(inter) {}
	~ParserArray();
	void load(Common::File *fd);
	int size() const { return _items.size(); }
	ParserArrayItem *operator[](int index) const { return _items[index]; }
	static int type() { return 3; }
protected:
	Common::Array<ParserArrayItem*> _items;
};

class ScriptFunction {
public:
	ScriptFunction(ScriptInterpreter *inter);
	~ScriptFunction();
	void load(Common::File *fd);
	void jumpAbsolute(uint32 ofs);
	void jumpRelative(int32 ofs);
	byte readByte();
	uint32 readUint32();
protected:
	ScriptInterpreter *_inter;
	Common::MemoryReadStream *_code;
};

struct ScriptFunctionEntry {
	uint32 offset;
	ScriptFunction *func;
	ScriptFunctionEntry(uint32 funcOffset) : offset(funcOffset), func(NULL) {
	}
};

struct ScriptDataEntry {
	uint32 offset;
	ScriptDataType type;
	ScriptDataEntry(uint32 dataOffset, ScriptDataType dataType) : offset(dataOffset), type(dataType) {
	}
};

enum ScriptKernelVariable {
	kGameLanguage,
	kGameVersion,
	kGameCurrentRoom,
	kGameNewRoom,
	kGamePreviousRoom,
	kGameNewSection,
	kKernelTrigger,
	kKernelTriggerMode,
	kKernelFirstFade,
	kKernelSuppressFadeUp,
	kKernelContinueHandlingTrigger,
	kKernelUseDebugMonitor,
	kPlayerPosX,
	kPlayerPosY,
	kPlayerFacing,
	kPlayerScale,
	kPlayerDepth,
	kPlayerWalkX,
	kPlayerWalkY,
	kPlayerReadyToWalk,
	kPlayerNeedToWalk,
	kPlayerCommandReady,
	kPlayerWalkerInThisScene,
	kPlayerVerb,
	kWalkerInitialized,
	kCallDaemonEveryLoop,
	kConvCurrentTalker,
	kConvCurrentNode,
	kConvCurrentEntry,
	kConvSoundToPlay,
	kInterfaceVisible
};

class ScriptInterpreter {
public:
	ScriptInterpreter(M4Engine *vm);
	~ScriptInterpreter();
	/* Opens a M4 program file */
	void open(const char *filename);
	void close();
	/* Loads a function via the index. Creates the function object if it's not already loaded. */
	ScriptFunction *loadFunction(uint32 index);
	/* Loads a function via the exported name. */
	ScriptFunction *loadFunction(const Common::String &name);
	/* Unload all loaded functions.
	   This should be called before entering a new room to free unused functions. */
	void unloadFunctions();
	//TODO void unloadData();
	/* Executes a function. */
	int runFunction(ScriptFunction *scriptFunction);

	void push(const ScriptValue &value);
	void pop(ScriptValue &value);
	void dumpStack();
	void dumpRegisters();
	void dumpGlobalVars();

	int toInteger(const ScriptValue &value);

	const char *toString(const ScriptValue &value);

	// Is this ok?
	template<class T>
	const T& toData(const ScriptValue &value) {
		printf("ScriptInterpreter::toData() index = %d; type = %d; max = %d\n", value.value, _data[value.value]->type, _data.size());
		assert((uint32)value.value < _data.size());
		T *result = 0;
		_dataCache->load(_scriptFile, _data[value.value]->offset, result);
		return *result;
	}

	const char *getGlobalString(int index) const {
		return _constStrings[index];
	}

	const char *loadGlobalString(Common::File *fd);

	void test();

protected:

	M4Engine *_vm;

	typedef Common::HashMap<Common::String, uint32, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FunctionNameMap;
	Common::File *_scriptFile;
	/* An array of offset/ScriptFunction* pairs for each script function */
	Common::Array<ScriptFunctionEntry*> _functions;

	// DEBUG only
	Common::Array<Common::String> _scriptFunctionNames;

	Common::Array<ScriptDataEntry*> _data;
	/* Maps function name -> index of function in _functions array */
	FunctionNameMap _functionNames;
	StringTable _constStrings;
	/* The currently running function */
	ScriptFunction *_runningFunction;
	int _localStackPtr;

	ScriptValue _registers[8];

	ScriptValue _stack[512];
	int _stackPtr;

	int _globalVarCount;
	ScriptValue _globalVars[1024];

	int _logicGlobals[512];

	int _cmpFlags;

	ScriptDataCache *_dataCache;

	int _lineNum;

	typedef int (ScriptInterpreter::*KernelFunction)();
	struct KernelFunctionEntry {
		KernelFunction proc;
		const char *desc;
	};
	const KernelFunctionEntry *_kernelFunctions;
	uint16 _kernelFunctionsMax;

	struct KernelVariableEntry {
		ScriptKernelVariable var;
		const char *desc;
	};
	const KernelVariableEntry *_kernelVars;
	int16 _kernelVarsMax;

	void initScriptKernel();

	void loadValue(ScriptValue &value);
	void writeValue(ScriptValue &value);
	void copyValue(ScriptValue &destValue, ScriptValue &sourceValue);
	void derefValue(ScriptValue &value);

	void callKernelFunction(uint32 index);
	ScriptValue getArg(uint32 index);
	void dumpArgs(uint32 count);

	void callFunction(uint32 index);

	bool execOpcode(byte opcode);

	// Kernel functions
	int o1_handleStreamBreak();
	int o1_handlePlayBreak();
	int o1_dispatchTriggerOnSoundState();
	int o1_getRangedRandomValue();
	int o1_getTicks();
	int o1_preloadSound();
	int o1_unloadSound();
	int o1_stopSound();
	int o1_playSound();
	int o1_playLoopingSound();
	int o1_setSoundVolume();
	int o1_getSoundStatus();
	int o1_getSoundDuration();
	int o1_loadSeries();
	int o1_unloadSeries();
	int o1_showSeries();
	int o1_playSeries();
	int o1_setSeriesFrameRate();
	int o1_playBreakSeries();
	int o1_preloadBreakSeries();
	int o1_unloadBreakSeries();
	int o1_startBreakSeries();
	int o1_dispatchTrigger();
	int o1_terminateMachine();
	int o1_sendWoodScriptMessage();
	int o1_runConversation();
	int o1_loadConversation();
	int o1_exportConversationValue();
	int o1_exportConversationPointer();
	int o1_fadeInit();
	int o1_fadeSetStart();
	int o1_fadeToBlack();
	int o1_initPaletteCycle();
	int o1_stopPaletteCycle();
	int o1_setHotspot();
	int o1_hideWalker();
	int o1_showWalker();
	int o1_setWalkerLocation();
	int o1_setWalkerFacing();
	int o1_walk();
	int o1_overrideCrunchTime();
	int o1_addBlockingRect();
	int o1_triggerTimerProc();
	int o1_setPlayerCommandsAllowed();
	int o1_getPlayerCommandsAllowed();
	int o1_updatePlayerInfo();
	int o1_hasPlayerSaid();
	int o1_hasPlayerSaidAny();
	int o1_playerHotspotWalkOverride();
	int o1_setPlayerFacingAngle();
	int o1_disablePlayerFadeToBlack();
	int o1_enablePlayer();
	int o1_disablePlayer();
	int o1_freshenSentence();
	int o1_playerHasItem();
	int o1_playerGiveItem();
	int o1_moveObject();
	int o1_setStopSoundsBetweenRooms();
	int o1_backupPalette();
	int o1_unloadWilburWalker();
	int o1_globalTriggerProc();
	int o1_wilburSpeech();
	int o1_wilburSaid();
	int o1_wilburParse();
	int o1_wilburTalk();
	int o1_wilburFinishedTalking();
	//int ();

	// Kernel vars
	void getKernelVar(int index, ScriptValue &value);
	void setKernelVar(int index, const ScriptValue &value);

};

} // End of namespace M4

#endif
