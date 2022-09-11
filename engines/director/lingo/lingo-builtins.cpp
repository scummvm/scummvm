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

#include "common/system.h"
#include "common/tokenizer.h"
#include "common/translation.h"

#include "gui/message.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macmenu.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/window.h"
#include "director/stxt.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"

#include "image/pict.h"

namespace Director {

static BuiltinProto builtins[] = {
	// Math
	{ "abs",			LB::b_abs,			1, 1, 200, FBLTIN },	// D2 function
	{ "atan",			LB::b_atan,			1, 1, 400, FBLTIN },	//			D4 f
	{ "cos",			LB::b_cos,			1, 1, 400, FBLTIN },	//			D4 f
	{ "exp",			LB::b_exp,			1, 1, 400, FBLTIN },	//			D4 f
	{ "float",			LB::b_float,		1, 1, 400, FBLTIN },	//			D4 f
	{ "integer",		LB::b_integer,		1, 1, 300, FBLTIN },	//		D3 f
	{ "log",			LB::b_log,			1, 1, 400, FBLTIN },	//			D4 f
	{ "pi",				LB::b_pi,			0, 0, 400, FBLTIN },	//			D4 f
	{ "power",			LB::b_power,		2, 2, 400, FBLTIN },	//			D4 f
	{ "random",			LB::b_random,		1, 1, 200, FBLTIN },	// D2 f
	{ "sin",			LB::b_sin,			1, 1, 400, FBLTIN },	//			D4 f
	{ "sqrt",			LB::b_sqrt,			1, 1, 200, FBLTIN },	// D2 f
	{ "tan",			LB::b_tan,			1, 1, 400, FBLTIN },	//			D4 f
	// String
	{ "chars",			LB::b_chars,		3, 3, 200, FBLTIN },	// D2 f
	{ "charToNum",		LB::b_charToNum,	1, 1, 200, FBLTIN },	// D2 f
	{ "length",			LB::b_length,		1, 1, 200, FBLTIN },	// D2 f
	{ "numToChar",		LB::b_numToChar,	1, 1, 200, FBLTIN },	// D2 f
	{ "offset",			LB::b_offset,		2, 3, 200, FBLTIN },	// D2 f
	{ "string",			LB::b_string,		1, 1, 200, FBLTIN },	// D2 f
	{ "value",		 	LB::b_value,		1, 1, 200, FBLTIN },	// D2 f
	// Lists
	{ "add",			LB::b_add,			2, 2, 400, HBLTIN },	//			D4 handler
	{ "addAt",			LB::b_addAt,		3, 3, 400, HBLTIN },	//			D4 h
	{ "addProp",		LB::b_addProp,		3, 3, 400, HBLTIN },	//			D4 h
	{ "append",			LB::b_append,		2, 2, 400, HBLTIN },	//			D4 h
	{ "count",			LB::b_count,		1, 1, 400, FBLTIN },	//			D4 f
	{ "deleteAt",		LB::b_deleteAt,		2, 2, 400, HBLTIN },	//			D4 h
	{ "deleteProp",		LB::b_deleteProp,	2, 2, 400, HBLTIN },	//			D4 h
	{ "findPos",		LB::b_findPos,		2, 2, 400, FBLTIN },	//			D4 f
	{ "findPosNear",	LB::b_findPosNear,	2, 2, 400, FBLTIN },	//			D4 f
	{ "getaProp",		LB::b_getaProp,		2, 2, 400, FBLTIN },	//			D4 f
	{ "getAt",			LB::b_getAt,		2, 2, 400, FBLTIN },	//			D4 f
	{ "getLast",		LB::b_getLast,		1, 1, 400, FBLTIN },	//			D4 f
	{ "getOne",			LB::b_getOne,		2, 2, 400, FBLTIN },	//			D4 f
	{ "getPos",			LB::b_getPos,		2, 2, 400, FBLTIN },	//			D4 f
	{ "getProp",		LB::b_getProp,		2, 2, 400, FBLTIN },	//			D4 f
	{ "getPropAt",		LB::b_getPropAt,	2, 2, 400, FBLTIN },	//			D4 f
	{ "list",			LB::b_list,			-1, 0, 400, FBLTIN },	//			D4 f
	{ "listP",			LB::b_listP,		1, 1, 400, FBLTIN },	//			D4 f
	{ "max",			LB::b_max,			-1,0, 400, FBLTIN },	//			D4 f
	{ "min",			LB::b_min,			-1,0, 400, FBLTIN },	//			D4 f
	{ "setaProp",		LB::b_setaProp,		3, 3, 400, HBLTIN },	//			D4 h
	{ "setAt",			LB::b_setAt,		3, 3, 400, HBLTIN },	//			D4 h
	{ "setProp",		LB::b_setProp,		3, 3, 400, HBLTIN },	//			D4 h
	{ "sort",			LB::b_sort,			1, 1, 400, HBLTIN },	//			D4 h
	// Files
	{ "closeDA",	 	LB::b_closeDA, 		0, 0, 200, CBLTIN },	// D2 c
	{ "closeResFile",	LB::b_closeResFile,	0, 1, 200, CBLTIN },	// D2 c
	{ "closeXlib",		LB::b_closeXlib,	0, 1, 200, CBLTIN },	// D2 c
	{ "getNthFileNameInFolder",LB::b_getNthFileNameInFolder,2,2,400,FBLTIN }, //D4 f
	{ "open",			LB::b_open,			1, 2, 200, CBLTIN },	// D2 c
	{ "openDA",	 		LB::b_openDA, 		1, 1, 200, CBLTIN },	// D2 c
	{ "openResFile",	LB::b_openResFile,	1, 1, 200, CBLTIN },	// D2 c
	{ "openXlib",		LB::b_openXlib,		1, 1, 200, CBLTIN },	// D2 c
	{ "saveMovie",		LB::b_saveMovie,	0, 1, 400, CBLTIN },	//			D4 c
	{ "setCallBack",	LB::b_setCallBack,	2, 2, 200, CBLTIN },	// D2 c
	{ "showResFile",	LB::b_showResFile,	0, 1, 200, CBLTIN },	// D2 c
	{ "showXlib",		LB::b_showXlib,		0, 1, 200, CBLTIN },	// D2 c
	{ "xFactoryList",	LB::b_xFactoryList,	1, 1, 300, FBLTIN },	//		D3 f
	// Control
	{ "abort",			LB::b_abort,		0, 0, 400, CBLTIN },	//			D4 c
	{ "continue",		LB::b_continue,		0, 0, 200, CBLTIN },	// D2 c
	{ "dontPassEvent",	LB::b_dontPassEvent,0, 0, 200, CBLTIN },	// D2 c
	{ "delay",	 		LB::b_delay,		1, 1, 200, CBLTIN },	// D2 c
	{ "do",		 		LB::b_do,			1, 1, 200, CBLTIN },	// D2 c
	{ "go",		 		LB::b_go,			1, 2, 200, CBLTIN },	// D2 c
	{ "halt",	 		LB::b_halt,			0, 0, 400, CBLTIN },	//			D4 c
	{ "nothing",		LB::b_nothing,		0, 0, 200, CBLTIN },	// D2 c
	{ "pass",			LB::b_pass,			0, 0, 400, CBLTIN },	//			D4 c
	{ "pause",			LB::b_pause,		0, 0, 200, CBLTIN },	// D2 c
	{ "play",			LB::b_play,			0, 2, 200, CBLTIN },	// D2 c
	{ "playAccel",		LB::b_playAccel,	-1,0, 200, CBLTIN },	// D2
		// play done													// D2
	{ "preLoad",		LB::b_preLoad,		-1,0, 300, CBLTIN },	//		D3.1 c
	{ "preLoadCast",	LB::b_preLoadCast,	-1,0, 300, CBLTIN },	//		D3.1 c
	{ "quit",			LB::b_quit,			0, 0, 200, CBLTIN },	// D2 c
	{ "restart",		LB::b_restart,		0, 0, 200, CBLTIN },	// D2 c
	{ "return",			LB::b_return,		0, 1, 200, CBLTIN },	// D2 f
	{ "shutDown",		LB::b_shutDown,		0, 0, 200, CBLTIN },	// D2 c
	{ "startTimer",		LB::b_startTimer,	0, 0, 200, CBLTIN },	// D2 c
		// when keyDown													// D2
		// when mouseDown												// D2
		// when mouseUp													// D2
		// when timeOut													// D2
	// Types
	{ "factory",		LB::b_factory,		1, 1, 300, FBLTIN },	//		D3
	{ "floatP",			LB::b_floatP,		1, 1, 300, FBLTIN },	//		D3
	{ "ilk",	 		LB::b_ilk,			1, 2, 400, FBLTIN },	//			D4 f
	{ "integerp",		LB::b_integerp,		1, 1, 200, FBLTIN },	// D2 f
	{ "objectp",		LB::b_objectp,		1, 1, 200, FBLTIN },	// D2 f
	{ "pictureP",		LB::b_pictureP,		1, 1, 400, FBLTIN },	//			D4 f
	{ "stringp",		LB::b_stringp,		1, 1, 200, FBLTIN },	// D2 f
	{ "symbolp",		LB::b_symbolp,		1, 1, 200, FBLTIN },	// D2 f
	{ "voidP",			LB::b_voidP,		1, 1, 400, FBLTIN },	//			D4 f
	// Misc
	{ "alert",	 		LB::b_alert,		1, 1, 200, CBLTIN },	// D2 c
	{ "clearGlobals",	LB::b_clearGlobals,	0, 0, 300, CBLTIN },	//		D3.1 c
	{ "cursor",	 		LB::b_cursor,		1, 1, 200, CBLTIN },	// D2 c
	{ "framesToHMS",	LB::b_framesToHMS,	4, 4, 300, FBLTIN },	//		D3 f
	{ "HMStoFrames",	LB::b_HMStoFrames,	4, 4, 300, FBLTIN },	//		D3 f
	{ "param",	 		LB::b_param,		1, 1, 400, FBLTIN },	//			D4 f
	{ "printFrom",	 	LB::b_printFrom,	-1,0, 200, CBLTIN },	// D2 c
	{ "put",			LB::b_put,			-1,0, 200, CBLTIN },	// D2
		// set															// D2
	{ "showGlobals",	LB::b_showGlobals,	0, 0, 200, CBLTIN },	// D2 c
	{ "showLocals",		LB::b_showLocals,	0, 0, 200, CBLTIN },	// D2 c
	// Score
	{ "constrainH",		LB::b_constrainH,	2, 2, 200, FBLTIN },	// D2 f
	{ "constrainV",		LB::b_constrainV,	2, 2, 200, FBLTIN },	// D2 f
	{ "copyToClipBoard",LB::b_copyToClipBoard,1,1, 400, CBLTIN }, //			D4 c
	{ "duplicate",		LB::b_duplicate,	1, 2, 400, CBLTIN },	//			D4 c
	{ "editableText",	LB::b_editableText,	0, 0, 200, CBLTIN },	// D2, FIXME: the field in D4+
	{ "erase",			LB::b_erase,		1, 1, 400, CBLTIN },	//			D4 c
	{ "findEmpty",		LB::b_findEmpty,	1, 1, 400, FBLTIN },	//			D4 f
		// go															// D2
	{ "importFileInto",	LB::b_importFileInto,2, 2, 400, CBLTIN }, //			D4 c
	{ "installMenu",	LB::b_installMenu,	1, 1, 200, CBLTIN },	// D2 c
	{ "label",			LB::b_label,		1, 1, 200, FBLTIN },	// D2 f
	{ "marker",			LB::b_marker,		1, 1, 200, FBLTIN },	// D2 f
	{ "move",			LB::b_move,			1, 2, 400, CBLTIN },	//			D4 c
	{ "moveableSprite",	LB::b_moveableSprite,0, 0, 200, CBLTIN },// D2, FIXME: the field in D4+
	{ "pasteClipBoardInto",LB::b_pasteClipBoardInto,1,1,400,CBLTIN },//		D4 c
	{ "puppetPalette",	LB::b_puppetPalette, -1,0, 200, CBLTIN },// D2 c
	{ "puppetSound",	LB::b_puppetSound,	-1,0, 200, CBLTIN },	// D2 c
	{ "puppetSprite",	LB::b_puppetSprite,	-1,0, 200, CBLTIN },	// D2 c
	{ "puppetTempo",	LB::b_puppetTempo,	1, 1, 200, CBLTIN },	// D2 c
	{ "puppetTransition",LB::b_puppetTransition,-1,0,200, CBLTIN },// D2 c
	{ "ramNeeded",		LB::b_ramNeeded,	2, 2, 300, FBLTIN },	//		D3.1 f
	{ "rollOver",		LB::b_rollOver,		1, 1, 200, FBLTIN },	// D2 f
	{ "spriteBox",		LB::b_spriteBox,	5, 5, 200, CBLTIN },	// D2 c
	{ "unLoad",			LB::b_unLoad,		0, 2, 300, CBLTIN },	//		D3.1 c
	{ "unLoadCast",		LB::b_unLoadCast,	0, 2, 300, CBLTIN },	//		D3.1 c
	{ "updateStage",	LB::b_updateStage,	0, 0, 200, CBLTIN },	// D2 c
	{ "zoomBox",		LB::b_zoomBox,		-1,0, 200, CBLTIN },	// D2 c
	{"immediateSprite", LB::b_immediateSprite, -1, 0, 200, CBLTIN}, // D2 c
	// Point
	{ "point",			LB::b_point,		2, 2, 400, FBLTIN },	//			D4 f
	{ "inside",			LB::b_inside,		2, 2, 400, FBLTIN },	//			D4 f
	{ "intersect",		LB::b_intersect,	2, 2, 400, FBLTIN },	//			D4 f
	{ "map",			LB::b_map,			3, 3, 400, FBLTIN },	//			D4 f
	{ "rect",			LB::b_rect,			2, 4, 400, FBLTIN },	//			D4 f
	{ "union",			LB::b_union,		2, 2, 400, FBLTIN },	//			D4 f
	// Sound
	{ "beep",	 		LB::b_beep,			0, 1, 200, CBLTIN },	// D2
	{ "mci",	 		LB::b_mci,			1, 1, 300, CBLTIN },	//		D3.1 c
	{ "mciwait",		LB::b_mciwait,		1, 1, 400, CBLTIN },	//			D4 c
	{ "sound",			LB::b_sound,		2, 3, 300, CBLTIN },	//		D3 c
	{ "soundBusy",		LB::b_soundBusy,	1, 1, 300, FBLTIN },	//		D3 f
	// Constants
	{ "backspace",		LB::b_backspace,	0, 0, 200, KBLTIN },	// D2 konstant
	{ "empty",			LB::b_empty,		0, 0, 200, KBLTIN },	// D2 k
	{ "enter",			LB::b_enter,		0, 0, 200, KBLTIN },	// D2 k
	{ "false",			LB::b_false,		0, 0, 200, KBLTIN },	// D2 k
	{ "quote",			LB::b_quote,		0, 0, 200, KBLTIN },	// D2 k
	{ "return",			LB::b_returnconst,	0, 0, 200, KBLTIN },	// D2 k
	{ "tab",			LB::b_tab,			0, 0, 200, KBLTIN },	// D2 k
	{ "true",			LB::b_true,			0, 0, 200, KBLTIN },	// D2 k
	{ "version",		LB::b_version,		0, 0, 300, KBLTIN },	//		D3 k
	// References
	{ "cast",			LB::b_cast,			1, 1, 400, FBLTIN },	//			D4 f
	{ "script",			LB::b_script,		1, 1, 400, FBLTIN },	//			D4 f
	{ "window",			LB::b_window,		1, 1, 400, FBLTIN },	//			D4 f
	// Chunk operations
	{ "numberOfChars",	LB::b_numberofchars,1, 1, 300, FBLTIN },	//			D3 f
	{ "numberOfItems",	LB::b_numberofitems,1, 1, 300, FBLTIN },	//			D3 f
	{ "numberOfLines",	LB::b_numberoflines,1, 1, 300, FBLTIN },	//			D3 f
	{ "numberOfWords",	LB::b_numberofwords,1, 1, 300, FBLTIN },	//			D3 f

	// ScummVM Asserts: Used for testing ScummVM's Lingo implementation
	{ "scummvmAssert",	LB::b_scummvmassert,1, 2, 200, HBLTIN },
	{ "scummvmAssertEqual",	LB::b_scummvmassertequal,2,3,200,HBLTIN },

	// XCMD/XFCN (HyperCard), normally exposed
	{ "GetVolumes", LB::b_getVolumes, 0, 0, 400, FBLTIN },

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void Lingo::initBuiltIns() {
	initBuiltIns(builtins);
}

void Lingo::initBuiltIns(BuiltinProto protos[]) {
	for (BuiltinProto *blt = protos; blt->name; blt++) {
		if (blt->version > _vm->getVersion())
			continue;

		Symbol sym;

		sym.name = new Common::String(blt->name);
		sym.type = blt->type;
		sym.nargs = blt->minArgs;
		sym.maxArgs = blt->maxArgs;

		sym.u.bltin = blt->func;

		switch (blt->type) {
		case CBLTIN:
			_builtinCmds[blt->name] = sym;
			break;
		case FBLTIN:
			_builtinFuncs[blt->name] = sym;
			break;
		case HBLTIN:
			_builtinCmds[blt->name] = sym;
			_builtinFuncs[blt->name] = sym;
			break;
		case KBLTIN:
			_builtinConsts[blt->name] = sym;
		default:
			break;
		}
	}
}

void Lingo::cleanupBuiltIns() {
	_builtinCmds.clear();
	_builtinFuncs.clear();
	_builtinConsts.clear();
}

void Lingo::cleanupBuiltIns(BuiltinProto protos[]) {
	for (BuiltinProto *blt = protos; blt->name; blt++) {
		switch (blt->type) {
		case CBLTIN:
			_builtinCmds.erase(blt->name);
			break;
		case FBLTIN:
			_builtinFuncs.erase(blt->name);
			break;
		case HBLTIN:
			_builtinCmds.erase(blt->name);
			_builtinFuncs.erase(blt->name);
			break;
		case KBLTIN:
			_builtinConsts.erase(blt->name);
		default:
			break;
		}
	}
}

void Lingo::printSTUBWithArglist(const char *funcname, int nargs, const char *prefix) {
	Common::String s(funcname);

	s += '(';

	for (int i = 0; i < nargs; i++) {
		Datum d = _stack[_stack.size() - nargs + i];

		s += d.asString(true);

		if (i != nargs - 1)
			s += ", ";
	}

	s += ")";

	debug(5, "%s %s", prefix, s.c_str());
}

void Lingo::convertVOIDtoString(int arg, int nargs) {
	if (_stack[_stack.size() - nargs + arg].type == VOID) {
		if (_stack[_stack.size() - nargs + arg].u.s != nullptr)
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
	Datum res(atan(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_cos(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(cos(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_exp(int nargs) {
	Datum d = g_lingo->pop();
	// Lingo uses int, so we're enforcing it
	Datum res((double)exp((double)d.asInt()));
	g_lingo->push(res);
}

void LB::b_float(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(d.asFloat());
	g_lingo->push(res);
}

void LB::b_integer(int nargs) {
	Datum d = g_lingo->pop();
	Datum res;

	if (d.type == FLOAT) {
		if (g_director->getVersion() < 500) {	// Note that D4 behaves differently from asInt()
			res = (int)(d.u.f + 0.5);		// Yes, +0.5 even for negative numbers
		} else {
			res = (int)round(d.u.f);
		}
	} else {
		res = d.asInt();
	}

	g_lingo->push(res);
}

void LB::b_log(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(log(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_pi(int nargs) {
	Datum res((double)M_PI);
	g_lingo->push(res);
}

void LB::b_power(int nargs) {
	Datum d1 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum res(pow(d2.asFloat(), d1.asFloat()));
	g_lingo->push(d1);
}

void LB::b_random(int nargs) {
	Datum max = g_lingo->pop();
	Datum res((int)(g_director->_rnd.getRandom(max.asInt()) + 1));
	g_lingo->push(res);
}

void LB::b_sin(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(sin(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_sqrt(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(sqrt(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_tan(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(tan(d.asFloat()));
	g_lingo->push(res);
}

///////////////////
// String
///////////////////
void LB::b_chars(int nargs) {
	Datum d3 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum s = g_lingo->pop();
	TYPECHECK(s, STRING);

	if (g_director->getVersion() < 400 && (d2.type == FLOAT || d3.type == FLOAT)) {
		warning("LB::b_chars: Called with a float in Director 2 and 3 mode. chars' can't handle floats");
		g_lingo->push(Datum(0));
		return;
	}

	int to = d3.asInt();
	int from = d2.asInt();

	Common::U32String src = s.asString().decode(Common::kUtf8);

	int len = src.size();
	int f = MAX(0, MIN(len, from - 1));
	int t = MAX(0, MIN(len, to));

	Common::String res;
	if (f > t) {
		res = Common::String("");
	} else {
		res = src.substr(f, t - f).encode(Common::kUtf8);
	}

	g_lingo->push(res);
}

void LB::b_charToNum(int nargs) {
	Datum d = g_lingo->pop();

	TYPECHECK(d, STRING);

	Common::U32String str = d.asString().decode(Common::kUtf8);
	if (str.size() == 0) {
		g_lingo->push(0);
		return;
	}

	g_lingo->push(charToNum(str[0]));
}

void LB::b_length(int nargs) {
	Datum d = g_lingo->pop();
	TYPECHECK(d, STRING);

	Common::U32String src = d.asString().decode(Common::kUtf8);
	int res = src.size();
	g_lingo->push(res);
}

void LB::b_numToChar(int nargs) {
	int num = g_lingo->pop().asInt();
	g_lingo->push(Common::U32String(numToChar(num)).encode(Common::kUtf8));
}

void LB::b_offset(int nargs) {
	if (nargs == 3) {
		b_offsetRect(nargs);
		return;
	}
	Common::String source = g_lingo->pop().asString();
	Common::String target = g_lingo->pop().asString();

	const char *str = strstr(source.c_str(), target.c_str());

	if (str == nullptr)
		g_lingo->push(Datum(0));
	else
		g_lingo->push(Datum(int(str - source.c_str() + 1)));
}

void LB::b_string(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(d.asString());
	g_lingo->push(res);
}

void LB::b_value(int nargs) {
	Datum d = g_lingo->pop();
	if (d.type != STRING) {
		g_lingo->push(d);
		return;
	}

	Common::String expr = d.asString();
	if (expr.empty()) {
		g_lingo->push(Datum(0));
		return;
	}
	Common::String code = "return " + expr;
	// Compile the code to an anonymous function and call it
	ScriptContext *sc = g_lingo->_compiler->compileAnonymous(code);
	Symbol sym = sc->_eventHandlers[kEventGeneric];
	LC::call(sym, 0, true);
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
	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK2(indexD, INT, FLOAT);
	int index = indexD.asInt();
	TYPECHECK(list, ARRAY);

	int size = list.u.farr->arr.size();
	if (index > size) {
		for (int i = 0; i < index - size - 1; i++)
			list.u.farr->arr.push_back(Datum(0));
	}
	list.u.farr->arr.insert_at(index - 1, value);
}

void LB::b_addProp(int nargs) {
	Datum value = g_lingo->pop();
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK(list, PARRAY);

	PCell cell = PCell(prop, value);
	list.u.parr->arr.push_back(cell);

	if (list.u.parr->_sorted) {
		if (list.u.parr->arr.empty())
			list.u.parr->arr.push_back(cell);
		else {
			uint pos = list.u.parr->arr.size();
			for (uint i = 0; i < list.u.parr->arr.size(); i++) {
				if (list.u.parr->arr[i].p.asString() > cell.p.asString()) {
					pos = i;
					break;
				}
			}
			list.u.parr->arr.insert_at(pos, cell);
		}
	} else {
		list.u.parr->arr.push_back(cell);
	}
}

void LB::b_append(int nargs) {
	Datum value = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK(list, ARRAY);

	if (list.u.farr->_sorted) {
		if (list.u.farr->arr.empty())
			list.u.farr->arr.push_back(value);
		else {
			uint pos = list.u.farr->arr.size();
			for (uint i = 0; i < list.u.farr->arr.size(); i++) {
				if (list.u.farr->arr[i].asInt() > value.asInt()) {
					pos = i;
					break;
				}
			}
			list.u.farr->arr.insert_at(pos, value);
		}
	} else {
		list.u.farr->arr.push_back(value);
	}
}

void LB::b_count(int nargs) {
	Datum list = g_lingo->pop();
	Datum result;
	result.type = INT;

	switch (list.type) {
	case ARRAY:
		result.u.i = list.u.farr->arr.size();
		break;
	case PARRAY:
		result.u.i = list.u.parr->arr.size();
		break;
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}

	g_lingo->push(result);
}

void LB::b_deleteAt(int nargs) {
	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	TYPECHECK2(list, ARRAY, PARRAY);
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
		list.u.farr->arr.remove_at(index - 1);
		break;
	case PARRAY:
		list.u.parr->arr.remove_at(index - 1);
		break;
	default:
		break;
	}
}

void LB::b_deleteProp(int nargs) {
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(list, ARRAY, PARRAY);

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(prop);
		b_deleteAt(nargs);
		break;
	case PARRAY: {
		int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
		if (index > 0) {
			list.u.parr->arr.remove_at(index - 1);
		}
		break;
	}
	default:
		break;
	}
}

void LB::b_findPos(int nargs) {
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	Datum d(0);
	TYPECHECK(list, PARRAY);

	int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
	if (index > 0) {
		d.type = INT;
		d.u.i = index;
	}

	g_lingo->push(d);
}

void LB::b_findPosNear(int nargs) {
	Common::String prop = g_lingo->pop().asString();
	Datum list = g_lingo->pop();
	Datum res(0);
	TYPECHECK(list, PARRAY);

	// FIXME: Integrate with compareTo framework
	prop.toLowercase();

	for (uint i = 0; i < list.u.parr->arr.size(); i++) {
		Datum p = list.u.parr->arr[i].p;
		Common::String tgt = p.asString();
		tgt.toLowercase();
		if (tgt.find(prop.c_str()) == 0) {
			res.u.i = i + 1;
			break;
		}
	}

	g_lingo->push(res);
}

void LB::b_getaProp(int nargs) {
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(prop);
		b_getAt(nargs);
		break;
	case PARRAY: {
		Datum d;
		int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
		if (index > 0) {
			d = list.u.parr->arr[index - 1].v;
		}
		g_lingo->push(d);
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_getAt(int nargs) {
	Datum indexD = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	Datum list = g_lingo->pop();
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
	case RECT:
		ARRBOUNDSCHECK(index, list);
		g_lingo->push(list.u.farr->arr[index - 1]);
		break;
	case PARRAY:
		ARRBOUNDSCHECK(index, list);
		g_lingo->push(list.u.parr->arr[index - 1].v);
		break;
	default:
		TYPECHECK3(list, ARRAY, PARRAY, RECT);
	}
}

void LB::b_getLast(int nargs) {
	Datum list = g_lingo->pop();
	switch (list.type) {
	case ARRAY:
		g_lingo->push(list.u.farr->arr.back());
		break;
	case PARRAY:
		g_lingo->push(list.u.parr->arr.back().v);
		break;
	default:
		TYPECHECK(list, ARRAY);
	}
}

void LB::b_getOne(int nargs) {
	Datum val = g_lingo->pop();
	Datum list = g_lingo->pop();

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(val);
		b_getPos(nargs);
		break;
	case PARRAY: {
		Datum d;
		int index = LC::compareArrays(LC::eqData, list, val, true, true).u.i;
		if (index > 0) {
			d = list.u.parr->arr[index - 1].p;
		}
		g_lingo->push(d);
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_getPos(int nargs) {
	Datum val = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(list, ARRAY, PARRAY);

	switch (list.type) {
	case ARRAY: {
		Datum d(0);
		int index = LC::compareArrays(LC::eqData, list, val, true).u.i;
		if (index > 0) {
			d.u.i = index;
		}
		g_lingo->push(d);
		break;
	}
	case PARRAY: {
		Datum d(0);
		int index = LC::compareArrays(LC::eqData, list, val, true, true).u.i;
		if (index > 0) {
			d.u.i = index;
		}
		g_lingo->push(d);
		break;
	}
	default:
		break;
	}
}

void LB::b_getProp(int nargs) {
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(list, ARRAY, PARRAY);

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(prop);
		b_getPos(nargs);
		break;
	case PARRAY: {
		int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
		if (index > 0) {
			g_lingo->push(list.u.parr->arr[index - 1].v);
		} else {
			error("b_getProp: Property %s not found", prop.asString().c_str());
		}
		break;
	}
	default:
		break;
	}
}

void LB::b_getPropAt(int nargs) {
	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	TYPECHECK(list, PARRAY);
	int index = indexD.asInt();

	g_lingo->push(list.u.parr->arr[index - 1].p);
}

void LB::b_list(int nargs) {
	Datum result;
	result.type = ARRAY;
	result.u.farr = new FArray;

	for (int i = 0; i < nargs; i++)
		result.u.farr->arr.insert_at(0, g_lingo->pop());

	g_lingo->push(result);
}

void LB::b_listP(int nargs) {
	Datum list = g_lingo->pop();
	Datum d(0);
	if (list.type == ARRAY || list.type == PARRAY) {
		d.u.i = 1;
	}
	g_lingo->push(d);
}

void LB::b_max(int nargs) {
	Datum max;
	max.type = INT;
	max.u.i = 0;

	if (nargs == 1) {
		Datum d = g_lingo->pop();
		if (d.type == ARRAY) {
			uint arrsize = d.u.farr->arr.size();
			for (uint i = 0; i < arrsize; i++) {
				Datum item = d.u.farr->arr[i];
				if (i == 0 || item > max) {
					max = item;
				}
			}
		} else {
			max = d;
		}
	} else if (nargs > 0) {
		for (int i = 0; i < nargs; i++) {
			Datum d = g_lingo->_stack[g_lingo->_stack.size() - nargs + i];
			if (d.type == ARRAY) {
				warning("b_max: undefined behavior: array mixed with other args");
			}
			if (i == 0 || d > max) {
				max = d;
			}
		}
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(max);
}

void LB::b_min(int nargs) {
	Datum min;
	min.type = INT;
	min.u.i = 0;

	if (nargs == 1) {
		Datum d = g_lingo->pop();
		if (d.type == ARRAY) {
			uint arrsize = d.u.farr->arr.size();
			for (uint i = 0; i < arrsize; i++) {
				Datum item = d.u.farr->arr[i];
				if (i == 0 || item < min) {
					min = item;
				}
			}
		} else {
			min = d;
		}
	} else if (nargs > 0) {
		for (int i = 0; i < nargs; i++) {
			Datum d = g_lingo->_stack[g_lingo->_stack.size() - nargs + i];
			if (d.type == ARRAY) {
				warning("b_min: undefined behavior: array mixed with other args");
			}
			if (i == 0 || d < min) {
				min = d;
			}
		}
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(min);
}

void LB::b_setaProp(int nargs) {
	Datum value = g_lingo->pop();
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(prop);
		g_lingo->push(value);
		b_setAt(nargs);
		break;
	case PARRAY: {
		int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
		if (index > 0) {
			list.u.parr->arr[index - 1].v = value;
		} else {
			PCell cell = PCell(prop, value);
			list.u.parr->arr.push_back(cell);
		}
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_setAt(int nargs) {
	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK2(indexD, INT, FLOAT);
	TYPECHECK3(list, ARRAY, PARRAY, RECT);
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
		if ((uint)index <= list.u.farr->arr.size()) {
			list.u.farr->arr[index - 1] = value;
		} else {
			int inserts = index - list.u.farr->arr.size();
			while (--inserts)
				list.u.farr->arr.push_back(Datum(0));
			list.u.farr->arr.push_back(value);
		}
		break;
	case PARRAY:
		ARRBOUNDSCHECK(index, list);
		list.u.parr->arr[index - 1].v = value;
		break;
	case RECT:
		ARRBOUNDSCHECK(index, list);
		list.u.farr->arr[index-1] = value;
	default:
		break;
	}
}

void LB::b_setProp(int nargs) {
	Datum value = g_lingo->pop();
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK(list, PARRAY);

	int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
	if (index > 0) {
		list.u.parr->arr[index - 1].v = value;
	} else {
		warning("b_setProp: Property not found");
	}
}

static bool sortArrayHelper(const Datum &lhs, const Datum &rhs) {
	return lhs.asInt() < rhs.asInt();
}

static bool sortPArrayHelper(const PCell &lhs, const PCell &rhs) {
	return lhs.p.asString() < rhs.p.asString();
}

void LB::b_sort(int nargs) {
	// in D4 manual, p266. linear list is sorted by values
	// property list is sorted alphabetically by properties
	// once the list is sorted, it maintains it's sort order even when we add new variables using add command
	// see b_append to get more details.
	Datum list = g_lingo->pop();

	if (list.type == ARRAY) {
		Common::sort(list.u.farr->arr.begin(), list.u.farr->arr.end(), sortArrayHelper);
		list.u.farr->_sorted = true;

	} else if (list.type == PARRAY) {
		Common::sort(list.u.parr->arr.begin(), list.u.parr->arr.end(), sortPArrayHelper);
		list.u.parr->_sorted = true;

	} else {
		warning("LB::b_sort can not handle argument of type %s", list.type2str());
	}
}


///////////////////
// Files
///////////////////
void LB::b_closeDA(int nargs) {
	warning("BUILDBOT: closeDA is not supported in ScummVM");
}

void LB::b_closeResFile(int nargs) {
	if (nargs == 0) { // Close all res files
		g_director->_openResFiles.clear();
		return;
	}
	Datum d = g_lingo->pop();
	Common::String resFileName = g_director->getCurrentWindow()->getCurrentPath() + d.asString();

	g_director->_openResFiles.erase(resFileName);
}

void LB::b_closeXlib(int nargs) {
	if (nargs ==0) { // Close all Xlibs
		g_lingo->closeOpenXLibs();
		return;
	}

	Datum d = g_lingo->pop();
	Common::String xlibName = d.asString();
	g_lingo->closeXLib(xlibName);
}

void LB::b_getNthFileNameInFolder(int nargs) {
	int fileNum = g_lingo->pop().asInt() - 1;
	Common::String path = pathMakeRelative(g_lingo->pop().asString(), true, false, true);
	// for directory, we either return the correct path, which we can access recursively.
	// or we get a wrong path, which will lead us to a non-exist file node

	Common::StringTokenizer directory_list(path, Common::String(g_director->_dirSeparator));
	Common::FSNode d = Common::FSNode(*g_director->getGameDataDir());
	while (d.exists() && !directory_list.empty()) {
		d = d.getChild(directory_list.nextToken());
	}

	Datum r;
	if (d.exists()) {
		Common::FSList f;
		if (!d.getChildren(f, Common::FSNode::kListAll)) {
			warning("Cannot access directory %s", path.c_str());
		} else {
			if ((uint)fileNum < f.size()) {
				// here, we sort all the fileNames
				Common::Array<Common::String> fileNameList;
				for (uint i = 0; i < f.size(); i++)
					fileNameList.push_back(f[i].getName());
				Common::sort(fileNameList.begin(), fileNameList.end());
				r = Datum(fileNameList[fileNum]);
			}
		}
	}

	g_lingo->push(r);
}

void LB::b_open(int nargs) {
	Datum d = g_lingo->pop();
	if (nargs == 2)
		g_lingo->pop();
	warning("LB::b_open(): Unsupported command open encountered -> The movie tried to open %s", d.asString().c_str());

	if (!debugChannelSet(-1, kDebugFewFramesOnly) &&
		!(g_director->getGameGID() == GID_TEST || g_director->getGameGID() == GID_TESTALL)) {
		Common::U32String message = Common::String::format("Unsupported command open encountered -> The movie tried to execute open %s!", d.asString().c_str());
		GUI::MessageDialog dialog(message, _("Ok"));
		dialog.runModal();
	}
}

void LB::b_openDA(int nargs) {
	Datum d = g_lingo->pop();

	warning("BUILDBOT: openDA is not supported in ScummVM");
}

void LB::b_openResFile(int nargs) {
	Datum d = g_lingo->pop();
	Common::String resPath = g_director->getCurrentWindow()->getCurrentPath() + d.asString();

	if (g_director->getPlatform() == Common::kPlatformWindows) {
		warning("STUB: BUILDBOT: b_openResFile(%s) on Windows", d.asString().c_str());
		return;
	}

	if (!g_director->_openResFiles.contains(resPath)) {
		MacArchive *resFile = new MacArchive();

		if (resFile->openFile(pathMakeRelative(resPath))) {
			g_director->_openResFiles.setVal(resPath, resFile);
		}
	}
}

void LB::b_openXlib(int nargs) {
	// TODO: When Xtras are implemented, determine whether to initialize
	// the XObject or Xtra version of FileIO
	Common::String xlibName;

	Datum d = g_lingo->pop();
	if (g_director->getPlatform() == Common::kPlatformMacintosh) {
		// try opening the file as a resfile
		Common::String resPath = g_director->getCurrentWindow()->getCurrentPath() + d.asString();
		if (!g_director->_openResFiles.contains(resPath)) {
			MacArchive *resFile = new MacArchive();

			if (resFile->openFile(pathMakeRelative(resPath))) {
				g_director->_openResFiles.setVal(resPath, resFile);
				uint32 XCOD = MKTAG('X', 'C', 'O', 'D');
				uint32 XCMD = MKTAG('X', 'C', 'M', 'D');

				Common::Array<uint16> rsrcList = resFile->getResourceIDList(XCOD);

				for (uint i = 0; i < rsrcList.size(); i++) {
					xlibName = resFile->getResourceDetail(XCOD, rsrcList[i]).name.c_str();
					g_lingo->openXLib(xlibName, kXObj);
				}

				rsrcList = resFile->getResourceIDList(XCMD);
				for (uint i = 0; i < rsrcList.size(); i++) {
					xlibName = resFile->getResourceDetail(XCMD, rsrcList[i]).name.c_str();
					g_lingo->openXLib(xlibName, kXObj);
				}
				return;
			}
		}
	}

	xlibName = d.asString();
	g_lingo->openXLib(xlibName, kXObj);
}

void LB::b_saveMovie(int nargs) {
	g_lingo->printSTUBWithArglist("b_saveMovie", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_setCallBack(int nargs) {
	for (int i = 0; i < nargs; i++)
		g_lingo->pop();
	warning("STUB: b_setCallBack");
}

void LB::b_showResFile(int nargs) {
	if (nargs)
		g_lingo->pop();
	Common::String out;
	for (auto it = g_director->_openResFiles.begin(); it != g_director->_openResFiles.end(); it++)
		out += it->_key + "\n";
	g_debugger->debugLogFile(out, false);
}

void LB::b_showXlib(int nargs) {
	if (nargs)
		g_lingo->pop();
	Common::String out;
	for (auto it = g_lingo->_openXLibs.begin(); it != g_lingo->_openXLibs.end(); it++)
		out += it->_key + "\n";
	g_debugger->debugLogFile(out, false);
}

void LB::b_xFactoryList(int nargs) {
	Datum d = g_lingo->pop();
	d.type = STRING;
	d.u.s = new Common::String();

	for (auto it = g_lingo->_openXLibs.begin(); it != g_lingo->_openXLibs.end(); it++)
		*d.u.s += it->_key + "\n";
	g_lingo->push(d);
}

///////////////////
// Control
///////////////////
void LB::b_abort(int nargs) {
	g_lingo->_abort = true;
}

void LB::b_continue(int nargs) {
	g_director->_playbackPaused = false;
}

void LB::b_dontPassEvent(int nargs) {
	g_lingo->_passEvent = false;
	warning("dontPassEvent raised");
}

void LB::b_nothing(int nargs) {
	// Noop
}

void LB::b_delay(int nargs) {
	Datum d = g_lingo->pop();

	g_director->getCurrentMovie()->getScore()->_nextFrameTime = g_system->getMillis() + (float)d.asInt() / 60 * 1000;
}

void LB::b_do(int nargs) {
	Common::String code = g_lingo->pop().asString();
	ScriptContext *sc = g_lingo->_compiler->compileAnonymous(code);
	Symbol sym = sc->_eventHandlers[kEventGeneric];

	// Check if we have anything to execute
	if (sym.type == VOIDSYM)
		return;

	LC::call(sym, 0, false);
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

		if (firstArg.type == SYMBOL) {
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
				TYPECHECK(movie, STRING);

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
	g_lingo->_passEvent = true;
	warning("pass raised");
}

void LB::b_pause(int nargs) {
	g_director->_playbackPaused = true;
}

void LB::b_play(int nargs) {
	// Builtin function for play as used by the Director bytecode engine.
	//
	// Accepted arguments:
	// 0  									# "play done"
	// (STRING|INT) frame
	// STRING movie, (STRING|INT) frame

	Datum movie;
	Datum frame;

	switch (nargs) {
	case 2:
		movie = g_lingo->pop();
		// fall through
	case 1:
		frame = g_lingo->pop();
		if (!(frame.type == INT && frame.u.i == 0 && nargs == 1))
			break;
		// fall through
	case 0:
		frame.type = SYMBOL;
		frame.u.s = new Common::String("done");
		break;
	default:
		warning("b_play: expected 0, 1 or 2 args, not %d", nargs);
		g_lingo->dropStack(nargs);

		return;
	}

	g_lingo->func_play(frame, movie);
}

void LB::b_playAccel(int nargs) {
	g_lingo->printSTUBWithArglist("b_playAccel", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_preLoad(int nargs) {
	// We always pretend we preloaded all frames
	// Returning the number of the last frame successfully "loaded"
	if (nargs == 0) {
		g_lingo->_theResult = Datum((int)g_director->getCurrentMovie()->getScore()->_frames.size());
		return;
	}

	g_lingo->_theResult = g_lingo->pop();

	if (nargs == 2)
		g_lingo->pop();
}

void LB::b_preLoadCast(int nargs) {
	// We always pretend we preloaded all cast
	// Returning the number of the last cast successfully "loaded"

	if (nargs > 1) {
		g_lingo->_theResult = g_lingo->pop();
	} else {
		g_lingo->_theResult = 1;
	}

	if (nargs == 2)
		g_lingo->pop();
}

void LB::b_framesToHMS(int nargs) {
	int fractionalSeconds = g_lingo->pop().asInt();
	int dropFrame = g_lingo->pop().asInt();
	int fps = g_lingo->pop().asInt();
	int frames = g_lingo->pop().asInt();

	if (fps <= 0)
		fps = -fps;

	bool negative = frames < 0;
	if (negative)
		frames = -frames;

	int framesPerMin = 60 * fps;
	int framesPerHour = 60 * framesPerMin;

	if (dropFrame)
		warning("STUB: b_framesToHMS: Unhandled dropFrame option");

	int h = MIN(frames / framesPerHour, 99);
	int m = (frames % framesPerHour) / framesPerMin;
	int s = (frames % framesPerMin) / fps;

	int residual;
	if (fractionalSeconds) {
		int ms = (1000 * (frames % fps)) / fps;
		residual = (ms + 5) / 10; // round to nearest centisecond
	} else {
		residual = frames % fps;
	}

	Common::String hms = Common::String::format(
		"%c%02d:%02d:%02d.%02d%c",
		negative ? '-' : ' ',
		h, m, s, residual,
		dropFrame ? 'd' : ' '
	);

	g_lingo->push(hms);
}

void LB::b_HMStoFrames(int nargs) {
	// The original implementation of this accepts some really weird,
	// seemingly malformed input strings.
	// (Try, for example, "12345678901234567890")
	// It's probably not worth supporting them all unless we need to,
	// so only well-formed input is handled right now.

	int fractionalSeconds = g_lingo->pop().asInt();
	int dropFrame = g_lingo->pop().asInt();
	int fps = g_lingo->pop().asInt();
	Common::String hms = g_lingo->pop().asString();

	if (fps <= 0)
		fps = 1;

	const char *ptr = hms.c_str();
	while (Common::isSpace(*ptr))
		ptr++;

	// read sign
	bool negative = false;
	if (*ptr == '-') {
		negative = true;
		ptr++;
	}

	// read HH, MM, and SS
	int secs = 0;
	for (int i = 0; i < 3; i++) {
		if (*ptr == ':' || Common::isSpace(*ptr))
			ptr ++;

		if (!Common::isDigit(*ptr))
			break;

		int part = 0;
		for (int j = 0; j < 2 && Common::isDigit(*ptr); j++) {
			part = (10 * part) + (*ptr - '0');
			ptr++;
		}
		secs = (60 * secs) + part;
	}
	int frames = secs * fps;

	// read FF
	if (*ptr == '.') {
		ptr++;

		int part = 0;
		for (int i = 0; i < 2 && Common::isDigit(*ptr); i++) {
			part = (10 * part) + (*ptr - '0');
			ptr++;
		}
		if (fractionalSeconds) {
			frames += (part * fps + 50) / 100; // round to nearest frame
		} else {
			frames += part;
		}
	}

	// read D
	if (*ptr == 'd' || *ptr == 'D') {
		ptr++;
		dropFrame = 1;
	}

	while (Common::isSpace(*ptr))
		ptr++;

	if (*ptr != '\0')
		warning("b_HMStoFrames: Unexpected character '%c'", *ptr);

	if (dropFrame)
		warning("STUB: b_HMStoFrames: Unhandled dropFrame option");

	if (negative)
		frames = -frames;

	g_lingo->push(frames);
}

void LB::b_param(int nargs) {
	g_lingo->printSTUBWithArglist("b_param", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_printFrom(int nargs) {
	warning("BUILDBOT: printFrom is not supported in ScummVM");

	g_lingo->dropStack(nargs);
}

void LB::b_quit(int nargs) {
	if (g_director->getCurrentMovie())
		g_director->getCurrentMovie()->getScore()->_playState = kPlayStopped;

	g_lingo->pushVoid();
}

void LB::b_return(int nargs) {
	CFrame *fp = g_director->getCurrentWindow()->_callstack.back();

	Datum retVal;
	if (nargs > 0) {
		retVal = g_lingo->pop();
		g_lingo->_theResult = retVal;	// Store result for possible reference
	}

	// clear any temp values from loops
	while (g_lingo->_stack.size() > fp->stackSizeBefore)
		g_lingo->pop();

	// Do not allow a factory's mNew method to return a value
	if (nargs > 0 && !(g_lingo->_currentMe.type == OBJECT && g_lingo->_currentMe.u.obj->getObjType() == kFactoryObj
			&& fp->sp.name->equalsIgnoreCase("mNew"))) {
		g_lingo->push(retVal);
	}

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
	g_director->getCurrentMovie()->_lastTimerReset = g_director->getMacTicks();
}

///////////////////
// Types
///////////////////
void LB::b_factory(int nargs) {
	Datum factoryName = g_lingo->pop();
	factoryName.type = GLOBALREF;
	Datum o = g_lingo->varFetch(factoryName);
	if (o.type == OBJECT && (o.u.obj->getObjType() & (kFactoryObj | kXObj))
			&& o.u.obj->getName().equalsIgnoreCase(*factoryName.u.s) && o.u.obj->getInheritanceLevel() == 1) {
		g_lingo->push(o);
	} else {
		g_lingo->push(Datum(0));
	}
}

void LB::b_floatP(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == FLOAT) ? 1 : 0);
	g_lingo->push(res);
}

void LB::b_ilk(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(Common::String(d.type2str(true)));
	g_lingo->push(res);
}

void LB::b_integerp(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == INT) ? 1 : 0);
	g_lingo->push(res);
}

void LB::b_objectp(int nargs) {
	Datum d = g_lingo->pop();
	Datum res;
	if (d.type == OBJECT) {
		res = !d.u.obj->isDisposed();
	} else {
		res = 0;
	}
	g_lingo->push(res);
}

void LB::b_pictureP(int nargs) {
	g_lingo->pop();
	warning("STUB: b_pictureP");
	g_lingo->push(Datum(0));
}

void LB::b_stringp(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == STRING) ? 1 : 0);
	g_lingo->push(res);
}

void LB::b_symbolp(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == SYMBOL) ? 1 : 0);
	g_lingo->push(res);
}

void LB::b_voidP(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == VOID) ? 1 : 0);
	g_lingo->push(res);
}


///////////////////
// Misc
///////////////////
void LB::b_alert(int nargs) {
	Datum d = g_lingo->pop();

	Common::String alert = d.asString();
	warning("b_alert(%s)", alert.c_str());

	if (g_director->getGameGID() == GID_TEST) {
		warning("b_alert: Skipping due to tests");

		return;
	}

	if (!debugChannelSet(-1, kDebugFewFramesOnly)) {
		g_director->_wm->clearHandlingWidgets();
		GUI::MessageDialog dialog(alert.c_str(), _("OK"));
		dialog.runModal();
	}
}

void LB::b_clearGlobals(int nargs) {
	for (DatumHash::iterator it = g_lingo->_globalvars.begin(); it != g_lingo->_globalvars.end(); it++) {
		if (!it->_value.ignoreGlobal) {
			g_lingo->_globalvars.erase(it);
		}
	}
}

void LB::b_cursor(int nargs) {
	Datum d = g_lingo->pop();
	g_lingo->func_cursor(d);
}

void LB::b_put(int nargs) {
	// Prints a statement to the Message window
	Common::String output;
	for (int i = nargs - 1; i >= 0; i--) {
		output += g_lingo->peek(i).asString(true);
		if (i > 0)
			output += " ";
	}
	if (g_debugger->isActive()) {
		g_debugger->debugLogFile(output, true);
	} else {
		debug("-- %s", output.c_str());
	}
	g_lingo->dropStack(nargs);
}

void LB::b_showGlobals(int nargs) {
	b_version(0);
	Datum ver = g_lingo->pop();
	Common::String global_out = "-- Global Variables --\nversion = ";
	global_out += ver.asString() + "\n";
	if (g_lingo->_globalvars.size()) {
		for (auto it = g_lingo->_globalvars.begin(); it != g_lingo->_globalvars.end(); it++) {
			if (!it->_value.ignoreGlobal) {
				global_out += it->_key + " = " + it->_value.asString() + "\n";
			}
		}
	}
	g_debugger->debugLogFile(global_out, false);
}

void LB::b_showLocals(int nargs) {
	Common::String local_out = "-- Local Variables --\n";
	if (g_lingo->_localvars) {
		for (auto it = g_lingo->_localvars->begin(); it != g_lingo->_localvars->end(); it++) {
			local_out += it->_key + " = " + it->_value.asString() + "\n";
		}
	}
	g_debugger->debugLogFile(local_out, false);
}

///////////////////
// Score
///////////////////
void LB::b_constrainH(int nargs) {
	Datum num = g_lingo->pop();
	Datum sprite = g_lingo->pop();
	Score *score = g_director->getCurrentMovie()->getScore();
	int res = 0;
	if (score) {
		Channel *ch = score->getChannelById(sprite.asInt());
		if (ch) {
			res = CLIP<int> (num.asInt(), ch->getBbox().left, ch->getBbox().right);
		} else {
			warning("b_constrainH: cannot find channel %d", sprite.asInt());
		}
	} else {
		warning("b_constrainH: no score");
	}

	g_lingo->push(Datum(res));
}

void LB::b_constrainV(int nargs) {
	Datum num = g_lingo->pop();
	Datum sprite = g_lingo->pop();

	Score *score = g_director->getCurrentMovie()->getScore();
	int res = 0;
	if (score) {
		Channel *ch = score->getChannelById(sprite.asInt());
		if (ch) {
			res = CLIP<int> (num.asInt(), ch->getBbox().top, ch->getBbox().bottom);
		} else {
			warning("b_constrainH: cannot find channel %d", sprite.asInt());
		}
	} else {
		warning("b_constrainV: no score");
	}

	g_lingo->push(Datum(res));
}

void LB::b_copyToClipBoard(int nargs) {
	Datum d = g_lingo->pop();
	g_director->_clipBoard = new CastMemberID(d.asMemberID());
}

void LB::b_duplicate(int nargs) {
	Datum to = g_lingo->pop();
	Datum from = g_lingo->pop();

	Frame *currentFrame = g_director->getCurrentMovie()->getScore()->_frames[g_director->getCurrentMovie()->getScore()->getCurrentFrame()];
	CastMember *castMember = g_director->getCurrentMovie()->getCastMember(from.asMemberID());
	auto channels = g_director->getCurrentMovie()->getScore()->_channels;

	castMember->setModified(true);
	g_director->getCurrentMovie()->createOrReplaceCastMember(to.asMemberID(), castMember);

	for (uint16 i = 0; i < currentFrame->_sprites.size(); i++) {
		if (currentFrame->_sprites[i]->_castId == to.asMemberID())
			currentFrame->_sprites[i]->setCast(to.asMemberID());
	}

	for (uint i = 0; i < channels.size(); i++) {
		if (channels[i]->_sprite->_castId == to.asMemberID()) {
			channels[i]->_sprite->setCast(to.asMemberID());
			channels[i]->_dirty = true;
		}
	}
}

void LB::b_editableText(int nargs) {
	Score *sc = g_director->getCurrentMovie()->getScore();
	if (!sc) {
		warning("b_editableText: no score");
		g_lingo->dropStack(nargs);
		return;
	}

	if (nargs == 2) {
		Datum state = g_lingo->pop();
		Datum sprite = g_lingo->pop();
		if ((uint)sprite.asInt() < sc->_channels.size()) {
			sc->getSpriteById(sprite.asInt())->_editable = state.asInt();
			sc->getOriginalSpriteById(sprite.asInt())->_editable = state.asInt();
		} else {
			warning("b_editableText: sprite index out of bounds");
		}
	} else if (nargs == 0 && g_director->getVersion() < 400) {
		g_lingo->dropStack(nargs);

		if (g_lingo->_currentChannelId == -1) {
			warning("b_editableText: channel Id is missing");
			return;
		}
		sc->getSpriteById(g_lingo->_currentChannelId)->_editable = true;
		sc->getOriginalSpriteById(g_lingo->_currentChannelId)->_editable = true;
	} else {
		warning("b_editableText: unexpectedly received %d arguments", nargs);
		g_lingo->dropStack(nargs);
	}
}

void LB::b_erase(int nargs) {
	Datum d = g_lingo->pop();
	CastMember *eraseCast = g_director->getCurrentMovie()->getCastMember(d.asMemberID());

	if (eraseCast) {
		eraseCast->_erase = true;
		Common::Array<Channel *> channels = g_director->getCurrentMovie()->getScore()->_channels;

		for (uint i = 0; i < channels.size(); i++) {
			if (channels[i]->_sprite->_castId == d.asMemberID()) {
				channels[i]->_dirty = true;
			}
		}
	}
}

void LB::b_findEmpty(int nargs) {
	Datum d = g_lingo->pop();
	Cast *cast = g_director->getCurrentMovie()->getCast();
	uint16 c_start = cast->_castArrayStart;
	uint16 c_end = cast->_castArrayEnd;

	if (d.type != CASTREF) {
		warning("Incorrect argument type for findEmpty");
		return;
	}

	if (d.u.cast->member > c_end) {
		d.type = INT;
		g_lingo->push(d);
		return;
	}

	if (d.u.cast->member > c_start) {
		c_start = (uint16) d.u.cast->member;
	}

	for (uint16 i = c_start; i <= c_end; i++) {
		if (!(cast->getCastMember(i) && cast->getCastMember(i)->_type != kCastTypeNull)) {
			d.u.i = i;
			d.type = INT;
			g_lingo->push(d);
			return;
		}
	}

	d.type = INT;
	d.u.i = (int) c_end + 1;
	g_lingo->push(d);
}

void LB::b_importFileInto(int nargs) {

	Datum file = g_lingo->pop();
	Datum dst = g_lingo->pop();

	if (!dst.isCastRef()) {
		warning("b_importFileInto(): bad cast ref field type: %s", dst.type2str());
		return;
	}

	CastMemberID memberID = *dst.u.cast;

	if (!(file.asString().matchString("*.pic") || file.asString().matchString("*.pict"))) {
		warning("LB::b_importFileInto : %s is not a valid PICT file", file.asString().c_str());
		return;
	}

	Common::String path = pathMakeRelative(file.asString());
	Common::File in;
	in.open(path);

	if (!in.isOpen()) {
		warning("b_importFileInto(): Cannot open file %s", path.c_str());
		return;
	}

	Image::PICTDecoder *img = new Image::PICTDecoder();
	img->loadStream(in);
	in.close();

	Movie *movie = g_director->getCurrentMovie();
	BitmapCastMember *bitmapCast = new BitmapCastMember(movie->getCast(), memberID.member, img);
	movie->createOrReplaceCastMember(memberID, bitmapCast);
	bitmapCast->setModified(true);
	const Graphics::Surface *surf = img->getSurface();
	bitmapCast->_size = surf->pitch * surf->h + img->getPaletteColorCount() * 3;
	auto channels = g_director->getCurrentMovie()->getScore()->_channels;

	for (uint i = 0; i < channels.size(); i++) {
		if (channels[i]->_sprite->_castId == dst.asMemberID()) {
			channels[i]->setCast(memberID);
			channels[i]->_dirty = true;
		}
	}
}

void menuCommandsCallback(int action, Common::String &text, void *data) {
	g_director->getCurrentMovie()->queueUserEvent(kEventMenuCallback, action);
}

void LB::b_installMenu(int nargs) {
	// installMenu castNum
	Datum d = g_lingo->pop();

	CastMemberID memberID = d.asMemberID();
	if (memberID.member == 0) {
		g_director->_wm->removeMenu();
		return;
	}

	CastMember *member = g_director->getCurrentMovie()->getCastMember(memberID);
	if (!member) {
		g_lingo->lingoError("installMenu: Unknown %s", memberID.asString().c_str());
		return;
	}
	if (member->_type != kCastText) {
		g_lingo->lingoError("installMenu: %s is not a field", memberID.asString().c_str());
		return;
	}
	TextCastMember *field = static_cast<TextCastMember *>(member);

	Common::U32String menuStxt = g_lingo->_compiler->codePreprocessor(field->getText(), field->getCast()->_lingoArchive, kNoneScript, memberID, true);
	int linenum = -1; // We increment it before processing

	Graphics::MacMenu *menu = g_director->_wm->addMenu();
	int submenu = -1;
	Common::String submenuText;
	Common::String command;
	int commandId = 100;

	menu->setCommandsCallback(menuCommandsCallback, g_director);

	debugC(3, kDebugLingoExec, "installMenu: '%s'", Common::toPrintable(menuStxt).encode().c_str());

	LingoArchive *mainArchive = g_director->getCurrentMovie()->getMainLingoArch();

	// Since loading the STXT converts the text to Unicode based on the platform
	// encoding, we need to fetch the correct Unicode character for the platform.

	// STXT sections use Mac-style carriage returns for line breaks.
	// The code preprocessor converts carriage returns to line feeds.
	const uint32 LINE_BREAK = 0x0a;
	// Menu definitions use the character 0xc3 to denote a checkmark.
	// For Mac, this is √. For Windows, this is Ã.
	const uint8 CHECKMARK_CHAR = 0xc3;
	const uint32 CHECKMARK_U32 = numToChar(CHECKMARK_CHAR);
	const char *CHECKMARK_STR = "\xc3\x83"; // "Ã"
	// Menu definitions use the character 0xc5 to denote a code separator.
	// For Mac, this is ≈. For Windows, this is Å.
	const uint8 CODE_SEPARATOR_CHAR = 0xc5;
	const uint32 CODE_SEPARATOR_U32 = numToChar(CODE_SEPARATOR_CHAR);
	const char *CODE_SEPARATOR_STR = "\xc3\x85"; // "Å"

	Common::U32String lineBuffer;

	for (const Common::u32char_type_t *s = menuStxt.c_str(); *s; s++) {
		lineBuffer.clear();
		while (*s && *s != LINE_BREAK) {
			if (*s == CHECKMARK_U32) {
				lineBuffer += CHECKMARK_CHAR;
				s++;
			} else if (*s == CODE_SEPARATOR_U32) {
				lineBuffer += CODE_SEPARATOR_CHAR;
				s++;
			} else if (*s == CONTINUATION) { // fast forward to the next line
				s++;
				if (*s == LINE_BREAK) {
					lineBuffer += ' ';
					s++;
				}
			} else {
				lineBuffer += *s++;
			}
		}
		linenum++;
		Common::String line = lineBuffer.encode();

		if (line.empty())
			continue;

		if (line.hasPrefixIgnoreCase("menu:")) {
			const char *p = &line.c_str()[5];

			while (*p && (*p == ' ' || *p == '\t'))
				p++;

			if (!submenuText.empty()) { // Adding submenu for previous menu
				menu->createSubMenuFromString(submenu, submenuText.c_str(), 0);
			}

			if (!strcmp(p, "@"))
				p = "\xf0";	// Apple symbol

			submenu = menu->addMenuItem(nullptr, Common::String(p));

			submenuText.clear();

			continue;
		}

		// If the line has a UTF8 checkmark, replace it with a byte character
		// as expected by MacMenu.
		size_t checkOffset = line.find(CHECKMARK_STR);
		if (checkOffset != Common::String::npos) {
			line.erase(checkOffset, strlen(CHECKMARK_STR));
			line.insertChar(CHECKMARK_CHAR, checkOffset);
		}

		// Split the line at the code separator
		size_t sepOffset = line.find(CODE_SEPARATOR_STR);

		Common::String text;

		if (sepOffset != Common::String::npos) {
			text = Common::String(line.c_str(), line.c_str() + sepOffset);
			command = Common::String(line.c_str() + sepOffset + strlen(CODE_SEPARATOR_STR));
		} else {
			text = line;
			command = "";
		}

		text.trim();
		command.trim();

		submenuText += text;

		if (!submenuText.empty()) {
			if (!command.empty()) {
				while (mainArchive->getScriptContext(kEventScript, commandId)) {
					commandId++;
				}
				mainArchive->replaceCode(command.decode(Common::kMacRoman), kEventScript, commandId);
				submenuText += Common::String::format("[%d];", commandId);
			} else {
				submenuText += ';';
			}
		}

		if (!*s) // if we reached end of string, do not increment it but break
			break;
	}

	if (!submenuText.empty()) {
		menu->createSubMenuFromString(submenu, submenuText.c_str(), 0);
	}
}

void LB::b_label(int nargs) {
	Datum d = g_lingo->pop();
	uint16 label = g_lingo->func_label(d);

	g_lingo->push(label);
}

void LB::b_marker(int nargs) {
	// marker functions as label when the input is a string
	Datum d = g_lingo->pop();
	int marker;
	if (d.type == STRING) {
		marker = g_lingo->func_label(d);
	} else {
		marker = g_lingo->func_marker(d.asInt());
	}
	g_lingo->push(marker);
}

void LB::b_move(int nargs) {
	Datum src, dest;

	if (nargs == 1) {
		Datum d;
		d.type = CASTREF;
		d.u.cast = new CastMemberID();
		d.u.cast->member = (int) g_director->getCurrentMovie()->getCast()->_castArrayStart;
		g_lingo->push(d);
		b_findEmpty(1);
		dest = g_lingo->pop();
		src = g_lingo->pop();
	} else if (nargs == 2) {
		dest = g_lingo->pop();
		src = g_lingo->pop();
	}

	//Convert dest datum to type CASTREF if it is INT
	//As CastMemberID constructor changes all the values, datum_int is used to preserve int
	if (dest.type == INT) {
		dest.type = CASTREF;
		int datum_int = dest.u.i;
		dest.u.cast = new CastMemberID();
		dest.u.cast->member = datum_int;
	}

	//No need to move if src and dest are same
	if (src.u.cast->member == dest.u.cast->member) {
		return;
	}

	if (!g_director->getCurrentMovie()->getCastMember(*src.u.cast)) {
		warning("b_move: Source CastMember doesn't exist");
		return;
	}

	if (src.u.cast->castLib != 0) {
		warning("b_move: wrong castLib '%d' in src CastMemberID", src.u.cast->castLib);
	}

	g_lingo->push(dest);
	b_erase(1);

	Movie *movie = g_director->getCurrentMovie();
	uint16 frame = movie->getScore()->getCurrentFrame();
	Frame *currentFrame = movie->getScore()->_frames[frame];
	auto channels = g_director->getCurrentMovie()->getScore()->_channels;


	movie->getScore()->renderFrame(frame, kRenderForceUpdate);

	CastMember *toMove = g_director->getCurrentMovie()->getCastMember(src.asMemberID());
	CastMember *toReplace = new CastMember(*toMove);
	toReplace->_type = kCastTypeNull;
	g_director->getCurrentMovie()->createOrReplaceCastMember(dest.asMemberID(), toMove);
	g_director->getCurrentMovie()->createOrReplaceCastMember(src.asMemberID(), toReplace);

	for (uint16 i = 0; i < currentFrame->_sprites.size(); i++) {
		if (currentFrame->_sprites[i]->_castId == dest.asMemberID())
			currentFrame->_sprites[i]->setCast(dest.asMemberID());
	}

	for (uint i = 0; i < channels.size(); i++) {
		if (channels[i]->_sprite->_castId == dest.asMemberID()) {
			channels[i]->_sprite->setCast(CastMemberID(1, 0));
			channels[i]->_dirty = true;
		}
	}

	movie->getScore()->renderFrame(frame, kRenderForceUpdate);
}

void LB::b_moveableSprite(int nargs) {
	Score *sc = g_director->getCurrentMovie()->getScore();
	Frame *frame = sc->_frames[g_director->getCurrentMovie()->getScore()->getCurrentFrame()];

	if (g_lingo->_currentChannelId == -1) {
		warning("b_moveableSprite: channel Id is missing");
		assert(0);
		return;
	}

	// since we are using value copying, in order to make it taking effect immediately. we modify the sprites in channel
	if (sc->_channels[g_lingo->_currentChannelId])
		sc->_channels[g_lingo->_currentChannelId]->_sprite->_moveable = true;
	frame->_sprites[g_lingo->_currentChannelId]->_moveable = true;
}

void LB::b_pasteClipBoardInto(int nargs) {
	Datum to = g_lingo->pop();
	if (!g_director->_clipBoard) {
		warning("LB::b_pasteClipBoardInto(): Nothing to paste from clipboard, skipping paste..");
		return;
	}

	Movie *movie = g_director->getCurrentMovie();
	uint16 frame = movie->getScore()->getCurrentFrame();
	Frame *currentFrame = movie->getScore()->_frames[frame];
	CastMember *castMember = movie->getCastMember(*g_director->_clipBoard);
	auto channels = movie->getScore()->_channels;

	castMember->setModified(true);
	movie->createOrReplaceCastMember(*to.u.cast, castMember);

	for (uint16 i = 0; i < currentFrame->_sprites.size(); i++) {
		if (currentFrame->_sprites[i]->_castId == to.asMemberID())
			currentFrame->_sprites[i]->setCast(to.asMemberID());
	}

	for (uint i = 0; i < channels.size(); i++) {
		if (channels[i]->_sprite->_castId == to.asMemberID()) {
			channels[i]->_sprite->setCast(to.asMemberID());
			channels[i]->_dirty = true;
		}
	}
}

void LB::b_puppetPalette(int nargs) {
	g_lingo->convertVOIDtoString(0, nargs);
	int numFrames = 0, speed = 0, palette = 0;
	Datum d;

	switch (nargs) {
	case 3:
		numFrames = g_lingo->pop().asInt();
		// fall through
	case 2:
		speed = g_lingo->pop().asInt();
		// fall through
	case 1:
		d = g_lingo->pop();

		if (d.type == STRING) {
			// TODO: It seems that there are not strings for Mac and Win system palette
			Common::String palStr = d.asString();
			if (palStr.equalsIgnoreCase("Rainbow")) {
				palette = kClutRainbow;
			} else if (palStr.equalsIgnoreCase("Grayscale")) {
				palette = kClutGrayscale;
			} else if (palStr.equalsIgnoreCase("Pastels")) {
				palette = kClutPastels;
			} else if (palStr.equalsIgnoreCase("Vivid")) {
				palette = kClutVivid;
			} else if (palStr.equalsIgnoreCase("NTSC")) {
				palette = kClutNTSC;
			} else if (palStr.equalsIgnoreCase("Metallic")) {
				palette = kClutMetallic;
			}
		}
		if (!palette) {
			CastMember *member = g_director->getCurrentMovie()->getCastMember(d.asMemberID());

			if (member && member->_type == kCastPalette)
				palette = ((PaletteCastMember *)member)->getPaletteId();
		}
		break;
	default:
		ARGNUMCHECK(1);
		return;
	}

	if (palette) {
		g_director->setPalette(palette);
		g_director->getCurrentMovie()->getScore()->_puppetPalette = true;
	} else {
		// Setting puppetPalette to 0 disables it (Lingo Dictionary, 226)
		Score *sc = g_director->getCurrentMovie()->getScore();
		g_director->getCurrentMovie()->getScore()->_puppetPalette = false;

		// FIXME: set system palette decided by platform, should be fixed after windows palette is working.
		// try to set mac system palette if lastPalette is 0.
		if (sc->_lastPalette == 0)
			g_director->setPalette(-1);
		else
			g_director->setPalette(sc->resolvePaletteId(sc->_lastPalette));
	}

	// TODO: Implement advanced features that use these.
	if (numFrames || speed)
		warning("b_puppetPalette: Skipping extra features");
}

void LB::b_puppetSound(int nargs) {

	if (nargs < 1 || nargs >= 3) {
		warning("b_puppetSound(): needs 1 or 2 args");
		return;
	}

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	Score *score = g_director->getCurrentMovie()->getScore();

	if (!score) {
		warning("b_puppetSound(): no score");
		return;
	}

	// Most variants of puppetSound don't actually play the sound
	// until the playback head moves or updateStage is called.
	// So we'll just queue it to be played later.

	if (nargs == 1) {
		CastMemberID castMember = g_lingo->pop().asMemberID();

		// in D2 manual p206, puppetSound 0 will turn off the puppet status of sound
		sound->setPuppetSound(castMember, 1);
	} else {
		if (g_director->getVersion() < 400) {
			// in D2/3/3.1 interactivity manual, 2 args represent the menu and submenu sounds
			int submenu = g_lingo->pop().asInt();
			int menu = g_lingo->pop().asInt();

			if (menu < kMinSampledMenu || menu > kMaxSampledMenu)
				warning("LB::puppetSound: menu number is not available");

			sound->setPuppetSound(SoundID(kSoundExternal, menu, submenu), 1);
		} else {
			// Two-argument puppetSound is undocumented in D4.
			// It is however documented in the D5 Lingo dictionary.
			CastMemberID castMember = g_lingo->pop().asMemberID();
			int channel = g_lingo->pop().asInt();
			sound->setPuppetSound(castMember, channel);

			// The D4 two-arg variant of puppetSound plays
			// immediately for some inexplicable reason.
			sound->playPuppetSound(channel);
		}
	}
}

void LB::b_immediateSprite(int nargs) {
	Score *sc = g_director->getCurrentMovie()->getScore();
	if (!sc) {
		warning("b_immediateSprite: no score");
		g_lingo->dropStack(nargs);
		return;
	}

	if (nargs == 2) {
		Datum state = g_lingo->pop();
		Datum sprite = g_lingo->pop();

		Sprite *sp = sc->getSpriteById(sprite.asInt());
		if ((uint)sprite.asInt() < sc->_channels.size()) {
			if (sc->getNextFrame() && !sp->_immediate) {
				// same as puppetSprite
				Channel *channel = sc->getChannelById(sprite.asInt());

				channel->replaceSprite(sc->_frames[sc->getNextFrame()]->_sprites[sprite.asInt()]);
				channel->_dirty = true;
			}

			sc->getSpriteById(sprite.asInt())->_immediate = (bool)state.asInt();
		} else {
			warning("b_immediateSprite: sprite index out of bounds");
		}
	} else if (nargs == 0 && g_director->getVersion() < 400) {
		g_lingo->dropStack(nargs);

		if (g_lingo->_currentChannelId == -1) {
			warning("b_immediateSprite: channel Id is missing");
			return;
		}
		sc->getSpriteById(g_lingo->_currentChannelId)->_immediate = true;
	} else {
		warning("b_immediateSprite: unexpectedly received %d arguments", nargs);
		g_lingo->dropStack(nargs);
	}
}

void LB::b_puppetSprite(int nargs) {
	Score *sc = g_director->getCurrentMovie()->getScore();
	if (!sc) {
		warning("b_puppetSprite: no score");
		g_lingo->dropStack(nargs);
		return;
	}

	if (nargs == 2) {
		Datum state = g_lingo->pop();
		Datum sprite = g_lingo->pop();

		Sprite *sp = sc->getSpriteById(sprite.asInt());
		if ((uint)sprite.asInt() < sc->_channels.size()) {
			if (sc->getNextFrame() && !sp->_puppet) {
				// WORKAROUND: If a frame update is queued, update the sprite to the
				// sprite in new frame before setting puppet (Majestic).
				Channel *channel = sc->getChannelById(sprite.asInt());

				channel->replaceSprite(sc->_frames[sc->getNextFrame()]->_sprites[sprite.asInt()]);
				channel->_dirty = true;
			}

			sc->getSpriteById(sprite.asInt())->_puppet = (bool)state.asInt();
		} else {
			warning("b_puppetSprite: sprite index out of bounds");
		}
	} else if (nargs == 0 && g_director->getVersion() < 400) {
		g_lingo->dropStack(nargs);

		if (g_lingo->_currentChannelId == -1) {
			warning("b_puppetSprite: channel Id is missing");
			return;
		}
		sc->getSpriteById(g_lingo->_currentChannelId)->_puppet = true;
	} else {
		warning("b_puppetSprite: unexpectedly received %d arguments", nargs);
		g_lingo->dropStack(nargs);
	}
}

void LB::b_puppetTempo(int nargs) {
	g_director->getCurrentMovie()->getScore()->_puppetTempo = g_lingo->pop().asInt();
}

void LB::b_puppetTransition(int nargs) {
	// puppetTransition whichTransition [, time] [, chunkSize] [, changeArea]
	Window *stage = g_director->getCurrentWindow();
	uint16 duration = 250, area = 1, chunkSize = 1, type = 0;

	switch (nargs) {
	case 4:
		area = g_lingo->pop().asInt();
		// fall through
	case 3:
		chunkSize = g_lingo->pop().asInt();
		// fall through
	case 2:
		duration = g_lingo->pop().asInt();
		// fall through
	case 1:
		type = ((TransitionType)(g_lingo->pop().asInt()));
		break;
	default:
		ARGNUMCHECK(1);
		g_lingo->dropStack(nargs);
		return;
	}

	if (stage->_puppetTransition) {
		warning("b_puppetTransition: Transition already queued");
		return;
	}

	stage->_puppetTransition = new TransParams(duration, area, chunkSize, ((TransitionType)type));
}

void LB::b_ramNeeded(int nargs) {
	Datum d = g_lingo->pop();
	Datum d2 = g_lingo->pop();

	// We do not need RAM, we have it all

	g_lingo->push(Datum(0));
}

void LB::b_rollOver(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(0);
	int arg = d.asInt();

	Score *score = g_director->getCurrentMovie()->getScore();

	if (!score) {
		warning("b_rollOver: Reference to an empty score");
		return;
	}

	if (arg >= (int32) score->_channels.size()) {
		g_lingo->push(res);
		return;
	}

	Common::Point pos = g_director->getCurrentWindow()->getMousePos();

	if (score->checkSpriteIntersection(arg, pos))
		res.u.i = 1; // TRUE

	g_lingo->push(res);
}

void LB::b_spriteBox(int nargs) {
	int b = g_lingo->pop().asInt();
	int r = g_lingo->pop().asInt();
	int t = g_lingo->pop().asInt();
	int l = g_lingo->pop().asInt();
	int spriteId = g_lingo->pop().asInt();
	Channel *channel = g_director->getCurrentMovie()->getScore()->getChannelById(spriteId);

	if (!channel)
		return;

	g_director->getCurrentWindow()->addDirtyRect(channel->getBbox());
	channel->setBbox(l, t, r, b);
	channel->_dirty = true;
}

void LB::b_unLoad(int nargs) {
	// No op for us, we do not unload casts

	g_lingo->dropStack(nargs);
}

void LB::b_unLoadCast(int nargs) {
	// No op for us, we do not unload casts

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
		delayTicks = d.asInt();
	}

	int endSpriteId = g_lingo->pop().asInt();
	int startSpriteId = g_lingo->pop().asInt();

	Score *score = g_director->getCurrentMovie()->getScore();
	uint16 curFrame = score->getCurrentFrame();

	Common::Rect startRect = score->_channels[startSpriteId]->getBbox();
	if (startRect.isEmpty()) {
		warning("b_zoomBox: unknown start sprite #%d", startSpriteId);
		return;
	}

	// Looks for endSprite in the current frame, otherwise
	// Looks for endSprite in the next frame
	Common::Rect endRect = score->_channels[endSpriteId]->getBbox();
	if (endRect.isEmpty()) {
		if ((uint)curFrame + 1 < score->_frames.size()) {
			Channel endChannel(score->_frames[curFrame + 1]->_sprites[endSpriteId]);
			endRect = endChannel.getBbox();
		}
	}

	if (endRect.isEmpty()) {
		if ((uint)curFrame - 1 > 0) {
			Channel endChannel(score->_frames[curFrame - 1]->_sprites[endSpriteId]);
			endRect = endChannel.getBbox();
		}
	}

	if (endRect.isEmpty()) {
		warning("b_zoomBox: unknown end sprite #%d", endSpriteId);
		return;
	}

	Graphics::ZoomBox *box = new Graphics::ZoomBox;
	box->start = startRect;
	box->end = endRect;
	box->delay = delayTicks;
	box->step = 0;
	box->startTime = g_system->getMillis();
	box->nextTime  = g_system->getMillis() + 1000 * box->step / 60;

	g_director->_wm->addZoomBox(box);
}

void LB::b_updateStage(int nargs) {
	if (g_director->getGameGID() == GID_TEST) {
		warning("b_updateStage: Skipping due to tests");

		return;
	}

	Movie *movie = g_director->getCurrentMovie();
	if (!movie) {
		warning("b_updateStage: no movie");

		return;
	}

	Score *score = movie->getScore();

	score->updateWidgets(movie->_videoPlayback);
	movie->getWindow()->render();

	// play any puppet sounds that have been queued
	score->playSoundChannel(score->getCurrentFrame());

	if (score->_cursorDirty) {
		score->renderCursor(movie->getWindow()->getMousePos());
		score->_cursorDirty = false;
	}

	g_director->draw();

	if (debugChannelSet(-1, kDebugFewFramesOnly)) {
		score->_framesRan++;
			warning("LB::b_updateStage(): ran frame %0d", score->_framesRan);

		if (score->_framesRan > 9) {
			warning("b_updateStage(): exiting due to debug few frames only");
			score->_playState = kPlayStopped;
		}
	}
}


///////////////////
// Point
///////////////////
void LB::b_point(int nargs) {
	Datum y(g_lingo->pop().asFloat());
	Datum x(g_lingo->pop().asFloat());
	Datum d;

	d.u.farr = new FArray;

	d.u.farr->arr.push_back(x);
	d.u.farr->arr.push_back(y);
	d.type = POINT;

	g_lingo->push(d);
}

void LB::b_rect(int nargs) {
	Datum d(0);

	if (nargs == 4) {
		Datum bottom(g_lingo->pop().asInt());
		Datum right(g_lingo->pop().asInt());
		Datum top(g_lingo->pop().asInt());
		Datum left(g_lingo->pop().asInt());

		d.u.farr = new FArray;
		d.u.farr->arr.push_back(left);
		d.u.farr->arr.push_back(top);
		d.u.farr->arr.push_back(right);
		d.u.farr->arr.push_back(bottom);
		d.type = RECT;
	} else if (nargs == 2) {
		Datum p2 = g_lingo->pop();
		Datum p1 = g_lingo->pop();

		if (p2.type == POINT && p1.type == POINT) {
			d.u.farr = new FArray;
			d.u.farr->arr.push_back(p1.u.farr->arr[0]);
			d.u.farr->arr.push_back(p1.u.farr->arr[1]);
			d.u.farr->arr.push_back(p2.u.farr->arr[0]);
			d.u.farr->arr.push_back(p2.u.farr->arr[1]);
			d.type = RECT;
		} else
			warning("LB::b_rect: Rect need 2 Point variable as argument");

	} else {
		warning("LB::b_rect: Rect doesn't support %d args", nargs);
		g_lingo->dropStack(nargs);
	}

	g_lingo->push(d);
}


void LB::b_intersect(int nargs) {
	Datum d;
	Datum r2 = g_lingo->pop();
	Datum r1 = g_lingo->pop();
	Common::Rect rect1(r1.u.farr->arr[0].asInt(), r1.u.farr->arr[1].asInt(), r1.u.farr->arr[2].asInt(), r1.u.farr->arr[3].asInt());
	Common::Rect rect2(r2.u.farr->arr[0].asInt(), r2.u.farr->arr[1].asInt(), r2.u.farr->arr[2].asInt(), r2.u.farr->arr[3].asInt());

	d.type = INT;
	d.u.i = rect1.intersects(rect2);

	g_lingo->push(d);
}

void LB::b_inside(int nargs) {
	Datum d;
	Datum r2 = g_lingo->pop();
	Datum p1 = g_lingo->pop();
	Common::Rect rect2(r2.u.farr->arr[0].asInt(), r2.u.farr->arr[1].asInt(), r2.u.farr->arr[2].asInt(), r2.u.farr->arr[3].asInt());
	Common::Point point1(p1.u.farr->arr[0].asInt(), p1.u.farr->arr[1].asInt());

	d.type = INT;
	d.u.i = rect2.contains(point1);

	g_lingo->push(d);
}

void LB::b_map(int nargs) {
	Datum toRect = g_lingo->pop();
	Datum fromRect = g_lingo->pop();
	Datum srcArr = g_lingo->pop();

	if (!(toRect.type == RECT || (toRect.type == ARRAY && toRect.u.farr->arr.size() == 4)) ||
		!(fromRect.type == RECT || (fromRect.type == ARRAY && fromRect.u.farr->arr.size() == 4))) {
		warning("LB::b_map(): Invalid Datum Type of source and destination Rects");
		return;
	}

	int toWidth = toRect.u.farr->arr[2].u.i - toRect.u.farr->arr[0].u.i;
	int toHeight = toRect.u.farr->arr[3].u.i - toRect.u.farr->arr[1].u.i;
	int fromWidth = fromRect.u.farr->arr[2].u.i - fromRect.u.farr->arr[0].u.i;
	int fromHeight = fromRect.u.farr->arr[3].u.i - fromRect.u.farr->arr[1].u.i;

	if (!(srcArr.type == POINT ||
		srcArr.type == RECT ||
		(srcArr.type == ARRAY && (srcArr.u.farr->arr.size() == 2 || srcArr.u.farr->arr.size() == 4)))) {
		warning("LB::b_map(): Invalid Datum type of input Point / Rect");
		return;
	}

	Datum d;
	d.type = POINT;
	d.u.farr = new FArray();
	d.u.farr->arr.push_back((srcArr.u.farr->arr[0].u.i - fromRect.u.farr->arr[0].u.i) * (toWidth / fromWidth) + toRect.u.farr->arr[0].u.i);
	d.u.farr->arr.push_back((srcArr.u.farr->arr[1].u.i - fromRect.u.farr->arr[1].u.i) * (toHeight / fromHeight) + toRect.u.farr->arr[1].u.i);

	if (srcArr.type == RECT ||
		(srcArr.type == ARRAY && srcArr.u.farr->arr.size() == 4)) {
		d.type = RECT;
		d.u.farr->arr.push_back((srcArr.u.farr->arr[2].u.i - srcArr.u.farr->arr[0].u.i) * (toWidth / fromWidth) + d.u.farr->arr[0].u.i);
		d.u.farr->arr.push_back((srcArr.u.farr->arr[3].u.i - srcArr.u.farr->arr[1].u.i) * (toWidth / fromWidth) + d.u.farr->arr[1].u.i);
	}

	g_lingo->push(d);
}

void LB::b_offsetRect(int nargs) {
	Datum vert = g_lingo->pop();
	Datum hori = g_lingo->pop();
	Datum rect = g_lingo->pop();

	if (vert.type != INT ||
		hori.type != INT ||
		!(rect.type == RECT || (rect.type == ARRAY && rect.u.farr->arr.size() >= 4))) {
		warning(" LB::b_offsetRect(): Invalid DatumType of inputs");
		g_lingo->push(rect);
	}

	//When vert is positive, rect moves higher
	//When hori is positive, rect moves towards right

	rect.u.farr->arr[0].u.i += hori.u.i;
	rect.u.farr->arr[2].u.i += hori.u.i;
	rect.u.farr->arr[1].u.i -= vert.u.i;
	rect.u.farr->arr[3].u.i -= vert.u.i;


	g_lingo->push(rect);
}

void LB::b_union(int nargs) {
	if (nargs != 2) {
		warning("Wrong number of arguments for b_union: Expected 2, got %d", nargs);
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}

	Datum rect1 = g_lingo->pop();
	Datum rect2 = g_lingo->pop();

	if (rect1.type != RECT || rect2.type != RECT) {
		warning("Wrong type of arguments for b_union");
		g_lingo->push(Datum(0));
		return;
	}

	Datum res;
	res.type = RECT;

	res.u.farr = new FArray();
	res.u.farr->arr.push_back(Datum(MIN(rect1.u.farr->arr[0].u.i, rect2.u.farr->arr[0].u.i)));
	res.u.farr->arr.push_back(Datum(MIN(rect1.u.farr->arr[1].u.i, rect2.u.farr->arr[1].u.i)));
	res.u.farr->arr.push_back(Datum(MAX(rect1.u.farr->arr[2].u.i, rect2.u.farr->arr[2].u.i)));
	res.u.farr->arr.push_back(Datum(MAX(rect1.u.farr->arr[3].u.i, rect2.u.farr->arr[3].u.i)));

	g_lingo->push(res);
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

	g_lingo->func_mci(d.asString());
}

void LB::b_mciwait(int nargs) {
	Datum d = g_lingo->pop();

	g_lingo->func_mciwait(d.asString());
}

void LB::b_sound(int nargs) {
	// Builtin function for sound as used by the Director bytecode engine.
	//
	// Accepted arguments:
	// "close", INT soundChannel
	// "fadeIn", INT soundChannel(, INT ticks)
	// "fadeOut", INT soundChannel(, INT ticks)
	// "playFile", INT soundChannel, STRING fileName
	// "stop", INT soundChannel

	if (nargs < 2 || nargs > 3) {
		warning("b_sound: expected 2 or 3 args, not %d", nargs);
		g_lingo->dropStack(nargs);

		return;
	}

	int ticks;
	Datum secondArg = g_lingo->pop();
	Datum firstArg = g_lingo->pop();
	Datum verb;
	if (nargs > 2) {
		verb = g_lingo->pop();
	} else {
		verb = firstArg;
		firstArg = secondArg;
	}

	if (verb.type != STRING && verb.type != SYMBOL) {
		warning("b_sound: verb arg should be of type STRING, not %s", verb.type2str());
		return;
	}

	DirectorSound *soundManager = g_director->getCurrentWindow()->getSoundManager();

	if (verb.u.s->equalsIgnoreCase("close") || verb.u.s->equalsIgnoreCase("stop")) {
		if (nargs != 2) {
			warning("sound %s: expected 1 argument, got %d", verb.u.s->c_str(), nargs - 1);
			return;
		}

		TYPECHECK(firstArg, INT);
		soundManager->stopSound(firstArg.u.i);
	} else if (verb.u.s->equalsIgnoreCase("fadeIn")) {
		if (nargs > 2) {
			TYPECHECK(secondArg, INT);
			ticks = secondArg.u.i;
		} else {
			ticks = 15 * (60 / g_director->getCurrentMovie()->getScore()->_currentFrameRate);
		}

		TYPECHECK(firstArg, INT);
		soundManager->registerFade(firstArg.u.i, true, ticks);
		g_director->getCurrentMovie()->getScore()->_activeFade = firstArg.u.i;
		return;
	} else if (verb.u.s->equalsIgnoreCase("fadeOut")) {
		if (nargs > 2) {
			TYPECHECK(secondArg, INT);
			ticks = secondArg.u.i;
		} else {
			ticks = 15 * (60 / g_director->getCurrentMovie()->getScore()->_currentFrameRate);
		}

		TYPECHECK(firstArg, INT);
		soundManager->registerFade(firstArg.u.i, false, ticks);
		g_director->getCurrentMovie()->getScore()->_activeFade = firstArg.u.i;
		return;
	} else if (verb.u.s->equalsIgnoreCase("playFile")) {
		ARGNUMCHECK(3)

		TYPECHECK(firstArg, INT);
		TYPECHECK(secondArg, STRING);

		soundManager->playFile(pathMakeRelative(*secondArg.u.s), firstArg.u.i);
	} else {
		warning("b_sound: unknown verb %s", verb.u.s->c_str());
	}
}

void LB::b_soundBusy(int nargs) {
	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	Datum whichChannel = g_lingo->pop();

	TYPECHECK(whichChannel, INT);

	bool isBusy = sound->isChannelActive(whichChannel.u.i);
	Datum result;
	result.type = INT;
	result.u.i = isBusy ? 1 : 0;
	g_lingo->push(result);
}

///////////////////
// Constants
///////////////////
void LB::b_backspace(int nargs) {
	g_lingo->push(Datum(Common::String("\b")));
}

void LB::b_empty(int nargs) {
	g_lingo->push(Datum(Common::String("")));
}

void LB::b_enter(int nargs) {
	g_lingo->push(Datum(Common::String("\03")));
}

void LB::b_false(int nargs) {
	g_lingo->push(Datum(0));
}

void LB::b_quote(int nargs) {
	g_lingo->push(Datum(Common::String("\"")));
}

void LB::b_returnconst(int nargs) {
	g_lingo->push(Datum(Common::String("\r")));
}

void LB::b_tab(int nargs) {
	g_lingo->push(Datum(Common::String("\t")));
}

void LB::b_true(int nargs) {
	g_lingo->push(Datum(1));
}

void LB::b_version(int nargs) {
	int major = g_director->getVersion() / 100;
	int minor = (g_director->getVersion() / 10) % 10;
	int patch = g_director->getVersion() % 10;
	Common::String res;
	if (patch) {
		res = Common::String::format("%d.%d.%d", major, minor, patch);
	} else {
		res = Common::String::format("%d.%d", major, minor);
	}
	g_lingo->push(res);
}

///////////////////
// References
///////////////////
void LB::b_cast(int nargs) {
	Datum d = g_lingo->pop();
	Datum res = d.asMemberID();
	res.type = CASTREF;
	g_lingo->push(res);
}

void LB::b_script(int nargs) {
	Datum d = g_lingo->pop();
	CastMemberID memberID = d.asMemberID();
	CastMember *cast = g_director->getCurrentMovie()->getCastMember(memberID);

	if (cast) {
		ScriptContext *script = nullptr;

		if (cast->_type == kCastLingoScript) {
			// script cast can be either a movie script or score script
			script = g_director->getCurrentMovie()->getScriptContext(kMovieScript, memberID);
			if (!script)
				script = g_director->getCurrentMovie()->getScriptContext(kScoreScript, memberID);
		} else {
			g_director->getCurrentMovie()->getScriptContext(kCastScript, memberID);
		}

		if (script) {
			g_lingo->push(script);
			return;
		}
	}

	g_lingo->push(Datum());
}

void LB::b_window(int nargs) {
	Datum d = g_lingo->pop();
	Common::String windowName = d.asString();
	FArray *windowList = g_lingo->_windowList.u.farr;

	for (uint i = 0; i < windowList->arr.size(); i++) {
		if (windowList->arr[i].type != OBJECT || windowList->arr[i].u.obj->getObjType() != kWindowObj)
			continue;

		Window *window = static_cast<Window *>(windowList->arr[i].u.obj);
		if (window->getName().equalsIgnoreCase(windowName)) {
			g_lingo->push(window);
			return;
		}
	}

	Graphics::MacWindowManager *wm = g_director->getMacWindowManager();
	Window *window = new Window(wm->getNextId(), false, false, false, wm, g_director, false);
	window->setName(windowName);
	window->setTitle(windowName);
	window->resize(1, 1, true);
	window->setVisible(false, true);
	wm->addWindowInitialized(window);
	windowList->arr.push_back(window);
	g_lingo->push(window);
}

void LB::b_numberofchars(int nargs) {
	Datum d = g_lingo->pop();
	Datum chunkRef = LC::lastChunk(kChunkChar, d);
	g_lingo->push(chunkRef.u.cref->startChunk);
}

void LB::b_numberofitems(int nargs) {
	Datum d = g_lingo->pop();
	Datum chunkRef = LC::lastChunk(kChunkItem, d);
	g_lingo->push(chunkRef.u.cref->startChunk);
}

void LB::b_numberoflines(int nargs) {
	Datum d = g_lingo->pop();
	Datum chunkRef = LC::lastChunk(kChunkLine, d);
	g_lingo->push(chunkRef.u.cref->startChunk);
}

void LB::b_numberofwords(int nargs) {
	Datum d = g_lingo->pop();
	Datum chunkRef = LC::lastChunk(kChunkWord, d);
	g_lingo->push(chunkRef.u.cref->startChunk);
}

void LB::b_scummvmassert(int nargs) {
	Datum line = g_lingo->pop();
	Datum d = g_lingo->pop();

	if (d.asInt() == 0) {
		warning("LB::b_scummvmassert: is false at line %d", line.asInt());
	}
	assert(d.asInt() != 0);
}

void LB::b_scummvmassertequal(int nargs) {
	Datum line = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	int result = (d1 == d2);
	if (!result) {
		warning("LB::b_scummvmassertequals: %s is not equal %s at line %d", d1.asString().c_str(), d2.asString().c_str(), line.asInt());
	}
	assert(result == 1);
}

void LB::b_getVolumes(int nargs) {
	// Right now, only "Journeyman Project 2: Buried in Time" is known to check
	// for its volume name.
	Datum d;
	d.type = ARRAY;
	d.u.farr = new FArray;
	d.u.farr->arr.push_back(Datum("Buried in Time\252 1"));

	g_lingo->push(d);
}

} // End of namespace Director
