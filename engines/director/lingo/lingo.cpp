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

#include "common/archive.h"
#include "common/file.h"
#include "common/str-array.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"
#include "director/frame.h"
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
	_indef = false;
	_ignoreMe = false;
	_immediateMode = false;

	_linenumber = _colnumber = 0;

	_hadError = false;

	_inFactory = false;

	_floatPrecision = 4;
	_floatPrecisionFormat = "%.4f";

	_cursorOnStack = false;

	_exitRepeat = false;

	_localvars = NULL;

	initEventHandlerTypes();

	initBuiltIns();
	initFuncs();
	initTheEntities();

	warning("Lingo Inited");
}

Lingo::~Lingo() {
}

const char *Lingo::findNextDefinition(const char *s) {
	const char *res = s;

	while (*res) {
		while (*res && (*res == ' ' || *res == '\t' || *res == '\n'))
			res++;

		if (!*res)
			return NULL;

		if (!strncmp(res, "macro ", 6)) {
			debugC(1, kDebugLingoCompile, "See macro");
			return res;
		}

		if (!strncmp(res, "on ", 3)) {
			debugC(1, kDebugLingoCompile, "See on");
			return res;
		}

		if (!strncmp(res, "factory ", 8)) {
			debugC(1, kDebugLingoCompile, "See factory");
			return res;
		}

		if (!strncmp(res, "method ", 7)) {
			debugC(1, kDebugLingoCompile, "See method");
			return res;
		}

		while (*res && *res != '\n')
			res++;
	}

	return NULL;
}

void Lingo::addCode(const char *code, ScriptType type, uint16 id) {
	debugC(1, kDebugLingoCompile, "Add code \"%s\" for type %s with id %d", code, scriptType2str(type), id);

	if (_scripts[type].contains(id)) {
		delete _scripts[type][id];
	}

	_currentScript = new ScriptData;
	_currentScriptType = type;
	_scripts[type][id] = _currentScript;
	_currentEntityId = id;

	_linenumber = _colnumber = 1;
	_hadError = false;

	const char *begin, *end;

	if (!strncmp(code, "menu:", 5)) {
		debugC(1, kDebugLingoCompile, "Parsing menu");
		parseMenu(code);

		return;
	}

	// macros and factories have conflicting grammar. Thus we ease life for the parser.
	if ((begin = findNextDefinition(code))) {
		bool first = true;

		while ((end = findNextDefinition(begin + 1))) {

			if (first) {
				begin = code;
				first = false;
			}
			Common::String chunk(begin, end);

			if (chunk.hasPrefix("factory") || chunk.hasPrefix("method"))
				_inFactory = true;
			else if (chunk.hasPrefix("macro") || chunk.hasPrefix("on"))
				_inFactory = false;
			else
				_inFactory = false;

			debugC(1, kDebugLingoCompile, "Code chunk:\n#####\n%s#####", chunk.c_str());

			parse(chunk.c_str());

			if (debugChannelSet(3, kDebugLingoCompile)) {
				uint pc = 0;
				while (pc < _currentScript->size()) {
					Common::String instr = decodeInstruction(pc, &pc);
					debugC(2, kDebugLingoCompile, "[%5d] %s", pc, instr.c_str());
				}
			}

			_currentScript->clear();

			begin = end;
		}

		_hadError = true; // HACK: This is for preventing test execution

		debugC(1, kDebugLingoCompile, "Code chunk:\n#####\n%s#####", begin);
		parse(begin);
	} else {
		parse(code);

		code1(STOP);
	}

	_inFactory = false;

	if (debugChannelSet(3, kDebugLingoCompile)) {
		if (_currentScript->size() && !_hadError)
			Common::hexdump((byte *)&_currentScript->front(), _currentScript->size() * sizeof(inst));

		uint pc = 0;
		while (pc < _currentScript->size()) {
			Common::String instr = decodeInstruction(pc, &pc);
			debugC(2, kDebugLingoCompile, "[%5d] %s", pc, instr.c_str());
		}
	}
}

void Lingo::executeScript(ScriptType type, uint16 id) {
	if (!_scripts[type].contains(id)) {
		debugC(3, kDebugLingoExec, "Request to execute non-existant script type %d id %d", type, id);
		return;
	}

	debugC(1, kDebugLingoExec, "Executing script type: %s, id: %d", scriptType2str(type), id);

	_currentScript = _scripts[type][id];
	_pc = 0;
	_returning = false;

	_localvars = new SymbolHash;

	execute(_pc);

	cleanLocalVars();
}

void Lingo::restartLingo() {
	warning("STUB: restartLingo()");

	for (int i = 0; i <= kMaxScriptType; i++) {
		for (ScriptHash::iterator it = _scripts[i].begin(); it != _scripts[i].end(); ++it)
			delete it->_value;

		_scripts[i].clear();
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
	int opType = INT;

	if (d1.type == FLOAT || d2.type == FLOAT) {
		opType = FLOAT;
		d1.toFloat();
		d2.toFloat();
	}

	return opType;
}

int Datum::toInt() {
	switch (type) {
	case INT:
		// no-op
		break;
	case FLOAT:
		u.i = (int)u.f;
		type = INT;
		break;
	default:
		warning("Incorrect operation toInt() for type: %s", type2str());
	}

	return u.i;
}

double Datum::toFloat() {
	switch (type) {
	case INT:
		u.f = (double)u.i;
		type = FLOAT;
		break;
	case FLOAT:
		// no-op
		break;
	default:
		warning("Incorrect operation toFloat() for type: %s", type2str());
	}

	return u.f;
}

Common::String *Datum::toString() {
	Common::String *s = new Common::String;
	switch (type) {
	case INT:
		*s = Common::String::format("%d", u.i);
		break;
	case FLOAT:
		*s = Common::String::format(g_lingo->_floatPrecisionFormat.c_str(), u.f);
		break;
	case STRING:
		delete s;
		s = u.s;
		break;
	case SYMBOL:
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
		*s = Common::String::format("field#%d", u.i);
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

			if (!_hadError)
				executeScript(kMovieScript, counter);
			else
				debug(">> Skipping execution");

			free(script);

			counter++;
		}

		inFile.close();
	}
}

void Lingo::executeImmediateScripts(Frame *frame) {
	for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
		if (_vm->getCurrentScore()->_immediateActions.contains(frame->_sprites[i]->_scriptId)) {
			g_lingo->processEvent(kEventMouseUp, kFrameScript, frame->_sprites[i]->_scriptId);
		}
	}
}

} // End of namespace Director
