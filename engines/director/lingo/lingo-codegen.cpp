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

#include "common/endian.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-object.h"

namespace Director {

LingoCompiler::LingoCompiler() {
	_assemblyAST = nullptr;
	_assemblyArchive = nullptr;
	_currentAssembly = nullptr;
	_assemblyContext = nullptr;

	_indef = false;

	_linenumber = _colnumber = _bytenumber = 0;
	_lines[0] = _lines[1] = _lines[2] = nullptr;

	_inFactory = false;

	_hadError = false;
}

ScriptContext *LingoCompiler::compileAnonymous(const char *code) {
	debugC(1, kDebugCompile, "Compiling anonymous lingo\n"
			"***********\n%s\n\n***********", code);

	return compileLingo(code, nullptr, kNoneScript, 0, "[anonymous]", true);
}

ScriptContext *LingoCompiler::compileLingo(const char *code, LingoArchive *archive, ScriptType type, uint16 id, const Common::String &scriptName, bool anonymous) {
	_assemblyArchive = archive;
	_assemblyAST = nullptr;
	ScriptContext *mainContext = _assemblyContext = new ScriptContext(scriptName, archive, type, id);
	_currentAssembly = new ScriptData;

	_methodVars = new VarTypeHash;
	_linenumber = _colnumber = 1;
	_hadError = false;

	if (!strncmp(code, "menu:", 5) || scumm_strcasestr(code, "\nmenu:")) {
		debugC(1, kDebugCompile, "Parsing menu");
		parseMenu(code);

		return nullptr;
	}

	// Preprocess the code for ease of the parser
	Common::String codeNorm = codePreprocessor(code, archive, type, id);
	code = codeNorm.c_str();

	// Parse the Lingo and build an AST
	parse(code);

	// Generate bytecode
	if (_assemblyAST) {
		_assemblyAST->compile();
	}

	// for D4 and above, there usually won't be any code left.
	// all scoped methods will be defined and stored by the code parser
	// however D3 and below allow scopeless functions!
	// and these can show up in D4 when imported from other movies

	if (!_currentAssembly->empty()) {
		// end of script, add a c_procret so stack frames work as expected
		code1(LC::c_procret);
		code1(STOP);

		if (debugChannelSet(3, kDebugCompile)) {
			if (_currentAssembly->size() && !_hadError)
				Common::hexdump((byte *)&_currentAssembly->front(), _currentAssembly->size() * sizeof(inst));

			debugC(2, kDebugCompile, "<resulting code>");
			uint pc = 0;
			while (pc < _currentAssembly->size()) {
				uint spc = pc;
				Common::String instr = g_lingo->decodeInstruction(_assemblyArchive, _currentAssembly, pc, &pc);
				debugC(2, kDebugCompile, "[%5d] %s", spc, instr.c_str());
			}
			debugC(2, kDebugCompile, "<end code>");
		}

		Symbol currentFunc;

		currentFunc.type = HANDLER;
		currentFunc.u.defn = _currentAssembly;
		Common::String typeStr = Common::String(scriptType2str(type));
		currentFunc.name = new Common::String("[" + typeStr + " " + _assemblyContext->getName() + "]");
		currentFunc.ctx = _assemblyContext;
		currentFunc.archive = archive;
		currentFunc.anonymous = anonymous;
		// arg names should be empty, but just in case
		Common::Array<Common::String> *argNames = new Common::Array<Common::String>;
		for (uint i = 0; i < _argstack.size(); i++) {
			argNames->push_back(Common::String(_argstack[i]->c_str()));
		}
		Common::Array<Common::String> *varNames = new Common::Array<Common::String>;
		for (Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator it = _methodVars->begin(); it != _methodVars->end(); ++it) {
			if (it->_value == kVarLocal)
				varNames->push_back(Common::String(it->_key));
		}

		if (debugChannelSet(1, kDebugCompile)) {
			debug("Function vars");
			debugN("  Args: ");
			for (uint i = 0; i < argNames->size(); i++) {
				debugN("%s, ", (*argNames)[i].c_str());
			}
			debugN("\n");
			debugN("  Local vars: ");
			for (uint i = 0; i < varNames->size(); i++) {
				debugN("%s, ", (*varNames)[i].c_str());
			}
			debugN("\n");
		}

		currentFunc.argNames = argNames;
		currentFunc.varNames = varNames;
		_assemblyContext->_eventHandlers[kEventGeneric] = currentFunc;
	}

	delete _methodVars;
	_methodVars = nullptr;
	_currentAssembly = nullptr;
	delete _assemblyAST;
	_assemblyAST = nullptr;
	_assemblyContext = nullptr;
	_assemblyArchive = nullptr;
	return mainContext;
}

Symbol LingoCompiler::codeDefine(Common::String &name, int start, int nargs, int end, bool removeCode) {
	if (debugChannelSet(-1, kDebugFewFramesOnly) || debugChannelSet(1, kDebugCompile))
		debug("codeDefine(\"%s\"(len: %d), %d, %d, %d)",
			name.c_str(), _currentAssembly->size() - 1, start, nargs, end);

	if (end == -1)
		end = _currentAssembly->size();

	ScriptData *code = new ScriptData(&(*_currentAssembly)[start], end - start);
	Common::Array<Common::String> *argNames = new Common::Array<Common::String>;
	for (uint i = 0; i < _argstack.size(); i++) {
		argNames->push_back(Common::String(_argstack[i]->c_str()));
	}
	Common::Array<Common::String> *varNames = new Common::Array<Common::String>;
	for (Common::HashMap<Common::String, VarType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator it = _methodVars->begin(); it != _methodVars->end(); ++it) {
		if (it->_value == kVarLocal)
			varNames->push_back(Common::String(it->_key));
	}

	Symbol sym = _assemblyContext->define(name, nargs, code, argNames, varNames);

	if (debugChannelSet(1, kDebugCompile)) {
		debug("Function vars");
		debugN("  Args: ");
		for (uint i = 0; i < argNames->size(); i++) {
			debugN("%s, ", (*argNames)[i].c_str());
		}
		debugN("\n");
		debugN("  Local vars: ");
		for (uint i = 0; i < varNames->size(); i++) {
			debugN("%s, ", (*varNames)[i].c_str());
		}
		debugN("\n");
	}

	// Now remove all defined code from the _currentAssembly
	if (removeCode)
		for (int i = end - 1; i >= start; i--) {
			_currentAssembly->remove_at(i);
		}

	return sym;
}

int LingoCompiler::codeString(const char *str) {
	int numInsts = calcStringAlignment(str);

	// Where we copy the string over
	int pos = _currentAssembly->size();

	// Allocate needed space in script
	for (int i = 0; i < numInsts; i++)
		_currentAssembly->push_back(0);

	byte *dst = (byte *)&_currentAssembly->front() + pos * sizeof(inst);

	memcpy(dst, str, strlen(str) + 1);

	return _currentAssembly->size();
}

int LingoCompiler::codeFloat(double f) {
	int numInsts = calcCodeAlignment(sizeof(double));

	// Where we copy the string over
	int pos = _currentAssembly->size();

	// Allocate needed space in script
	for (int i = 0; i < numInsts; i++)
		_currentAssembly->push_back(0);

	double *dst = (double *)((byte *)&_currentAssembly->front() + pos * sizeof(inst));

	*dst = f;

	return _currentAssembly->size();
}

int LingoCompiler::codeInt(int val) {
	inst i = 0;
	WRITE_UINT32(&i, val);
	code1(i);

	return _currentAssembly->size();
}

bool LingoCompiler::isInArgStack(Common::String *s) {
	for (uint i = 0; i < _argstack.size(); i++)
		if (_argstack[i]->equalsIgnoreCase(*s))
			return true;

	return false;
}

void LingoCompiler::codeArg(Common::String *s) {
	_argstack.push_back(new Common::String(*s));
}

void LingoCompiler::clearArgStack() {
	for (uint i = 0; i < _argstack.size(); i++)
		delete _argstack[i];

	_argstack.clear();
}

int LingoCompiler::codeCmd(Common::String *s, int numpar) {
	// Insert current line number to our asserts
	if (s->equalsIgnoreCase("scummvmAssert") || s->equalsIgnoreCase("scummvmAssertEqual")) {
		code1(LC::c_intpush);
		codeInt(_linenumber);

		numpar++;
	}

	int ret = code1(LC::c_callcmd);

	codeString(s->c_str());

	inst num = 0;
	WRITE_UINT32(&num, numpar);
	code1(num);

	return ret;
}

int LingoCompiler::codeFunc(Common::String *s, int numpar) {
	int ret = code1(LC::c_callfunc);

	codeString(s->c_str());

	inst num = 0;
	WRITE_UINT32(&num, numpar);
	code1(num);

	return ret;
}

void LingoCompiler::codeLabel(int label) {
	_labelstack.push_back(label);
	debugC(4, kDebugCompile, "codeLabel: Added label %d", label);
}

void LingoCompiler::processIf(int toplabel, int endlabel) {
	inst iend;

	debugC(4, kDebugCompile, "processIf(%d, %d)", toplabel, endlabel);

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

		debugC(4, kDebugCompile, "processIf: label at %d", label);

		WRITE_UINT32(&iend, endlabel - label + 1);

		(*_currentAssembly)[label] = iend;	/* end, if cond fails */
	}
}

void LingoCompiler::registerMethodVar(const Common::String &name, VarType type) {
	if (!_methodVars->contains(name)) {
		(*_methodVars)[name] = type;
		if (type == kVarProperty || type == kVarInstance) {
			_assemblyContext->_properties[name] = Datum();
		} else if (type == kVarGlobal) {
			g_lingo->_globalvars[name] = Datum();
		}
	}
}

void LingoCompiler::codeFactory(Common::String &name) {
	// FIXME: The factory's context should not be tied to the LingoArchive
	// but bytecode needs it to resolve names
	_assemblyContext->setName(name);
	_assemblyContext->setFactory(true);
	if (!g_lingo->_globalvars.contains(name)) {
		g_lingo->_globalvars[name] = _assemblyContext;
	} else {
		warning("Factory '%s' already defined", name.c_str());
	}
}

void LingoCompiler::parseMenu(const char *code) {
	warning("STUB: parseMenu");
}

} // End of namespace Director
