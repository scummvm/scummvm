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
	_inCond = false;

	_floatPrecision = 4;
	_floatPrecisionFormat = "%.4f";

	_cursorOnStack = false;

	_exitRepeat = false;

	_localvars = NULL;

	initEventHandlerTypes();

	initBuiltIns();
	initFuncs();
	initBytecode();
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
	debugC(1, kDebugLingoCompile, "Add code for type %s with id %d\n"
			"***********\n%s\n\n***********", scriptType2str(type), id, code);

	if (_scriptContexts[type].contains(id)) {
		for (size_t j = 0; j < _scriptContexts[type][id]->functions.size(); j++) {
			delete _scriptContexts[type][id]->functions[j];
		}
		delete _scriptContexts[type][id];
	}

	_currentScriptContext = new ScriptContext;
	_currentScriptType = type;
	_currentEntityId = id;
	_scriptContexts[type][id] = _currentScriptContext;

	// FIXME: unpack into seperate functions
	_currentScriptFunction = 0;
	_currentScriptContext->functions.push_back(new ScriptData);
	_currentScript = _currentScriptContext->functions[_currentScriptFunction];

	_linenumber = _colnumber = 1;
	_hadError = false;

	const char *begin, *end;

	if (!strncmp(code, "menu:", 5)) {
		debugC(1, kDebugLingoCompile, "Parsing menu");
		parseMenu(code);

		return;
	}

	// Strip comments for ease of the parser
	Common::String codeNorm = stripComments(code);
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
					Common::String instr = decodeInstruction(_currentScript, pc, &pc);
					debugC(2, kDebugLingoCompile, "[%5d] %s", pc, instr.c_str());
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
			Common::String instr = decodeInstruction(_currentScript, pc, &pc);
			debugC(2, kDebugLingoCompile, "[%5d] %s", pc, instr.c_str());
		}
		debugC(2, kDebugLingoCompile, "<end code>");
	}
}

static Common::String nexttok(const char *s, const char **newP = nullptr) {
	Common::String res;

	// Scan first non-whitespace
	while (*s && (*s == ' ' || *s == '\t')) // If we see a whitespace
		s++;

	// Now copy everything till whitespace
	while (*s && *s != ' ' && *s != '\t' && *s != '\n')
		res += *s++;

	if (newP)
		*newP = s;

	return res;
}

static Common::String prevtok(const char *s, const char *lineStart, const char **newP = nullptr) {
	Common::String res;

	// Scan first non-whitespace
	while (s >= lineStart && (*s == ' ' || *s == '\t')) // If we see a whitespace
		s--;

	// Now copy everything till whitespace
	while (s >= lineStart && *s != ' ' && *s != '\t')
		res = *s-- + res;

	if (newP)
		*newP = s;

	return res;
}

Common::String Lingo::stripComments(const char *s) {
	Common::String res;

	// Strip comments
	while (*s) {
		if (*s == '-' && *(s + 1) == '-') { // At the end of the line we will have \0
			while (*s && *s != '\n')
				s++;
		}

		if (*s)
			res += *s;

		s++;
	}

	Common::String tmp(res);
	res.clear();

	// Strip trailing whitespaces
	s = tmp.c_str();
	while (*s) {
		if (*s == ' ' || *s == '\t') { // If we see a whitespace
			const char *ps = s; // Remember where we saw it

			while (*ps == ' ' || *ps == '\t')	// Scan until end of whitespaces
				ps++;

			if (*ps) {	// Not end of the string
				if (*ps == '\n') {	// If it is newline, then we continue from it
					s = ps;
				} else {	// It is not a newline
					while (s != ps) {	// Add all whitespaces
						res += *s;
						s++;
					}
				}
			}
		}

		if (*s)
			res += *s;

		s++;
	}

	tmp = res;
	s = tmp.c_str();
	res.clear();

	// Preprocess if statements
	Common::String line, tok;
	const char *lineStart, *prevEnd;
	int iflevel = 0;

	while (*s) {
		line.clear();

		// Get next line
		while (*s && *s != '\n') { // If we see a whitespace
			if (*s == '\xc2') {
				res += *s++;
				if (*s == '\n') {
					line += ' ';
					res += *s++;
				}
			} else {
				res += *s;
				line += tolower(*s++);
			}
		}
		debugC(2, kDebugLingoParse, "line: %d                         '%s'", iflevel, line.c_str());

		if (line.size() < 4) { // If line is too small, then skip it
			if (*s)	// copy newline symbol
				res += *s++;

			debugC(2, kDebugLingoParse, "too small");

			continue;
		}

		tok = nexttok(line.c_str(), &lineStart);
		if (tok.equals("if")) {
			tok = prevtok(&line.c_str()[line.size() - 1], lineStart, &prevEnd);
			debugC(2, kDebugLingoParse, "start-if <%s>", tok.c_str());

			if (tok.equals("if")) {
				debugC(2, kDebugLingoParse, "end-if");
				tok = prevtok(prevEnd, lineStart);

				if (tok.equals("end")) {
					// do nothing, we open and close same line
					debugC(2, kDebugLingoParse, "end-end");
				} else {
					iflevel++;
				}
			} else if (tok.equals("then")) {
				debugC(2, kDebugLingoParse, "last-then");
				iflevel++;
			} else if (tok.equals("else")) {
				debugC(2, kDebugLingoParse, "last-else");
				iflevel++;
			} else { // other token
				// Now check if we have tNLELSE
				if (!*s) {
					iflevel++;	// end, we have to add 'end if'
					break;
				}
				const char *s1 = s + 1;

				while (*s1 && *s1 == '\n')
					s1++;
				tok = nexttok(s1);

				if (tok.equalsIgnoreCase("else")) { // ignore case because it is look-ahead
					debugC(2, kDebugLingoParse, "tNLELSE");
					iflevel++;
				} else {
					debugC(2, kDebugLingoParse, "++++ end if (no nlelse after single liner)");
					res += " end if";
				}
			}
		} else if (tok.equals("else")) {
			debugC(2, kDebugLingoParse, "start-else");
			bool elseif = false;

			tok = nexttok(lineStart);
			if (tok.equals("if")) {
				debugC(2, kDebugLingoParse, "second-if");
				elseif = true;
			} else if (tok.empty()) {
				debugC(2, kDebugLingoParse, "lonely-else");
				continue;
			}

			tok = prevtok(&line.c_str()[line.size() - 1], lineStart, &prevEnd);
			debugC(2, kDebugLingoParse, "last: '%s'", tok.c_str());

			if (tok.equals("if")) {
				debugC(2, kDebugLingoParse, "end-if");
				tok = prevtok(prevEnd, lineStart);

				if (tok.equals("end")) {
					debugC(2, kDebugLingoParse, "end-end");
					iflevel--;
				}
			} else if (tok.equals("then")) {
				debugC(2, kDebugLingoParse, "last-then");

				if (elseif == false) {
					warning("Badly nested then");
				}
			} else if (tok.equals("else")) {
				debugC(2, kDebugLingoParse, "last-else");
				if (elseif == false) {
					warning("Badly nested else");
				}
			} else { // check if we have tNLELSE
				if (!*s) {
					break;
				}
				const char *s1 = s + 1;

				while (*s1 && *s1 == '\n')
					s1++;
				tok = nexttok(s1);

				if (tok.equalsIgnoreCase("else")) {
					// Nothing to do here, same level
					debugC(2, kDebugLingoParse, "tNLELSE");
				} else {
					debugC(2, kDebugLingoParse, "++++ end if (no tNLELSE)");
					res += " end if";
					iflevel--;
				}
			}
		} else if (tok.equals("end")) {
			debugC(2, kDebugLingoParse, "start-end");

			tok = nexttok(lineStart);
			if (tok.equals("if")) {
				debugC(2, kDebugLingoParse, "second-if");
				iflevel--;
			}
		}
	}

	for (int i = 0; i < iflevel; i++) {
		debugC(2, kDebugLingoParse, "++++ end if (unclosed)");
		res += "\nend if";
	}


	debugC(2, kDebugLingoParse, "#############\n%s\n#############", res.c_str());

	return res;
}

void Lingo::executeScript(ScriptType type, uint16 id, uint16 function) {
	if (!_scriptContexts[type].contains(id)) {
		debugC(3, kDebugLingoExec, "Request to execute non-existant script type %d id %d", type, id);
		return;
	}
	if (function >= _scriptContexts[type][id]->functions.size()) {
		debugC(3, kDebugLingoExec, "Request to execute non-existant function %d in script type %d id %d", function, type, id);
		return;
	}

	debugC(1, kDebugLingoExec, "Executing script type: %s, id: %d, function: %d", scriptType2str(type), id, function);

	_currentScriptContext = _scriptContexts[type][id];
	_currentScript = _currentScriptContext->functions[function];
	_pc = 0;
	_returning = false;

	_localvars = new SymbolHash;

	execute(_pc);

	cleanLocalVars();
}

void Lingo::restartLingo() {
	warning("STUB: restartLingo()");

	for (int i = 0; i <= kMaxScriptType; i++) {
		for (ScriptContextHash::iterator it = _scriptContexts[i].begin(); it != _scriptContexts[i].end(); ++it) {
			for (size_t j = 0; j < it->_value->functions.size(); j++) {
				delete it->_value->functions[j];
			}
			delete it->_value;
		}

		_scriptContexts[i].clear();
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
		*s = Common::String::format("\"%s\"", u.s->c_str());
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
	for (uint16 i = 0; i < CHANNEL_COUNT; i++) {
		if (_vm->getCurrentScore()->_immediateActions.contains(frame->_sprites[i]->_scriptId)) {
			g_lingo->processEvent(kEventMouseUp, kFrameScript, frame->_sprites[i]->_scriptId);
		}
	}
}

} // End of namespace Director
