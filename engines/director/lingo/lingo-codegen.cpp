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
#include "director/lingo/lingo.h"

#include "director/util.h"

namespace Director {

void Lingo::execute(uint pc) {
	for (_pc = pc; !_returning && (*_currentScript)[_pc] != STOP;) {
		Common::String instr = decodeInstruction(_currentScript, _pc);
		uint current = _pc;

		if (debugChannelSet(5, kDebugLingoExec))
			printStack("Stack before: ", current);

		debugC(1, kDebugLingoExec, "[%3d]: %s", current, instr.c_str());

		_pc++;
		(*((*_currentScript)[_pc - 1]))();

		if (debugChannelSet(5, kDebugLingoExec))
			printStack("Stack after: ", current);

		if (_pc >= (*_currentScript).size()) {
			warning("Lingo::execute(): Bad PC (%d)", _pc);
			break;
		}
	}
}

void Lingo::printStack(const char *s, uint pc) {
	Common::String stack(s);

	for (uint i = 0; i < _stack.size(); i++) {
		Datum d = _stack[i];
		d.toString();
		stack += Common::String::format("<%s> ", d.u.s->c_str());
	}
	debugC(5, kDebugLingoExec, "[%3d]: %s", pc, stack.c_str());
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

Symbol *Lingo::lookupVar(const char *name, bool create, bool putInGlobalList) {
	Symbol *sym = nullptr;

	// Looking for the cast member constants
	if (_vm->getVersion() < 4) { // TODO: There could be a flag 'Allow Outdated Lingo' in Movie Info in D4
		int val = castNumToNum(name);

		if (val != -1) {
			if (!create)
				error("Cast reference used in wrong context: %s", name);

			sym = new Symbol;

			sym->type = CASTREF;
			sym->u.i = val;

			return sym;
		}
	}

	if (!_localvars || !_localvars->contains(name)) { // Create variable if it was not defined
		// Check if it is a global symbol
		if (_globalvars.contains(name) && _globalvars[name]->type == SYMBOL)
			return _globalvars[name];

		if (!create)
			return NULL;

		sym = new Symbol;
		sym->name = name;
		sym->type = VOID;
		sym->u.i = 0;

		if (_localvars)
			(*_localvars)[name] = sym;

		if (putInGlobalList) {
			sym->global = true;
			_globalvars[name] = sym;
		}
	} else {
		sym = (*_localvars)[name];

		if (sym->global)
			sym = _globalvars[name];
	}

	return sym;
}

void Lingo::cleanLocalVars() {
	// Clean up current scope local variables and clean up memory
	debugC(3, kDebugLingoExec, "cleanLocalVars: have %d vars", _localvars->size());

	for (SymbolHash::const_iterator h = _localvars->begin(); h != _localvars->end(); ++h) {
		if (!h->_value->global) {
			delete h->_value;
		}
	}

	delete g_lingo->_localvars;

	g_lingo->_localvars = 0;
}

Symbol *Lingo::define(Common::String &name, int nargs, ScriptData *code) {
	Symbol *sym = getHandler(name);
	if (sym == NULL) { // Create variable if it was not defined
		sym = new Symbol;

		sym->name = name;
		sym->type = HANDLER;

		if (!_eventHandlerTypeIds.contains(name)) {
			_builtins[name] = sym;
		} else {
			_handlers[ENTITY_INDEX(_eventHandlerTypeIds[name.c_str()], _currentEntityId)] = sym;
		}
	} else {
		// we don't want to be here. The getHandler call should have used the EntityId and the result
		// should have been unique!
		warning("Redefining handler '%s'", name.c_str());
		delete sym->u.defn;
	}

	sym->u.defn = code;
	sym->nargs = nargs;
	sym->maxArgs = nargs;

	if (debugChannelSet(1, kDebugLingoCompile)) {
		uint pc = 0;
		while (pc < sym->u.defn->size()) {
			uint spc = pc;
			Common::String instr = g_lingo->decodeInstruction(sym->u.defn, pc, &pc);
			debugC(1, kDebugLingoCompile, "[%5d] %s", spc, instr.c_str());
		}
		debugC(1, kDebugLingoCompile, "<end define code>");
	}

	return sym;
}

Symbol *Lingo::define(Common::String &name, int start, int nargs, Common::String *prefix, int end, bool removeCode) {
	if (prefix)
		name = *prefix + "-" + name;

	debugC(1, kDebugLingoCompile, "define(\"%s\"(len: %d), %d, %d, \"%s\", %d) entity: %d",
			name.c_str(), _currentScript->size() - 1, start, nargs, (prefix ? prefix->c_str() : ""),
			end, _currentEntityId);

	if (end == -1)
		end = _currentScript->size();

	ScriptData *code = new ScriptData(&(*_currentScript)[start], end - start);
	Symbol *sym = define(name, nargs, code);

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

int Lingo::codeArray(int arraySize) {
	inst i = 0;
	WRITE_UINT32(&i, arraySize);
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

void Lingo::codeArgStore() {
	for (int i = _argstack.size() - 1; i >= 0; i--) {
		code1(c_varpush);
		codeString(_argstack[i]->c_str());
		code1(c_assign);
	}
}

int Lingo::codeSetImmediate(bool state) {
	g_lingo->_immediateMode = state;

	int res = g_lingo->code1(g_lingo->c_setImmediate);
	inst i = 0;
	WRITE_UINT32(&i, state);
	g_lingo->code1(i);

	return res;
}

int Lingo::codeFunc(Common::String *s, int numpar) {
	int ret = g_lingo->code1(g_lingo->c_call);

	g_lingo->codeString(s->c_str());

	inst num = 0;
	WRITE_UINT32(&num, numpar);
	g_lingo->code1(num);

	return ret;
}

int Lingo::codeMe(Common::String *method, int numpar) {
	// Check if need to encode reference to the factory
	if (method == nullptr) {
		int ret = g_lingo->code1(g_lingo->c_factory);
		g_lingo->codeString(g_lingo->_currentFactory.c_str());

		return ret;
	}

	int ret = g_lingo->code1(g_lingo->c_call);

	Common::String m(g_lingo->_currentFactory);

	m += '-';
	m += *method;

	g_lingo->codeString(m.c_str());

	inst num = 0;
	WRITE_UINT32(&num, numpar);
	g_lingo->code1(num);

	return ret;
}

void Lingo::codeLabel(int label) {
	_labelstack.push_back(label);
	debugC(4, kDebugLingoCompile, "codeLabel: Added label %d", label);
}

void Lingo::processIf(int startlabel, int endlabel, int finalElse) {
	inst ielse1, iend;
	int  else1 = 0;

	debugC(4, kDebugLingoCompile, "processIf(%d, %d, %d)", startlabel, endlabel, finalElse);

	WRITE_UINT32(&iend, endlabel);

	int finalElsePos = -1;
	bool multiIf = _labelstack.size() > 1;

	while (true) {
		if (_labelstack.empty()) {
			warning("Label stack underflow");
			break;
		}

		int label = _labelstack.back();
		_labelstack.pop_back();

		// This is beginning of our if()
		if (!label)
			break;

		if (else1)
			else1 = else1 - label;

		// Store position of the last 'if', so we could set reference to the
		// 'finalElse' part
		if (finalElse && finalElsePos == -1) {
			finalElsePos = label;
		}

		debugC(4, kDebugLingoCompile, "processIf: %d: %d %d", label, else1 + label, endlabel + label);

		WRITE_UINT32(&ielse1, else1);
		(*_currentScript)[label + 2] = ielse1;    /* elsepart */
		(*_currentScript)[label + 3] = iend;      /* end, if cond fails */

		else1 = label;
	}

	if (multiIf && finalElsePos != -1) {
		debugC(4, kDebugLingoCompile, "processIf: storing %d to %d", finalElse - finalElsePos + startlabel, finalElsePos);
		WRITE_UINT32(&ielse1, finalElse - finalElsePos + startlabel);
		(*_currentScript)[finalElsePos + 2] = ielse1;
	}
}

void Lingo::codeFactory(Common::String &name) {
	_currentFactory = name;

	Symbol *sym = new Symbol;

	sym->name = name;
	sym->type = BLTIN;
	sym->nargs = -1;
	sym->maxArgs = 0;
	sym->parens = true;
	sym->u.bltin = g_lingo->b_factory;

	_handlers[ENTITY_INDEX(_eventHandlerTypeIds[name.c_str()], _currentEntityId)] = sym;
}

}
