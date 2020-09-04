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

#include "common/file.h"
#include "common/config-manager.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/util.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-object.h"

namespace Director {

Lingo *g_lingo;

Symbol::Symbol() {
	name = nullptr;
	type = VOIDSYM;
	u.s = nullptr;
	refCount = new int;
	*refCount = 1;
	nargs = 0;
	maxArgs = 0;
	parens = true;
	targetType = kNoneObj;
	argNames = nullptr;
	varNames = nullptr;
	ctx = nullptr;
	archive = nullptr;
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
	parens = s.parens;
	targetType = s.targetType;
	argNames = s.argNames;
	varNames = s.varNames;
	ctx = s.ctx;
	archive = s.archive;
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
	parens = s.parens;
	targetType = s.targetType;
	argNames = s.argNames;
	varNames = s.varNames;
	ctx = s.ctx;
	archive = s.archive;
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

Lingo::Lingo(DirectorEngine *vm) : _vm(vm) {
	g_lingo = this;

	_currentArchive = nullptr;
	_currentScript = 0;
	_currentScriptContext = nullptr;

	_assemblyArchive = nullptr;
	_currentAssembly = nullptr;
	_assemblyContext = nullptr;

	_currentChannelId = -1;
	_globalCounter = 0;
	_pc = 0;
	_abort = false;
	_indef = kStateNone;
	_indef = kStateNone;
	_immediateMode = false;
	_expectError = false;
	_caughtError = false;

	_linenumber = _colnumber = _bytenumber = 0;
	_lines[0] = _lines[1] = _lines[2] = nullptr;

	_hadError = false;

	_inFactory = false;
	_inCond = false;

	_floatPrecision = 4;
	_floatPrecisionFormat = "%.4f";

	_localvars = NULL;

	//kTheEntities
	_itemDelimiter = ',';

	// events
	_passEvent = false;
	_perFrameHook = Datum();

	_windowList.type = ARRAY;
	_windowList.u.farr = new DatumArray;

	initEventHandlerTypes();

	initBuiltIns();
	initFuncs();
	initBytecode();
	initTheEntities();
	initMethods();
	initXLibs();

	warning("Lingo Inited");
}

Lingo::~Lingo() {
	resetLingo();
	cleanupFuncs();
	cleanupMethods();
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
}

LingoArchive::~LingoArchive() {
	for (int i = 0; i <= kMaxScriptType; i++) {
		for (ScriptContextHash::iterator it = scriptContexts[i].begin(); it != scriptContexts[i].end(); ++it) {
			delete it->_value;
		}
	}
}

ScriptContext *LingoArchive::getScriptContext(ScriptType type, uint16 id) {
	if (!scriptContexts[type].contains(id)) {
		return nullptr;
	}
	return scriptContexts[type][id];
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

Symbol Lingo::getHandler(const Common::String &name) {
	if (!_eventHandlerTypeIds.contains(name)) {
		// local functions
		if (_currentScriptContext && _currentScriptContext->_functionHandlers.contains(name))
			return _currentScriptContext->_functionHandlers[name];

		Symbol sym = g_director->getCurrentMovie()->getHandler(name);
		if (sym.type != VOIDSYM)
			return sym;
	}
	return Symbol();
}

const char *Lingo::findNextDefinition(const char *s) {
	const char *res = s;

	while (*res) {
		while (*res && (*res == ' ' || *res == '\t' || *res == '\n'))
			res++;

		if (!*res)
			return NULL;

		if (!scumm_strnicmp(res, "macro ", 6)) {
			debugC(1, kDebugCompile, "findNextDefinition(): See 'macros ' construct");
			return res;
		}

		if (!scumm_strnicmp(res, "on ", 3)) {
			debugC(1, kDebugCompile, "findNextDefinition(): See 'on ' construct");
			return res;
		}

		if (!scumm_strnicmp(res, "factory ", 8)) {
			debugC(1, kDebugCompile, "findNextDefinition(): See 'factory ' construct");
			return res;
		}

		if (!scumm_strnicmp(res, "method ", 7)) {
			debugC(1, kDebugCompile, "findNextDefinition(): See 'method ' construct");
			return res;
		}

		while (*res && *res != '\n')
			res++;
	}

	return NULL;
}

void LingoArchive::addCode(const char *code, ScriptType type, uint16 id, const char *scriptName) {
	debugC(1, kDebugCompile, "Add code for type %s(%d) with id %d in '%s%s'\n"
			"***********\n%s\n\n***********", scriptType2str(type), type, id, g_director->getCurrentPath().c_str(), cast->getMacName().c_str(), code);

	if (getScriptContext(type, id)) {
		// We can't undefine context data because it could be used in e.g. symbols.
		// Although it has a legit case when kTheScriptText re sets code.
		// Warn on double definitions.
		warning("Script already defined for type %d, id %d", type, id);
	}

	Common::String contextName;
	if (scriptName && strlen(scriptName) > 0)
		contextName = Common::String(scriptName);
	else
		contextName = Common::String::format("%d", id);

	ScriptContext *sc = g_lingo->compileLingo(code, this, type, id, contextName);
	if (sc) {
		scriptContexts[type][id] = sc;
		*sc->_refCount += 1;
	}
}

ScriptContext *Lingo::compileAnonymous(const char *code) {
	debugC(1, kDebugCompile, "Compiling anonymous lingo\n"
			"***********\n%s\n\n***********", code);

	return compileLingo(code, nullptr, kNoneScript, 0, "[anonymous]", true);
}

ScriptContext *Lingo::compileLingo(const char *code, LingoArchive *archive, ScriptType type, uint16 id, const Common::String &scriptName, bool anonymous) {
	_assemblyArchive = archive;
	ScriptContext *mainContext = _assemblyContext = new ScriptContext(scriptName, archive, type, id);
	_currentAssembly = new ScriptData;

	_methodVars = new VarTypeHash;
	_linenumber = _colnumber = 1;
	_hadError = false;

	const char *begin, *end;

	if (!strncmp(code, "menu:", 5) || scumm_strcasestr(code, "\nmenu:")) {
		debugC(1, kDebugCompile, "Parsing menu");
		parseMenu(code);

		return nullptr;
	}

	// Preprocess the code for ease of the parser
	Common::String codeNorm = codePreprocessor(code, archive, type, id);
	code = codeNorm.c_str();
	begin = code;

	// macros and factories have conflicting grammar. Thus we ease life for the parser.
	if ((end = findNextDefinition(code))) {
		do {
			Common::String chunk(begin, end);

			if (chunk.hasPrefixIgnoreCase("factory")) {
				_inFactory = true;
				_assemblyContext = new ScriptContext(scriptName, archive, type, id);
			} else if (chunk.hasPrefixIgnoreCase("method")) {
				_inFactory = true;
				// remain in factory context
			} else if (chunk.hasPrefixIgnoreCase("macro") || chunk.hasPrefixIgnoreCase("on")) {
				_inFactory = false;
				_assemblyContext = mainContext;
			} else {
				_inFactory = false;
				_assemblyContext = mainContext;
			}

			debugC(1, kDebugCompile, "Code chunk:\n#####\n%s#####", chunk.c_str());

			parse(chunk.c_str());

			if (debugChannelSet(3, kDebugCompile)) {
				debugC(2, kDebugCompile, "<current code>");
				uint pc = 0;
				while (pc < _currentAssembly->size()) {
					uint spc = pc;
					Common::String instr = decodeInstruction(_assemblyArchive, _currentAssembly, pc, &pc);
					debugC(2, kDebugCompile, "[%5d] %s", spc, instr.c_str());
				}
				debugC(2, kDebugCompile, "<end code>");
			}

			begin = end;
		} while ((end = findNextDefinition(begin + 1)));

		_inFactory = false;
		_assemblyContext = mainContext;

		debugC(1, kDebugCompile, "Last code chunk:\n#####\n%s\n#####", begin);
	}

	parse(begin);

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
				Common::String instr = decodeInstruction(_assemblyArchive, _currentAssembly, pc, &pc);
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
	_assemblyContext = nullptr;
	_assemblyArchive = nullptr;
	return mainContext;
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

		if (frame->sp.type != VOIDSYM) {
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

Common::String Lingo::decodeInstruction(LingoArchive *archive, ScriptData *sd, uint pc, uint *newPc) {
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

					res += Common::String::format(" \"%s\"", archive->names[v].c_str());
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

void Lingo::execute(uint pc) {
	uint localCounter = 0;

	for (_pc = pc; !_abort && (*_currentScript)[_pc] != STOP;) {
		if (_globalCounter > 1000 && debugChannelSet(-1, kDebugFewFramesOnly)) {
			warning("Lingo::execute(): Stopping due to debug few frames only");
			_vm->getCurrentMovie()->getScore()->_playState = kPlayStopped;
			break;
		}
	
		Common::String instr = decodeInstruction(_currentArchive, _currentScript, _pc);
		uint current = _pc;

		if (debugChannelSet(5, kDebugLingoExec))
			printStack("Stack before: ", current);

		if (debugChannelSet(9, kDebugLingoExec)) {
			debug("Vars before");
			printAllVars();
			if (_currentMe.type == OBJECT)
				debug("me: %s", _currentMe.asString(true).c_str());
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

		if (!_abort && _pc >= (*_currentScript).size()) {
			warning("Lingo::execute(): Bad PC (%d)", _pc);
			break;
		}

		_globalCounter++;
		localCounter++;

		// process events every so often
		if (localCounter % 100 == 0) {
			_vm->processEvents();
			if (_vm->getCurrentMovie()->getScore()->_playState == kPlayStopped)
				break;
		}
	}

	_abort = false;
}

void Lingo::executeScript(ScriptType type, uint16 id) {
	Movie *movie = _vm->getCurrentMovie();
	if (!movie) {
		warning("Request to execute script with no movie");
		return;
	}

	ScriptContext *sc = movie->getScriptContext(type, id);

	if (!sc) {
		debugC(3, kDebugLingoExec, "Request to execute non-existent script type %d id %d", type, id);
		return;
	}

	if (!sc->_eventHandlers.contains(kEventGeneric)) {
		debugC(3, kDebugLingoExec, "Request to execute script type %d id %d with no scopeless lingo", type, id);
		return;
	}

	debugC(1, kDebugLingoExec, "Executing script type: %s, id: %d", scriptType2str(type), id);

	Symbol sym = sc->_eventHandlers[kEventGeneric];
	LC::call(sym, 0, false);
	execute(_pc);
}

void Lingo::executeHandler(const Common::String &name) {
	debugC(1, kDebugLingoExec, "Executing script handler : %s", name.c_str());
	Symbol sym = getHandler(name);
	LC::call(sym, 0, false);
	execute(_pc);
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
		_abort = true;
	}
}

void Lingo::resetLingo() {
	debugC(3, kDebugLingoExec, "Resetting Lingo!");

	g_director->_wm->removeMenu();

	// TODO
	//
	// reset the following:
	// the keyDownScript
	// the mouseUpScript
	// the mouseDownScript
	// the beepOn
	// the constraint properties
	// the cursor
	// the immediate sprite properties
	// the puppetSprite
	// cursor commands
	//
	// NOTE:
	// timeoutScript is not reset
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
			char *endPtr = 0;
			strtod(src.c_str(), &endPtr);
			if (*endPtr == 0) {
				d1Type = FLOAT;
			}
		}
	}
	if (d2Type == STRING) {
		Common::String src = d2.asString();
		if (!src.empty()) {
			char *endPtr = 0;
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

	if (d1Type == FLOAT || d2Type == FLOAT) {
		opType = FLOAT;
	} else if (d1Type == INT && d2Type == INT) {
		opType = INT;
	}

	return opType;
}

Datum::Datum() {
	u.s = nullptr;
	type = VOID;
	refCount = new int;
	*refCount = 1;
}

Datum::Datum(const Datum &d) {
	type = d.type;
	u = d.u;
	refCount = d.refCount;
	*refCount += 1;
}

Datum& Datum::operator=(const Datum &d) {
	if (this != &d && refCount != d.refCount) {
		reset();
		type = d.type;
		u = d.u;
		refCount = d.refCount;
		*refCount += 1;
	}
	return *this;
}

Datum::Datum(int val) {
	u.i = val;
	type = INT;
	refCount = new int;
	*refCount = 1;
}

Datum::Datum(double val) {
	u.f = val;
	type = FLOAT;
	refCount = new int;
	*refCount = 1;
}

Datum::Datum(const Common::String &val) {
	u.s = new Common::String(val);
	type = STRING;
	refCount = new int;
	*refCount = 1;
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
		case VAR:
		case STRING:
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
		default:
			break;
		}
		if (type != OBJECT) // object owns refCount
			delete refCount;
	}
#endif
}

Datum Datum::eval() {
	if (type == VAR || type == FIELDREF || type == CHUNKREF) {
		return g_lingo->varFetch(*this);
	}

	return *this;
}

int Datum::asInt() const {
	int res = 0;

	switch (type) {
	case STRING:
		{
			Common::String src = asString();
			char *endPtr = 0;
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
			char *endPtr = 0;
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
			s = Common::String::format("#%s", u.obj->getName().c_str());
		} else {
			s = u.obj->asString();
		}
		break;
	case VOID:
		s = "#void";
		break;
	case VAR:
		s = Common::String::format("var: #%s", u.s->c_str());
		break;
	case CASTREF:
		s = Common::String::format("cast %d", u.i);
		break;
	case FIELDREF:
		{
			int idx = u.i;
			CastMember *member = g_director->getCurrentMovie()->getCastMember(idx);
			if (!member) {
				warning("asString(): Unknown cast id %d", idx);
				s = "";
				break;
			}

			s = Common::String::format("field: \"%s\"", ((TextCastMember *)member)->getText().c_str());
		}
		break;
	case CHUNKREF:
		{
			Common::String src = u.cref->source.asString(true);
			s = Common::String::format("chunk: char %d to %d of %s", u.cref->start, u.cref->end, src.c_str());
		}
		break;
	case POINT:
		s = "point:";
		// fallthrough
	case ARRAY:
		s += "[";

		for (uint i = 0; i < u.farr->size(); i++) {
			if (i > 0)
				s += ", ";
			Datum d = u.farr->operator[](i);
			s += d.asString(printonly);
		}

		s += "]";
		break;
	case PARRAY:
		s = "[";
		if (u.parr->size() == 0)
			s += ":";
		for (uint i = 0; i < u.parr->size(); i++) {
			if (i > 0)
				s += ", ";
			Datum p = u.parr->operator[](i).p;
			Datum v = u.parr->operator[](i).v;
			s += Common::String::format("%s:%s", p.asString(printonly).c_str(), v.asString(printonly).c_str());
		}

		s += "]";
		break;
	default:
		warning("Incorrect operation asString() for type: %s", type2str());
	}

	return s;
}

int Datum::asCastId() const {
	Movie *movie = g_director->getCurrentMovie();
	if (!movie) {
		warning("Datum::asCastId: No movie");
		return 0;
	}

	int castId = 0;
	switch (type) {
	case STRING:
		{
			CastMember *member = movie->getCastMemberByName(asString());
			if (member)
				return member->getID();
			
			g_lingo->lingoError("Datum::asCastId: reference to non-existent cast member: %s", asString().c_str());
			return 0;
		}
		break;
	case INT:
	case CASTREF:
		castId = u.i;
		break;
	case FLOAT:
		castId = u.f;
		break;
	case VOID:
		warning("Datum::asCastId: reference to VOID cast ID");
		break;
	default:
		error("Datum::asCastId: unsupported cast ID type %s", type2str());
	}

	return castId;
}

const char *Datum::type2str(bool isk) const {
	static char res[20];

	switch (isk ? u.i : type) {
	case INT:
		return isk ? "#integer" : "INT";
	case FLOAT:
		return isk ? "#float" : "FLOAT";
	case STRING:
		return isk ? "#string" : "STRING";
	case CASTREF:
		return "CASTREF";
	case VOID:
		return isk ? "#void" : "VOID";
	case POINT:
		return isk ? "#point" : "POINT";
	case SYMBOL:
		return isk ? "#symbol" : "SYMBOL";
	case OBJECT:
		return isk ? "#object" : "OBJECT";
	case FIELDREF:
		return "FIELDREF";
	case CHUNKREF:
		return "CHUNKREF";
	case VAR:
		return isk ? "#var" : "VAR";
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
			return toLowercaseMac(asString()).equals(toLowercaseMac(d.asString()));
		} else {
			return asString().equals(d.asString());
		}
	case OBJECT:
		return u.obj == d.u.obj;
	case CASTREF:
		return u.i == d.u.i;
	default:
		break;
	}
	return 0;
}

int Datum::compareTo(Datum &d, bool ignoreCase) const {
	int alignType = g_lingo->getAlignedType(*this, d, false);

	if (alignType == FLOAT) {
		double f1 = asFloat();
		double f2 = d.asFloat();
		if (f1 < f2) {
			return -1;
		} else if (f1 == f2) {
			return 0;
		} else {
			return 1;
		}
	} else if (alignType == INT) {
		double i1 = asInt();
		double i2 = d.asInt();
		if (i1 < i2) {
			return -1;
		} else if (i1 == i2) {
			return 0;
		} else {
			return 1;
		}
	} else if (alignType == STRING) {
		if (ignoreCase) {
			return toLowercaseMac(asString()).compareTo(toLowercaseMac(d.asString()));
		} else {
			return asString().compareTo(d.asString());
		}
	} else {
		warning("Invalid comparison between types %s and %s", type2str(), d.type2str());
		return 0;
	}
}

void Lingo::parseMenu(const char *code) {
	warning("STUB: parseMenu");
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

			_hadError = false;
			mainArchive->addCode(script, kMovieScript, counter);

			if (!debugChannelSet(-1, kDebugCompileOnly)) {
				if (!_hadError)
					executeScript(kMovieScript, counter);
				else
					debug(">> Skipping execution");
			}

			free(script);

			counter++;
		}

		inFile.close();
	}
}

void Lingo::executeImmediateScripts(Frame *frame) {
	for (uint16 i = 0; i <= _vm->getCurrentMovie()->getScore()->_numChannelsDisplayed; i++) {
		if (_vm->getCurrentMovie()->getScore()->_immediateActions.contains(frame->_sprites[i]->_scriptId)) {
			// From D5 only explicit event handlers are processed
			// Before that you could specify commands which will be executed on mouse up
			if (_vm->getVersion() < 500)
				g_lingo->processEvent(kEventGeneric, kScoreScript, frame->_sprites[i]->_scriptId, i);
			else
				g_lingo->processEvent(kEventMouseUp, kScoreScript, frame->_sprites[i]->_scriptId, i);
		}
	}
}

void Lingo::executePerFrameHook(int frame, int subframe) {
	if (_perFrameHook.type == OBJECT) {
		Symbol method = _perFrameHook.u.obj->getMethod("mAtFrame");
		if (method.type != VOIDSYM) {
			debugC(1, kDebugLingoExec, "Executing perFrameHook : <%s>(mAtFrame, %d, %d)", _perFrameHook.asString(true).c_str(), frame, subframe);
			push(_perFrameHook);
			push(frame);
			push(subframe);
			LC::call(method, 3, false);
			execute(_pc);
		}
	}
}

void Lingo::printAllVars() {
	debugN("  Local vars: ");
	if (_localvars) {
		for (DatumHash::iterator i = _localvars->begin(); i != _localvars->end(); ++i) {
			debugN("%s, ", (*i)._key.c_str());
		}
	} else {
		debugN("(no local vars)");
	}
	debugN("\n");

	if (_currentMe.type == OBJECT && _currentMe.u.obj->getObjType() & (kFactoryObj | kScriptObj)) {
		ScriptContext *script = static_cast<ScriptContext *>(_currentMe.u.obj);
		debugN("  Instance/property vars: ");
		for (DatumHash::iterator i = script->_properties.begin(); i != script->_properties.end(); ++i) {
			debugN("%s, ", (*i)._key.c_str());
		}
		debugN("\n");
	}

	debugN("  Global vars: ");
	for (DatumHash::iterator i = _globalvars.begin(); i != _globalvars.end(); ++i) {
		debugN("%s, ", (*i)._key.c_str());
	}
	debugN("\n");
}

int Lingo::getInt(uint pc) {
	return (int)READ_UINT32(&((*_currentScript)[pc]));
}

void Lingo::varAssign(Datum &var, Datum &value, bool global, DatumHash *localvars) {
	if (localvars == nullptr) {
		localvars = _localvars;
	}

	if (var.type == VAR) {
		Common::String name = *var.u.s;

		if (localvars && localvars->contains(name)) {
			(*localvars)[name] = value;
			if (global)
				warning("varAssign: variable %s is local, not global", name.c_str());
			return;
		}
		if (_currentMe.type == OBJECT && _currentMe.u.obj->hasProp(name)) {
			_currentMe.u.obj->setProp(name, value);
			if (global)
				warning("varAssign: variable %s is instance or property, not global", name.c_str());
			return;
		}
		if (_globalvars.contains(name)) {
			_globalvars[name] = value;
			if (!global)
				warning("varAssign: variable %s is global, not local", name.c_str());
			return;
		}

		warning("varAssign: variable %s not defined", name.c_str());
	} else if (var.type == FIELDREF || var.type == CASTREF) {
		Movie *movie = g_director->getCurrentMovie();
		if (!movie) {
			warning("varAssign: Assigning to a reference to an empty movie");
			return;
		}
		int castId = var.u.i;
		CastMember *member = movie->getCastMember(castId);
		if (!member) {
			warning("varAssign: Unknown cast id %d", castId);
			return;
		}
		switch (member->_type) {
		case kCastText:
			((TextCastMember *)member)->setText(value.asString().c_str());
			break;
		default:
			warning("varAssign: Unhandled cast type %d", member->_type);
			break;
		}
	} else {
		warning("varAssign: assignment to non-variable");
	}
}

Datum Lingo::varFetch(Datum &var, bool global, DatumHash *localvars, bool silent) {
	if (localvars == nullptr) {
		localvars = _localvars;
	}

	Datum result;

	if (var.type == VAR) {
		Datum d;
		Common::String name = *var.u.s;

		if (localvars && localvars->contains(name)) {
			if (global)
				warning("varFetch: variable %s is local, not global", name.c_str());
			return (*localvars)[name];
		}
		if (_currentMe.type == OBJECT && _currentMe.u.obj->hasProp(name)) {
			if (global)
				warning("varFetch: variable %s is instance or property, not global", name.c_str());
			return _currentMe.u.obj->getProp(name);
		}
		if (_globalvars.contains(name)) {
			if (!global)
				warning("varFetch: variable %s is global, not local", name.c_str());
			return _globalvars[name];
		}

		if (!silent)
			warning("varFetch: variable %s not found", name.c_str());
		return result;
	} else if (var.type == FIELDREF || var.type == CASTREF) {
		Movie *movie = g_director->getCurrentMovie();
		if (!movie) {
			warning("varFetch: Assigning to a reference to an empty movie");
			return result;
		}
		int castId = var.u.i;
		CastMember *member = movie->getCastMember(castId);
		if (!member) {
			warning("varFetch: Unknown cast id %d", castId);
			return result;
		}
		switch (member->_type) {
		case kCastText:
			result.type = STRING;
			result.u.s = new Common::String(((TextCastMember *)member)->getText());
			break;
		default:
			warning("varFetch: Unhandled cast type %d", member->_type);
			break;
		}
	} else if (var.type == CHUNKREF) {
		Common::String src = var.u.cref->source.eval().asString();
		result.type = STRING;
		result.u.s = new Common::String(src.substr(var.u.cref->start, var.u.cref->end - var.u.cref->start));
	} else {
		warning("varFetch: fetch from non-variable");
	}

	return result;
}

} // End of namespace Director
