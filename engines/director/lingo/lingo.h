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

#ifndef DIRECTOR_LINGO_LINGO_H
#define DIRECTOR_LINGO_LINGO_H

#include "common/hash-ptr.h"
#include "common/hash-str.h"
#include "common/str-array.h"
#include "common/queue.h"
#include "common/rect.h"

#include "director/types.h"

namespace Audio {
class AudioStream;
}
namespace Common {
class SeekableReadStreamEndian;
}

namespace Director {

struct ChunkReference;
struct MenuReference;
struct TheEntity;
struct TheEntityField;
struct LingoArchive;
struct LingoV4Bytecode;
struct LingoV4TheEntity;
struct Node;
class AbstractObject;
class Cast;
class ScriptContext;
class DirectorEngine;
class Frame;
class LingoCompiler;

typedef void (*inst)(void);
#define	STOP (inst)0
#define ENTITY_INDEX(t,id) ((t) * 100000 + (id))
#define printWithArgList g_lingo->printSTUBWithArglist

int calcStringAlignment(const char *s);
int calcCodeAlignment(int l);

typedef Common::Array<inst> ScriptData;

struct FuncDesc {
	Common::String name;
	const char *proto;

	FuncDesc(Common::String n, const char *p) { name = n; proto = p; }
};

typedef Common::HashMap<void *, FuncDesc *> FuncHash;

struct BuiltinProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int version;
	SymbolType type;
};

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
	int targetType;	/* valid target objects, for method builtins */

	Common::Array<Common::String> *argNames;
	Common::Array<Common::String> *varNames;
	ScriptContext *ctx;		/* optional script context to execute with */
	AbstractObject *target;			/* optional method target */
	bool anonymous;

	Symbol();
	Symbol(const Symbol &s);
	Symbol& operator=(const Symbol &s);
	void reset();
	~Symbol();
};

struct PArray {
	bool _sorted;
	PropertyArray arr;

	PArray() : _sorted(false) {}

	PArray(int size) : _sorted(false), arr(size) {}
};

struct FArray {
	bool _sorted;
	DatumArray arr;

	FArray() : _sorted(false) {}

	FArray(int size) : _sorted(false), arr(size) {}
};


struct Datum {	/* interpreter stack type */
	DatumType type;

	union {
		int	i;				/* INT, ARGC, ARGCNORET */
		double f;			/* FLOAT */
		Common::String *s;	/* STRING, VARREF, OBJECT */
		FArray *farr;	/* ARRAY, POINT, RECT */
		PArray *parr; /* PARRAY */
		AbstractObject *obj; /* OBJECT */
		ChunkReference *cref; /* CHUNKREF */
		CastMemberID *cast;	/* CASTREF, FIELDREF */
		MenuReference *menu; /* MENUREF	*/
	} u;

	int *refCount;

	Datum();
	Datum(const Datum &d);
	Datum& operator=(const Datum &d);
	Datum(int val);
	Datum(double val);
	Datum(const Common::String &val);
	Datum(AbstractObject *val);
	Datum(const CastMemberID &val);
	Datum(const Common::Rect &rect);
	void reset();

	~Datum() {
		reset();
	}

	Datum eval() const;
	double asFloat() const;
	int asInt() const;
	Common::String asString(bool printonly = false) const;
	CastMemberID asMemberID() const;
	Common::Point asPoint() const;

	bool isRef() const;
	bool isVarRef() const;
	bool isCastRef() const;

	const char *type2str(bool isk = false) const;

	int equalTo(Datum &d, bool ignoreCase = false) const;
	CompareResult compareTo(Datum &d) const;

	bool operator==(Datum &d) const;
	bool operator>(Datum &d) const;
	bool operator<(Datum &d) const;
	bool operator>=(Datum &d) const;
	bool operator<=(Datum &d) const;
};

struct ChunkReference {
	Datum source;
	ChunkType type;
	int startChunk;
	int endChunk;
	int start;
	int end;

	ChunkReference(const Datum &src, ChunkType t, int sc, int ec, int s, int e)
		: source(src), type(t), startChunk(sc), endChunk(ec), start(s), end(e) {}
};

struct MenuReference {
	int menuIdNum;
	Common::String *menuIdStr;
	int menuItemIdNum;
	Common::String *menuItemIdStr;

	MenuReference();
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
typedef void (*XLibFunc)(int);
typedef Common::HashMap<Common::String, XLibFunc, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> XLibFuncHash;
typedef Common::HashMap<Common::String, ObjectType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> OpenXLibsHash;

typedef Common::HashMap<Common::String, TheEntity *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityHash;
typedef Common::HashMap<Common::String, TheEntityField *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityFieldHash;

struct CFrame {	/* proc/func call stack frame */
	Symbol			sp;					/* symbol table entry */
	int				retPC;				/* where to resume after return */
	ScriptData		*retScript;			/* which script to resume after return */
	ScriptContext	*retContext;		/* which script context to use after return */
	bool			retFreezeContext;	/* whether the context should be frozen after return */
	DatumHash		*retLocalVars;
	Datum			retMe;				/* which me obj to use after return */
	uint			stackSizeBefore;
	bool			allowRetVal;		/* whether to allow a return value */
	Datum			defaultRetVal;		/* default return value */
};

struct LingoEvent {
	LEvent event;
	int eventId;
	ScriptType scriptType;
	CastMemberID scriptId;
	bool passByDefault;
	int channelId;

	LingoEvent (LEvent e, int ei, ScriptType st, CastMemberID si, bool pass, int ci = -1) {
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

	void addCode(const Common::U32String &code, ScriptType type, uint16 id, const char *scriptName = nullptr);
	void removeCode(ScriptType type, uint16 id);
	void replaceCode(const Common::U32String &code, ScriptType type, uint16 id, const char *scriptName = nullptr);
	void addCodeV4(Common::SeekableReadStreamEndian &stream, uint16 lctxIndex, const Common::String &archName, uint16 version);
	void addNamesV4(Common::SeekableReadStreamEndian &stream);
};

class Lingo {

public:
	Lingo(DirectorEngine *vm);
	~Lingo();

	void resetLingo();
	int getMenuNum();
	int getMenuItemsNum(Datum &d);

	void executeHandler(const Common::String &name);
	void executeScript(ScriptType type, CastMemberID id);
	void printStack(const char *s, uint pc);
	void printCallStack(uint pc);
	Common::String decodeInstruction(ScriptData *sd, uint pc, uint *newPC = NULL);

	void reloadBuiltIns();
	void initBuiltIns();
	void initBuiltIns(BuiltinProto protos[]);
	void cleanupBuiltIns();
	void cleanupBuiltIns(BuiltinProto protos[]);
	void initFuncs();
	void cleanupFuncs();
	void initBytecode();
	void initMethods();
	void cleanupMethods();
	void initXLibs();
	void cleanupXLibs();

	Common::String normalizeXLibName(Common::String name);
	void openXLib(Common::String name, ObjectType type);
	void closeXLib(Common::String name);
	void closeOpenXLibs();
	void reloadOpenXLibs();

	void runTests();

	// lingo-events.cpp
private:
	void initEventHandlerTypes();
	void processEvent(LEvent event, ScriptType st, CastMemberID scriptId, int channelId = -1);

public:
	ScriptType event2script(LEvent ev);
	Symbol getHandler(const Common::String &name);

	void processEvents(Common::Queue<LingoEvent> &queue);

public:
	void execute();
	void loadStateFromWindow();
	void saveStateToWindow();
	void pushContext(const Symbol funcSym, bool allowRetVal, Datum defaultRetVal);
	void popContext(bool aborting = false);
	bool hasFrozenContext();
	void cleanLocalVars();
	void varAssign(const Datum &var, const Datum &value);
	Datum varFetch(const Datum &var, bool silent = false);
	Common::U32String evalChunkRef(const Datum &var);
	Datum findVarV4(int varType, const Datum &id);
	CastMemberID resolveCastMember(const Datum &memberID, const Datum &castLib);

	int getAlignedType(const Datum &d1, const Datum &d2, bool numsOnly);

	void printAllVars();

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
	void func_cursor(Datum cursorDatum);
	int func_marker(int m);
	uint16 func_label(Datum &label);

	// lingo-the.cpp
public:
	void initTheEntities();
	void cleanUpTheEntities();
	const char *entity2str(int id);
	const char *field2str(int id);

	// global kTheEntity
	Datum _actorList;
	Common::u32char_type_t _itemDelimiter;
	bool _exitLock;
	bool _preLoadEventAbort; // no-op, everything is always preloaded
	Datum _searchPath;
	bool _trace;	// state of movie's trace function
	int _traceLoad; // internal Director verbosity level
	bool _updateMovieEnabled;
	bool _romanLingo;

	Datum getTheEntity(int entity, Datum &id, int field);
	void setTheEntity(int entity, Datum &id, int field, Datum &d);
	Datum getTheSprite(Datum &id, int field);
	void setTheSprite(Datum &id, int field, Datum &d);
	Datum getTheCast(Datum &id, int field);
	void setTheCast(Datum &id, int field, Datum &d);
	Datum getTheField(Datum &id1, int field);
	void setTheField(Datum &id1, int field, Datum &d);
	Datum getTheChunk(Datum &chunk, int field);
	void setTheChunk(Datum &chunk, int field, Datum &d);
	void getObjectProp(Datum &obj, Common::String &propName);
	void setObjectProp(Datum &obj, Common::String &propName, Datum &d);
	Datum getTheDate(int field);
	Datum getTheTime(int field);

private:
	Common::StringArray _entityNames;
	Common::StringArray _fieldNames;

public:
	LingoCompiler *_compiler;

	int _currentChannelId;
	ScriptContext *_currentScriptContext;
	ScriptData *_currentScript;
	Datum _currentMe;

	bool _freezeContext;
	bool _abort;
	bool _expectError;
	bool _caughtError;

	TheEntityHash _theEntities;
	TheEntityFieldHash _theEntityFields;

	int _objectEntityId;

	SymbolHash _builtinCmds;
	SymbolHash _builtinFuncs;
	SymbolHash _builtinConsts;
	SymbolHash _methods;
	XLibFuncHash _xlibOpeners;
	XLibFuncHash _xlibClosers;

	OpenXLibsHash _openXLibs;

	Common::String _floatPrecisionFormat;

public:
	void push(Datum d);
	Datum pop();
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

	// lingo-utils.cpp
private:
	Common::HashMap<uint32, Common::U32String> _charNormalizations;
	void initCharNormalizations();

public:
	Common::String normalizeString(const Common::String &str);
};

extern Lingo *g_lingo;

} // End of namespace Director

#endif
