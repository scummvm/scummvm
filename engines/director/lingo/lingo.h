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
struct ScriptContext;
class DirectorEngine;
class Frame;

enum LexerDefineState {
	kStateNone,
	kStateInDef,
	kStateInArgs
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
	Common::String name;
	int type;
	union {
		int		i;			/* VAR */
		double	f;			/* FLOAT */
		ScriptData	*defn;	/* FUNCTION, PROCEDURE */
		void (*func)();		/* OPCODE */
		void (*bltin)(int);	/* BUILTIN */
		Common::String	*s;	/* STRING */
		DatumArray *farr;	/* ARRAY, POINT, RECT */
	} u;
	int nargs;		/* number of arguments */
	int maxArgs;	/* maximal number of arguments, for builtins */
	bool parens;	/* whether parens required or not, for builitins */

	bool global;
	Common::Array<Common::String> *argNames;
	Common::Array<Common::String> *varNames;
	ScriptContext *ctx;		/* optional script context to execute with */
	int archiveIndex; 		/* optional archive to execute with */

	Symbol();
};

struct Datum {	/* interpreter stack type */
	int type;

	union {
		int	i;				/* INT, ARGC, ARGCNORET */
		double f;			/* FLOAT */
		Common::String *s;	/* STRING */
		Symbol	*sym;
		DatumArray *farr;	/* ARRAY, POINT, RECT */
	} u;

	Datum() { u.sym = NULL; type = VOID; }
	Datum(int val) { u.i = val; type = INT; }
	Datum(double val) { u.f = val; type = FLOAT; }
	Datum(Common::String *val) { u.s = val; type = STRING; }

	double toFloat();
	int toInt();
	Common::String *toString();

	const char *type2str(bool isk = false);
};

struct Builtin {
	void (*func)(void);
	int nargs;

	Builtin(void (*func1)(void), int nargs1) : func(func1), nargs(nargs1) {}
};

struct ScriptContext {
	Common::Array<Symbol *> functions;
	Common::Array<Datum> constants;
};

typedef Common::HashMap<int32, ScriptContext *> ScriptContextHash;
typedef Common::Array<Datum> StackData;
typedef Common::HashMap<Common::String, Symbol *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SymbolHash;
typedef Common::HashMap<Common::String, Builtin *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> BuiltinHash;

typedef Common::HashMap<Common::String, TheEntity *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityHash;
typedef Common::HashMap<Common::String, TheEntityField *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityFieldHash;

struct CFrame {	/* proc/func call stack frame */
	Symbol	*sp;	/* symbol table entry */
	int		retpc;	/* where to resume after return */
	ScriptData	*retscript;	 /* which script to resume after return */
	ScriptContext	*retctx;   /* which script context to use after return */
	int 	retarchive;	/* which archive to use after return */
	SymbolHash *localvars;
};


struct LingoArchive {
	ScriptContextHash scriptContexts[kMaxScriptType + 1];
	Common::Array<Common::String> names;
};


class Lingo {

public:
	Lingo(DirectorEngine *vm);
	~Lingo();

	void restartLingo();

	void addCode(const char *code, ScriptType type, uint16 id);
	void addCodeV4(Common::SeekableSubReadStreamEndian &stream, ScriptType type, uint16 id);
	void addNamesV4(Common::SeekableSubReadStreamEndian &stream);
	void executeHandler(Common::String name);
	void executeScript(ScriptType type, uint16 id, uint16 function);
	void printStack(const char *s, uint pc);
	Common::String decodeInstruction(ScriptData *sd, uint pc, uint *newPC = NULL);

	void initBuiltIns();
	void initFuncs();
	void initBytecode();

	void runTests();

public:
	Common::String codePreprocessor(const char *s, bool simple = false);

private:
	Common::String preprocessReturn(Common::String in);
	const char *findNextDefinition(const char *s);

	// lingo-events.cpp
private:
	void initEventHandlerTypes();
	void primaryEventHandler(LEvent event);
	void processInputEvent(LEvent event);
	void processFrameEvent(LEvent event);
	void processGenericEvent(LEvent event);
	void runMovieScript(LEvent event);
	void processSpriteEvent(LEvent event);
	void processEvent(LEvent event, ScriptType st, int entityId);

public:
	ScriptContext *getScriptContext(ScriptType type, uint16 id);
	Common::String getName(uint16 id);
	ScriptType event2script(LEvent ev);
	Symbol *getHandler(Common::String &name);

	void processEvent(LEvent event);

public:
	void execute(uint pc);
	void pushContext();
	void popContext();
	Symbol *lookupVar(const char *name, bool create = true, bool putInGlobalList = false);
	void cleanLocalVars();
	Symbol *define(Common::String &s, int nargs, ScriptData *code);
	Symbol *define(Common::String &s, int start, int nargs, Common::String *prefix = NULL, int end = -1, bool removeCode = true);
	void processIf(int elselabel, int endlabel, int finalElse);
	void varAssign(Datum &var, Datum &value);
	Datum varFetch(Datum &var);

	int alignTypes(Datum &d1, Datum &d2);

	void printAllVars();

	int code1(inst code) { _currentScript->push_back(code); return _currentScript->size() - 1; }
	int code2(inst code_1, inst code_2) { int o = code1(code_1); code1(code_2); return o; }
	int code3(inst code_1, inst code_2, inst code_3) { int o = code1(code_1); code1(code_2); code1(code_3); return o; }
	int code4(inst code_1, inst code_2, inst code_3, inst code_4) { int o = code1(code_1); code1(code_2); code1(code_3); code1(code_4); return o; }
	int codeString(const char *s);
	void codeLabel(int label);
	int codeInt(int val);

	int calcStringAlignment(const char *s) {
		return calcCodeAlignment(strlen(s) + 1);
	}
	int calcCodeAlignment(int l) {
		int instLen = sizeof(inst);
		return (l + instLen - 1) / instLen;
	}

	void codeArg(Common::String *s);
	void codeArgStore();
	int codeSetImmediate(bool state);
	int codeFunc(Common::String *s, int numpar);
	int codeMe(Common::String *method, int numpar);
	int codeFloat(double f);
	void codeFactory(Common::String &s);

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

	void factoryCall(Common::String &name, int nargs);

	void func_mci(Common::String &s);
	void func_mciwait(Common::String &s);
	void func_beep(int repeats);
	void func_goto(Datum &frame, Datum &movie);
	void func_gotoloop();
	void func_gotonext();
	void func_gotoprevious();
	void func_play(Datum &frame, Datum &movie);
	void func_playdone();
	void func_cursor(int c, int mask);
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
	const char *entity2str(int id);
	const char *field2str(int id);

private:
	Common::StringArray _entityNames;
	Common::StringArray _fieldNames;

public:
	bool isInArgStack(Common::String *s);
	void clearArgStack();

public:
	ScriptType _currentScriptType;
	uint16 _currentEntityId;
	ScriptContext *_currentScriptContext;
	uint16 _currentScriptFunction;
	ScriptData *_currentScript;
	bool _returning;
	bool _nextRepeat;
	LexerDefineState _indef;
	bool _ignoreMe;
	bool _immediateMode;

	Common::Array<CFrame *> _callstack;
	Common::Array<Common::String *> _argstack;
	TheEntityHash _theEntities;
	TheEntityFieldHash _theEntityFields;

	int _objectEntityId;

	Common::Array<int> _labelstack;

	SymbolHash _builtins;
	Common::HashMap<Common::String, bool> _twoWordBuiltins;
	Common::HashMap<uint32, Symbol *> _handlers;

	int _linenumber;
	int _colnumber;

	Common::String _floatPrecisionFormat;

	bool _hadError;

	bool _inFactory;
	Common::String _currentFactory;
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

public:
	void executeImmediateScripts(Frame *frame);
};

extern Lingo *g_lingo;

} // End of namespace Director

#endif
