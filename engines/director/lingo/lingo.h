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

#include "common/hash-ptr.h"
#include "common/hash-str.h"

#include "director/types.h"

namespace Audio {
class AudioStream;
}
namespace Common {
class SeekableReadStreamEndian;
}

namespace Director {

struct ChunkReference;
struct TheEntity;
struct TheEntityField;
struct LingoArchive;
struct LingoV4Bytecode;
struct LingoV4TheEntity;
class AbstractObject;
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
	SymbolType type;
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
	LingoArchive *archive; 	/* optional archive to execute with */
	AbstractObject *target;			/* optional method target */
	bool anonymous;

	Symbol();
	Symbol(const Symbol &s);
	Symbol& operator=(const Symbol &s);
	void reset();
	~Symbol();
};

struct Datum {	/* interpreter stack type */
	int type;

	union {
		int	i;				/* INT, ARGC, ARGCNORET */
		double f;			/* FLOAT */
		Common::String *s;	/* STRING, VAR, OBJECT */
		DatumArray *farr;	/* ARRAY, POINT, RECT */
		PropertyArray *parr; /* PARRAY */
		AbstractObject *obj; /* OBJECT */
		ChunkReference *cref; /* CHUNKREF */
	} u;

	int *refCount;

	Datum();
	Datum(const Datum &d);
	Datum& operator=(const Datum &d);
	Datum(int val);
	Datum(double val);
	Datum(const Common::String &val);
	Datum(AbstractObject *val);
	void reset();

	~Datum() {
		reset();
	}

	Datum eval();
	double asFloat() const;
	int asInt() const;
	Common::String asString(bool printonly = false) const;
	int asCastId() const;

	const char *type2str(bool isk = false) const;

	int equalTo(Datum &d, bool ignoreCase = false) const;
	int compareTo(Datum &d, bool ignoreCase = false) const;
};

struct ChunkReference {
	Datum source;
	int start;
	int end;

	ChunkReference(const Datum &src, uint s, uint e) : source(src), start(s), end(e) {}
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

struct CFrame {	/* proc/func call stack frame */
	Symbol	sp;	/* symbol table entry */
	int		retpc;	/* where to resume after return */
	ScriptData *retscript;		/* which script to resume after return */
	ScriptContext *retctx;		/* which script context to use after return */
	LingoArchive *retarchive;	/* which archive to use after return */
	DatumHash *localvars;
	Datum retMe; /* which me obj to use after return */
	uint stackSizeBefore;
	bool allowRetVal;			/* whether to allow a return value */
	Datum defaultRetVal;		/* default return value */
};

struct LingoEvent {
	LEvent event;
	int eventId;
	ScriptType scriptType;
	int scriptId;
	bool passByDefault;
	int channelId;

	LingoEvent (LEvent e, int ei, ScriptType st, int si, bool pass, int ci = -1) {
		event = e;
		eventId = ei;
		scriptType = st;
		scriptId = si;
		passByDefault = pass;
		channelId = ci;
	}
};


struct LingoArchive {
	LingoArchive(Cast *c) : cast(c) {};
	~LingoArchive();

	Cast *cast;
	ScriptContextHash lctxContexts;
	ScriptContextHash scriptContexts[kMaxScriptType + 1];
	Common::Array<Common::String> names;
	Common::HashMap<uint32, Common::String> primaryEventHandlers;
	SymbolHash functionHandlers;

	ScriptContext *getScriptContext(ScriptType type, uint16 id);
	Common::String getName(uint16 id);

	void addCode(const char *code, ScriptType type, uint16 id, const char *scriptName = nullptr);
	void addCodeV4(Common::SeekableReadStreamEndian &stream, uint16 lctxIndex, const Common::String &archName);
	void addNamesV4(Common::SeekableReadStreamEndian &stream);
};

struct RepeatBlock {
	Common::Array<uint32> exits;
	Common::Array<uint32> nexts;
};

class Lingo {

public:
	Lingo(DirectorEngine *vm);
	~Lingo();

	void resetLingo();

	ScriptContext *compileAnonymous(const char *code);
	ScriptContext *compileLingo(const char *code, LingoArchive *archive, ScriptType type, uint16 id, const Common::String &scriptName, bool anonyomous = false);
	ScriptContext *compileLingoV4(Common::SeekableReadStreamEndian &stream, LingoArchive *archive, const Common::String &archName);
	void executeHandler(const Common::String &name);
	void executeScript(ScriptType type, uint16 id);
	void printStack(const char *s, uint pc);
	void printCallStack(uint pc);
	Common::String decodeInstruction(LingoArchive *archive, ScriptData *sd, uint pc, uint *newPC = NULL);

	void reloadBuiltIns();
	void initBuiltIns();
	void cleanupBuiltIns();
	void initFuncs();
	void cleanupFuncs();
	void initBytecode();
	void initMethods();
	void cleanupMethods();
	void initXLibs();
	void cleanupXLibs();
	void openXLib(Common::String name, ObjectType type);

	void runTests();

	// lingo-preprocessor.cpp
public:
	Common::String codePreprocessor(const char *s, LingoArchive *archive, ScriptType type, uint16 id, bool simple = false);

	// lingo-patcher.cpp
	Common::String patchLingoCode(Common::String &line, LingoArchive *archive, ScriptType type, uint16 id, int linenumber);

	// lingo.cpp
private:
	const char *findNextDefinition(const char *s);

	// lingo-events.cpp
private:
	void initEventHandlerTypes();
	void processEvent(LEvent event, ScriptType st, int entityId, int channelId = -1);

public:
	ScriptType event2script(LEvent ev);
	Symbol getHandler(const Common::String &name);

	void processEvents();

public:
	void execute(uint pc);
	void pushContext(const Symbol funcSym, bool allowRetVal, Datum defaultRetVal);
	void popContext();
	void cleanLocalVars();
	void varAssign(Datum &var, Datum &value, bool global = false, DatumHash *localvars = nullptr);
	Datum varFetch(Datum &var, bool global = false, DatumHash *localvars = nullptr, bool silent = false);
	Datum findVarV4(int varType, const Datum &id);

	int getAlignedType(const Datum &d1, const Datum &d2, bool numsOnly);

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
	int getInt(uint pc);
	double readFloat() { double d = getFloat(_pc); _pc += calcCodeAlignment(sizeof(double)); return d; }
	double getFloat(uint pc) { return *(double *)(&((*_currentScript)[pc])); }
	char *readString() { char *s = getString(_pc); _pc += calcStringAlignment(s); return s; }
	char *getString(uint pc) { return (char *)(&((*_currentScript)[pc])); }

	void pushVoid();

	void printSTUBWithArglist(const char *funcname, int nargs, const char *prefix = "STUB:");
	void convertVOIDtoString(int arg, int nargs);
	void dropStack(int nargs);
	void drop(uint num);

	void lingoError(const char *s, ...);

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
	uint16 func_label(Datum &label);

	// lingo-the.cpp
public:
	void initTheEntities();
	void cleanUpTheEntities();
	const char *entity2str(int id);
	const char *field2str(int id);

	// global kTheEntity
	char _itemDelimiter;

	Datum getTheEntity(int entity, Datum &id, int field);
	void setTheEntity(int entity, Datum &id, int field, Datum &d);
	Datum getTheMenuItemEntity(int entity, Datum &menuId, int field, Datum &menuItemId);
	void setTheMenuItemEntity(int entity, Datum &menuId, int field, Datum &menuItemId, Datum &d);
	Datum getTheSprite(Datum &id, int field);
	void setTheSprite(Datum &id, int field, Datum &d);
	Datum getTheCast(Datum &id, int field);
	void setTheCast(Datum &id, int field, Datum &d);
	Datum getTheField(Datum &id1, int field);
	void setTheField(Datum &id1, int field, Datum &d);
	Datum getObjectProp(Datum &obj, Common::String &propName);
	void setObjectProp(Datum &obj, Common::String &propName, Datum &d);
	Datum getTheDate(int field);
	Datum getTheTime(int field);

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
	int codeCmd(Common::String *s, int numpar);
	Symbol codeDefine(Common::String &s, int start, int nargs, int end = -1, bool removeCode = true);
	void codeFactory(Common::String &s);
	int codeFloat(double f);
	int codeFunc(Common::String *s, int numpar);
	int codeInt(int val);
	void codeLabel(int label);
	int codeString(const char *s);
	void processIf(int toplabel, int endlabel);
	void varCreate(const Common::String &name, bool global, DatumHash *localvars = nullptr);

	LingoArchive *_assemblyArchive;
	ScriptContext *_assemblyContext;
	ScriptData *_currentAssembly;
	LexerDefineState _indef;
	LexerDefineState _indefStore;
	uint _linenumber;
	uint _colnumber;
	uint _bytenumber;
	const char *_lines[3];
	bool _inFactory;
	Common::Array<RepeatBlock *> _repeatStack;

	Common::Array<Common::String *> _argstack;
	Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *_methodVars;
	Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> *_methodVarsStash;

public:
	int _currentChannelId;
	LingoArchive *_currentArchive;
	ScriptContext *_currentScriptContext;
	ScriptData *_currentScript;
	Datum _currentMe;

	bool _abort;
	bool _immediateMode;
	bool _expectError;
	bool _caughtError;

	Common::Array<CFrame *> _callstack;
	TheEntityHash _theEntities;
	TheEntityFieldHash _theEntityFields;

	int _objectEntityId;

	Common::Array<int> _labelstack;

	SymbolHash _builtinCmds;
	SymbolHash _builtinFuncs;
	SymbolHash _builtinConsts;
	SymbolHash _methods;
	SymbolHash _xlibInitializers;

	Common::String _floatPrecisionFormat;

	bool _hadError;

	bool _inCond;

private:
	int parse(const char *code);
	void parseMenu(const char *code);

public:
	void push(Datum d);
	Datum pop(bool eval = true);
	Datum peek(uint offset, bool eval = true);

public:
	Common::HashMap<uint32, const char *> _eventHandlerTypes;
	Common::HashMap<Common::String, uint32, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _eventHandlerTypeIds;
	Common::HashMap<Common::String, Audio::AudioStream *> _audioAliases;

	DatumHash _globalvars;
	DatumHash *_localvars;

	FuncHash _functions;

	Common::HashMap<int, LingoV4Bytecode *> _lingoV4;
	Common::HashMap<int, LingoV4TheEntity *> _lingoV4TheEntity;

	uint _globalCounter;
	uint _pc;

	StackData _stack;

	DirectorEngine *_vm;

	int _floatPrecision;

	Datum _theResult;

	// events
	bool _passEvent;
	Datum _perFrameHook;

	Datum _windowList;

public:
	void executeImmediateScripts(Frame *frame);
	void executePerFrameHook(int frame, int subframe);
};

extern Lingo *g_lingo;

} // End of namespace Director

#endif
