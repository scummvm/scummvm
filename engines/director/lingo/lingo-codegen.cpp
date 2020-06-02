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

#include "director/director.h"
#include "director/cast.h"
#include "director/score.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"

#include "director/util.h"

namespace Director {

void Lingo::execute(uint pc) {
	int counter = 0;

	for (_pc = pc; !_abort && (*_currentScript)[_pc] != STOP && !_nextRepeat;) {
		Common::String instr = decodeInstruction(_currentScript, _pc);
		uint current = _pc;

		if (debugChannelSet(5, kDebugLingoExec))
			printStack("Stack before: ", current);

		if (debugChannelSet(9, kDebugLingoExec)) {
			debug("Vars before");
			printAllVars();
			if (_currentMeObj)
				debug("me: %s", _currentMeObj->name->c_str());
		}

		debugC(1, kDebugLingoExec, "[%3d]: %s", current, instr.c_str());

		_pc++;
		(*((*_currentScript)[_pc - 1]))();

		if (debugChannelSet(5, kDebugLingoExec))
			printStack("Stack after: ", current);

		if (debugChannelSet(9, kDebugLingoExec)) {
			debug("Vars after");
			printAllVars();
		}

		if (_pc >= (*_currentScript).size()) {
			warning("Lingo::execute(): Bad PC (%d)", _pc);
			break;
		}

		if (++counter > 1000 && debugChannelSet(-1, kDebugFewFramesOnly)) {
			warning("Lingo::execute(): Stopping due to debug few frames only");
			break;
		}
	}

	_abort = false;
}

void Lingo::printStack(const char *s, uint pc) {
	Common::String stack(s);

	for (uint i = 0; i < _stack.size(); i++) {
		Datum d = _stack[i];
		stack += Common::String::format("<%s> ", d.asString(true).c_str());
	}
	debugC(5, kDebugLingoExec, "[%3d]: %s", pc, stack.c_str());
}

void Lingo::printCallStack(uint pc) {
	debugC(5, kDebugLingoExec, "Call stack:");
	for (int i = 0; i < (int)g_lingo->_callstack.size(); i++) {
		CFrame *frame = g_lingo->_callstack[i];
		uint framePc = pc;
		if (i < (int)g_lingo->_callstack.size() - 1)
			framePc = g_lingo->_callstack[i + 1]->retpc;

		if (frame->sp.type != VOID) {
			debugC(5, kDebugLingoExec, "#%d %s:%d", i + 1,
				g_lingo->_callstack[i]->sp.name->c_str(),
				framePc
			);
		} else {
			debugC(5, kDebugLingoExec, "#%d [unknown]:%d", i + 1,
				framePc
			);
		}
	}
}

Common::String Lingo::decodeInstruction(ScriptData *sd, uint pc, uint *newPc) {
	Symbol sym;
	Common::String res;

	sym.u.func = (*sd)[pc++];
	if (_functions.contains((void *)sym.u.s)) {
		res = _functions[(void *)sym.u.s]->name;
		const char *pars = _functions[(void *)sym.u.s]->proto;
		inst i;
		uint start = pc;

		while (*pars) {
			switch (*pars++) {
			case 'i':
				{
					i = (*sd)[pc++];
					int v = READ_UINT32(&i);

					res += Common::String::format(" %d", v);
					break;
				}
			case 'f':
				{
					Datum d;
					i = (*sd)[pc++];
					d.u.f = *(double *)(&i);

					res += Common::String::format(" %f", d.u.f);
					break;
				}
			case 'o':
				{
					i = (*sd)[pc++];
					int v = READ_UINT32(&i);

					res += Common::String::format(" [%5d]", v + start - 1);
					break;
				}
			case 's':
				{
					char *s = (char *)&(*sd)[pc];
					pc += calcStringAlignment(s);

					res += Common::String::format(" \"%s\"", s);
					break;
				}
			case 'E':
				{
					i = (*sd)[pc++];
					int v = READ_UINT32(&i);

					res += Common::String::format(" %s", entity2str(v));
					break;
				}
			case 'F':
				{
					i = (*sd)[pc++];
					int v = READ_UINT32(&i);

					res += Common::String::format(" %s", field2str(v));
					break;
				}
			case 'N':
				{
					i = (*sd)[pc++];
					int v = READ_UINT32(&i);

					res += Common::String::format(" \"%s\"", getName(v).c_str());
					break;
				}
			default:
				warning("decodeInstruction: Unknown parameter type: %c", pars[-1]);
			}

			if (*pars)
				res += ',';
		}
	} else {
		res = "<unknown>";
	}

	if (newPc)
		*newPc = pc;

	return res;
}

void Lingo::cleanLocalVars() {
	// Clean up current scope local variables and clean up memory
	debugC(3, kDebugLingoExec, "cleanLocalVars: have %d vars", _localvars->size());

	g_lingo->_localvars->clear();
	delete g_lingo->_localvars;

	g_lingo->_localvars = nullptr;
}

Symbol Lingo::define(Common::String &name, int nargs, ScriptData *code, Common::Array<Common::String> *argNames, Common::Array<Common::String> *varNames, Object *factory) {
	Symbol sym;
	sym.name = new Common::String(name);
	sym.type = HANDLER;
	sym.u.defn = code;
	sym.nargs = nargs;
	sym.maxArgs = nargs;
	sym.argNames = argNames;
	sym.varNames = varNames;
	sym.ctx = _currentScriptContext;
	sym.archiveIndex = _archiveIndex;

	if (debugChannelSet(1, kDebugLingoCompile)) {
		uint pc = 0;
		while (pc < sym.u.defn->size()) {
			uint spc = pc;
			Common::String instr = g_lingo->decodeInstruction(sym.u.defn, pc, &pc);
			debugC(1, kDebugLingoCompile, "[%5d] %s", spc, instr.c_str());
		}
		debugC(1, kDebugLingoCompile, "<end define code>");
	}

	if (factory) {
		if (factory->methods.contains(name)) {
			warning("Redefining method '%s' on factory '%s'", name.c_str(), factory->name->c_str());
		}
		factory->methods[name] = sym;

		// FIXME: Method names can conflict with vars. This won't work all the time.
		Datum target(name);
		target.type = VAR;
		Datum source(name);
		source.type = SYMBOL;
		g_lingo->varCreate(name, true);
		g_lingo->varAssign(target, source, true);
	} else {
		Symbol existing = getHandler(name);
		if (existing.type != VOID)
			warning("Redefining handler '%s'", name.c_str());

		if (!_eventHandlerTypeIds.contains(name)) {
			_archives[_archiveIndex].functionHandlers[name] = sym;
		} else {
			_archives[_archiveIndex].eventHandlers[ENTITY_INDEX(_eventHandlerTypeIds[name.c_str()], _currentEntityId)] = sym;
		}
	}

	return sym;
}

Symbol Lingo::codeDefine(Common::String &name, int start, int nargs, Object *factory, int end, bool removeCode) {
	debugC(1, kDebugLingoCompile, "codeDefine(\"%s\"(len: %d), %d, %d, \"%s\", %d) entity: %d",
			name.c_str(), _currentScript->size() - 1, start, nargs, (factory ? factory->name->c_str() : ""),
			end, _currentEntityId);

	if (end == -1)
		end = _currentScript->size();

	ScriptData *code = new ScriptData(&(*_currentScript)[start], end - start);
	Common::Array<Common::String> *argNames = new Common::Array<Common::String>;
	for (uint i = 0; i < _argstack.size(); i++) {
		argNames->push_back(Common::String(_argstack[i]->c_str()));
	}
	Common::Array<Common::String> *varNames = new Common::Array<Common::String>;
	for (Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator it = _methodVars->begin(); it != _methodVars->end(); ++it) {
		if (it->_value == kVarLocal)
			varNames->push_back(Common::String(it->_key));
	}
	Symbol sym = define(name, nargs, code, argNames, varNames, factory);

	// Now remove all defined code from the _currentScript
	if (removeCode)
		for (int i = end - 1; i >= start; i--) {
			_currentScript->remove_at(i);
		}

	return sym;
}

int Lingo::codeString(const char *str) {
	int numInsts = calcStringAlignment(str);

	// Where we copy the string over
	int pos = _currentScript->size();

	// Allocate needed space in script
	for (int i = 0; i < numInsts; i++)
		_currentScript->push_back(0);

	byte *dst = (byte *)&_currentScript->front() + pos * sizeof(inst);

	memcpy(dst, str, strlen(str) + 1);

	return _currentScript->size();
}

int Lingo::codeFloat(double f) {
	int numInsts = calcCodeAlignment(sizeof(double));

	// Where we copy the string over
	int pos = _currentScript->size();

	// Allocate needed space in script
	for (int i = 0; i < numInsts; i++)
		_currentScript->push_back(0);

	double *dst = (double *)((byte *)&_currentScript->front() + pos * sizeof(inst));

	*dst = f;

	return _currentScript->size();
}

int Lingo::codeInt(int val) {
	inst i = 0;
	WRITE_UINT32(&i, val);
	g_lingo->code1(i);

	return _currentScript->size();
}

bool Lingo::isInArgStack(Common::String *s) {
	for (uint i = 0; i < _argstack.size(); i++)
		if (_argstack[i]->equalsIgnoreCase(*s))
			return true;

	return false;
}

void Lingo::codeArg(Common::String *s) {
	_argstack.push_back(new Common::String(*s));
}

void Lingo::clearArgStack() {
	for (uint i = 0; i < _argstack.size(); i++)
		delete _argstack[i];

	_argstack.clear();
}

int Lingo::codeSetImmediate(bool state) {
	g_lingo->_immediateMode = state;

	int res = g_lingo->code1(LC::c_setImmediate);
	inst i = 0;
	WRITE_UINT32(&i, state);
	g_lingo->code1(i);

	return res;
}

int Lingo::codeFunc(Common::String *s, int numpar) {
	int ret = g_lingo->code1(LC::c_call);

	g_lingo->codeString(s->c_str());

	inst num = 0;
	WRITE_UINT32(&num, numpar);
	g_lingo->code1(num);

	return ret;
}

// int Lingo::codeMe(Common::String *method, int numpar) {
// 	// Check if need to encode reference to the factory
// 	if (method == nullptr) {
// 		int ret = g_lingo->code1(LC::c_factory);
// 		g_lingo->codeString(g_lingo->_currentFactory->name->c_str());

// 		return ret;
// 	}

// 	int ret = g_lingo->code1(LC::c_call);

// 	Common::String m(g_lingo->_currentFactory->name);

// 	m += '-';
// 	m += *method;

// 	g_lingo->codeString(m.c_str());

// 	inst num = 0;
// 	WRITE_UINT32(&num, numpar);
// 	g_lingo->code1(num);

// 	return ret;
// }

void Lingo::codeLabel(int label) {
	_labelstack.push_back(label);
	debugC(4, kDebugLingoCompile, "codeLabel: Added label %d", label);
}

void Lingo::processIf(int toplabel, int endlabel) {
	inst iend;

	debugC(4, kDebugLingoCompile, "processIf(%d, %d)", toplabel, endlabel);

	while (true) {
		if (_labelstack.empty()) {
			warning("Lingo::processIf(): Label stack underflow");
			break;
		}

		int label = _labelstack.back();
		_labelstack.pop_back();

		// This is beginning of our if()
		if (!label)
			break;

		debugC(4, kDebugLingoCompile, "processIf: label at %d", label);

		WRITE_UINT32(&iend, endlabel - label + 1);

		(*_currentScript)[label] = iend;      /* end, if cond fails */
	}
}

int Lingo::castIdFetch(Datum &var) {
	Score *score = _vm->getCurrentScore();
	if (!score) {
		warning("castIdFetch: Score is empty");
		return 0;
	}

	int id = 0;
	if (var.type == STRING) {
		if (score->_castsNames.contains(*var.u.s))
			id = score->_castsNames[*var.u.s];
		else
			warning("castIdFetch: reference to non-existent cast member: %s", var.u.s->c_str());
	} else if (var.type == INT || var.type == FLOAT) {
		int castId = var.asInt();
		if (!_vm->getCastMember(castId))
			warning("castIdFetch: reference to non-existent cast ID: %d", castId);
		else
			id = castId;
	} else if (var.type == VOID) {
		warning("castIdFetch: reference to VOID cast ID");
		return 0;
	} else {
		error("castIdFetch: was expecting STRING or INT, got %s", var.type2str());
	}

	return id;
}

void Lingo::varCreate(const Common::String &name, bool global) {
	if (_localvars && _localvars->contains(name)) {
		if (global)
			warning("varCreate: variable %s is local, not global", name.c_str());
		return;
	} else if (_currentMeObj && _currentMeObj->hasVar(name)) {
		if (global)
			warning("varCreate: variable %s is instance or property, not global", name.c_str());
		return;
	} else  if (_globalvars.contains(name)) {
		if (!global)
			warning("varCreate: variable %s is global, not local", name.c_str());
		return;
	}

	if (global) {
		_globalvars[name] = Symbol();
		_globalvars[name].name = new Common::String(name);
	} else {
		(*_localvars)[name] = Symbol();
		(*_localvars)[name].name = new Common::String(name);
	}
}

void Lingo::varAssign(Datum &var, Datum &value, bool global) {
	if (var.type != VAR && var.type != REFERENCE) {
		warning("varAssign: assignment to non-variable");
		return;
	}

	if (var.type == VAR) {
		Symbol *sym = nullptr;
		Common::String name = *var.u.s;

		if (_localvars && _localvars->contains(name)) {
			sym = &(*_localvars)[name];
			if (global)
				warning("varAssign: variable %s is local, not global", name.c_str());
		} else if (_currentMeObj && _currentMeObj->hasVar(name)) {
			sym = &_currentMeObj->getVar(name);
			if (global)
				warning("varAssign: variable %s is instance or property, not global", sym->name->c_str());
		} else if (_globalvars.contains(name)) {
			sym = &_globalvars[name];
			if (!global)
				warning("varAssign: variable %s is global, not local", name.c_str());
		}

		if (!sym) {
			warning("varAssign: variable %s not defined", name.c_str());
			return;
		}

		if (sym->type != INT && sym->type != VOID &&
				sym->type != FLOAT && sym->type != STRING &&
				sym->type != ARRAY && sym->type != PARRAY) {
			warning("varAssign: assignment to non-variable '%s'", sym->name->c_str());
			return;
		}

		sym->reset();
		sym->refCount = value.refCount;
		*sym->refCount += 1;
		sym->name = new Common::String(name);
		sym->type = value.type;
		if (value.type == INT) {
			sym->u.i = value.u.i;
		} else if (value.type == FLOAT) {
			sym->u.f = value.u.f;
		} else if (value.type == STRING || value.type == SYMBOL || value.type == OBJECT) {
			sym->u.s = value.u.s;
		} else if (value.type == POINT || value.type == ARRAY) {
			sym->u.farr = value.u.farr;
		} else if (value.type == PARRAY) {
			sym->u.parr = value.u.parr;
		} else if (value.type == VOID) {
			sym->u.i = 0;
		} else {
			warning("varAssign: unhandled type: %s", value.type2str());
			sym->u.s = value.u.s;
		}
	} else if (var.type == REFERENCE) {
		Score *score = g_director->getCurrentScore();
		if (!score) {
			warning("varAssign: Assigning to a reference to an empty score");
			return;
		}
		int referenceId = var.u.i;
		Cast *member = g_director->getCastMember(referenceId);
		if (!member) {
			warning("varAssign: Unknown cast id %d", referenceId);
			return;
		}
		switch (member->_type) {
		case kCastText:
			((TextCast *)member)->setText(value.asString().c_str());
			break;
		default:
			warning("varAssign: Unhandled cast type %s", tag2str(member->_type));
			break;
		}
	}
}

Datum Lingo::varFetch(Datum &var, bool global) {
	Datum result;
	result.type = VOID;
	if (var.type != VAR && var.type != REFERENCE) {
		warning("varFetch: fetch from non-variable");
		return result;
	}

	if (var.type == VAR) {
		Symbol *sym = nullptr;
		Common::String name = *var.u.s;

		if (_currentMeObj != nullptr && name.equalsIgnoreCase("me")) {
			result.type = OBJECT;
			result.u.obj = _currentMeObj;
			return result;
		}
		if (_localvars && _localvars->contains(name)) {
			sym = &(*_localvars)[name];
			if (global)
				warning("varFetch: variable %s is local, not global", sym->name->c_str());
		} else if (_currentMeObj && _currentMeObj->hasVar(name)) {
			sym = &_currentMeObj->getVar(name);
			if (global)
				warning("varFetch: variable %s is instance or property, not global", sym->name->c_str());
		} else if (_globalvars.contains(name)) {
			sym = &_globalvars[name];
			if (!global)
				warning("varFetch: variable %s is global, not local", sym->name->c_str());
		}

		if (!sym) {
			warning("varFetch: variable %s not found", name.c_str());
			return result;
		}

		result.type = sym->type;
		delete result.refCount;
		result.refCount = sym->refCount;
		*result.refCount += 1;

		if (sym->type == INT)
			result.u.i = sym->u.i;
		else if (sym->type == FLOAT)
			result.u.f = sym->u.f;
		else if (sym->type == STRING || sym->type == SYMBOL || sym->type == OBJECT)
			result.u.s = sym->u.s;
		else if (sym->type == POINT || sym->type == ARRAY)
			result.u.farr = sym->u.farr;
		else if (sym->type == PARRAY)
			result.u.parr = sym->u.parr;
		else if (sym->type == VOID)
			result.u.i = 0;
		else {
			warning("varFetch: unhandled type: %s", var.type2str());
			result.type = VOID;
		}

	} else if (var.type == REFERENCE) {
		Cast *cast = _vm->getCastMember(var.u.i);
		if (cast) {
			switch (cast->_type) {
			case kCastText:
				result.type = STRING;
				result.u.s = new Common::String(((TextCast *)cast)->getText());
				break;
			default:
				warning("varFetch: Unhandled cast type %s", tag2str(cast->_type));
				break;
			}
		} else {
			warning("varFetch: Unknown cast id %d", var.u.i);
		}

	}

	return result;
}

void Lingo::codeFactory(Common::String &name) {
	Object *obj = new Object;
	obj->name = new Common::String(name);
	obj->type = kFactoryObj;
	obj->inheritanceLevel = 1;
	obj->scriptContext = _currentScriptContext;
	obj->objArray = new Common::HashMap<uint32, Datum>; 

	_currentFactory = obj;
	if (!_globalvars.contains(name)) {
		_globalvars[name] = Symbol();
		_globalvars[name].name = new Common::String(name);
		_globalvars[name].global = true;
		_globalvars[name].type = OBJECT;
		_globalvars[name].u.obj = obj;
	} else {
		warning("Factory '%s' already defined", name.c_str());
	}
}

}
