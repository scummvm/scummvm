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
#include "common/str.h"
#include "common/str-array.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/cast.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/util.h"

#include "graphics/macgui/macwindowmanager.h"

namespace Director {

Lingo *g_lingo;

Symbol::Symbol() {
	name = nullptr;
	type = VOID;
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
	archiveIndex = kArchMain;
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
	archiveIndex = s.archiveIndex;
}

Symbol& Symbol::operator=(const Symbol &s) {
	if (this != &s) {
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
		archiveIndex = s.archiveIndex;
	}
	return *this;
}

void Symbol::reset() {
	*refCount -= 1;
	if (*refCount <= 0) {
		if (name)
			delete name;

		if (type == HANDLER)
			delete u.defn;
		else if (type == STRING)
			delete u.s;

		if (argNames)
			delete argNames;
		if (varNames)
			delete varNames;
		delete refCount;
	}
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

	_currentScript = 0;
	_currentScriptContext = nullptr;

	_assemblyArchive = 0;
	_currentAssembly = nullptr;
	_assemblyContext = nullptr;

	_currentChannelId = -1;
	_pc = 0;
	_abort = false;
	_indef = kStateNone;
	_indef = kStateNone;
	_immediateMode = false;

	_linenumber = _colnumber = _bytenumber = _lastbytenumber = _errorbytenumber = 0;
	_ignoreError = false;

	_hadError = false;

	_inFactory = false;
	_inCond = false;

	_floatPrecision = 4;
	_floatPrecisionFormat = "%.4f";

	_cursorOnStack = false;

	_localvars = NULL;

	_dontPassEvent = false;

	_archiveIndex = kArchMain;

	_perFrameHook = Datum();

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
	resetLingo(false);
	cleanupBuiltins();
}

ScriptContext *Lingo::getScriptContext(int archiveIndex, ScriptType type, uint16 id) {
	if (archiveIndex < 0)
		return NULL;

	if (!_archives[archiveIndex].scriptContexts[type].contains(id)) {
		return NULL;
	}

	return _archives[archiveIndex].scriptContexts[type][id];
}

Common::String Lingo::getName(uint16 id) {
	Common::String result;
	if (id >= _archives[_archiveIndex].names.size()) {
		warning("Name id %d not in list", id);
		return result;
	}
	result = _archives[_archiveIndex].names[id];
	return result;
}

Symbol Lingo::getHandler(const Common::String &name) {
	Symbol result;
	if (!_eventHandlerTypeIds.contains(name)) {
		// local functions
		if (_currentScriptContext && _currentScriptContext->_functionHandlers.contains(name))
			return _currentScriptContext->_functionHandlers[name];

		// local scripts
		if (_archives[kArchMain].functionHandlers.contains(name))
			return _archives[kArchMain].functionHandlers[name];

		// shared scripts
		if (_archives[kArchShared].functionHandlers.contains(name))
			return _archives[kArchShared].functionHandlers[name];

		if (_builtins.contains(name))
			return _builtins[name];

		return result;
	}
	return result;
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

void Lingo::addCode(const char *code, int archiveIndex, ScriptType type, uint16 id, const char *scriptName) {
	if (_archiveIndex < 0) {
		warning("Lingo::addCode(): Invalid archiveIndex");
		return;
	}

	debugC(1, kDebugCompile, "Add code for type %s(%d) with id %d\n"
			"***********\n%s\n\n***********", scriptType2str(type), type, id, code);

	if (getScriptContext(archiveIndex, type, id)) {
		// We can't undefine context data because it could be used in e.g. symbols.
		// Although it has a legit case when kTheScriptText re sets code.
		// Warn on double definitions.
		warning("Script already defined for type %d, id %d", id, type);
	}

	Common::String contextName;
	if (scriptName && strlen(scriptName) > 0)
		contextName = Common::String(scriptName);
	else
		contextName = Common::String::format("%d", id);
	
	ScriptContext *sc = compileLingo(code, archiveIndex, type, id, contextName);
	_archives[_assemblyArchive].scriptContexts[type][id] = sc;
}

ScriptContext *Lingo::compileAnonymous(const char *code) {
	debugC(1, kDebugCompile, "Compiling anonymous lingo\n"
			"***********\n%s\n\n***********", code);
	
	return compileLingo(code, kArchNone, kNoneScript, 0, "[anonymous]");
}

ScriptContext *Lingo::compileLingo(const char *code, int archiveIndex, ScriptType type, uint16 id, const Common::String &scriptName) {
	_assemblyArchive = archiveIndex;
	ScriptContext *sc = _assemblyContext = new ScriptContext(type, scriptName);
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
	Common::String codeNorm = codePreprocessor(code, type, id);
	code = codeNorm.c_str();
	begin = code;

	// macros and factories have conflicting grammar. Thus we ease life for the parser.
	if ((end = findNextDefinition(code))) {
		do {
			Common::String chunk(begin, end);

			if (chunk.hasPrefixIgnoreCase("factory") || chunk.hasPrefixIgnoreCase("method"))
				_inFactory = true;
			else if (chunk.hasPrefixIgnoreCase("macro") || chunk.hasPrefixIgnoreCase("on"))
				_inFactory = false;
			else
				_inFactory = false;

			debugC(1, kDebugCompile, "Code chunk:\n#####\n%s#####", chunk.c_str());

			parse(chunk.c_str());

			if (debugChannelSet(3, kDebugCompile)) {
				debugC(2, kDebugCompile, "<current code>");
				uint pc = 0;
				while (pc < _currentAssembly->size()) {
					uint spc = pc;
					Common::String instr = decodeInstruction(_currentAssembly, pc, &pc);
					debugC(2, kDebugCompile, "[%5d] %s", spc, instr.c_str());
				}
				debugC(2, kDebugCompile, "<end code>");
			}

			begin = end;
		} while ((end = findNextDefinition(begin + 1)));

		debugC(1, kDebugCompile, "Last code chunk:\n#####\n%s\n#####", begin);
		parse(begin);

		// end of script, add a c_procret so stack frames work as expected
		code1(LC::c_procret);
		code1(STOP);
	} else {
		parse(code);

		code1(LC::c_procret);
		code1(STOP);
	}

	_inFactory = false;

	if (debugChannelSet(3, kDebugCompile)) {
		if (_currentAssembly->size() && !_hadError)
			Common::hexdump((byte *)&_currentAssembly->front(), _currentAssembly->size() * sizeof(inst));

		debugC(2, kDebugCompile, "<resulting code>");
		uint pc = 0;
		while (pc < _currentAssembly->size()) {
			uint spc = pc;
			Common::String instr = decodeInstruction(_currentAssembly, pc, &pc);
			debugC(2, kDebugCompile, "[%5d] %s", spc, instr.c_str());
		}
		debugC(2, kDebugCompile, "<end code>");
	}

	// for D4 and above, there won't be any code left. all scoped methods
	// will be defined and stored by the code parser, and this function we save
	// will be blank.
	// however D3 and below allow scopeless functions!
	Symbol currentFunc;

	currentFunc.type = HANDLER;
	currentFunc.u.defn = _currentAssembly;
	Common::String typeStr = Common::String(scriptType2str(type));
	currentFunc.name = new Common::String("[" + typeStr + " " + _assemblyContext->_name + "]");
	currentFunc.ctx = _assemblyContext;
	currentFunc.archiveIndex = _assemblyArchive;
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
	delete _methodVars;
	_methodVars = nullptr;

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
	_assemblyContext->_eventHandlers[kEventNone] = currentFunc;
	_assemblyContext = nullptr;
	_currentAssembly = nullptr;
	return sc;
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

void Lingo::execute(uint pc) {
	int counter = 0;

	for (_pc = pc; !_abort && (*_currentScript)[_pc] != STOP;) {
		Common::String instr = decodeInstruction(_currentScript, _pc);
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

		if (++counter > 1000 && debugChannelSet(-1, kDebugFewFramesOnly)) {
			warning("Lingo::execute(): Stopping due to debug few frames only");
			_vm->getCurrentScore()->_stopPlay = true;
			break;
		}
	}

	_abort = false;
}

void Lingo::executeScript(ScriptType type, uint16 id) {
	ScriptContext *sc = getScriptContext(_archiveIndex, type, id);

	if (!sc) {
		debugC(3, kDebugLingoExec, "Request to execute non-existant script type %d id %d", type, id);
		return;
	}

	debugC(1, kDebugLingoExec, "Executing script type: %s, id: %d", scriptType2str(type), id);

	Symbol sym = sc->_eventHandlers[kEventNone];
	LC::call(sym, 0);
	execute(_pc);
}

void Lingo::executeHandler(const Common::String &name) {
	debugC(1, kDebugLingoExec, "Executing script handler : %s", name.c_str());
	Symbol sym = getHandler(name);
	LC::call(sym, 0);
	execute(_pc);
}

void Lingo::resetLingo(bool keepSharedCast) {
	debugC(3, kDebugLingoExec, "Resetting Lingo!");

	for (int a = 0; a < 2; a++) {
		if (a == 1 && keepSharedCast)
			continue;

		LingoArchive *arch = &_archives[a];
		for (int i = 0; i <= kMaxScriptType; i++) {
			for (ScriptContextHash::iterator it = arch->scriptContexts[i].begin(); it != arch->scriptContexts[i].end(); ++it) {
				it->_value->_eventHandlers.clear();
				it->_value->_functionHandlers.clear();
				delete it->_value;
			}

			arch->scriptContexts[i].clear();
		}

		arch->names.clear();
		arch->functionHandlers.clear();
	}

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

int Lingo::getAlignedType(Datum &d1, Datum &d2) {
	int opType = VOID;

	int d1Type = d1.type;
	int d2Type = d2.type;

	if (d1Type == STRING || d1Type == FIELDREF) {
		Common::String src = d1.asString();
		char *endPtr = 0;
		strtod(src.c_str(), &endPtr);
		if (*endPtr == 0) {
			d1Type = FLOAT;
		}
	}
	if (d2Type == STRING || d2Type == FIELDREF) {
		Common::String src = d1.asString();
		char *endPtr = 0;
		strtod(src.c_str(), &endPtr);
		if (*endPtr == 0) {
			d2Type = FLOAT;
		}
	}

	// VOID equals to 0
	if (d1Type == VOID)
		d1Type = INT;
	if (d2Type == VOID)
		d2Type = INT;

	if (d1Type == FLOAT || d2Type == FLOAT) {
		opType = FLOAT;
	} else if ((d1Type == INT || d1Type == CASTREF) && (d2Type == INT || d2Type == CASTREF)) {
		opType = INT;
	}

	return opType;
}

void Datum::reset() {
	if (!refCount)
		return;

	*refCount -= 1;
	if (*refCount <= 0) {
		switch (type) {
		case VAR:
			// fallthrough
		case STRING:
			delete u.s;
			break;
		case ARRAY:
			// fallthrough
		case POINT:
			// fallthrough
		case RECT:
			delete u.farr;
			break;
		case PARRAY:
			delete u.parr;
			break;
		case OBJECT:
			delete u.obj;
			break;
		case CASTREF:
		case FIELDREF:
			// fallthrough
		case INT:
			// fallthrough
		case FLOAT:
			// fallthrough
		default:
			break;
		}
		delete refCount;
	}

}

Datum Datum::eval() {
	if (type != VAR) { // It could be cast ref
		lazy = false;
		return *this;
	}

	return g_lingo->varFetch(*this);
}

int Datum::asInt() {
	int res = 0;

	switch (type) {
	case STRING:
	case FIELDREF:
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
	case CASTREF:
		res = u.i;
		break;
	case FLOAT:
		res = (int)u.f;
		break;
	default:
		warning("Incorrect operation asInt() for type: %s", type2str());
	}

	return res;
}

double Datum::asFloat() {
	double res = 0.0;

	switch (type) {
	case STRING:
	case FIELDREF:
		{
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
	case CASTREF:
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

Common::String Datum::asString(bool printonly) {
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
			s = Common::String::format("#%s", u.s->c_str());
		} else {
			s = Common::String::format("symbol: #%s", u.s->c_str());
		}
		break;
	case OBJECT:
		if (!printonly) {
			s = Common::String::format("#%s", u.obj->name->c_str());
		} else {
			s = Common::String::format("object: #%s %d %p", u.obj->name->c_str(), u.obj->inheritanceLevel, (void *)u.obj);
		}
		break;
	case VOID:
		s = "#void";
		break;
	case VAR:
		s = Common::String::format("var: #%s", u.s->c_str());
		break;
	case CASTREF:
	case FIELDREF:
		{
			int idx = u.i;
			Cast *member = g_director->getCastMember(idx);
			if (!member) {
				warning("asString(): Unknown cast id %d", idx);
				s = "";
				break;
			}

			if (type == FIELDREF) {
				if (!printonly) {
					s = ((TextCast *)member)->getText();
				} else {
					s = Common::String::format("reference: \"%s\"", ((TextCast *)member)->getText().c_str());
				}
			} else {
				s = Common::String::format("cast %d", idx);
			}
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

	if (printonly && lazy) {
		s += " (lazy)";
	}

	return s;
}

const char *Datum::type2str(bool isk) {
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
	case VAR:
		return isk ? "#var" : "VAR";
	default:
		snprintf(res, 20, "-- (%d) --", type);
		return res;
	}
}

int Datum::equalTo(Datum &d, bool ignoreCase) {
	int alignType = g_lingo->getAlignedType(*this, d);

	if (alignType == FLOAT) {
		return asFloat() == d.asFloat();
	} else if (alignType == INT) {
		return asInt() == d.asInt();
	} else if ((type == STRING && d.type == STRING) || (type == SYMBOL && d.type == SYMBOL)) {
		if (ignoreCase) {
			return toLowercaseMac(asString()).equals(toLowercaseMac(d.asString()));
		} else {
			return asString().equals(d.asString());
		}
	} else if (type == OBJECT && d.type == OBJECT) {
		return u.obj == d.u.obj;
	} else {
		return 0;
	}
}

int Datum::compareTo(Datum &d, bool ignoreCase) {
	int alignType = g_lingo->getAlignedType(*this, d);

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
	} else if ((type == STRING && d.type == STRING) || (type == SYMBOL && d.type == SYMBOL)) {
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

Datum ScriptContext::getParentScript() {
	if (_parentScript.type != OBJECT) {
		_parentScript.type = OBJECT;
		_parentScript.u.obj = new Object(_name, kScriptObj, new ScriptContext(*this));
		for (Common::Array<Common::String>::iterator it = _propNames.begin(); it != _propNames.end(); ++it) {
			_parentScript.u.obj->properties[*it] = Datum();
		}
	}
	return _parentScript;
}

void Lingo::parseMenu(const char *code) {
	warning("STUB: parseMenu");
}

void Lingo::runTests() {
	Common::File inFile;
	Common::ArchiveMemberList fsList;
	SearchMan.listMatchingMembers(fsList, "*.lingo");
	Common::StringArray fileList;

	// Repurpose commandline option --start-movie to run a specific lingo script.
	if (ConfMan.hasKey("start_movie")) {
		fileList.push_back(ConfMan.get("start_movie"));
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
			addCode(script, kArchMain, kMovieScript, counter);

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
	for (uint16 i = 0; i <= _vm->getCurrentScore()->_numChannelsDisplayed; i++) {
		if (_vm->getCurrentScore()->_immediateActions.contains(frame->_sprites[i]->_scriptId)) {
			// From D5 only explicit event handlers are processed
			// Before that you could specify commands which will be executed on mouse up
			if (_vm->getVersion() < 5)
				g_lingo->processEvent(kEventNone, kArchMain, kScoreScript, frame->_sprites[i]->_scriptId, i);
			else
				g_lingo->processEvent(kEventMouseUp, kArchMain, kScoreScript, frame->_sprites[i]->_scriptId, i);
		}
	}
}

void Lingo::executePerFrameHook(int frame, int subframe) {
	if (_perFrameHook.type == OBJECT) {
		Symbol method = _perFrameHook.u.obj->getMethod("mAtFrame");
		if (method.type != VOID) {
			debugC(1, kDebugLingoExec, "Executing perFrameHook : <%s>(mAtFrame, %d, %d)", _perFrameHook.asString(true).c_str(), frame, subframe);
			push(Datum(frame));
			push(Datum(subframe));
			LC::call(method, 2);
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

	if (_currentMe.type == OBJECT) {
		debugN("  Instance/property vars: ");
		for (DatumHash::iterator i = _currentMe.u.obj->properties.begin(); i != _currentMe.u.obj->properties.end(); ++i) {
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

void Lingo::varAssign(Datum &var, Datum &value, bool global, DatumHash *localvars) {
	if (localvars == nullptr) {
		localvars = _localvars;
	}

	if (var.type != VAR && var.type != FIELDREF) {
		warning("varAssign: assignment to non-variable");
		return;
	}

	if (var.type == VAR) {
		Datum *d = nullptr;
		Common::String name = *var.u.s;

		if (localvars && localvars->contains(name)) {
			d = &(*localvars)[name];
			if (global)
				warning("varAssign: variable %s is local, not global", name.c_str());
		} else if (_currentMe.type == OBJECT && _currentMe.u.obj->hasVar(name)) {
			d = &_currentMe.u.obj->getVar(name);
			if (global)
				warning("varAssign: variable %s is instance or property, not global", name.c_str());
		} else if (_globalvars.contains(name)) {
			d = &_globalvars[name];
			if (!global)
				warning("varAssign: variable %s is global, not local", name.c_str());
		}

		if (!d) {
			warning("varAssign: variable %s not defined", name.c_str());
			return;
		}

		*d = value;
	} else if (var.type == FIELDREF) {
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

Datum Lingo::varFetch(Datum &var, bool global, DatumHash *localvars) {
	if (localvars == nullptr) {
		localvars = _localvars;
	}

	Datum result;
	result.type = VOID;
	if (var.type != VAR && var.type != FIELDREF) {
		warning("varFetch: fetch from non-variable");
		return result;
	}

	if (var.type == VAR) {
		Datum *d = nullptr;
		Common::String name = *var.u.s;

		// For kScriptObj handlers the target is an argument
		// (and can be renamed from 'me)
		if (_currentMe.type == OBJECT && _currentMe.u.obj->type != kScriptObj && name.equalsIgnoreCase("me")) {
			result = _currentMe;
			return result;
		}
		if (localvars && localvars->contains(name)) {
			d = &(*localvars)[name];
			if (global)
				warning("varFetch: variable %s is local, not global", name.c_str());
		} else if (_currentMe.type == OBJECT && _currentMe.u.obj->hasVar(name)) {
			d = &_currentMe.u.obj->getVar(name);
			if (global)
				warning("varFetch: variable %s is instance or property, not global", name.c_str());
		} else if (_globalvars.contains(name)) {
			d = &_globalvars[name];
			if (!global)
				warning("varFetch: variable %s is global, not local", name.c_str());
		}

		if (!d) {
			warning("varFetch: variable %s not found", name.c_str());
			return result;
		}

		return *d;
	} else if (var.type == FIELDREF) {
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

} // End of namespace Director
