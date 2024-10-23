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

#include "common/endian.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-the.h"

namespace Director {

#define COMPILE(node) \
	{ \
		bool refModeStore = _refMode; \
		_refMode = false; \
		bool success = (node)->accept(this); \
		_refMode = refModeStore; \
		if (!success) \
			return false; \
	}

#define COMPILE_REF(node) \
	{ \
		bool refModeStore = _refMode; \
		_refMode = true; \
		bool success = (node)->accept(this); \
		_refMode = refModeStore; \
		if (!success) \
			return false; \
	}

#define COMPILE_LIST(list) \
	{ \
		bool refModeStore = _refMode; \
		_refMode = false; \
		bool success = true; \
		for (uint i = 0; i < (list)->size(); i++) { \
			success = (*(list))[i]->accept(this); \
			if (!success) \
				break; \
		} \
		_refMode = refModeStore; \
		if (!success) \
			return false; \
	}

class NodeStore {
public:
	explicit NodeStore(Node *node) : _node(node) {
		_node->startOffset = g_lingo->_compiler->_currentAssembly->size() - 1;
	}
	~NodeStore() { _node->endOffset = g_lingo->_compiler->_currentAssembly->size() - 1; }

private:
	Node *_node = nullptr;
};

LingoCompiler::LingoCompiler() {
	_assemblyAST = nullptr;
	_assemblyArchive = nullptr;
	_currentAssembly = nullptr;
	_assemblyContext = nullptr;
	_assemblyId = -1;

	_indef = false;
	_methodVars = nullptr;

	_linenumber = _colnumber = _bytenumber = 0;
	_lines[0] = _lines[1] = _lines[2] = nullptr;

	_inFactory = false;
	_currentLoop = nullptr;
	_refMode = false;

	_hadError = false;
}

ScriptContext *LingoCompiler::compileAnonymous(const Common::U32String &code, uint32 preprocFlags) {
	debugC(1, kDebugCompile, "Compiling anonymous lingo\n"
			"***********\n%s\n\n***********", code.encode().c_str());

	return compileLingo(code, nullptr, kNoneScript, CastMemberID(0, 0), "[anonymous]", true, preprocFlags);
}

ScriptContext *LingoCompiler::compileLingo(const Common::U32String &code, LingoArchive *archive, ScriptType type, CastMemberID id, const Common::String &scriptName, bool anonymous, uint32 preprocFlags) {
	_assemblyArchive = archive;
	_assemblyAST = nullptr;
	_assemblyId = id.member;
	ScriptContext *mainContext = _assemblyContext = new ScriptContext(scriptName, type, _assemblyId);
	_currentAssembly = new ScriptData;

	_methodVars = new VarTypeHash;
	_linenumber = _colnumber = 1;
	_hadError = false;

	// Preprocess the code for ease of the parser
	Common::U32String codePrep = codePreprocessor(code, archive, type, id, preprocFlags);

	// Search the methods
	mainContext->_methodNames = prescanMethods(codePrep);

	Common::String codeNorm = codePrep.encode(Common::kUtf8);
	const char *utf8Code = codeNorm.c_str();

	// Parse the Lingo and build an AST
	parse(utf8Code);
	// If it doesn't work, and we have kLPPTrimGarbage enabled,
	// have another try with the input trimmed to the last valid character.
	if (!_assemblyAST && (preprocFlags & kLPPTrimGarbage)) {
		delete _assemblyContext;
		delete _currentAssembly;
		delete _methodVars;
		_assemblyId = id.member;
		mainContext = _assemblyContext = new ScriptContext(scriptName, type, _assemblyId);
		_currentAssembly = new ScriptData;
		_methodVars = new VarTypeHash;
		mainContext->_methodNames = prescanMethods(codePrep);
		_linenumber = _colnumber = 1;
		_hadError = false;
		codeNorm = codeNorm.substr(0, _bytenumber - 1) + "\n";
		utf8Code = codeNorm.c_str();
		parse(utf8Code);
	}
	if (!_assemblyAST) {
		delete _assemblyContext;
		delete _currentAssembly;
		delete _methodVars;
		_assemblyId = -1;
		return nullptr;
	}

	// Generate bytecode
	if (!_assemblyAST->accept(this)) {
		_hadError = true;
		delete _assemblyContext;
		delete _currentAssembly;
		delete _methodVars;
		_assemblyId = -1;
		return nullptr;
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
				Common::String instr = g_lingo->decodeInstruction(_currentAssembly, pc, &pc);
				debugC(2, kDebugCompile, "[%5d] %s", spc, instr.c_str());
			}
			debugC(2, kDebugCompile, "<end code>");
		}

		Symbol currentFunc;

		currentFunc.type = HANDLER;
		currentFunc.u.defn = _currentAssembly;
		Common::String typeStr = Common::String(scriptType2str(type));
		currentFunc.name = new Common::String("scummvm_" + typeStr + "_" + _assemblyContext->getName());
		currentFunc.ctx = _assemblyContext;
		currentFunc.anonymous = anonymous;
		Common::Array<Common::String> *argNames = new Common::Array<Common::String>;
		Common::Array<Common::String> *varNames = new Common::Array<Common::String>;
		for (auto &it : *_methodVars) {
			if (it._value == kVarLocal)
				varNames->push_back(Common::String(it._key));
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
		_assemblyContext->_functionHandlers[*currentFunc.name] = currentFunc;
		_assemblyContext->_eventHandlers[kEventGeneric] = currentFunc;
	} else {
		delete _currentAssembly;
	}

	// Register this context's functions with the containing archive.
	if (_assemblyArchive) {
		for (auto &it : _assemblyContext->_functionHandlers) {
			if (!_assemblyArchive->functionHandlers.contains(it._key)) {
				_assemblyArchive->functionHandlers[it._key] = it._value;
			}
		}
	}

	delete _methodVars;
	_methodVars = nullptr;
	_currentAssembly = nullptr;

	if (debugChannelSet(-1, kDebugImGui)) {
		_assemblyContext->_assemblyAST = _assemblyAST;
	} else {
		_assemblyAST.reset();
	}
	_assemblyAST = nullptr;
	_assemblyContext = nullptr;
	_assemblyArchive = nullptr;
	_assemblyId = -1;
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
	inst i = nullptr;
	WRITE_UINT32(&i, val);
	code1(i);

	return _currentAssembly->size();
}

int LingoCompiler::codeCmd(const Common::String &s, int numpar) {
	int ret = code1(LC::c_callcmd);

	codeString(s.c_str());

	inst num = nullptr;
	WRITE_UINT32(&num, numpar);
	code1(num);

	return ret;
}

int LingoCompiler::codeFunc(const Common::String &s, int numpar) {
	int ret = code1(LC::c_callfunc);

	codeString(s.c_str());

	inst num = nullptr;
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
		if (_indef)
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
			if (!_assemblyContext->hasProp(name))
				_assemblyContext->setProp(name, Datum(), true);
		} else if (type == kVarGlobal) {
			if (!g_lingo->_globalvars.contains(name))
				g_lingo->_globalvars[name] = Datum();
		}
	}
}

void LingoCompiler::registerFactory(Common::String &name) {
	_assemblyContext->setName(name);
	_assemblyContext->setFactory(true);
	g_lingo->_globalvars[name] = _assemblyContext;
	// Add the factory to the list in the archive
	if (_assemblyArchive) {
		if (!_assemblyArchive->factoryContexts.contains(_assemblyId)) {
			_assemblyArchive->factoryContexts[_assemblyId] = new Common::HashMap<Common::String, ScriptContext *>();
		}
		if (!_assemblyArchive->factoryContexts[_assemblyId]->contains(name)) {
			_assemblyContext->incRefCount();
			(*_assemblyArchive->factoryContexts[_assemblyId])[name] = _assemblyContext;
		}
	}
}

void LingoCompiler::updateLoopJumps(uint nextTargetPos, uint exitTargetPos) {
	if (!_currentLoop)
		return;

	for (uint i = 0; i < _currentLoop->nextRepeats.size(); i++) {
		uint nextRepeatPos = _currentLoop->nextRepeats[i];
		inst jmpOffset = nullptr;
		WRITE_UINT32(&jmpOffset, nextTargetPos - nextRepeatPos);
		(*_currentAssembly)[nextRepeatPos + 1] = jmpOffset;
	}
	for (uint i = 0; i < _currentLoop->exitRepeats.size(); i++) {
		uint exitRepeatPos = _currentLoop->exitRepeats[i];
		inst jmpOffset = nullptr;
		WRITE_UINT32(&jmpOffset, exitTargetPos - exitRepeatPos);
		(*_currentAssembly)[exitRepeatPos + 1] = jmpOffset;
	}
}

/* ScriptNode */

bool LingoCompiler::visitScriptNode(ScriptNode *node) {
	NodeStore store(node);
	COMPILE_LIST(node->children);
	return true;
}

/* FactoryNode */

bool LingoCompiler::visitFactoryNode(FactoryNode *node) {
	NodeStore store(node);
	_inFactory = true;
	ScriptContext *mainContext = _assemblyContext;
	_assemblyContext = new ScriptContext(*node->name, mainContext->_scriptType, mainContext->_id);

	COMPILE_LIST(node->methods);
	registerFactory(*node->name);

	_inFactory = false;
	_assemblyContext = mainContext;
	return true;
}

/* HandlerNode */

bool LingoCompiler::visitHandlerNode(HandlerNode *node) {
	NodeStore store(node);
	_indef = true;
	ScriptData *mainAssembly = _currentAssembly;
	_currentAssembly = new ScriptData;
	VarTypeHash *mainMethodVars = _methodVars;
	_methodVars = new VarTypeHash;

	if (_inFactory) {
		registerMethodVar("me", kVarArgument);
	}
	for (uint i = 0; i < node->args->size(); i++) {
		registerMethodVar(*(*node->args)[i], kVarArgument);
	}
	for (auto &i : *mainMethodVars) {
		if (i._value == kVarGlobal)
			registerMethodVar(i._key, kVarGlobal);
	}
	for (uint32 i = 1; i <= _assemblyContext->getPropCount(); i++) {
		registerMethodVar(_assemblyContext->getPropAt(i), _inFactory ? kVarInstance : kVarProperty);
	}

	COMPILE_LIST(node->stmts);
	code1(LC::c_procret);

	if (debugChannelSet(1, kDebugCompile))
		debug("define handler \"%s\" (len: %d)", node->name->c_str(), _currentAssembly->size() - 1);

	Common::Array<Common::String> *argNames = new Common::Array<Common::String>;
	if (_inFactory) {
		argNames->push_back("me");
	}
	for (uint i = 0; i < node->args->size(); i++) {
		argNames->push_back(Common::String((*node->args)[i]->c_str()));
	}
	Common::Array<Common::String> *varNames = new Common::Array<Common::String>;
	for (auto &it : *_methodVars) {
		if (it._value == kVarLocal)
			varNames->push_back(Common::String(it._key));
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
	return true;
}

/* CmdNode */

bool LingoCompiler::visitCmdNode(CmdNode *node) {
	NodeStore store(node);
	uint numargs = node->args->size();

	if (node->name->equalsIgnoreCase("go") && numargs == 1 && (*node->args)[0]->type == kVarNode){
		VarNode *var = static_cast<VarNode *>((*node->args)[0]);
		if (var->name->equalsIgnoreCase("loop") ||
				var->name->equalsIgnoreCase("next") ||
				var->name->equalsIgnoreCase("previous")) {
			code1(LC::c_symbolpush);
			codeString(var->name->c_str());
			codeCmd(*node->name, 1);
			return true;
		}
	}

	// `play done` compiles to `play()`
	if (node->name->equalsIgnoreCase("play") && numargs == 1 && (*node->args)[0]->type == kVarNode) {
		VarNode *var = static_cast<VarNode *>((*node->args)[0]);
		if (var->name->equalsIgnoreCase("done")) {
			codeCmd(*node->name, 0);
			return true;
		}
	}

	if (node->name->equalsIgnoreCase("playAccel")) {
		for (uint i = 0; i < numargs; i++) {
			Node *arg = (*node->args)[i];
			if (arg->type == kVarNode) {
				code1(LC::c_symbolpush);
				codeString(static_cast<VarNode *>(arg)->name->c_str());
			} else {
				COMPILE(arg);
			}
		}
		codeCmd(*node->name, numargs);
		return true;
	}

	if (node->name->equalsIgnoreCase("sound") && numargs >= 1 && (*node->args)[0]->type == kVarNode) {
		VarNode *var = static_cast<VarNode *>((*node->args)[0]);
		if (var->name->equalsIgnoreCase("close") ||
				var->name->equalsIgnoreCase("fadeIn") ||
				var->name->equalsIgnoreCase("fadeOut") ||
				var->name->equalsIgnoreCase("playFile") ||
				var->name->equalsIgnoreCase("stop")) {
			code1(LC::c_symbolpush);
			codeString(var->name->c_str());
			for (uint i = 1; i < numargs; i++) {
				COMPILE((*node->args)[i]);
			}
			codeCmd(*node->name, numargs);
			return true;
		}
	}

	if (numargs >= 1 && (*node->args)[0]->type == kVarNode) {
		// This could be a method call. Code the first arg as a reference.
		COMPILE_REF((*node->args)[0]);
		for (uint i = 1; i < numargs; i++) {
			COMPILE((*node->args)[i]);
		}
	} else {
		COMPILE_LIST(node->args);
	}
	// Insert current line number to our asserts
	if (node->name->equalsIgnoreCase("scummvmAssert") || node->name->equalsIgnoreCase("scummvmAssertEqual")) {
		code1(LC::c_intpush);
		codeInt(node->lineNumber);
		numargs++;
	}
	codeCmd(*node->name, numargs);
	return true;
}

/* PutIntoNode */

bool LingoCompiler::visitPutIntoNode(PutIntoNode *node) {
	NodeStore store(node);
	if (node->var->type == kVarNode) {
		registerMethodVar(*static_cast<VarNode *>(node->var)->name);
	}
	COMPILE(node->val);
	COMPILE_REF(node->var);
	code1(LC::c_assign);
	return true;
}

/* PutAfterNode */

bool LingoCompiler::visitPutAfterNode(PutAfterNode *node) {
	NodeStore store(node);
	if (node->var->type == kVarNode) {
		registerMethodVar(*static_cast<VarNode *>(node->var)->name);
	}
	COMPILE(node->val);
	COMPILE_REF(node->var);
	code1(LC::c_putafter);
	return true;
}

/* PutBeforeNode */

bool LingoCompiler::visitPutBeforeNode(PutBeforeNode *node) {
	NodeStore store(node);
	if (node->var->type == kVarNode) {
		registerMethodVar(*static_cast<VarNode *>(node->var)->name);
	}
	COMPILE(node->val);
	COMPILE_REF(node->var);
	code1(LC::c_putbefore);
	return true;
}

/* SetNode */

int LingoCompiler::getTheFieldID(int entity, const Common::String &field, bool silent) {
	Common::String fieldKey = Common::String::format("%d%s", entity, field.c_str());
	if (!g_lingo->_theEntityFields.contains(fieldKey)) {
		if (!silent)
			warning("BUILDBOT: LingoCompiler::getTheFieldId: Unhandled the field %s of %s", field.c_str(), g_lingo->entity2str(entity));
		return -1;
	}
	return g_lingo->_theEntityFields[fieldKey]->field;
}

bool LingoCompiler::visitSetNode(SetNode *node) {
	NodeStore store(node);
	if (node->var->type == kTheNode) {
		TheNode *the = static_cast<TheNode *>(node->var);
		if (g_lingo->_theEntities.contains(*the->prop) && !g_lingo->_theEntities[*the->prop]->hasId) {
			COMPILE(node->val);
			code1(LC::c_intpush);
			codeInt(0); // Put dummy id
			code1(LC::c_theentityassign);
			codeInt(g_lingo->_theEntities[*the->prop]->entity);
			codeInt(0);	// No field
			return true;
		}
		warning("BUILDBOT: LingoCompiler:visitSetNode: Unhandled the entity '%s'", the->prop->c_str());
		return false;
	}

	if (node->var->type == kTheOfNode) {
		TheOfNode *the = static_cast<TheOfNode *>(node->var);
		switch (the->obj->type) {
		case kChunkExprNode:
			{
				int fieldId = getTheFieldID(kTheChunk, *the->prop, true);
				if (fieldId >= 0) {
					COMPILE(node->val);
					COMPILE_REF(the->obj);
					code1(LC::c_theentityassign);
					codeInt(kTheChunk);
					codeInt(fieldId);
					return true;
				}
				// fall back to generic object
			}
			break;
		case kFuncNode:
			{
				FuncNode *func = static_cast<FuncNode *>(the->obj);
				if (func->args->size() == 1) {
					if (func->name->equalsIgnoreCase("cast")) {
						int fieldId = getTheFieldID(kTheCast, *the->prop, true);
						if (fieldId >= 0) {
							COMPILE(node->val);
							COMPILE((*func->args)[0]);
							code1(LC::c_theentityassign);
							codeInt(kTheCast);
							codeInt(fieldId);
							return true;
						}
						// fall back to generic object
					}
					if (func->name->equalsIgnoreCase("field")) {
						int fieldId = getTheFieldID(kTheField, *the->prop, true);
						if (fieldId >= 0) {
							COMPILE(node->val);
							COMPILE((*func->args)[0]);
							code1(LC::c_theentityassign);
							codeInt(kTheField);
							codeInt(fieldId);
							return true;
						}
						// fall back to generic object
					}
					// window is an object and is handled by that case
				}
			}
			break;
		case kMenuNode:
			{
				MenuNode *menu = static_cast<MenuNode *>(the->obj);
				int fieldId = getTheFieldID(kTheMenu, *the->prop);
				if (fieldId < 0)
					return false;
				COMPILE(node->val);
				COMPILE(menu->arg);
				code1(LC::c_theentityassign);
				codeInt(kTheMenu);
				codeInt(fieldId);
				return true;
			}
			break;
		case kMenuItemNode:
			{
				MenuItemNode *menuItem = static_cast<MenuItemNode *>(the->obj);
				int fieldId = getTheFieldID(kTheMenuItem, *the->prop);
				if (fieldId < 0)
					return false;
				COMPILE(node->val);
				COMPILE(menuItem->arg1)
				COMPILE(menuItem->arg2);
				code1(LC::c_theentityassign);
				codeInt(kTheMenuItem);
				codeInt(fieldId);
				return true;
			}
			break;
		case kSoundNode:
			{
				SoundNode *sound = static_cast<SoundNode *>(the->obj);
				int fieldId = getTheFieldID(kTheSoundEntity, *the->prop);
				if (fieldId < 0)
					return false;
				COMPILE(node->val);
				COMPILE(sound->arg);
				code1(LC::c_theentityassign);
				codeInt(kTheSoundEntity);
				codeInt(fieldId);
				return true;
			}
			break;
		case kSpriteNode:
			{
				SpriteNode *sprite = static_cast<SpriteNode *>(the->obj);
				int fieldId = getTheFieldID(kTheSprite, *the->prop);
				if (fieldId < 0)
					return false;
				COMPILE(node->val);
				COMPILE(sprite->arg);
				code1(LC::c_theentityassign);
				codeInt(kTheSprite);
				codeInt(fieldId);
				return true;
			}
			break;
		case kVarNode:
			{
				VarNode *var = static_cast<VarNode *>(the->obj);
				if (the->prop->equalsIgnoreCase("number") && var->name->equalsIgnoreCase("castMembers")) {
					COMPILE(node->val);
					code1(LC::c_intpush);
					codeInt(0); // Put dummy id
					code1(LC::c_theentityassign);
					codeInt(kTheCastMembers);
					codeInt(kTheNumber);
					return true;
				}
			}
			break;
		default:
			break;
		}

		if (g_director->getVersion() >= 400) {
			COMPILE(node->val);
			COMPILE(the->obj);
			code1(LC::c_objectpropassign);
			codeString(the->prop->c_str());
			return true;
		}

		warning("BUILDBOT: LingoCompiler::visitSetNode: Unhandled the field %s", the->prop->c_str());
		return false;
	}

	if (node->var->type == kVarNode) {
		registerMethodVar(*static_cast<VarNode *>(node->var)->name);
	}
	COMPILE(node->val);
	COMPILE_REF(node->var);
	code1(LC::c_assign);
	return true;
}

/* GlobalNode */

bool LingoCompiler::visitGlobalNode(GlobalNode *node) {
	NodeStore store(node);
	for (uint i = 0; i < node->names->size(); i++) {
		registerMethodVar(*(*node->names)[i], kVarGlobal);
	}
	// Before Director 4, a global statement initializes the var to 0.
	if (g_director->getVersion() < 400) {
		for (uint i = 0; i < node->names->size(); i++) {
			code1(LC::c_globalinit);
			codeString((*node->names)[i]->c_str());
		}
	}
	return true;
}

/* PropertyNode */

bool LingoCompiler::visitPropertyNode(PropertyNode *node) {
	NodeStore store(node);
	for (uint i = 0; i < node->names->size(); i++) {
		registerMethodVar(*(*node->names)[i], kVarProperty);
	}
	return true;
}

/* InstanceNode */

bool LingoCompiler::visitInstanceNode(InstanceNode *node) {
	NodeStore store(node);
	for (uint i = 0; i < node->names->size(); i++) {
		registerMethodVar(*(*node->names)[i], kVarInstance);
	}
	return true;
}

/* IfStmtNode */

bool LingoCompiler::visitIfStmtNode(IfStmtNode *node) {
	NodeStore store(node);
	COMPILE(node->cond);
	uint jzPos = _currentAssembly->size();
	code2(LC::c_jumpifz, nullptr);
	COMPILE_LIST(node->stmts);
	uint endPos = _currentAssembly->size();

	inst jzOffset = nullptr;
	WRITE_UINT32(&jzOffset, endPos - jzPos);
	(*_currentAssembly)[jzPos + 1] = jzOffset;

	return true;
}

/* IfElseStmtNode */

bool LingoCompiler::visitIfElseStmtNode(IfElseStmtNode *node) {
	NodeStore store(node);
	COMPILE(node->cond);
	uint jzPos = _currentAssembly->size();
	code2(LC::c_jumpifz, nullptr);
	COMPILE_LIST(node->stmts1);

	uint jmpPos = _currentAssembly->size();
	code2(LC::c_jump, nullptr);
	uint block2StartPos = _currentAssembly->size();
	COMPILE_LIST(node->stmts2);
	uint endPos = _currentAssembly->size();

	inst jzOffset = nullptr;
	WRITE_UINT32(&jzOffset, block2StartPos - jzPos);
	(*_currentAssembly)[jzPos + 1] = jzOffset;

	inst jmpOffset = nullptr;
	WRITE_UINT32(&jmpOffset, endPos - jmpPos);
	(*_currentAssembly)[jmpPos + 1] = jmpOffset;

	return true;
}

/* RepeatWhileNode */

bool LingoCompiler::visitRepeatWhileNode(RepeatWhileNode *node) {
	NodeStore store(node);
	LoopNode *prevLoop = _currentLoop;
	_currentLoop = node;

	uint startPos = _currentAssembly->size();
	COMPILE(node->cond);
	uint jzPos = _currentAssembly->size();
	code2(LC::c_jumpifz, nullptr);
	COMPILE_LIST(node->stmts);
	uint jmpPos = _currentAssembly->size();
	code2(LC::c_jump, nullptr);
	uint endPos = _currentAssembly->size();

	inst jzOffset = nullptr;
	WRITE_UINT32(&jzOffset, endPos - jzPos);
	(*_currentAssembly)[jzPos + 1] = jzOffset;

	inst jmpOffset = nullptr;
	WRITE_UINT32(&jmpOffset, startPos - jmpPos);
	(*_currentAssembly)[jmpPos + 1] = jmpOffset;

	updateLoopJumps(jmpPos, endPos);
	_currentLoop = prevLoop;

	return true;
}

/* RepeatWithToNode */

bool LingoCompiler::visitRepeatWithToNode(RepeatWithToNode *node) {
	NodeStore store(node);
	LoopNode *prevLoop = _currentLoop;
	_currentLoop = node;

	COMPILE(node->start);
	codeVarSet(*node->var);

	uint startPos = _currentAssembly->size();
	codeVarGet(*node->var);
	COMPILE(node->end);
	if (node->down) {
		code1(LC::c_ge);
	} else {
		code1(LC::c_le);
	}
	uint jzPos = _currentAssembly->size();
	code2(LC::c_jumpifz, nullptr);

	COMPILE_LIST(node->stmts);

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
	code2(LC::c_jump, nullptr);
	uint endPos = _currentAssembly->size();

	inst jzOffset = nullptr;
	WRITE_UINT32(&jzOffset, endPos - jzPos);
	(*_currentAssembly)[jzPos + 1] = jzOffset;

	inst jmpOffset = nullptr;
	WRITE_UINT32(&jmpOffset, startPos - jmpPos);
	(*_currentAssembly)[jmpPos + 1] = jmpOffset;

	updateLoopJumps(incrementPos, endPos);
	_currentLoop = prevLoop;

	return true;
}

/* RepeatWithInNode */

bool LingoCompiler::visitRepeatWithInNode(RepeatWithInNode *node) {
	NodeStore store(node);
	LoopNode *prevLoop = _currentLoop;
	_currentLoop = node;

	COMPILE(node->list);
	code1(LC::c_stackpeek);
	codeInt(0);
	codeFunc("count", 1);
	code1(LC::c_intpush);	// start counter
	codeInt(1);

	uint startPos = _currentAssembly->size();
	code1(LC::c_stackpeek);	// get counter
	codeInt(0);
	code1(LC::c_stackpeek);	// get array size
	codeInt(2);
	code1(LC::c_le);
	uint jzPos = _currentAssembly->size();
	code2(LC::c_jumpifz, nullptr);

	code1(LC::c_stackpeek);	// get list
	codeInt(2);
	code1(LC::c_stackpeek);	// get counter
	codeInt(1);
	codeFunc("getAt", 2);
	codeVarSet(*node->var);
	COMPILE_LIST(node->stmts);

	uint incrementPos = _currentAssembly->size();
	code1(LC::c_intpush);
	codeInt(1);
	code1(LC::c_add);	// Increment counter

	uint jmpPos = _currentAssembly->size();
	code2(LC::c_jump, nullptr);
	uint endPos = _currentAssembly->size();
	code1(LC::c_stackdrop);	// remove list, size, counter
	codeInt(3);

	inst jzOffset = nullptr;
	WRITE_UINT32(&jzOffset, endPos - jzPos);
	(*_currentAssembly)[jzPos + 1] = jzOffset;

	inst jmpOffset = nullptr;
	WRITE_UINT32(&jmpOffset, startPos - jmpPos);
	(*_currentAssembly)[jmpPos + 1] = jmpOffset;

	updateLoopJumps(incrementPos, endPos);
	_currentLoop = prevLoop;

	return true;
}

/* NextRepeatNode */

bool LingoCompiler::visitNextRepeatNode(NextRepeatNode *node) {
	NodeStore store(node);
	if (!_currentLoop) {
		warning("BUILDBOT: LingoCompiler::visitNextRepeatNode: next repeat not inside repeat loop");
		return false;
	}
	_currentLoop->nextRepeats.push_back(_currentAssembly->size());
	code2(LC::c_jump, nullptr);
	return true;
}

/* ExitRepeatNode */

bool LingoCompiler::visitExitRepeatNode(ExitRepeatNode *node) {
	NodeStore store(node);
	if (!_currentLoop) {
		warning("BUILDBOT: LingoCompiler::visitExitRepeatLoop: exit repeat not inside repeat loop");
		return false;
	}
	_currentLoop->exitRepeats.push_back(_currentAssembly->size());
	code2(LC::c_jump, nullptr);
	return true;
}

/* ExitNode */

bool LingoCompiler::visitExitNode(ExitNode *node) {
	NodeStore store(node);
	code1(LC::c_procret);
	return true;
}

/* ReturnNode */

bool LingoCompiler::visitReturnNode(ReturnNode *node) {
	NodeStore store(node);
	if (node->expr) {
		COMPILE_REF(node->expr);
		codeCmd("return", 1);
	} else {
		codeCmd("return", 0);
	}
	return true;
}


/* TellNode */

bool LingoCompiler::visitTellNode(TellNode *node) {
	NodeStore store(node);
	COMPILE(node->target);
	code1(LC::c_tell);
	COMPILE_LIST(node->stmts);
	code1(LC::c_telldone);
	return true;
}

/* WhenNode */

bool LingoCompiler::visitWhenNode(WhenNode *node) {
	NodeStore store(node);
	code1(LC::c_stringpush);
	codeString(node->code->c_str());
	code1(LC::c_whencode);
	codeString(node->event->c_str());
	return true;
}

/* DeleteNode */

bool LingoCompiler::visitDeleteNode(DeleteNode *node) {
	NodeStore store(node);
	COMPILE_REF(node->chunk);
	code1(LC::c_delete);
	return true;
}

/* HiliteNode */

bool LingoCompiler::visitHiliteNode(HiliteNode *node) {
	NodeStore store(node);
	COMPILE_REF(node->chunk);
	code1(LC::c_hilite);
	return true;
}

/* AssertErrorNode */

bool LingoCompiler::visitAssertErrorNode(AssertErrorNode *node) {
	NodeStore store(node);
	code1(LC::c_asserterror);
	COMPILE(node->stmt);
	code1(LC::c_asserterrordone);
	return true;
}

/* IntNode */

bool LingoCompiler::visitIntNode(IntNode *node) {
	NodeStore store(node);
	code1(LC::c_intpush);
	codeInt(node->val);
	return true;
}

/* FloatNode */

bool LingoCompiler::visitFloatNode(FloatNode *node) {
	NodeStore store(node);
	code1(LC::c_floatpush);
	codeFloat(node->val);
	return true;
}

/* SymbolNode */

bool LingoCompiler::visitSymbolNode(SymbolNode *node) {
	NodeStore store(node);
	code1(LC::c_symbolpush);
	codeString(node->val->c_str());
	return true;
}

/* StringNode */

bool LingoCompiler::visitStringNode(StringNode *node) {
	NodeStore store(node);
	code1(LC::c_stringpush);
	codeString(node->val->c_str());
	return true;
}

/* ListNode */

bool LingoCompiler::visitListNode(ListNode *node) {
	NodeStore store(node);
	COMPILE_LIST(node->items);
	code1(LC::c_arraypush);
	codeInt(node->items->size());
	return true;
}

/* PropListNode */

bool LingoCompiler::visitPropListNode(PropListNode *node) {
	NodeStore store(node);
	bool refModeStore = _refMode;
	_refMode = false;
	bool success = true;
	for (uint i = 0; i < node->items->size(); i++) {
		Node *item = (*node->items)[i];
		if (item->type != kPropPairNode) {
			// We have a keyless expression, as in ["key": "value", "keyless expression"]
			// Automatically set its key to its index in the list.
			code1(LC::c_intpush);
			codeInt(i + 1);
		}
		success = item->accept(this);
		if (!success)
			break;
	}
	_refMode = refModeStore;
	if (!success)
		return false;

	code1(LC::c_proparraypush);
	codeInt(node->items->size());
	return true;
}

/* PropPairNode */

bool LingoCompiler::visitPropPairNode(PropPairNode *node) {
	NodeStore store(node);
	COMPILE(node->key);
	COMPILE(node->val);
	return true;
}

/* FuncNode */

bool LingoCompiler::visitFuncNode(FuncNode *node) {
	NodeStore store(node);
	if (node->name->equalsIgnoreCase("field") && node->args->size() >= 1) {
		COMPILE((*node->args)[0]);
		if (_refMode) {
			code1(LC::c_fieldref);
		} else {
			code1(LC::c_field);
		}
		return true;
	}

	if (node->args->size() >= 1 && (*node->args)[0]->type == kVarNode) {
		// This could be a method call. Code the first arg as a reference.
		COMPILE_REF((*node->args)[0]);
		for (uint i = 1; i < node->args->size(); i++) {
			COMPILE((*node->args)[i]);
		}
	} else {
		COMPILE_LIST(node->args);
	}
	codeFunc(*node->name, node->args->size());
	return true;
}

/* VarNode */

bool LingoCompiler::visitVarNode(VarNode *node) {
	NodeStore store(node);
	if (g_director->getVersion() < 400 || (g_director->getCurrentMovie() && g_director->getCurrentMovie()->_allowOutdatedLingo)) {
		int val = castNumToNum(node->name->c_str());
		if (val != -1) {
			code1(LC::c_intpush);
			codeInt(val);
			return true;
		}
	}
	if (g_lingo->_builtinConsts.contains(*node->name)) {
		code1(LC::c_constpush);
		codeString(node->name->c_str());
		return true;
	}
	if (_refMode) {
		codeVarRef(*node->name);
		return true;
	}
	codeVarGet(*node->name);
	return true;
}

/* ParensNode */

bool LingoCompiler::visitParensNode(ParensNode *node) {
	NodeStore store(node);
	COMPILE(node->expr);
	return true;
}

/* UnaryOpNode */

bool LingoCompiler::visitUnaryOpNode(UnaryOpNode *node) {
	NodeStore store(node);
	COMPILE(node->arg);
	code1(node->op);
	return true;
}

/* BinaryOpNode */

bool LingoCompiler::visitBinaryOpNode(BinaryOpNode *node) {
	NodeStore store(node);
	COMPILE(node->a);
	COMPILE(node->b);
	code1(node->op);
	return true;
}

/* FrameNode */

bool LingoCompiler::visitFrameNode(FrameNode *node) {
	NodeStore store(node);
	COMPILE(node->arg);
	return true;
}

/* MovieNode */

bool LingoCompiler::visitMovieNode(MovieNode *node) {
	NodeStore store(node);
	COMPILE(node->arg);
	return true;
}

/* IntersectsNode */

bool LingoCompiler::visitIntersectsNode(IntersectsNode *node) {
	NodeStore store(node);
	COMPILE(node->sprite1);
	COMPILE(node->sprite2);
	code1(LC::c_intersects);
	return true;
}

/* WithinNode */

bool LingoCompiler::visitWithinNode(WithinNode *node) {
	NodeStore store(node);
	COMPILE(node->sprite1);
	COMPILE(node->sprite2);
	code1(LC::c_within);
	return true;
}

/* TheNode */

bool LingoCompiler::visitTheNode(TheNode *node) {
	NodeStore store(node);
	if (g_lingo->_theEntities.contains(*node->prop) && !g_lingo->_theEntities[*node->prop]->hasId) {
		code1(LC::c_intpush);
		codeInt(0); // Put dummy id
		code1(LC::c_theentitypush);
		codeInt(g_lingo->_theEntities[*node->prop]->entity);
		codeInt(0);	// No field
		return true;
	}

	warning("BUILDBOT: LingoCompiler:visitTheNode: Unhandled the entity '%s'", node->prop->c_str());
	return false;
}

/* TheOfNode */

bool LingoCompiler::visitTheOfNode(TheOfNode *node) {
	NodeStore store(node);
	switch (node->obj->type) {
	case kChunkExprNode:
		{
			int fieldId = getTheFieldID(kTheChunk, *node->prop, true);
			if (fieldId >= 0) {
				COMPILE_REF(node->obj);
				code1(LC::c_theentitypush);
				codeInt(kTheChunk);
				codeInt(fieldId);
				return true;
			}
			// fall back to generic object
		}
		break;
	case kFuncNode:
		{
			FuncNode *func = static_cast<FuncNode *>(node->obj);
			if (func->args->size() == 1) {
				if (func->name->equalsIgnoreCase("cast")) {
					int fieldId = getTheFieldID(kTheCast, *node->prop, true);
					if (fieldId >= 0) {
						COMPILE((*func->args)[0]);
						code1(LC::c_theentitypush);
						codeInt(kTheCast);
						codeInt(fieldId);
						return true;
					}
					// fall back to generic object
				}
				if (func->name->equalsIgnoreCase("field")) {
					int fieldId = getTheFieldID(kTheField, *node->prop, true);
					if (fieldId >= 0) {
						COMPILE((*func->args)[0]);
						code1(LC::c_theentitypush);
						codeInt(kTheField);
						codeInt(fieldId);
						return true;
					}
					// fall back to generic object
				}
				// window is an object and is handled by that case
			}
		}
		break;
	case kMenuNode:
		{
			MenuNode *menu = static_cast<MenuNode *>(node->obj);
			int fieldId = getTheFieldID(kTheMenu, *node->prop);
			if (fieldId < 0)
				return false;
			COMPILE(menu->arg);
			code1(LC::c_theentitypush);
			codeInt(kTheMenu);
			codeInt(fieldId);
			return true;
		}
		break;
	case kMenuItemNode:
		{
			MenuItemNode *menuItem = static_cast<MenuItemNode *>(node->obj);
			int fieldId = getTheFieldID(kTheMenuItem, *node->prop);
			if (fieldId < 0)
				return false;
			COMPILE(menuItem->arg1)
			COMPILE(menuItem->arg2);
			code1(LC::c_themenuentitypush);
			codeInt(kTheMenuItem);
			codeInt(fieldId);
			return true;
		}
		break;
	case kSoundNode:
		{
			SoundNode *sound = static_cast<SoundNode *>(node->obj);
			int fieldId = getTheFieldID(kTheSoundEntity, *node->prop);
			if (fieldId < 0)
				return false;
			COMPILE(sound->arg);
			code1(LC::c_theentitypush);
			codeInt(kTheSoundEntity);
			codeInt(fieldId);
			return true;
		}
		break;
	case kSpriteNode:
		{
			SpriteNode *sprite = static_cast<SpriteNode *>(node->obj);
			int fieldId = getTheFieldID(kTheSprite, *node->prop);
			if (fieldId < 0)
				return false;
			COMPILE(sprite->arg);
			code1(LC::c_theentitypush);
			codeInt(kTheSprite);
			codeInt(fieldId);
			return true;
		}
		break;
	case kVarNode:
		{
			VarNode *var = static_cast<VarNode *>(node->obj);
			if (node->prop->equalsIgnoreCase("number") && var->name->equalsIgnoreCase("castMembers")) {
				code1(LC::c_intpush);
				codeInt(0); // Put dummy id
				code1(LC::c_theentitypush);
				codeInt(kTheCastMembers);
				codeInt(kTheNumber);
				return true;
			}
		}
		break;
	default:
		break;
	}

	if (g_director->getVersion() >= 400) {
		COMPILE(node->obj);
		code1(LC::c_objectproppush);
		codeString(node->prop->c_str());
		return true;
	}

	if (g_lingo->_builtinFuncs.contains(*node->prop)) {
		COMPILE(node->obj);
		codeFunc(*node->prop, 1);
		return true;
	}

	warning("BUILDBOT: LingoCompiler::visitTheOfNode: Unhandled the field %s", node->prop->c_str());
	return false;
}

/* TheNumberOfNode */

bool LingoCompiler::visitTheNumberOfNode(TheNumberOfNode *node) {
	NodeStore store(node);
	switch (node->type) {
	case kNumberOfCastlibs:
		codeInt(0); // Put dummy id
		code1(LC::c_theentitypush);
		codeInt(kTheCastLibs);
		codeInt(kTheNumber);
		break;
	case kNumberOfChars:
		COMPILE(node->arg);
		codeFunc("numberOfChars", 1);
		break;
	case kNumberOfWords:
		COMPILE(node->arg);
		codeFunc("numberOfWords", 1);
		break;
	case kNumberOfItems:
		COMPILE(node->arg);
		codeFunc("numberOfItems", 1);
		break;
	case kNumberOfLines:
		COMPILE(node->arg);
		codeFunc("numberOfLines", 1);
		break;
	case kNumberOfMenuItems:
		{
			if (node->arg->type != kMenuNode) {
				warning("BUILDBOT: LingoCompiler::visitTheNumberOfNode: expected menu");
				return false;
			}
			MenuNode *menu = static_cast<MenuNode *>(node->arg);
			COMPILE(menu->arg);
			code1(LC::c_theentitypush);
			codeInt(kTheMenuItems);
			codeInt(kTheNumber);
		}
		break;
	case kNumberOfMenus:
		codeInt(0); // Put dummy id
		code1(LC::c_theentitypush);
		codeInt(kTheMenus);
		codeInt(kTheNumber);
		break;
	case kNumberOfXtras:
		codeInt(0); // Put dummy id
		code1(LC::c_theentitypush);
		codeInt(kTheXtras);
		codeInt(kTheNumber);
		break;
	}
	return true;
}

/* TheLastNode */

bool LingoCompiler::visitTheLastNode(TheLastNode *node) {
	NodeStore store(node);
	code1(LC::c_intpush);
	codeInt(-30000);
	code1(LC::c_intpush);
	codeInt(0);
	if (_refMode) {
		COMPILE_REF(node->arg);
	} else {
		COMPILE(node->arg);
	}
	switch (node->type) {
	case kChunkChar:
		if (_refMode) {
			code1(LC::c_charToOfRef);
		} else {
			code1(LC::c_charToOf);
		}
		break;
	case kChunkWord:
		if (_refMode) {
			code1(LC::c_wordToOfRef);
		} else {
			code1(LC::c_wordToOf);
		}
		break;
	case kChunkItem:
		if (_refMode) {
			code1(LC::c_itemToOfRef);
		} else {
			code1(LC::c_itemToOf);
		}
		break;
	case kChunkLine:
		if (_refMode) {
			code1(LC::c_lineToOfRef);
		} else {
			code1(LC::c_lineToOf);
		}
		break;
	}
	return true;
}

/* TheDateTimeNode */

bool LingoCompiler::visitTheDateTimeNode(TheDateTimeNode *node) {
	NodeStore store(node);
	code1(LC::c_intpush);
	codeInt(0); // Put dummy id
	code1(LC::c_theentitypush);
	codeInt(node->entity);
	codeInt(node->field);
	return true;
}

/* MenuNode */

bool LingoCompiler::visitMenuNode(MenuNode *node) {
	NodeStore store(node);
	return true;
}

/* MenuItemNode */

bool LingoCompiler::visitMenuItemNode(MenuItemNode *node) {
	NodeStore store(node);
	return true;
}

/* SoundItem */

bool LingoCompiler::visitSoundNode(SoundNode *node) {
	NodeStore store(node);
	return true;
}

/* SpriteNode */

bool LingoCompiler::visitSpriteNode(SpriteNode *node) {
	NodeStore store(node);
	return true;
}

/* ChunkExprNode */

bool LingoCompiler::visitChunkExprNode(ChunkExprNode *node) {
	NodeStore store(node);
	COMPILE(node->start);
	if (node->end) {
		COMPILE(node->end);
	} else {
		code1(LC::c_intpush);
		codeInt(0);
	}
	if (_refMode) {
		COMPILE_REF(node->src);
	} else {
		COMPILE(node->src);
	}
	switch (node->type) {
	case kChunkChar:
		if (_refMode) {
			code1(LC::c_charToOfRef);
		} else {
			code1(LC::c_charToOf);
		}
		break;
	case kChunkWord:
		if (_refMode) {
			code1(LC::c_wordToOfRef);
		} else {
			code1(LC::c_wordToOf);
		}
		break;
	case kChunkItem:
		if (_refMode) {
			code1(LC::c_itemToOfRef);
		} else {
			code1(LC::c_itemToOf);
		}
		break;
	case kChunkLine:
		if (_refMode) {
			code1(LC::c_lineToOfRef);
		} else {
			code1(LC::c_lineToOf);
		}
		break;
	}
	return true;
}

} // End of namespace Director
