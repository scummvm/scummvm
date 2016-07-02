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

#include "common/debug.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "audio/audiostream.h"
#include "common/str.h"
#include "engines/director/director.h"
#include "engines/director/score.h"
#include "director/lingo/lingo-gr.h"

namespace Director {

enum LEvent {
	kEventPrepareMovie,
	kEventStartMovie,
	kEventStopMovie,

	kEventNew,
	kEventBeginSprite,
	kEventEndSprite,

	kEventNone,
	kEventEnterFrame,
	kEventPrepareFrame,
	kEventIdle,
	kEventStepFrame,
	kEventExitFrame,

	kEventActivateWindow,
	kEventDeactivateWindow,
	kEventMoveWindow,
	kEventResizeWindow,
	kEventOpenWindow,
	kEventCloseWindow,

	kEventKeyUp,
	kEventKeyDown,
	kEventMouseUp,
	kEventMouseDown,
	kEventRightMouseUp,
	kEventRightMouseDown,
	kEventMouseEnter,
	kEventMouseLeave,
	kEventMouseUpOutSide,
	kEventMouseWithin,

	kEventStart
};

typedef void (*inst)(void);
#define	STOP (inst)0

typedef Common::Array<inst> ScriptData;

struct Symbol {	/* symbol table entry */
	char *name;
	int type;
	union {
		int		val;		/* VAR */
		float	fval;		/* FLOAT */
		ScriptData	*defn;	/* FUNCTION, PROCEDURE */
		char	*str;		/* STRING */
	} u;
	int nargs;
	bool global;

	Symbol();
};

struct Datum {	/* interpreter stack type */
	int type;

	union {
		int	i;
		float f;
		Symbol	*sym;
	} u;

	Datum() { u.sym = NULL; type = VOID; }

	float toFloat();
	int toInt();
};

struct Builtin {
	void (*func)(void);
	int nargs;

	Builtin(void (*func1)(void), int nargs1) : func(func1), nargs(nargs1) {}
};

typedef Common::HashMap<int32, ScriptData *> ScriptHash;
typedef Common::Array<Datum> StackData;
typedef Common::HashMap<Common::String, Symbol *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SymbolHash;
typedef Common::HashMap<Common::String, Builtin *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> BuiltinHash;

struct CFrame {	/* proc/func call stack frame */
	Symbol	*sp;	/* symbol table entry */
	int		retpc;	/* where to resume after return */
	ScriptData	*retscript;	 /* which script to resume after return */
	SymbolHash *localvars;
};

class Lingo {
public:
	Lingo(DirectorEngine *vm);
	~Lingo();

	void addCode(Common::String code, ScriptType type, uint16 id);
	void executeScript(ScriptType type, uint16 id);

	void processEvent(LEvent event, int entityId);

	void initBuiltIns();

public:
	void execute(int pc);
	void pushContext();
	void popContext();
	Symbol *lookupVar(const char *name, bool create = true, bool putInGlobalList = false);
	void cleanLocalVars();
	void define(Common::String &s, int start, int nargs);

	int alignTypes(Datum &d1, Datum &d2);

	int code1(inst code) { _currentScript->push_back(code); return _currentScript->size() - 1; }
	int code2(inst code_1, inst code_2) { int o = code1(code_1); code1(code_2); return o; }
	int code3(inst code_1, inst code_2, inst code_3) { int o = code1(code_1); code1(code_2); code1(code_3); return o; }
	int codeString(const char *s);

	int calcStringAlignment(const char *s) {
		return calcCodeAlignment(strlen(s) + 1);
	}
	int calcCodeAlignment(int l) {
		int instLen = sizeof(inst);
		return (l + instLen - 1) / instLen;
	}

	void codeArg(Common::String *s);
	void codeArgStore();
	int codeId(Common::String &s);
	int codeId_(Common::String &s);
	int codeFloat(float f);

	static void c_xpop();
	static void c_printtop();
	static void c_add();
	static void c_sub();
	static void c_mul();
	static void c_div();
	static void c_negate();
	static void c_constpush();
	static void c_fconstpush();
	static void c_varpush();
	static void c_assign();
	bool verify(Symbol *s);
	static void c_eval();
	static void c_repeatwhilecode();
	static void c_repeatwithcode();
	static void c_ifcode();
	static void c_eq();
	static void c_neq();
	static void c_gt();
	static void c_lt();
	static void c_ge();
	static void c_le();
	static void c_call();
	static void c_procret();

	static void c_exit();
	static void c_mci();
	static void c_mciwait();
	static void c_goto();
	static void c_gotoloop();
	static void c_gotonext();
	static void c_gotoprevious();
	static void c_global();

	static void b_random();

	void func_mci(Common::String &s);
	void func_mciwait(Common::String &s);
	void func_goto(Common::String &frame, Common::String &movie);
	void func_gotoloop();
	void func_gotonext();
	void func_gotoprevious();

public:
	ScriptData *_currentScript;
	ScriptType _currentScriptType;
	bool _returning;
	bool _indef;

	Common::Array<CFrame *> _callstack;
	Common::Array<Common::String *> _argstack;
	BuiltinHash _builtins;

private:
	int parse(const char *code);
	void push(Datum d);
	Datum pop(void);

	Common::HashMap<uint32, const char *> _eventHandlerTypes;
	Common::HashMap<Common::String, Audio::AudioStream *> _audioAliases;

	ScriptHash _scripts[kMaxScriptType + 1];

	SymbolHash _globalvars;
	SymbolHash *_localvars;
	SymbolHash _handlers;

	int _pc;

	StackData _stack;

	DirectorEngine *_vm;
};

extern Lingo *g_lingo;

} // End of namespace Director

#endif
