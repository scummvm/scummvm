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
#include "director/movie.h"
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
	_currentLoop = nullptr;
	_refMode = false;

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
		compile(_assemblyAST);
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
		Common::Array<Common::String> *argNames = new Common::Array<Common::String>;
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
	} else {
		delete _currentAssembly;
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

void LingoCompiler::codeVarSet(const Common::String &name) {
	registerMethodVar(name);
	codeVarRef(name);
	code1(LC::c_assign);
}

void LingoCompiler::codeVarRef(const Common::String &name) {
	VarType type;
	if (_methodVars->contains(name)) {
		type = (*_methodVars)[name];
	} else {
		warning("LingoCompiler::codeVarRef: var %s referenced before definition", name.c_str());
		type = kVarGeneric;
	}
	switch (type) {
	case kVarGeneric:
		code1(LC::c_varrefpush);
		break;
	case kVarGlobal:
		code1(LC::c_globalrefpush);
		break;
	case kVarLocal:
	case kVarArgument:
		code1(LC::c_localrefpush);
		break;
	case kVarProperty:
	case kVarInstance:
		code1(LC::c_proprefpush);
		break;
	}
	codeString(name.c_str());
}

void LingoCompiler::codeVarGet(const Common::String &name) {
	VarType type;
	if (_methodVars->contains(name)) {
		type = (*_methodVars)[name];
	} else {
		warning("LingoCompiler::codeVarGet: var %s referenced before definition", name.c_str());
		type = kVarGeneric;
	}
	switch (type) {
	case kVarGeneric:
		code1(LC::c_varpush);
		break;
	case kVarGlobal:
		code1(LC::c_globalpush);
		break;
	case kVarLocal:
	case kVarArgument:
		code1(LC::c_localpush);
		break;
	case kVarProperty:
	case kVarInstance:
		code1(LC::c_proppush);
		break;
	}
	codeString(name.c_str());
}

void LingoCompiler::registerMethodVar(const Common::String &name, VarType type) {
	if (!_methodVars->contains(name)) {
		if (_indef && type == kVarGeneric) {
			type = kVarLocal;
		}
		(*_methodVars)[name] = type;
		if (type == kVarProperty || type == kVarInstance) {
			_assemblyContext->_properties[name] = Datum();
		} else if (type == kVarGlobal) {
			g_lingo->_globalvars[name] = Datum();
		}
	}
}

void LingoCompiler::registerFactory(Common::String &name) {
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

void LingoCompiler::updateLoopJumps(uint nextTargetPos, uint exitTargetPos) {
	if (!_currentLoop)
		return;
	
	for (uint i = 0; i < _currentLoop->nextRepeats.size(); i++) {
		uint nextRepeatPos = _currentLoop->nextRepeats[i];
		inst jmpOffset = 0;
		WRITE_UINT32(&jmpOffset, nextTargetPos - nextRepeatPos);
		(*_currentAssembly)[nextRepeatPos + 1] = jmpOffset; 
	}
	for (uint i = 0; i < _currentLoop->exitRepeats.size(); i++) {
		uint exitRepeatPos = _currentLoop->exitRepeats[i];
		inst jmpOffset = 0;
		WRITE_UINT32(&jmpOffset, exitTargetPos - exitRepeatPos);
		(*_currentAssembly)[exitRepeatPos + 1] = jmpOffset; 
	}
}

void LingoCompiler::parseMenu(const char *code) {
	warning("STUB: parseMenu");
}

void LingoCompiler::compile(Node *node) {
	node->accept(this);
}

void LingoCompiler::compileRef(Node *node) {
	_refMode = true;
	node->accept(this);
	_refMode = false;
}

void LingoCompiler::compileList(NodeList *nodes) {
	for (uint i = 0; i < nodes->size(); i++) {
		compile((*nodes)[i]);
	}
}

/* ScriptNode */

void LingoCompiler::visitScriptNode(ScriptNode *node) {
	compileList(node->children);
}

/* FactoryNode */

void LingoCompiler::visitFactoryNode(FactoryNode *node) {
	_inFactory = true;
	ScriptContext *mainContext = _assemblyContext;
	_assemblyContext = new ScriptContext(mainContext->getName(), mainContext->_archive, mainContext->_scriptType, mainContext->_id);

	compileList(node->methods);
	registerFactory(*node->name);

	_inFactory = false;
	_assemblyContext = mainContext;
}

/* HandlerNode */

void LingoCompiler::visitHandlerNode(HandlerNode *node) {
	_indef = true;
	ScriptData *mainAssembly = _currentAssembly;
	_currentAssembly = new ScriptData;
	VarTypeHash *mainMethodVars = _methodVars;
	_methodVars = new VarTypeHash;

	for (VarTypeHash::iterator i = mainMethodVars->begin(); i != mainMethodVars->end(); ++i) {
		if (i->_value == kVarGlobal || i->_value == kVarProperty)
			(*_methodVars)[i->_key] = i->_value;
	}
	if (_inFactory) {
		for (DatumHash::iterator i = _assemblyContext->_properties.begin(); i != _assemblyContext->_properties.end(); ++i) {
			(*_methodVars)[i->_key] = kVarInstance;
		}
	}

	compileList(node->stmts);
	code1(LC::c_procret);

	if (debugChannelSet(-1, kDebugFewFramesOnly) || debugChannelSet(1, kDebugCompile))
		debug("define handler \"%s\" (len: %d)", node->name->c_str(), _currentAssembly->size() - 1);

	Common::Array<Common::String> *argNames = new Common::Array<Common::String>;
	for (uint i = 0; i < node->args->size(); i++) {
		argNames->push_back(Common::String((*node->args)[i]->c_str()));
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

	_assemblyContext->define(*node->name, _currentAssembly, argNames, varNames);

	_indef = false;
	_currentAssembly = mainAssembly;
	delete _methodVars;
	_methodVars = mainMethodVars;
}

/* CmdNode */

void LingoCompiler::visitCmdNode(CmdNode *node) {
	compileList(node->args);
	codeCmd(node->name, node->args->size());
}

/* PutIntoNode */

void LingoCompiler::visitPutIntoNode(PutIntoNode *node) {
	if (node->var->type == kVarNode) {
		registerMethodVar(*static_cast<VarNode *>(node->var)->name);	
	}
	compile(node->val);
	compileRef(node->var);
	code1(LC::c_assign);
}

/* PutAfterNode */

void LingoCompiler::visitPutAfterNode(PutAfterNode *node) {
	if (node->var->type == kVarNode) {
		registerMethodVar(*static_cast<VarNode *>(node->var)->name);	
	}
	compile(node->val);
	compileRef(node->var);
	code1(LC::c_putafter);
}

/* PutBeforeNode */

void LingoCompiler::visitPutBeforeNode(PutBeforeNode *node) {
	if (node->var->type == kVarNode) {
		registerMethodVar(*static_cast<VarNode *>(node->var)->name);	
	}
	compile(node->val);
	compileRef(node->var);
	code1(LC::c_putbefore);
}

/* SetNode */

void LingoCompiler::visitSetNode(SetNode *node) {
	if (node->var->type == kVarNode) {
		registerMethodVar(*static_cast<VarNode *>(node->var)->name);	
	}
	compile(node->val);
	compileRef(node->var);
	code1(LC::c_assign);
}

/* GlobalNode */

void LingoCompiler::visitGlobalNode(GlobalNode *node) {
	for (uint i = 0; i < node->names->size(); i++) {
		registerMethodVar(*(*node->names)[i], kVarGlobal);
	}
}

/* PropertyNode */

void LingoCompiler::visitPropertyNode(PropertyNode *node) {
	for (uint i = 0; i < node->names->size(); i++) {
		registerMethodVar(*(*node->names)[i], kVarProperty);
	}
}

/* InstanceNode */

void LingoCompiler::visitInstanceNode(InstanceNode *node) {
	for (uint i = 0; i < node->names->size(); i++) {
		registerMethodVar(*(*node->names)[i], kVarInstance);
	}
}

/* IfStmtNode */

void LingoCompiler::visitIfStmtNode(IfStmtNode *node) {
	compile(node->cond);
	uint jzPos = _currentAssembly->size();
	code2(LC::c_jumpifz, 0);
	compileList(node->stmts);
	uint endPos = _currentAssembly->size();

	inst jzOffset = 0;
	WRITE_UINT32(&jzOffset, endPos - jzPos);
	(*_currentAssembly)[jzPos + 1] = jzOffset;
}

/* IfElseStmtNode */

void LingoCompiler::visitIfElseStmtNode(IfElseStmtNode *node) {
	compile(node->cond);
	uint jzPos = _currentAssembly->size();
	code2(LC::c_jumpifz, 0);
	compileList(node->stmts1);

	uint jmpPos = _currentAssembly->size();
	code2(LC::c_jump, 0);
	uint block2StartPos = _currentAssembly->size();
	compileList(node->stmts2);
	uint endPos = _currentAssembly->size();

	inst jzOffset = 0;
	WRITE_UINT32(&jzOffset, block2StartPos - jzPos);
	(*_currentAssembly)[jzPos + 1] = jzOffset;

	inst jmpOffset = 0;
	WRITE_UINT32(&jmpOffset, endPos - jmpPos);
	(*_currentAssembly)[jmpPos + 1] = jmpOffset;
}

/* RepeatWhileNode */

void LingoCompiler::visitRepeatWhileNode(RepeatWhileNode *node) {
	LoopNode *prevLoop = _currentLoop;
	_currentLoop = node;

	uint startPos = _currentAssembly->size();
	compile(node->cond);
	uint jzPos = _currentAssembly->size();
	code2(LC::c_jumpifz, 0);
	compileList(node->stmts);
	uint jmpPos = _currentAssembly->size();
	code2(LC::c_jump, 0);
	uint endPos = _currentAssembly->size();

	inst jzOffset = 0;
	WRITE_UINT32(&jzOffset, endPos - jzPos);
	(*_currentAssembly)[jzPos + 1] = jzOffset;

	inst jmpOffset = 0;
	WRITE_UINT32(&jmpOffset, startPos - jmpPos);
	(*_currentAssembly)[jmpPos + 1] = jmpOffset;

	updateLoopJumps(jmpPos, endPos);
	_currentLoop = prevLoop;
}

/* RepeatWithToNode */

void LingoCompiler::visitRepeatWithToNode(RepeatWithToNode *node) {
	LoopNode *prevLoop = _currentLoop;
	_currentLoop = node;

	compile(node->start);
	codeVarSet(*node->var);

	uint startPos = _currentAssembly->size();
	codeVarGet(*node->var);
	compile(node->end);
	if (node->down) {
		code1(LC::c_ge);
	} else {
		code1(LC::c_le);
	}
	uint jzPos = _currentAssembly->size();
	code2(LC::c_jumpifz, 0);

	compileList(node->stmts);

	uint incrementPos = _currentAssembly->size();
	codeVarGet(*node->var);
	code1(LC::c_intpush);
	codeInt(1);
	if (node->down) {
		code1(LC::c_sub);
	} else {
		code1(LC::c_add);
	}
	codeVarSet(*node->var);

	uint jmpPos = _currentAssembly->size();
	code2(LC::c_jump, 0);
	uint endPos = _currentAssembly->size();

	inst jzOffset = 0;
	WRITE_UINT32(&jzOffset, endPos - jzPos);
	(*_currentAssembly)[jzPos + 1] = jzOffset;

	inst jmpOffset = 0;
	WRITE_UINT32(&jmpOffset, startPos - jmpPos);
	(*_currentAssembly)[jmpPos + 1] = jmpOffset;

	updateLoopJumps(incrementPos, endPos);
	_currentLoop = prevLoop;
}

/* NextRepeatNode */

void LingoCompiler::visitNextRepeatNode(NextRepeatNode *node) {
	if (_currentLoop) {
		_currentLoop->nextRepeats.push_back(_currentAssembly->size());
		code2(LC::c_jump, 0);
	} else {
		warning("# LINGO: next repeat not inside repeat loop");
	}
}

/* ExitRepeatNode */

void LingoCompiler::visitExitRepeatNode(ExitRepeatNode *node) {
	if (_currentLoop) {
		_currentLoop->exitRepeats.push_back(_currentAssembly->size());
		code2(LC::c_jump, 0);
	} else {
		warning("# LINGO: exit repeat not inside repeat loop");
	}
}

/* IntNode */

void LingoCompiler::visitIntNode(IntNode *node) {
	code1(LC::c_intpush);
	codeInt(node->val);
}

/* FloatNode */

void LingoCompiler::visitFloatNode(FloatNode *node) {
	code1(LC::c_floatpush);
	codeFloat(node->val);
}

/* SymbolNode */

void LingoCompiler::visitSymbolNode(SymbolNode *node) {
	code1(LC::c_symbolpush);
	codeString(node->val->c_str());
}

/* StringNode */

void LingoCompiler::visitStringNode(StringNode *node) {
	code1(LC::c_stringpush);
	codeString(node->val->c_str());
}

/* FuncNode */

void LingoCompiler::visitFuncNode(FuncNode *node) {
	compileList(node->args);
	codeFunc(node->name, node->args->size());
}

/* VarNode */

void LingoCompiler::visitVarNode(VarNode *node) {
	if (_refMode) {
		codeVarRef(*node->name);
		return;
	}
	if (g_lingo->_builtinConsts.contains(*node->name)) {
		code1(LC::c_constpush);
		codeString(node->name->c_str());
		return;
	}
	if (g_director->getVersion() < 400 || g_director->getCurrentMovie()->_allowOutdatedLingo) {
		int val = castNumToNum(node->name->c_str());
		if (val != -1) {
			code1(LC::c_intpush);
			codeInt(val);
			return;
		}
	}
	codeVarGet(*node->name);
}

/* ParensNode */

void LingoCompiler::visitParensNode(ParensNode *node) {
	compile(node->expr);
}

/* UnaryOpNode */

void LingoCompiler::visitUnaryOpNode(UnaryOpNode *node) {
	compile(node->arg);
	code1(node->op);
}

/* BinaryOpNode */

void LingoCompiler::visitBinaryOpNode(BinaryOpNode *node) {
	compile(node->a);
	compile(node->b);
	code1(node->op);
}

} // End of namespace Director
