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
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-gr.h"

namespace Director {

#define ARGNUMCHECK(n) \
	if (nargs != (n)) { \
		warning("%s: expected %d argument%s, got %d", __FUNCTION__, (n), ((n) == 1 ? "" : "s"), nargs); \
		g_lingo->dropStack(nargs); \
		return; \
	}

#define TYPECHECK(datum,t) \
	if ((datum).type != (t)) { \
		warning("%s: %s arg should be of type %s, not %s", __FUNCTION__, #datum, #t, (datum).type2str()); \
		return; \
	}

#define TYPECHECK2(datum, t1, t2)	\
	if ((datum).type != (t1) && (datum).type != (t2)) { \
		warning("%s: %s arg should be of type %s or %s, not %s", __FUNCTION__, #datum, #t1, #t2, (datum).type2str()); \
		return; \
	}

#define TYPECHECK3(datum, t1, t2, t3)	\
	if ((datum).type != (t1) && (datum).type != (t2) && (datum).type != (t3)) { \
		warning("%s: %s arg should be of type %s, %s, or %s, not %s", __FUNCTION__, #datum, #t1, #t2, #t3, (datum).type2str()); \
		return; \
	}

#define ARRBOUNDSCHECK(idx,array) \
	if ((idx)-1 < 0 || (idx) > (int)(array).u.farr->size()) { \
		warning("%s: index out of bounds (%d of %d)", __FUNCTION__, (idx), (array).u.farr->size()); \
		return; \
	}

static struct BuiltinProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	bool parens;
	int version;
	SymbolType type;
} builtins[] = {
	// Math
	{ "abs",			LB::b_abs,			1, 1, true, 200, FBLTIN },	// D2 function
	{ "atan",			LB::b_atan,			1, 1, true, 400, FBLTIN },	//			D4 f
	{ "cos",			LB::b_cos,			1, 1, true, 400, FBLTIN },	//			D4 f
	{ "exp",			LB::b_exp,			1, 1, true, 400, FBLTIN },	//			D4 f
	{ "float",			LB::b_float,		1, 1, true, 400, FBLTIN },	//			D4 f
	{ "integer",		LB::b_integer,		1, 1, true, 300, FBLTIN },	//		D3 f
	{ "log",			LB::b_log,			1, 1, true, 400, FBLTIN },	//			D4 f
	{ "pi",				LB::b_pi,			0, 0, true, 400, FBLTIN },	//			D4 f
	{ "power",			LB::b_power,		2, 2, true, 400, FBLTIN },	//			D4 f
	{ "random",			LB::b_random,		1, 1, true, 200, FBLTIN },	// D2 f
	{ "sin",			LB::b_sin,			1, 1, true, 400, FBLTIN },	//			D4 f
	{ "sqrt",			LB::b_sqrt,			1, 1, true, 200, FBLTIN },	// D2 f
	{ "tan",			LB::b_tan,			1, 1, true, 400, FBLTIN },	//			D4 f
	// String
	{ "chars",			LB::b_chars,		3, 3, true, 200, FBLTIN },	// D2 f
	{ "charToNum",		LB::b_charToNum,	1, 1, true, 200, FBLTIN },	// D2 f
	{ "delete",			LB::b_delete,		1, 1, true, 300, CBLTIN },	//		D3 command
	{ "hilite",			LB::b_hilite,		1, 1, true, 300, CBLTIN },	//		D3 c
	{ "length",			LB::b_length,		1, 1, true, 200, FBLTIN },	// D2 f
	{ "numToChar",		LB::b_numToChar,	1, 1, true, 200, FBLTIN },	// D2 f
	{ "offset",			LB::b_offset,		2, 3, true, 200, FBLTIN },	// D2 f
	{ "string",			LB::b_string,		1, 1, true, 200, FBLTIN },	// D2 f
	{ "value",		 	LB::b_value,		1, 1, true, 200, FBLTIN },	// D2 f
	// Lists
	{ "add",			LB::b_add,			2, 2, false, 400, HBLTIN },	//			D4 handler
	{ "addAt",			LB::b_addAt,		3, 3, false, 400, HBLTIN },	//			D4 h
	{ "addProp",		LB::b_addProp,		3, 3, false, 400, HBLTIN },	//			D4 h
	{ "append",			LB::b_append,		2, 2, false, 400, HBLTIN },	//			D4 h
	{ "count",			LB::b_count,		1, 1, true,  400, FBLTIN },	//			D4 f
	{ "deleteAt",		LB::b_deleteAt,		2, 2, false, 400, HBLTIN },	//			D4 h
	{ "deleteProp",		LB::b_deleteProp,	2, 2, false, 400, HBLTIN },	//			D4 h
	{ "findPos",		LB::b_findPos,		2, 2, true,  400, FBLTIN },	//			D4 f
	{ "findPosNear",	LB::b_findPosNear,	2, 2, true,  400, FBLTIN },	//			D4 f
	{ "getaProp",		LB::b_getaProp,		2, 2, true,  400, FBLTIN },	//			D4 f
	{ "getAt",			LB::b_getAt,		2, 2, true,  400, FBLTIN },	//			D4 f
	{ "getLast",		LB::b_getLast,		1, 1, true,  400, FBLTIN },	//			D4 f
	{ "getOne",			LB::b_getOne,		2, 2, true,  400, FBLTIN },	//			D4 f
	{ "getPos",			LB::b_getPos,		2, 2, true,  400, FBLTIN },	//			D4 f
	{ "getProp",		LB::b_getProp,		2, 2, true,  400, FBLTIN },	//			D4 f
	{ "getPropAt",		LB::b_getPropAt,	2, 2, true,  400, FBLTIN },	//			D4 f
	{ "list",			LB::b_list,			-1, 0, true, 400, FBLTIN },	//			D4 f
	{ "listP",			LB::b_listP,		1, 1, true,  400, FBLTIN },	//			D4 f
	{ "max",			LB::b_max,			-1,0, true,  400, FBLTIN },	//			D4 f
	{ "min",			LB::b_min,			-1,0, true,  400, FBLTIN },	//			D4 f
	{ "setaProp",		LB::b_setaProp,		3, 3, false, 400, HBLTIN },	//			D4 h
	{ "setAt",			LB::b_setAt,		3, 3, false, 400, HBLTIN },	//			D4 h
	{ "setProp",		LB::b_setProp,		3, 3, false, 400, HBLTIN },	//			D4 h
	{ "sort",			LB::b_sort,			1, 1, false, 400, HBLTIN },	//			D4 h
	// Files
	{ "closeDA",	 	LB::b_closeDA, 		0, 0, false, 200, CBLTIN },	// D2 c
	{ "closeResFile",	LB::b_closeResFile,	0, 1, false, 200, CBLTIN },	// D2 c
	{ "closeXlib",		LB::b_closeXlib,	0, 1, false, 200, CBLTIN },	// D2 c
	{ "getNthFileNameInFolder",LB::b_getNthFileNameInFolder,2,2,true,400,FBLTIN }, //D4 f
	{ "open",			LB::b_open,			1, 2, false, 200, CBLTIN },	// D2 c
	{ "openDA",	 		LB::b_openDA, 		1, 1, false, 200, CBLTIN },	// D2 c
	{ "openResFile",	LB::b_openResFile,	1, 1, false, 200, CBLTIN },	// D2 c
	{ "openXlib",		LB::b_openXlib,		1, 1, false, 200, CBLTIN },	// D2 c
	{ "saveMovie",		LB::b_saveMovie,	1, 1, false, 400, CBLTIN },	//			D4 c
	{ "setCallBack",	LB::b_setCallBack,	2, 2, false, 300, CBLTIN },	//		D3 c
	{ "showResFile",	LB::b_showResFile,	0, 1, false, 200, CBLTIN },	// D2 c
	{ "showXlib",		LB::b_showXlib,		0, 1, false, 200, CBLTIN },	// D2 c
	{ "xFactoryList",	LB::b_xFactoryList,	1, 1, true,  300, FBLTIN },	//		D3 f
	// Control
	{ "abort",			LB::b_abort,		0, 0, false, 400, CBLTIN },	//			D4 c
	{ "continue",		LB::b_continue,		0, 0, false, 200, CBLTIN },	// D2 c
	{ "dontPassEvent",	LB::b_dontPassEvent,0, 0, false, 200, CBLTIN },	// D2 c
	{ "delay",	 		LB::b_delay,		1, 1, false, 200, CBLTIN },	// D2 c
	{ "do",		 		LB::b_do,			1, 1, false, 200, CBLTIN },	// D2 c
	{ "go",		 		LB::b_go,			1, 2, false, 400, CBLTIN },	// 			D4 c
	{ "halt",	 		LB::b_halt,			0, 0, false, 400, CBLTIN },	//			D4 c
	{ "nothing",		LB::b_nothing,		0, 0, false, 200, CBLTIN },	// D2 c
	{ "pass",			LB::b_pass,			0, 0, false, 400, CBLTIN },	//			D4 c
	{ "pause",			LB::b_pause,		0, 0, false, 200, CBLTIN },	// D2 c
	{ "play",			LB::b_play,			0, 2, false, 200, CBLTIN },	// D2 c
	{ "playAccel",		LB::b_playAccel,	-1,0, false, 200, CBLTIN },	// D2
		// play done													// D2
	{ "preLoad",		LB::b_preLoad,		-1,0, false, 300, CBLTIN },	//		D3.1 c
	{ "preLoadCast",	LB::b_preLoadCast,	-1,0, false, 300, CBLTIN },	//		D3.1 c
	{ "quit",			LB::b_quit,			0, 0, false, 200, CBLTIN },	// D2 c
	{ "restart",		LB::b_restart,		0, 0, false, 200, CBLTIN },	// D2 c
	{ "return",			LB::b_return,		0, 1, false, 200, CBLTIN },	// D2 f
	{ "scummvm_returnNumber", LB::b_returnNumber, 1, 1, false, 200, CBLTIN }, // D2 f
	{ "shutDown",		LB::b_shutDown,		0, 0, false, 200, CBLTIN },	// D2 c
	{ "startTimer",		LB::b_startTimer,	0, 0, false, 200, CBLTIN },	// D2 c
		// when keyDown													// D2
		// when mouseDown												// D2
		// when mouseUp													// D2
		// when timeOut													// D2
	// Types
	{ "factory",		LB::b_factory,		1, 1, true,  300, FBLTIN },	//		D3
	{ "floatP",			LB::b_floatP,		1, 1, true,  300, FBLTIN },	//		D3
	{ "ilk",	 		LB::b_ilk,			1, 2, false, 400, FBLTIN },	//			D4 f
	{ "integerp",		LB::b_integerp,		1, 1, true,  200, FBLTIN },	// D2 f
	{ "objectp",		LB::b_objectp,		1, 1, true,  200, FBLTIN },	// D2 f
	{ "pictureP",		LB::b_pictureP,		1, 1, true,  400, FBLTIN },	//			D4 f
	{ "stringp",		LB::b_stringp,		1, 1, true,  200, FBLTIN },	// D2 f
	{ "symbolp",		LB::b_symbolp,		1, 1, true,  200, FBLTIN },	// D2 f
	{ "voidP",			LB::b_voidP,		1, 1, true,  400, FBLTIN },	//			D4 f
	// Misc
	{ "alert",	 		LB::b_alert,		1, 1, false, 200, CBLTIN },	// D2 c
	{ "clearGlobals",	LB::b_clearGlobals,	0, 0, false, 300, CBLTIN },	//		D3.1 c
	{ "cursor",	 		LB::b_cursor,		1, 1, false, 200, CBLTIN },	// D2 c
	{ "framesToHMS",	LB::b_framesToHMS,	4, 4, false, 300, FBLTIN },	//		D3 f
	{ "HMStoFrames",	LB::b_HMStoFrames,	4, 4, false, 300, FBLTIN },	//		D3 f
	{ "param",	 		LB::b_param,		1, 1, true,  400, FBLTIN },	//			D4 f
	{ "printFrom",	 	LB::b_printFrom,	-1,0, false, 200, CBLTIN },	// D2 c
	{ "put",			LB::b_put,			-1,0, false, 200, CBLTIN },	// D2
		// set															// D2
	{ "showGlobals",	LB::b_showGlobals,	0, 0, false, 200, CBLTIN },	// D2 c
	{ "showLocals",		LB::b_showLocals,	0, 0, false, 200, CBLTIN },	// D2 c
	// Score
	{ "constrainH",		LB::b_constrainH,	2, 2, true,  200, FBLTIN },	// D2 f
	{ "constrainV",		LB::b_constrainV,	2, 2, true,  200, FBLTIN },	// D2 f
	{ "copyToClipBoard",LB::b_copyToClipBoard,1,1, false, 400, CBLTIN }, //			D4 c
	{ "duplicate",		LB::b_duplicate,	1, 2, false, 400, CBLTIN },	//			D4 c
	{ "editableText",	LB::b_editableText,	0, 0, false, 200, CBLTIN },	// D2, FIXME: the field in D4+
	{ "erase",			LB::b_erase,		1, 1, false, 400, CBLTIN },	//			D4 c
	{ "findEmpty",		LB::b_findEmpty,	1, 1, true,  400, FBLTIN },	//			D4 f
		// go															// D2
	{ "importFileInto",	LB::b_importFileInto,2, 2, false, 400, CBLTIN }, //			D4 c
	{ "installMenu",	LB::b_installMenu,	1, 1, false, 200, CBLTIN },	// D2 c
	{ "label",			LB::b_label,		1, 1, true,  200, FBLTIN },	// D2 f
	{ "marker",			LB::b_marker,		1, 1, true,  200, FBLTIN },	// D2 f
	{ "move",			LB::b_move,			1, 2, false, 400, CBLTIN },	//			D4 c
	{ "moveableSprite",	LB::b_moveableSprite,0, 0, false, 200, CBLTIN },// D2, FIXME: the field in D4+
	{ "pasteClipBoardInto",LB::b_pasteClipBoardInto,1,1,false,400,CBLTIN },//		D4 c
	{ "puppetPalette",	LB::b_puppetPalette, -1,0, false, 200, CBLTIN },// D2 c
	{ "puppetSound",	LB::b_puppetSound,	-1,0, false, 200, CBLTIN },	// D2 c
	{ "puppetSprite",	LB::b_puppetSprite,	-1,0, false, 200, CBLTIN },	// D2 c
	{ "puppetTempo",	LB::b_puppetTempo,	1, 1, false, 200, CBLTIN },	// D2 c
	{ "puppetTransition",LB::b_puppetTransition,-1,0,false,200, CBLTIN },// D2 c
	{ "ramNeeded",		LB::b_ramNeeded,	2, 2, true,  300, FBLTIN },	//		D3.1 f
	{ "rollOver",		LB::b_rollOver,		1, 1, true,  299, FBLTIN },	// D2 f
	{ "spriteBox",		LB::b_spriteBox,	-1,0, false, 200, CBLTIN },	// D2 c
	{ "unLoad",			LB::b_unLoad,		0, 2, false, 300, CBLTIN },	//		D3.1 c
	{ "unLoadCast",		LB::b_unLoadCast,	0, 2, false, 300, CBLTIN },	//		D3.1 c
	{ "updateStage",	LB::b_updateStage,	0, 0, false, 200, CBLTIN },	// D2 c
	{ "zoomBox",		LB::b_zoomBox,		-1,0, false, 200, CBLTIN },	// D2 c
	// Point
	{ "point",			LB::b_point,		2, 2, true,  400, FBLTIN },	//			D4 f
	{ "inside",			LB::b_inside,		2, 2, true,  400, FBLTIN },	//			D4 f
	{ "intersect",		LB::b_intersect,	2, 2, false, 400, FBLTIN },	//			D4 f
	{ "map",			LB::b_map,			3, 3, true,  400, FBLTIN },	//			D4 f
	{ "rect",			LB::b_rect,			4, 4, true,  400, FBLTIN },	//			D4 f
	{ "union",			LB::b_union,		2, 2, true,  400, FBLTIN },	//			D4 f
	// Sound
	{ "beep",	 		LB::b_beep,			0, 1, false, 200, CBLTIN },	// D2
	{ "mci",	 		LB::b_mci,			1, 1, false, 300, CBLTIN },	//		D3.1 c
	{ "mciwait",		LB::b_mciwait,		1, 1, false, 400, CBLTIN },	//			D4 c
	{ "sound",			LB::b_sound,		2, 3, false, 300, CBLTIN },	//		D3 c
	{ "soundBusy",		LB::b_soundBusy,	1, 1, true,  300, FBLTIN },	//		D3 f
	// Constants
	{ "backspace",		LB::b_backspace,	0, 0, false, 200, KBLTIN },	// D2 konstant
	{ "empty",			LB::b_empty,		0, 0, false, 200, KBLTIN },	// D2 k
	{ "enter",			LB::b_enter,		0, 0, false, 200, KBLTIN },	// D2 k
	{ "false",			LB::b_false,		0, 0, false, 200, KBLTIN },	// D2 k
	{ "quote",			LB::b_quote,		0, 0, false, 200, KBLTIN },	// D2 k
	{ "return",			LB::b_returnconst,	0, 0, false, 200, KBLTIN },	// D2 k
	{ "tab",			LB::b_tab,			0, 0, false, 200, KBLTIN },	// D2 k
	{ "true",			LB::b_true,			0, 0, false, 200, KBLTIN },	// D2 k
	{ "version",		LB::b_version,		0, 0, false, 300, KBLTIN },	//		D3 k
	// References
	{ "cast",			LB::b_cast,			1, 1, false, 400, FBLTIN },	//			D4 f
	{ "field",			LB::b_field,		1, 1, false, 300, FBLTIN },	//		D3 f
	{ "script",			LB::b_script,		1, 1, false, 400, FBLTIN },	//			D4 f
	{ "window",			LB::b_window,		1, 1, false, 400, FBLTIN },	//			D4 f
	// Chunk operations
	{ "numberOfChars",	LB::b_numberofchars,1, 1, false, 300, FBLTIN },	//			D3 f
	{ "numberOfItems",	LB::b_numberofitems,1, 1, false, 300, FBLTIN },	//			D3 f
	{ "numberOfLines",	LB::b_numberoflines,1, 1, false, 300, FBLTIN },	//			D3 f
	{ "numberOfWords",	LB::b_numberofwords,1, 1, false, 300, FBLTIN },	//			D3 f
	{ "lastCharOf",		LB::b_lastcharof,	1, 1, false, 400, FBLTIN },	//			D4 f
	{ "lastItemOf",		LB::b_lastitemof,	1, 1, false, 400, FBLTIN },	//			D4 f
	{ "lastLineOf",		LB::b_lastlineof,	1, 1, false, 400, FBLTIN },	//			D4 f
	{ "lastWordOf",		LB::b_lastwordof,	1, 1, false, 400, FBLTIN },	//			D4 f

	// ScummVM Asserts: Used for testing ScummVM's Lingo implementation
	{ "scummvmAssert",	LB::b_scummvmassert,1, 2, true,  200, HBLTIN },
	{ "scummvmAssertEqual",	LB::b_scummvmassertequal,2,3,true,200,HBLTIN },

	// XCOD/XFCN (HyperCard), normally exposed
	{ "GetVolumes", LB::b_getVolumes, 0, 0, true, 400, FBLTIN },

	{ 0, 0, 0, 0, false, 0, VOIDSYM }
};

void Lingo::initBuiltIns() {
	for (BuiltinProto *blt = builtins; blt->name; blt++) {
		if (blt->version > _vm->getVersion())
			continue;

		Symbol sym;

		sym.name = new Common::String(blt->name);
		sym.type = blt->type;
		sym.nargs = blt->minArgs;
		sym.maxArgs = blt->maxArgs;
		sym.parens = blt->parens;
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
	if (_stack[_stack.size() - nargs + arg].type == VOIDSYM) {
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

	if (g_director->getVersion() < 500) {	// Note that D4 behaves differently from asInt()
		res = (int)(d.u.f + 0.5);		// Yes, +0.5 even for negative numbers
	} else {
		res = (int)round(d.u.f);
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

	Common::String src = s.asString();

	int len = strlen(src.c_str());
	int f = MAX(0, MIN(len, from - 1));
	int t = MAX(0, MIN(len, to));

	Common::String result;
	if (f > t) {
		result = Common::String("");
	} else {
		result = Common::String(&(src.c_str()[f]), &(src.c_str()[t]));
	}

	Datum res(result);
	g_lingo->push(res);
}

void LB::b_charToNum(int nargs) {
	Datum d = g_lingo->pop();

	TYPECHECK(d, STRING);

	int chr = (uint8)d.u.s->c_str()[0];

	Datum res(chr);
	g_lingo->push(res);
}

void LB::b_delete(int nargs) {
	Datum d = g_lingo->pop();

	Datum res(d.asInt());

	warning("STUB: b_delete");

	g_lingo->push(res);
}

void LB::b_hilite(int nargs) {
	Datum d = g_lingo->pop();

	Datum res(d.asInt());

	warning("STUB: b_hilite");

	g_lingo->push(res);
}

void LB::b_length(int nargs) {
	Datum d = g_lingo->pop();
	TYPECHECK(d, STRING);

	int len = strlen(d.asString().c_str());

	Datum res(len);
	g_lingo->push(res);
}

void LB::b_numToChar(int nargs) {
	Datum d = g_lingo->pop();

	char result[2];
	result[0] = (char)d.asInt();
	result[1] = 0;

	g_lingo->push(Datum(Common::String(result)));
}

void LB::b_offset(int nargs) {
	if (nargs == 3) {
		b_offsetRect(nargs);
		return;
	}
	Common::String target = g_lingo->pop().asString();
	Common::String source = g_lingo->pop().asString();

	warning("STUB: b_offset()");

	g_lingo->push(Datum(0));
}

void LB::b_string(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(d.asString());
	g_lingo->push(res);
}

void LB::b_value(int nargs) {
	Datum d = g_lingo->pop();
	Common::String expr = d.asString();
	if (expr.empty()) {
		g_lingo->push(Datum(0));
		return;
	}
	Common::String code = "scummvm_returnNumber " + expr;
	// Compile the code to an anonymous function and call it
	ScriptContext *sc = g_lingo->compileAnonymous(code.c_str());
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
	ARGNUMCHECK(3);

	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK2(indexD, INT, FLOAT);
	int index = indexD.asInt();
	TYPECHECK(list, ARRAY);

	int size = list.u.farr->size();
	if (index > size) {
		for (int i = 0; i < index - size - 1; i++)
			list.u.farr->push_back(Datum(0));
	}
	list.u.farr->insert_at(index - 1, value);
}

void LB::b_addProp(int nargs) {
	ARGNUMCHECK(3);

	Datum value = g_lingo->pop();
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK(list, PARRAY);

	PCell cell = PCell(prop, value);
	list.u.parr->push_back(cell);
}

void LB::b_append(int nargs) {
	ARGNUMCHECK(2);

	Datum value = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK(list, ARRAY);

	list.u.farr->push_back(value);
}

void LB::b_count(int nargs) {
	ARGNUMCHECK(1);

	Datum list = g_lingo->pop();
	Datum result;
	result.type = INT;

	switch (list.type) {
	case ARRAY:
		result.u.i = list.u.farr->size();
		break;
	case PARRAY:
		result.u.i = list.u.parr->size();
		break;
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}

	g_lingo->push(result);
}

void LB::b_deleteAt(int nargs) {
	ARGNUMCHECK(2);

	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	TYPECHECK2(list, ARRAY, PARRAY);
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
		list.u.farr->remove_at(index - 1);
		break;
	case PARRAY:
		list.u.parr->remove_at(index - 1);
		break;
	default:
		break;
	}
}

void LB::b_deleteProp(int nargs) {
	ARGNUMCHECK(2);

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
			list.u.parr->remove_at(index - 1);
		}
		break;
	}
	default:
		break;
	}
}

void LB::b_findPos(int nargs) {
	ARGNUMCHECK(2);

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
	ARGNUMCHECK(2);

	Common::String prop = g_lingo->pop().asString();
	Datum list = g_lingo->pop();
	Datum res(0);
	TYPECHECK(list, PARRAY);

	// FIXME: Integrate with compareTo framework
	prop.toLowercase();

	for (uint i = 0; i < list.u.parr->size(); i++) {
		Datum p = list.u.parr->operator[](i).p;
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
	ARGNUMCHECK(2);
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
			d = list.u.parr->operator[](index - 1).v;
		}
		g_lingo->push(d);
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_getAt(int nargs) {
	ARGNUMCHECK(2);

	Datum indexD = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	Datum list = g_lingo->pop();
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
		ARRBOUNDSCHECK(index, list);
		g_lingo->push(list.u.farr->operator[](index - 1));
		break;
	case PARRAY:
		ARRBOUNDSCHECK(index, list);
		g_lingo->push(list.u.parr->operator[](index - 1).v);
		break;
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_getLast(int nargs) {
	ARGNUMCHECK(1);

	Datum list = g_lingo->pop();
	switch (list.type) {
	case ARRAY:
		g_lingo->push(list.u.farr->back());
		break;
	case PARRAY:
		g_lingo->push(list.u.parr->back().v);
		break;
	default:
		TYPECHECK(list, ARRAY);
	}
}

void LB::b_getOne(int nargs) {
	ARGNUMCHECK(2);
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
			d = list.u.parr->operator[](index - 1).p;
		}
		g_lingo->push(d);
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_getPos(int nargs) {
	ARGNUMCHECK(2);
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
	ARGNUMCHECK(2);
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
			g_lingo->push(list.u.parr->operator[](index - 1).v);
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
	ARGNUMCHECK(2);

	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	TYPECHECK(list, PARRAY);
	int index = indexD.asInt();

	g_lingo->push(list.u.parr->operator[](index - 1).p);
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
	ARGNUMCHECK(1);
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
			uint arrsize = d.u.farr->size();
			for (uint i = 0; i < arrsize; i++) {
				Datum item = d.u.farr->operator[](i);
				if (i == 0 || item.compareTo(max) > 0) {
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
			if (i == 0 || d.compareTo(max) > 0) {
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
			uint arrsize = d.u.farr->size();
			for (uint i = 0; i < arrsize; i++) {
				Datum item = d.u.farr->operator[](i);
				if (i == 0 || item.compareTo(min) < 0) {
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
			if (i == 0 || d.compareTo(min) < 0) {
				min = d;
			}
		}
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(min);
}

void LB::b_setaProp(int nargs) {
	ARGNUMCHECK(3);
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
			list.u.parr->operator[](index - 1).v = value;
		} else {
			PCell cell = PCell(prop, value);
			list.u.parr->push_back(cell);
		}
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_setAt(int nargs) {
	ARGNUMCHECK(3);
	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK2(indexD, INT, FLOAT);
	TYPECHECK2(list, ARRAY, PARRAY);
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
		if ((uint)index <= list.u.farr->size()) {
			list.u.farr->operator[](index - 1) = value;
		} else {
			int inserts = index - list.u.farr->size();
			while (--inserts)
				list.u.farr->push_back(Datum(0));
			list.u.farr->push_back(value);
		}
		break;
	case PARRAY:
		ARRBOUNDSCHECK(index, list);
		list.u.parr->operator[](index - 1).v = value;
		break;
	default:
		break;
	}
}

void LB::b_setProp(int nargs) {
	ARGNUMCHECK(3);
	Datum value = g_lingo->pop();
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK(list, PARRAY);

	int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
	if (index > 0) {
		list.u.parr->operator[](index - 1).v = value;
	} else {
		warning("b_setProp: Property not found");
	}
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
	if (nargs == 0) { // Close all res files
		g_director->_openResFiles.clear();
		return;
	}
	Datum d = g_lingo->pop();
	Common::String resFileName = g_director->getCurrentWindow()->getCurrentPath() + d.asString();

	g_director->_openResFiles.erase(resFileName);
}

void LB::b_closeXlib(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_closeXlib(%s)", d.asString().c_str());
}

void LB::b_getNthFileNameInFolder(int nargs) {
	ARGNUMCHECK(2);

	int fileNum = g_lingo->pop().asInt() - 1;
	Common::String path = pathMakeRelative(g_lingo->pop().asString(), true, false, true);
	Common::FSNode d = Common::FSNode(*g_director->getGameDataDir()).getChild(path);

	Datum r;
	if (d.exists()) {
		Common::FSList f;
		if (!d.getChildren(f, Common::FSNode::kListAll)) {
			warning("Cannot acces directory %s", path.c_str());
		} else {
			if ((uint)fileNum < f.size())
				r = Datum(f.operator[](fileNum).getName());
		}
	}

	g_lingo->push(r);
}

void LB::b_open(int nargs) {
	g_lingo->printSTUBWithArglist("b_open", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_openDA(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_openDA(%s)", d.asString().c_str());
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

	Datum d = g_lingo->pop();
	Common::String xlibName = d.asString();
	g_lingo->openXLib(xlibName, kXObj);
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

	warning("STUB: b_showResFile(%s)", d.asString().c_str());
}

void LB::b_showXlib(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_showXlib(%s)", d.asString().c_str());
}

void LB::b_xFactoryList(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_xFactoryList(%s)", d.asString().c_str());
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
	Datum d = g_lingo->pop();
	warning("STUB: b_do(%s)", d.asString().c_str());
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
		if (!(frame.type == INT && frame.u.i == 0) && nargs == 1)
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

	g_lingo->_theResult = g_lingo->pop();

	if (nargs == 2)
		g_lingo->pop();
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
	if (g_director->getCurrentMovie())
		g_director->getCurrentMovie()->getScore()->_playState = kPlayStopped;

	g_lingo->pushVoid();
}

void LB::b_return(int nargs) {
	CFrame *fp = g_lingo->_callstack.back();

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

void LB::b_returnNumber(int nargs) {
	Datum d = g_lingo->pop();
	// Only return numeric values
	if (d.type == INT || d.type == FLOAT)
		g_lingo->push(d);
	else
		g_lingo->push(Datum());
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
	factoryName.type = VAR;
	Datum o = g_lingo->varFetch(factoryName, true);
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
	ARGNUMCHECK(1);
	Datum d = g_lingo->pop();

	Common::String alert = d.asString();
	warning("b_alert(%s)", alert.c_str());

	if (!debugChannelSet(-1, kDebugFewFramesOnly)) {
		GUI::MessageDialog dialog(alert.c_str(), "OK");
		dialog.runModal();
	}
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

		g_lingo->func_cursor(sprite.asCastId(), mask.asCastId());
	} else {
		g_lingo->func_cursor(d.asInt(), -1);
	}
}

void LB::b_put(int nargs) {
	// Prints a statement to the Message window
	Common::String output;
	for (int i = nargs - 1; i >= 0; i--) {
		output += g_lingo->peek(i).asString(true);
		if (i > 0)
			output += " ";
	}
	debug("-- %s", output.c_str());
	g_lingo->dropStack(nargs);
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

	warning("STUB: b_constrainH(%d, %d)", sprite.asInt(), num.asInt());

	g_lingo->push(Datum(0));
}

void LB::b_constrainV(int nargs) {
	Datum num = g_lingo->pop();
	Datum sprite = g_lingo->pop();

	warning("STUB: b_constrainV(%d, %d)", sprite.asInt(), num.asInt());

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
	} else {
		warning("b_editableText: unexpectedly received %d arguments", nargs);
		g_lingo->dropStack(nargs);
	}
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
	g_director->getCurrentMovie()->registerEvent(kEventMenuCallback, action);
}

void LB::b_installMenu(int nargs) {
	// installMenu castNum
	Datum d = g_lingo->pop();

	int castId = d.asCastId();

	if (castId == 0) {
		g_director->_wm->removeMenu();
		return;
	}

	CastMember *member = g_director->getCurrentMovie()->getCastMember(castId);
	if (!member) {
		g_lingo->lingoError("installMenu: Unknown cast number #%d", castId);
		return;
	}
	if (member->_type != kCastText) {
		g_lingo->lingoError("installMenu: Cast member %d is not a field", castId);
		return;
	}
	TextCastMember *field = static_cast<TextCastMember *>(member);

	Common::String menuStxt = g_lingo->codePreprocessor(field->getText().c_str(), field->getCast()->_lingoArchive, kNoneScript, castId, true);
	Common::String line;
	int linenum = -1; // We increment it before processing

	Graphics::MacMenu *menu = g_director->_wm->addMenu();
	int submenu = -1;
	Common::String submenuText;
	Common::String command;
	int commandId = 100;

	menu->setCommandsCallback(menuCommandsCallback, g_director);

	debugC(3, kDebugLingoExec, "installMenu: '%s'", Common::toPrintable(menuStxt).c_str());

	LingoArchive *mainArchive = g_director->getCurrentMovie()->getMainLingoArch();

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
				menu->createSubMenuFromString(submenu, submenuText.c_str(), 0);
			}

			if (!strcmp(p, "@"))
				p = "\xf0";	// Apple symbol

			submenu = menu->addMenuItem(nullptr, Common::String(p));

			submenuText.clear();

			continue;
		}

		// We have \xc5 as a separator
		const char *p = strchr(line.c_str(), '\xc5');

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

		submenuText += text;

		if (!submenuText.empty()) {
			if (!command.empty()) {
				while (mainArchive->getScriptContext(kEventScript, commandId)) {
					commandId++;
				}
				mainArchive->addCode(command.c_str(), kEventScript, commandId);
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
	Datum d = g_lingo->pop();
	int marker = g_lingo->func_marker(d.asInt());
	g_lingo->push(marker);
}

void LB::b_move(int nargs) {
	g_lingo->printSTUBWithArglist("b_move", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_moveableSprite(int nargs) {
	Frame *frame = g_director->getCurrentMovie()->getScore()->_frames[g_director->getCurrentMovie()->getScore()->getCurrentFrame()];

	if (g_lingo->_currentChannelId == -1) {
		warning("b_moveableSprite: channel Id is missing");
		assert(0);
		return;
	}

	frame->_sprites[g_lingo->_currentChannelId]->_moveable = true;
}

void LB::b_pasteClipBoardInto(int nargs) {
	g_lingo->printSTUBWithArglist("b_pasteClipBoardInto", nargs);

	g_lingo->dropStack(nargs);
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
			} else {
				CastMember *member = g_director->getCurrentMovie()->getCastMemberByName(palStr);

				if (member && member->_type == kCastPalette)
					palette = ((PaletteCastMember *)member)->getPaletteId();
			}
		} else {
			CastMember *member = g_director->getCurrentMovie()->getCastMember(d.asInt());

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
		g_director->setPalette(g_director->getCurrentMovie()->getScore()->_lastPalette);
		g_director->getCurrentMovie()->getScore()->_puppetPalette = false;
	}

	// TODO: Implement advanced features that use these.
	if (numFrames || speed)
		warning("b_puppetPalette: Skipping extra features");
}

void LB::b_puppetSound(int nargs) {
	ARGNUMCHECK(1);

	DirectorSound *sound = g_director->getSoundManager();
	Datum castMember = g_lingo->pop();
	Score *score = g_director->getCurrentMovie()->getScore();

	if (!score) {
		warning("b_puppetSound(): no score");
		return;
	}

	int castId = castMember.asCastId();
	sound->playCastMember(castId, 1);
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
	ARGNUMCHECK(5);

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
	ARGNUMCHECK(0);

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
	if (movie->_videoPlayback) {
		movie->getScore()->renderFrame(movie->getScore()->getCurrentFrame(), kRenderNoWindowRender);
	}

	if (movie->getWindow()->render())
		g_director->draw();

	if (debugChannelSet(-1, kDebugFewFramesOnly)) {
		score->_framesRan++;

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

	if (verb.u.s->equalsIgnoreCase("close") || verb.u.s->equalsIgnoreCase("stop")) {
		if (nargs != 2) {
			warning("sound %s: expected 1 argument, got %d", verb.u.s->c_str(), nargs - 1);
			return;
		}

		TYPECHECK(firstArg, INT);
		g_director->getSoundManager()->stopSound(firstArg.u.i);
	} else if (verb.u.s->equalsIgnoreCase("fadeIn")) {
		if (nargs > 2) {
			TYPECHECK(secondArg, INT);
			ticks = secondArg.u.i;
		} else {
			ticks = 15 * (60 / g_director->getCurrentMovie()->getScore()->_currentFrameRate);
		}

		TYPECHECK(firstArg, INT);
		g_director->getSoundManager()->registerFade(firstArg.u.i, true, ticks);
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
		g_director->getSoundManager()->registerFade(firstArg.u.i, false, ticks);
		g_director->getCurrentMovie()->getScore()->_activeFade = firstArg.u.i;
		return;
	} else if (verb.u.s->equalsIgnoreCase("playFile")) {
		ARGNUMCHECK(3)

		TYPECHECK(firstArg, INT);
		TYPECHECK(secondArg, STRING);

		g_director->getSoundManager()->playFile(pathMakeRelative(*secondArg.u.s), firstArg.u.i);
	} else {
		warning("b_sound: unknown verb %s", verb.u.s->c_str());
	}
}

void LB::b_soundBusy(int nargs) {
	ARGNUMCHECK(1);

	DirectorSound *sound = g_director->getSoundManager();
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
	g_lingo->push(Datum(Common::String("\n")));
}

void LB::b_false(int nargs) {
	g_lingo->push(Datum(0));
}

void LB::b_quote(int nargs) {
	g_lingo->push(Datum(Common::String("\"")));
}

void LB::b_returnconst(int nargs) {
	g_lingo->push(Datum(Common::String("\n")));
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
	Datum res = d.asCastId();
	res.type = CASTREF;
	g_lingo->push(res);
}

void LB::b_field(int nargs) {
	Datum d = g_lingo->pop();
	Datum res = d.asCastId();
	res.type = FIELDREF;
	g_lingo->push(res);
}

void LB::b_script(int nargs) {
	Datum d = g_lingo->pop();
	int castId = d.asCastId();
	CastMember *cast = g_director->getCurrentMovie()->getCastMember(castId);

	if (cast) {
		ScriptContext *script = nullptr;

		if (cast->_type == kCastLingoScript) {
			// script cast can be either a movie script or score script
			script = g_director->getCurrentMovie()->getScriptContext(kMovieScript, castId);
			if (!script)
				script = g_director->getCurrentMovie()->getScriptContext(kScoreScript, castId);
		} else {
			g_director->getCurrentMovie()->getScriptContext(kCastScript, castId);
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
	DatumArray *windowList = g_lingo->_windowList.u.farr;

	for (uint i = 0; i < windowList->size(); i++) {
		if ((*windowList)[i].type != OBJECT || (*windowList)[i].u.obj->getObjType() != kWindowObj)
			continue;

		Window *window = static_cast<Window *>((*windowList)[i].u.obj);
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
	windowList->push_back(window);
	g_lingo->push(window);
}

void LB::b_numberofchars(int nargs) {
	Datum d = g_lingo->pop();

	int len = strlen(d.asString().c_str());

	Datum res(len);
	g_lingo->push(res);
}

void LB::b_numberofitems(int nargs) {
	Datum d = g_lingo->pop();

	int numberofitems = 1;
	Common::String contents = d.asString();
	for (uint32 i = 0;  i < contents.size(); i++) {
		if (contents[i] == g_lingo->_itemDelimiter)
			numberofitems++;
	}

	Datum res(numberofitems);
	g_lingo->push(res);
}

void LB::b_numberoflines(int nargs) {
	Datum d = g_lingo->pop();

	int numberoflines = 1;
	Common::String contents = d.asString();
	for (uint32 i = 0; i < contents.size(); i++) {
		if (contents[i] == '\n')
			numberoflines++;
	}

	Datum res(numberoflines);
	g_lingo->push(res);
}

void LB::b_numberofwords(int nargs) {
	Datum d = g_lingo->pop();

	int numberofwords = 0;
	Common::String contents = d.asString();
	if (contents.empty()) {
		g_lingo->push(Datum(0));
		return;
	}
	for (uint32 i = 1; i < contents.size(); i++) {
		if (Common::isSpace(contents[i]) && !Common::isSpace(contents[i - 1]))
			numberofwords++;
	}
	// Count the last word
	if (!Common::isSpace(contents[contents.size() - 1]))
		numberofwords++;

	Datum res(numberofwords);
	g_lingo->push(res);
}

void LB::b_lastcharof(int nargs) {
	Datum d = g_lingo->pop();
	if (d.type != STRING) {
		warning("LB::b_lastcharof(): Called with wrong data type: %s", d.type2str());
		g_lingo->push(Datum(""));
		return;
	}

	Common::String contents = d.asString();
	Common::String res;
	if (contents.size() != 0)
		res = contents.lastChar();
	g_lingo->push(Datum(res));
}

void LB::b_lastitemof(int nargs) {
	Datum d = g_lingo->pop();
	if (d.type != STRING) {
		warning("LB::b_lastitemof(): Called with wrong data type: %s", d.type2str());
		g_lingo->push(Datum(""));
		return;
	}

	Common::String res;
	Common::String chunkExpr = d.asString();
	uint pos = chunkExpr.findLastOf(g_lingo->_itemDelimiter);
	if (pos == Common::String::npos) {
		res = chunkExpr;
	} else {
		pos++; // skip the item delimiter
		res = chunkExpr.substr(pos , chunkExpr.size() - pos);
	}

	g_lingo->push(Datum(res));
}

void LB::b_lastlineof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastlineof");

	g_lingo->push(Datum(0));
}

void LB::b_lastwordof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastwordof");

	g_lingo->push(Datum(0));
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

	int result = d1.equalTo(d2);
	if (!result) {
		warning("LB::b_scummvmassertequals: %s is not equal %s at line %d", d1.asString().c_str(), d2.asString().c_str(), line.asInt());
	}
	assert(result == 1);
}

void LB::b_getVolumes(int nargs) {
	ARGNUMCHECK(0);

	// Right now, only "Journeyman Project 2: Buried in Time" is known to check
	// for its volume name.
	Datum d;
	d.type = ARRAY;
	d.u.farr = new DatumArray;
	d.u.farr->push_back(Datum("Buried in Time\252 1"));

	g_lingo->push(d);
}

} // End of namespace Director
