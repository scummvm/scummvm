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
struct ScriptContext;
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
		int		i;			/* VAR */
		double	f;			/* FLOAT */
		ScriptData	*defn;	/* HANDLER */
		void (*func)();		/* OPCODE */
		void (*bltin)(int);	/* BUILTIN */
		Common::String	*s;	/* STRING */
		DatumArray *farr;	/* ARRAY, POINT, RECT */
		PropertyArray *parr;
		Object *obj;
	} u;

	int *refCount;

	int nargs;		/* number of arguments */
	int maxArgs;	/* maximal number of arguments, for builtins */
	bool parens;	/* whether parens required or not, for builitins */
	int methodType;	/* valid target objects, for method builtins */

	bool global;
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
	void reset() {
		*refCount -= 1;
		if (*refCount <= 0) {
			switch (type) {
			case VAR:
				// fallthrough
			case STRING:
				delete u.s;
				break;
			case ARRAY:
				// fallthrough
			case POINT:
				// fallthrough
			case RECT:
				delete u.farr;
				break;
			case PARRAY:
				delete u.parr;
				break;
			case REFERENCE:
				// fallthrough
			case INT:
				// fallthrough
			case FLOAT:
				// fallthrough
			default:
				break;
			}
			delete refCount;
		}

	}

	~Datum() {
		reset();
	}

	Datum eval();
	double asFloat();
	int asInt();
	Common::String asString(bool printonly = false);

	const char *type2str(bool isk = false);

	int compareTo(Datum &d, bool ignoreCase = false);
};

struct PCell {
	Datum p;
	Datum v;

	PCell();
	PCell(Datum &prop, Datum &val);
};

struct Builtin {
	void (*func)(void);
	int nargs;

	Builtin(void (*func1)(void), int nargs1) : func(func1), nargs(nargs1) {}
};

struct ScriptContext {
	Common::Array<Symbol> functions;
	Common::Array<Datum> constants;
};

typedef Common::HashMap<int32, ScriptContext *> ScriptContextHash;
typedef Common::Array<Datum> StackData;
typedef Common::HashMap<Common::String, Symbol, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SymbolHash;
typedef Common::HashMap<Common::String, Builtin *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> BuiltinHash;

typedef Common::HashMap<Common::String, TheEntity *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityHash;
typedef Common::HashMap<Common::String, TheEntityField *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityFieldHash;

enum ObjectType {
	kNoneObj = 0,
	kFactoryObj = 1 << 0,
	kXObj = 1 << 1,
	kScriptObj = 1 << 2
};

struct Object {
	Common::String *name;
	ObjectType type;
	bool disposed;

	Object *prototype;
	SymbolHash properties;
	SymbolHash methods;
	int inheritanceLevel; // 1 for original object
	ScriptContext *scriptContext;

	// used only for factories
	Common::HashMap<uint32, Datum> *objArray;

	Object(const Common::String &objName, ObjectType objType, ScriptContext *objContext = nullptr) {
		name = new Common::String(objName);
		type = objType;
		disposed = false;
		inheritanceLevel = 1;
		scriptContext = objContext;
		if (objType == kFactoryObj) {
			objArray = new Common::HashMap<uint32, Datum>;
		} else {
			objArray = nullptr;
		}
	}

	Object *clone();
	void addProperty(const Common::String &propName);
	Symbol getMethod(const Common::String &methodName);
	bool hasVar(const Common::String &varName);
	Symbol &getVar(const Common::String &varName);
};

struct CFrame {	/* proc/func call stack frame */
	Symbol	sp;	/* symbol table entry */
	int		retpc;	/* where to resume after return */
	ScriptData	*retscript;	 /* which script to resume after return */
	ScriptContext	*retctx;   /* which script context to use after return */
	int 	retarchive;	/* which archive to use after return */
	SymbolHash *localvars;
	Object *retMeObj; /* which me obj to use after return */
};

struct LingoEvent {
	LEvent event;
	ScriptType st;
	int entityId;
	int channelId;

	LingoEvent (LEvent e, ScriptType s, int ei, int ci = -1) {
		event = e;
		st = s;
		entityId = ei;
		channelId = ci;
	}
};


struct LingoArchive {
	ScriptContextHash scriptContexts[kMaxScriptType + 1];
	Common::Array<Common::String> names;
	Common::HashMap<uint32, Symbol> eventHandlers;
	Common::HashMap<uint32, Common::String> primaryEventHandlers;
	SymbolHash functionHandlers;
};

class Lingo {

public:
	Lingo(DirectorEngine *vm);
	~Lingo();

	void restartLingo();

	void addCode(const char *code, ScriptType type, uint16 id);
	void addCodeV4(Common::SeekableSubReadStreamEndian &stream, ScriptType type, uint16 id, Common::String &archName);
	void addNamesV4(Common::SeekableSubReadStreamEndian &stream);
	void executeHandler(Common::String name);
	void executeScript(ScriptType type, uint16 id, uint16 function);
	void printStack(const char *s, uint pc);
	void printCallStack(uint pc);
	Common::String decodeInstruction(ScriptData *sd, uint pc, uint *newPC = NULL);

	void initBuiltIns();
	void cleanupBuiltins();
	void initFuncs();
	void initBytecode();
	void initMethods();

	void runTests();

	// lingo-preprocessor.cpp
public:
	Common::String codePreprocessor(const char *s, ScriptType type, uint16 id, bool simple = false);

	// lingo-patcher.cpp
	Common::String patchLingoCode(Common::String &line, ScriptType type, uint16 id, int linenumber);

	// lingo.cpp
private:
	const char *findNextDefinition(const char *s);

	// lingo-events.cpp
private:
	void initEventHandlerTypes();
	void setPrimaryEventHandler(LEvent event, const Common::String &code);
	void primaryEventHandler(LEvent event);
	void registerInputEvent(LEvent event);
	void registerFrameEvent(LEvent event);
	void registerGenericEvent(LEvent event);
	void runMovieScript(LEvent event);
	void registerSpriteEvent(LEvent event);
	void processEvent(LEvent event, ScriptType st, int entityId, int channelId = -1);

	Common::Queue<LingoEvent> _eventQueue;

public:
	ScriptContext *getScriptContext(ScriptType type, uint16 id);
	Common::String getName(uint16 id);
	ScriptType event2script(LEvent ev);
	Symbol getHandler(const Common::String &name);

	int getEventCount();
	void processEvent(LEvent event);
	void processEvents();
	void registerEvent(LEvent event);

public:
	void execute(uint pc);
	void pushContext(const Symbol *funcSym = nullptr);
	void popContext();
	void cleanLocalVars();
	int castIdFetch(Datum &var);
	void varAssign(Datum &var, Datum &value, bool global = false, SymbolHash *localvars = nullptr);
	Datum varFetch(Datum &var, bool global = false, SymbolHash *localvars = nullptr);

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
	Datum getTheSprite(Datum &id, int field);
	Datum getTheCast(Datum &id, int field);
	Datum getObjectField(Common::String &obj, int field);
	void setObjectField(Common::String &obj, int field, Datum &d);
	Datum getObjectRef(Common::String &obj, Common::String &field);
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
	void varCreate(const Common::String &name, bool global, SymbolHash *localvars = nullptr);

	ScriptData *_currentAssembly;
	LexerDefineState _indef;
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

	Common::Array<Common::String *> _argstack;
	Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *_methodVars;
	Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *_methodVarsStash;

public:
	uint16 _currentEntityId;
	int _currentChannelId;
	ScriptContext *_currentScriptContext;
	ScriptData *_currentScript;
	Object *_currentMeObj;

	bool _abort;
	bool _nextRepeat;
	bool _immediateMode;

	Common::Array<CFrame *> _callstack;
	TheEntityHash _theEntities;
	TheEntityFieldHash _theEntityFields;

	int _objectEntityId;

	Common::Array<int> _labelstack;

	SymbolHash _builtins;
	SymbolHash _methods;
	SymbolHash _xlibs;

	Common::String _floatPrecisionFormat;

	bool _hadError;

	bool _inCond;

	bool _exitRepeat;

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

	SymbolHash _globalvars;
	SymbolHash *_localvars;

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
