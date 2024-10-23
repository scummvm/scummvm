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

// Heavily inspired by hoc
// Copyright (C) AT&T 1995
// All Rights Reserved
//
// Permission to use, copy, modify, and distribute this software and
// its documentation for any purpose and without fee is hereby
// granted, provided that the above copyright notice appear in all
// copies and that both that the copyright notice and this
// permission notice and warranty disclaimer appear in supporting
// documentation, and that the name of AT&T or any of its entities
// not be used in advertising or publicity pertaining to
// distribution of the software without specific, written prior
// permission.
//
// AT&T DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
// INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL AT&T OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
// SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
// IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
// ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
// THIS SOFTWARE.

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/mactext.h"

#include "director/director.h"
#include "director/debugger.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/channel.h"
#include "director/castmember/castmember.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-the.h"

namespace Director {

static struct FuncDescr {
	const inst func;
	const char *name;
	const char *args;
} funcDescr[] = {
	{ nullptr,					"STOP",				""  },
	{ LC::c_asserterror,	"c_asserterror",	"" },
	{ LC::c_asserterrordone,"c_asserterrordone","" },
	{ LC::c_add,			"c_add",			"" },
	{ LC::c_ampersand,		"c_ampersand",		"" },
	{ LC::c_and,			"c_and",			"" },
	{ LC::c_argcnoretpush,	"c_argcnoretpush",	"i" },
	{ LC::c_argcpush,		"c_argcpush",		"i" },
	{ LC::c_arraypush,		"c_arraypush",		"i" },
	{ LC::c_assign,			"c_assign",			""  },
	{ LC::c_callcmd,		"c_callcmd",		"si" },
	{ LC::c_callfunc,		"c_callfunc",		"si" },
	{ LC::c_charToOf,		"c_charToOf",		"" },	// D3
	{ LC::c_charToOfRef,	"c_charToOfRef",	"" },	// D3
	{ LC::c_concat,			"c_concat",			"" },
	{ LC::c_constpush,		"c_constpush",		"s" },
	{ LC::c_contains,		"c_contains",		"" },
	{ LC::c_delete,			"c_delete", 		"" },
	{ LC::c_div,			"c_div",			"" },
	{ LC::c_eq,				"c_eq",				"" },
	{ LC::c_field,			"c_field",			"" },
	{ LC::c_fieldref,		"c_fieldref",		"" },
	{ LC::c_floatpush,		"c_floatpush",		"f" },
	{ LC::c_globalinit,		"c_globalinit",		"s" },
	{ LC::c_globalpush,		"c_globalpush",		"s" },
	{ LC::c_globalrefpush,	"c_globalrefpush",	"s" },
	{ LC::c_ge,				"c_ge",				"" },
	{ LC::c_gt,				"c_gt",				"" },
	{ LC::c_hilite,			"c_hilite",			"" },
	{ LC::c_intersects,		"c_intersects",		"" },
	{ LC::c_intpush,		"c_intpush",		"i" },
	{ LC::c_itemToOf,		"c_itemToOf",		"" },	// D3
	{ LC::c_itemToOfRef,	"c_itemToOfRef",	"" },	// D3
	{ LC::c_jump,			"c_jump",			"o" },
	{ LC::c_jumpifz,		"c_jumpifz",		"o" },
	{ LC::c_le,				"c_le",				"" },
	{ LC::c_lineToOf,		"c_lineToOf",		"" },	// D3
	{ LC::c_lineToOfRef,	"c_lineToOfRef",	"" },	// D3
	{ LC::c_localpush,		"c_localpush",		"s" },
	{ LC::c_localrefpush,	"c_localrefpush",	"s" },
	{ LC::c_lt,				"c_lt",				"" },
	{ LC::c_mod,			"c_mod",			"" },
	{ LC::c_mul,			"c_mul",			"" },
	{ LC::c_namepush,		"c_namepush",		"s" },
	{ LC::c_negate,			"c_negate",			"" },
	{ LC::c_neq,			"c_neq",			"" },
	{ LC::c_not,			"c_not",			"" },
	{ LC::c_objectpropassign,"c_objectpropassign","s" }, // prop
	{ LC::c_objectproppush,	"c_objectproppush","s" }, // prop
	{ LC::c_of,				"c_of",				"" },
	{ LC::c_or,				"c_or",				"" },
	{ LC::c_procret,		"c_procret",		"" },
	{ LC::c_proparraypush,	"c_proparraypush",	"i" },
	{ LC::c_proppush,		"c_proppush",		"s" },
	{ LC::c_proprefpush,	"c_proprefpush",	"s" },
	{ LC::c_putafter,		"c_putafter",		"" },	// D3
	{ LC::c_putbefore,		"c_putbefore",		"" },	// D3
	{ LC::c_starts,			"c_starts",			"" },
	{ LC::c_stringpush,		"c_stringpush",		"s" },
	{ LC::c_sub,			"c_sub",			"" },
	{ LC::c_swap,			"c_swap",			"" },
	{ LC::c_symbolpush,		"c_symbolpush",		"s" },	// D3
	{ LC::c_tell,			"c_tell",			"" },
	{ LC::c_telldone,		"c_telldone",		"" },
	{ LC::c_theentityassign,"c_theentityassign","EF" },
	{ LC::c_theentitypush,	"c_theentitypush",	"EF" }, // entity, field
	{ LC::c_themenuentitypush,"c_themenuentitypush","EF" },
	{ LC::c_varpush,		"c_varpush",		"s" },
	{ LC::c_varrefpush,		"c_varrefpush",		"s" },
	{ LC::c_voidpush,		"c_voidpush",		""  },
	{ LC::c_whencode,		"c_whencode",		"s" },
	{ LC::c_within,			"c_within",			"" },
	{ LC::c_wordToOf,		"c_wordToOf",		"" },	// D3
	{ LC::c_wordToOfRef,	"c_wordToOfRef",	"" },	// D3
	{ LC::c_xpop,			"c_xpop",			""  },
	{ LC::cb_call,			"cb_call",			"s" },
	{ LC::cb_delete,		"cb_delete",		"i" },
	{ LC::cb_hilite,		"cb_hilite",		"" },
	{ LC::cb_globalassign,	"cb_globalassign",	"s" },
	{ LC::cb_globalpush,	"cb_globalpush",	"s" },
	{ LC::cb_list,			"cb_list",			"" },
	{ LC::cb_proplist,		"cb_proplist",		"" },
	{ LC::cb_localcall,		"cb_localcall",		"i" },
	{ LC::cb_objectcall,	"cb_objectcall",	"i" },
	{ LC::cb_objectfieldassign, "cb_objectfieldassign", "s" },
	{ LC::cb_objectfieldpush, "cb_objectfieldpush", "s" },
	{ LC::cb_varrefpush,	"cb_varrefpush",	"s" },
	{ LC::cb_theassign,		"cb_theassign",		"s" },
	{ LC::cb_theassign2,	"cb_theassign2",	"s" },
	{ LC::cb_thepush,		"cb_thepush",		"s" },
	{ LC::cb_thepush2,		"cb_thepush2",		"s" },
	{ LC::cb_unk,			"cb_unk",			"i" },
	{ LC::cb_unk1,			"cb_unk1",			"ii" },
	{ LC::cb_unk2,			"cb_unk2",			"iii" },
	{ LC::cb_varassign,		"cb_varassign",		"s" },
	{ LC::cb_varpush,		"cb_varpush",		"s" },
	{ LC::cb_v4assign,		"cb_v4assign",		"i" },
	{ LC::cb_v4assign2,		"cb_v4assign2",		"i" },
	{ LC::cb_v4theentitypush,"cb_v4theentitypush","i" },
	{ LC::cb_v4theentitynamepush,"cb_v4theentitynamepush","s" },
	{ LC::cb_v4theentityassign,"cb_v4theentityassign","i" },
	{ LC::cb_zeropush,		"cb_zeropush",		"" },
	{ LC::c_stackpeek,		"c_stackpeek",		"i" },
	{ LC::c_stackdrop,		"c_stackdrop",		"i" },
	{ nullptr, nullptr, nullptr }
};

void Lingo::initFuncs() {
	Symbol sym;
	for (FuncDescr *fnc = funcDescr; fnc->name; fnc++) {
		sym.u.func = fnc->func;
		_functions[(void *)sym.u.s] = new FuncDesc(fnc->name, fnc->args);
	}
}

void Lingo::cleanupFuncs() {
	for (auto &it : _functions)
		delete it._value;
}

void Lingo::push(Datum d) {
	_stack.push_back(d);
}

Datum Lingo::getVoid() {
	Datum d;
	d.u.s = nullptr;
	d.type = VOID;
	return d;
}

void Lingo::pushVoid() {
	Datum d = getVoid();
	push(d);
}

Datum Lingo::pop() {
	assert (_stack.size() != 0);

	Datum ret = _stack.back();
	_stack.pop_back();

	return ret;
}

Datum Lingo::peek(uint offset) {
	assert (_stack.size() > offset);

	Datum ret = _stack[_stack.size() - 1 - offset];

	return ret;
}

void LC::c_xpop() {
	g_lingo->pop();
}

void Lingo::switchStateFromWindow() {
	Window *window = _vm->getCurrentWindow();
	_state = window->getLingoState();
}

void Lingo::pushContext(const Symbol funcSym, bool allowRetVal, Datum defaultRetVal, int paramCount, int nargs) {
	Common::Array<CFrame *> &callstack = _state->callstack;

	debugC(5, kDebugLingoExec, "Pushing frame %d", callstack.size() + 1);
	CFrame *fp = new CFrame;

	fp->retPC = _state->pc;
	fp->retScript = _state->script;
	fp->retContext = _state->context;
	fp->retLocalVars = _state->localVars;
	fp->retMe = _state->me;
	fp->sp = funcSym;
	fp->allowRetVal = allowRetVal;
	fp->defaultRetVal = defaultRetVal;
	fp->paramCount = paramCount;  // number of args, excluding nulls for missing named args
	for (int i = 0; i < nargs; i++) { // number of args on the stack
		fp->paramList.insert_at(0, pop());
	}

	_state->script = funcSym.u.defn;

	// Do not set the context for anonymous functions that are called from factory
	// ie something like b_do(), which is called from mNew() should have access to instance
	// variables, thus it is in same context as the caller.
	if (!(funcSym.anonymous && _state->me.type == OBJECT && _state->me.u.obj->getObjType() & (kFactoryObj | kScriptObj)))
		_state->me = funcSym.target;

	if (funcSym.ctx) {
		_state->context = funcSym.ctx;
		_state->context->incRefCount();
	}

	DatumHash *localvars = new DatumHash;
	if (funcSym.anonymous && _state->localVars) {
		// Execute anonymous functions within the current var frame.
		for (auto it = _state->localVars->begin(); it != _state->localVars->end(); ++it) {
			localvars->setVal(it->_key, it->_value);
		}
	}

	if (funcSym.argNames) {
		if (funcSym.argNames->size() > fp->paramList.size()) {
			debugC(1, kDebugLingoExec, "%d arg names defined for %d args! Ignoring the last %d names", funcSym.argNames->size(), fp->paramList.size(), funcSym.argNames->size() - fp->paramList.size());
		}
		for (int i = (int)funcSym.argNames->size() - 1; i >= 0; i--) {
			Common::String name = (*funcSym.argNames)[i];
			if (!localvars->contains(name)) {
				if (i < (int)fp->paramList.size()) {
					Datum value = fp->paramList[i];
					(*localvars)[name] = value;
				} else {
					(*localvars)[name] = Datum();
				}
			} else {
				warning("Argument %s already defined", name.c_str());
			}
		}
	}
	if (funcSym.varNames) {
		for (auto &it : *funcSym.varNames) {
			Common::String name = it;
			if (!localvars->contains(name)) {
				(*localvars)[name] = Datum();
			} else {
				warning("Variable %s already defined", name.c_str());
			}
		}
	}
	_state->localVars = localvars;

	fp->stackSizeBefore = _stack.size();

	callstack.push_back(fp);

	if (debugChannelSet(2, kDebugLingoExec)) {
		printCallStack(0);
	}
	_state->pc = 0;
	g_debugger->pushContextHook();
}

void Lingo::popContext(bool aborting) {
	Common::Array<CFrame *> &callstack = _state->callstack;

	debugC(5, kDebugLingoExec, "Popping frame %d", callstack.size());
	CFrame *fp = callstack.back();
	callstack.pop_back();

	if (_stack.size() == fp->stackSizeBefore + 1) {
		if (!fp->allowRetVal) {
			debugC(5, kDebugLingoExec, "dropping return value");
			pop();
		}
	} else if (_stack.size() == fp->stackSizeBefore) {
		if (fp->allowRetVal) {
			// Don't warn about missing return value if there's an explicit, non-VOID default,
			// e.g. for factories' mNew method.
			if (fp->defaultRetVal.type == VOID) {
				warning("handler %s did not return value", fp->sp.name->c_str());
			}
			push(fp->defaultRetVal);
		}
	} else if (_stack.size() > fp->stackSizeBefore) {
		if (aborting) {
			// Since we're aborting execution, we should expect that some extra
			// values are left on the stack.
			while (_stack.size() > fp->stackSizeBefore) {
				pop();
			}
		} else {
			error("handler %s returned extra %d values", fp->sp.name->c_str(), _stack.size() - fp->stackSizeBefore);
		}
	} else {
		error("handler %s popped extra %d values", fp->sp.name->c_str(), fp->stackSizeBefore - _stack.size());
	}

	_state->context->decRefCount();

	_state->script = fp->retScript;
	_state->context = fp->retContext;
	_state->pc = fp->retPC;
	_state->me = fp->retMe;

	// For anonymous functions, copy the local var state back to the parent
	if (fp->sp.anonymous && fp->retLocalVars) {
		for (auto it = _state->localVars->begin(); it != _state->localVars->end(); ++it) {
			fp->retLocalVars->setVal(it->_key, it->_value);
		}
	}
	cleanLocalVars();
	_state->localVars = fp->retLocalVars;

	if (debugChannelSet(2, kDebugLingoExec)) {
		printCallStack(_state->pc);
	}

	delete fp;

	g_debugger->popContextHook();
}

void Lingo::freezeState() {
	Window *window = _vm->getCurrentWindow();
	window->freezeLingoState();
	switchStateFromWindow();
}

void Lingo::freezePlayState() {
	Window *window = _vm->getCurrentWindow();
	window->freezeLingoPlayState();
	switchStateFromWindow();
}

void LC::c_constpush() {
	Common::String name(g_lingo->readString());

	Symbol funcSym;
	if (g_lingo->_builtinConsts.contains(name)) {
		funcSym = g_lingo->_builtinConsts[name];
	}

	LC::call(funcSym, 0, true);
}

void LC::c_intpush() {
	int value = g_lingo->readInt();
	g_lingo->push(Datum(value));
}

void LC::c_voidpush() {
	Datum d;
	d.u.s = nullptr;
	d.type = VOID;
	g_lingo->push(d);
}

void LC::c_floatpush() {
	double value = g_lingo->readFloat();
	g_lingo->push(Datum(value));
}

void LC::c_stringpush() {
	char *s = g_lingo->readString();
	g_lingo->push(Datum(Common::String(s)));
}

void LC::c_symbolpush() {
	char *s = g_lingo->readString();

	// TODO: FIXME: Currently we push string
	// If you change it, you must also fix func_play for "play done"
	// command
	Datum d = Datum(Common::String(s));
	d.type = SYMBOL;

	g_lingo->push(d);
}

void LC::c_namepush() {
	Datum d(g_lingo->readString());
	d.type = SYMBOL;
	g_lingo->push(d);
}

void LC::c_argcpush() {
	Datum d;
	int argsSize = g_lingo->readInt();

	d.u.i = argsSize;
	d.type = ARGC;
	g_lingo->push(d);
}

void LC::c_argcnoretpush() {
	Datum d;
	int argsSize = g_lingo->readInt();

	d.u.i = argsSize;
	d.type = ARGCNORET;
	g_lingo->push(d);
}

void LC::c_arraypush() {
	Datum d;
	int arraySize = g_lingo->readInt();

	d.type = ARRAY;
	d.u.farr = new FArray;

	for (int i = 0; i < arraySize; i++)
		d.u.farr->arr.insert_at(0, g_lingo->pop());

	g_lingo->push(d);
}

void LC::c_proparraypush() {
	Datum d;
	int arraySize = g_lingo->readInt();

	d.type = PARRAY;
	d.u.parr = new PArray;

	for (int i = 0; i < arraySize; i++) {
		Datum v = g_lingo->pop();
		Datum p = g_lingo->pop();

		PCell cell = PCell(p, v);
		d.u.parr->arr.insert_at(0, cell);
	}

	g_lingo->push(d);
}

void LC::c_globalinit() {
	Common::String name(g_lingo->readString());
	if (!g_lingo->_globalvars.contains(name) || g_lingo->_globalvars[name].type == VOID) {
		g_lingo->_globalvars[name] = Datum(0);
	}
}

void LC::c_varrefpush() {
	Common::String name(g_lingo->readString());
	Datum d(name);
	d.type = VARREF;
	g_lingo->push(d);
}

void LC::c_globalrefpush() {
	Common::String name(g_lingo->readString());
	Datum d(name);
	d.type = GLOBALREF;
	g_lingo->push(d);
}

void LC::c_localrefpush() {
	Common::String name(g_lingo->readString());
	Datum d(name);
	d.type = LOCALREF;
	g_lingo->push(d);
}

void LC::c_proprefpush() {
	Common::String name(g_lingo->readString());
	Datum d(name);
	d.type = PROPREF;
	g_lingo->push(d);
}

void LC::c_varpush() {
	LC::c_varrefpush();
	Datum d = g_lingo->pop();
	g_lingo->push(g_lingo->varFetch(d));
}

void LC::c_globalpush() {
	LC::c_globalrefpush();
	Datum d = g_lingo->pop();
	g_lingo->push(g_lingo->varFetch(d));
}

void LC::c_localpush() {
	LC::c_localrefpush();
	Datum d = g_lingo->pop();
	g_lingo->push(g_lingo->varFetch(d));
}

void LC::c_proppush() {
	LC::c_proprefpush();
	Datum d = g_lingo->pop();
	g_lingo->push(g_lingo->varFetch(d));
}

void LC::c_stackpeek() {
	int peekOffset = g_lingo->readInt();
	g_lingo->push(g_lingo->peek(peekOffset));
}

void LC::c_stackdrop() {
	int dropCount = g_lingo->readInt();
	for (int i = 0; i < dropCount; i++) {
		g_lingo->pop();
	}
}

void LC::c_assign() {
	Datum d1, d2;
	d1 = g_lingo->pop();
	d2 = g_lingo->pop();

	g_lingo->varAssign(d1, d2);
}

void LC::c_theentitypush() {
	Datum id = g_lingo->pop();

	int entity = g_lingo->readInt();
	int field  = g_lingo->readInt();

	Datum d = g_lingo->getTheEntity(entity, id, field);
	g_lingo->push(d);
}

void LC::c_themenuentitypush() {
	int entity = g_lingo->readInt();
	int field  = g_lingo->readInt();

	Datum menuId = g_lingo->pop();
	Datum menuItemId;
	Datum menuRef;
	menuRef.u.menu = new MenuReference();
	if (menuId.type == INT) {
		menuRef.u.menu->menuIdNum = menuId.u.i;
	} else if (menuId.type == STRING) {
		menuRef.u.menu->menuIdStr = menuId.u.s;
	} else {
		warning("LC::c_themenuentitypush : Unknown type of menu Reference %d", menuId.type);
		g_lingo->push(Datum());
		return;
	}

	if (entity != kTheMenuItems) { // "<entity> of menuitems" has 1 parameter
		menuItemId = g_lingo->pop();
		if (menuItemId.type == INT) {
			menuRef.u.menu->menuItemIdNum = menuItemId.u.i;
		} else if (menuItemId.type == STRING) {
			menuRef.u.menu->menuItemIdStr = menuItemId.u.s;
		} else {
			warning("LC::c_themenuentitypush : Unknown type of menuItem Reference %d", menuId.type);
			g_lingo->push(Datum());
			return;
		}
	}

	Datum d = g_lingo->getTheEntity(entity, menuRef, field);
	g_lingo->push(d);
}

void LC::c_theentityassign() {
	Datum id = g_lingo->pop();

	int entity = g_lingo->readInt();
	int field  = g_lingo->readInt();

	if (entity == kTheMenuItem) {
		Datum itemRef = g_lingo->pop();
		Datum menuRef;
		menuRef.u.menu = new MenuReference();
		menuRef.type = MENUREF;
		if (id.type == STRING) {
			menuRef.u.menu->menuIdStr = id.u.s;
		} else if (id.type == INT) {
			menuRef.u.menu->menuIdNum = id.u.i;
		} else {
			warning("LC::c_theentityassign : Unknown menu reference type %d", id.type);
			return;
		}

		if (itemRef.type == STRING) {
			menuRef.u.menu->menuItemIdStr = itemRef.u.s;
		} else if (itemRef.type == INT) {
			menuRef.u.menu->menuItemIdNum = itemRef.u.i;
		} else {
			warning("LC::c_theentityassign : Unknown menuItem reference type %d", id.type);
			return;
		}

		Datum d = g_lingo->pop();
		g_lingo->setTheEntity(entity, menuRef, field, d);
	} else {
		Datum d = g_lingo->pop();
		g_lingo->setTheEntity(entity, id, field, d);
	}
}

void LC::c_objectproppush() {
	Datum obj = g_lingo->pop();
	Common::String propName = g_lingo->readString();

	g_lingo->getObjectProp(obj, propName);
}

void LC::c_objectpropassign() {
	Datum obj = g_lingo->pop();
	Common::String propName = g_lingo->readString();

	Datum d = g_lingo->pop();

	g_lingo->setObjectProp(obj, propName, d);
}

void LC::c_swap() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(d2);
	g_lingo->push(d1);
}

static DatumType getArrayAlignedType(Datum &d1, Datum &d2) {
	if (d1.type == POINT && (d2.type == RECT || (d2.type == ARRAY && d2.u.farr->arr.size() != 2)))
		return ARRAY;

	if (d1.type == POINT)
		return POINT;

	if (d1.type == RECT && (d2.type == POINT || (d2.type == ARRAY && d2.u.farr->arr.size() != 4)))
		return ARRAY;

	if (d1.type == RECT)
		return RECT;

	if (!d1.isArray())
		return d2.type;

	return ARRAY;
}

Datum LC::mapBinaryOp(Datum (*mapFunc)(Datum &, Datum &), Datum &d1, Datum &d2) {
	// At least one of d1 and d2 must be an array
	uint arraySize;

	if (d1.isArray() && d2.isArray()) {
		arraySize = MIN(d1.u.farr->arr.size(), d2.u.farr->arr.size());
	} else if (d1.type == PARRAY && d2.type == PARRAY) {
		arraySize = MIN(d1.u.parr->arr.size(), d2.u.parr->arr.size());
	// if d1 and d2 are different arrays, result is [x+d2 for x in d1], with type of d1
	} else if (d1.isArray() && d2.type == PARRAY) {
		arraySize = d1.u.farr->arr.size();
	} else if (d1.type == PARRAY && d2.isArray()) {
		arraySize = d1.u.parr->arr.size();
	} else if (d1.isArray() || d1.type == PARRAY) {
		arraySize = d1.type == PARRAY ? d1.u.parr->arr.size() : d1.u.farr->arr.size();
	} else {
		arraySize = d2.type == PARRAY ? d2.u.parr->arr.size() : d2.u.farr->arr.size();
	}
	Datum res;
	if (d1.type == PARRAY) {
		res.type = PARRAY;
		res.u.parr = new PArray(arraySize);
	} else {
		res.type = getArrayAlignedType(d1, d2);
		res.u.farr = new FArray(arraySize);
	}
	Datum a = d1;
	Datum b = d2;
	for (uint i = 0; i < arraySize; i++) {
		if (d1.isArray()) {
			a = d1.u.farr->arr[i];
		} else if (d1.type == PARRAY) {
			a = d1.u.parr->arr[i].v;
		}
		if (d2.isArray()) {
			b = d2.u.farr->arr[i];
		} else if (d2.type == PARRAY) {
			b = d2.u.parr->arr[i].v;
		}
		if (res.type == PARRAY) {
			res.u.parr->arr[i] = PCell(d1.u.parr->arr[i].p, mapFunc(a, b));
		} else {
			res.u.farr->arr[i] = mapFunc(a, b);
		}
	}
	return res;
}

Datum LC::addData(Datum &d1, Datum &d2) {
	if (d1.type == CASTREF || d2.type == CASTREF) {
		warning("LC::addData(): attempting to add a cast reference! This always produces 0, but might be a sign that an earlier part of the script has returned incorrect data.");
		return Datum(0);
	}

	if (d1.isArray() || d2.isArray() || d1.type == PARRAY || d2.type == PARRAY) {
		return LC::mapBinaryOp(LC::addData, d1, d2);
	}

	int alignedType = g_lingo->getAlignedType(d1, d2, false);

	Datum res;
	if (alignedType == FLOAT) {
		res = Datum(d1.asFloat() + d2.asFloat());
	} else if (alignedType == INT) {
		res = Datum(d1.asInt() + d2.asInt());
	} else {
		g_lingo->lingoError("LC::addData(): not supported between types %s and %s", d1.type2str(), d2.type2str());
	}
	return res;
}

void LC::c_add() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::addData(d1, d2));
}

Datum LC::subData(Datum &d1, Datum &d2) {
	if (d1.type == CASTREF || d2.type == CASTREF) {
		warning("LC::subData(): attempting to subtract a cast reference! This always produces 0, but might be a sign that an earlier part of the script has returned incorrect data.");
		return Datum(0);
	}

	if (d1.isArray() || d2.isArray() || d1.type == PARRAY || d2.type == PARRAY) {
		return LC::mapBinaryOp(LC::subData, d1, d2);
	}

	int alignedType = g_lingo->getAlignedType(d1, d2, false);

	Datum res;
	if (alignedType == FLOAT) {
		res = Datum(d1.asFloat() - d2.asFloat());
	} else if (alignedType == INT) {
		res = Datum(d1.asInt() - d2.asInt());
	} else {
		g_lingo->lingoError("LC::subData(): not supported between types %s and %s", d1.type2str(), d2.type2str());
	}
	return res;
}

void LC::c_sub() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::subData(d1, d2));
}

Datum LC::mulData(Datum &d1, Datum &d2) {
	if (d1.type == CASTREF || d2.type == CASTREF) {
		warning("LC::mulData(): attempting to multiply a cast reference! This always produces 0, but might be a sign that an earlier part of the script has returned incorrect data.");
		return Datum(0);
	}

	if (d1.isArray() || d2.isArray() || d1.type == PARRAY || d2.type == PARRAY) {
		return LC::mapBinaryOp(LC::mulData, d1, d2);
	}

	int alignedType = g_lingo->getAlignedType(d1, d2, false);

	Datum res;
	if (alignedType == FLOAT) {
		res = Datum(d1.asFloat() * d2.asFloat());
	} else if (alignedType == INT) {
		res = Datum(d1.asInt() * d2.asInt());
	} else {
		g_lingo->lingoError("LC::mulData(): not supported between types %s and %s", d1.type2str(), d2.type2str());
	}
	return res;
}

void LC::c_mul() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::mulData(d1, d2));
}

Datum LC::divData(Datum &d1, Datum &d2) {
	if (d1.type == CASTREF || d2.type == CASTREF) {
		warning("LC::divData(): attempting to divide a cast reference! This always produces 0, but might be a sign that an earlier part of the script has returned incorrect data.");
		return Datum(0);
	}

	if (d1.isArray() || d2.isArray() || d1.type == PARRAY || d2.type == PARRAY) {
		return LC::mapBinaryOp(LC::divData, d1, d2);
	}

	if ((d2.type == INT && d2.u.i == 0) ||
			(d2.type == FLOAT && d2.u.f == 0.0)) {
		warning("LC::divData(): division by zero");
		d2 = Datum(1);
	}

	int alignedType = g_lingo->getAlignedType(d1, d2, false);

	if (g_director->getVersion() < 400)	// pre-D4 is INT-only
		alignedType = INT;

	Datum res;
	if (alignedType == FLOAT) {
		res = Datum(d1.asFloat() / d2.asFloat());
	} else if (alignedType == INT) {
		res = Datum(d1.asInt() / d2.asInt());
	} else {
		g_lingo->lingoError("LC::divData(): not supported between types %s and %s", d1.type2str(), d2.type2str());
	}

	return res;
}

void LC::c_div() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(divData(d1, d2));
}

Datum LC::modData(Datum &d1, Datum &d2) {
	if (d1.isArray() || d2.isArray()) {
		return LC::mapBinaryOp(LC::modData, d1, d2);
	}

	int i1 = d1.asInt();
	int i2 = d2.asInt();
	if (i2 == 0) {
		g_lingo->lingoError("LC::modData(): division by zero");
		i2 = 1;
	}

	Datum res(i1 % i2);
	return res;
}

void LC::c_mod() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::modData(d1, d2));
}

Datum LC::negateData(Datum &d) {
	if (d.isArray()) {
		uint arraySize = d.u.farr->arr.size();
		Datum res;
		res.type = d.type;
		res.u.farr = new FArray(arraySize);
		for (uint i = 0; i < arraySize; i++) {
			res.u.farr->arr[i] = LC::negateData(d.u.farr->arr[i]);
		}
		return res;
	}

	Datum res;
	if (d.type == INT) {
		res = Datum(-d.asInt());
	} else if (d.type == FLOAT) {
		res = Datum(-d.asFloat());
	} else if (d.type == VOID) {
		res = Datum(0);
	} else {
		g_lingo->lingoError("LC::negateData(): not supported for type %s", d.type2str());
	}

	return res;
}

void LC::c_negate() {
	Datum d = g_lingo->pop();
	g_lingo->push(negateData(d));
}

void LC::c_ampersand() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Datum res(d1.asString() + d2.asString());
	g_lingo->push(res);
}

void LC::c_putbefore() {
	Datum var = g_lingo->pop();
	Datum a = g_lingo->pop();
	Datum b = g_lingo->varFetch(var);

	Datum res(a.asString() + b.asString());
	g_lingo->varAssign(var, res);
}

void LC::c_putafter() {
	Datum var = g_lingo->pop();
	Datum a = g_lingo->pop();
	Datum b = g_lingo->varFetch(var);

	Datum res(b.asString() + a.asString());
	g_lingo->varAssign(var, res);
}

void LC::c_concat() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Datum res(d1.asString() + " " + d2.asString());
	g_lingo->push(res);
}

void LC::c_contains() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Common::String s1 = g_lingo->normalizeString(d1.asString());
	Common::String s2 = g_lingo->normalizeString(d2.asString());

	int res = s1.contains(s2) ? 1 : 0;

	g_lingo->push(Datum(res));
}

void LC::c_starts() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Common::String s1 = g_lingo->normalizeString(d1.asString());
	Common::String s2 = g_lingo->normalizeString(d2.asString());

	int res = s1.hasPrefix(s2) ? 1 : 0;

	g_lingo->push(Datum(res));
}

void LC::c_intersects() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Score *score = g_director->getCurrentMovie()->getScore();
	Channel *sprite1 = score->getChannelById(d1.asInt());
	Channel *sprite2 = score->getChannelById(d2.asInt());

	if (!sprite1 || !sprite2) {
		g_lingo->push(Datum(0));
		return;
	}

	// don't regard quick draw shape as matte type
	if ((!sprite1->_sprite->isQDShape() && sprite1->_sprite->_ink == kInkTypeMatte) && (!sprite2->_sprite->isQDShape() && sprite2->_sprite->_ink == kInkTypeMatte)) {
		g_lingo->push(Datum(sprite2->isMatteIntersect(sprite1)));
	} else {
		g_lingo->push(Datum(sprite2->getBbox().intersects(sprite1->getBbox())));
	}
}

void LC::c_within() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Score *score = g_director->getCurrentMovie()->getScore();
	Channel *sprite1 = score->getChannelById(d1.asInt());
	Channel *sprite2 = score->getChannelById(d2.asInt());

	if (!sprite1 || !sprite2) {
		g_lingo->push(Datum(0));
		return;
	}

	// don't regard quick draw shape as matte type
	if ((!sprite1->_sprite->isQDShape() && sprite1->_sprite->_ink == kInkTypeMatte) && (!sprite2->_sprite->isQDShape() && sprite2->_sprite->_ink == kInkTypeMatte)) {
		g_lingo->push(Datum(sprite2->isMatteWithin(sprite1)));
	} else {
		g_lingo->push(Datum(sprite2->getBbox().contains(sprite1->getBbox())));
	}
}

Datum LC::chunkRef(ChunkType type, int startChunk, int endChunk, const Datum &src) {
	// A chunk expression is made up of 0 or more chunks within a source text.
	// This function returns a reference to the source text, the start index of the first chunk,
	// and the end index of the last chunk in the chunk expression.

	// startChunk == -30000 means return the last chunk
	if (startChunk < 1 && startChunk != -30000)
		return src;

	if (endChunk < 1 || startChunk == -30000)
		endChunk = startChunk;

	Common::U32String str = g_lingo->evalChunkRef(src);

	// these hold the bounds of the last chunk in the expression
	int chunkNum = 0;
	int chunkStartIdx = -1;
	int chunkEndIdx = -1;

	// these hold the bounds of the entire chunk expression
	int exprStartIdx = -1;
	int exprEndIdx = -1;

	switch (type) {
	case kChunkChar:
		if (startChunk < 1) {
			// last char was requested. set its bounds.
			chunkNum = str.size();
			chunkStartIdx = str.size() - 1;
			chunkEndIdx = str.size();
		} else if (startChunk <= (int)str.size()) {
			exprStartIdx = MIN(startChunk, (int)str.size()) - 1;
			exprEndIdx = MIN(endChunk, (int)str.size());
		}
		break;
	case kChunkWord:
		{
			int idx = 0;
			while (idx < (int)str.size() && Common::isSpace(str[idx])) {
				idx++;
			}
			while (idx < (int)str.size()) {
				// each loop processes one chunk
				chunkNum++;

				// start of chunk
				chunkStartIdx = idx;
				if (chunkNum == startChunk) {
					exprStartIdx = chunkStartIdx;
				}

				while (idx < (int)str.size() && !Common::isSpace(str[idx])) {
					idx++;
				}

				// end of chunk
				chunkEndIdx = idx;

				if (chunkNum == endChunk) {
					exprEndIdx = chunkEndIdx;
					break;
				}

				while (idx < (int)str.size() && Common::isSpace(str[idx])) {
					idx++;
				}
			}
		}
		break;
	case kChunkItem:
	case kChunkLine:
		{
			Common::u32char_type_t delimiter = (type == kChunkItem) ? g_lingo->_itemDelimiter : '\r';

			int idx = 0;
			while (true) {
				// each loop processes one chunk
				chunkNum++;

				// start of chunk
				chunkStartIdx = idx;
				if (chunkNum == startChunk) {
					exprStartIdx = chunkStartIdx;
				}

				while (idx < (int)str.size() && str[idx] != delimiter) {
					idx++;
				}

				// end of chunk
				chunkEndIdx = idx;
				if (chunkNum == endChunk) {
					exprEndIdx = chunkEndIdx;
					break;
				}

				if (idx == (int)str.size())
					break;

				idx++; // skip delimiter
			}
		}
		break;
	}

	if (startChunk == -30000) {
		// return the last chunk we found
		startChunk = chunkNum;
		endChunk = chunkNum;
		exprStartIdx = chunkStartIdx;
		exprEndIdx = chunkEndIdx;
	} else {
		if (exprStartIdx < 0) {
			// we never found the requested start chunk
			exprStartIdx = -1;
		}
		if (exprEndIdx < 0) {
			// we never found the requested end chunk
			exprEndIdx = str.size();
		}
	}

	Datum res;
	res.u.cref = new ChunkReference(src, type, startChunk, endChunk, exprStartIdx, exprEndIdx);
	res.type = CHUNKREF;
	return res;
}

Datum LC::lastChunk(ChunkType type, const Datum &src) {
	return chunkRef(type, -30000, 0, src);
}

Datum LC::readChunkRef(const Datum &src) {
	Datum lastLine = g_lingo->pop();
	Datum firstLine = g_lingo->pop();
	Datum lastItem = g_lingo->pop();
	Datum firstItem = g_lingo->pop();
	Datum lastWord = g_lingo->pop();
	Datum firstWord = g_lingo->pop();
	Datum lastChar = g_lingo->pop();
	Datum firstChar = g_lingo->pop();

	Datum ref = src;
	if (firstLine.asInt() != 0)
		ref = LC::chunkRef(kChunkLine, firstLine.asInt(), lastLine.asInt(), ref);
	if (firstItem.asInt() != 0)
		ref = LC::chunkRef(kChunkItem, firstItem.asInt(), lastItem.asInt(), ref);
	if (firstWord.asInt() != 0)
		ref = LC::chunkRef(kChunkWord, firstWord.asInt(), lastWord.asInt(), ref);
	if (firstChar.asInt() != 0)
		ref = LC::chunkRef(kChunkChar, firstChar.asInt(), lastChar.asInt(), ref);

	return ref;
}

void LC::c_of() {
	Datum src = g_lingo->pop();
	Datum ref = readChunkRef(src);
	g_lingo->push(ref.eval());
}

void LC::c_charToOfRef() {
	Datum src = g_lingo->pop();
	Datum indexTo = g_lingo->pop();
	Datum indexFrom = g_lingo->pop();

	if ((indexTo.type != INT && indexTo.type != FLOAT) || (indexFrom.type != INT && indexFrom.type != FLOAT)
			|| (src.type != STRING && !src.isRef())) {
		warning("LC::c_charToOfRef(): Called with wrong data types: %s, %s and %s", indexTo.type2str(), indexFrom.type2str(), src.type2str());
		g_lingo->push(Datum(""));
		return;
	}

	g_lingo->push(LC::chunkRef(kChunkChar, indexFrom.asInt(), indexTo.asInt(), src));
}

void LC::c_charToOf() {
	LC::c_charToOfRef();
	Datum ref = g_lingo->pop();
	g_lingo->push(ref.eval());
}

void LC::c_itemToOfRef() {
	Datum src = g_lingo->pop();
	Datum indexTo = g_lingo->pop();
	Datum indexFrom = g_lingo->pop();

	if ((indexTo.type != INT && indexTo.type != FLOAT) || (indexFrom.type != INT && indexFrom.type != FLOAT)
			|| (src.type != STRING && !src.isRef())) {
		warning("LC::c_itemToOfRef(): Called with wrong data types: %s, %s and %s", indexTo.type2str(), indexFrom.type2str(), src.type2str());
		g_lingo->push(Datum(""));
		return;
	}

	g_lingo->push(LC::chunkRef(kChunkItem, indexFrom.asInt(), indexTo.asInt(), src));
}

void LC::c_itemToOf() {
	LC::c_itemToOfRef();
	Datum ref = g_lingo->pop();
	g_lingo->push(ref.eval());
}

void LC::c_lineToOfRef() {
	Datum src = g_lingo->pop();
	Datum indexTo = g_lingo->pop();
	Datum indexFrom = g_lingo->pop();

	if ((indexTo.type != INT && indexTo.type != FLOAT) || (indexFrom.type != INT && indexFrom.type != FLOAT)
			|| (src.type != STRING && !src.isRef())) {
		warning("LC::c_lineToOfRef(): Called with wrong data types: %s, %s and %s", indexTo.type2str(), indexFrom.type2str(), src.type2str());
		g_lingo->push(Datum(""));
		return;
	}

	g_lingo->push(LC::chunkRef(kChunkLine, indexFrom.asInt(), indexTo.asInt(), src));
}

void LC::c_lineToOf() {
	LC::c_lineToOfRef();
	Datum ref = g_lingo->pop();
	g_lingo->push(ref.eval());
}

void LC::c_wordToOfRef() {
	Datum src = g_lingo->pop();
	Datum indexTo = g_lingo->pop();
	Datum indexFrom = g_lingo->pop();

	if ((indexTo.type != INT && indexTo.type != FLOAT) || (indexFrom.type != INT && indexFrom.type != FLOAT)
			|| (src.type != STRING && !src.isRef())) {
		warning("LC::c_wordToOfRef(): Called with wrong data types: %s, %s and %s", indexTo.type2str(), indexFrom.type2str(), src.type2str());
		g_lingo->push(Datum(""));
		return;
	}

	g_lingo->push(LC::chunkRef(kChunkWord, indexFrom.asInt(), indexTo.asInt(), src));
}

void LC::c_wordToOf() {
	LC::c_wordToOfRef();
	Datum ref = g_lingo->pop();
	g_lingo->push(ref.eval());
}

void LC::c_and() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Datum res((d1.asInt() && d2.asInt()) ? 1 : 0);

	g_lingo->push(res);
}

void LC::c_or() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Datum res((d1.asInt() || d2.asInt()) ? 1 : 0);

	g_lingo->push(res);
}

void LC::c_not() {
	// Not returns true when a variable is undefined or is an int and is zero.
	Datum res;
	Datum d = g_lingo->pop();

	if ((d.type == INT && d.u.i == 0) || d.type == VOID) {
		res = Datum(1);
	} else {
		res = Datum(0);
	}

	g_lingo->push(res);
}

Datum LC::compareArrays(Datum (*compareFunc)(Datum, Datum), Datum d1, Datum d2, bool location, bool value) {
	// At least one of d1 and d2 must be an array
	bool d1isArr = d1.isArray() || d1.type == PARRAY;
	bool d2isArr = d2.isArray() || d2.type == PARRAY;
	uint32 d1size = d1.isArray() ? d1.u.farr->arr.size() : d1.type == PARRAY ? d1.u.parr->arr.size() : 0;
	uint32 d2size = d2.isArray() ? d2.u.farr->arr.size() : d2.type == PARRAY ? d2.u.parr->arr.size() : 0;
	// The calling convention of this checking function is a bit weird:
	// - If the location flag is set, you're searching for element d2 in list d1
	// - If the location flag is not set and there are two array-like arguments passed, you are comparing
	//   elements and therefore need to truncate output to the smaller size.
	// - Otherwise, you are comparing an array to a single element, and all elements
	//   of the array need to be checked.
	uint arraySize = location ? d1size : ((d1isArr && d2isArr) ? MIN(d1size, d2size) : MAX(d1size, d2size));

	Datum res;
	res = location ? -1 : 1;
	Datum a = d1;
	Datum b = d2;
	for (uint i = 0; i < arraySize; i++) {
		if (d1.isArray()) {
			a = d1.u.farr->arr[i];
		} else if (d1.type == PARRAY) {
			PCell t = d1.u.parr->arr[i];
			a = value ? t.v : t.p;
		}

		if (!location && d2.isArray()) {
			b = d2.u.farr->arr[i];
		} else if (!location && d2.type == PARRAY) {
			PCell t = d2.u.parr->arr[i];
			b = value ? t.v : t.p;
		}

		// Special case, we can retrieve symbolic key by giving their string representation, ie
		// for arr [a: "abc", "b": "def"], both getProp(arr, "a") and getProp(arr, #a) will return "abc",
		// vice-versa is also true, ie getProp(arr, "b") and getProp(arr, #b) will return "def"
		if (a.type == SYMBOL && b.type == STRING) {
			a = Datum(a.asString());
		} else if (a.type == STRING && b.type == SYMBOL) {
            b = Datum(b.asString());
        }

		res = compareFunc(a, b);
		if (!location) {
			if (res.u.i == 0) {
				break;
			}
		} else {
			if (res.u.i == 1) {
				// Lingo indexing starts at 1
				res.u.i = (int)i + 1;
				break;
			}
		}
	}
	return res;
}

Datum LC::eqData(Datum d1, Datum d2) {
	// D4 has a bug, and only checks the elements on the left array.
	// Therefore if the left array is bigger, don't bother checking.
	// LC::compareArrays will trim the inputs to the shortest length.
	// (Mac 4.0.4 is fixed, Win 4.0.4 is not)
	bool hasArrayBug = (g_director->getVersion() < 500 && g_director->getPlatform() == Common::kPlatformWindows) ||
		(g_director->getVersion() < 404 && g_director->getPlatform() == Common::kPlatformMacintosh);

	if (d1.isArray() || d2.isArray() || d1.type == PARRAY || d2.type == PARRAY) {
		bool d1isArr = d1.isArray() || d1.type == PARRAY;
		bool d2isArr = d2.isArray() || d2.type == PARRAY;
		uint32 d1size = d1.isArray() ? d1.u.farr->arr.size() : d1.type == PARRAY ? d1.u.parr->arr.size() : 0;
		uint32 d2size = d2.isArray() ? d2.u.farr->arr.size() : d2.type == PARRAY ? d2.u.parr->arr.size() : 0;
		if (hasArrayBug && d1isArr && d2isArr && d1size > d2size) {
			// D4; only check arrays if the left size is less than or equal to the right side
			return Datum(0);
		} else if (!hasArrayBug && d1isArr && d2isArr && d1size != d2size) {
			// D5 and up is fixed; only check arrays if the sizes are the same.
			return Datum(0);
		}
		return LC::compareArrays(LC::eqData, d1, d2, false, true);
	}
	Datum check;
	check = d1.equalTo(d2, true);
	return check;
}

Datum LC::eqDataStrict(Datum d1, Datum d2) {
	// b_getPos and b_getOne will do case-sensitive
	// string comparison when determining a match.
	// As opposed to, y'know, the whole rest of
	// Director which is case insensitive.
	if (d1.type == STRING && d2.type == STRING) {
		return Datum(*d1.u.s == *d2.u.s ? 1 : 0);
	}
	// ARRAYs and PARRAYs will do a pointer check,
	// not a contents check
	if (d1.isArray() && d2.isArray()) {
		return Datum(d1.u.farr == d2.u.farr ? 1 : 0);
	}
	if (d1.type == PARRAY && d2.type == PARRAY) {
		return Datum(d1.u.parr == d2.u.parr ? 1 : 0);
	}
	return LC::eqData(d1, d2);
}

void LC::c_eq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::eqData(d1, d2));
}

Datum LC::neqData(Datum d1, Datum d2) {
	// invert the output of eqData
	return LC::eqData(d1, d2).asInt() ? 0 : 1;
}

void LC::c_neq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::neqData(d1, d2));
}

Datum LC::gtData(Datum d1, Datum d2) {
	if (d1.isArray() || d2.isArray() || d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::gtData, d1, d2, false, true);
	}
	Datum check;
	check = (d1 > d2 ? 1 : 0);
	return check;
}

void LC::c_gt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::gtData(d1, d2));
}

Datum LC::ltData(Datum d1, Datum d2) {
	if (d1.isArray() || d2.isArray() || d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::ltData, d1, d2, false, true);
	}
	Datum check;
	check = d1 < d2 ? 1 : 0;
	return check;
}

void LC::c_lt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::ltData(d1, d2));
}

Datum LC::geData(Datum d1, Datum d2) {
	if (d1.isArray() || d2.isArray() || d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::geData, d1, d2, false, true);
	}
	Datum check;
	check = d1 >= d2 ? 1 : 0;
	return check;
}

void LC::c_ge() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::geData(d1, d2));
}

Datum LC::leData(Datum d1, Datum d2) {
	if (d1.isArray() || d2.isArray() || d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::leData, d1, d2, false, true);
	}
	Datum check;
	check =  d1 <= d2 ? 1 : 0;
	return check;
}

void LC::c_le() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::leData(d1, d2));
}

void LC::c_jump() {
	int jump = g_lingo->readInt();
	g_lingo->_state->pc = g_lingo->_state->pc + jump - 2;
}

void LC::c_jumpifz() {
	int jump = g_lingo->readInt();
	int test = g_lingo->pop().asInt();
	if (test == 0) {
		g_lingo->_state->pc = g_lingo->_state->pc + jump - 2;
	}
}

void LC::c_whencode() {
	Common::String eventname(g_lingo->readString());
	Datum code = g_lingo->pop();
	Datum nullId;

	// the following when events are supported by D3
	if (eventname.equalsIgnoreCase("keyDown")) {
		g_lingo->setTheEntity(kTheKeyDownScript, nullId, kTheNOField, code);
	} else if (eventname.equalsIgnoreCase("keyUp")) {
		g_lingo->setTheEntity(kTheKeyUpScript, nullId, kTheNOField, code);
	} else if (eventname.equalsIgnoreCase("mouseDown")) {
		g_lingo->setTheEntity(kTheMouseDownScript, nullId, kTheNOField, code);
	} else if (eventname.equalsIgnoreCase("mouseUp")) {
		g_lingo->setTheEntity(kTheMouseUpScript, nullId, kTheNOField, code);
	} else if (eventname.equalsIgnoreCase("timeOut")) {
		g_lingo->setTheEntity(kTheTimeoutScript, nullId, kTheNOField, code);
	} else {
		warning("LC::c_whencode(): unsupported event handler %s", eventname.c_str());
	}
}

void LC::c_tell() {
	// swap out current window
	Datum window = g_lingo->pop();
	g_lingo->push(g_director->getCurrentWindow());
	if (window.type != OBJECT || window.u.obj->getObjType() != kWindowObj) {
		warning("LC::c_tell(): wrong argument type: %s", window.type2str());
		return;
	}
	Window *w = static_cast<Window *>(window.u.obj);
	w->ensureMovieIsLoaded();
	if (w->getCurrentMovie() == nullptr) {
		warning("LC::c_tell(): window has no movie");
		return;
	}
	g_director->setCurrentWindow(w);

}

void LC::c_telldone() {
	Datum returnWindow = g_lingo->pop();
	if (returnWindow.type != OBJECT || returnWindow.u.obj->getObjType() != kWindowObj) {
		warning("LC::c_telldone(): wrong return window type: %s", returnWindow.type2str());
		return;
	}
	g_director->setCurrentWindow(static_cast<Window *>(returnWindow.u.obj));
}


//************************
// Built-in functions
//************************

void LC::c_callcmd() {
	Common::String name(g_lingo->readString());

	int nargs = g_lingo->readInt();

	LC::call(name, nargs, false);
}

void LC::c_callfunc() {
	Common::String name(g_lingo->readString());

	int nargs = g_lingo->readInt();

	LC::call(name, nargs, true);
}

void LC::call(const Common::String &name, int nargs, bool allowRetVal) {
	if (debugChannelSet(3, kDebugLingoExec))
		g_lingo->printArgs(name.c_str(), nargs, "call:");

	Symbol funcSym;

	if (nargs > 0) {
		Datum firstArg = g_lingo->_stack[g_lingo->_stack.size() - nargs];

		// Factory/XObject method call
		if (firstArg.isVarRef()) { // first arg could be method name
			Datum objName(name);
			objName.type = VARREF;
			Datum obj = g_lingo->varFetch(objName, true);
			if (obj.type == OBJECT && (obj.u.obj->getObjType() & (kFactoryObj | kXObj))) {
				debugC(3, kDebugLingoExec, "Factory/XObject method called on object: <%s>", obj.asString(true).c_str());
				AbstractObject *target = obj.u.obj;
				if (firstArg.u.s->equalsIgnoreCase("mNew")) {
					target = target->clone();
				}
				funcSym = target->getMethod(*firstArg.u.s);
				if (funcSym.type != VOIDSYM) {
					g_lingo->_stack[g_lingo->_stack.size() - nargs] = funcSym.target; // Set first arg to target
					call(funcSym, nargs, allowRetVal);
				} else {
					g_lingo->lingoError("Object <%s> has no method '%s'", obj.asString(true).c_str(), firstArg.u.s->c_str());
				}
				return;
			}
			firstArg = g_lingo->_stack[g_lingo->_stack.size() - nargs] = firstArg.eval();
		}

		// Script/Xtra method call
		if (firstArg.type == OBJECT && !(firstArg.u.obj->getObjType() & (kFactoryObj | kXObj))) {
			debugC(3, kDebugLingoExec, "Script/Xtra method called on object: <%s>", firstArg.asString(true).c_str());
			AbstractObject *target = firstArg.u.obj;
			if (name.equalsIgnoreCase("birth") || name.equalsIgnoreCase("new")) {
				target = target->clone();
			}
			funcSym = target->getMethod(name);
			if (funcSym.type != VOIDSYM) {
				g_lingo->_stack[g_lingo->_stack.size() - nargs] = target; // Set first arg to target
				call(funcSym, nargs, allowRetVal);
				return;
			}
		}
	}

	// If we're calling from within a me object, and it has a function handler with a
	// matching name, include the me object in the CFrame (so we still get property lookups).
	// Doesn't matter that the first arg isn't the me object (which would have been caught
	// by the Factory/XObject code above).
	//
	// If the method is called from outside and without the object as the first arg,
	// it will still work using the normal getHandler lookup.
	// However properties will return garbage (the number 3??).
	if (g_lingo->_state->me.type == OBJECT) {
		AbstractObject *target = g_lingo->_state->me.u.obj;
		funcSym = target->getMethod(name);
		if (funcSym.type != VOIDSYM) {
			call(funcSym, nargs, allowRetVal);
			return;
		}
	}

	// Handler
	funcSym = g_lingo->getHandler(name);

	if (g_lingo->_builtinListHandlers.contains(name) && nargs >= 1) {
		// Lingo builtin functions in the "List" category have very strange override mechanics.
		// If the first argument is an ARRAY or PARRAY, it will use the builtin.
		// Otherwise, it will fall back to whatever handler is defined globally.
		Datum firstArg = g_lingo->peek(nargs - 1);
		if (firstArg.type == ARRAY || firstArg.type == PARRAY ||
				firstArg.type == POINT || firstArg.type == RECT) {
			funcSym = g_lingo->_builtinListHandlers[name];
		}
	}

	if (funcSym.type == VOIDSYM) { // The built-ins could be overridden
		// Builtin
		if (allowRetVal) {
			if (g_lingo->_builtinFuncs.contains(name)) {
				funcSym = g_lingo->_builtinFuncs[name];
			}
		} else {
			if (g_lingo->_builtinCmds.contains(name)) {
				funcSym = g_lingo->_builtinCmds[name];
			}
		}
	}

	// use lingo-the as fallback. we can only use functions as fallback, not properties
	if (funcSym.type == VOIDSYM && g_lingo->_theEntities.contains(name) && g_lingo->_theEntities[name]->isFunction) {
		Datum id;
		Datum res = g_lingo->getTheEntity(g_lingo->_theEntities[name]->entity, id, kTheNOField);
		g_lingo->push(res);
		return;
	}

	call(funcSym, nargs, allowRetVal);
}

void LC::call(const Symbol &funcSym, int nargs, bool allowRetVal) {
	int paramCount = nargs;
	Datum target = funcSym.target;

	if (funcSym.type == VOIDSYM) {
		if (funcSym.name) {
			// Lingo was also treating all 'the' entities as functions
			if (g_lingo->_theEntities.contains(*funcSym.name) && nargs == 0) {
				warning("Calling builtin '%s' as a function", funcSym.name->c_str());

				TheEntity *entity = g_lingo->_theEntities[*funcSym.name];
				Datum id;
				id.u.i = 0;
				id.type = VOID;

				g_lingo->push(g_lingo->getTheEntity(entity->entity, id, 0));

				return;
			}

			g_lingo->lingoError("Call to undefined handler '%s'. Dropping %d stack items", funcSym.name->c_str(), nargs);
		} else {
			g_lingo->lingoError("Call to undefined handler. Dropping %d stack items", nargs);
		}

		for (int i = 0; i < nargs; i++)
			g_lingo->pop();

		// Push dummy value
		if (allowRetVal)
			g_lingo->pushVoid();

		return;
	}

	if (funcSym.type != HANDLER && target.type != VOID) {
		// Drop the target argument (only needed for user-defined methods)
		g_lingo->_stack.remove_at(g_lingo->_stack.size() - nargs);
		nargs--;
	}

	if (funcSym.nargs != -1) {
		if (funcSym.type == HANDLER || funcSym.type == HBLTIN) {
			// Lingo supports providing a different number of arguments than expected,
			// and several games rely on this behaviour.
			if (funcSym.nargs > nargs) {
				debugC(1, kDebugLingoExec, "Incorrect number of arguments for handler '%s' (%d, expected %d to %d). Adding extra %d voids",
							funcSym.name->c_str(), nargs, funcSym.nargs, funcSym.maxArgs, funcSym.nargs - nargs);
				while (nargs < funcSym.nargs) {
					Datum d;
					d.u.s = nullptr;
					d.type = VOID;
					g_lingo->push(d);
					nargs++;
				}
			}
		} else if (funcSym.nargs > nargs || funcSym.maxArgs < nargs) {
			warning("Incorrect number of arguments for builtin '%s' (%d, expected %d to %d). Dropping %d stack items.",
						funcSym.name->c_str(), nargs, funcSym.nargs, funcSym.maxArgs, nargs);

			for (int i = 0; i < nargs; i++)
				g_lingo->pop();

			// Push dummy value
			if (allowRetVal)
				g_lingo->pushVoid();

			return;
		}
	}

	if (funcSym.type != HANDLER) {
		g_debugger->builtinHook(funcSym);
		uint stackSizeBefore = g_lingo->_stack.size() - nargs;

		if (target.type != VOID) {
			// Only need to update the me obj
			// Pushing an entire stack frame is not necessary
			Datum retMe = g_lingo->_state->me;
			g_lingo->_state->me = target;

			// WORKAROUND: m_Perform needs to know if value should be returned or not (to create a new context frames for handles)
			if (funcSym.name->equals("perform"))
				g_lingo->push(Datum(allowRetVal));

			(*funcSym.u.bltin)(nargs);
			g_lingo->_state->me = retMe;
		} else {
			(*funcSym.u.bltin)(nargs);
		}

		uint stackSize = g_lingo->_stack.size();

		if (funcSym.u.bltin != LB::b_return && funcSym.u.bltin != LB::b_value) {
			if (stackSize == stackSizeBefore + 1) {
				// Set "the result" to return value!, when a method
				// this is for handling result after execution!
				Datum top = g_lingo->peek(0);
				if (top.type == INT)
					g_lingo->_theResult = top;

				if (!allowRetVal) {
					Datum extra = g_lingo->pop();
					warning("Builtin '%s' dropping return value: %s", funcSym.name->c_str(), extra.asString(true).c_str());
				}
			} else if (stackSize == stackSizeBefore) {
				// No value, hence "the result" is VOID
				g_lingo->_theResult = g_lingo->getVoid();

				if (allowRetVal)
					error("Builtin '%s' did not return value", funcSym.name->c_str());
			} else if (stackSize > stackSizeBefore) {
				error("Builtin '%s' returned extra %d values", funcSym.name->c_str(), stackSize - stackSizeBefore);
			} else {
				error("Builtin '%s' popped extra %d values", funcSym.name->c_str(), stackSizeBefore - stackSize);
			}
		}
		return;
	}

	Datum defaultRetVal;
	if (funcSym.target && funcSym.target->getObjType() == kFactoryObj && funcSym.name->equalsIgnoreCase("mNew")) {
		defaultRetVal = funcSym.target; // return me
	}

	g_lingo->pushContext(funcSym, allowRetVal, defaultRetVal, paramCount, nargs);
}

void LC::c_procret() {
	// Equivalent of Lingo's "exit" command.
	// If we hit this instruction, wipe whatever new is on the Lingo stack,
	// as we could e.g. be in a loop.
	// Returning a value must be done by calling LB::b_return().
	Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
	CFrame *fp = callstack.back();
	int extra = g_lingo->_stack.size() - fp->stackSizeBefore;
	if (extra > 0) {
		debugC(5, kDebugLingoExec, "c_procret: dropping %d items", extra);
		g_lingo->dropStack(extra);
	} else if (extra < 0) {
		error("c_procret: handler %s has a stack delta size of %d", fp->sp.name->c_str(), extra);
	}

	procret();
}

void LC::procret() {
	// Lingo stack must be empty or have one value
	Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;

	if (callstack.size() == 0) {
		warning("LC::c_procret(): Call stack underflow");
		g_lingo->_abort = true;
		return;
	}

	g_lingo->popContext();

	if (callstack.size() == 0) {
		debugC(5, kDebugLingoExec, "Call stack empty, returning");
		g_lingo->_abort = true;
		return;
	}
}


void LC::c_delete() {
	Datum d = g_lingo->pop();

	Datum field;
	int start, end;
	if (d.type == CHUNKREF) {
		start = d.u.cref->start;
		end = d.u.cref->end;
		field = d.u.cref->source;
		while (field.type == CHUNKREF) {
			start += field.u.cref->start;
			end += field.u.cref->start;
			field = field.u.cref->source;
		}
		if (!field.isVarRef() && !field.isCastRef()) {
			warning("BUILDBOT: c_delete: bad chunk ref field type: %s", field.type2str());
			return;
		}
	} else if (d.isRef()) {
		field = d;
		start = 0;
		end = -1;
	} else {
		warning("BUILDBOT: c_delete: bad field type: %s", d.type2str());
		return;
	}

	if (start < 0)
		return;

	Common::U32String text = g_lingo->evalChunkRef(field);
	if (d.type == CHUNKREF) {
		switch (d.u.cref->type) {
		case kChunkChar:
			break;
		case kChunkWord:
			while (end < (int)text.size() && Common::isSpace(text[end]))
				end++;
			break;
		case kChunkItem:
		case kChunkLine:
			// when deleting the first item, include the delimiter after the item
			// deleting another item, remove the delimiter in front
			if (start == 0) {
				end++;
			} else {
				start--;
			}
			break;
		}
	}

	Common::U32String res = text.substr(0, start);
	if (end >= 0) {
		res += text.substr(end);
	}
	Datum s;
	s.u.s = new Common::String(res, Common::kUtf8);
	s.type = STRING;
	g_lingo->varAssign(field, s);
}

void LC::c_hilite() {
	Datum d = g_lingo->pop();

	CastMemberID fieldId;
	int start, end;
	if (d.type == CHUNKREF) {
		start = d.u.cref->start;
		end = d.u.cref->end;
		Datum src = d.u.cref->source;
		while (src.type == CHUNKREF) {
			start += src.u.cref->start;
			end += src.u.cref->start;
			src = src.u.cref->source;
		}
		if (src.isCastRef()) {
			fieldId = *d.u.cast;
		} else {
			warning("BUILDBOT: c_hilite: bad chunk ref field type: %s", src.type2str());
			return;
		}
	} else if (d.isCastRef()) {
		fieldId = *d.u.cast;
		start = 0;
		end = -1;
	} else {
		warning("BUILDBOT: c_hilite: bad field type: %s", d.type2str());
		return;
	}

	if (start < 0)
		return;

	Score *score = g_director->getCurrentMovie()->getScore();
	uint16 spriteId = score->getSpriteIdByMemberId(fieldId);
	if (spriteId == 0)
		return;

	Channel *channel = score->getChannelById(spriteId);
	if (channel->_sprite->_cast && channel->_sprite->_cast->_type == kCastText && channel->_widget) {
		((Graphics::MacText *)channel->_widget)->setSelection(start, true);
		((Graphics::MacText *)channel->_widget)->setSelection(end, false);
	}
}

void LC::c_fieldref() {
	Datum castLib;
	if (g_director->getVersion() >= 500)
		castLib = g_lingo->pop();
	Datum member = g_lingo->pop();
	Datum res = member.asMemberID(kCastTypeAny, castLib.asInt());
	res.type = FIELDREF;
	g_lingo->push(res);
}

void LC::c_field() {
	LC::c_fieldref();
	Datum d = g_lingo->pop();
	Datum ref = d.eval();
	g_lingo->push(ref.eval());
}

void LC::c_asserterror() {
	g_lingo->_expectError = true;
	g_lingo->_caughtError = false;
}

void LC::c_asserterrordone() {
	if (!g_lingo->_caughtError) {
		warning("BUILDBOT: c_asserterrordone: did not catch error");
	}
	g_lingo->_expectError = false;
}

} // End of namespace Director
