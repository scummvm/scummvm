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

#include "common/file.h"
#include "common/config-manager.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/util.h"
#include "director/castmember/castmember.h"
#include "director/castmember/text.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/lingo-object.h"

namespace Director {

Lingo *g_lingo;

int calcStringAlignment(const char *s) {
	return calcCodeAlignment(strlen(s) + 1);
}

int calcCodeAlignment(int l) {
	int instLen = sizeof(inst);
	return (l + instLen - 1) / instLen;
}

Symbol::Symbol() {
	name = nullptr;
	type = VOIDSYM;
	u.s = nullptr;
	refCount = new int;
	*refCount = 1;
	nargs = 0;
	maxArgs = 0;
	targetType = kNoneObj;
	argNames = nullptr;
	varNames = nullptr;
	ctx = nullptr;
	target = nullptr;
	anonymous = false;
}

Symbol::Symbol(const Symbol &s) {
	name = s.name;
	type = s.type;
	u = s.u;
	refCount = s.refCount;
	*refCount += 1;
	nargs = s.nargs;
	maxArgs = s.maxArgs;
	targetType = s.targetType;
	argNames = s.argNames;
	varNames = s.varNames;
	ctx = s.ctx;
	target = s.target;
	anonymous = s.anonymous;
}

Symbol& Symbol::operator=(const Symbol &s) {
	if (this == &s)
		return *this;

	reset();
	name = s.name;
	type = s.type;
	u = s.u;
	refCount = s.refCount;
	*refCount += 1;
	nargs = s.nargs;
	maxArgs = s.maxArgs;
	targetType = s.targetType;
	argNames = s.argNames;
	varNames = s.varNames;
	ctx = s.ctx;
	target = s.target;
	anonymous = s.anonymous;

	return *this;
}

void Symbol::reset() {
	*refCount -= 1;
	// Coverity thinks that we always free memory, as it assumes
	// (correctly) that there are cases when refCount == 0
	// Thus, DO NOT COMPILE, trick it and shut tons of false positives
#ifndef __COVERITY__
	if (*refCount <= 0) {
		if (name)
			delete name;

		if (type == HANDLER)
			delete u.defn;

		if (argNames)
			delete argNames;
		if (varNames)
			delete varNames;
		delete refCount;
	}
#endif
}

Symbol::~Symbol() {
	reset();
}

PCell::PCell() {
}

PCell::PCell(const Datum &prop, const Datum &val) {
	p = prop;
	v = val;
}

MenuReference::MenuReference() {
	menuIdNum = -1;
	menuIdStr = nullptr;
	menuItemIdNum = -1;
	menuItemIdStr = nullptr;
}

LingoState::~LingoState() {
	for (uint i = 0; i < callstack.size(); i++) {
		if (callstack[i]->retLocalVars)
			delete callstack[i]->retLocalVars;
		if (callstack[i]->retContext) {
			*callstack[i]->retContext->_refCount -= 1;
			if (*callstack[i]->retContext->_refCount == 0)
				delete callstack[i]->retContext;
		}
		delete callstack[i];
	}
	if (localVars)
		delete localVars;
	if (context) {
		*context->_refCount -= 1;
		if (*context->_refCount == 0)
			delete context;
	}

}

Lingo::Lingo(DirectorEngine *vm) : _vm(vm) {
	g_lingo = this;

	_state = nullptr;
	_currentChannelId = -1;
	_globalCounter = 0;
	_freezeState = false;
	_abort = false;
	_expectError = false;
	_caughtError = false;

	_floatPrecision = 4;
	_floatPrecisionFormat = "%.4f";

	//kTheEntities
	_actorList.type = ARRAY;
	_actorList.u.farr = new FArray;

	_itemDelimiter = ',';
	_exitLock = false;
	_preLoadEventAbort = false;
	_romanLingo = (_vm->getLanguage() != Common::JA_JPN); // Japanrdr gamrs typically require 3-byte encodings

	_searchPath.type = ARRAY;
	_searchPath.u.farr = new FArray;

	_trace = false;
	_traceLoad = 0;
	_updateMovieEnabled = false;

	// events
	_passEvent = false;
	_perFrameHook = Datum();

	_windowList.type = ARRAY;
	_windowList.u.farr = new FArray;

	_compiler = new LingoCompiler;

	initEventHandlerTypes();
	initCharNormalizations();

	initBuiltIns();
	initFuncs();
	initBytecode();
	initTheEntities();
	initMethods();
	initXLibs();

	warning("Lingo Inited");
}

Lingo::~Lingo() {
	cleanupLingo();
	cleanupFuncs();
	cleanupMethods();
	delete _compiler;
}

void Lingo::reloadBuiltIns() {
	debug("Reloading builtins");
	cleanupBuiltIns();
	cleanUpTheEntities();
	cleanupMethods();
	cleanupXLibs();
	initBuiltIns();
	initTheEntities();
	initMethods();
	initXLibs();
	reloadOpenXLibs();
}

LingoArchive::~LingoArchive() {
	// First cleanup the ScriptContexts that are only in LctxContexts.
	// LctxContexts has a huge overlap with scriptContexts.
	for (ScriptContextHash::iterator it = lctxContexts.begin(); it != lctxContexts.end(); ++it){
		ScriptContext *script = it->_value;
		if (script->getOnlyInLctxContexts()) {
			*script->_refCount -= 1;
			if (*script->_refCount <= 0)
				delete script;
		}
	}

	for (int i = 0; i <= kMaxScriptType; i++) {
		for (ScriptContextHash::iterator it = scriptContexts[i].begin(); it != scriptContexts[i].end(); ++it) {
			*it->_value->_refCount -= 1;
			if (*it->_value->_refCount <= 0)
				delete it->_value;
		}
	}

	for (auto it : factoryContexts) {
		for (auto jt : *it._value) {
			*jt._value->_refCount -= 1;
			if (*jt._value->_refCount <= 0)
				delete jt._value;
		}
		delete it._value;
	}
}

ScriptContext *LingoArchive::getScriptContext(ScriptType type, uint16 id) {
	if (!scriptContexts[type].contains(id)) {
		return nullptr;
	}
	return scriptContexts[type][id];
}

ScriptContext *LingoArchive::findScriptContext(uint16 id) {
	for (int i = 0; i < kMaxScriptType + 1; i++) {
		if (scriptContexts[i].contains(id)) {
			return scriptContexts[i][id];
		}
	}
	return nullptr;
}

Common::String LingoArchive::getName(uint16 id) {
	Common::String result;
	if (id >= names.size()) {
		warning("Name id %d not in list", id);
		return result;
	}
	result = names[id];
	return result;
}

Common::String LingoArchive::formatFunctionList(const char *prefix) {
	Common::String result;
	for (int i = 0; i <= kMaxScriptType; i++) {
		result += Common::String::format("%s%s:\n", prefix, scriptType2str((ScriptType)i));
		if (scriptContexts[i].size() == 0)
			result += Common::String::format("%s  [empty]\n", prefix);
		for (ScriptContextHash::iterator it = scriptContexts[i].begin(); it != scriptContexts[i].end(); ++it) {
			result += Common::String::format("%s  %d", prefix, it->_key);
			CastMemberInfo *cmi = cast->getCastMemberInfo(it->_key);
			if (cmi && !cmi->name.empty()) {
				result += Common::String::format(" \"%s\"", cmi->name.c_str());
			}
			result += ":\n";
			result += (*it->_value).formatFunctionList(Common::String::format("%s    ", prefix).c_str());
		}
	}
	result += Common::String::format("%sFactories:\n", prefix);
	if (factoryContexts.empty()) {
		result += Common::String::format("%s  [empty]\n", prefix);
	} else {
		for (auto it : factoryContexts) {
			result += Common::String::format("%s  %d:\n", prefix, it._key);
			if (it._value->empty()) {
				result += Common::String::format("%s    [empty]\n", prefix);
			} else {
				for (auto jt : *it._value) {
					result += Common::String::format("%s    %s:\n", prefix, jt._key.c_str());
					result += jt._value->formatFunctionList(Common::String::format("%s      ", prefix).c_str());
				}
			}
		}
	}
	return result;
}

Symbol Lingo::getHandler(const Common::String &name) {
	Symbol sym;

	// local functions
	if (_state->context && _state->context->_functionHandlers.contains(name))
		return _state->context->_functionHandlers[name];

	sym = g_director->getCurrentMovie()->getHandler(name);
	if (sym.type != VOIDSYM)
		return sym;

	sym.type = VOIDSYM;
	sym.name = new Common::String(name);
	return sym;
}

void LingoArchive::addCode(const Common::U32String &code, ScriptType type, uint16 id, const char *scriptName, uint32 preprocFlags) {
	debugC(1, kDebugCompile, "Add code for type %s(%d) with id %d in '%s%s'\n"
			"***********\n%s\n\n***********", scriptType2str(type), type, id, utf8ToPrintable(g_director->getCurrentPath()).c_str(), utf8ToPrintable(cast->getMacName()).c_str(), code.encode().c_str());

	if (getScriptContext(type, id)) {
		// Replace the pre-existing context but warn about it.
		// For cases where replacing the script context is expected (e.g. 'when' event handlers)
		// use replaceCode instead of addCode.
		warning("Script already defined for type %d, id %d", type, id);
		removeCode(type, id);
	}

	Common::String contextName;
	if (scriptName && strlen(scriptName) > 0)
		contextName = Common::String(scriptName);
	else
		contextName = Common::String::format("%d", id);

	ScriptContext *sc = g_lingo->_compiler->compileLingo(code, this, type, CastMemberID(id, cast->_castLibID), contextName, false, preprocFlags);
	if (sc) {
		scriptContexts[type][id] = sc;
		*sc->_refCount += 1;
	}
}

void LingoArchive::removeCode(ScriptType type, uint16 id) {
	ScriptContext *ctx = getScriptContext(type, id);
	if (!ctx)
		return;

	*ctx->_refCount -= 1;
	if (*ctx->_refCount <= 0) {
		delete ctx;
	}
	scriptContexts[type].erase(id);
}

void LingoArchive::replaceCode(const Common::U32String &code, ScriptType type, uint16 id, const char *scriptName) {
	removeCode(type, id);
	addCode(code, type, id, scriptName);
}

Common::String Lingo::formatStack() {
	Common::String stack;

	for (uint i = 0; i < _stack.size(); i++) {
		Datum d = _stack[i];
		stack += Common::String::format("<%s> ", d.asString(true).c_str());
	}
	return stack;
}

void Lingo::printStack(const char *s, uint pc) {
	Common::String stack(s);
	stack += formatStack();

	debugC(5, kDebugLingoExec, "[%5d]: %s", pc, stack.c_str());
}

Common::String Lingo::formatCallStack(uint pc) {
	Common::String result;
	Common::Array<CFrame *> &callstack = _state->callstack;
	if (callstack.size() == 0) {
		result += Common::String("End of execution\n");
		return result;
	}
	result += Common::String("Call stack:\n");
	for (int i = 0; i < (int)callstack.size(); i++) {
		CFrame *frame = callstack[callstack.size() - i - 1];
		uint framePc = pc;
		if (i > 0)
			framePc = callstack[callstack.size() - i]->retPC;

		if (frame->sp.type != VOIDSYM) {
			result += Common::String::format("#%d ", i);
			if (frame->sp.ctx && frame->sp.ctx->_id) {
				result += Common::String::format("%d:", frame->sp.ctx->_id);
			}
			result += Common::String::format("%s at [%5d]\n",
				frame->sp.name->c_str(),
				framePc
			);
		} else {
			result += Common::String::format("#%d [unknown] at [%5d]\n", i,
				framePc
			);
		}
	}
	return result;
}

void Lingo::printCallStack(uint pc) {
	debugC(2, kDebugLingoExec, "\n%s", formatCallStack(pc).c_str());
}

Common::String Lingo::formatFrame() {
	Common::String result;
	Common::Array<CFrame *> &callstack = _state->callstack;
	if (callstack.size() == 0) {
		return Common::String("End of execution");
	}
	if (_state->context->_id)
		result += Common::String::format("%d:", _state->context->_id);
	CFrame *frame = callstack[callstack.size() - 1];
	if (frame->sp.type == VOIDSYM || !frame->sp.name)
		result += "[unknown]";
	else
		result += frame->sp.name->c_str();
	result += Common::String::format(" at [%5d]", _state->pc);
	return result;
}

Common::String Lingo::formatCurrentInstruction() {
	Common::String instr = decodeInstruction(_state->script, _state->pc);
	if (instr.empty())
		return instr;
	return Common::String::format("[%5d]: %s", _state->pc, instr.c_str());
}

Common::String Lingo::decodeInstruction(ScriptData *sd, uint pc, uint *newPc) {
	void *opcodeFunc;
	Common::String res;
	if (!sd || pc >= sd->size())
		return res;

	opcodeFunc = (void *)(*sd)[pc++];
	if (_functions.contains(opcodeFunc)) {
		res = _functions[opcodeFunc]->name;
		const char *pars = _functions[opcodeFunc]->proto;
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
					i = (*sd)[pc++];
					double d = *(double *)(&i);

					res += Common::String::format(" %f", d);
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
			default:
				warning("Lingo::decodeInstruction(): Unknown parameter type: %c", pars[-1]);
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

Common::String Lingo::decodeScript(ScriptData *sd) {
	uint pc = 0;
	Common::String result;
	while (pc < sd->size()) {
		result += Common::String::format("[%5d] ", pc);
		result += Common::String::format("%s\n", Lingo::decodeInstruction(sd, pc, &pc).c_str());
	}
	return result;
}

Common::String Lingo::formatFunctionName(Symbol &sym) {
	Common::String result;
	if (sym.type != HANDLER)
		return result;
	if (sym.name && sym.name->size())
		result += Common::String::format("%s(", sym.name->c_str());
	else
		result += "<unknown>(";
	for (int i = 0; i < sym.nargs; i++) {
		result += (*sym.argNames)[i].c_str();
		if (i < (sym.nargs - 1))
			result += ", ";
	}
	result += ")";
	return result;
}

Common::String Lingo::formatFunctionBody(Symbol &sym) {
	Common::String result;
	if (sym.type != HANDLER)
		return result;
	if (sym.ctx && sym.ctx->_id)
		result += Common::String::format("%d:", sym.ctx->_id);
	result += formatFunctionName(sym);
	result += "\n";
	result += decodeScript(sym.u.defn);
	return result;
}

void Lingo::execute() {
	uint localCounter = 0;

	while (!_abort && !_freezeState && _state->script && (*_state->script)[_state->pc] != STOP) {
		if (_globalCounter > 1000 && debugChannelSet(-1, kDebugFewFramesOnly)) {
			warning("Lingo::execute(): Stopping due to debug few frames only");
			_vm->getCurrentMovie()->getScore()->_playState = kPlayStopped;
			break;
		}

		// process events every so often
		if (localCounter > 0 && localCounter % 100 == 0) {
			_vm->processEvents();
			g_system->updateScreen();
			if (_vm->getCurrentMovie()->getScore()->_playState == kPlayStopped) {
				_freezeState = true;
				break;
			}
		}

		uint current = _state->pc;

		if (debugChannelSet(5, kDebugLingoExec))
			printStack("Stack before: ", current);

		if (debugChannelSet(9, kDebugLingoExec)) {
			debug("Vars before");
			printAllVars();
			if (_state->me.type == OBJECT)
				debug("me: %s", _state->me.asString(true).c_str());
		}

		if (debugChannelSet(4, kDebugLingoExec)) {
			Common::String instr = decodeInstruction(_state->script, _state->pc);
			debugC(4, kDebugLingoExec, "[%5d]: %s", current, instr.c_str());
		}

		g_debugger->stepHook();

		_state->pc++;
		(*((*_state->script)[_state->pc - 1]))();

		if (debugChannelSet(5, kDebugLingoExec))
			printStack("Stack after: ", current);

		if (debugChannelSet(9, kDebugLingoExec)) {
			debug("Vars after");
			printAllVars();
		}

		_globalCounter++;
		localCounter++;

		if (!_abort && _state->pc >= (*_state->script).size()) {
			warning("Lingo::execute(): Bad PC (%d)", _state->pc);
			break;
		}
	}

	if (_freezeState) {
		debugC(5, kDebugLingoExec, "Lingo::execute(): Context is frozen, pausing execution");
		freezeState();
	} else if (_abort || _vm->getCurrentMovie()->getScore()->_playState == kPlayStopped) {
		// Clean up call stack
		while (_state->callstack.size()) {
			popContext(true);
		}
	}
	_abort = false;
	_freezeState = false;

	g_debugger->stepHook();
}

void Lingo::executeScript(ScriptType type, CastMemberID id) {
	Movie *movie = _vm->getCurrentMovie();
	if (!movie) {
		warning("Request to execute script with no movie");
		return;
	}

	ScriptContext *sc = movie->getScriptContext(type, id);

	if (!sc) {
		debugC(3, kDebugLingoExec, "Request to execute non-existent script type %d id %d of castLib %d", type, id.member, id.castLib);
		return;
	}

	if (!sc->_eventHandlers.contains(kEventGeneric)) {
		debugC(3, kDebugLingoExec, "Request to execute script type %d id %d of castLib %d with no scopeless lingo", type, id.member, id.castLib);
		return;
	}

	debugC(1, kDebugLingoExec, "Executing script type: %s, id: %d, castLib %d", scriptType2str(type), id.member, id.castLib);

	Symbol sym = sc->_eventHandlers[kEventGeneric];
	LC::call(sym, 0, false);
	execute();
}

void Lingo::executeHandler(const Common::String &name) {
	debugC(1, kDebugLingoExec, "Executing script handler : %s", name.c_str());
	Symbol sym = getHandler(name);
	LC::call(sym, 0, false);
	execute();
}

void Lingo::lingoError(const char *s, ...) {
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, 1024, s, va);
	va_end(va);

	if (_expectError) {
		warning("Caught Lingo error: %s", buf);
		_caughtError = true;
	} else {
		warning("BUILDBOT: Uncaught Lingo error: %s", buf);
		if (debugChannelSet(-1, kDebugLingoStrict)) {
			error("Uncaught Lingo error");
		}
		_abort = true;
	}
}

void Lingo::resetLingoGo() {
	// Reset lingo items that are reset on `go` command
	// Director 4 Lingo Dictionary  p.102
	Datum emptyDatum = Datum("");
	Datum dZero = Datum(0);
	Datum nullId;
	g_lingo->setTheEntity(kTheBeepOn, nullId, kTheNOField, dZero);
	g_lingo->setTheEntity(kTheKeyDownScript, nullId, kTheNOField, emptyDatum);
	g_lingo->setTheEntity(kTheMouseDownScript, nullId, kTheNOField, emptyDatum);
	g_lingo->setTheEntity(kTheMouseUpScript, nullId, kTheNOField, emptyDatum);

	// TODO
	// Should also be reset based on: Director 4 Lingo Dictionary p.102
	// the constraint properties
	// cursor of sprite
	// immediate of sprite
	// cursor
	// puppetSprite
}

void Lingo::cleanupLingo() {
	g_director->_wm->removeMenu();

	while (_state->callstack.size()) {
		popContext(true);
	}
}

void Lingo::resetLingo() {
	debugC(3, kDebugLingoExec, "Resetting Lingo!");
	cleanupLingo();
	resetLingoGo();
}

int Lingo::getAlignedType(const Datum &d1, const Datum &d2, bool numsOnly) {
	int opType = VOID;

	int d1Type = d1.type;
	int d2Type = d2.type;

	if (d1Type == d2Type && (!numsOnly || d1Type == INT || d1Type == FLOAT))
		return d1Type;

	if (d1Type == STRING) {
		Common::String src = d1.asString();
		if (!src.empty()) {
			char *endPtr = nullptr;
			strtod(src.c_str(), &endPtr);
			if (*endPtr == 0) {
				d1Type = FLOAT;
			}
		}
	}
	if (d2Type == STRING) {
		Common::String src = d2.asString();
		if (!src.empty()) {
			char *endPtr = nullptr;
			strtod(src.c_str(), &endPtr);
			if (*endPtr == 0) {
				d2Type = FLOAT;
			}
		}
	}

	// VOID equals to 0
	if (d1Type == VOID)
		d1Type = INT;
	if (d2Type == VOID)
		d2Type = INT;
	if (d1Type == OBJECT)
		d1Type = STRING;
	if (d2Type == OBJECT)
		d2Type = STRING;

	if (d1Type == FLOAT || d2Type == FLOAT) {
		opType = FLOAT;
	} else if (d1Type == INT && d2Type == INT) {
		opType = INT;
	} else if ((d1Type == STRING && d2Type == INT) || (d1Type == INT && d2Type == STRING)) {
		opType = STRING;
	}

	return opType;
}

Datum::Datum() {
	u.s = nullptr;
	type = VOID;
	refCount = new int;
	*refCount = 1;
	ignoreGlobal = false;
}

Datum::Datum(const Datum &d) {
	type = d.type;
	u = d.u;
	refCount = d.refCount;
	*refCount += 1;
	ignoreGlobal = false;
}

Datum& Datum::operator=(const Datum &d) {
	if (this != &d && refCount != d.refCount) {
		reset();
		type = d.type;
		u = d.u;
		refCount = d.refCount;
		*refCount += 1;
	}
	ignoreGlobal = false;
	return *this;
}

Datum::Datum(int val) {
	u.i = val;
	type = INT;
	refCount = new int;
	*refCount = 1;
	ignoreGlobal = false;
}

Datum::Datum(double val) {
	u.f = val;
	type = FLOAT;
	refCount = new int;
	*refCount = 1;
	ignoreGlobal = false;
}

Datum::Datum(const Common::String &val) {
	u.s = new Common::String(val);
	type = STRING;
	refCount = new int;
	*refCount = 1;
	ignoreGlobal = false;
}

Datum::Datum(AbstractObject *val) {
	u.obj = val;
	if (val) {
		type = OBJECT;
		refCount = val->getRefCount();
		*refCount += 1;
	} else {
		type = VOID;
		refCount = new int;
		*refCount = 1;
	}
	ignoreGlobal = false;
}

Datum::Datum(const CastMemberID &val) {
	u.cast = new CastMemberID(val);
	type = CASTREF;
	refCount = new int;
	*refCount = 1;
	ignoreGlobal = false;
}

Datum::Datum(const Common::Rect &rect) {
	type = RECT;
	u.farr = new FArray;
	u.farr->arr.push_back(Datum(rect.left));
	u.farr->arr.push_back(Datum(rect.top));
	u.farr->arr.push_back(Datum(rect.right));
	u.farr->arr.push_back(Datum(rect.bottom));
	refCount = new int;
	*refCount = 1;
	ignoreGlobal = false;
}

void Datum::reset() {
	if (!refCount)
		return;

	*refCount -= 1;
	// Coverity thinks that we always free memory, as it assumes
	// (correctly) that there are cases when refCount == 0
	// Thus, DO NOT COMPILE, trick it and shut tons of false positives
#ifndef __COVERITY__
	if (*refCount <= 0) {
		switch (type) {
		case VOID:
		case INT:
		case FLOAT:
		case ARGC:
		case ARGCNORET:
			break;
		case VARREF:
		case GLOBALREF:
		case LOCALREF:
		case PROPREF:
		case STRING:
		case SYMBOL:
			delete u.s;
			break;
		case ARRAY:
		case POINT:
		case RECT:
			delete u.farr;
			break;
		case PARRAY:
			delete u.parr;
			break;
		case OBJECT:
			if (u.obj->getObjType() == kWindowObj) {
				Window *window = static_cast<Window *>(u.obj);
				g_director->_wm->removeWindow(window);
				g_director->_wm->removeMarked();
			} else {
				delete u.obj;
			}
			break;
		case CHUNKREF:
			delete u.cref;
			break;
		case CASTREF:
		case FIELDREF:
			delete u.cast;
			break;
		case MENUREF:
			delete u.menu;
			break;
		case PICTUREREF:
			delete u.picture;
			break;
		default:
			warning("Datum::reset(): Unprocessed REF type %d", type);
			break;
		}
		if (type != OBJECT) // object owns refCount
			delete refCount;
	}
#endif
}

Datum Datum::eval() const {
	if (isRef()) {
		return g_lingo->varFetch(*this);
	}

	return Datum(*this);
}

int Datum::asInt() const {
	int res = 0;

	switch (type) {
	case STRING:
		{
			Common::String src = asString();
			char *endPtr = nullptr;
			int result = strtol(src.c_str(), &endPtr, 10);
			if (*endPtr == 0) {
				res = result;
			} else {
				warning("Invalid int '%s'", src.c_str());
			}
		}
		break;
	case VOID:
		// no-op
		break;
	case INT:
		res = u.i;
		break;
	case FLOAT:
		if (g_director->getVersion() < 400) {
			res = round(u.f);
		} else {
			res = (int)u.f;
		}
		break;
	default:
		warning("Incorrect operation asInt() for type: %s", type2str());
	}

	return res;
}

double Datum::asFloat() const {
	double res = 0.0;

	switch (type) {
	case STRING:		{
			Common::String src = asString();
			char *endPtr = nullptr;
			double result = strtod(src.c_str(), &endPtr);
			if (*endPtr == 0) {
				res = result;
			} else {
				warning("Invalid float '%s'", src.c_str());
			}
		}
		break;
	case VOID:
		// no-op
		break;
	case INT:
		res = (double)u.i;
		break;
	case FLOAT:
		res = u.f;
		break;
	default:
		warning("Incorrect operation makeFloat() for type: %s", type2str());
	}

	return res;
}

Common::String Datum::asString(bool printonly) const {
	Common::String s;
	switch (type) {
	case INT:
		s = Common::String::format("%d", u.i);
		break;
	case ARGC:
		s = Common::String::format("argc: %d", u.i);
		break;
	case ARGCNORET:
		s = Common::String::format("argcnoret: %d", u.i);
		break;
	case FLOAT:
		s = Common::String::format(g_lingo->_floatPrecisionFormat.c_str(), u.f);
		if (printonly)
			s += "f";		// 0.0f
		break;
	case STRING:
		if (!printonly) {
			s = *u.s;
		} else {
			s = Common::String::format("\"%s\"", u.s->c_str());
		}
		break;
	case SYMBOL:
		if (!printonly) {
			s = *u.s;
		} else {
			s = Common::String::format("#%s", u.s->c_str());
		}
		break;
	case OBJECT:
		if (!printonly) {
			// Object names in Director are: "<Object:hex>"
			// the starting '<' is important, it's used when comparing objects and integers
			s = Common::String::format("<Object:%08x>", ((uint32)(size_t)((void *)u.obj)) & 0xffffffff);
		} else {
			s = u.obj->asString();
		}
		break;
	case VOID:
		if (!printonly) {
			s = "";
		} else {
			if (g_director->getVersion() < 400) {
				s = "<NoValue>";
			} else {
				s = "<Void>";
			}
		}
		break;
	case VARREF:
		s = Common::String::format("var: #%s", u.s->c_str());
		break;
	case GLOBALREF:
		s = Common::String::format("global: #%s", u.s->c_str());
		break;
	case LOCALREF:
		s = Common::String::format("local: #%s", u.s->c_str());
		break;
	case PROPREF:
		s = Common::String::format("property: #%s", u.s->c_str());
		break;
	case CASTREF:
		s = Common::String::format("member %d of castLib %d", u.cast->member, u.cast->castLib);
		break;
	case FIELDREF:
		s = Common::String::format("field %d of castLib %d", u.cast->member, u.cast->castLib);
		break;
	case CHUNKREF:
		{
			Common::String chunkType;
			switch (u.cref->type) {
			case kChunkChar:
				chunkType = "char";
				break;
			case kChunkWord:
				chunkType = "word";
				break;
			case kChunkItem:
				chunkType = "item";
				break;
			case kChunkLine:
				 chunkType = "line";
				break;
			}
			Common::String src = u.cref->source.asString(true);
			Common::String chunk = eval().asString(true);
			s += Common::String::format("chunk: %s %d to %d of %s (%s)", chunkType.c_str(), u.cref->startChunk, u.cref->endChunk, src.c_str(), chunk.c_str());
		}
		break;
	case ARRAY:
		s += "[";

		for (uint i = 0; i < u.farr->arr.size(); i++) {
			if (i > 0)
				s += ", ";
			Datum d = u.farr->arr[i];
			s += d.asString(true);
		}

		s += "]";
		break;
	case PARRAY:
		s = "[";
		if (u.parr->arr.size() == 0)
			s += ":";
		for (uint i = 0; i < u.parr->arr.size(); i++) {
			if (i > 0)
				s += ", ";
			Datum p = u.parr->arr[i].p;
			Datum v = u.parr->arr[i].v;
			s += Common::String::format("%s: %s", p.asString(true).c_str(), v.asString(true).c_str());
		}

		s += "]";
		break;
	case POINT:
		s = "point(";
		for (uint i = 0; i < u.farr->arr.size(); i++) {
			if (i > 0)
				s += ", ";
			s += Common::String::format("%d", u.farr->arr[i].asInt());
		}
		s += ")";

		break;
	case RECT:
		s = "rect(";
		for (uint i = 0; i < u.farr->arr.size(); i++) {
			if (i > 0)
				s += ", ";
			s += Common::String::format("%d", u.farr->arr[i].asInt());
		}

		s += ")";
		break;
	case MENUREF:
		s = Common::String::format("menu(%d, %d)", u.menu->menuIdNum, u.menu->menuItemIdNum);
		break;
	case PICTUREREF:
		s = Common::String::format("picture: %p", (void*)u.picture->_picture);
		break;
	default:
		warning("Incorrect operation asString() for type: %s", type2str());
	}

	return s;
}

CastMemberID Datum::asMemberID(CastType castType) const {
	if (type == CASTREF || type == FIELDREF)
		return *u.cast;

	return g_lingo->resolveCastMember(*this, 0, castType);
}

Common::Point Datum::asPoint() const {
	if (type != POINT) {
		warning("Incorrect operation asPoint() for type: %s", type2str());
		return Common::Point(0, 0);
	}

	return Common::Point(u.farr->arr[0].asInt(), u.farr->arr[1].asInt());
}

bool Datum::isRef() const {
	return (isVarRef() || isCastRef() || type == CHUNKREF);
}

bool Datum::isVarRef() const {
	return (type == VARREF || type == GLOBALREF || type == LOCALREF || type == PROPREF);
}

bool Datum::isCastRef() const {
	return (type == CASTREF || type == FIELDREF);
}

const char *Datum::type2str(bool ilk) const {
	static char res[20];

	switch (type) {
	case ARGC:
		return "ARGC";
	case ARGCNORET:
		return "ARGCNORET";
	case ARRAY:
		return ilk ? "linearlist" : "ARRAY";
	case CASTREF:
		return "CASTREF";
	case CHUNKREF:
		return "CHUNKREF";
	case FIELDREF:
		return "FIELDREF";
	case FLOAT:
		return ilk ? "float" : "FLOAT";
	case GLOBALREF:
		return "GLOBALREF";
	case INT:
		return ilk ? "integer" : "INT";
	case LOCALREF:
		return "LOCALREF";
	case MENUREF:
		return "MENUREF";
	case OBJECT:
		return ilk ? "object" : "OBJECT";
	case PARRAY:
		return ilk ? "proplist" : "PARRAY";
	case PICTUREREF:
		return ilk ? "picture" :  "PICTUREREF";
	case POINT:
		return ilk ? "point" : "POINT";
	case PROPREF:
		return "PROPREF";
	case RECT:
		return ilk ? "rect" : "RECT";
	case STRING:
		return ilk ? "string" : "STRING";
	case SYMBOL:
		return ilk ? "symbol" : "SYMBOL";
	case VARREF:
		return "VARREF";
	case VOID:
		return ilk ? "void" : "VOID";
	default:
		snprintf(res, 20, "-- (%d) --", type);
		return res;
	}
}

int Datum::equalTo(Datum &d, bool ignoreCase) const {
	int alignType = g_lingo->getAlignedType(*this, d, false);

	switch (alignType) {
	case FLOAT:
		return asFloat() == d.asFloat();
	case INT:
		return asInt() == d.asInt();
	case STRING:
	case SYMBOL:
		if (ignoreCase) {
			return g_lingo->normalizeString(asString()).equals(g_lingo->normalizeString(d.asString()));
		} else {
			return asString().equals(d.asString());
		}
	case OBJECT:
		return u.obj == d.u.obj;
	case CASTREF:
		return *u.cast == *d.u.cast;
	case PICTUREREF:
		return 0; // Original always returns 0 on picture reference comparison
	default:
		break;
	}
	return 0;
}

bool Datum::operator==(Datum &d) const {
	return equalTo(d);
}

bool Datum::operator>(Datum &d) const {
	return compareTo(d) == kCompareGreater;
}

bool Datum::operator<(Datum &d) const {
	return compareTo(d) == kCompareLess;
}

bool Datum::operator>=(Datum &d) const {
	CompareResult res = compareTo(d);
	return res == kCompareGreater || res == kCompareEqual;
}

bool Datum::operator<=(Datum &d) const {
	CompareResult res = compareTo(d);
	return res == kCompareLess || res == kCompareEqual;
}

CompareResult Datum::compareTo(Datum &d) const {
	int alignType = g_lingo->getAlignedType(*this, d, false);

	if (alignType == FLOAT) {
		double f1 = asFloat();
		double f2 = d.asFloat();
		if (f1 < f2) {
			return kCompareLess;
		} else if (f1 == f2) {
			return kCompareEqual;
		} else {
			return kCompareGreater;
		}
	} else if (alignType == INT) {
		double i1 = asInt();
		double i2 = d.asInt();
		if (i1 < i2) {
			return kCompareLess;
		} else if (i1 == i2) {
			return kCompareEqual;
		} else {
			return kCompareGreater;
		}
	} else if (alignType == STRING) {
		int res = compareStrings(asString(), d.asString());
		if (res < 0) {
			return kCompareLess;
		} else if (res == 0) {
			return kCompareEqual;
		} else {
			return kCompareGreater;
		}
	} else {
		warning("Invalid comparison between types %s and %s", type2str(), d.type2str());
		return kCompareError;
	}
}

void Lingo::runTests() {
	Common::File inFile;
	Common::ArchiveMemberList fsList;
	SearchMan.listMatchingMembers(fsList, "*.lingo");
	Common::StringArray fileList;

	LingoArchive *mainArchive = g_director->getCurrentMovie()->getMainLingoArch();

	Common::String startMovie = _vm->getStartMovie().startMovie;
	if (startMovie.size() > 0) {
		fileList.push_back(startMovie);
	} else {
		for (Common::ArchiveMemberList::iterator it = fsList.begin(); it != fsList.end(); ++it)
			fileList.push_back((*it)->getName());
	}

	Common::sort(fileList.begin(), fileList.end());

	int counter = 1;

	for (uint i = 0; i < fileList.size(); i++) {
		Common::SeekableReadStream *const  stream = SearchMan.createReadStreamForMember(fileList[i]);
		if (stream) {
			uint size = stream->size();

			char *script = (char *)calloc(size + 1, 1);

			stream->read(script, size);

			debug(">> Compiling file %s of size %d, id: %d", fileList[i].c_str(), size, counter);

			mainArchive->addCode(Common::U32String(script, Common::kMacRoman), kTestScript, counter);

			if (!debugChannelSet(-1, kDebugCompileOnly)) {
				if (!_compiler->_hadError)
					executeScript(kTestScript, CastMemberID(counter, 0));
				else
					debug(">> Skipping execution");
			}

			free(script);
			delete stream;
			counter++;
		}

		inFile.close();
	}
}

void Lingo::executeImmediateScripts(Frame *frame) {
	for (uint16 i = 0; i <= _vm->getCurrentMovie()->getScore()->_numChannelsDisplayed; i++) {
		if (_vm->getCurrentMovie()->getScore()->_immediateActions.contains(frame->_sprites[i]->_scriptId.member)) {
			// From D5 only explicit event handlers are processed
			// Before that you could specify commands which will be executed on mouse up
			if (_vm->getVersion() < 500)
				processEvent(kEventGeneric, kScoreScript, frame->_sprites[i]->_scriptId, i);
			else
				processEvent(kEventMouseUp, kScoreScript, frame->_sprites[i]->_scriptId, i);
		}
	}
}

void Lingo::executePerFrameHook(int frame, int subframe) {
	if (_vm->getVersion() < 400) {
		if (_perFrameHook.type == OBJECT) {
			Symbol method = _perFrameHook.u.obj->getMethod("mAtFrame");
			if (method.type != VOIDSYM) {
				debugC(1, kDebugLingoExec, "Executing perFrameHook : <%s>(mAtFrame, %d, %d)", _perFrameHook.asString(true).c_str(), frame, subframe);
				push(_perFrameHook);
				push(frame);
				push(subframe);
				LC::call(method, 3, false);
				execute();
			}
		}
	} else if (_actorList.u.farr->arr.size() > 0) {
		for (uint i = 0; i < _actorList.u.farr->arr.size(); i++) {
			Datum actor = _actorList.u.farr->arr[i];
			Symbol method = actor.u.obj->getMethod("stepFrame");
			if (method.nargs == 1)
				push(actor);
			LC::call(method, method.nargs, false);
			execute();
		}
	}
}

void Lingo::cleanLocalVars() {
	// Clean up current scope local variables and clean up memory
	debugC(3, kDebugLingoExec, "cleanLocalVars: have %d vars", _state->localVars->size());

	_state->localVars->clear();
	delete _state->localVars;

	_state->localVars = nullptr;
}

Common::String Lingo::formatAllVars() {
	Common::String result;

	result += Common::String("  Local vars:\n");
	if (_state->localVars) {
		for (DatumHash::iterator i = _state->localVars->begin(); i != _state->localVars->end(); ++i) {
			result += Common::String::format("    %s - [%s] %s\n", (*i)._key.c_str(), (*i)._value.type2str(), (*i)._value.asString(true).c_str());
		}
	} else {
		result += Common::String("    (no local vars)\n");
	}
	result += Common::String("\n");

	if (_state->me.type == OBJECT && _state->me.u.obj->getObjType() & (kFactoryObj | kScriptObj)) {
		ScriptContext *script = static_cast<ScriptContext *>(_state->me.u.obj);
		result += Common::String("  Instance/property vars: \n");
		for (DatumHash::iterator i = script->_properties.begin(); i != script->_properties.end(); ++i) {
			result += Common::String::format("    %s - [%s] %s\n", (*i)._key.c_str(), (*i)._value.type2str(), (*i)._value.asString(true).c_str());
		}
		result += Common::String("\n");
	}

	result += Common::String("  Global vars:\n");
	for (DatumHash::iterator i = _globalvars.begin(); i != _globalvars.end(); ++i) {
		result += Common::String::format("    %s - [%s] %s\n", (*i)._key.c_str(), (*i)._value.type2str(), (*i)._value.asString(true).c_str());
	}
	result += Common::String("\n");
	return result;
}

void Lingo::printAllVars() {
	debugN("%s", formatAllVars().c_str());
}

int Lingo::getInt(uint pc) {
	return (int)READ_UINT32(&((*_state->script)[pc]));
}

void Lingo::varAssign(const Datum &var, const Datum &value) {
	switch (var.type) {
	case VARREF:
		{
			Common::String name = *var.u.s;
			if (_state->localVars && _state->localVars->contains(name)) {
				(*_state->localVars)[name] = value;
				g_debugger->varWriteHook(name);
				return;
			}
			if (_state->me.type == OBJECT && _state->me.u.obj->hasProp(name)) {
				_state->me.u.obj->setProp(name, value);
				g_debugger->varWriteHook(name);
				return;
			}
			_globalvars[name] = value;
			g_debugger->varWriteHook(name);
		}
		break;
	case GLOBALREF:
		// Global variables declared by `global varname` within a handler are not listed anywhere
		// in Lscr, unlike globals declared outside of a handler and every other variable type.
		// So while we require other variable types to be initialized before assigning to them,
		// let's not enforce that for globals.
		_globalvars[*var.u.s] = value;
		break;
	case LOCALREF:
		{
			Common::String name = *var.u.s;
			if (_state->localVars && _state->localVars->contains(name)) {
				(*_state->localVars)[name] = value;
				g_debugger->varWriteHook(name);
			} else {
				warning("varAssign: local variable %s not defined", name.c_str());
			}
		}
		break;
	case PROPREF:
		{
			Common::String name = *var.u.s;
			if (_state->me.type == OBJECT && _state->me.u.obj->hasProp(name)) {
				_state->me.u.obj->setProp(name, value);
				g_debugger->varWriteHook(name);
			} else {
				warning("varAssign: property %s not defined", name.c_str());
			}
		}
		break;
	case FIELDREF:
	case CASTREF:
		{
			Movie *movie = g_director->getCurrentMovie();
			if (!movie) {
				warning("varAssign: Assigning to a reference to an empty movie");
				return;
			}
			CastMember *member = movie->getCastMember(*var.u.cast);
			if (!member) {
				warning("varAssign: Unknown %s", var.u.cast->asString().c_str());
				return;
			}
			switch (member->_type) {
			case kCastText:
				((TextCastMember *)member)->setRawText(value.asString());
				break;
			default:
				warning("varAssign: Unhandled cast type %d", member->_type);
				break;
			}
		}
		break;
	case CHUNKREF:
		{
			Common::U32String src = evalChunkRef(var.u.cref->source);
			Common::U32String res;
			if (var.u.cref->start >= 0) {
				res = src.substr(0, var.u.cref->start) + value.asString().decode(Common::kUtf8) + src.substr(var.u.cref->end);
			} else {
				// non-existent chunk - insert more chars, items, or lines
				res = src;
				int numberOfChunks = LC::lastChunk(var.u.cref->type, var.u.cref->source).u.cref->startChunk;
				switch (var.u.cref->type) {
				case kChunkChar:
					while (numberOfChunks < var.u.cref->startChunk - 1) {
						res += ' ';
						numberOfChunks++;
					}
					break;
				case kChunkWord:
					break;
				case kChunkItem:
					while (numberOfChunks < var.u.cref->startChunk ) {
						res += _itemDelimiter;
						numberOfChunks++;
					}
					break;
				case kChunkLine:
					while (numberOfChunks < var.u.cref->startChunk ) {
						res += '\r';
						numberOfChunks++;
					}
					break;
				}
				res += value.asString();
			}
			varAssign(var.u.cref->source, res.encode(Common::kUtf8));
		}
		break;
	default:
		warning("varAssign: assignment to non-variable");
		break;
	}
}

Datum Lingo::varFetch(const Datum &var, bool silent) {
	Datum result;

	switch (var.type) {
	case VARREF:
		{
			Datum d;
			Common::String name = *var.u.s;
			g_debugger->varReadHook(name);

			if (_state->localVars && _state->localVars->contains(name)) {
				return (*_state->localVars)[name];
			}
			if (_state->me.type == OBJECT && _state->me.u.obj->hasProp(name)) {
				return _state->me.u.obj->getProp(name);
			}
			if (_globalvars.contains(name)) {
				return _globalvars[name];
			}

			if (!silent)
				debugC(1, kDebugLingoExec, "varFetch: variable %s not found", name.c_str());
			return result;
		}
		break;
	case GLOBALREF:
		{
			Common::String name = *var.u.s;
			g_debugger->varReadHook(name);
			if (_globalvars.contains(name)) {
				return _globalvars[name];
			}
			debugC(1, kDebugLingoExec, "varFetch: global variable %s not defined", name.c_str());
			return result;
		}
		break;
	case LOCALREF:
		{
			Common::String name = *var.u.s;
			g_debugger->varReadHook(name);
			if (_state->localVars && _state->localVars->contains(name)) {
				return (*_state->localVars)[name];
			}
			debugC(1, kDebugLingoExec, "varFetch: local variable %s not defined", name.c_str());
			return result;
		}
		break;
	case PROPREF:
		{
			Common::String name = *var.u.s;
			g_debugger->varReadHook(name);
			if (_state->me.type == OBJECT && _state->me.u.obj->hasProp(name)) {
				return _state->me.u.obj->getProp(name);
			}
			warning("varFetch: property %s not defined", name.c_str());
			return result;
		}
		break;
	case FIELDREF:
	case CASTREF:
	case CHUNKREF:
		{
			Common::String chunk(evalChunkRef(var), Common::kUtf8);
			result = Datum(chunk);
		}
		break;
	default:
		warning("varFetch: fetch from non-variable");
		break;
	}

	return result;
}

Common::U32String Lingo::evalChunkRef(const Datum &var) {
	Common::U32String result;

	switch (var.type) {
	case VARREF:
	case GLOBALREF:
	case LOCALREF:
	case PROPREF:
		result = varFetch(var).asString().decode(Common::kUtf8);
		break;
	case FIELDREF:
	case CASTREF:
		{
			Movie *movie = g_director->getCurrentMovie();
			if (!movie) {
				warning("evalChunkRef: Assigning to a reference to an empty movie");
				return result;
			}
			CastMember *member = movie->getCastMember(*var.u.cast);
			if (!member) {
				warning("evalChunkRef: Unknown %s", var.u.cast->asString().c_str());
				return result;
			}
			switch (member->_type) {
			case kCastText:
				result = ((TextCastMember *)member)->getText();
				break;
			default:
				warning("evalChunkRef: Unhandled cast type %d", member->_type);
				break;
			}
		}
		break;
	case CHUNKREF:
		{
			Common::U32String src = evalChunkRef(var.u.cref->source);
			if (var.u.cref->start >= 0) {
				result = src.substr(var.u.cref->start, var.u.cref->end - var.u.cref->start);
			}
		}
		break;
	default:
		result = var.asString().decode(Common::kUtf8);
		break;
	}

	return result;
}

CastMemberID Lingo::resolveCastMember(const Datum &memberID, const Datum &castLib, CastType type) {
	Movie *movie = g_director->getCurrentMovie();
	if (!movie) {
		warning("Lingo::resolveCastMember: No movie");
		return CastMemberID(-1, castLib.asInt());
	}

	switch (memberID.type) {
	case STRING:
		{
			CastMember *member = movie->getCastMemberByNameAndType(memberID.asString(), castLib.asInt(), type);
			if (member)
				return CastMemberID(member->getID(), castLib.asInt());

			warning("Lingo::resolveCastMember: reference to non-existent cast member: %s", memberID.asString().c_str());
			return CastMemberID(-1, castLib.asInt());
		}
		break;
	case INT:
	case FLOAT:
		return CastMemberID(memberID.asInt(), castLib.asInt());
		break;
	case VOID:
		warning("Lingo::resolveCastMember: reference to VOID member ID");
		break;
	default:
		error("Lingo::resolveCastMember: unsupported member ID type %s", memberID.type2str());
	}

	return CastMemberID(-1, castLib.asInt());
}

void Lingo::exposeXObject(const char *name, Datum obj) {
	_globalvars[name] = obj;
	_globalvars[name].ignoreGlobal = true;
}

PictureReference::~PictureReference() {
	delete _picture;
}

} // End of namespace Director
