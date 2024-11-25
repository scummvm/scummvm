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

namespace Audio {
class AudioStream;
}
namespace Common {
class SeekableReadStreamEndian;
}

namespace Director {

struct ChunkReference;
struct MenuReference;
struct PictureReference;
struct TheEntity;
struct TheEntityField;
struct LingoArchive;
struct LingoV4Bytecode;
struct LingoV4TheEntity;
struct Node;
struct Picture;
class AbstractObject;
class Cast;
class ScriptContext;
class DirectorEngine;
class Frame;
class LingoCompiler;
struct Breakpoint;

typedef void (*inst)(void);
#define	STOP (inst)0
#define ENTITY_INDEX(t,id) ((t) * 100000 + (id))

int calcStringAlignment(const char *s);
int calcCodeAlignment(int l);

typedef Common::Array<inst> ScriptData;

struct FuncDesc {
	Common::String name;
	const char *proto;

	FuncDesc(Common::String n, const char *p) { name = n; proto = p; }
};

typedef Common::HashMap<void *, FuncDesc *> FuncHash;

typedef Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> MethodHash;

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
	bool operator==(Symbol &s) const;
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
		PictureReference *picture; /* PICTUREREF */
	} u;

	int *refCount;

	bool ignoreGlobal; // True if this Datum should be ignored by showGlobals and clearGlobals

	Datum();
	Datum(const Datum &d);
	Datum& operator=(const Datum &d);
	Datum(int val);
	Datum(double val);
	Datum(const Common::String &val);
	Datum(AbstractObject *val);
	Datum(const CastMemberID &val);
	Datum(const Common::Point &point);
	Datum(const Common::Rect &rect);
	void reset();

	~Datum() {
		reset();
	}

	Datum eval() const;
	double asFloat() const;
	int asInt() const;
	Common::String asString(bool printonly = false) const;
	CastMemberID asMemberID(CastType castType = kCastTypeAny, int castLib = 0) const;
	Common::Point asPoint() const;
	Datum clone() const;

	bool isRef() const;
	bool isVarRef() const;
	bool isCastRef() const;
	bool isArray() const;
	bool isNumeric() const;

	const char *type2str(bool ilk = false) const;

	int equalTo(Datum &d, bool ignoreCase = false) const;
	uint32 compareTo(Datum &d) const;

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

struct PictureReference {
	Picture *_picture = nullptr;
	~PictureReference();
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
typedef Common::HashMap<int32, Common::HashMap<Common::String, ScriptContext *> *> FactoryContextHash;
typedef Common::Array<Datum> StackData;
typedef Common::HashMap<Common::String, Symbol, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SymbolHash;
typedef Common::HashMap<Common::String, Datum, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> DatumHash;
typedef Common::HashMap<Common::String, Builtin *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> BuiltinHash;
typedef Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> VarTypeHash;
typedef void (*XLibOpenerFunc)(ObjectType, const Common::Path &);
typedef void (*XLibCloserFunc)(ObjectType);
typedef Common::HashMap<Common::String, XLibOpenerFunc, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> XLibOpenerFuncHash;
typedef Common::HashMap<Common::String, XLibCloserFunc, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> XLibCloserFuncHash;
typedef Common::HashMap<Common::String, ObjectType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> OpenXLibsHash;
typedef Common::HashMap<Common::String, AbstractObject *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> OpenXLibsStateHash;

typedef Common::HashMap<Common::String, const TheEntity *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityHash;
typedef Common::HashMap<Common::String, const TheEntityField *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityFieldHash;

struct CFrame {	/* proc/func call stack frame */
	Symbol			sp;					/* symbol table entry */
	int				retPC;				/* where to resume after return */
	ScriptData		*retScript;			/* which script to resume after return */
	ScriptContext	*retContext;		/* which script context to use after return */
	DatumHash		*retLocalVars;
	Datum			retMe;				/* which me obj to use after return */
	uint			stackSizeBefore;
	bool			allowRetVal;		/* whether to allow a return value */
	Datum			defaultRetVal;		/* default return value */
	int				paramCount;			/* original number of arguments submitted */
	Common::Array<Datum> paramList;		/* original argument list */
};

struct LingoEvent {
	LEvent event;
	int eventId;
	EventHandlerSourceType eventHandlerSourceType;
	ScriptType scriptType;
	bool passByDefault;
	uint16 channelId;
	CastMemberID scriptId;
	Common::Point mousePos;

	LingoEvent (LEvent e, int ei, ScriptType st, bool pass, CastMemberID si = CastMemberID(), Common::Point mp = Common::Point(-1, -1)) {
		event = e;
		eventId = ei;
		eventHandlerSourceType = kNoneHandler;
		scriptType = st;
		passByDefault = pass;
		channelId = 0;
		scriptId = si;
		mousePos = mp;
	}

	LingoEvent (LEvent e, int ei, EventHandlerSourceType ehst, bool pass, Common::Point mp = Common::Point(-1, -1)) {
		event = e;
		eventId = ei;
		eventHandlerSourceType = ehst;
		scriptType = kNoneScript;
		passByDefault = pass;
		channelId = 0;
		scriptId = CastMemberID();
		mousePos = mp;
	}
};


struct LingoArchive {
	LingoArchive(Cast *c) : cast(c) {};
	~LingoArchive();

	Cast *cast;
	ScriptContextHash lctxContexts;
	ScriptContextHash scriptContexts[kMaxScriptType + 1];
	FactoryContextHash factoryContexts;
	Common::Array<Common::String> names;
	Common::HashMap<uint32, Common::String> primaryEventHandlers;
	SymbolHash functionHandlers;

	ScriptContext *getScriptContext(ScriptType type, uint16 id);
	ScriptContext *findScriptContext(uint16 id);
	Common::String getName(uint16 id);
	Common::String formatFunctionList(const char *prefix);

	void addCode(const Common::U32String &code, ScriptType type, uint16 id, const char *scriptName = nullptr, uint32 preprocFlags = kLPPNone);
	void patchCode(const Common::U32String &code, ScriptType type, uint16 id, const char *scriptName = nullptr, uint32 preprocFlags = kLPPNone);
	void removeCode(ScriptType type, uint16 id);
	void replaceCode(const Common::U32String &code, ScriptType type, uint16 id, const char *scriptName = nullptr);
	void addCodeV4(Common::SeekableReadStreamEndian &stream, uint16 lctxIndex, const Common::String &archName, uint16 version);
	void addNamesV4(Common::SeekableReadStreamEndian &stream);

	// lingo-patcher.cpp
	void patchScriptHandler(ScriptType type, CastMemberID id);
};

struct LingoState {
	// Execution state for a Lingo process, created every time
	// a top-level handler is called (e.g. on mouseDown).
	// Can be swapped out when another script gets called with priority.
	// Call frames are pushed and popped from the callstack with
	// pushContext and popContext.
	Common::Array<CFrame *> callstack;		// call stack
	uint pc = 0;							// current program counter
	ScriptData *script = nullptr;			// current Lingo script
	ScriptContext *context = nullptr;		// current Lingo script context
	DatumHash *localVars = nullptr;			// current local variables
	Datum me;								// current me object

	~LingoState();
};

enum LingoExecState {
	kRunning,
	kPause,
};

class Lingo {

public:
	Lingo(DirectorEngine *vm);
	~Lingo();

	void resetLingo();
	void cleanupLingo();
	void resetLingoGo();

	int getMenuNum();
	int getMenuItemsNum(Datum &d);
	int getXtrasNum();
	int getCastLibsNum();
	int getMembersNum();

	void executeHandler(const Common::String &name);
	void executeScript(ScriptType type, CastMemberID id);
	Common::String formatStack();
	void printStack(const char *s, uint pc);
	Common::String formatCallStack(uint pc);
	void printCallStack(uint pc);
	Common::String formatFrame();
	Common::String formatCurrentInstruction();
	Common::String decodeInstruction(ScriptData *sd, uint pc, uint *newPC = NULL);
	Common::String decodeScript(ScriptData *sd);
	Common::String formatFunctionName(Symbol &sym);
	Common::String formatFunctionBody(Symbol &sym);

	void reloadBuiltIns();
	void initBuiltIns();
	void initBuiltIns(const BuiltinProto protos[]);
	void cleanupBuiltIns();
	void cleanupBuiltIns(const BuiltinProto protos[]);
	void initFuncs();
	void cleanupFuncs();
	void initBytecode();
	void initMethods();
	void cleanupMethods();
	void initXLibs();
	void cleanupXLibs();

	Common::String normalizeXLibName(Common::String name);
	void openXLib(Common::String name, ObjectType type, const Common::Path &path);
	void closeXLib(Common::String name);
	void closeOpenXLibs();
	void reloadOpenXLibs();

	void runTests();

	// lingo-events.cpp
private:
	void initEventHandlerTypes();
	bool processEvent(LEvent event, ScriptType st, CastMemberID scriptId, int channelId = -1);

public:
	ScriptType event2script(LEvent ev);
	Symbol getHandler(const Common::String &name);

	void processEvents(Common::Queue<LingoEvent> &queue, bool isInputEvent);

public:
	bool execute();
	void switchStateFromWindow();
	void freezeState();
	void freezePlayState();
	void pushContext(const Symbol funcSym, bool allowRetVal, Datum defaultRetVal, int paramCount, int nargs);
	void popContext(bool aborting = false);
	void cleanLocalVars();
	void varAssign(const Datum &var, const Datum &value);
	Datum varFetch(const Datum &var, bool silent = false);
	Common::U32String evalChunkRef(const Datum &var);
	Datum findVarV4(int varType, const Datum &id);
	CastMemberID resolveCastMember(const Datum &memberID, const Datum &castLib, CastType type);
	void exposeXObject(const char *name, Datum obj);

	int getAlignedType(const Datum &d1, const Datum &d2, bool equality);

	Common::String formatAllVars();
	void printAllVars();

	inst readInst() { return getInst(_state->pc++); }
	inst getInst(uint pc) { return (*_state->script)[pc]; }
	int readInt() { return getInt(_state->pc++); }
	int getInt(uint pc);
	double readFloat() { double d = getFloat(_state->pc); _state->pc += calcCodeAlignment(sizeof(double)); return d; }
	double getFloat(uint pc) { return *(double *)(&((*_state->script)[_state->pc])); }
	char *readString() { char *s = getString(_state->pc); _state->pc += calcStringAlignment(s); return s; }
	char *getString(uint pc) { return (char *)(&((*_state->script)[_state->pc])); }

	Datum getVoid();
	void pushVoid();

	void printArgs(const char *funcname, int nargs, const char *prefix = nullptr);
	inline void printSTUBWithArglist(const char *funcname, int nargs) { printArgs(funcname, nargs, "STUB: "); }
	void convertVOIDtoString(int arg, int nargs);
	void dropStack(int nargs);
	void drop(uint num);

	void lingoError(const char *s, ...);

	void func_mci(const Common::String &name);
	void func_mciwait(const Common::String &name);
	void func_beep(int repeats);
	void func_goto(Datum &frame, Datum &movie, bool commandgo = false );
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
	Datum getTheDeskTopRectList();

private:
	Common::StringArray _entityNames;
	Common::StringArray _fieldNames;

public:
	LingoCompiler *_compiler;
	LingoState *_state;

	int _currentChannelId;

	bool _freezeState;
	bool _freezePlay;
	bool _playDone;
	bool _abort;
	bool _expectError;
	bool _caughtError;

	TheEntityHash _theEntities;
	TheEntityFieldHash _theEntityFields;

	int _objectEntityId;

	SymbolHash _builtinCmds;
	SymbolHash _builtinFuncs;
	SymbolHash _builtinConsts;
	SymbolHash _builtinListHandlers;
	SymbolHash _methods;
	XLibOpenerFuncHash _xlibOpeners;
	XLibCloserFuncHash _xlibClosers;

	OpenXLibsHash _openXLibs;
	OpenXLibsStateHash _openXLibsState;
	Common::StringArray _openXtras;

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

	FuncHash _functions;

	Common::HashMap<int, const LingoV4Bytecode *> _lingoV4;
	Common::HashMap<int, const LingoV4TheEntity *> _lingoV4TheEntity;

	uint _globalCounter;

	StackData _stack;

	DirectorEngine *_vm;

	int _floatPrecision;

	Datum _theResult;

	// events
	bool _passEvent;
	Datum _perFrameHook;

	Datum _windowList;
	Symbol _currentInputEvent;

	struct {
		LingoExecState _state = kRunning;
		bool (*_shouldPause)() = nullptr;
	} _exec;

public:
	void executeImmediateScripts(Frame *frame);
	void executePerFrameHook(int frame, int subframe);

	// lingo-utils.cpp
private:
	Common::HashMap<uint32, Common::U32String> _charNormalizations;
	void initCharNormalizations();

public:
	Common::String normalizeString(const Common::String &str);

public:
	void addBreakpoint(Breakpoint &bp);
	bool delBreakpoint(int id);
	Breakpoint *getBreakpoint(int id);

	const Common::Array<Breakpoint> &getBreakpoints() const { return _breakpoints; }
	Common::Array<Breakpoint> &getBreakpoints() { return _breakpoints; }

private:
	int _bpNextId = 1;
	Common::Array<Breakpoint> _breakpoints;
};

extern Lingo *g_lingo;

} // End of namespace Director

#endif
