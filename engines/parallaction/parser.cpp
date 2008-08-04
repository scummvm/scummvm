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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "parallaction/parallaction.h"


namespace Parallaction {

#define MAX_TOKENS	50

int				_numTokens;
char			_tokens[MAX_TOKENS][MAX_TOKEN_LEN];

Script::Script(Common::ReadStream *input, bool disposeSource) : _input(input), _disposeSource(disposeSource), _line(0) {}

Script::~Script() {
	if (_disposeSource)
		delete _input;
}

char *Script::readLine(char *buf, size_t bufSize) {

	uint16 _si;
	char v2 = 0;
	for ( _si = 0; _si<bufSize; _si++) {

		v2 = _input->readSByte();

		if (v2 == 0xA || v2 == 0xD || _input->eos()) break;
		if (!_input->eos() && _si < bufSize) buf[_si] = v2;
	}

	_line++;

	if (_si == 0 && _input->eos())
		return 0;

	buf[_si] = 0xA;
	buf[_si+1] = '\0';

	return buf;

}



void Script::clearTokens() {

	for (uint16 i = 0; i < MAX_TOKENS; i++)
		_tokens[i][0] = '\0';

	_numTokens = 0;

	return;

}

void Script::skip(const char* endToken) {

	while (scumm_stricmp(_tokens[0], endToken)) {
		readLineToken(true);
	}

}

//
//	Scans 's' until one of the stop-chars in 'brk' is found, building a token.
//	If the routine encounters quotes, it will extract the contained text and
//  make a proper token. When scanning inside quotes, 'brk' is ignored and
//  only newlines are considered stop-chars.
//
//	The routine returns the unparsed portion of the input string 's'.
//
char *parseNextToken(char *s, char *tok, uint16 count, const char *brk, bool ignoreQuotes) {

	enum STATES { NORMAL, QUOTED };

	STATES state = NORMAL;

	while (count > 0) {

		switch (state) {
		case NORMAL:
			if (*s == '\0') {
				*tok = '\0';
				return s;
			}

			if (strchr(brk, *s)) {
				*tok = '\0';
				return ++s;
			}

			if (*s == '"') {
				if (ignoreQuotes) {
					*tok++ = *s++;
					count--;
				} else {
					state = QUOTED;
					s++;
				}
			} else {
				*tok++ = *s++;
				count--;
			}
			break;

		case QUOTED:
			if (*s == '\0') {
				*tok = '\0';
				return s;
			}
			if (*s == '"' || *s == '\n' || *s == '\t') {
				*tok = '\0';
				return ++s;
			}

			*tok++ = *s++;
			count--;
			break;
		}

	}

	*tok = '\0';
	// TODO: if execution flows here, make *REALLY* sure everything has been parsed
	// out of the input string. This is what is supposed to happen, but never ever
	// allocated time to properly check.

	return tok;

}

uint16 Script::fillTokens(char* line) {

	uint16 i = 0;
	while (strlen(line) > 0 && i < MAX_TOKENS) {
		line = parseNextToken(line, _tokens[i], MAX_TOKEN_LEN, " \t\n");
		line = Common::ltrim(line);
		i++;
	}

	_numTokens = i;

	return i;
}

bool isCommentLine(char *text) {
	return text[0] == '#';
}

bool isStartOfCommentBlock(char *text) {
	return (text[0] == '[');
}

bool isEndOfCommentBlock(char *text) {
	return (text[0] == ']');
}

uint16 Script::readLineToken(bool errorOnEOF) {

	clearTokens();

	bool inBlockComment = false, inLineComment;

	char buf[200];
	char *line = NULL;
	do {
		inLineComment = false;

		line = readLine(buf, 200);

		if (line == NULL) {
			if (errorOnEOF)
				error("unexpected end of file while parsing");
			else
				return 0;
		}
		line = Common::ltrim(line);

		if (isCommentLine(line)) {
			inLineComment = true;
		} else
		if (isStartOfCommentBlock(line)) {
			inBlockComment = true;
		} else
		if (isEndOfCommentBlock(line)) {
			inBlockComment = false;
		}

	} while (inLineComment || inBlockComment || strlen(line) == 0);

	return fillTokens(line);
}


void Parser::reset() {
	_currentOpcodes = 0;
	_currentStatements = 0;

	_statements.clear();
	_opcodes.clear();
}

void Parser::pushTables(OpcodeSet *opcodes, Table *statements) {
	_opcodes.push(_currentOpcodes);
	_statements.push(_currentStatements);

	_currentOpcodes = opcodes;
	_currentStatements = statements;
}

void Parser::popTables() {
	assert(_opcodes.size() > 0);

	_currentOpcodes = _opcodes.pop();
	_currentStatements = _statements.pop();
}

void Parser::parseStatement() {
	assert(_currentOpcodes != 0);

	_lookup = _currentStatements->lookup(_tokens[0]);

	debugC(9, kDebugParser, "parseStatement: %s (lookup = %i)", _tokens[0], _lookup);

	(*(*_currentOpcodes)[_lookup])();
}


#define BLOCK_BASE	100

class StatementDef {
protected:
	Common::String makeLineFromTokens() {
		Common::String space(" ");
		Common::String newLine("\n");
		Common::String text;
		for (int i = 0; i < _numTokens; i++)
			text += (Common::String(_tokens[i]) + space);
		text.deleteLastChar();
		text += newLine;
		return text;
	}

public:
	uint _score;
	const char*	_name;


	StatementDef(uint score, const char *name) : _score(score), _name(name) { }
	virtual ~StatementDef() { }

	virtual Common::String makeLine(Script &script) = 0;

};


class SimpleStatementDef : public StatementDef {

public:
	SimpleStatementDef(uint score, const char *name) : StatementDef(score, name) { }

	Common::String makeLine(Script &script) {
		return makeLineFromTokens();
	}

};



class BlockStatementDef : public StatementDef {

	const char*	_ending1;
	const char*	_ending2;

public:
	BlockStatementDef(uint score, const char *name, const char *ending1, const char *ending2 = 0) : StatementDef(score, name), _ending1(ending1),
		_ending2(ending2) { }

	Common::String makeLine(Script &script) {
		Common::String text = makeLineFromTokens();
		bool end;
		do {
			script.readLineToken(true);
			text += makeLineFromTokens();
			end = !scumm_stricmp(_ending1, _tokens[0]) || (_ending2 && !scumm_stricmp(_ending2, _tokens[0]));
		} while (!end);
		return text;
	}

};

class CommentStatementDef : public StatementDef {

	Common::String parseComment(Script &script) {
		Common::String result;
		char buf[401];

		do {
			script.readLine(buf, 400);
			buf[strlen(buf)-1] = '\0';
			if (!scumm_stricmp(buf, "endtext"))
				break;
			result += Common::String(buf) + "\n";
		} while (true);
		result += "endtext\n";
		return result;
	}

public:
	CommentStatementDef(uint score, const char *name) : StatementDef(score, name) { }

	Common::String makeLine(Script &script) {
		Common::String text = makeLineFromTokens();
		text += parseComment(script);
		return text;
	}

};




PreProcessor::PreProcessor() {
	_defs.push_back(new SimpleStatementDef(1, "disk" ));
	_defs.push_back(new SimpleStatementDef(2, "location" ));
	_defs.push_back(new SimpleStatementDef(3, "localflags" ));
	_defs.push_back(new SimpleStatementDef(4, "flags" ));
	_defs.push_back(new SimpleStatementDef(5, "zeta" ));
	_defs.push_back(new SimpleStatementDef(6, "music" ));
	_defs.push_back(new SimpleStatementDef(7, "sound" ));
	_defs.push_back(new SimpleStatementDef(8, "mask" ));
	_defs.push_back(new SimpleStatementDef(9, "path" ));
	_defs.push_back(new SimpleStatementDef(10, "character" ));
	_defs.push_back(new CommentStatementDef(11, "comment" ));
	_defs.push_back(new CommentStatementDef(12, "endcomment" ));
	_defs.push_back(new BlockStatementDef(13,  "ifchar", "endif" ));
	_defs.push_back(new BlockStatementDef(BLOCK_BASE, "zone", "endanimation", "endzone" ));
	_defs.push_back(new BlockStatementDef(BLOCK_BASE, "animation", "endanimation", "endzone" ));
	_defs.push_back(new BlockStatementDef(1000, "commands", "endcommands" ));
	_defs.push_back(new BlockStatementDef(1001, "acommands", "endcommands" ));
	_defs.push_back(new BlockStatementDef(1002, "escape", "endcommands" ));
	_defs.push_back(new SimpleStatementDef(2000, "endlocation"));
}

PreProcessor::~PreProcessor() {
	DefList::iterator it = _defs.begin();
	for (; it != _defs.end(); it++) {
		delete *it;
	}
}

StatementDef* PreProcessor::findDef(const char* name) {
	DefList::iterator it = _defs.begin();
	for (; it != _defs.end(); it++) {
		if (!scumm_stricmp((*it)->_name, name)) {
			return *it;
		}
	}
	return 0;
}



uint PreProcessor::getDefScore(StatementDef* def) {
	if (def->_score == BLOCK_BASE) {
		_numZones++;
		return (_numZones + BLOCK_BASE);
	}
	return def->_score;
}


void PreProcessor::preprocessScript(Script &script, StatementList &list) {
	_numZones = 0;
	Common::String text;
	do {
		script.readLineToken(false);
		if (_numTokens == 0)
			break;

		StatementDef *def = findDef(_tokens[0]);
		assert(def);

		text = def->makeLine(script);
		int score = getDefScore(def);
		list.push_back(StatementListNode(score, def->_name, text));
	} while (true);
	Common::sort(list.begin(), list.end());
}




void testPreprocessing(Parallaction *vm, const char *filename) {
	Script *script = vm->_disk->loadLocation(filename);
	StatementList list;
	PreProcessor pp;
	pp.preprocessScript(*script, list);
	delete script;
	Common::DumpFile dump;
	dump.open(filename);
	StatementList::iterator it = list.begin();
	for ( ; it != list.end(); it++) {
		dump.write((*it)._text.c_str(), (*it)._text.size());
	}
	dump.close();
}


} // namespace Parallaction
