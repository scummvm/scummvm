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

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/stxt.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macmenu.h"

namespace Director {

static struct BuiltinProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	bool parens;
	int version;
	int type;
} builtins[] = {
	// Math
	{ "abs",			LB::b_abs,			1, 1, true, 2, FBLTIN },	// D2 function
	{ "atan",			LB::b_atan,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "cos",			LB::b_cos,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "exp",			LB::b_exp,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "float",			LB::b_float,		1, 1, true, 4, FBLTIN },	//			D4 f
	{ "integer",		LB::b_integer,		1, 1, true, 3, FBLTIN },	//		D3 f
	{ "log",			LB::b_log,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "pi",				LB::b_pi,			0, 0, true, 4, FBLTIN },	//			D4 f
	{ "power",			LB::b_power,		2, 2, true, 4, FBLTIN },	//			D4 f
	{ "random",			LB::b_random,		1, 1, true, 2, FBLTIN },	// D2 f
	{ "sin",			LB::b_sin,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "sqrt",			LB::b_sqrt,			1, 1, true, 2, FBLTIN },	// D2 f
	{ "tan",			LB::b_tan,			1, 1, true, 4, FBLTIN },	//			D4 f
	// String
	{ "chars",			LB::b_chars,		3, 3, true, 2, FBLTIN },	// D2 f
	{ "charToNum",		LB::b_charToNum,	1, 1, true, 2, FBLTIN },	// D2 f
	{ "delete",			LB::b_delete,		1, 1, true, 3, BLTIN },		//		D3 command
	{ "hilite",			LB::b_hilite,		1, 1, true, 3, BLTIN },		//		D3 c
	{ "length",			LB::b_length,		1, 1, true, 2, FBLTIN },	// D2 f
	{ "numToChar",		LB::b_numToChar,	1, 1, true, 2, FBLTIN },	// D2 f
	{ "offset",			LB::b_offset,		2, 3, true, 2, FBLTIN },	// D2 f
	{ "string",			LB::b_string,		1, 1, true, 2, FBLTIN },	// D2 f
	{ "value",		 	LB::b_value,		1, 1, true, 2, FBLTIN },	// D2 f
	// Lists
	{ "add",			LB::b_add,			2, 2, false, 4, BLTIN },	//			D4 c
	{ "addAt",			LB::b_addAt,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "addProp",		LB::b_addProp,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "append",			LB::b_append,		2, 2, false, 4, BLTIN },	//			D4 c
	{ "count",			LB::b_count,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "deleteAt",		LB::b_deleteAt,		2, 2, false, 4, BLTIN },	//			D4 c
	{ "deleteProp",		LB::b_deleteProp,	2, 2, false, 4, BLTIN },	//			D4 c
	{ "findPos",		LB::b_findPos,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "findPosNear",	LB::b_findPosNear,	2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getaProp",		LB::b_getaProp,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getAt",			LB::b_getAt,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getLast",		LB::b_getLast,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "getOne",			LB::b_getOne,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getPos",			LB::b_getPos,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getProp",		LB::b_getProp,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getPropAt",		LB::b_getPropAt,	2, 2, true,  4, FBLTIN },	//			D4 f
	{ "list",			LB::b_list,			-1, 0, true, 4, FBLTIN },	//			D4 f
	{ "listP",			LB::b_listP,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "max",			LB::b_max,			1, 1, true,  4, FBLTIN },	//			D4 f
	{ "min",			LB::b_min,			1, 1, true,  4, FBLTIN },	//			D4 f
	{ "setaProp",		LB::b_setaProp,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "setAt",			LB::b_setAt,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "setProp",		LB::b_setProp,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "sort",			LB::b_sort,			1, 1, false, 4, BLTIN },	//			D4 c
	// Files
	{ "closeDA",	 	LB::b_closeDA, 		0, 0, false, 2, BLTIN },	// D2 c
	{ "closeResFile",	LB::b_closeResFile,	0, 1, false, 2, BLTIN },	// D2 c
	{ "closeXlib",		LB::b_closeXlib,	0, 1, false, 2, BLTIN },	// D2 c
	{ "getNthFileNameInFolder",LB::b_getNthFileNameInFolder,2,2,true,4,FBLTIN },//	D4 f
		// open															// D2 c
	{ "openDA",	 		LB::b_openDA, 		1, 1, false, 2, BLTIN },	// D2 c
	{ "openResFile",	LB::b_openResFile,	1, 1, false, 2, BLTIN },	// D2 c
	{ "openXlib",		LB::b_openXlib,		1, 1, false, 2, BLTIN },	// D2 c
	{ "saveMovie",		LB::b_saveMovie,	1, 1, false, 4, BLTIN },	//			D4 c
	{ "setCallBack",	LB::b_setCallBack,	2, 2, false, 3, BLTIN },	//		D3 c
	{ "showResFile",	LB::b_showResFile,	0, 1, false, 2, BLTIN },	// D2 c
	{ "showXlib",		LB::b_showXlib,		0, 1, false, 2, BLTIN },	// D2 c
	{ "xFactoryList",	LB::b_xFactoryList,	1, 1, true,  3, FBLTIN },	//		D3 f
	// Control
	{ "abort",			LB::b_abort,		0, 0, false, 4, BLTIN },	//			D4 c
	{ "continue",		LB::b_continue,		0, 0, false, 2, BLTIN },	// D2 c
	{ "dontPassEvent",	LB::b_dontPassEvent,0, 0, false, 2, BLTIN },	// D2 c
	{ "delay",	 		LB::b_delay,		1, 1, false, 2, BLTIN },	// D2 c
	{ "do",		 		LB::b_do,			1, 1, false, 2, BLTIN },	// D2 c
	{ "go",		 		LB::b_go,			1, 2, false, 4, BLTIN },	// 			D4 c
	{ "halt",	 		LB::b_halt,			0, 0, false, 4, BLTIN },	//			D4 c
	{ "nothing",		LB::b_nothing,		0, 0, false, 2, BLTIN },	// D2 c
	{ "pass",			LB::b_pass,			0, 0, false, 4, BLTIN },	//			D4 c
	{ "pause",			LB::b_pause,		0, 0, false, 2, BLTIN },	// D2 c
		// play															// D2 c
	{ "playAccel",		LB::b_playAccel,	-1,0, false, 2, BLTIN },	// D2
		// play done													// D2
	{ "preLoad",		LB::b_preLoad,		-1,0, false, 3, BLTIN },	//		D3.1 c
	{ "preLoadCast",	LB::b_preLoadCast,	-1,0, false, 3, BLTIN },	//		D3.1 c
	{ "quit",			LB::b_quit,			0, 0, false, 2, BLTIN },	// D2 c
	{ "restart",		LB::b_restart,		0, 0, false, 2, BLTIN },	// D2 c
	{ "return",			LB::b_return,		0, 1, false, 2, BLTIN },	// D2 f
	{ "shutDown",		LB::b_shutDown,		0, 0, false, 2, BLTIN },	// D2 c
	{ "startTimer",		LB::b_startTimer,	0, 0, false, 2, BLTIN },	// D2 c
		// when keyDown													// D2
		// when mouseDown												// D2
		// when mouseUp													// D2
		// when timeOut													// D2
	// Types
	{ "factory",		LB::b_factoryP,		1, 1, true,  3, FBLTIN },	//		D3
	{ "floatP",			LB::b_floatP,		1, 1, true,  3, FBLTIN },	//		D3
	{ "ilk",	 		LB::b_ilk,			1, 2, false, 4, FBLTIN },	//			D4 f
	{ "integerp",		LB::b_integerp,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "objectp",		LB::b_objectp,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "pictureP",		LB::b_pictureP,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "stringp",		LB::b_stringp,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "symbolp",		LB::b_symbolp,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "voidP",			LB::b_voidP,		1, 1, true,  4, FBLTIN },	//			D4 f
	// Misc
	{ "alert",	 		LB::b_alert,		1, 1, false, 2, BLTIN },	// D2 c
	{ "birth",	 		LB::b_birth,		-1,0, false, 4, FBLTIN },	//			D4 f
	{ "clearGlobals",	LB::b_clearGlobals,	0, 0, false, 3, BLTIN },	//		D3.1 c
	{ "cursor",	 		LB::b_cursor,		1, 1, false, 2, BLTIN },	// D2 c
	{ "framesToHMS",	LB::b_framesToHMS,	4, 4, false, 3, FBLTIN },	//		D3 f
	{ "HMStoFrames",	LB::b_HMStoFrames,	4, 4, false, 3, FBLTIN },	//		D3 f
	{ "param",	 		LB::b_param,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "printFrom",	 	LB::b_printFrom,	-1,0, false, 2, BLTIN },	// D2 c
		// put															// D2
		// set															// D2
	{ "showGlobals",	LB::b_showGlobals,	0, 0, false, 2, BLTIN },	// D2 c
	{ "showLocals",		LB::b_showLocals,	0, 0, false, 2, BLTIN },	// D2 c
	// Score
	{ "constrainH",		LB::b_constrainH,	2, 2, true,  2, FBLTIN },	// D2 f
	{ "constrainV",		LB::b_constrainV,	2, 2, true,  2, FBLTIN },	// D2 f
	{ "copyToClipBoard",LB::b_copyToClipBoard,1,1, false, 4, BLTIN },	//			D4 c
	{ "duplicate",		LB::b_duplicate,	1, 2, false, 4, BLTIN },	//			D4 c
	{ "editableText",	LB::b_editableText,	0, 0, false, 2, BLTIN },	// D2, FIXME: the field in D4+
	{ "erase",			LB::b_erase,		1, 1, false, 4, BLTIN },	//			D4 c
	{ "findEmpty",		LB::b_findEmpty,	1, 1, true,  4, FBLTIN },	//			D4 f
		// go															// D2
	{ "importFileInto",	LB::b_importFileInto,2, 2, false, 4, BLTIN },	//			D4 c
	{ "installMenu",	LB::b_installMenu,	1, 1, false, 2, BLTIN },	// D2 c
	{ "label",			LB::b_label,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "marker",			LB::b_marker,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "move",			LB::b_move,			1, 2, false, 4, BLTIN },	//			D4 c
	{ "moveableSprite",	LB::b_moveableSprite,0, 0, false, 2, BLTIN },	// D2, FIXME: the field in D4+
	{ "pasteClipBoardInto",LB::b_pasteClipBoardInto,1,1,false,4,BLTIN },//			D4 c
	{ "puppetPalette",	LB::b_puppetPalette, -1,0, false, 2, BLTIN },	// D2 c
	{ "puppetSound",	LB::b_puppetSound,	-1,0, false, 2, BLTIN },	// D2 c
	{ "puppetSprite",	LB::b_puppetSprite,	-1,0, false, 2, BLTIN },	// D2 c
	{ "puppetTempo",	LB::b_puppetTempo,	1, 1, false, 2, BLTIN },	// D2 c
	{ "puppetTransition",LB::b_puppetTransition,-1,0,false,2, BLTIN },	// D2 c
	{ "ramNeeded",		LB::b_ramNeeded,	2, 2, true,  3, FBLTIN },	//		D3.1 f
	{ "rollOver",		LB::b_rollOver,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "spriteBox",		LB::b_spriteBox,	-1,0, false, 2, BLTIN },	// D2 c
	{ "unLoad",			LB::b_unLoad,		0, 2, false, 3, BLTIN },	//		D3.1 c
	{ "unLoadCast",		LB::b_unLoadCast,	0, 2, false, 3, BLTIN },	//		D3.1 c
	{ "updateStage",	LB::b_updateStage,	0, 0, false, 2, BLTIN },	// D2 c
	{ "zoomBox",		LB::b_zoomBox,		-1,0, false, 2, BLTIN },	// D2 c
	// Point
	{ "point",			LB::b_point,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "inside",			LB::b_inside,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "intersect",		LB::b_intersect,	2, 2, false, 4, FBLTIN },	//			D4 f
	{ "map",			LB::b_map,			3, 3, true,  4, FBLTIN },	//			D4 f
	{ "rect",			LB::b_rect,			4, 4, true,  4, FBLTIN },	//			D4 f
	{ "union",			LB::b_union,		2, 2, true,  4, FBLTIN },	//			D4 f
	// Sound
	{ "beep",	 		LB::b_beep,			0, 1, false, 2, BLTIN },	// D2
	{ "mci",	 		LB::b_mci,			1, 1, false, 3, BLTIN },	//		D3.1 c
	{ "mciwait",		LB::b_mciwait,		1, 1, false, 4, BLTIN },	//			D4 c
	{ "sound-close",	LB::b_soundClose, 	1, 1, false, 4, BLTIN },	//			D4 c
	{ "sound-fadeIn",	LB::b_soundFadeIn, 	1, 2, false, 3, BLTIN },	//		D3 c
	{ "sound-fadeOut",	LB::b_soundFadeOut, 1, 2, false, 3, BLTIN },	//		D3 c
	{ "sound-playFile",	LB::b_soundPlayFile,2, 2, false, 3, BLTIN },	//		D3 c
	{ "sound-stop",		LB::b_soundStop,	1, 1, false, 2, BLTIN },	//		D2 c
	{ "soundBusy",		LB::b_soundBusy,	1, 1, true,  3, FBLTIN },	//		D3 f
	// Window
	{ "close",			LB::b_close,		1, 1, false, 4, BLTIN },	//			D4 c
	{ "forget",			LB::b_forget,		1, 1, false, 4, BLTIN },	//			D4 c
	{ "inflate",		LB::b_inflate,		3, 3, true,  4, FBLTIN },	//			D4 f
	{ "moveToBack",		LB::b_moveToBack,	1, 1, false, 4, BLTIN },	//			D4 c
	{ "moveToFront",	LB::b_moveToFront,	1, 1, false, 4, BLTIN },	//			D4 c
	// Constants
	{ "backspace",		LB::b_backspace,	0, 0, false, 2, FBLTIN },	// D2
	{ "empty",			LB::b_empty,		0, 0, false, 2, FBLTIN },	// D2
	{ "enter",			LB::b_enter,		0, 0, false, 2, FBLTIN },	// D2
	{ "false",			LB::b_false,		0, 0, false, 2, FBLTIN },	// D2
	{ "quote",			LB::b_quote,		0, 0, false, 2, FBLTIN },	// D2
	{ "scummvm_return",	LB::b_returnconst,	0, 0, false, 2, FBLTIN },	// D2
	{ "tab",			LB::b_tab,			0, 0, false, 2, FBLTIN },	// D2
	{ "true",			LB::b_true,			0, 0, false, 2, FBLTIN },	// D2
	{ "version",		LB::b_version,		0, 0, false, 3, FBLTIN },	//		D3
	// References
	{ "cast",			LB::b_cast,			1, 1, false, 4, RBLTIN },	//			D4 f
	{ "field",			LB::b_field,		1, 1, false, 3, RBLTIN },	//		D3 f
//	{ "me",				LB::b_me,			-1,0, false, 3, FBLTIN },	//		D3				// works as normal ID, see c_varpush
	{ "script",			LB::b_script,		1, 1, false, 4, RBLTIN },	//			D4 f
	{ "window",			LB::b_window,		1, 1, false, 4, RBLTIN },	//			D4 f
	// Chunk operations
	{ "numberOfChars",	LB::b_numberofchars,1, 1, false, 4, FBLTIN },	//			D4 f
	{ "numberOfItems",	LB::b_numberofitems,1, 1, false, 4, FBLTIN },	//			D4 f
	{ "numberOfLines",	LB::b_numberoflines,1, 1, false, 4, FBLTIN },	//			D4 f
	{ "numberOfWords",	LB::b_numberofwords,1, 1, false, 4, FBLTIN },	//			D4 f
	{ "lastCharOf",		LB::b_lastcharof,	1, 1, false, 4, FBLTIN },	//			D4 f
	{ "lastItemOf",		LB::b_lastitemof,	1, 1, false, 4, FBLTIN },	//			D4 f
	{ "lastLineOf",		LB::b_lastlineof,	1, 1, false, 4, FBLTIN },	//			D4 f
	{ "lastWordOf",		LB::b_lastwordof,	1, 1, false, 4, FBLTIN },	//			D4 f

	{ 0, 0, 0, 0, false, 0, 0 }
};

static const char *twoWordBuiltins[] = {
	//"duplicate",
	//"erase",
	"sound",
	0
};

static const char *predefinedMethods[] = {
	"mAtFrame",				// D3
	"mDescribe",			// D3
	"mDispose",				// D3
	"mGet",					// D3
	"mInstanceRespondsTo",	// D3
	"mMessageList",			// D3
	"mName",				// D3
	"mNew",					// D3
	"mPerform",				// D3
	"mPut",					// D3
	"mRespondsTo",			// D3
	0
};

void Lingo::initBuiltIns() {
	for (BuiltinProto *blt = builtins; blt->name; blt++) {
		if (blt->version > _vm->getVersion())
			continue;

		Symbol *sym = new Symbol;

		sym->name = blt->name;
		sym->type = blt->type;
		sym->nargs = blt->minArgs;
		sym->maxArgs = blt->maxArgs;
		sym->parens = blt->parens;
		sym->u.bltin = blt->func;

		_builtins[blt->name] = sym;

		_functions[(void *)sym->u.s] = new FuncDesc(blt->name, "");
	}

	for (const char **b = twoWordBuiltins; *b; b++)
		_twoWordBuiltins[*b] = true;

	// Set predefined methods
	for (const char **b = predefinedMethods; *b; b++) {
		Symbol *s = g_lingo->lookupVar(*b, true, true);
		s->type = SYMBOL;
		s->u.s = new Common::String(*b);
	}
}

void Lingo::printSTUBWithArglist(const char *funcname, int nargs, const char *prefix) {
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

	debug(5, "%s %s", prefix, s.c_str());
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
void LB::b_abs(int nargs) {
	Datum d = g_lingo->pop();

	if (d.type == INT)
		d.u.i = ABS(d.u.i);
	else if (d.type == FLOAT)
		d.u.f = ABS(d.u.f);

	g_lingo->push(d);
}

void LB::b_atan(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = atan(d.u.f);
	g_lingo->push(d);
}

void LB::b_cos(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = cos(d.u.f);
	g_lingo->push(d);
}

void LB::b_exp(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt(); // Lingo uses int, so we're enforcing it
	d.toFloat();
	d.u.f = exp(d.u.f);
	g_lingo->push(d);
}

void LB::b_float(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	g_lingo->push(d);
}

void LB::b_integer(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	g_lingo->push(d);
}

void LB::b_log(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = log(d.u.f);
	g_lingo->push(d);
}

void LB::b_pi(int nargs) {
	Datum d;
	d.toFloat();
	d.u.f = M_PI;
	g_lingo->push(d);
}

void LB::b_power(int nargs) {
	Datum d1 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	d1.toFloat();
	d2.toFloat();
	d1.u.f = pow(d2.u.f, d1.u.f);
	g_lingo->push(d1);
}

void LB::b_random(int nargs) {
	Datum max = g_lingo->pop();
	Datum res;

	max.toInt();

	res.u.i = g_lingo->_vm->_rnd.getRandomNumber(max.u.i - 1) + 1;
	res.type = INT;

	g_lingo->push(res);
}

void LB::b_sin(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = sin(d.u.f);
	g_lingo->push(d);
}

void LB::b_sqrt(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = sqrt(d.u.f);
	g_lingo->push(d);
}

void LB::b_tan(int nargs) {
	Datum d = g_lingo->pop();
	d.toFloat();
	d.u.f = tan(d.u.f);
	g_lingo->push(d);
}

///////////////////
// String
///////////////////
void LB::b_chars(int nargs) {
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

void LB::b_charToNum(int nargs) {
	Datum d = g_lingo->pop();

	if (d.type != STRING)
		error("Incorrect type for 'charToNum' function: %s", d.type2str());

	byte chr = d.u.s->c_str()[0];
	delete d.u.s;

	d.u.i = chr;
	d.type = INT;
	g_lingo->push(d);
}

void LB::b_delete(int nargs) {
	Datum d = g_lingo->pop();

	d.toInt();

	warning("STUB: b_delete");

	g_lingo->push(Datum((char)d.u.i));
}

void LB::b_hilite(int nargs) {
	Datum d = g_lingo->pop();

	d.toInt();

	warning("STUB: b_hilite");

	g_lingo->push(Datum((char)d.u.i));
}

void LB::b_length(int nargs) {
	Datum d = g_lingo->pop();
	if (d.type == REFERENCE)
		d.toString();

	if (d.type != STRING)
		error("Incorrect type for 'length' function: %s", d.type2str());

	int len = strlen(d.u.s->c_str());
	delete d.u.s;

	d.u.i = len;
	d.type = INT;
	g_lingo->push(d);
}

void LB::b_numToChar(int nargs) {
	Datum d = g_lingo->pop();

	d.toInt();

	g_lingo->push(Datum((char)d.u.i));
}

void LB::b_offset(int nargs) {
	if (nargs == 3) {
		b_offsetRect(nargs);
		return;
	}
	Datum target = g_lingo->pop();
	Datum source = g_lingo->pop();

	target.toString();
	source.toString();

	warning("STUB: b_offset()");

	g_lingo->push(Datum(0));
}

void LB::b_string(int nargs) {
	Datum d = g_lingo->pop();
	d.toString();
	g_lingo->push(d);
}

void LB::b_value(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	warning("STUB: b_value()");
	g_lingo->push(d);
}

///////////////////
// Lists
///////////////////
void LB::b_add(int nargs) {
	// FIXME: when a list is "sorted", add should insert based on
	// the current ordering. otherwise, append to the end.
	LB::b_append(nargs);
}

void LB::b_addAt(int nargs) {
	if (nargs != 3) {
		warning("b_addAt: expected 3 args, not %d", nargs);
		g_lingo->dropStack(nargs);
		return;
	}
	Datum value = g_lingo->pop();
	Datum index = g_lingo->pop();
	Datum list = g_lingo->pop();
	if (index.type != INT) {
		warning("b_addAt: index arg should be of type INT, not %s", index.type2str());
		return;
	}
	if (list.type != ARRAY) {
		warning("b_addAt: list arg should be of type ARRAY, not %s", list.type2str());
		return;
	}
	list.u.farr->insert_at(index.u.i-1, value);
}

void LB::b_addProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_addProp", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_append(int nargs) {
	if (nargs != 2) {
		warning("b_append: expected 2 args, not %d", nargs);
		g_lingo->dropStack(nargs);
		return;
	}
	Datum value = g_lingo->pop();
	Datum list = g_lingo->pop();
	if (list.type != ARRAY) {
		warning("b_append: list arg should be of type ARRAY, not %s", list.type2str());
		return;
	}
	list.u.farr->push_back(value);
}

void LB::b_count(int nargs) {
	if (nargs != 1) {
		warning("b_count: expected 1 args, not %d", nargs);
		g_lingo->dropStack(nargs);
		return;
	}
	Datum list = g_lingo->pop();
	if (list.type != ARRAY) {
		warning("b_append: list arg should be of type ARRAY, not %s", list.type2str());
		return;
	}
	Datum result;
	result.type = INT;
	result.u.i = list.u.farr->size();
	g_lingo->push(result);
}

void LB::b_deleteAt(int nargs) {
	if (nargs != 2) {
		warning("b_deleteAt: expected 2 args, not %d", nargs);
		g_lingo->dropStack(nargs);
		return;
	}
	Datum index = g_lingo->pop();
	Datum list = g_lingo->pop();
	if (index.type != INT) {
		warning("b_deleteAt: index arg should be of type INT, not %s", index.type2str());
		return;
	}
	if (list.type != ARRAY) {
		warning("b_deleteAt: list arg should be of type ARRAY, not %s", list.type2str());
		return;
	}
	list.u.farr->remove_at(index.u.i-1);
}

void LB::b_deleteProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_deleteProp", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_findPos(int nargs) {
	g_lingo->printSTUBWithArglist("b_findPos", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_findPosNear(int nargs) {
	g_lingo->printSTUBWithArglist("b_findPosNear", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_getaProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_getaProp", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_getAt(int nargs) {
	g_lingo->printSTUBWithArglist("b_getAt", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_getLast(int nargs) {
	g_lingo->printSTUBWithArglist("b_getLast", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_getOne(int nargs) {
	g_lingo->printSTUBWithArglist("b_getOne", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_getPos(int nargs) {
	g_lingo->printSTUBWithArglist("b_getPos", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_getProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_getProp", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_getPropAt(int nargs) {
	g_lingo->printSTUBWithArglist("b_getPropAt", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_list(int nargs) {
	Datum result;
	result.type = ARRAY;
	result.u.farr = new DatumArray;

	for (int i = 0; i < nargs; i++)
		result.u.farr->insert_at(0, g_lingo->pop());

	g_lingo->push(result);
}

void LB::b_listP(int nargs) {
	g_lingo->printSTUBWithArglist("b_listP", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_max(int nargs) {
	g_lingo->printSTUBWithArglist("b_max", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_min(int nargs) {
	g_lingo->printSTUBWithArglist("b_min", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_setaProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_setaProp", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_setAt(int nargs) {
	g_lingo->printSTUBWithArglist("b_setAt", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_setProp(int nargs) {
	g_lingo->printSTUBWithArglist("b_setProp", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_sort(int nargs) {
	g_lingo->printSTUBWithArglist("b_sort", nargs);
	g_lingo->dropStack(nargs);
}


///////////////////
// Files
///////////////////
void LB::b_closeDA(int nargs) {
	warning("STUB: b_closeDA");
}

void LB::b_closeResFile(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_closeResFile(%s)", d.u.s->c_str());

	delete d.u.s;
}

void LB::b_closeXlib(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_closeXlib(%s)", d.u.s->c_str());

	delete d.u.s;
}

void LB::b_getNthFileNameInFolder(int nargs) {
	g_lingo->printSTUBWithArglist("b_getNthFileNameInFolder", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_openDA(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_openDA(%s)", d.u.s->c_str());

	delete d.u.s;
}

void LB::b_openResFile(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_openResFile(%s)", d.u.s->c_str());

	delete d.u.s;
}

void LB::b_openXlib(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_openXlib(%s)", d.u.s->c_str());

	delete d.u.s;
}

void LB::b_saveMovie(int nargs) {
	g_lingo->printSTUBWithArglist("b_saveMovie", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_setCallBack(int nargs) {
	warning("STUB: b_setCallBack");
}

void LB::b_showResFile(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_showResFile(%s)", d.u.s->c_str());

	delete d.u.s;
}

void LB::b_showXlib(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_showXlib(%s)", d.u.s->c_str());

	delete d.u.s;
}

void LB::b_xFactoryList(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_xFactoryList(%s)", d.u.s->c_str());

	delete d.u.s;
}

///////////////////
// Control
///////////////////
void LB::b_abort(int nargs) {
	warning("STUB: b_abort");
}

void LB::b_continue(int nargs) {
	g_director->_playbackPaused = false;
}

void LB::b_dontPassEvent(int nargs) {
	g_lingo->_dontPassEvent = true;
	warning("dontPassEvent raised");
}

void LB::b_nothing(int nargs) {
	// Noop
}

void LB::b_delay(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();

	g_director->getCurrentScore()->_nextFrameTime = g_system->getMillis() + (float)d.u.i / 60 * 1000;
}

void LB::b_do(int nargs) {
	Datum d = g_lingo->pop();
	d.toString();
	warning("STUB: b_do(%s)", d.u.s->c_str());
}

void LB::b_go(int nargs) {
	// Builtin function for go as used by the Director bytecode engine.
	//
	// Accepted arguments:
	// "loop"
	// "next"
	// "previous"
	// (STRING|INT) frame
	// STRING movie, (STRING|INT) frame

	if (nargs >= 1 && nargs <= 2) {
		Datum firstArg = g_lingo->pop();
		nargs -= 1;
		bool callSpecial = false;

		if (firstArg.type == STRING) {
			if (*firstArg.u.s == "loop") {
				g_lingo->func_gotoloop();
				callSpecial = true;
			} else if (*firstArg.u.s == "next") {
				g_lingo->func_gotonext();
				callSpecial = true;
			} else if (*firstArg.u.s == "previous") {
				g_lingo->func_gotoprevious();
				callSpecial = true;
			}
		}

		if (!callSpecial) {
			Datum movie;
			Datum frame;

			if (nargs > 0) {
				movie = firstArg;
				if (movie.type != STRING) {
					warning("b_go: movie arg should be of type STRING, not %s", movie.type2str());
				}
				frame = g_lingo->pop();
				nargs -= 1;
			} else {
				frame = firstArg;
			}

			if (frame.type != STRING && frame.type != INT) {
				warning("b_go: frame arg should be of type STRING or INT, not %s", frame.type2str());
			}

			g_lingo->func_goto(frame, movie);
		}

		if (nargs > 0) {
			warning("b_go: ignoring %d extra args", nargs);
			g_lingo->dropStack(nargs);
		}

	} else {
		warning("b_go: expected 1 or 2 args, not %d", nargs);
		g_lingo->dropStack(nargs);
	}
}

void LB::b_halt(int nargs) {
	b_quit(nargs);

	warning("Movie halted");
}

void LB::b_pass(int nargs) {
	g_lingo->printSTUBWithArglist("b_pass", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_pause(int nargs) {
	g_director->_playbackPaused = true;
}

void LB::b_playAccel(int nargs) {
	g_lingo->printSTUBWithArglist("b_playAccel", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_preLoad(int nargs) {
	g_lingo->printSTUBWithArglist("b_preLoad", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_preLoadCast(int nargs) {
	g_lingo->printSTUBWithArglist("b_preLoadCast", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_framesToHMS(int nargs) {
	g_lingo->printSTUBWithArglist("b_framesToHMS", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_HMStoFrames(int nargs) {
	g_lingo->printSTUBWithArglist("b_HMStoFrames", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_param(int nargs) {
	g_lingo->printSTUBWithArglist("b_param", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_printFrom(int nargs) {
	g_lingo->printSTUBWithArglist("b_printFrom", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_quit(int nargs) {
	if (g_director->getCurrentScore())
		g_director->getCurrentScore()->_stopPlay = true;

	g_lingo->pushVoid();
}

void LB::b_return(int nargs) {
	// We do not touch the top of the stack, it will be returned
	LC::c_procret();
}

void LB::b_restart(int nargs) {
	b_quit(nargs);

	warning("Computer restarts");
}

void LB::b_shutDown(int nargs) {
	b_quit(nargs);

	warning("Computer shuts down");
}

void LB::b_startTimer(int nargs) {
	warning("STUB: b_startTimer");
}

///////////////////
// Types
///////////////////
void LB::b_factoryP(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	d.u.i = 1;
	g_lingo->push(d);

	warning("STUB: b_factoryP");
}

void LB::b_floatP(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == FLOAT) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}

void LB::b_ilk(int nargs) {
	Datum d = g_lingo->pop();
	d.u.i = d.type;
	d.type = SYMBOL;
	g_lingo->push(d);
}

void LB::b_integerp(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == INT) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}

void LB::b_objectp(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == OBJECT) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}

void LB::b_pictureP(int nargs) {
	g_lingo->pop();
	warning("STUB: b_pictureP");
	g_lingo->push(Datum(0));
}

void LB::b_stringp(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == STRING) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}

void LB::b_symbolp(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == SYMBOL) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}

void LB::b_voidP(int nargs) {
	Datum d = g_lingo->pop();
	int res = (d.type == VOID) ? 1 : 0;
	d.toInt();
	d.u.i = res;
	g_lingo->push(d);
}


///////////////////
// Misc
///////////////////
void LB::b_alert(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	warning("STUB: b_alert(%s)", d.u.s->c_str());

	delete d.u.s;
}

void LB::b_birth(int nargs) {
	g_lingo->printSTUBWithArglist("b_birth", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_clearGlobals(int nargs) {
	g_lingo->printSTUBWithArglist("b_clearGlobals", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_cursor(int nargs) {
	Datum d = g_lingo->pop();

	if (d.type == ARRAY) {
		Datum sprite = d.u.farr->operator[](0);
		Datum mask = d.u.farr->operator[](1);
		sprite.toInt();
		mask.toInt();

		g_lingo->func_cursor(sprite.u.i, mask.u.i);
	} else {
		d.toInt();
		g_lingo->func_cursor(d.u.i, -1);
	}
}

void LB::b_showGlobals(int nargs) {
	warning("STUB: b_showGlobals");
}

void LB::b_showLocals(int nargs) {
	warning("STUB: b_showLocals");
}

///////////////////
// Score
///////////////////
void LB::b_constrainH(int nargs) {
	Datum num = g_lingo->pop();
	Datum sprite = g_lingo->pop();

	num.toInt();
	sprite.toInt();

	warning("STUB: b_constrainH(%d, %d)", sprite.u.i, num.u.i);

	g_lingo->push(Datum(0));
}

void LB::b_constrainV(int nargs) {
	Datum num = g_lingo->pop();
	Datum sprite = g_lingo->pop();

	num.toInt();
	sprite.toInt();

	warning("STUB: b_constrainV(%d, %d)", sprite.u.i, num.u.i);

	g_lingo->push(Datum(0));
}

void LB::b_copyToClipBoard(int nargs) {
	g_lingo->printSTUBWithArglist("b_copyToClipBoard", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_duplicate(int nargs) {
	g_lingo->printSTUBWithArglist("b_duplicate", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_editableText(int nargs) {
	warning("STUB: b_editableText");
}

void LB::b_erase(int nargs) {
	g_lingo->printSTUBWithArglist("b_erase", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_findEmpty(int nargs) {
	g_lingo->printSTUBWithArglist("b_findEmpty", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_importFileInto(int nargs) {
	g_lingo->printSTUBWithArglist("b_importFileInto", nargs);

	g_lingo->dropStack(nargs);
}

void menuCommandsCallback(int action, Common::String &text, void *data) {
	Common::String name = Common::String::format("scummvmMenu%d", action);

	LC::call(name, 0);
}

void LB::b_installMenu(int nargs) {
	// installMenu castNum
	Datum d = g_lingo->pop();

	d.toInt();

	if (g_director->getVersion() < 4)
		d.u.i += g_director->getCurrentScore()->_castIDoffset;

	const Stxt *stxt = g_director->getCurrentScore()->_loadedStxts->getVal(d.u.i, nullptr);

	if (!stxt) {
		warning("installMenu: Unknown cast number #%d", d.u.i);
		return;
	}

	Common::String menuStxt = g_lingo->codePreprocessor(stxt->_ptext.c_str(), true);
	Common::String line;
	int linenum = -1; // We increment it before processing

	Graphics::MacMenu *menu = g_director->_wm->addMenu();
	int submenu = -1;
	Common::String submenuText;
	Common::String command;
	int commandId = 100;

	Common::String handlers;

	menu->setCommandsCallback(menuCommandsCallback, g_director);

	debugC(3, kDebugLingoExec, "installMenu: '%s'", Common::toPrintable(menuStxt).c_str());

	for (const byte *s = (const byte *)menuStxt.c_str(); *s; s++) {
		// Get next line
		line.clear();
		while (*s && *s != '\n') { // If we see a whitespace
			if (*s == (byte)'\xc2') {
				s++;
				if (*s == '\n') {
					line += ' ';

					s++;
				}
			} else {
				line += *s++;
			}
		}

		linenum++;

		if (line.empty())
			continue;

		if (line.hasPrefixIgnoreCase("menu:")) {
			const char *p = &line.c_str()[5];

			while (*p && (*p == ' ' || *p == '\t'))
				p++;

			if (!submenuText.empty()) { // Adding submenu for previous menu
				if (!command.empty()) {
					handlers += g_lingo->genMenuHandler(&commandId, command);
					submenuText += Common::String::format("[%d]", commandId);
				}

				menu->createSubMenuFromString(submenu, submenuText.c_str(), 0);
			}

			if (!strcmp(p, "@"))
				p = "\xf0";	// Apple symbol

			submenu = menu->addMenuItem(nullptr, Common::String(p));

			submenuText.clear();

			continue;
		}

		// We have either '=' or \xc5 as a separator
		const char *p = strchr(line.c_str(), '=');

		if (!p)
			p = strchr(line.c_str(), '\xc5');

		Common::String text;

		if (p) {
			text = Common::String(line.c_str(), p);
			command = Common::String(p + 1);
		} else {
			text = line;
			command = "";
		}

		text.trim();
		command.trim();

		if (!submenuText.empty()) {
			if (!command.empty()) {
				handlers += g_lingo->genMenuHandler(&commandId, command);
				submenuText += Common::String::format("[%d];", commandId);
			} else {
				submenuText += ';';
			}
		}

		submenuText += text;

		if (!*s) // if we reached end of string, do not increment it but break
			break;
	}

	if (!submenuText.empty()) {
		if (!command.empty()) {
			handlers += g_lingo->genMenuHandler(&commandId, command);
			submenuText += Common::String::format("[%d]", commandId);
		}
		menu->createSubMenuFromString(submenu, submenuText.c_str(), 0);
	}

	g_lingo->addCode(handlers.c_str(), kMovieScript, 1337);
}

Common::String Lingo::genMenuHandler(int *commandId, Common::String &command) {
	Common::String name;

	do {
		(*commandId)++;

		name = Common::String::format("scummvmMenu%d", *commandId);
	} while (getHandler(name) != NULL);

	return Common::String::format("on %s\n  %s\nend %s\n\n", name.c_str(), command.c_str(), name.c_str());
}

void LB::b_label(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	warning("STUB: b_label(%d)", d.u.i);

	g_lingo->push(Datum(0));
}

void LB::b_marker(int nargs) {
	Datum d = g_lingo->pop();
	d.toInt();
	int marker = g_lingo->func_marker(d.u.i);
	g_lingo->push(marker);
}

void LB::b_move(int nargs) {
	g_lingo->printSTUBWithArglist("b_move", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_moveableSprite(int nargs) {
	Frame *frame = g_director->getCurrentScore()->_frames[g_director->getCurrentScore()->getCurrentFrame()];

	// Will have no effect
	frame->_sprites[g_lingo->_currentEntityId]->_moveable = true;

	g_director->setDraggedSprite(frame->_sprites[g_lingo->_currentEntityId]->_castId);
}

void LB::b_pasteClipBoardInto(int nargs) {
	g_lingo->printSTUBWithArglist("b_pasteClipBoardInto", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_puppetPalette(int nargs) {
	g_lingo->convertVOIDtoString(0, nargs);

	g_lingo->printSTUBWithArglist("b_puppetPalette", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_puppetSound(int nargs) {
	g_lingo->convertVOIDtoString(0, nargs);

	g_lingo->printSTUBWithArglist("b_puppetSound", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_puppetSprite(int nargs) {
	g_lingo->printSTUBWithArglist("b_puppetSprite", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_puppetTempo(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_puppetTempo(%d)", d.u.i);
}

void LB::b_puppetTransition(int nargs) {
	g_lingo->printSTUBWithArglist("b_puppetTransition", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_ramNeeded(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_ramNeeded(%d)", d.u.i);

	g_lingo->push(Datum(0));
}

void LB::b_rollOver(int nargs) {
	Datum d = g_lingo->pop();

	d.toInt();

	int arg = d.u.i;

	d.u.i = 0; // FALSE

	if (!g_director->getCurrentScore()) {
		warning("b_rollOver: Reference to an empty score");
		return;
	}

	Frame *frame = g_director->getCurrentScore()->_frames[g_director->getCurrentScore()->getCurrentFrame()];

	if (arg >= (int32) frame->_sprites.size()) {
		g_lingo->push(d);
		return;
	}

	Common::Point pos = g_system->getEventManager()->getMousePos();

	if (frame->checkSpriteIntersection(arg, pos))
		d.u.i = 1; // TRUE

	g_lingo->push(d);
}

void LB::b_spriteBox(int nargs) {
	g_lingo->printSTUBWithArglist("b_spriteBox", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_unLoad(int nargs) {
	g_lingo->printSTUBWithArglist("b_unLoad", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_unLoadCast(int nargs) {
	g_lingo->printSTUBWithArglist("b_unLoadCast", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_zoomBox(int nargs) {
	// zoomBox startSprite, endSprite [, delatTicks]
	//   ticks are in 1/60th, default 1
	if (nargs < 2 || nargs > 3) {
		warning("b_zoomBox: expected 2 or 3 arguments, got %d", nargs);

		g_lingo->dropStack(nargs);

		return;
	}

	int delayTicks = 1;
	if (nargs > 2) {
		Datum d = g_lingo->pop();
		d.toInt();

		delayTicks = d.u.i;
	}

	Datum endSprite = g_lingo->pop();
	Datum startSprite = g_lingo->pop();

	startSprite.toInt();
	endSprite.toInt();

	Score *score = g_director->getCurrentScore();
	uint16 curFrame = score->getCurrentFrame();
	Frame *frame = score->_frames[curFrame];

	Common::Rect *startRect = frame->getSpriteRect(startSprite.u.i);
	if (!startRect) {
		warning("b_zoomBox: unknown start sprite #%d", startSprite.u.i);
		return;
	}

	// Looks for endSprite in the current frame, otherwise
	// Looks for endSprite in the next frame
	Common::Rect *endRect = frame->getSpriteRect(endSprite.u.i);
	if (!endRect) {
		if ((uint)curFrame + 1 < score->_frames.size())
			score->_frames[curFrame + 1]->getSpriteRect(endSprite.u.i);
	}

	if (!endRect) {
		warning("b_zoomBox: unknown end sprite #%d", endSprite.u.i);
		return;
	}

	ZoomBox *box = new ZoomBox;
	box->start = *startRect;
	box->end = *endRect;
	box->delay = delayTicks;
	box->step = 0;
	box->startTime = g_system->getMillis();
	box->nextTime  = g_system->getMillis() + 1000 * box->step / 60;

	score->addZoomBox(box);
}

void LB::b_updateStage(int nargs) {
	warning("STUB: b_updateStage");
}


///////////////////
// Window
///////////////////

void LB::b_close(int nargs) {
	g_lingo->printSTUBWithArglist("b_close", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_forget(int nargs) {
	g_lingo->printSTUBWithArglist("b_forget", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_inflate(int nargs) {
	g_lingo->printSTUBWithArglist("b_inflate", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_moveToBack(int nargs) {
	g_lingo->printSTUBWithArglist("b_moveToBack", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_moveToFront(int nargs) {
	g_lingo->printSTUBWithArglist("b_moveToFront", nargs);
	g_lingo->dropStack(nargs);
}


///////////////////
// Point
///////////////////
void LB::b_point(int nargs) {
	Datum y = g_lingo->pop();
	Datum x = g_lingo->pop();
	Datum d;

	x.toFloat();
	y.toFloat();

	d.u.farr = new DatumArray;

	d.u.farr->push_back(x);
	d.u.farr->push_back(y);
	d.type = POINT;

	g_lingo->push(d);
}

void LB::b_rect(int nargs) {
	g_lingo->printSTUBWithArglist("b_rect", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}


void LB::b_intersect(int nargs) {
	g_lingo->printSTUBWithArglist("b_intersect", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_inside(int nargs) {
	g_lingo->printSTUBWithArglist("b_inside", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_map(int nargs) {
	g_lingo->printSTUBWithArglist("b_map", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_offsetRect(int nargs) {
	g_lingo->printSTUBWithArglist("b_offsetRect", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_union(int nargs) {
	g_lingo->printSTUBWithArglist("b_union", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}


///////////////////
// Sound
///////////////////
void LB::b_beep(int nargs) {
	int repeat = 1;
	if (nargs == 1) {
		Datum d = g_lingo->pop();
		repeat = d.u.i;
	}
	g_lingo->func_beep(repeat);
}

void LB::b_mci(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	g_lingo->func_mci(*d.u.s);
}

void LB::b_mciwait(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();

	g_lingo->func_mciwait(*d.u.s);
}

void LB::b_soundBusy(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundBusy", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_soundClose(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundClose", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_soundFadeIn(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundFadeIn", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_soundFadeOut(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundFadeOut", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_soundPlayFile(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundPlayFile", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_soundStop(int nargs) {
	g_lingo->printSTUBWithArglist("b_soundStop", nargs);

	g_lingo->dropStack(nargs);
}

///////////////////
// Constants
///////////////////
void LB::b_backspace(int nargs) {
	g_lingo->push(Datum(new Common::String("\b")));
}

void LB::b_empty(int nargs) {
	g_lingo->push(Datum(new Common::String("")));
}

void LB::b_enter(int nargs) {
	g_lingo->push(Datum(new Common::String("\n")));
}

void LB::b_false(int nargs) {
	g_lingo->push(Datum(0));
}

void LB::b_quote(int nargs) {
	g_lingo->push(Datum(new Common::String("\"")));
}

void LB::b_returnconst(int nargs) {
	g_lingo->push(Datum(new Common::String("\n")));
}

void LB::b_tab(int nargs) {
	g_lingo->push(Datum(new Common::String("\t")));
}

void LB::b_true(int nargs) {
	g_lingo->push(Datum(1));
}

void LB::b_version(int nargs) {
	switch (g_director->getVersion()) {
	case 3:
		g_lingo->push(Datum(new Common::String("3.1.1"))); // Mac
		break;
	case 4:
		g_lingo->push(Datum(new Common::String("4.0"))); // Mac
		break;
	default:
		error("Unsupported Director for 'version'");
		break;
	}
}

///////////////////
// Factory
///////////////////
void LB::b_factory(int nargs) {
	// This is intentionally empty
}

void Lingo::factoryCall(Common::String &name, int nargs) {
	Common::String s("factoryCall: ");

	s += name;

	convertVOIDtoString(0, nargs);

	printSTUBWithArglist(s.c_str(), nargs);

	Datum method = _stack[_stack.size() - nargs + 0];

	drop(nargs - 1);

	s = name + "-" + *method.u.s;

	debugC(3, kDebugLingoExec, "Stack size before call: %d, nargs: %d", _stack.size(), nargs);
	LC::call(s, nargs);
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
void LB::b_cast(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_cast");

	d.type = REFERENCE;
	d.u.i = 0;

	g_lingo->push(d);
}

void LB::b_field(int nargs) {
	Datum d = g_lingo->pop();

	int id;

	if (!g_director->getCurrentScore()) {
		warning("b_field: Assigning to a field in an empty score");
		d.u.i = 0;
		d.type = INT;
		g_lingo->push(d);
		return;
	}

	if (d.type == STRING) {
		if (g_director->getCurrentScore()->_castsNames.contains(*d.u.s))
			id = g_director->getCurrentScore()->_castsNames[*d.u.s];
		else
			error("b_field: Reference to non-existent field: %s", d.u.s->c_str());
	} else if (d.type == INT || d.type == FLOAT) {
		d.toInt();
		id = d.u.i;
	} else {
		error("b_field: Incorrect reference type: %s", d.type2str());
	}

	d.u.i = id;

	d.type = REFERENCE;

	g_lingo->push(d);
}

void LB::b_script(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_script");

	d.type = REFERENCE;
	d.u.i = 0;

	g_lingo->push(d);
}

void LB::b_window(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_window");

	d.type = REFERENCE;
	d.u.i = 0;

	g_lingo->push(d);
}

void LB::b_numberofchars(int nargs) {
	Datum d = g_lingo->pop();
	d.toString();

	int len = strlen(d.u.s->c_str());
	delete d.u.s;

	d.u.i = len;
	d.type = INT;
	g_lingo->push(d);
}

void LB::b_numberofitems(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();
	int numberofitems = 1;
	Common::String contents = *d.u.s;
	for (uint32 i = 0;  i < d.u.s->size(); i++) {
		if (contents[i] == ',')
			numberofitems++;
	}
	delete d.u.s;

	d.u.i = numberofitems;
	d.type = INT;

	g_lingo->push(d);
}

void LB::b_numberoflines(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();
	int numberoflines = 1;
	Common::String contents = *d.u.s;
	for (uint32 i = 0; i < d.u.s->size(); i++) {
		if (contents[i] == '\n')
			numberoflines++;
	}
	delete d.u.s;

	d.u.i = numberoflines;
	d.type = INT;

	g_lingo->push(d);
}

void LB::b_numberofwords(int nargs) {
	Datum d = g_lingo->pop();

	d.toString();
	int numberofwords = 0;
	Common::String contents = *d.u.s;
	for (uint32 i = 1; i < d.u.s->size(); i++) {
		if (Common::isSpace(contents[i]) && !Common::isSpace(contents[i - 1]))
			numberofwords++;
	}
	// Count the last word
	if (!Common::isSpace(contents[d.u.s->size() - 1]))
		numberofwords++;

	d.u.i = numberofwords;
	d.type = INT;

	g_lingo->push(d);
}

void LB::b_lastcharof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastcharof");
	d.toInt();
	d.u.i = 0;

	g_lingo->push(d);
}

void LB::b_lastitemof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastitemof");
	d.toInt();
	d.u.i = 0;

	g_lingo->push(d);
}

void LB::b_lastlineof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastlineof");
	d.toInt();
	d.u.i = 0;

	g_lingo->push(d);
}

void LB::b_lastwordof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastwordof");
	d.toInt();
	d.u.i = 0;

	g_lingo->push(d);
}

} // End of namespace Director
