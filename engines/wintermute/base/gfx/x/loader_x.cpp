/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "engines/wintermute/base/gfx/x/loader_x.h"

namespace Wintermute {

void nextTokenText(Common::MemoryReadStream &buffer, int &lineCount, Token &tok) {
	char current = buffer.readSByte();

	while (Common::isSpace(current)) {
		if (current == '\n') {
			++lineCount;
		}

		current = buffer.readSByte();
	}

	if (Common::isAlpha(current) || current == '_') {
		tok.pushChar(current);
		current = buffer.readSByte();

		while (Common::isAlnum(current) || current == '_') {
			tok.pushChar(current);
			current = buffer.readSByte();
		}

		buffer.seek(-1, SEEK_CUR);
		tok._type = IDENTIFIER;
		return;
	} else if (Common::isDigit(current) || current == '-') {
		tok.pushChar(current);
		current = buffer.readSByte();

		while (Common::isDigit(current)) {
			tok.pushChar(current);
			current = buffer.readSByte();
		}

		if (current == '.') {
			tok.pushChar(current);
			current = buffer.readSByte();

			while (Common::isDigit(current)) {
				tok.pushChar(current);
				current = buffer.readSByte();
			}

			buffer.seek(-1, SEEK_CUR);
			tok._type = FLOAT;
			return;
		}

		buffer.seek(-1, SEEK_CUR);
		tok._type = INT;
		return;
	} else if (current == '<') {
		// a uuid consists of 36 characters, 32 alphanumeric characters
		// and four "-". We add space for a null character at the
		// end of the tempoary buffer
		const int uuidSize = 37;
		char uuid[uuidSize];
		buffer.read(uuid, 36);
		uuid[uuidSize - 1] = 0;

		current = buffer.readSByte();

		if (current != '>') {
			warning("Wrong UUID format at line %i", lineCount);
		} else {
			tok._textVal = uuid;
			tok._type = UUID;
			return;
		}
	} else if (current == '"') {
		current = buffer.readSByte();

		while (current != '"') {
			tok.pushChar(current);
			current = buffer.readSByte();
		}

		tok._type = STRING;
		return;
	}

	switch (current) {
	case '(':
		tok._type = OPEN_PAREN;
		break;
	case ')':
		tok._type = CLOSE_PAREN;
		break;
	case '{':
		tok._type = OPEN_BRACES;
		break;
	case '}':
		tok._type = CLOSE_BRACES;
		break;
	case ']':
		tok._type = OPEN_BRACKET;
		break;
	case '[':
		tok._type = CLOSE_BRACKET;
		break;
	case ',':
		tok._type = COMMA;
		break;
	case ';':
		tok._type = SEMICOLON;
		break;
	case '.':
		tok._type = DOT;
		break;
	case '\0':
		tok._type = NULL_CHAR;
		break;
	default:
		tok._type = UNKNOWN_TOKEN;
		warning("Unknown token %c at line %i", current, lineCount);
	}
}

void nextTokenBinary(Common::MemoryReadStream &buffer, Token &tok) {
	warning("nextTokenBinary in x_file_lexer.cpp not implemented yet");
}

XFileLexer::XFileLexer(byte *buffer, uint32 fileSize, bool isText)
    : _buffer(buffer, fileSize), _lineCount(1), _isText(isText) {
}

void XFileLexer::advanceToNextToken() {
	_tok._textVal.clear();

	if (_isText) {
		nextTokenText(_buffer, _lineCount, _tok);
	} else {
		nextTokenBinary(_buffer, _tok);
	}
}

bool XFileLexer::eof() {
	return _buffer.eos();
}

bool XFileLexer::tokenIsIdentifier() {
	return _tok._type == IDENTIFIER;
}

bool XFileLexer::tokenIsIdentifier(const char *val) {
	return _tok._type == IDENTIFIER && _tok._textVal == val;
}

void XFileLexer::advanceOnOpenBraces() {
	if (_tok._type == OPEN_BRACES) {
		advanceToNextToken();
	}
}

bool XFileLexer::reachedClosedBraces() {
	return _tok._type == CLOSE_BRACES;
}

TokenType XFileLexer::getTypeOfToken() {
	return _tok._type;
}

bool XFileLexer::tokenIsOfType(TokenType type) {
	return _tok._type == type;
}

int XFileLexer::tokenToInt() {
	return atoi(_tok._textVal.c_str());
}

double XFileLexer::tokenToFloat() {
	return atof(_tok._textVal.c_str());
}

Common::String XFileLexer::tokenToString() {
	return _tok._textVal;
}

void Token::pushChar(char c) {
	_textVal.insertChar(c, _textVal.size());
}

float readFloat(XFileLexer &lexer) {
	float tmp = lexer.tokenToFloat();
	lexer.advanceToNextToken();
	lexer.advanceToNextToken(); // skip comma or semicolon
	return tmp;
}

int readInt(XFileLexer &lexer) {
	int tmp = lexer.tokenToInt();
	lexer.advanceToNextToken();
	lexer.advanceToNextToken(); // skip comma or semicolon
	return tmp;
}

Common::String readString(XFileLexer &lexer) {
	Common::String tmp = lexer.tokenToString();
	lexer.advanceToNextToken();
	lexer.advanceToNextToken(); // skip comma or semicolon
	return tmp;
}

} // namespace Wintermute
