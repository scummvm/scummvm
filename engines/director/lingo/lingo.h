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

#ifndef DIRECTOR_LINGO_LINGO_H
#define DIRECTOR_LINGO_LINGO_H

#include "audio/audiostream.h"
#include "common/hash-ptr.h"
#include "common/hash-str.h"
#include "common/endian.h"
#include "common/str-array.h"

#include "director/types.h"
#include "director/lingo/lingo-gr.h"

namespace Common {
	class SeekableSubReadStreamEndian;
}

namespace Director {

struct TheEntity;
struct TheEntityField;
struct LingoV4Bytecode;
struct LingoV4TheEntity;
struct Object;
class ScriptContext;
class DirectorEngine;
class Frame;

enum LexerDefineState {
	kStateNone,
	kStateInDef,
	kStateInArgs
};

enum VarType {
	kVarArgument,
	kVarProperty,
	kVarInstance,
	kVarGlobal,
	kVarLocal
};

typedef void (*inst)(void);
#define	STOP (inst)0
#define ENTITY_INDEX(t,id) ((t) * 100000 + (id))

typedef Common::Array<inst> ScriptData;

struct FuncDesc {
	Common::String name;
	const char *proto;

	FuncDesc(Common::String n, const char *p) { name = n; proto = p; }
};

typedef Common::HashMap<void *, FuncDesc *> FuncHash;

struct Symbol {	/* symbol table entry */
	Common::String *name;
	int type;
	union {
		ScriptData	*defn;	/* HANDLER */
		void (*func)();		/* OPCODE */
		void (*bltin)(int);	/* BUILTIN */
		Common::String	*s;	/* STRING */
	} u;

	int *refCount;

	int nargs;		/* number of arguments */
	int maxArgs;	/* maximal number of arguments, for builtins */
	bool parens;	/* whether parens required or not, for builitins */
	int targetType;	/* valid target objects, for method builtins */

	Common::Array<Common::String> *argNames;
	Common::Array<Common::String> *varNames;
	ScriptContext *ctx;		/* optional script context to execute with */
	int archiveIndex; 		/* optional archive to execute with */

	Symbol();
	Symbol(const Symbol &s);
	Symbol& operator=(const Symbol &s);
	void reset();
	~Symbol();
};

struct Datum {	/* interpreter stack type */
	int type;
	bool lazy; // evaluate when popped off stack

	union {
		int	i;				/* INT, ARGC, ARGCNORET */
		double f;			/* FLOAT */
		Common::String *s;	/* STRING, VAR, OBJECT */
		DatumArray *farr;	/* ARRAY, POINT, RECT */
		PropertyArray *parr; /* PARRAY */
		Object *obj;
	} u;

	int *refCount;

	Datum() {
		u.s = nullptr;
		type = VOID;
		lazy = false;
		refCount = new int;
		*refCount = 1;
	}
	Datum(const Datum &d) {
		type = d.type;
		lazy = d.lazy;
		u = d.u;
		refCount = d.refCount;
		*refCount += 1;
	}
	Datum& operator=(const Datum &d) {
		if (this != &d) {
			reset();
			type = d.type;
			u = d.u;
			refCount = d.refCount;
			*refCount += 1;
		}
		return *this;
	}
	Datum(int val) {
		u.i = val;
		type = INT;
		lazy = false;
		refCount = new int;
		*refCount = 1;
	}
	Datum(double val) {
		u.f = val;
		type = FLOAT;
		lazy = false;
		refCount = new int;
		*refCount = 1;
	}
	Datum(const Common::String &val) {
		u.s = new Common::String(val);
		type = STRING;
		lazy = false;
		refCount = new int;
		*refCount = 1;
	}
	Datum(Object *val) {
		u.obj = val;
		type = OBJECT;
		lazy = false;
		refCount = new int;
		*refCount = 1;
	}

	void reset();

	~Datum() {
		reset();
	}

	Datum eval();
	double asFloat();
	int asInt();
	Common::String asString(bool printonly = false);

	const char *type2str(bool isk = false);

	int equalTo(Datum &d, bool ignoreCase = false);
	int compareTo(Datum &d, bool ignoreCase = false);
};

struct PCell {
	Datum p;
	Datum v;

	PCell();
	PCell(const Datum &prop, const Datum &val);
};

struct Builtin {
	void (*func)(void);
	int nargs;

	Builtin(void (*func1)(void), int nargs1) : func(func1), nargs(nargs1) {}
};

typedef Common::HashMap<int32, ScriptContext *> ScriptContextHash;
typedef Common::Array<Datum> StackData;
typedef Common::HashMap<Common::String, Symbol, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SymbolHash;
typedef Common::HashMap<Common::String, Datum, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> DatumHash;
typedef Common::HashMap<Common::String, Builtin *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> BuiltinHash;
typedef Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> VarTypeHash;

typedef Common::HashMap<Common::String, TheEntity *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityHash;
typedef Common::HashMap<Common::String, TheEntityField *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityFieldHash;

class ScriptContext {
public:
	ScriptType _type;
	Common::String _name;
	Common::Array<Common::String> _functionNames; // used by cb_localcall
	SymbolHash _functionHandlers;
	Common::HashMap<uint32, Symbol> _eventHandlers;
	Common::Array<Datum> _constants;
	Common::Array<Common::String> _propNames;
	Datum _target;

	ScriptContext(ScriptType type, Common::String name) : _type(type), _name(name) {}
	ScriptContext(const ScriptContext &sc) {
		_type = sc._type;
		_name = sc._name;
		_functionNames = sc._functionNames;
		for (SymbolHash::iterator it = sc._functionHandlers.begin(); it != sc._functionHandlers.end(); ++it) {
			_functionHandlers[it->_key] = it->_value;
			_functionHandlers[it->_key].ctx = this;
		}
		for (Common::HashMap<uint32, Symbol>::iterator it = sc._eventHandlers.begin(); it != sc._eventHandlers.end(); ++it) {
			_eventHandlers[it->_key] = it->_value;
			_eventHandlers[it->_key].ctx = this;
		}
		_constants = sc._constants;
		_propNames = sc._propNames;
	}

	Datum getParentScript();

private:
	Datum _parentScript;
};

enum ObjectType {
	kNoneObj = 0,
	kFactoryObj = 1 << 0,
	kXObj = 1 << 1,
	kScriptObj = 1 << 2,
	kXtraObj = 1 << 3,
	kAllObj = kFactoryObj | kXObj | kScriptObj | kXtraObj
};

struct Object {
	Common::String *name;
	ObjectType type;
	bool disposed;

	DatumHash properties;
	ScriptContext *ctx;
	int inheritanceLevel; // 1 for original object

	// used only for factories
	Common::HashMap<uint32, Datum> *objArray;

	Object(const Common::String &objName, ObjectType objType, ScriptContext *objCtx) {
		name = new Common::String(objName);
		type = objType;
		disposed = false;
		inheritanceLevel = 1;
		ctx = objCtx;
		ctx->_target = this;

		// Don't include the ctx's reference to me in the refCount.
		// Once that's the only remaining reference,
		// I should be destroyed, killing the ctx with me.
		*ctx->_target.refCount -= 1;

		if (objType == kFactoryObj) {
			objArray = new Common::HashMap<uint32, Datum>;
		} else {
			objArray = nullptr;
		}
	}

	Object(const Object &obj) {
		name = new Common::String(*obj.name);
		type = obj.type;
		disposed = obj.disposed;
		inheritanceLevel = obj.inheritanceLevel + 1;
		properties = obj.properties;
		ctx = new ScriptContext(*obj.ctx);
		ctx->_target = this;
		*ctx->_target.refCount -= 1;

		if (obj.objArray) {
			objArray = new Common::HashMap<uint32, Datum>(*obj.objArray);
		} else {
			objArray = nullptr;
		}
	}

	virtual ~Object() {
		delete name;
		delete objArray;
		ctx->_target.refCount = nullptr; // refCount has already been freed
		delete ctx;
	}

	virtual Object *clone();
	Symbol getMethod(const Common::String &methodName);
	bool hasVar(const Common::String &varName);
	Datum &getVar(const Common::String &varName);
	bool hasProp(const Common::String &propName);
	Datum &getProp(const Common::String &propName);
};

struct CFrame {	/* proc/func call stack frame */
	Symbol	sp;	/* symbol table entry */
	int		retpc;	/* where to resume after return */
	ScriptData	*retscript;	 /* which script to resume after return */
	ScriptContext	*retctx;   /* which script context to use after return */
	int 	retarchive;	/* which archive to use after return */
	DatumHash *localvars;
	Datum retMe; /* which me obj to use after return */
};

struct LingoEvent {
	LEvent event;
	int archiveIndex;
	ScriptType st;
	int scriptId;
	int channelId;

	LingoEvent (LEvent e, int ai, ScriptType s, int si, int ci = -1) {
		event = e;
		archiveIndex = ai;
		st = s;
		scriptId = si;
		channelId = ci;
	}
};


struct LingoArchive {
	ScriptContextHash scriptContexts[kMaxScriptType + 1];
	Common::Array<Common::String> names;
	Common::HashMap<uint32, Common::String> primaryEventHandlers;
	SymbolHash functionHandlers;
};

struct RepeatBlock {
	Common::Array<uint32> exits;
	Common::Array<uint32> nexts;
};

class Lingo {

public:
	Lingo(DirectorEngine *vm);
	~Lingo();

	void resetLingo(bool keepSharedCast);

	void addCode(const char *code, int archiveIndex, ScriptType type, uint16 id, const char *scriptName = nullptr);
	ScriptContext *compileAnonymous(const char *code);
	void addCodeV4(Common::SeekableSubReadStreamEndian &stream, int archiveIndex, ScriptType type, uint16 id, const Common::String &scriptName, Common::String &archName);
	void addNamesV4(Common::SeekableSubReadStreamEndian &stream, int archiveIndex);
	void executeHandler(const Common::String &name);
	void executeScript(ScriptType type, uint16 id);
	void printStack(const char *s, uint pc);
	void printCallStack(uint pc);
	Common::String decodeInstruction(ScriptData *sd, uint pc, uint *newPC = NULL);

	void initBuiltIns();
	void cleanupBuiltins();
	void initFuncs();
	void initBytecode();
	void initMethods();
	void initXLibs();
	void openXLib(const Common::String &name, ObjectType type);

	void runTests();

	// lingo-preprocessor.cpp
public:
	Common::String codePreprocessor(const char *s, ScriptType type, uint16 id, bool simple = false);

	// lingo-patcher.cpp
	Common::String patchLingoCode(Common::String &line, ScriptType type, uint16 id, int linenumber);

	// lingo.cpp
private:
	ScriptContext *compileLingo(const char *code, int archiveIndex, ScriptType type, uint16 id, const Common::String &scriptName);
	const char *findNextDefinition(const char *s);

	// lingo-events.cpp
private:
	void initEventHandlerTypes();
	void setPrimaryEventHandler(LEvent event, const Common::String &code);
	void primaryEventHandler(LEvent event);
	void registerSpriteEvent(LEvent event, int spriteId);
	void registerFrameEvent(LEvent event);
	void registerMovieEvent(LEvent event);
	void processEvent(LEvent event, int archiveIndex, ScriptType st, int entityId, int channelId = -1);

	Common::Queue<LingoEvent> _eventQueue;

public:
	ScriptContext *getScriptContext(int archiveIndex, ScriptType type, uint16 id);
	Common::String getName(uint16 id);
	ScriptType event2script(LEvent ev);
	Symbol getHandler(const Common::String &name);

	int getEventCount();
	void processEvent(LEvent event, int spriteId = 0);
	void processEvents();
	void registerEvent(LEvent event, int spriteId = 0);

public:
	void execute(uint pc);
	void pushContext(const Symbol funcSym, bool preserveVarFrame = false);
	void popContext();
	void cleanLocalVars();
	int castIdFetch(Datum &var);
	void varAssign(Datum &var, Datum &value, bool global = false, DatumHash *localvars = nullptr);
	Datum varFetch(Datum &var, bool global = false, DatumHash *localvars = nullptr);

	int getAlignedType(Datum &d1, Datum &d2);

	void printAllVars();

	int calcStringAlignment(const char *s) {
		return calcCodeAlignment(strlen(s) + 1);
	}
	int calcCodeAlignment(int l) {
		int instLen = sizeof(inst);
		return (l + instLen - 1) / instLen;
	}

	int codeSetImmediate(bool state);
	// int codeMe(Common::String *method, int numpar);

	inst readInst() { return getInst(_pc++); }
	inst getInst(uint pc) { return (*_currentScript)[pc]; }
	int readInt() { return getInt(_pc++); }
	int getInt(uint pc) { return (int)READ_UINT32(&((*_currentScript)[pc])); }
	double readFloat() { double d = getFloat(_pc); _pc += calcCodeAlignment(sizeof(double)); return d; }
	double getFloat(uint pc) { return *(double *)(&((*_currentScript)[pc])); }
	char *readString() { char *s = getString(_pc); _pc += calcStringAlignment(s); return s; }
	char *getString(uint pc) { return (char *)(&((*_currentScript)[pc])); }

	void pushVoid();

	void printSTUBWithArglist(const char *funcname, int nargs, const char *prefix = "STUB:");
	void convertVOIDtoString(int arg, int nargs);
	void dropStack(int nargs);
	void drop(uint num);

	void factoryCall(const Common::String &name, int nargs);

	void func_mci(const Common::String &name);
	void func_mciwait(const Common::String &name);
	void func_beep(int repeats);
	void func_goto(Datum &frame, Datum &movie);
	void func_gotoloop();
	void func_gotonext();
	void func_gotoprevious();
	void func_play(Datum &frame, Datum &movie);
	void func_playdone();
	void func_cursor(int cursorId, int maskId);
	int func_marker(int m);

	// lingo-the.cpp
public:
	void initTheEntities();
	void setTheEntity(int entity, Datum &id, int field, Datum &d);
	void setTheMenuItemEntity(int entity, Datum &menuId, int field, Datum &menuItemId, Datum &d);
	void setTheSprite(Datum &id, int field, Datum &d);
	void setTheCast(Datum &id, int field, Datum &d);
	Datum getTheEntity(int entity, Datum &id, int field);
	void setTheField(Datum &id1, int field, Datum &d);
	Datum getTheSprite(Datum &id, int field);
	Datum getTheCast(Datum &id, int field);
	Datum getTheField(Datum &id1, int field);
	Datum getObjectProp(Datum &obj, Common::String &propName);
	void setObjectProp(Datum &obj, Common::String &propName, Datum &d);
	Datum getTheMenuItemEntity(int entity, Datum &menuId, int field, Datum &menuItemId);
	const char *entity2str(int id);
	const char *field2str(int id);

private:
	Common::StringArray _entityNames;
	Common::StringArray _fieldNames;

// compiler resources
public:
	bool isInArgStack(Common::String *s);
	void clearArgStack();

	int code1(inst code) { _currentAssembly->push_back(code); return _currentAssembly->size() - 1; }
	int code2(inst code_1, inst code_2) { int o = code1(code_1); code1(code_2); return o; }
	int code3(inst code_1, inst code_2, inst code_3) { int o = code1(code_1); code1(code_2); code1(code_3); return o; }
	int code4(inst code_1, inst code_2, inst code_3, inst code_4) { int o = code1(code_1); code1(code_2); code1(code_3); code1(code_4); return o; }
	void codeArg(Common::String *s);
	Symbol codeDefine(Common::String &s, int start, int nargs, Object *obj = nullptr, int end = -1, bool removeCode = true);
	void codeFactory(Common::String &s);
	int codeFloat(double f);
	int codeFunc(Common::String *s, int numpar);
	int codeInt(int val);
	void codeLabel(int label);
	int codeString(const char *s);
	Symbol define(Common::String &s, int nargs, ScriptData *code, Common::Array<Common::String> *argNames = nullptr, Common::Array<Common::String> *varNames = nullptr, Object *obj = nullptr);
	void processIf(int toplabel, int endlabel);
	void varCreate(const Common::String &name, bool global, DatumHash *localvars = nullptr);

	int _assemblyArchive;
	ScriptContext *_assemblyContext;
	ScriptData *_currentAssembly;
	LexerDefineState _indef;
	LexerDefineState _indefStore;
	int _linenumber;
	int _colnumber;
	int _bytenumber;
	Common::String _lasttoken;
	int _lastbytenumber;
	Common::String _errortoken;
	int _errorbytenumber;
	bool _ignoreError;
	bool _inFactory;
	Object *_currentFactory;
	Common::Array<RepeatBlock *> _repeatStack;

	Common::Array<Common::String *> _argstack;
	Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *_methodVars;
	Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *_methodVarsStash;

public:
	uint16 _currentEntityId;
	int _currentChannelId;
	ScriptContext *_currentScriptContext;
	ScriptData *_currentScript;
	Datum _currentMe;

	bool _abort;
	bool _immediateMode;

	Common::Array<CFrame *> _callstack;
	TheEntityHash _theEntities;
	TheEntityFieldHash _theEntityFields;

	int _objectEntityId;

	Common::Array<int> _labelstack;

	SymbolHash _builtins;
	SymbolHash _methods;
	SymbolHash _xlibInitializers;

	Common::String _floatPrecisionFormat;

	bool _hadError;

	bool _inCond;

	bool _cursorOnStack;

private:
	int parse(const char *code);
	void parseMenu(const char *code);

public:
	Common::String genMenuHandler(int *commandId, Common::String &command);

public:
	void push(Datum d);
	Datum pop(void);
	Datum peek(uint offset);

public:
	Common::HashMap<uint32, const char *> _eventHandlerTypes;
	Common::HashMap<Common::String, uint32, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _eventHandlerTypeIds;
	Common::HashMap<Common::String, Audio::AudioStream *> _audioAliases;

	DatumHash _globalvars;
	DatumHash *_localvars;

	FuncHash _functions;

	Common::HashMap<int, LingoV4Bytecode *> _lingoV4;
	Common::HashMap<int, LingoV4TheEntity *> _lingoV4TheEntity;

	LingoArchive _archives[2];
	int _archiveIndex;

	uint _pc;

	StackData _stack;

	DirectorEngine *_vm;

	int _floatPrecision;

	bool _dontPassEvent;

	Datum _perFrameHook;

public:
	void executeImmediateScripts(Frame *frame);
	void executePerFrameHook(int frame, int subframe);
};

extern Lingo *g_lingo;

} // End of namespace Director

#endif
