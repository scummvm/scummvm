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

#include "director/lingo/lingo.h"
#include "common/file.h"
#include "audio/decoders/wave.h"

#include "director/lingo/lingo-gr.h"
#include "director/util.h"

namespace Director {

void Lingo::execute(uint pc) {
	for(_pc = pc; (*_currentScript)[_pc] != STOP && !_returning;) {
		Common::String instr = decodeInstruction(_pc);

		if (debugChannelSet(5, kDebugLingoExec))
			printStack("Stack before: ");

		debugC(1, kDebugLingoExec, "[%3d]: %s", _pc, instr.c_str());

		_pc++;
		(*((*_currentScript)[_pc - 1]))();

		if (debugChannelSet(5, kDebugLingoExec))
			printStack("Stack after: ");
	}
}

void Lingo::printStack(const char *s) {
	Common::String stack(s);

	for (uint i = 0; i < _stack.size(); i++) {
		Datum d = _stack[i];
		d.toString();
		stack += Common::String::format("<%s> ", d.u.s->c_str());
	}
	debugC(5, kDebugLingoExec, "%s", stack.c_str());
}

Common::String Lingo::decodeInstruction(uint pc, uint *newPc) {
	Symbol sym;
	Common::String res;

	sym.u.func = (*_currentScript)[pc++];
	if (_functions.contains((void *)sym.u.s)) {
		res = _functions[(void *)sym.u.s]->name;
		const char *pars = _functions[(void *)sym.u.s]->proto;
		inst i;

		while (*pars) {
			switch (*pars++) {
			case 'i':
				{
					i = (*_currentScript)[pc++];
					int v = READ_UINT32(&i);

					res += Common::String::format(" %d", v);
					break;
				}
			case 'f':
				{
					Datum d;
					i = (*_currentScript)[pc++];
					d.u.f = *(double *)(&i);

					res += Common::String::format(" %f", d.u.f);
					break;
				}
			case 'o':
				{
					i = (*_currentScript)[pc++];
					int v = READ_UINT32(&i);

					res += Common::String::format(" [%5d]", v);
					break;
				}
			case 's':
				{
					char *s = (char *)&(*_currentScript)[pc];
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

void Lingo::define(Common::String &name, int start, int nargs, Common::String *prefix, int end) {
	if (prefix)
		name = *prefix + "-" + name;

	debugC(1, kDebugLingoCompile, "define(\"%s\", %d, %d, %d)", name.c_str(), start, _currentScript->size() - 1, nargs);

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

	if (end == -1)
		end = _currentScript->size();

	sym->u.defn = new ScriptData(&(*_currentScript)[start], end - start + 1);
	sym->nargs = nargs;
	sym->maxArgs = nargs;
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

int Lingo::codeConst(int val) {
	int res = g_lingo->code1(g_lingo->c_constpush);
	inst i = 0;
	WRITE_UINT32(&i, val);
	g_lingo->code1(i);

	return res;
}

int Lingo::codeArray(int arraySize) {
	int res = g_lingo->code1(g_lingo->c_arraypush);
	inst i = 0;
	WRITE_UINT32(&i, arraySize);
	g_lingo->code1(i);

	return res;
}

void Lingo::codeArg(Common::String *s) {
	_argstack.push_back(s);
}

void Lingo::codeArgStore() {
	while (true) {
		if (_argstack.empty()) {
			break;
		}

		Common::String *arg = _argstack.back();
		_argstack.pop_back();

		code1(c_varpush);
		codeString(arg->c_str());
		code1(c_assign);

		delete arg;
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
}

void Lingo::processIf(int elselabel, int endlabel) {
	inst ielse1, iend;
	int  else1 = elselabel;

	WRITE_UINT32(&iend, endlabel);

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

		WRITE_UINT32(&ielse1, else1);
		(*_currentScript)[label + 2] = ielse1;    /* elsepart */
		(*_currentScript)[label + 3] = iend;      /* end, if cond fails */

		else1 = label;
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
