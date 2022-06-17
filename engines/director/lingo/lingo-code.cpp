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
#include "director/castmember.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-object.h"
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
	{ LC::c_themenuitementityassign,"c_themenuitementityassign","EF" },
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
	for (FuncHash::iterator it = _functions.begin(); it != _functions.end(); ++it)
		delete it->_value;
}

void Lingo::push(Datum d) {
	_stack.push_back(d);
}

void Lingo::pushVoid() {
	Datum d;
	d.u.s = nullptr;
	d.type = VOID;
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

void Lingo::loadStateFromWindow() {
	Window *window = _vm->getCurrentWindow();
	_pc = window->_retPC;
	_currentScript = window->_retScript;
	_currentScriptContext = window->_retContext;
	_freezeContext = window->_retFreezeContext;
	_localvars = window->_retLocalVars;
	_currentMe = window->_retMe;
}

void Lingo::saveStateToWindow() {
	Window *window = _vm->getCurrentWindow();
	window->_retPC = _pc;
	window->_retScript = _currentScript;
	window->_retContext = _currentScriptContext;
	window->_retFreezeContext = _freezeContext;
	window->_retLocalVars = _localvars;
	window->_retMe = _currentMe;
}

void Lingo::pushContext(const Symbol funcSym, bool allowRetVal, Datum defaultRetVal) {
	Common::Array<CFrame *> &callstack = _vm->getCurrentWindow()->_callstack;

	debugC(5, kDebugLingoExec, "Pushing frame %d", callstack.size() + 1);
	CFrame *fp = new CFrame;

	fp->retPC = g_lingo->_pc;
	fp->retScript = g_lingo->_currentScript;
	fp->retContext = g_lingo->_currentScriptContext;
	fp->retFreezeContext = g_lingo->_freezeContext;
	fp->retLocalVars = g_lingo->_localvars;
	fp->retMe = g_lingo->_currentMe;
	fp->sp = funcSym;
	fp->allowRetVal = allowRetVal;
	fp->defaultRetVal = defaultRetVal;

	g_lingo->_currentScript = funcSym.u.defn;

	if (funcSym.target)
		g_lingo->_currentMe = funcSym.target;

	if (funcSym.ctx) {
		g_lingo->_currentScriptContext = funcSym.ctx;
		*g_lingo->_currentScriptContext->_refCount += 1;
	}
	g_lingo->_freezeContext = false;

	DatumHash *localvars = g_lingo->_localvars;
	if (!funcSym.anonymous) {
		// Execute anonymous functions within the current var frame.
		localvars = new DatumHash;
	}

	if (funcSym.argNames) {
		int symNArgs = funcSym.nargs;
		if ((int)funcSym.argNames->size() < symNArgs) {
			int dropSize = symNArgs - funcSym.argNames->size();
			warning("%d arg names defined for %d args! Dropping the last %d values", funcSym.argNames->size(), symNArgs, dropSize);
			for (int i = 0; i < dropSize; i++) {
				g_lingo->pop();
				symNArgs -= 1;
			}
		} else if ((int)funcSym.argNames->size() > symNArgs) {
			warning("%d arg names defined for %d args! Ignoring the last %d names", funcSym.argNames->size(), symNArgs, funcSym.argNames->size() - symNArgs);
		}
		for (int i = symNArgs - 1; i >= 0; i--) {
			Common::String name = (*funcSym.argNames)[i];
			if (!localvars->contains(name)) {
				Datum value = g_lingo->pop();
				(*localvars)[name] = value;
			} else {
				warning("Argument %s already defined", name.c_str());
				g_lingo->pop();
			}
		}
	}
	if (funcSym.varNames) {
		for (Common::Array<Common::String>::iterator it = funcSym.varNames->begin(); it != funcSym.varNames->end(); ++it) {
			Common::String name = *it;
			if (!localvars->contains(name)) {
				(*localvars)[name] = Datum();
			} else {
				warning("Variable %s already defined", name.c_str());
			}
		}
	}
	g_lingo->_localvars = localvars;

	fp->stackSizeBefore = _stack.size();

	callstack.push_back(fp);

	if (debugChannelSet(2, kDebugLingoExec)) {
		g_lingo->printCallStack(0);
	}
}

void Lingo::popContext(bool aborting) {
	Common::Array<CFrame *> &callstack = _vm->getCurrentWindow()->_callstack;

	debugC(5, kDebugLingoExec, "Popping frame %d", callstack.size());
	CFrame *fp = callstack.back();
	callstack.pop_back();

	if (_stack.size() == fp->stackSizeBefore + 1) {
		if (!fp->allowRetVal) {
			debugC(5, kDebugLingoExec, "dropping return value");
			g_lingo->pop();
		}
	} else if (_stack.size() == fp->stackSizeBefore) {
		if (fp->allowRetVal) {
			// Don't warn about missing return value if there's an explicit, non-VOID default,
			// e.g. for factories' mNew method.
			if (fp->defaultRetVal.type == VOID) {
				warning("handler %s did not return value", fp->sp.name->c_str());
			}
			g_lingo->push(fp->defaultRetVal);
		}
	} else if (_stack.size() > fp->stackSizeBefore) {
		if (aborting) {
			// Since we're aborting execution, we should expect that some extra
			// values are left on the stack.
			while (_stack.size() > fp->stackSizeBefore) {
				g_lingo->pop();
			}
		} else {
			error("handler %s returned extra %d values", fp->sp.name->c_str(), _stack.size() - fp->stackSizeBefore);
		}
	} else {
		error("handler %s popped extra %d values", fp->sp.name->c_str(), fp->stackSizeBefore - _stack.size());
	}

	*g_lingo->_currentScriptContext->_refCount -= 1;
	if (*g_lingo->_currentScriptContext->_refCount <= 0) {
		delete g_lingo->_currentScriptContext;
	}

	g_lingo->_currentScript = fp->retScript;
	g_lingo->_currentScriptContext = fp->retContext;
	g_lingo->_freezeContext = fp->retFreezeContext;
	g_lingo->_pc = fp->retPC;
	g_lingo->_currentMe = fp->retMe;

	// Restore local variables
	if (!fp->sp.anonymous) {
		g_lingo->cleanLocalVars();
		g_lingo->_localvars = fp->retLocalVars;
	}

	if (debugChannelSet(2, kDebugLingoExec)) {
		g_lingo->printCallStack(g_lingo->_pc);
	}

	delete fp;
}

bool Lingo::hasFrozenContext() {
	if (g_lingo->_freezeContext)
		return true;

	Common::Array<CFrame *> &callstack = _vm->getCurrentWindow()->_callstack;
	for (uint i = 0; i < callstack.size(); i++) {
		if (callstack[i]->retFreezeContext)
			return true;
	}

	return false;
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

	if (entity != kTheMenuItems) { // "<entity> of menuitems" has 1 parameter
		menuItemId = g_lingo->pop();
	}

	Datum d = g_lingo->getTheMenuItemEntity(entity, menuId, field, menuItemId);
	g_lingo->push(d);
}

void LC::c_theentityassign() {
	Datum id = g_lingo->pop();

	int entity = g_lingo->readInt();
	int field  = g_lingo->readInt();

	Datum d = g_lingo->pop();
	g_lingo->setTheEntity(entity, id, field, d);
}

void LC::c_themenuitementityassign() {
	int entity = g_lingo->readInt();
	int field  = g_lingo->readInt();

	Datum d = g_lingo->pop();
	Datum menuId = g_lingo->pop();
	Datum menuItemId;

	if (entity != kTheMenuItems) { // "<entity> of menuitems" has 2 parameters
		menuItemId = g_lingo->pop();
	}

	g_lingo->setTheMenuItemEntity(entity, menuId, field, menuItemId, d);
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

static bool isArray(Datum &d1) {
	if (d1.type == ARRAY || d1.type == POINT || d1.type == RECT)
		return true;

	return false;
}

static DatumType getArrayAlignedType(Datum &d1, Datum &d2) {
	if (d1.type == POINT && d2.type == ARRAY && d2.u.farr->arr.size() < 2)
		return ARRAY;

	if (d1.type == POINT)
		return POINT;

	if (d1.type == RECT && (d2.type == POINT || (d2.type == ARRAY && d2.u.farr->arr.size() < 4)))
		return ARRAY;

	if (d1.type == RECT)
		return RECT;

	if (!isArray(d1))
		return d2.type;

	return ARRAY;
}

Datum LC::mapBinaryOp(Datum (*mapFunc)(Datum &, Datum &), Datum &d1, Datum &d2) {
	// At least one of d1 and d2 must be an array
	uint arraySize;
	if (isArray(d1) && isArray(d2)) {
		arraySize = MIN(d1.u.farr->arr.size(), d2.u.farr->arr.size());
	} else if (isArray(d1)) {
		arraySize = d1.u.farr->arr.size();
	} else {
		arraySize = d2.u.farr->arr.size();
	}
	Datum res;
	res.type = getArrayAlignedType(d1, d2);
	res.u.farr = new FArray(arraySize);
	Datum a = d1;
	Datum b = d2;
	for (uint i = 0; i < arraySize; i++) {
		if (isArray(d1)) {
			a = d1.u.farr->arr[i];
		}
		if (isArray(d2)) {
			b = d2.u.farr->arr[i];
		}
		res.u.farr->arr[i] = mapFunc(a, b);
	}
	return res;
}

Datum LC::addData(Datum &d1, Datum &d2) {
	if (isArray(d1) || isArray(d2)) {
		return LC::mapBinaryOp(LC::addData, d1, d2);
	}

	int alignedType = g_lingo->getAlignedType(d1, d2, true);

	Datum res;
	if (alignedType == FLOAT) {
		res = Datum(d1.asFloat() + d2.asFloat());
	} else if (alignedType == INT) {
		res = Datum(d1.asInt() + d2.asInt());
	} else {
		warning("LC::addData(): not supported between types %s and %s", d1.type2str(), d2.type2str());
	}
	return res;
}

void LC::c_add() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::addData(d1, d2));
}

Datum LC::subData(Datum &d1, Datum &d2) {
	if (isArray(d1) || isArray(d2)) {
		return LC::mapBinaryOp(LC::subData, d1, d2);
	}

	int alignedType = g_lingo->getAlignedType(d1, d2, true);

	Datum res;
	if (alignedType == FLOAT) {
		res = Datum(d1.asFloat() - d2.asFloat());
	} else if (alignedType == INT) {
		res = Datum(d1.asInt() - d2.asInt());
	} else {
		warning("LC::subData(): not supported between types %s and %s", d1.type2str(), d2.type2str());
	}
	return res;
}

void LC::c_sub() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::subData(d1, d2));
}

Datum LC::mulData(Datum &d1, Datum &d2) {
	if (isArray(d1) || isArray(d2)) {
		return LC::mapBinaryOp(LC::mulData, d1, d2);
	}

	int alignedType = g_lingo->getAlignedType(d1, d2, true);

	Datum res;
	if (alignedType == FLOAT) {
		res = Datum(d1.asFloat() * d2.asFloat());
	} else if (alignedType == INT) {
		res = Datum(d1.asInt() * d2.asInt());
	} else {
		warning("LC::mulData(): not supported between types %s and %s", d1.type2str(), d2.type2str());
	}
	return res;
}

void LC::c_mul() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::mulData(d1, d2));
}

Datum LC::divData(Datum &d1, Datum &d2) {
	if (isArray(d1) || isArray(d2)) {
		return LC::mapBinaryOp(LC::divData, d1, d2);
	}

	if ((d2.type == INT && d2.u.i == 0) ||
			(d2.type == FLOAT && d2.u.f == 0.0)) {
		warning("LC::divData(): division by zero");
		d2 = Datum(1);
	}

	int alignedType = g_lingo->getAlignedType(d1, d2, true);

	if (g_director->getVersion() < 400)	// pre-D4 is INT-only
		alignedType = INT;

	Datum res;
	if (alignedType == FLOAT) {
		res = Datum(d1.asFloat() / d2.asFloat());
	} else if (alignedType == INT) {
		res = Datum(d1.asInt() / d2.asInt());
	} else {
		warning("LC::divData(): not supported between types %s and %s", d1.type2str(), d2.type2str());
	}

	return res;
}

void LC::c_div() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(divData(d1, d2));
}

Datum LC::modData(Datum &d1, Datum &d2) {
	if (isArray(d1) || isArray(d2)) {
		return LC::mapBinaryOp(LC::modData, d1, d2);
	}

	int i1 = d1.asInt();
	int i2 = d2.asInt();
	if (i2 == 0) {
		warning("LC::modData(): division by zero");
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
	if (isArray(d)) {
		uint arraySize = d.u.farr->arr.size();
		Datum res;
		res.type = d.type;
		res.u.farr = new FArray(arraySize);
		for (uint i = 0; i < arraySize; i++) {
			res.u.farr->arr[i] = LC::negateData(d.u.farr->arr[i]);
		}
		return res;
	}

	Datum res = d;
	if (res.type == INT) {
		res.u.i = -res.u.i;
	} else if (res.type == FLOAT) {
		res.u.f = -res.u.f;
	} else {
		warning("LC::negateData(): not supported for type %s", res.type2str());
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

	d1.type = INT;
	d1.u.i = res;

	g_lingo->push(d1);
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
	Datum d = g_lingo->pop();

	Datum res(d.asInt() == 0 ? 1 : 0);

	g_lingo->push(res);
}

Datum LC::compareArrays(Datum (*compareFunc)(Datum, Datum), Datum d1, Datum d2, bool location, bool value) {
	// At least one of d1 and d2 must be an array
	uint arraySize;
	if (d1.type == ARRAY && d2.type == ARRAY) {
		arraySize = MIN(d1.u.farr->arr.size(), d2.u.farr->arr.size());
	} else if (d1.type == PARRAY && d2.type == PARRAY) {
		arraySize = MIN(d1.u.parr->arr.size(), d2.u.parr->arr.size());
	} else if (d1.type == ARRAY) {
		arraySize = d1.u.farr->arr.size();
	} else if (d1.type == PARRAY) {
		arraySize = d1.u.parr->arr.size();
	} else if (d2.type == ARRAY) {
		arraySize = d2.u.farr->arr.size();
	} else if (d2.type == PARRAY) {
		arraySize = d2.u.parr->arr.size();
	} else {
		warning("LC::compareArrays(): Called with wrong data types: %s and %s", d1.type2str(), d2.type2str());
		return Datum(0);
	}

	Datum res;
	res.type = INT;
	res.u.i = location ? -1 : 1;
	Datum a = d1;
	Datum b = d2;
	for (uint i = 0; i < arraySize; i++) {
		if (d1.type == ARRAY) {
			a = d1.u.farr->arr[i];
		} else if (d1.type == PARRAY) {
			PCell t = d1.u.parr->arr[i];
			a = value ? t.v : t.p;
		}

		if (d2.type == ARRAY) {
			b = d2.u.farr->arr[i];
		} else if (d2.type == PARRAY) {
			PCell t = d2.u.parr->arr[i];
			b = value ? t.v : t.p;
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
	// Lingo doesn't bother checking list equality if the left is longer
	if (d1.type == ARRAY && d2.type == ARRAY &&
			d1.u.farr->arr.size() > d2.u.farr->arr.size()) {
		return Datum(0);
	}
	if (d1.type == PARRAY && d2.type == PARRAY &&
			d1.u.parr->arr.size() > d2.u.parr->arr.size()) {
		return Datum(0);
	}
	if (d1.type == ARRAY || d2.type == ARRAY ||
			d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::eqData, d1, d2, false, true);
	}
	d1.u.i = d1.equalTo(d2, true);
	d1.type = INT;
	return d1;
}

void LC::c_eq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::eqData(d1, d2));
}

Datum LC::neqData(Datum d1, Datum d2) {
	if (d1.type == ARRAY || d2.type == ARRAY ||
			d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::neqData, d1, d2, false, true);
	}
	d1.u.i = !d1.equalTo(d2, true);
	d1.type = INT;
	return d1;
}

void LC::c_neq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::neqData(d1, d2));
}

Datum LC::gtData(Datum d1, Datum d2) {
	if (d1.type == ARRAY || d2.type == ARRAY ||
			d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::gtData, d1, d2, false, true);
	}
	d1.u.i = d1 > d2 ? 1 : 0;
	d1.type = INT;
	return d1;
}

void LC::c_gt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::gtData(d1, d2));
}

Datum LC::ltData(Datum d1, Datum d2) {
	if (d1.type == ARRAY || d2.type == ARRAY ||
			d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::ltData, d1, d2, false, true);
	}
	d1.u.i = d1 < d2 ? 1 : 0;
	d1.type = INT;
	return d1;
}

void LC::c_lt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::ltData(d1, d2));
}

Datum LC::geData(Datum d1, Datum d2) {
	if (d1.type == ARRAY || d2.type == ARRAY ||
			d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::geData, d1, d2, false, true);
	}
	d1.u.i = d1 >= d2 ? 1 : 0;
	d1.type = INT;
	return d1;
}

void LC::c_ge() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::geData(d1, d2));
}

Datum LC::leData(Datum d1, Datum d2) {
	if (d1.type == ARRAY || d2.type == ARRAY ||
			d1.type == PARRAY || d2.type == PARRAY) {
		return LC::compareArrays(LC::leData, d1, d2, false, true);
	}
	d1.u.i = d1 <= d2 ? 1 : 0;
	d1.type = INT;
	return d1;
}

void LC::c_le() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(LC::leData(d1, d2));
}

void LC::c_jump() {
	int jump = g_lingo->readInt();
	g_lingo->_pc = g_lingo->_pc + jump - 2;
}

void LC::c_jumpifz() {
	int jump = g_lingo->readInt();
	int test = g_lingo->pop().asInt();
	if (test == 0) {
		g_lingo->_pc = g_lingo->_pc + jump - 2;
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
	if (static_cast<Window *>(window.u.obj)->getCurrentMovie() == nullptr) {
		warning("LC::c_tell(): window has no movie");
		return;
	}
	g_director->setCurrentWindow(static_cast<Window *>(window.u.obj));

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
		printWithArgList(name.c_str(), nargs, "call:");

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
				g_lingo->_stack[g_lingo->_stack.size() - nargs] = funcSym.target; // Set first arg to target
				call(funcSym, nargs, allowRetVal);
				return;
			}
		}
	}

	// Handler
	funcSym = g_lingo->getHandler(name);

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
	Datum target = funcSym.target;

	if (funcSym.type == VOIDSYM) {
		if (funcSym.name)
			g_lingo->lingoError("Call to undefined handler '%s'. Dropping %d stack items", funcSym.name->c_str(), nargs);
		else
			g_lingo->lingoError("Call to undefined handler. Dropping %d stack items", nargs);

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
			if (funcSym.maxArgs < nargs) {
				warning("Incorrect number of arguments for handler %s (%d, expected %d to %d). Dropping extra %d",
							funcSym.name->c_str(), nargs, funcSym.nargs, funcSym.maxArgs, nargs - funcSym.maxArgs);
				while (nargs > funcSym.maxArgs) {
					g_lingo->pop();
					nargs--;
				}
			}
			if (funcSym.nargs > nargs) {
				warning("Incorrect number of arguments for handler %s (%d, expected %d to %d). Adding extra %d voids",
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
			warning("Incorrect number of arguments for builtin %s (%d, expected %d to %d). Dropping %d stack items.",
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
		uint stackSizeBefore = g_lingo->_stack.size() - nargs;

		if (target.type != VOID) {
			// Only need to update the me obj
			// Pushing an entire stack frame is not necessary
			Datum retMe = g_lingo->_currentMe;
			g_lingo->_currentMe = target;
			(*funcSym.u.bltin)(nargs);
			g_lingo->_currentMe = retMe;
		} else {
			(*funcSym.u.bltin)(nargs);
		}

		uint stackSize = g_lingo->_stack.size();

		if (funcSym.u.bltin != LB::b_return && funcSym.u.bltin != LB::b_value) {
			if (stackSize == stackSizeBefore + 1) {
				if (!allowRetVal) {
					warning("dropping return value");
					g_lingo->pop();
				}
			} else if (stackSize == stackSizeBefore) {
				if (allowRetVal)
					error("builtin function %s did not return value", funcSym.name->c_str());
			} else if (stackSize > stackSizeBefore) {
				error("builtin %s returned extra %d values", funcSym.name->c_str(), stackSize - stackSizeBefore);
			} else {
				error("builtin %s popped extra %d values", funcSym.name->c_str(), stackSizeBefore - stackSize);
			}
		}
		return;
	}

	Datum defaultRetVal;
	if (funcSym.target && funcSym.target->getObjType() == kFactoryObj && funcSym.name->equalsIgnoreCase("mNew")) {
		defaultRetVal = funcSym.target; // return me
	}

	g_lingo->pushContext(funcSym, allowRetVal, defaultRetVal);

	g_lingo->_pc = 0;
}

void LC::c_procret() {
	Common::Array<CFrame *> &callstack = g_director->getCurrentWindow()->_callstack;

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
			// last char of the first portion is the delimiter. skip it.
			if (start > 0)
				start--;
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
	Datum d = g_lingo->pop();
	Datum res = d.asMemberID();
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
		warning("c_asserterrordone: did not catch error");
	}
	g_lingo->_expectError = false;
}

} // End of namespace Director
