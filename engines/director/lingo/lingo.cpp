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
#include "common/str-array.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/cast.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/sprite.h"

namespace Director {

Lingo *g_lingo;

Symbol::Symbol() {
	type = VOID;
	u.s = NULL;
	nargs = 0;
	maxArgs = 0;
	parens = true;
	global = false;
}

Lingo::Lingo(DirectorEngine *vm) : _vm(vm) {
	g_lingo = this;

	_currentScript = 0;
	_currentScriptType = kMovieScript;
	_currentEntityId = 0;
	_pc = 0;
	_returning = false;
	_nextRepeat = false;
	_indef = kStateNone;
	_ignoreMe = false;
	_immediateMode = false;

	_linenumber = _colnumber = 0;

	_hadError = false;

	_inFactory = false;
	_inCond = false;

	_floatPrecision = 4;
	_floatPrecisionFormat = "%.4f";

	_cursorOnStack = false;

	_exitRepeat = false;

	_localvars = NULL;

	_dontPassEvent = false;

	_archiveIndex = 0;

	initEventHandlerTypes();

	initBuiltIns();
	initFuncs();
	initBytecode();
	initTheEntities();

	warning("Lingo Inited");
}

Lingo::~Lingo() {
}

ScriptContext *Lingo::getScriptContext(ScriptType type, uint16 id) {
	if (type > ARRAYSIZE(_archives[_archiveIndex].scriptContexts) ||
			!_archives[_archiveIndex].scriptContexts[type].contains(id)) {
		return NULL;
	}

	return _archives[_archiveIndex].scriptContexts[type][id];
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

const char *Lingo::findNextDefinition(const char *s) {
	const char *res = s;

	while (*res) {
		while (*res && (*res == ' ' || *res == '\t' || *res == '\n'))
			res++;

		if (!*res)
			return NULL;

		if (!scumm_strnicmp(res, "macro ", 6)) {
			debugC(1, kDebugLingoCompile, "findNextDefinition(): See 'macros ' construct");
			return res;
		}

		if (!scumm_strnicmp(res, "on ", 3)) {
			debugC(1, kDebugLingoCompile, "findNextDefinition(): See 'on ' construct");
			return res;
		}

		if (!scumm_strnicmp(res, "factory ", 8)) {
			debugC(1, kDebugLingoCompile, "findNextDefinition(): See 'factory ' construct");
			return res;
		}

		if (!scumm_strnicmp(res, "method ", 7)) {
			debugC(1, kDebugLingoCompile, "findNextDefinition(): See 'method ' construct");
			return res;
		}

		while (*res && *res != '\n')
			res++;
	}

	return NULL;
}

void Lingo::addCode(const char *code, ScriptType type, uint16 id) {
	debugC(1, kDebugLingoCompile, "Add code for type %s(%d) with id %d\n"
			"***********\n%s\n\n***********", scriptType2str(type), type, id, code);

	if (getScriptContext(type, id)) {
		// We can't undefine context data because it could be used in e.g. symbols.
		// Abort on double definitions.
		error("Script already defined for type %d, id %d", id, type);
		return;
	}

	_currentScriptContext = new ScriptContext;
	_currentScriptType = type;
	_currentEntityId = id;
	_archives[_archiveIndex].scriptContexts[type][id] = _currentScriptContext;

	// FIXME: unpack into seperate functions
	_currentScriptFunction = 0;
	_currentScriptContext->functions.push_back(new Symbol);
	_currentScript = new ScriptData;
	_currentScriptContext->functions[_currentScriptFunction]->type = HANDLER;
	_currentScriptContext->functions[_currentScriptFunction]->u.defn = _currentScript;
	_currentScriptContext->functions[_currentScriptFunction]->ctx = _currentScriptContext;

	_linenumber = _colnumber = 1;
	_hadError = false;

	const char *begin, *end;

	if (!strncmp(code, "menu:", 5)) {
		debugC(1, kDebugLingoCompile, "Parsing menu");
		parseMenu(code);

		return;
	}

	// Strip comments for ease of the parser
	Common::String codeNorm = codePreprocessor(code);
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

			debugC(1, kDebugLingoCompile, "Code chunk:\n#####\n%s#####", chunk.c_str());

			parse(chunk.c_str());

			if (debugChannelSet(3, kDebugLingoCompile)) {
				debugC(2, kDebugLingoCompile, "<current code>");
				uint pc = 0;
				while (pc < _currentScript->size()) {
					uint spc = pc;
					Common::String instr = decodeInstruction(_currentScript, pc, &pc);
					debugC(2, kDebugLingoCompile, "[%5d] %s", spc, instr.c_str());
				}
				debugC(2, kDebugLingoCompile, "<end code>");
			}

			begin = end;
		} while ((end = findNextDefinition(begin + 1)));

		debugC(1, kDebugLingoCompile, "Last code chunk:\n#####\n%s\n#####", begin);
		parse(begin);

		code1(STOP);
	} else {
		parse(code);

		code1(STOP);
	}

	_inFactory = false;

	if (debugChannelSet(3, kDebugLingoCompile)) {
		if (_currentScript->size() && !_hadError)
			Common::hexdump((byte *)&_currentScript->front(), _currentScript->size() * sizeof(inst));

		debugC(2, kDebugLingoCompile, "<resulting code>");
		uint pc = 0;
		while (pc < _currentScript->size()) {
			uint spc = pc;
			Common::String instr = decodeInstruction(_currentScript, pc, &pc);
			debugC(2, kDebugLingoCompile, "[%5d] %s", spc, instr.c_str());
		}
		debugC(2, kDebugLingoCompile, "<end code>");
	}
}

void Lingo::executeScript(ScriptType type, uint16 id, uint16 function) {
	ScriptContext *sc = getScriptContext(type, id);
	if (!sc) {
		debugC(3, kDebugLingoExec, "Request to execute non-existant script type %d id %d", type, id);
		return;
	}
	if (function >= sc->functions.size()) {
		debugC(3, kDebugLingoExec, "Request to execute non-existant function %d in script type %d id %d", function, type, id);
		return;
	}

	debugC(1, kDebugLingoExec, "Executing script type: %s, id: %d, function: %d", scriptType2str(type), id, function);

	_currentScriptContext = sc;
	_currentScript = _currentScriptContext->functions[function]->u.defn;
	_pc = 0;
	_returning = false;

	_localvars = new SymbolHash;

	execute(_pc);

	cleanLocalVars();
}

void Lingo::executeHandler(Common::String name) {
	_returning = false;
	_localvars = new SymbolHash;

	debugC(1, kDebugLingoExec, "Executing script handler : %s", name.c_str());
	LC::call(name, 0);

	cleanLocalVars();
}

void Lingo::restartLingo() {
	warning("STUB: restartLingo()");

	for (int i = 0; i <= kMaxScriptType; i++) {
		for (ScriptContextHash::iterator it = _archives[_archiveIndex].scriptContexts[i].begin(); it != _archives[_archiveIndex].scriptContexts[i].end(); ++it) {
			for (size_t j = 0; j < it->_value->functions.size(); j++) {
				delete it->_value->functions[j];
			}
			delete it->_value;
		}

		_archives[_archiveIndex].scriptContexts[i].clear();
	}

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
	// custom menus
	//
	// NOTE:
	// tuneousScript is not reset
}

int Lingo::alignTypes(Datum &d1, Datum &d2) {
	int opType = VOID;

	if (d1.type == STRING) {
		char *endPtr = 0;
		int i = strtol(d1.u.s->c_str(), &endPtr, 10);
		if (*endPtr == 0) {
			d1.type = INT;
			d1.u.i = i;
		} else {
			double d = strtod(d1.u.s->c_str(), &endPtr);
			if (*endPtr == 0) {
				d1.type = FLOAT;
				d1.u.f = d;
			} else {
				warning("Unable to parse '%s' as a number", d1.u.s->c_str());
			}
		}
	}
	if (d2.type == STRING) {
		char *endPtr = 0;
		int i = strtol(d2.u.s->c_str(), &endPtr, 10);
		if (*endPtr == 0) {
			d2.type = INT;
			d2.u.i = i;
		} else {
			double d = strtod(d2.u.s->c_str(), &endPtr);
			if (*endPtr == 0) {
				d2.type = FLOAT;
				d2.u.f = d;
			} else {
				warning("Unable to parse '%s' as a number", d2.u.s->c_str());
			}
		}
	}


	if (d1.type == FLOAT || d2.type == FLOAT) {
		opType = FLOAT;
		d1.toFloat();
		d2.toFloat();
	} else if (d1.type == INT && d2.type == INT) {
		opType = INT;
	} else {
		warning("No numeric type alignment available");
	}

	return opType;
}

int Datum::toInt() {
	switch (type) {
	case REFERENCE:
		toString();
		// fallthrough
	case STRING:
		{
			char *endPtr = 0;
			int result = strtol(u.s->c_str(), &endPtr, 10);
			if (*endPtr == 0) {
				u.i = result;
			} else {
				warning("Invalid int '%s'", u.s->c_str());
				u.i = 0;
			}
		}
		break;
	case VOID:
		u.i = 0;
		break;
	case INT:
		// no-op
		break;
	case FLOAT:
		u.i = (int)u.f;
		break;
	default:
		warning("Incorrect operation toInt() for type: %s", type2str());
	}

	type = INT;

	return u.i;
}

double Datum::toFloat() {
	switch (type) {
	case REFERENCE:
		toString();
		// fallthrough
	case STRING:
		{
			char *endPtr = 0;
			double result = strtod(u.s->c_str(), &endPtr);
			if (*endPtr == 0) {
				u.f = result;
			} else {
				warning("Invalid float '%s'", u.s->c_str());
				u.f = 0.0;
			}
		}
		break;
	case VOID:
		u.f = 0.0;
		break;
	case INT:
		u.f = (double)u.i;
		break;
	case FLOAT:
		// no-op
		break;
	default:
		warning("Incorrect operation toFloat() for type: %s", type2str());
	}

	type = FLOAT;

	return u.f;
}

Common::String *Datum::toString() {
	Common::String *s = new Common::String;
	switch (type) {
	case INT:
		*s = Common::String::format("%d", u.i);
		break;
	case ARGC:
		*s = Common::String::format("argc: %d", u.i);
		break;
	case ARGCNORET:
		*s = Common::String::format("argcnoret: %d", u.i);
		break;
	case FLOAT:
		*s = Common::String::format(g_lingo->_floatPrecisionFormat.c_str(), u.f);
		break;
	case STRING:
		*s = *u.s;
		break;
	case SYMBOL:
		switch (u.i) {
		case INT:
			*s = "#integer";
			break;
		case FLOAT:
			*s = "#float";
			break;
		case STRING:
			*s = "#string";
			break;
		case SYMBOL:
			*s = "#symbol";
			break;
		case OBJECT:
			*s = "#object";
			break;
		case VOID:
			*s = "#void";
			break;
		case VAR:
			*s = "#scumm-var";
			break;
		case REFERENCE:
			*s = "#scumm-ref";
			break;
		default:
			*s = Common::String::format("#unknown%d", u.i);
		}
		break;
	case OBJECT:
		*s = Common::String::format("#%s", u.s->c_str());
		break;
	case VOID:
		*s = "#void";
		break;
	case VAR:
		*s = Common::String::format("var: #%s", u.sym->name.c_str());
		break;
	case REFERENCE:
		{
			int idx = u.i;
			Score *score = g_director->getCurrentScore();

			if (!score) {
				warning("toString(): No score");
				*s = "";
				break;
			}

			if (!score->_loadedCast->contains(idx)) {
				if (!score->_loadedCast->contains(idx - score->_castIDoffset)) {
					warning("toString(): Unknown REFERENCE %d", idx);
					*s = "";
					break;
				} else {
					idx -= 1024;
				}
			}

			*s = ((TextCast *)score->_loadedCast->getVal(idx))->_ptext;
		}
		break;
	case ARRAY:
		*s = "[";

		for (uint i = 0; i < u.farr->size(); i++) {
			if (i > 0)
				*s += ", ";
			*s += *u.farr->operator[](i).toString();
		}

		*s += "]";
		break;
	default:
		warning("Incorrect operation toString() for type: %s", type2str());
	}

	u.s = s;
	type = STRING;

	return u.s;
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
	case REFERENCE:
		return "REFERENCE";
	case VAR:
		return isk ? "#var" : "VAR";
	default:
		snprintf(res, 20, "-- (%d) --", type);
		return res;
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

	int counter = 1;

	for (Common::ArchiveMemberList::iterator it = fsList.begin(); it != fsList.end(); ++it)
		fileList.push_back((*it)->getName());

	Common::sort(fileList.begin(), fileList.end());

	for (uint i = 0; i < fileList.size(); i++) {
		Common::SeekableReadStream *const  stream = SearchMan.createReadStreamForMember(fileList[i]);
		if (stream) {
			uint size = stream->size();

			char *script = (char *)calloc(size + 1, 1);

			stream->read(script, size);

			debug(">> Compiling file %s of size %d, id: %d", fileList[i].c_str(), size, counter);

			_hadError = false;
			addCode(script, kMovieScript, counter);

			if (!debugChannelSet(-1, kDebugLingoCompileOnly)) {
				if (!_hadError)
					executeScript(kMovieScript, counter, 0);
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
			g_lingo->processEvent(kEventMouseUp, kFrameScript, frame->_sprites[i]->_scriptId);
		}
	}
}

void Lingo::printAllVars() {
	debugN("  Local vars: ");
	for (SymbolHash::iterator i = _localvars->begin(); i != _localvars->end(); ++i) {
		debugN("%s, ", (*i)._key.c_str());
	}
	debugN("\n");

	debugN("  Global vars: ");
	for (SymbolHash::iterator i = _globalvars.begin(); i != _globalvars.end(); ++i) {
		debugN("%s, ", (*i)._key.c_str());
	}
	debugN("\n");
}

} // End of namespace Director
