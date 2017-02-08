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

#include "director/lingo/lingo.h"

namespace Director {

static struct BuiltinProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	bool parens;
} builtins[] = {
	// Math
	{ "abs",			Lingo::b_abs,			1, 1, true },	// D2 function
	{ "atan",			Lingo::b_atan,			1, 1, true },	//			D4 function
	{ "cos",			Lingo::b_cos,			1, 1, true },	//			D4 function
	{ "exp",			Lingo::b_exp,			1, 1, true },	//			D4 function
	{ "float",			Lingo::b_float,			1, 1, true },	//			D4 function
	{ "integer",		Lingo::b_integer,		1, 1, true },	//		D3 f
	{ "log",			Lingo::b_log,			1, 1, true },	//			D4 f
	{ "pi",				Lingo::b_pi,			0, 0, true },	//			D4 f
	{ "power",			Lingo::b_power,			2, 2, true },	//			D4 f
	{ "random",			Lingo::b_random,		1, 1, true },	// D2 f
	{ "sin",			Lingo::b_sin,			1, 1, true },	//			D4 f
	{ "sqrt",			Lingo::b_sqrt,			1, 1, true },	// D2 f
	{ "tan",			Lingo::b_tan,			1, 1, true },	//			D4 f
	// String
	{ "chars",			Lingo::b_chars,			3, 3, true },	// D2
	{ "charToNum",		Lingo::b_charToNum,		1, 1, true },	// D2
	{ "delete",			Lingo::b_delete,		1, 1, true },	//		D3
	{ "hilite",			Lingo::b_hilite,		1, 1, true },	//		D3
	{ "length",			Lingo::b_length,		1, 1, true },	// D2
	{ "numToChar",		Lingo::b_numToChar,		1, 1, true },	// D2
	{ "offset",			Lingo::b_offset,		2, 2, true },	// D2
	{ "string",			Lingo::b_string,		1, 1, true },	// D2
	{ "value",		 	Lingo::b_value,			1, 1, true },	// D2 f
	// Lists
	{ "add",			Lingo::b_add,			2, 2, false },	//			D4 command
	{ "addAt",			Lingo::b_addAt,			3, 3, false },	//			D4 c
	{ "addProp",		Lingo::b_addProp,		3, 3, false },	//			D4 c
	{ "append",			Lingo::b_append,		2, 2, false },	//			D4 c
	{ "count",			Lingo::b_count,			1, 1, true },	//			D4 f
	{ "deleteAt",		Lingo::b_deleteAt,		2, 2, false },	//			D4 c
	{ "deleteProp",		Lingo::b_deleteProp,	2, 2, false },	//			D4 c
	{ "findPos",		Lingo::b_findPos,		2, 2, true },	//			D4 f
	{ "findPosNear",	Lingo::b_findPosNear,	2, 2, true },	//			D4 f
	{ "getaProp",		Lingo::b_getaProp,		2, 2, true },	//			D4 f
	{ "getAt",			Lingo::b_getAt,			2, 2, true },	//			D4 f
	{ "getLast",		Lingo::b_getLast,		1, 1, true },	//			D4 f
	{ "getOne",			Lingo::b_getOne,		2, 2, true },	//			D4 f
	{ "getPos",			Lingo::b_getPos,		2, 2, true },	//			D4 f
	{ "getProp",		Lingo::b_getProp,		2, 2, true },	//			D4 f
	{ "getPropAt",		Lingo::b_getPropAt,		2, 2, true },	//			D4 f
	{ "list",			Lingo::b_list,			-1, 0, true },	//			D4 f
	{ "listP",			Lingo::b_listP,			1, 1, true },	//			D4 f
	{ "max",			Lingo::b_max,			1, 1, true },	//			D4 f
	{ "min",			Lingo::b_min,			1, 1, true },	//			D4 f
	{ "setaProp",		Lingo::b_setaProp,		3, 3, false },	//			D4 c
	{ "setAt",			Lingo::b_setAt,			3, 3, false },	//			D4 c
	{ "setProp",		Lingo::b_setProp,		3, 3, false },	//			D4 c
	{ "sort",			Lingo::b_sort,			1, 1, false },	//			D4 c
	// Files
	{ "closeDA",	 	Lingo::b_closeDA, 		0, 0, false },	// D2 c
	{ "closeResFile",	Lingo::b_closeResFile,	0, 1, false },	// D2 c
	{ "closeXlib",		Lingo::b_closeXlib,		0, 1, false },	// D2 c
	{ "getNthFileNameInFolder",Lingo::b_getNthFileNameInFolder,2,2,true },//D4 f
		// open													// D2 c
	{ "openDA",	 		Lingo::b_openDA, 		1, 1, false },	// D2 c
	{ "openResFile",	Lingo::b_openResFile,	1, 1, false },	// D2 c
	{ "openXlib",		Lingo::b_openXlib,		1, 1, false },	// D2 c
	{ "saveMovie",		Lingo::b_saveMovie,		1, 1, false },	//			D4 c
	{ "setCallBack",	Lingo::b_setCallBack,	2, 2, false },	//		D3 c
	{ "showResFile",	Lingo::b_showResFile,	0, 1, false },	// D2 c
	{ "showXlib",		Lingo::b_showXlib,		0, 1, false },	// D2 c
	{ "xFactoryList",	Lingo::b_xFactoryList,	1, 1, true },	//		D3 f
	// Control
	{ "abort",			Lingo::b_abort,			0, 0, false },	//			D4 c
	{ "continue",		Lingo::b_continue,		0, 0, false },	// D2
	{ "dontPassEvent",	Lingo::b_dontPassEvent,	0, 0, false },	// D2 c
	{ "delay",	 		Lingo::b_delay,			1, 1, false },	// D2
	{ "do",		 		Lingo::b_do,			1, 1, false },	// D2
	{ "halt",	 		Lingo::b_halt,			0, 0, false },	//			D4 c
	{ "nothing",		Lingo::b_nothing,		0, 0, false },	// D2 c
	{ "pass",			Lingo::b_pass,			0, 0, false },	//			D4 c
	{ "pause",			Lingo::b_pause,			0, 0, false },	// D2
		// play													// D2
	{ "playAccel",		Lingo::b_playAccel,		-1,0, false },	// D2
		// play done											// D2
	{ "preLoad",		Lingo::b_preLoad,		-1,0, false },	//		D3
	{ "preLoadCast",	Lingo::b_preLoadCast,	-1,0, false },	//		D3
	{ "quit",			Lingo::b_quit,			0, 0, false },	// D2
	{ "restart",		Lingo::b_restart,		0, 0, false },	// D2
	{ "shutDown",		Lingo::b_shutDown,		0, 0, false },	// D2
	{ "startTimer",		Lingo::b_startTimer,	0, 0, false },	// D2
		// when keyDown											// D2
		// when mouseDown										// D2
		// when mouseUp											// D2
		// when timeOut											// D2
	// Types
	{ "floatP",			Lingo::b_floatP,		1, 1, true },	//		D3
	{ "ilk",	 		Lingo::b_ilk,			1, 2, false },	//			D4 f
	{ "integerp",		Lingo::b_integerp,		1, 1, true },	// D2 f
	{ "objectp",		Lingo::b_objectp,		1, 1, true },	// D2 f
	{ "pictureP",		Lingo::b_pictureP,		1, 1, true },	//			D4 f
	{ "stringp",		Lingo::b_stringp,		1, 1, true },	// D2 f
	{ "symbolp",		Lingo::b_symbolp,		1, 1, true },	// D2 f
	// Misc
	{ "alert",	 		Lingo::b_alert,			1, 1, false },	// D2 c
	{ "cursor",	 		Lingo::b_cursor,		1, 1, false },	// D2 c
	{ "framesToHMS",	Lingo::b_framesToHMS,	4, 4, false },	//		D3
	{ "HMStoFrames",	Lingo::b_HMStoFrames,	4, 4, false },	//		D3
	{ "param",	 		Lingo::b_param,			1, 1, true },	//			D4 f
	{ "printFrom",	 	Lingo::b_printFrom,		-1,0, false },	// D2
		// put													// D2
		// set													// D2
	{ "showGlobals",	Lingo::b_showGlobals,	0, 0, false },	// D2
	{ "showLocals",		Lingo::b_showLocals,	0, 0, false },	// D2
	// Score
	{ "constrainH",		Lingo::b_constrainH,	2, 2, true },	// D2
	{ "constrainV",		Lingo::b_constrainV,	2, 2, true },	// D2
	{ "copyToClipBoard",Lingo::b_copyToClipBoard,1,1, false },	//			D4 c
	{ "duplicate",		Lingo::b_duplicate,		1, 2, false },	//			D4 c
	{ "editableText",	Lingo::b_editableText,	0, 0, false },	// D2
	{ "erase",			Lingo::b_erase,			1, 1, false },	//			D4 c
	{ "findEmpty",		Lingo::b_findEmpty,		1, 1, true },	//			D4 f
		// go													// D2
	{ "importFileInto",	Lingo::b_importFileInto,2, 2, false },	//			D4 c
	{ "installMenu",	Lingo::b_installMenu,	1, 1, false },	// D2 c
	{ "intersect",		Lingo::b_intersect,		2, 2, false },	//			D4 f
	{ "label",			Lingo::b_label,			1, 1, true },	// D2
	{ "marker",			Lingo::b_marker,		1, 1, true },	// D2
	{ "move",			Lingo::b_move,			1, 2, false },	//			D4 c
	{ "moveableSprite",	Lingo::b_moveableSprite,0, 0, false },	// D2
	{ "pasteClipBoardInto",Lingo::b_pasteClipBoardInto,1, 1, false },	//	D4 c
	{ "puppetPalette",	Lingo::b_puppetPalette, -1,0, false },	// D2
	{ "puppetSound",	Lingo::b_puppetSound,	-1,0, false },	// D2
	{ "puppetSprite",	Lingo::b_puppetSprite,	-1,0, false },	// D2
	{ "puppetTempo",	Lingo::b_puppetTempo,	1, 1, false },	// D2
	{ "puppetTransition",Lingo::b_puppetTransition,-1,0, false },// D2
	{ "rollOver",		Lingo::b_rollOver,		1, 1, true },	// D2 f
	{ "spriteBox",		Lingo::b_spriteBox,		-1,0, false },	// D2
	{ "unLoad",			Lingo::b_unLoad,		0, 2, false },	//			D4 c
	{ "unLoadCast",		Lingo::b_unLoadCast,	0, 2, false },	//			D4 c
	{ "updateStage",	Lingo::b_updateStage,	0, 0, false },	// D2
	{ "zoomBox",		Lingo::b_zoomBox,		-1,0, false },	// D2
	// Point
	{ "point",			Lingo::b_point,			2, 2, true },
	// Sound
	{ "beep",	 		Lingo::b_beep,			0, 1, false },	// D2
	{ "mci",	 		Lingo::b_mci,			1, 1, false },
	{ "mciwait",		Lingo::b_mciwait,		1, 1, false },
	{ "sound-fadeIn",	Lingo::b_soundFadeIn, 	1, 2, false },	//		D3
	{ "sound-fadeOut",	Lingo::b_soundFadeOut, 	1, 2, false },	//		D3
	{ "sound-playFile",	Lingo::b_soundPlayFile, 2, 2, false },	//		D3 c
	{ "sound-stop",		Lingo::b_soundStop,	 	1, 1, false },	//		D3
	{ "soundBusy",		Lingo::b_soundBusy,	 	1, 1, true },	//		D3 f
	// Window
	{ "close",			Lingo::b_close,			1, 1, false },	//			D4 c
	{ "forget",			Lingo::b_forget,		1, 1, false },	//			D4 c
	{ "inflate",		Lingo::b_inflate,		3, 3, true },	//			D4 f
	{ "moveToBack",		Lingo::b_moveToBack,	1, 1, false },	//			D4 c
	{ "moveToFront",	Lingo::b_moveToFront,	1, 1, false },	//			D4 c
	// Constants
	{ "backspace",		Lingo::b_backspace,		0, 0, false },	// D2
	{ "empty",			Lingo::b_empty,			0, 0, false },	// D2
	{ "enter",			Lingo::b_enter,			0, 0, false },	// D2
	{ "false",			Lingo::b_false,			0, 0, false },	// D2
	{ "quote",			Lingo::b_quote,			0, 0, false },	// D2
	{ "return",			Lingo::b_return,		0, 0, false },	// D2
	{ "tab",			Lingo::b_tab,			0, 0, false },	// D2
	{ "true",			Lingo::b_true,			0, 0, false },	// D2
	{ "version",		Lingo::b_version,		0, 0, false },	//		D3
	// References
	{ "cast",			Lingo::b_cast,			1, 1, false },	//			D4 f
	{ "field",			Lingo::b_field,			1, 1, false },	//		D3 f
	{ "me",				Lingo::b_me,			-1,0, false },	//		D3
	{ "window",			Lingo::b_window,		1, 1, false },	//			D4 f

	{ 0, 0, 0, 0, false }
};

static const char *twoWordBuiltins[] = {
	//"duplicate",
	//"erase",
	"sound",
	0
};

static const char *builtinFunctions[] = {
	"abs",
	"atan",
	"backspace",
	"cast",
	"cos",
	"count",
	"empty",
	"enter",
	"exp",
	"false",
	"field",
	"findEmpty",
	"findPos",
	"findPosNear",
	"float",
	"getaProp",
	"getAt",
	"getLast",
	"getNthFileNameInFolder",
	"getOne",
	"getPos",
	"getProp",
	"getPropAt",
	"ilk",
	"inflate",
	"integer",
	"integerp",
	"intersect",
	"list",
	"listP",
	"log",
	"max",
	"min",
	"objectp",
	"pi",
	"pictureP",
	"power",
	"random",
	"rollOver",
	"quote",
	"return",
	"sin",
	"sqrt",
	"soundBusy",
	"stringp",
	"symbolp",
	"tab",
	"tan",
	"true",
	"value",
	"version",
	"window",
	"xFactoryList",
	0
};

void Lingo::initBuiltIns() {
	for (BuiltinProto *blt = builtins; blt->name; blt++) {
		Symbol *sym = new Symbol;

		sym->name = blt->name;
		sym->type = BLTIN;
		sym->nargs = blt->minArgs;
		sym->maxArgs = blt->maxArgs;
		sym->parens = blt->parens;
		sym->u.bltin = blt->func;

		_builtins[blt->name] = sym;

		_functions[(void *)sym->u.s] = new FuncDesc(blt->name, "");
	}

	for (const char **b = builtinFunctions; *b; b++) {
		if (!_builtins.contains(*b))
			error("Incorrectly setup builtinFunctions[]: %s", *b);

		_builtins[*b]->type = FBLTIN;
	}

	for (const char **b = twoWordBuiltins; *b; b++)
		_twoWordBuiltins[*b] = true;
}

void Lingo::printSTUBWithArglist(const char *funcname, int nargs) {
	Common::String s(funcname);

	s += '(';

	for (int i = 0; i < nargs; i++) {
		Datum d = _stack[_stack.size() - nargs + i];

		d.toString();
		s += *d.u.s;

		if (i != nargs - 1)
			s += ", ";
	}

	s += ")";

	warning("STUB: %s", s.c_str());
}

void Lingo::convertVOIDtoString(int arg, int nargs) {
	if (_stack[_stack.size() - nargs + arg].type == VOID) {
		if (_stack[_stack.size() - nargs + arg].u.s != NULL)
			g_lingo->_stack[_stack.size() - nargs + arg].type = STRING;
		else
			warning("Incorrect convertVOIDtoString for arg %d of %d", arg, nargs);
	}
}

void Lingo::dropStack(int nargs) {
	for (int i = 0; i < nargs; i++)
		pop();
}

void Lingo::drop(uint num) {
	if (num > _stack.size() - 1) {
		warning("Incorrect number of elements to drop from stack: %d > %d", num, _stack.size() - 1);
		return;
	}
	_stack.remove_at(_stack.size() - 1 - num);
}


///////////////////
// Math
///////////////////
void Lingo::b_abs(int nargs) {
	Datum d = g_lingo->pop();

	if (d.type == INT)
		d.u.i = ABS(d.u.i);
	else if (d.type == FLOAT)
		d.u.f = ABS(d.u.f);

	g_lingo->push(d);
}

void Lingo::b_atan(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = atan(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_cos(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = cos(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_exp(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt(); // Lingo uses int, so we're enforcing it
	d.toFloat();
	d.u.f = exp(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_float(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	g_lingo->push(d);
}

void Lingo::b_integer(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	g_lingo->push(d);
}

void Lingo::b_log(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = log(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_pi(int nargs) {
	Datum d;
	d.toFloat();
	d.u.f = M_PI;
	g_lingo->push(d);
}

void Lingo::b_power(int nargs) {
	Datum d1 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	d1.toFloat();
	d2.toFloat();
	d1.u.f = pow(d2.u.f, d1.u.f);
	g_lingo->push(d1);
}

void Lingo::b_random(int nargs) {
	Datum max = g_lingo->pop();
	Datum res;

	max.toInt();

	res.u.i = g_lingo->_vm->_rnd.getRandomNumber(max.u.i);
	res.type = INT;

	g_lingo->push(res);
}

void Lingo::b_sin(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = sin(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_sqrt(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = sqrt(d.u.f);
	g_lingo->push(d);
}

void Lingo::b_tan(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = tan(d.u.f);
	g_lingo->push(d);
}

///////////////////
// String
///////////////////
void Lingo::b_chars(int nargs) {
	Datum to = g_lingo->pop();
	Datum from = g_lingo->pop();
	Datum s = g_lingo->pop();

	if (s.type != STRING)
		error("Incorrect type for 'chars' function: %s", s.type2str());

	to.toInt();
	from.toInt();

	int len = strlen(s.u.s->c_str());
	int f = MAX(0, MIN(len, from.u.i - 1));
	int t = MAX(0, MIN(len, to.u.i));

	Common::String *res = new Common::String(&(s.u.s->c_str()[f]), &(s.u.s->c_str()[t]));

	delete s.u.s;

	s.u.s = res;
	s.type = STRING;
	g_lingo->push(s);
}

void Lingo::b_charToNum(int nargs) {
	Datum d = g_lingo->pop();

	if (d.type != STRING)
		error("Incorrect type for 'charToNum' function: %s", d.type2str());

	byte chr = d.u.s->c_str()[0];
	delete d.u.s;

	d.u.i = chr;
	d.type = INT;
	g_lingo->push(d);
}

void Lingo::b_delete(int nargs) {
	Datum d = g_lingo->pop();

	d.toInt();

	warning("STUB: b_delete");

	g_lingo->push(Datum((char)d.u.i));
}

void Lingo::b_hilite(int nargs) {
	Datum d = g_lingo->pop();

	d.toInt();

	warning("STUB: b_hilite");

	g_lingo->push(Datum((char)d.u.i));
}

void Lingo::b_length(int nargs) {
	Datum d = g_lingo->pop();

	if (d.type != STRING)
		error("Incorrect type for 'length' function: %s", d.type2str());

	int len = strlen(d.u.s->c_str());
	delete d.u.s;

	d.u.i = len;
	d.type = INT;
	g_lingo->push(d);
}

void Lingo::b_numToChar(int nargs) {
	Datum d = g_lingo->pop();

	d.toInt();

	g_lingo->push(Datum((char)d.u.i));
}

void Lingo::b_offset(int nargs) {
	Datum target = g_lingo->pop();
	Datum source = g_lingo->pop();

	target.toString();
	source.toString();

	warning("STUB: b_offset()");

	g_lingo->push(Datum(0));
}

void Lingo::b_string(int nargs) {
	Datum d = g_lingo->pop();
	d.toString();
	g_lingo->push(d);
}

void Lingo::b_value(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	warning("STUB: b_value()");
	g_lingo->push(d);
}

///////////////////
// Lists
///////////////////
void Lingo::b_add(int nargs) {
	g_lingo->printSTUBWithArglist("b_add", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_addAt(int nargs) {
	g_lingo->printSTUBWithArglist("b_addAt", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_addProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_addProp", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_append(int nargs) {
	g_lingo->printSTUBWithArglist("b_append", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_count(int nargs) {
	g_lingo->printSTUBWithArglist("b_count", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_deleteAt(int nargs) {
	g_lingo->printSTUBWithArglist("b_deleteAt", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_deleteProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_deleteProp", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_findPos(int nargs) {
	g_lingo->printSTUBWithArglist("b_findPos", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_findPosNear(int nargs) {
	g_lingo->printSTUBWithArglist("b_findPosNear", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_getaProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_getaProp", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_getAt(int nargs) {
	g_lingo->printSTUBWithArglist("b_getAt", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_getLast(int nargs) {
	g_lingo->printSTUBWithArglist("b_getLast", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_getOne(int nargs) {
	g_lingo->printSTUBWithArglist("b_getOne", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_getPos(int nargs) {
	g_lingo->printSTUBWithArglist("b_getPos", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_getProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_getProp", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_getPropAt(int nargs) {
	g_lingo->printSTUBWithArglist("b_getPropAt", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_list(int nargs) {
	g_lingo->printSTUBWithArglist("b_list", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_listP(int nargs) {
	g_lingo->printSTUBWithArglist("b_listP", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_max(int nargs) {
	g_lingo->printSTUBWithArglist("b_max", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_min(int nargs) {
	g_lingo->printSTUBWithArglist("b_min", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_setaProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_setaProp", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_setAt(int nargs) {
	g_lingo->printSTUBWithArglist("b_setAt", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_setProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_setProp", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_sort(int nargs) {
	g_lingo->printSTUBWithArglist("b_sort", nargs);
	g_lingo->dropStack(nargs);
}


///////////////////
// Files
///////////////////
void Lingo::b_closeDA(int nargs) {
	warning("STUB: b_closeDA");
}

void Lingo::b_closeResFile(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_closeResFile(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_closeXlib(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_closeXlib(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_getNthFileNameInFolder(int nargs) {
	g_lingo->printSTUBWithArglist("b_getNthFileNameInFolder", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void Lingo::b_openDA(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_openDA(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_openResFile(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_openResFile(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_openXlib(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_openXlib(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_saveMovie(int nargs) {
	g_lingo->printSTUBWithArglist("b_saveMovie", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_setCallBack(int nargs) {
	warning("STUB: b_setCallBack");
}

void Lingo::b_showResFile(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_showResFile(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_showXlib(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_showXlib(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_xFactoryList(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_xFactoryList(%s)", d.u.s->c_str());

	delete d.u.s;
}

///////////////////
// Control
///////////////////
void Lingo::b_abort(int nargs) {
	warning("STUB: b_abort");
}

void Lingo::b_continue(int nargs) {
	g_director->_playbackPaused = false;
}

void Lingo::b_dontPassEvent(int nargs) {
	warning("STUB: b_dontPassEvent");
}

void Lingo::b_nothing(int nargs) {
	warning("STUB: b_nothing");
}

void Lingo::b_delay(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	warning("STUB: b_delay(%d)", d.u.i);
}

void Lingo::b_do(int nargs) {
	Datum d = g_lingo->pop();
	d.toString();
	warning("STUB: b_do(%s)", d.u.s->c_str());
}

void Lingo::b_halt(int nargs) {
	b_quit(nargs);

	warning("Movie halted");
}

void Lingo::b_pass(int nargs) {
	g_lingo->printSTUBWithArglist("b_pass", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_pause(int nargs) {
	g_director->_playbackPaused = true;

	g_lingo->pushVoid();	// Fake value
}

void Lingo::b_playAccel(int nargs) {
	g_lingo->printSTUBWithArglist("b_playAccel", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_preLoad(int nargs) {
	g_lingo->printSTUBWithArglist("b_preLoad", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_preLoadCast(int nargs) {
	g_lingo->printSTUBWithArglist("b_preLoadCast", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_framesToHMS(int nargs) {
	g_lingo->printSTUBWithArglist("b_framesToHMS", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void Lingo::b_HMStoFrames(int nargs) {
	g_lingo->printSTUBWithArglist("b_HMStoFrames", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void Lingo::b_param(int nargs) {
	g_lingo->printSTUBWithArglist("b_param", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_printFrom(int nargs) {
	g_lingo->printSTUBWithArglist("b_printFrom", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_quit(int nargs) {
	if (g_director->_currentScore)
		g_director->_currentScore->_stopPlay = true;

	g_lingo->pushVoid();
}

void Lingo::b_restart(int nargs) {
	b_quit(nargs);

	warning("Computer restarts");
}

void Lingo::b_shutDown(int nargs) {
	b_quit(nargs);

	warning("Computer shuts down");
}

void Lingo::b_startTimer(int nargs) {
	warning("STUB: b_startTimer");
}

///////////////////
// Types
///////////////////
void Lingo::b_floatP(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == FLOAT) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}

void Lingo::b_ilk(int nargs) {
	Datum d = g_lingo->pop();
	d.u.i = d.type;
	d.type = SYMBOL;
	g_lingo->push(d);
}

void Lingo::b_integerp(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == INT) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}

void Lingo::b_objectp(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == OBJECT) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}

void Lingo::b_pictureP(int nargs) {
	g_lingo->pop();
	warning("STUB: b_pictureP");
	g_lingo->push(Datum(0));
}

void Lingo::b_stringp(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == STRING) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}

void Lingo::b_symbolp(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == SYMBOL) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}


///////////////////
// Misc
///////////////////
void Lingo::b_alert(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_alert(%s)", d.u.s->c_str());

	delete d.u.s;
}

void Lingo::b_cursor(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	g_lingo->func_cursor(d.u.i);
}

void Lingo::b_showGlobals(int nargs) {
	warning("STUB: b_showGlobals");
}

void Lingo::b_showLocals(int nargs) {
	warning("STUB: b_showLocals");
}

///////////////////
// Score
///////////////////
void Lingo::b_constrainH(int nargs) {
	Datum num = g_lingo->pop();
	Datum sprite = g_lingo->pop();

	num.toInt();
	sprite.toInt();

	warning("STUB: b_constrainH(%d, %d)", sprite.u.i, num.u.i);

	g_lingo->push(Datum(0));
}

void Lingo::b_constrainV(int nargs) {
	Datum num = g_lingo->pop();
	Datum sprite = g_lingo->pop();

	num.toInt();
	sprite.toInt();

	warning("STUB: b_constrainV(%d, %d)", sprite.u.i, num.u.i);

	g_lingo->push(Datum(0));
}

void Lingo::b_copyToClipBoard(int nargs) {
	g_lingo->printSTUBWithArglist("b_copyToClipBoard", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_duplicate(int nargs) {
	g_lingo->printSTUBWithArglist("b_duplicate", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_editableText(int nargs) {
	warning("STUB: b_editableText");
}

void Lingo::b_erase(int nargs) {
	g_lingo->printSTUBWithArglist("b_erase", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_findEmpty(int nargs) {
	g_lingo->printSTUBWithArglist("b_findEmpty", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void Lingo::b_importFileInto(int nargs) {
	g_lingo->printSTUBWithArglist("b_importFileInto", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_installMenu(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_installMenu(%d)", d.u.i);
}

void Lingo::b_intersect(int nargs) {
	g_lingo->printSTUBWithArglist("b_intersect", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void Lingo::b_label(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	warning("STUB: b_label(%d)", d.u.i);

	g_lingo->push(Datum(0));
}

void Lingo::b_marker(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	int marker = g_lingo->func_marker(d.u.i);
	g_lingo->push(marker);
}

void Lingo::b_move(int nargs) {
	g_lingo->printSTUBWithArglist("b_move", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_moveableSprite(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_moveableSprite(%d)", d.u.i);
}

void Lingo::b_pasteClipBoardInto(int nargs) {
	g_lingo->printSTUBWithArglist("b_pasteClipBoardInto", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_puppetPalette(int nargs) {
	g_lingo->convertVOIDtoString(0, nargs);

	g_lingo->printSTUBWithArglist("b_puppetPalette", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_puppetSound(int nargs) {
	g_lingo->convertVOIDtoString(0, nargs);

	g_lingo->printSTUBWithArglist("b_puppetSound", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_puppetSprite(int nargs) {
	g_lingo->printSTUBWithArglist("b_puppetSprite", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_puppetTempo(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_puppetTempo(%d)", d.u.i);
}

void Lingo::b_puppetTransition(int nargs) {
	g_lingo->printSTUBWithArglist("b_puppetTransition", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_rollOver(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_puppetTempo(%d)", d.u.i);

	g_lingo->push(Datum(0));
}

void Lingo::b_spriteBox(int nargs) {
	g_lingo->printSTUBWithArglist("b_spriteBox", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_unLoad(int nargs) {
	g_lingo->printSTUBWithArglist("b_unLoad", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_unLoadCast(int nargs) {
	g_lingo->printSTUBWithArglist("b_unLoadCast", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_zoomBox(int nargs) {
	g_lingo->printSTUBWithArglist("b_zoomBox", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_updateStage(int nargs) {
	warning("STUB: b_updateStage");
}


///////////////////
// Window
///////////////////

void Lingo::b_close(int nargs) {
	g_lingo->printSTUBWithArglist("b_close", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_forget(int nargs) {
	g_lingo->printSTUBWithArglist("b_forget", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_inflate(int nargs) {
	g_lingo->printSTUBWithArglist("b_inflate", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_moveToBack(int nargs) {
	g_lingo->printSTUBWithArglist("b_moveToBack", nargs);
	g_lingo->dropStack(nargs);
}

void Lingo::b_moveToFront(int nargs) {
	g_lingo->printSTUBWithArglist("b_moveToFront", nargs);
	g_lingo->dropStack(nargs);
}


///////////////////
// Point
///////////////////
void Lingo::b_point(int nargs) {
	Datum y = g_lingo->pop();
	Datum x = g_lingo->pop();
	Datum d;

	x.toFloat();
	y.toFloat();

	d.u.arr = new FloatArray;

	d.u.arr->push_back(x.u.f);
	d.u.arr->push_back(y.u.f);
	d.type = POINT;

	g_lingo->push(d);
}


///////////////////
// Sound
///////////////////
void Lingo::b_beep(int nargs) {
	Datum d = g_lingo->pop();
	g_lingo->func_beep(d.u.i);
}

void Lingo::b_mci(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	g_lingo->func_mci(*d.u.s);
}

void Lingo::b_mciwait(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	g_lingo->func_mciwait(*d.u.s);
}

void Lingo::b_soundBusy(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundBusy", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_soundFadeIn(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundFadeIn", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_soundFadeOut(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundFadeOut", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_soundPlayFile(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundPlayFile", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_soundStop(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundStop", nargs);

	g_lingo->dropStack(nargs);
}

///////////////////
// Constants
///////////////////
void Lingo::b_backspace(int nargs) {
	g_lingo->push(Datum(new Common::String("\b")));
}

void Lingo::b_empty(int nargs) {
	g_lingo->push(Datum(new Common::String("")));
}

void Lingo::b_enter(int nargs) {
	g_lingo->push(Datum(new Common::String("\n")));
}

void Lingo::b_false(int nargs) {
	g_lingo->push(Datum(0));
}

void Lingo::b_quote(int nargs) {
	g_lingo->push(Datum(new Common::String("\"")));
}

void Lingo::b_return(int nargs) {
	g_lingo->push(Datum(new Common::String("\r")));
}

void Lingo::b_tab(int nargs) {
	g_lingo->push(Datum(new Common::String("\t")));
}

void Lingo::b_true(int nargs) {
	g_lingo->push(Datum(1));
}

void Lingo::b_version(int nargs) {
	g_lingo->push(Datum(g_director->getVersion()));
}

///////////////////
// Factory
///////////////////
void Lingo::b_factory(int nargs) {
	// This is intentionally empty
}

// TODO:
// List of predefined methods in D3
// mAtFrame
// mDescribe
// mDispose
// mGet
// mInstanceRespondsTo
// mMessageList
// mName
// mNew
// mPerform
// mPut
// mRespondsTo

void Lingo::factoryCall(Common::String &name, int nargs) {
	Common::String s("factoryCall: ");

	s += name;

	convertVOIDtoString(0, nargs);

	printSTUBWithArglist(s.c_str(), nargs);

	Datum method = _stack[_stack.size() - nargs + 0];

	drop(nargs - 1);

	s = name + "-" + *method.u.s;

	debugC(3, kDebugLingoExec, "Stack size before call: %d, nargs: %d", _stack.size(), nargs);
	call(s, nargs);
	debugC(3, kDebugLingoExec, "Stack size after call: %d", _stack.size());

	if (!method.u.s->compareToIgnoreCase("mNew")) {
		Datum d;

		d.type = OBJECT;
		d.u.s = new Common::String(name);

		g_lingo->push(d);
	}
}

///////////////////
// References
///////////////////
void Lingo::b_cast(int nargs) {
	warning("STUB: b_cast");
}

void Lingo::b_field(int nargs) {
	warning("STUB: b_field");
}

void Lingo::b_me(int nargs) {
	g_lingo->printSTUBWithArglist("me", nargs);

	g_lingo->dropStack(nargs);
}

void Lingo::b_window(int nargs) {
	g_lingo->printSTUBWithArglist("b_window", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}


} // End of namespace Director
