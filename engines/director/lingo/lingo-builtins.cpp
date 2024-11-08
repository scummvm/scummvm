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
#include "common/translation.h"

#include "director/types.h"
#include "gui/message.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/debugger.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/stxt.h"
#include "director/util.h"
#include "director/window.h"
#include "director/castmember/castmember.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/palette.h"
#include "director/castmember/text.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
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
	{ "add",			LB::b_add,			2, 2, 400, HBLTIN_LIST },	//			D4 handler
	{ "addAt",			LB::b_addAt,		3, 3, 400, HBLTIN_LIST },	//			D4 h
	{ "addProp",		LB::b_addProp,		3, 3, 400, HBLTIN_LIST },	//			D4 h
	{ "append",			LB::b_append,		2, 2, 400, HBLTIN_LIST },	//			D4 h
	{ "count",			LB::b_count,		1, 1, 400, FBLTIN_LIST },	//			D4 f
	{ "deleteAt",		LB::b_deleteAt,		2, 2, 400, HBLTIN_LIST },	//			D4 h
	{ "deleteOne",		LB::b_deleteOne,	2, 2, 400, HBLTIN_LIST },	//			D4 h, undocumented?
	{ "deleteProp",		LB::b_deleteProp,	2, 2, 400, HBLTIN_LIST },	//			D4 h
	{ "duplicate",		LB::b_duplicateList,1, 1, 500, FBLTIN_LIST },	//				D5 f
	{ "findPos",		LB::b_findPos,		2, 2, 400, FBLTIN_LIST },	//			D4 f
	{ "findPosNear",	LB::b_findPosNear,	2, 2, 400, FBLTIN_LIST },	//			D4 f
	{ "getaProp",		LB::b_getaProp,		2, 2, 400, FBLTIN_LIST },	//			D4 f
	{ "getAt",			LB::b_getAt,		2, 2, 400, FBLTIN_LIST },	//			D4 f
	{ "getLast",		LB::b_getLast,		1, 1, 400, FBLTIN_LIST },	//			D4 f
	{ "getOne",			LB::b_getOne,		2, 2, 400, FBLTIN_LIST },	//			D4 f
	{ "getPos",			LB::b_getPos,		2, 2, 400, FBLTIN_LIST },	//			D4 f
	{ "getProp",		LB::b_getProp,		2, 2, 400, FBLTIN_LIST },	//			D4 f
	{ "getPropAt",		LB::b_getPropAt,	2, 2, 400, FBLTIN_LIST },	//			D4 f
	{ "list",			LB::b_list,			-1, 0, 400, FBLTIN_LIST },	//			D4 f
	{ "listP",			LB::b_listP,		1, 1, 400, FBLTIN_LIST },	//			D4 f
	{ "max",			LB::b_max,			-1,0, 400, FBLTIN_LIST },	//			D4 f
	{ "min",			LB::b_min,			-1,0, 400, FBLTIN_LIST },	//			D4 f
	{ "setaProp",		LB::b_setaProp,		3, 3, 400, HBLTIN_LIST },	//			D4 h
	{ "setAt",			LB::b_setAt,		3, 3, 400, HBLTIN_LIST },	//			D4 h
	{ "setProp",		LB::b_setProp,		3, 3, 400, HBLTIN_LIST },	//			D4 h
	{ "sort",			LB::b_sort,			1, 1, 400, HBLTIN_LIST },	//			D4 h
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
	{ "xtra",			LB::b_xtra,			1, 1, 500, FBLTIN },	//				D5 f
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
	{ "preLoadMember",	LB::b_preLoadCast,	-1,0, 500, CBLTIN },	//				D5 c
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
	{ "editableText",	LB::b_editableText,	0, 0, 200, CBLTIN },	// D2
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
	{ "unLoadMember",	LB::b_unLoadCast,	0, 2, 500, CBLTIN },	//				D5 c
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
	{ "mciwait",		LB::b_mciwait,		1, 1, 300, CBLTIN },	//		D3.1 c
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
	{ "castLib",		LB::b_castLib,		1, 1, 500, FBLTIN },	//				D5 f
	{ "member",			LB::b_member,		1, 2, 500, FBLTIN },	//				D5 f
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
	{ "scummvmNoFatalError", LB::b_scummvmNoFatalError,1,1,200,HBLTIN },

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
		case FBLTIN_LIST:
			_builtinListHandlers[blt->name] = sym; // fall-through
		case FBLTIN:
			_builtinFuncs[blt->name] = sym;
			break;
		case HBLTIN_LIST:
			_builtinListHandlers[blt->name] = sym; // fall-through
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

void Lingo::printArgs(const char *funcname, int nargs, const char *prefix) {
	Common::String s;
	if (prefix)
		s += Common::String(prefix);

	s += Common::String(funcname);
	s += '(';

	for (int i = 0; i < nargs; i++) {
		Datum d = _stack[_stack.size() - nargs + i];

		s += d.asString(true);

		if (i != nargs - 1)
			s += ", ";
	}

	s += ")";

	debug(3, "%s", s.c_str());
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
	Datum res(0);

	if (d.type == INT)
		res = Datum(ABS(d.u.i));
	else if (d.type == FLOAT)
		res = Datum(ABS(d.u.f));

	g_lingo->push(res);
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
	Datum res;

	if (d.type == STRING) {
		Common::String src = d.asString();
		char *endPtr = nullptr;
		double result = strtod(src.c_str(), &endPtr);
		if (*endPtr == 0) {
			res = result;
		} else {
			// for some reason, float(str) will return str if it doesn't work
			res = d;
		}
	} else {
		res = d.asFloat();
	}

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
	} else if (d.type == STRING) {
		Common::String src = d.asString();
		char *endPtr = nullptr;
		int result = (int)strtol(src.c_str(), &endPtr, 10);
		if (endPtr && endPtr != src.c_str() && (*endPtr == '\0' || *endPtr == ' ')) {
			res = result;
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
	g_lingo->push(res);
}

void LB::b_random(int nargs) {
	int max = g_lingo->pop().asInt();
	Datum res;
	// Output in D4/D5 seems to be bounded from 1-65535, regardless of input.
	if (max <= 0) {
		res = Datum((int)(g_director->_rnd.getRandom(65535) + 1));
	} else {
		max = MIN(max, 65535);
		res = Datum((int)(g_director->_rnd.getRandom(max) + 1));
	}
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
	if (d.type == INT || d.type == FLOAT || d.type == VOID) {
		g_lingo->push(0);
		return;
	}
	TYPECHECK(d, STRING);

	Common::U32String src = d.asString().decode(Common::kUtf8);
	int res = src.size();
	g_lingo->push(res);
}

void LB::b_numToChar(int nargs) {
	Datum d = g_lingo->pop();
	if (g_director->getVersion() < 400) {
		TYPECHECK(d, INT);
	} else if (d.type != INT) {
		warning("BUILDBOT: b_numToChar: Unimplemented behaviour for arg of type %s", (d).type2str());
		return;
	}

	int num = d.asInt();
	g_lingo->push(Common::U32String(numToChar(num)).encode(Common::kUtf8));
}

void LB::b_offset(int nargs) {
	if (nargs == 3) {
		b_offsetRect(nargs);
		return;
	}
	Common::String source = g_lingo->pop().asString();
	Common::String target = g_lingo->pop().asString();

	const char *str = d_strstr(source.c_str(), target.c_str());

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
	ScriptContext *sc = g_lingo->_compiler->compileAnonymous(code, kLPPTrimGarbage);
	if (!sc) {
		warning("b_value(): Failed to parse expression \"%s\", returning void", expr.c_str());
		g_lingo->pushVoid();
		return;
	}
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
	case RECT:
	case POINT:
		result.u.i = list.u.farr->arr.size();
		break;
	case PARRAY:
		result.u.i = list.u.parr->arr.size();
		break;
	case OBJECT:
		result.u.i = list.u.obj->getPropCount();
		break;
	default:
		TYPECHECK3(list, ARRAY, PARRAY, OBJECT);
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

void LB::b_deleteOne(int nargs) {
	Datum val = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(list, ARRAY, PARRAY);

	switch (list.type) {
	case ARRAY: {
		g_lingo->push(list);
		g_lingo->push(val);
		b_getPos(nargs);
		int index = g_lingo->pop().asInt();
		if (index > 0) {
			list.u.farr->arr.remove_at(index - 1);
		}
		break;
	}
	case PARRAY: {
		Datum d;
		int index = LC::compareArrays(LC::eqData, list, val, true, true).u.i;
		if (index > 0) {
			list.u.parr->arr.remove_at(index - 1);
		}
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
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


void LB::b_duplicateList(int nargs) {
	Datum list = g_lingo->pop();
	TYPECHECK2(list, ARRAY, PARRAY);
	g_lingo->push(list.clone());
}


void LB::b_findPos(int nargs) {
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	Datum d(0);
	TYPECHECK(list, PARRAY);

	int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
	if (index > 0) {
		d = index;
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
	case OBJECT:
		{
			if (prop.type != SYMBOL) {
				g_lingo->lingoError("b_getaProp(): symbol expected");
				return;
			}
			Datum d;
			if (list.u.obj->hasProp(*prop.u.s))
				d = list.u.obj->getProp(*prop.u.s);
			g_lingo->push(d);
		}
		break;
	default:
		TYPECHECK3(list, ARRAY, PARRAY, OBJECT);
		break;
	}
}

void LB::b_getAt(int nargs) {
	Datum indexD = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	Datum list = g_lingo->pop();
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
	case POINT:
	case RECT:
		ARRBOUNDSCHECK(index, list);
		g_lingo->push(list.u.farr->arr[index - 1]);
		break;
	case PARRAY:
		ARRBOUNDSCHECK(index, list);
		g_lingo->push(list.u.parr->arr[index - 1].v);
		break;
	default:
		TYPECHECK4(list, ARRAY, PARRAY, POINT, RECT);
	}
}

void LB::b_getLast(int nargs) {
	Datum list = g_lingo->pop();
	switch (list.type) {
	case ARRAY:
		if (list.u.farr->arr.empty()) {
			g_lingo->pushVoid();
		} else {
			g_lingo->push(list.u.farr->arr.back());
		}
		break;
	case PARRAY:
		if (list.u.farr->arr.empty()) {
			g_lingo->pushVoid();
		} else {
			g_lingo->push(list.u.parr->arr.back().v);
		}
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
		int index = LC::compareArrays(LC::eqDataStrict, list, val, true).u.i;
		if (index > 0) {
			d.u.i = index;
		}
		g_lingo->push(d);
		break;
	}
	case PARRAY: {
		Datum d(0);
		int index = LC::compareArrays(LC::eqDataStrict, list, val, true, true).u.i;
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

	switch (list.type) {
	case ARRAY:
		if (g_director->getVersion() < 500) {
			// D4 allows getProp to be called on ARRAYs
			g_lingo->push(list);
			g_lingo->push(prop);
			b_getAt(nargs);
		} else {
			g_lingo->lingoError("BUILDBOT: b_getProp: Attempted to call on an ARRAY");
		}
		break;
	case PARRAY: {
		int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
		if (index > 0) {
			g_lingo->push(list.u.parr->arr[index - 1].v);
		} else {
			g_lingo->lingoError("BUILDBOT: b_getProp: Property %s not found", prop.asString().c_str());
		}
		break;
	}
	case OBJECT:
		{
			if (prop.type != SYMBOL) {
				g_lingo->lingoError("BUILDBOT: b_getProp(): symbol expected, got %s", prop.type2str());
				return;
			}
			Datum d;
			if (list.u.obj->hasProp(*prop.u.s))
				d = list.u.obj->getProp(*prop.u.s);
			g_lingo->push(d);
		}
		break;
	default:
		TYPECHECK3(list, ARRAY, PARRAY, OBJECT);
		break;
	}
}

void LB::b_getPropAt(int nargs) {
	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	int index = indexD.asInt();
	switch (list.type) {
	case PARRAY:
		{
			if ((index <= 0) || (index > (int)list.u.parr->arr.size())) {
				g_lingo->lingoError("b_getPropAt(): index out of range");
				return;
			}
			g_lingo->push(list.u.parr->arr[index - 1].p);
		}
		break;
	case OBJECT:
		{
			if ((index <= 0) || (index > (int)list.u.obj->getPropCount())) {
				g_lingo->lingoError("b_getPropAt(): index out of range");
				return;
			}
			Common::String key = list.u.obj->getPropAt(index);
			Datum result(key);
			result.type = SYMBOL;
			g_lingo->push(result);
		}
		break;
	default:
		TYPECHECK2(list, PARRAY, OBJECT);
		break;
	}

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
	case OBJECT:
		{
			if (prop.type != SYMBOL) {
				g_lingo->lingoError("b_setaProp(): symbol expected");
				return;
			}
			list.u.obj->setProp(*prop.u.s, value);
		}
		break;
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

	switch (list.type) {
	case PARRAY:
		{
			int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
			if (index > 0) {
				list.u.parr->arr[index - 1].v = value;
			} else {
				warning("b_setProp: Property not found");
			}
		}
		break;
	case OBJECT:
		{
			if (prop.type != SYMBOL) {
				g_lingo->lingoError("BUILDBOT: b_setProp(): symbol expected, got %s", prop.type2str());
				return;
			}
			// unlike the PARRAY case, OBJECT seems to create
			// new properties without throwing an error
			list.u.obj->setProp(*prop.u.s, value);
		}
		break;
	default:
		TYPECHECK2(list, PARRAY, OBJECT);
		break;
	}
}

static bool sortArrayHelper(const Datum &lhs, const Datum &rhs) {
	return lhs.asString() < rhs.asString();
}

static bool sortNumericArrayHelper(const Datum &lhs, const Datum &rhs) {
	return lhs.asFloat() < rhs.asFloat();
}

static bool sortPArrayHelper(const PCell &lhs, const PCell &rhs) {
	return lhs.p.asString() < rhs.p.asString();
}

static bool sortNumericPArrayHelper(const PCell &lhs, const PCell &rhs) {
	return lhs.p.asFloat() < rhs.p.asFloat();
}

void LB::b_sort(int nargs) {
	// in D4 manual, p266. linear list is sorted by values
	// property list is sorted alphabetically by properties
	// once the list is sorted, it maintains its sort order even when we add new variables using add command
	// see b_append to get more details.
	Datum list = g_lingo->pop();

	if (list.type == ARRAY) {
		// Check to see if the array is full of numbers
		bool isNumeric = true;
		for (const auto &it : list.u.farr->arr) {
			isNumeric &= it.isNumeric();
		}
		if (isNumeric) {
			// Sorting an array of numbers will use numeric sort order.
			Common::sort(list.u.farr->arr.begin(), list.u.farr->arr.end(), sortNumericArrayHelper);
		} else {
			// Sorting an array of strings will use the string sort order.
			// Sorting an array of mixed types is undefined behaviour; sometimes the interpreter
			// will give an answer nearly the same as the string sort order, other times
			// the interpreter will softlock.
			Common::sort(list.u.farr->arr.begin(), list.u.farr->arr.end(), sortArrayHelper);
		}
		list.u.farr->_sorted = true;

	} else if (list.type == PARRAY) {
		// Check to see if the array is full of numbers
		bool isNumeric = true;
		for (const auto &it : list.u.parr->arr) {
			isNumeric &= it.p.isNumeric();
		}
		if (isNumeric) {
			Common::sort(list.u.parr->arr.begin(), list.u.parr->arr.end(), sortNumericPArrayHelper);
		} else {
			Common::sort(list.u.parr->arr.begin(), list.u.parr->arr.end(), sortPArrayHelper);
		}
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
	// closeResFile closes only resource files that were opened with openResFile.

	if (nargs == 0) { // Close all open resource files
		for (auto &it : g_director->_openResFiles)
			g_director->_allOpenResFiles.remove(it._key);

		g_director->_openResFiles.clear();
		return;
	}

	Datum d = g_lingo->pop();
	Common::Path resFileName(g_director->getCurrentWindow()->getCurrentPath() + d.asString(), g_director->_dirSeparator);

	if (g_director->_openResFiles.contains(resFileName)) {
		g_director->_openResFiles.erase(resFileName);

		g_director->_allOpenResFiles.remove(resFileName);
	}
}

void LB::b_closeXlib(int nargs) {
	if (nargs ==0) { // Close all Xlibs
		g_lingo->closeOpenXLibs();
		return;
	}

	Datum d = g_lingo->pop();
	Common::String xlibName = getFileName(d.asString());
	g_lingo->closeXLib(xlibName);
}

void LB::b_getNthFileNameInFolder(int nargs) {
	int fileNum = g_lingo->pop().asInt() - 1;
	Common::String pathRaw = g_lingo->pop().asString();
	if (pathRaw.empty()) {
		// If we receive a blank string as a path, it shouldn't match anything.
		g_lingo->push(Datum(""));
		return;
	}

	// getNthFileNameInFolder requires an absolute path as an input.
	// relative paths will not match anything.
	Common::Path path = findAbsolutePath(pathRaw, true);
	// for directory, we either return the correct path, which we can access recursively.
	// or we get a wrong path, which will lead us to a non-exist file node

	Common::StringArray directory_list = path.splitComponents();
	Common::FSNode d = Common::FSNode(*g_director->getGameDataDir());
	for (auto &it : directory_list) {
		d = d.getChild(it);
		if (!d.exists())
			break;
	}

	Datum r("");
	Common::Array<Common::String> fileNameList;

	// First, mix in any files injected from the quirks
	Common::Archive *cache = SearchMan.getArchive(kQuirksCacheArchive);
	if (cache) {
		Common::ArchiveMemberList files;

		cache->listMatchingMembers(files, path.append(path.empty() ? "*" : "/*", '/'), true);

		for (auto &fi : files) {
			fileNameList.push_back(Common::lastPathComponent(fi->getName(), '/'));
		}
	}

	// Next, mix in files from the game filesystem (if they exist)
	if (d.exists()) {
		Common::FSList f;
		if (!d.getChildren(f, Common::FSNode::kListAll)) {
			warning("Cannot access directory %s", path.toString(Common::Path::kNativeSeparator).c_str());
		} else {
			for (uint i = 0; i < f.size(); i++)
				fileNameList.push_back(f[i].getName());
		}
	}

	if (!fileNameList.empty() && (uint)fileNum < fileNameList.size()) {
		// Sort files alphabetically
		Common::sort(fileNameList.begin(), fileNameList.end());
		r = Datum(fileNameList[fileNum]);
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
	Common::Path resPath(g_director->getCurrentWindow()->getCurrentPath() + d.asString(), g_director->_dirSeparator);

	if (g_director->getPlatform() == Common::kPlatformWindows) {
 		warning("STUB: BUILDBOT: b_openResFile(%s) on Windows", d.asString().c_str());
 		return;
 	}

	if (!g_director->_allSeenResFiles.contains(resPath)) {
		MacArchive *arch = new MacArchive();
		if (arch->openFile(findPath(resPath))) {
			g_director->_openResFiles.setVal(resPath, arch);
			g_director->_allSeenResFiles.setVal(resPath, arch);
			g_director->addArchiveToOpenList(resPath);
		} else {
			delete arch;
		}
	}
}

void LB::b_openXlib(int nargs) {
	Common::String xlibName;

	Datum d = g_lingo->pop();

	Common::Path xlibPath = findXLibPath(d.asString(), true, false);
	if (g_director->getPlatform() == Common::kPlatformMacintosh) {
		// try opening the file as a Macintosh resource fork
		MacArchive *resFile = new MacArchive();
		if (resFile->openFile(xlibPath)) {
			uint32 XCOD = MKTAG('X', 'C', 'O', 'D');
			uint32 XCMD = MKTAG('X', 'C', 'M', 'D');
			uint32 XFCN = MKTAG('X', 'F', 'C', 'N');

			Common::Array<uint16> rsrcList = resFile->getResourceIDList(XCOD);

			for (uint i = 0; i < rsrcList.size(); i++) {
				xlibName = resFile->getResourceDetail(XCOD, rsrcList[i]).name.c_str();
				g_lingo->openXLib(xlibName, kXObj, xlibPath);
			}

			rsrcList = resFile->getResourceIDList(XCMD);
			for (uint i = 0; i < rsrcList.size(); i++) {
				xlibName = resFile->getResourceDetail(XCMD, rsrcList[i]).name.c_str();
				g_lingo->openXLib(xlibName, kXObj, xlibPath);
			}

			rsrcList = resFile->getResourceIDList(XFCN);
			for (uint i = 0; i < rsrcList.size(); i++) {
				xlibName = resFile->getResourceDetail(XFCN, rsrcList[i]).name.c_str();
				g_lingo->openXLib(xlibName, kXObj, xlibPath);
			}
			delete resFile;
			return;
		}
		delete resFile;
	}

	xlibName = getFileName(d.asString());

	// TODO: Figure out a nicer way of differentiating Xtras from XLibs on Mac
	if (xlibName.hasSuffixIgnoreCase(".x16") || xlibName.hasSuffixIgnoreCase(".x32")) {
		g_lingo->openXLib(xlibName, kXtraObj, xlibPath);
	} else {
		g_lingo->openXLib(xlibName, kXObj, xlibPath);
	}
}

void LB::b_saveMovie(int nargs) {
	g_lingo->printSTUBWithArglist("b_saveMovie", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_setCallBack(int nargs) {
	g_lingo->printSTUBWithArglist("b_setCallBack", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_showResFile(int nargs) {
	if (nargs)
		g_lingo->pop();
	Common::String out;
	for (auto &it : g_director->_allOpenResFiles)
		out += it.toString(g_director->_dirSeparator) + "\n";
	g_debugger->debugLogFile(out, false);
}

void LB::b_showXlib(int nargs) {
	if (nargs)
		g_lingo->pop();
	Common::String out;
	for (auto &it : g_lingo->_openXLibs)
		out += it._key + "\n";
	g_debugger->debugLogFile(out, false);
}

void LB::b_xFactoryList(int nargs) {
	g_lingo->pop();
	Datum d("");

	for (auto &it : g_lingo->_openXLibs)
		*d.u.s += it._key + "\n";
	g_lingo->push(d);
}

void LB::b_xtra(int nargs) {
	Datum d = g_lingo->pop();
	if (d.type == INT) {
		int i = d.asInt() -1; // Lingo index for XTRAs start at 1
		if (i >=0 && (uint)i < g_lingo->_openXtras.size()) {
			Datum var = g_lingo->_globalvars[g_lingo->_openXtras[i]];
			g_lingo->push(var);
			return;
		}
	} else {
		Common::String name = d.asString();
		if (g_lingo->_globalvars.contains(name)) {
			Datum var = g_lingo->_globalvars[name];
			if (var.type == OBJECT && var.u.obj->getObjType() == kXtraObj) {
				g_lingo->push(var);
				return;
			}
		}
	}
	g_lingo->lingoError("Xtra not found: %s", d.asString().c_str());
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
}

void LB::b_nothing(int nargs) {
	// Noop
}

void LB::b_delay(int nargs) {
	Datum d = g_lingo->pop();
	g_director->getCurrentMovie()->getScore()->setDelay(d.asInt());
}

void LB::b_do(int nargs) {
	Common::String code = g_lingo->pop().asString();
	ScriptContext *sc = g_lingo->_compiler->compileAnonymous(code);
	if (!sc) {
		warning("b_do(): compilation failed, ignoring");
		return;
	} else if (!sc->_eventHandlers.contains(kEventGeneric)) {
		warning("b_do(): compiled code did not return handler, ignoring");
		return;
	}
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

			if (nargs > 0 && firstArg.type == STRING) {
				movie = firstArg;
				TYPECHECK(movie, STRING);

				frame = g_lingo->pop();
				nargs -= 1;
			// Even if there's more than one argument, if the first
			// arg is an int, Director discards the remainder and
			// treats it as the frame.
			} else if (nargs > 0 && firstArg.type == INT) {
				frame = g_lingo->pop();
				nargs -= 1;
			} else {
				frame = firstArg;
			}

			if (frame.type != STRING && frame.type != INT) {
				warning("b_go: frame arg should be of type STRING or INT, not %s", frame.type2str());
			}

			g_lingo->func_goto(frame, movie, true);
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
		g_lingo->_theResult = Datum((int)g_director->getCurrentMovie()->getScore()->getFramesNum());
		return;
	}

	g_lingo->_theResult = g_lingo->pop();

	if (nargs > 1)
		g_lingo->dropStack(nargs - 1);
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

	fps = MAX(1, fps);

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
	int pos = g_lingo->pop().asInt();
	Datum result;
	CFrame *cf = g_lingo->_state->callstack[g_lingo->_state->callstack.size() - 1];
	// for named parameters, b_param must return what the current value is (i.e.
	// if the handler has changed it, return that)
	if (pos > 0 && cf->sp.argNames && pos <= (int)cf->sp.argNames->size()) {
		Datum func((*cf->sp.argNames)[pos - 1]);
		func.type = LOCALREF;
		result = g_lingo->varFetch(func);
	} else if (pos > 0 && pos <= (int)cf->paramList.size()) {
		// otherwise, if a function was called with extra unnamed arguments,
		// return that.
		result = cf->paramList[pos - 1];
	} else {
		warning("Invalid argument position %d", pos);
	}
	g_lingo->push(result);
}

void LB::b_printFrom(int nargs) {
	warning("BUILDBOT: printFrom is not supported in ScummVM");

	g_lingo->dropStack(nargs);
}

void LB::b_quit(int nargs) {
	Movie *movie = g_director->getCurrentMovie();
	if (movie)
		movie->getScore()->_playState = kPlayStopped;

	g_lingo->pushVoid();
}

void LB::b_return(int nargs) {
	CFrame *fp = g_lingo->_state->callstack.back();

	Datum retVal;
	if (nargs > 0) {
		retVal = g_lingo->pop();
		g_lingo->_theResult = retVal;	// Store result for possible reference
	}

	// clear any temp values from loops
	while (g_lingo->_stack.size() > fp->stackSizeBefore)
		g_lingo->pop();

	// Do not allow a factory's mNew method to return a value
	if (nargs > 0 && !(g_lingo->_state->me.type == OBJECT && g_lingo->_state->me.u.obj->getObjType() == kFactoryObj
			&& fp->sp.name->equalsIgnoreCase("mNew"))) {
		g_lingo->push(retVal);
	}

	LC::procret();
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
	Datum res(0);
	if (nargs == 1) {
		// Single-argument mode returns the type of the item as a symbol.
		// D4 is inconsistent about what types this variant is allowed to work with; e.g. #integer is fine,
		// but #proplist is not. For now, give a response for all types.
		Datum item = g_lingo->pop();
		res = Datum(Common::String(item.type2str(true)));
		res.type = SYMBOL;
		g_lingo->push(res);
		return;
	}

	if (nargs > 2) {
		warning("b_ilk: dropping %d extra args", nargs - 2);
		g_lingo->dropStack(nargs - 2);
	}

	// Two argument mode checks the type of the item against a symbol.
	Datum type = g_lingo->pop();
	Datum item = g_lingo->pop();
	if (type.type != SYMBOL) {
		warning("b_ilk: expected a symbol for second arg");
	} else {
		Common::String typeCopy = type.asString();

		// A special case is #list, which is the equivalent of checking the item type is one of #linearlist,
		// #proplist, #point and #rect.
		if (typeCopy.equalsIgnoreCase("list")) {
			res.u.i = item.type == ARRAY ? 1 : 0;
			res.u.i |= item.type == PARRAY ? 1 : 0;
			res.u.i |= item.type == POINT ? 1 : 0;
			res.u.i |= item.type == RECT ? 1 : 0;
		} else {
			res.u.i = typeCopy.equalsIgnoreCase(item.type2str(true)) ? 1 : 0;
		}
	}
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
	} else if (d.type == ARRAY || d.type == PARRAY) {
		res = 1;
	} else {
		res = 0;
	}
	g_lingo->push(res);
}

void LB::b_pictureP(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == PICTUREREF) ? 1 : 0);
	g_lingo->push(res);
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
		GUI::MessageDialog dialog(g_director->getCurrentMovie()->getCast()->decodeString(alert), _("OK"));
		dialog.runModal();
	}
}

void LB::b_clearGlobals(int nargs) {
	for (auto &it : g_lingo->_globalvars) {
		if (!it._value.ignoreGlobal) {
			// For some reason, factory objects are not removed
			// by this command.
			if (it._value.type == OBJECT && it._value.u.obj->getObjType() & (kFactoryObj | kScriptObj))
				continue;
			g_lingo->_globalvars.erase(it._key);
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
	if (g_lingo->_state->localVars) {
		for (auto it = g_lingo->_state->localVars->begin(); it != g_lingo->_state->localVars->end(); it++) {
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
	Datum to;
	Datum from;
	Movie *movie = g_director->getCurrentMovie();

	if (nargs >= 2) {
		nargs -= 2;
		g_lingo->dropStack(nargs);
		to = g_lingo->pop();
		from = g_lingo->pop();
		if (from.type != CASTREF)
			error("b_duplicate(): expected CASTREF for from, got %s", from.type2str());
		if (to.type == INT)
			to = Datum(CastMemberID(to.asInt(), DEFAULT_CAST_LIB));
		else if (to.type != CASTREF)
			error("b_duplicate(): expected CASTREF or INT for to, got %s", to.type2str());
	} else if (nargs == 1) {
		// use next available slot in the same cast library
		from = g_lingo->pop();
		if (from.type != CASTREF)
			error("b_duplicate(): expected CASTREF for from, got %s", from.type2str());
		if (!movie->getCasts()->contains(from.u.cast->castLib))
			error("b_duplicate(): couldn't find cast lib %d", from.u.cast->castLib);

		Cast *cast = movie->getCasts()->getVal(from.u.cast->castLib);
		to = Datum(CastMemberID(cast->getNextUnusedID(), from.u.cast->castLib));
	} else {
		error("b_duplicate(): expected at least 1 argument");
	}

	if (!movie->duplicateCastMember(*from.u.cast, *to.u.cast)) {
		warning("b_duplicate(): failed to copy cast member %s to %s", from.u.cast->asString().c_str(), to.u.cast->asString().c_str());
	}

	Score *score = movie->getScore();
	// force redraw any sprites
	score->refreshPointersForCastMemberID(*to.u.cast);
	b_updateStage(0);
	g_lingo->push(Datum(to.u.cast->member));
}

void LB::b_editableText(int nargs) {
	// editableText is deprecated in D4+ with the addition of "the editableText",
	// but is still a valid function call.
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
	} else if (nargs == 0) {
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
	Movie *movie = g_director->getCurrentMovie();
	CastMember *eraseCast = movie->getCastMember(d.asMemberID());

	if (eraseCast) {
		eraseCast->_erase = true;
		Common::Array<Channel *> channels = movie->getScore()->_channels;

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

	Datum res;

	if (d.u.cast->member > c_end) {
		res = d.u.cast->member;
		g_lingo->push(res);
		return;
	}

	if (d.u.cast->member > c_start) {
		c_start = (uint16) d.u.cast->member;
	}

	for (uint16 i = c_start; i <= c_end; i++) {
		if (!(cast->getCastMember(i) && cast->getCastMember(i)->_type != kCastTypeNull)) {
			res = i;
			g_lingo->push(res);
			return;
		}
	}

	res = (int) c_end + 1;
	g_lingo->push(res);
}

void LB::b_importFileInto(int nargs) {

	Common::String file = g_lingo->pop().asString();
	Datum dst = g_lingo->pop();

	if (!dst.isCastRef()) {
		warning("b_importFileInto(): bad cast ref field type: %s", dst.type2str());
		return;
	}

	CastMemberID memberID = *dst.u.cast;

	if (!(file.matchString("*.pic") || file.matchString("*.pict"))) {
		warning("LB::b_importFileInto : %s is not a valid PICT file", file.c_str());
		return;
	}

	Common::Path path = findPath(file);
	Common::File in;
	in.open(path);

	if (!in.isOpen()) {
		warning("b_importFileInto(): Cannot open file %s", path.toString().c_str());
		return;
	}

	Image::PICTDecoder *img = new Image::PICTDecoder();
	img->loadStream(in);
	in.close();

	Movie *movie = g_director->getCurrentMovie();
	Score *score = movie->getScore();
	BitmapCastMember *bitmapCast = new BitmapCastMember(movie->getCast(), memberID.member, img);
	movie->createOrReplaceCastMember(memberID, bitmapCast);
	bitmapCast->setModified(true);
	const Graphics::Surface *surf = img->getSurface();
	bitmapCast->_size = surf->pitch * surf->h + img->getPaletteColorCount() * 3;
	score->refreshPointersForCastMemberID(dst.asMemberID());
}

void menuCommandsCallback(int action, Common::String &text, void *data) {
	g_director->getCurrentMovie()->queueInputEvent(kEventMenuCallback, action);
}

void LB::b_installMenu(int nargs) {
	// installMenu castNum
	Datum d = g_lingo->pop();

	CastMemberID memberID = d.asMemberID(kCastText);
	if (memberID.member == 0) {
		debugC(3, kDebugLoading, "LB::b_installMenu(): removing menu");
		g_director->_wm->removeMenu();
		return;
	}
	Movie *movie = g_director->getCurrentMovie();
	CastMember *member = movie->getCastMember(memberID);
	if (!member) {
		g_lingo->lingoError("installMenu: Unknown %s", memberID.asString().c_str());
		return;
	}
	if (member->_type != kCastText) {
		g_lingo->lingoError("installMenu: %s is not a field", memberID.asString().c_str());
		return;
	}
	TextCastMember *field = static_cast<TextCastMember *>(member);

	Common::String menuStxt = field->getRawText();
	// clang reports linenum variable is unused
	// int linenum = -1; // We increment it before processing

	Graphics::MacMenu *menu = g_director->_wm->addMenu();
	int submenu = -1;
	Common::String submenuText;
	Common::String command;
	int commandId = 100;

	menu->setCommandsCallback(menuCommandsCallback, g_director);

	debugC(3, kDebugLoading, "LB::b_installMenu(): installing menu - '%s'", Common::toPrintable(menuStxt).c_str());

	LingoArchive *mainArchive = movie->getMainLingoArch();

	// STXT sections use Mac-style carriage returns for line breaks.
	const char LINE_BREAK_CHAR = '\x0D';
	// Menu definitions use the character 0xc5 to denote a code separator.
	// For Mac D4 and below, this is ≈. For Windows D4 and below, this is Å.
	char CODE_SEPARATOR_CHAR = '\xC5';
	char CODE_SEPARATOR_CHAR_2 = '\xC5';
	if (g_director->getVersion() >= 500) {
		// D5 changed this to be the pipe | character, the same in Windows and Mac.
		CODE_SEPARATOR_CHAR = '\x7C';
		// FIXME: For some reason there are games which use º (Mac) or ¼ (Win) and it works too?
		CODE_SEPARATOR_CHAR_2 = '\xBC';
	}
	// Continuation character is 0xac to denote a line running over.
	// For Mac, this is ¨. For Windows, this is ¬.
	const char CONTINUATION_CHAR = '\xAC';
	// Menu definitions use the character 0xc3 to denote a checkmark.
	// For Mac, this is √. For Windows, this is Ã.
	// This is used by MacMenu::createSubMenuFromString.

	Common::String line;

	for (auto it = menuStxt.begin(); it != menuStxt.end(); ++it) {
		line.clear();
		while (it != menuStxt.end() && *it != LINE_BREAK_CHAR) {
			if (*it == '-') {
				it++;
				if (it != menuStxt.end() && *it == '-') { // rest of the line is a comment
					while (it != menuStxt.end() && *it != LINE_BREAK_CHAR) {
						it++;
					}
					break;
				}
				line += '-';
			} else if (*it == CONTINUATION_CHAR) { // fast forward to the next line
				it++;
				if (*it == LINE_BREAK_CHAR) {
					line += ' ';
					it++;
				}
			} else if (*it == LINE_BREAK_CHAR) {
				break;
			} else {
				line += *it++;
			}
		}
		// clang reports linenum variable is unused
		// linenum++;

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

		// Split the line at the code separator
		size_t sepOffset = line.find(CODE_SEPARATOR_CHAR);
		if (sepOffset == Common::String::npos)
			sepOffset = line.find(CODE_SEPARATOR_CHAR_2);

		Common::String text;

		if (sepOffset != Common::String::npos) {
			text = Common::String(line.c_str(), line.c_str() + sepOffset);
			command = Common::String(line.c_str() + sepOffset + 1);
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

		if (it == menuStxt.end()) // if we reached end of string, do not increment it but break
			break;
	}

	if (!submenuText.empty()) {
		menu->createSubMenuFromString(submenu, submenuText.c_str(), 0);
	}
}

void LB::b_label(int nargs) {
	// label functions as marker when the input is an int
	Datum d = g_lingo->pop();
	int marker;
	if (d.type == STRING) {
		marker = g_lingo->func_label(d);
	} else {
		marker = g_lingo->func_marker(d.asInt());
	}
	g_lingo->push(marker);
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
		int id = (int) g_director->getCurrentMovie()->getCast()->_castArrayStart;
		CastMemberID castId(id, DEFAULT_CAST_LIB);
		Datum d = Datum(castId);
		g_lingo->push(d);
		b_findEmpty(1);
	} else if (nargs == 2) {
		// pass
	} else {
		ARGNUMCHECK(2);
		g_lingo->dropStack(nargs);
		return;
	}
	dest = g_lingo->pop();
	src = g_lingo->pop();

	// Convert dest datum to type CASTREF if it is INT.
	// Confirmed to always move to DEFAULT_CAST_LIB in D5
	if (dest.type == INT) {
		dest = Datum(CastMemberID(dest.asInt(), DEFAULT_CAST_LIB));
	}

	// No need to move if src and dest are same
	if (src == dest) {
		return;
	}

	Movie *movie = g_director->getCurrentMovie();
	CastMember *toMove = movie->getCastMember(src.asMemberID());

	if (!toMove) {
		warning("b_move: Source CastMember doesn't exist");
		return;
	}

	g_lingo->push(dest);
	Score *score = movie->getScore();
	uint16 frame = score->getCurrentFrameNum();

	score->renderFrame(frame, kRenderForceUpdate);

	movie->duplicateCastMember(src.asMemberID(), dest.asMemberID());
	movie->eraseCastMember(src.asMemberID());

	score->refreshPointersForCastMemberID(dest.asMemberID());
	score->refreshPointersForCastMemberID(src.asMemberID());

	score->renderFrame(frame, kRenderForceUpdate);
}

void LB::b_moveableSprite(int nargs) {
	Movie *movie = g_director->getCurrentMovie();
	Score *score = movie->getScore();
	Frame *frame = score->_currentFrame;

	if (g_lingo->_currentChannelId == -1) {
		warning("b_moveableSprite: channel Id is missing");
		assert(0);
		return;
	}

	// since we are using value copying, in order to make it taking effect immediately. we modify the sprites in channel
	if (score->_channels[g_lingo->_currentChannelId])
		score->_channels[g_lingo->_currentChannelId]->_sprite->_moveable = true;
	frame->_sprites[g_lingo->_currentChannelId]->_moveable = true;
}

void LB::b_pasteClipBoardInto(int nargs) {
	Datum to = g_lingo->pop();
	if (!g_director->_clipBoard) {
		warning("LB::b_pasteClipBoardInto(): Nothing to paste from clipboard, skipping paste..");
		return;
	}

	Movie *movie = g_director->getCurrentMovie();

	CastMember *castMember = movie->getCastMember(*g_director->_clipBoard);
	if (!castMember) {
		warning("LB:B_pasteClipBoardInto(): castMember not found");
		return;
	}

	Score *score = movie->getScore();
	castMember->setModified(true);
	movie->duplicateCastMember(*g_director->_clipBoard, *to.u.cast);
	score->refreshPointersForCastMemberID(to.asMemberID());
}

static const struct PaletteNames {
	const char *name;
	PaletteType type;
} paletteNames[] = {
	{ "System", kClutSystemMac },
	{ "System - Mac", kClutSystemMac },
	{ "Rainbow", kClutRainbow },
	{ "Grayscale", kClutGrayscale },
	{ "Pastels", kClutPastels },
	{ "Vivid", kClutVivid },
	{ "NTSC", kClutNTSC },
	{ "Metallic", kClutMetallic },
	//{ "Web 216", },
	//{ "VGA", },
	//{ "System - Win", },
	{ "SYSTEM - WIN (DIR 4)", kClutSystemWin },

	// Japanese palette names.
	// TODO: Check encoding. Original is SJIS
	{ "\x83V\x83X\x83""e\x83\x80 - Mac", kClutSystemMac },				// システム - Mac
	{ "\x83\x8C>\x83""C\x83\x93\x83{\x81[", kClutRainbow },				// レインボー
	{ "\x83O\x83>\x8C\x81[\x83X\x83P\x81[\x83\x8B", kClutGrayscale },	// グレースケール
	{ "\x83p\x83>X\x83""e\x83\x8B", kClutPastels },						// パステル
	{ "\x83r\x83>r\x83""b\x83h", kClutVivid },							// ビビッド
	{ "\x83\x81\x83^\x83\x8A\x83""b\x83N", kClutMetallic },				// メタリック
	// { "\x83V\x83X\x83""e\x83\x80 - Win", },							// システム - Win
	{ "\x83V\x83X\x83""e\x83\x80 - Win (Dir 4)", kClutSystemWin },		// システム - Win (Dir 4)
};

void LB::b_puppetPalette(int nargs) {
	g_lingo->convertVOIDtoString(0, nargs);
	int numFrames = 0, speed = 0;
	CastMemberID palette(0, 0);
	Datum d;
	Movie *movie = g_director->getCurrentMovie();

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

			for (int i = 0; i < ARRAYSIZE(paletteNames); i++) {
				if (palStr.equalsIgnoreCase(paletteNames[i].name))
					palette = CastMemberID(paletteNames[i].type, -1);
			}
		}
		if (palette.isNull()) {
			CastMember *member = movie->getCastMember(d.asMemberID());

			if (member && member->_type == kCastPalette)
				palette = ((PaletteCastMember *)member)->getPaletteId();
		}
		break;
	default:
		ARGNUMCHECK(1);
		return;
	}

	Score *score = movie->getScore();
	if (!palette.isNull()) {
		g_director->setPalette(palette);
		score->_puppetPalette = true;
	} else {
		// Setting puppetPalette to 0 disables it (Lingo Dictionary, 226)

		score->_puppetPalette = false;

		// FIXME: set system palette decided by platform, should be fixed after windows palette is working.
		// try to set mac system palette if lastPalette is 0.
		if (g_director->_lastPalette.isNull())
			g_director->setPalette(CastMemberID(kClutSystemMac, -1));
		else
			g_director->setPalette(g_director->_lastPalette);
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
		CastMemberID castMember = g_lingo->pop().asMemberID(kCastSound);

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
			Datum arg2 = g_lingo->pop();
			Datum arg1 = g_lingo->pop();
			int channel = 1;
			CastMemberID castMember;
			if (arg1.type == STRING) {
				// Apparently if the first argument is a string, it will be evaluated as per the 1-arg case
				castMember = arg1.asMemberID(kCastSound);
			} else {
				// FIXME: Figure out how to deal with multilib in D5+
				castMember = arg2.asMemberID(kCastSound);
				channel = arg1.asInt();
			}
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

		if ((uint)sprite.asInt() < sc->_channels.size()) {
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

		if ((uint)sprite.asInt() < sc->_channels.size()) {
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
	g_director->getCurrentMovie()->getScore()->setPuppetTempo(g_lingo->pop().asInt());
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
		// units are quarter-seconds
		duration = g_lingo->pop().asInt() * 250;
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
	debugC(3, kDebugImages, "b_puppetTransition(): type: %d, duration: %d, chunkSize: %d, area: %d", type, duration, chunkSize, area);

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

	// This automatically sets the stretch mode
	channel->_sprite->_stretch = true;

	g_director->getCurrentWindow()->addDirtyRect(channel->getBbox());
	channel->setBbox(
		l < r ? l : r,
		t < b ? t : b,
		r > l ? r : l,
		b > t ? b : t
	);
	if (channel->_sprite->_cast)
		channel->_sprite->_cast->setModified(true);
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
	uint16 curFrame = score->getCurrentFrameNum();

	Common::Rect startRect = score->_channels[startSpriteId]->getBbox();
	if (startRect.isEmpty()) {
		warning("b_zoomBox: unknown start sprite #%d", startSpriteId);
		return;
	}

	// Looks for endSprite in the current frame, otherwise
	// Looks for endSprite in the next frame
	Common::Rect endRect = score->_channels[endSpriteId]->getBbox();
	if (endRect.isEmpty()) {
		if ((uint)curFrame + 1 < score->getFramesNum()) {
			Frame *nextFrame = score->getFrameData(curFrame + 1);
			Channel endChannel(nullptr, nextFrame->_sprites[endSpriteId]);
			endRect = endChannel.getBbox();
			delete nextFrame;
		}
	}

	if (endRect.isEmpty()) {
		if ((uint)curFrame - 1 > 0) {
			Frame *prevFrame = score->getFrameData(curFrame - 1);
			Channel endChannel(nullptr, prevFrame->_sprites[endSpriteId]);
			endRect = endChannel.getBbox();
			delete prevFrame;
		}
	}

	if (endRect.isEmpty()) {
		warning("b_zoomBox: unknown end sprite #%d", endSpriteId);
		return;
	}

	if (Director::g_director->desktopEnabled()) {
		Director::Datum stageRect = Director::g_director->getStage()->getStageRect();
		Common::Point stageOffset(stageRect.u.farr->arr[0].asInt(), stageRect.u.farr->arr[1].asInt());
		startRect.translate(stageOffset.x, stageOffset.y);
		endRect.translate(stageOffset.x, stageOffset.y);
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
	Window *window = movie->getWindow();

	score->updateWidgets(movie->_videoPlayback);
	if (window->_puppetTransition) {
		window->playTransition(score->getCurrentFrameNum(), kRenderModeNormal, window->_puppetTransition->duration, window->_puppetTransition->area, window->_puppetTransition->chunkSize, window->_puppetTransition->type, score->_currentFrame->_mainChannels.scoreCachedPaletteId);
		delete window->_puppetTransition;
		window->_puppetTransition = nullptr;
	} else {
		movie->getWindow()->render();
	}

	// play any puppet sounds that have been queued
	score->playSoundChannel(true);

	if (score->_cursorDirty) {
		score->renderCursor(movie->getWindow()->getMousePos());
		score->_cursorDirty = false;
	}

	g_director->draw();

	if (debugChannelSet(-1, kDebugFewFramesOnly)) {
		g_director->_framesRan++;
		warning("LB::b_updateStage(): ran frame %0d", g_director->_framesRan);

		if (g_director->_framesRan > kFewFamesMaxCounter) {
			warning("b_updateStage(): exiting due to debug few frames only");
			score->_playState = kPlayStopped;
		}
	}
}


///////////////////
// Point
///////////////////
void LB::b_point(int nargs) {
	Datum y(g_lingo->pop().asInt());
	Datum x(g_lingo->pop().asInt());
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

	d = rect1.intersects(rect2);

	g_lingo->push(d);
}

void LB::b_inside(int nargs) {
	Datum d;
	Datum r2 = g_lingo->pop();
	Datum p1 = g_lingo->pop();
	Common::Rect rect2(r2.u.farr->arr[0].asInt(), r2.u.farr->arr[1].asInt(), r2.u.farr->arr[2].asInt(), r2.u.farr->arr[3].asInt());
	Common::Point point1(p1.u.farr->arr[0].asInt(), p1.u.farr->arr[1].asInt());

	d = rect2.contains(point1);

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
	Score *score = g_director->getCurrentMovie()->getScore();

	if (verb.u.s->equalsIgnoreCase("close") || verb.u.s->equalsIgnoreCase("stop")) {
		if (nargs != 2) {
			warning("sound %s: expected 1 argument, got %d", verb.u.s->c_str(), nargs - 1);
			return;
		}

		TYPECHECK(firstArg, INT);
		soundManager->stopSound(firstArg.u.i);
	} else if (verb.u.s->equalsIgnoreCase("fadeIn")) {
		if (nargs > 2) {
			TYPECHECK2(secondArg, INT, FLOAT);
			ticks = secondArg.asInt();
		} else {
			ticks = 15 * (60 / score->_currentFrameRate);
		}

		TYPECHECK(firstArg, INT);
		soundManager->registerFade(firstArg.u.i, true, ticks);
		score->_activeFade = true;
		return;
	} else if (verb.u.s->equalsIgnoreCase("fadeOut")) {
		if (nargs > 2) {
			TYPECHECK2(secondArg, INT, FLOAT);
			ticks = secondArg.asInt();
		} else {
			ticks = 15 * (60 / score->_currentFrameRate);
		}

		TYPECHECK2(firstArg, INT, FLOAT);
		soundManager->registerFade(firstArg.asInt(), false, ticks);
		score->_activeFade = true;
		return;
	} else if (verb.u.s->equalsIgnoreCase("playFile")) {
		ARGNUMCHECK(3)

		TYPECHECK(firstArg, INT);
		TYPECHECK(secondArg, STRING);

		soundManager->playFile(*secondArg.u.s, firstArg.u.i);
	} else {
		warning("b_sound: unknown verb %s", verb.u.s->c_str());
	}
}

void LB::b_soundBusy(int nargs) {
	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	Datum whichChannel = g_lingo->pop();

	// Horror Tour 2 calls this with a void argument
	TYPECHECK2(whichChannel, INT, VOID);
	int channel = whichChannel.u.i;
	if (whichChannel.type == VOID) {
		channel = 1;
	}

	bool isBusy = sound->isChannelActive(channel);
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

void LB::b_castLib(int nargs) {
	Datum d = g_lingo->pop();
	Datum res = d.asInt();
	res.type = CASTLIBREF;
	g_lingo->push(res);
}

void LB::b_member(int nargs) {
	Movie *movie = g_director->getCurrentMovie();
	CastMemberID res;
	if (nargs == 1) {
		Datum member = g_lingo->pop();
		if (member.isCastRef()) {
			res = member.asMemberID();
		} else if (member.isNumeric()) {
			res = movie->getCastMemberIDByMember(member.asInt());
		} else {
			res = movie->getCastMemberIDByName(member.asString());
		}
	} else if (nargs == 2) {
		Datum library = g_lingo->pop();
		Datum member = g_lingo->pop();
		int libId = -1;
		if (library.type == CASTLIBREF) {
			libId = library.u.i;
		} else if (library.isNumeric()) {
			libId = library.asInt();
		} else {
			libId = movie->getCastLibIDByName(library.asString());
		}
		if (member.isCastRef()) {
			res = member.asMemberID();
		} else if (member.isNumeric()) {
			res = CastMemberID(member.asInt(), libId);
		} else {
			res = movie->getCastMemberIDByNameAndType(member.asString(), libId, kCastTypeAny);
		}
	}
	if (!movie->getCastMember(res)) {
		g_lingo->lingoError("No match found for cast member");
		return;
	}
	g_lingo->push(res);
}

void LB::b_script(int nargs) {
	Datum d = g_lingo->pop();
	// FIXME: Check with later versions of director
	//        The kCastText check version breaks Phibos, which loads a
	//        non-kCastText script using this builtin.
	//        With the kCastText version, Phibos crashes during its intro.
	// CastMemberID memberID = d.asMemberID(kCastText);
	CastMemberID memberID = d.asMemberID();
	CastMember *cast = g_director->getCurrentMovie()->getCastMember(memberID);

	if (cast) {
		ScriptContext *script = nullptr;

		if (cast->_type == kCastLingoScript) {
			// script cast can be either a movie script, score script, or parent script (D5+)
			script = g_director->getCurrentMovie()->getScriptContext(kMovieScript, memberID);
			if (!script)
				script = g_director->getCurrentMovie()->getScriptContext(kScoreScript, memberID);
			if (!script)
				script = g_director->getCurrentMovie()->getScriptContext(kParentScript, memberID);
		} else {
			script = g_director->getCurrentMovie()->getScriptContext(kCastScript, memberID);
		}

		if (script) {
			g_lingo->push(script);
			return;
		}
	}
	warning("b_script(): No script context found for '%s'", d.asString(true).c_str());
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

	// Refer window by-indexing, lingo can request using "window #index" where #index is the index of window that is previously
	// created, in tutorial workshop `rect of window`, a window is created using 'open(window "ball")' and the same window is
	// referenced by 'window 1', ie 'put the rect of window 1 into field 9'
	if (d.type == INT || d.type == FLOAT) {
		int windowIndex = d.asInt() - 1;

		if (windowIndex >= 0 && windowIndex < (int)windowList->arr.size()) {
			if (windowList->arr[windowIndex].type == OBJECT && windowList->arr[windowIndex].u.obj->getObjType() == kWindowObj) {
				Window *window = static_cast<Window *>(windowList->arr[windowIndex].u.obj);
				g_lingo->push(window);
				return;
			}
		} else {
			warning("LB::b_window: Window referenced by index %d, out of bounds.", windowIndex);
		}
	}

	Graphics::MacWindowManager *wm = g_director->getMacWindowManager();
	Window *window = new Window(wm->getNextId(), false, false, false, wm, g_director, false);
	window->setName(windowName);
	window->setTitle(windowName);
	window->resizeInner(1, 1);
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
		warning("BUILDBOT: LB::b_scummvmassert: is false at line %d", line.asInt());
	}
	if (debugChannelSet(-1, kDebugLingoStrict)) {
		assert(d.asInt() != 0);
	}
}

void LB::b_scummvmassertequal(int nargs) {
	Datum line = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	int result;

	if (d1.type == ARRAY && d2.type == ARRAY) {
		result = LC::eqData(d1, d2).u.i;
	} else if (d1.type == PARRAY && d2.type == PARRAY) {
		result = LC::eqData(d1, d2).u.i;
	} else {
		result = (d1 == d2);
	}

	if (!result) {
		warning("BUILDBOT: LB::b_scummvmassertequals: %s is not equal %s at line %d", d1.asString().c_str(), d2.asString().c_str(), line.asInt());
	}
	if (debugChannelSet(-1, kDebugLingoStrict)) {
		assert(result == 1);
	}
}

void LB::b_scummvmNoFatalError(int nargs) {
	Datum flag = g_lingo->pop();

	g_director->_noFatalLingoError = (flag.asInt() != 0);

	debug("> scummvmNoFatalEror is set to %d", g_director->_noFatalLingoError);
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
