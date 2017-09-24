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

#include "director/director.h"
#include "director/score.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-the.h"

namespace Director {

enum LEvent {
	kEventPrepareMovie,
	kEventStartMovie,
	kEventStepMovie,
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
	kEventTimeout,

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
#define ENTITY_INDEX(t,id) ((t) * 100000 + (id))

typedef Common::Array<inst> ScriptData;
typedef Common::Array<double> FloatArray;

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
		FloatArray *arr;	/* ARRAY, POINT, RECT */
	} u;
	int nargs;		/* number of arguments */
	int maxArgs;	/* maximal number of arguments, for builtins */
	bool parens;	/* whether parens required or not, for builitins */

	bool global;

	Symbol();
};

struct Datum {	/* interpreter stack type */
	int type;

	union {
		int	i;
		double f;
		Common::String *s;
		Symbol	*sym;
		FloatArray *arr;	/* ARRAY, POINT, RECT */
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

typedef Common::HashMap<int32, ScriptData *> ScriptHash;
typedef Common::Array<Datum> StackData;
typedef Common::HashMap<Common::String, Symbol *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SymbolHash;
typedef Common::HashMap<Common::String, Builtin *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> BuiltinHash;

typedef Common::HashMap<Common::String, TheEntity *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityHash;
typedef Common::HashMap<Common::String, TheEntityField *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> TheEntityFieldHash;

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

	void restartLingo();

	void addCode(const char *code, ScriptType type, uint16 id);
	void executeScript(ScriptType type, uint16 id);
	void printStack(const char *s);
	Common::String decodeInstruction(uint pc, uint *newPC = NULL);

	void initBuiltIns();
	void initFuncs();
	void initTheEntities();

	void runTests();

private:
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
	ScriptType event2script(LEvent ev);
	Symbol *getHandler(Common::String &name);

	void processEvent(LEvent event);

public:
	void execute(uint pc);
	void pushContext();
	void popContext();
	Symbol *lookupVar(const char *name, bool create = true, bool putInGlobalList = false);
	void cleanLocalVars();
	void define(Common::String &s, int start, int nargs, Common::String *prefix = NULL, int end = -1);
	void processIf(int elselabel, int endlabel);

	int alignTypes(Datum &d1, Datum &d2);

	int code1(inst code) { _currentScript->push_back(code); return _currentScript->size() - 1; }
	int code2(inst code_1, inst code_2) { int o = code1(code_1); code1(code_2); return o; }
	int code3(inst code_1, inst code_2, inst code_3) { int o = code1(code_1); code1(code_2); code1(code_3); return o; }
	int codeString(const char *s);
	void codeLabel(int label);
	int codeConst(int val);
	int codeArray(int arraySize);

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

	void pushVoid();

	static void c_xpop();
	static void c_printtop();

	static void c_add();
	static void c_sub();
	static void c_mul();
	static void c_div();
	static void c_mod();
	static void c_negate();

	static void c_and();
	static void c_or();
	static void c_not();

	static void c_ampersand();
	static void c_after();
	static void c_before();
	static void c_concat();
	static void c_contains();
	static void c_starts();

	static void c_intersects();
	static void c_within();
	static void c_charOf();
	static void c_charToOf();
	static void c_itemOf();
	static void c_itemToOf();
	static void c_lineOf();
	static void c_lineToOf();
	static void c_wordOf();
	static void c_wordToOf();

	static void c_constpush();
	static void c_voidpush();
	static void c_fconstpush();
	static void c_stringpush();
	static void c_symbolpush();
	static void c_varpush();
	static void c_arraypush();
	static void c_assign();
	bool verify(Symbol *s);
	static void c_eval();
	static void c_setImmediate();

	static void c_swap();

	static void c_theentitypush();
	static void c_theentityassign();

	static void c_repeatwhilecode();
	static void c_repeatwithcode();
	static void c_ifcode();
	static void c_whencode();
	static void c_tellcode();
	static void c_exitRepeat();
	static void c_eq();
	static void c_neq();
	static void c_gt();
	static void c_lt();
	static void c_ge();
	static void c_le();
	static void c_call();

	void call(Common::String name, int nargs);

	static void c_procret();

	static void c_mci();
	static void c_mciwait();
	static void c_goto();
	static void c_gotoloop();
	static void c_gotonext();
	static void c_gotoprevious();
	static void c_global();
	static void c_instance();
	static void c_property();

	static void c_play();
	static void c_playdone();

	static void c_open();

	void printSTUBWithArglist(const char *funcname, int nargs, const char *prefix = "STUB:");
	void convertVOIDtoString(int arg, int nargs);
	void dropStack(int nargs);
	void drop(uint num);

	static void b_abs(int nargs);
	static void b_atan(int nargs);
	static void b_cos(int nargs);
	static void b_exp(int nargs);
	static void b_float(int nargs);
	static void b_integer(int nargs);
	static void b_log(int nargs);
	static void b_pi(int nargs);
	static void b_power(int nargs);
	static void b_random(int nargs);
	static void b_sin(int nargs);
	static void b_sqrt(int nargs);
	static void b_tan(int nargs);

	static void b_chars(int nargs);
	static void b_charToNum(int nargs);
	static void b_delete(int nargs);
	static void b_hilite(int nargs);
	static void b_length(int nargs);
	static void b_numToChar(int nargs);
	static void b_offset(int nargs);
	static void b_string(int nargs);

	static void b_add(int nargs);
	static void b_addAt(int nargs);
	static void b_addProp(int nargs);
	static void b_append(int nargs);
	static void b_count(int nargs);
	static void b_deleteAt(int nargs);
	static void b_deleteProp(int nargs);
	static void b_findPos(int nargs);
	static void b_findPosNear(int nargs);
	static void b_getaProp(int nargs);
	static void b_getAt(int nargs);
	static void b_getLast(int nargs);
	static void b_getOne(int nargs);
	static void b_getPos(int nargs);
	static void b_getProp(int nargs);
	static void b_getPropAt(int nargs);
	static void b_list(int nargs);
	static void b_listP(int nargs);
	static void b_max(int nargs);
	static void b_min(int nargs);
	static void b_setaProp(int nargs);
	static void b_setAt(int nargs);
	static void b_setProp(int nargs);
	static void b_sort(int nargs);

	static void b_floatP(int nargs);
	static void b_ilk(int nargs);
	static void b_integerp(int nargs);
	static void b_objectp(int nargs);
	static void b_pictureP(int nargs);
	static void b_stringp(int nargs);
	static void b_symbolp(int nargs);
	static void b_voidP(int nargs);

	static void b_alert(int nargs);
	static void b_birth(int nargs);
	static void b_clearGlobals(int nargs);
	static void b_cursor(int nargs);
	static void b_framesToHMS(int nargs);
	static void b_HMStoFrames(int nargs);
	static void b_param(int nargs);
	static void b_printFrom(int nargs);
	static void b_showGlobals(int nargs);
	static void b_showLocals(int nargs);
	static void b_value(int nargs);

	static void b_constrainH(int nargs);
	static void b_constrainV(int nargs);
	static void b_copyToClipBoard(int nargs);
	static void b_duplicate(int nargs);
	static void b_editableText(int nargs);
	static void b_erase(int nargs);
	static void b_findEmpty(int nargs);
	static void b_importFileInto(int nargs);
	static void b_installMenu(int nargs);
	static void b_label(int nargs);
	static void b_marker(int nargs);
	static void b_move(int nargs);
	static void b_moveableSprite(int nargs);
	static void b_pasteClipBoardInto(int nargs);
	static void b_puppetPalette(int nargs);
	static void b_puppetSound(int nargs);
	static void b_puppetSprite(int nargs);
	static void b_puppetTempo(int nargs);
	static void b_puppetTransition(int nargs);
	static void b_ramNeeded(int nargs);
	static void b_rollOver(int nargs);
	static void b_spriteBox(int nargs);
	static void b_unLoad(int nargs);
	static void b_unLoadCast(int nargs);
	static void b_updateStage(int nargs);
	static void b_zoomBox(int nargs);

	static void b_abort(int nargs);
	static void b_continue(int nargs);
	static void b_dontPassEvent(int nargs);
	static void b_delay(int nargs);
	static void b_do(int nargs);
	static void b_halt(int nargs);
	static void b_nothing(int nargs);
	static void b_pass(int nargs);
	static void b_pause(int nargs);
	static void b_playAccel(int nargs);
	static void b_preLoad(int nargs);
	static void b_preLoadCast(int nargs);
	static void b_quit(int nargs);
	static void b_restart(int nargs);
	static void b_shutDown(int nargs);
	static void b_startTimer(int nargs);

	static void b_closeDA(int nargs);
	static void b_closeResFile(int nargs);
	static void b_closeXlib(int nargs);
	static void b_getNthFileNameInFolder(int nargs);
	static void b_openDA(int nargs);
	static void b_openResFile(int nargs);
	static void b_openXlib(int nargs);
	static void b_setCallBack(int nargs);
	static void b_saveMovie(int nargs);
	static void b_showResFile(int nargs);
	static void b_showXlib(int nargs);
	static void b_xFactoryList(int nargs);

	static void b_point(int nargs);
	static void b_inside(int nargs);
	static void b_intersect(int nargs);
	static void b_map(int nargs);
	static void b_offsetRect(int nargs);
	static void b_rect(int nargs);
	static void b_union(int nargs);

	static void b_close(int nargs);
	static void b_forget(int nargs);
	static void b_inflate(int nargs);
	static void b_moveToBack(int nargs);
	static void b_moveToFront(int nargs);
	static void b_window(int nargs);

	static void b_beep(int nargs);
	static void b_mci(int nargs);
	static void b_mciwait(int nargs);
	static void b_soundBusy(int nargs);
	static void b_soundClose(int nargs);
	static void b_soundFadeIn(int nargs);
	static void b_soundFadeOut(int nargs);
	static void b_soundPlayFile(int nargs);
	static void b_soundStop(int nargs);

	static void b_ancestor(int nargs);
	static void b_backspace(int nargs);
	static void b_empty(int nargs);
	static void b_enter(int nargs);
	static void b_false(int nargs);
	static void b_quote(int nargs);
	static void b_return(int nargs);
	static void b_tab(int nargs);
	static void b_true(int nargs);
	static void b_version(int nargs);

	static void b_factory(int nargs);
	void factoryCall(Common::String &name, int nargs);

	static void b_cast(int nargs);
	static void b_field(int nargs);
	static void b_me(int nargs);
	static void b_script(int nargs);

	void func_mci(Common::String &s);
	void func_mciwait(Common::String &s);
	void func_beep(int repeats);
	void func_goto(Datum &frame, Datum &movie);
	void func_gotoloop();
	void func_gotonext();
	void func_gotoprevious();
	void func_play(Datum &frame, Datum &movie);
	void func_playdone();
	void func_cursor(int c);
	int func_marker(int m);

public:
	void setTheEntity(int entity, Datum &id, int field, Datum &d);
	void setTheSprite(Datum &id, int field, Datum &d);
	void setTheCast(Datum &id, int field, Datum &d);
	Datum getTheEntity(int entity, Datum &id, int field);
	Datum getTheSprite(Datum &id, int field);
	Datum getTheCast(Datum &id, int field);

public:
	ScriptData *_currentScript;
	ScriptType _currentScriptType;
	uint16 _currentEntityId;
	bool _returning;
	bool _indef;
	bool _ignoreMe;
	bool _immediateMode;

	Common::Array<CFrame *> _callstack;
	Common::Array<Common::String *> _argstack;
	TheEntityHash _theEntities;
	TheEntityFieldHash _theEntityFields;
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

	bool _exitRepeat;

	bool _cursorOnStack;

private:
	int parse(const char *code);
	void parseMenu(const char *code);

	void push(Datum d);
	Datum pop(void);

	Common::HashMap<uint32, const char *> _eventHandlerTypes;
	Common::HashMap<Common::String, uint32> _eventHandlerTypeIds;
	Common::HashMap<Common::String, Audio::AudioStream *> _audioAliases;

	ScriptHash _scripts[kMaxScriptType + 1];

	SymbolHash _globalvars;
	SymbolHash *_localvars;

	FuncHash _functions;

	uint _pc;

	StackData _stack;

	DirectorEngine *_vm;

	int _floatPrecision;

	bool dontPassEvent;

public:
	void executeImmediateScripts(Frame *frame);
};

extern Lingo *g_lingo;

} // End of namespace Director

#endif
