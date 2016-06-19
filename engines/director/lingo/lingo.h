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

typedef struct Symbol {	/* symbol table entry */
	char	*name;
	long	type;
	union {
		int		val;		/* VAR */
		float	fval;		/* FLOAT */
		inst	*defn;		/* FUNCTION, PROCEDURE */
		char	*str;		/* STRING */
	} u;

	Symbol();
} Symbol;

typedef union Datum {	/* interpreter stack type */
	int	val;
	Symbol	*sym;

	Datum() { val = 0; sym = NULL; }
} Datum;

typedef Common::Array<inst> ScriptData;
typedef Common::HashMap<int32, ScriptData *> ScriptHash;
typedef Common::Array<Datum> StackData;

class Lingo {
public:
	Lingo(DirectorEngine *vm);
	~Lingo();

	void addCode(Common::String code, ScriptType type, uint16 id);
	void executeScript(ScriptType type, uint16 id);

	void processEvent(LEvent event, int entityId);

	int code1(inst code) { _currentScript->push_back(code); return _currentScript->size(); }
	int code2(inst code_1, inst code_2) { code1(code_1); return code1(code_2); }
	int code3(inst code_1, inst code_2, inst code_3) { code1(code_1); code1(code_2); return code1(code_3); }
	int codeString(const char *s);

	int calcStringAlignment(const char *s) {
		int instLen = sizeof(inst);
		int l = strlen(s); return (l + 1 + instLen - 1) / instLen;
	}

public:
	static void func_xpop();
	static void func_printtop();
	static void func_add();
	static void func_sub();
	static void func_mul();
	static void func_div();
	static void func_negate();
	static void func_constpush();
	static void func_varpush();
	static void func_assign();
	bool verify(Symbol *s);
	static void func_eval();
	static void func_mci();
	static void func_mciwait();
	static void func_goto();
	static void func_gotoloop();
	static void func_gotonext();
	static void func_gotoprevious();

	void exec_mci(Common::String &s);
	void exec_mciwait(Common::String &s);
	void exec_goto(Common::String &frame, Common::String &movie);

private:
	int parse(const char *code);
	void push(Datum d);
	Datum pop(void);

	Common::HashMap<uint32, const char *> _eventHandlerTypes;
	Common::HashMap<Common::String, Audio::AudioStream *> _audioAliases;

	ScriptHash _scripts[kMaxScriptType + 1];
	ScriptData *_currentScript;

	Common::HashMap<Common::String, Symbol *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _vars;

	inst *_pc;

	StackData _stack;

	DirectorEngine *_vm;
};

extern Lingo *g_lingo;

} // End of namespace Director

#endif
