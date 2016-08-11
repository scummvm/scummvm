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

#include "engines/director/lingo/lingo.h"
#include "common/file.h"
#include "audio/decoders/wave.h"

#include "director/lingo/lingo-gr.h"

namespace Director {

void Lingo::execute(int pc) {
	for(_pc = pc; (*_currentScript)[_pc] != STOP && !_returning;) {

		for (uint i = 0; i < _stack.size(); i++) {
			debugN(5, "%d ", _stack[i].u.i);
		}
		debug(5, "%s", "");

		_pc++;
		(*((*_currentScript)[_pc - 1]))();
	}
}

Symbol *Lingo::lookupVar(const char *name, bool create, bool putInGlobalList) {
	Symbol *sym;

	// Looking for the cast member constants
	if (_vm->getVersion() < 4) { // TODO: There could be a flag 'Allow Outdated Lingo' in Movie Info in D4
		if (strlen(name) == 3) {
			if (tolower(name[0]) >= 'a' && tolower(name[0]) <= 'h' &&
				name[1] >= '1' && name[1] <= '8' &&
				name[2] >= '1' && name[2] <= '8') {

				if (!create)
					error("Cast reference used in wrong context: %s", name);

				int val = (tolower(name[0]) - 'a') * 64 + (name[1] - '1') * 8 + (name[2] - '1') + 1;
				sym = new Symbol;

				sym->type = CASTREF;
				sym->u.i = val;

				return sym;
			}
		}
	}

	if (!_localvars->contains(name)) { // Create variable if it was not defined
		if (!create)
			return NULL;

		sym = new Symbol;
		sym->name = (char *)calloc(strlen(name) + 1, 1);
		Common::strlcpy(sym->name, name, strlen(name) + 1);
		sym->type = VOID;
		sym->u.i = 0;

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
	for (SymbolHash::const_iterator h = _localvars->begin(); h != _localvars->end(); ++h) {
		if (!h->_value->global)
			delete h->_value;
	}
	delete g_lingo->_localvars;
}

void Lingo::define(Common::String &name, int start, int nargs, Common::String *prefix) {
	Symbol *sym;

	if (prefix)
		name = *prefix + "-" + name;

	debug(3, "define(\"%s\", %d, %d, %d)", name.c_str(), start, _currentScript->size() - 1, nargs);

	if (!_handlers.contains(name)) { // Create variable if it was not defined
		sym = new Symbol;

		sym->name = (char *)calloc(name.size() + 1, 1);
		Common::strlcpy(sym->name, name.c_str(), name.size() + 1);
		sym->type = HANDLER;

		_handlers[name] = sym;
	} else {
		sym = g_lingo->_handlers[name];

		warning("Redefining handler '%s'", name.c_str());
		delete sym->u.defn;
	}

	sym->u.defn = new ScriptData(&(*_currentScript)[start], _currentScript->size() - start + 1);
	sym->nargs = nargs;
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
		code1(c_xpop);

		delete arg;
	}
}

int Lingo::codeFunc(Common::String *s, int numpar) {
	int ret = g_lingo->code1(g_lingo->c_call);

	if (s->equalsIgnoreCase("me")) {
		if (!g_lingo->_currentFactory.empty()) {
			g_lingo->codeString(g_lingo->_currentFactory.c_str());
			debug(2, "Repaced 'me' with %s", g_lingo->_currentFactory.c_str());
		} else {
			warning("'me' out of factory method");
			g_lingo->codeString(s->c_str());
		}
	} else {
		g_lingo->codeString(s->c_str());
	}

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

		WRITE_UINT32(&ielse1, else1);
		(*_currentScript)[label + 2] = ielse1;    /* elsepart */
		(*_currentScript)[label + 3] = iend;      /* end, if cond fails */

		else1 = label;
	}
}

void Lingo::codeFactory(Common::String &name) {
	_currentFactory = name;

	Symbol *sym = new Symbol;

	sym->name = (char *)calloc(name.size() + 1, 1);
	Common::strlcpy(sym->name, name.c_str(), name.size());
	sym->type = BLTIN;
	sym->nargs = -1;
	sym->maxArgs = 0;
	sym->parens = true;
	sym->u.bltin = g_lingo->b_factory;

	_handlers[name] = sym;
}

}
