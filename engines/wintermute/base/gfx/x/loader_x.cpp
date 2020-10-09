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

	while (true) {
		if (Common::isSpace(current)) {
			if (current == '\n') {
				++lineCount;
			}

			current = buffer.readSByte();
		} else if (current == '/' || current == '#') {
			// single slashes do not seem to be used in .X files,
			// so checking for one should be enough

			while (current != '\n') {
				current = buffer.readSByte();
			}

			current = buffer.readSByte();
			++lineCount;
		} else {
			break;
		}
	}

	if (Common::isAlpha(current) || current == '_') {
		tok.pushChar(current);
		current = buffer.readSByte();

		while (Common::isAlnum(current) || current == '_' || current == '-') {
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

// based on MSDN .X file format documentation
const uint16 XBIN_TOKEN_NAME         = 1;
const uint16 XBIN_TOKEN_STRING       = 2;
const uint16 XBIN_TOKEN_INTEGER      = 3;
const uint16 XBIN_TOKEN_GUID         = 5;
const uint16 XBIN_TOKEN_INTEGER_LIST = 6;
const uint16 XBIN_TOKEN_FLOAT_LIST   = 7;

const uint16 XBIN_TOKEN_OBRACE    = 10;
const uint16 XBIN_TOKEN_CBRACE    = 11;
const uint16 XBIN_TOKEN_OPAREN    = 12;
const uint16 XBIN_TOKEN_CPAREN    = 13;
const uint16 XBIN_TOKEN_OBRACKET  = 14;
const uint16 XBIN_TOKEN_CBRACKET  = 15;
const uint16 XBIN_TOKEN_OANGLE    = 16;
const uint16 XBIN_TOKEN_CANGLE    = 17;
const uint16 XBIN_TOKEN_DOT       = 18;
const uint16 XBIN_TOKEN_COMMA     = 19;
const uint16 XBIN_TOKEN_SEMICOLON = 20;
const uint16 XBIN_TOKEN_TEMPLATE  = 31;
const uint16 XBIN_TOKEN_WORD      = 40;
const uint16 XBIN_TOKEN_DWORD     = 41;
const uint16 XBIN_TOKEN_FLOAT     = 42;
const uint16 XBIN_TOKEN_DOUBLE    = 43;
const uint16 XBIN_TOKEN_CHAR      = 44;
const uint16 XBIN_TOKEN_UCHAR     = 45;
const uint16 XBIN_TOKEN_SWORD     = 46;
const uint16 XBIN_TOKEN_SDWORD    = 47;
const uint16 XBIN_TOKEN_VOID      = 48;
const uint16 XBIN_TOKEN_LPSTR     = 49;
const uint16 XBIN_TOKEN_UNICODE   = 50;
const uint16 XBIN_TOKEN_CSTRING   = 51;
const uint16 XBIN_TOKEN_ARRAY     = 52;

void XFileLexer::nextTokenBinary() {
	uint16 current = _buffer.readUint16LE();
	uint32 length = -1;

	switch (current) {
	case XBIN_TOKEN_NAME:
		length = _buffer.readUint32LE();

		for (uint32 i = 0; i < length; ++i) {
			_tok.pushChar(_buffer.readByte());
		}

		_tok._type = IDENTIFIER;
		break;
	case XBIN_TOKEN_STRING:
		length = _buffer.readUint32LE();

		for (uint32 i = 0; i < length; ++i) {
			_tok.pushChar(_buffer.readByte());
		}

		_tok._type = STRING;
		break;
	case XBIN_TOKEN_INTEGER:
		_tok._integerVal = _buffer.readUint32LE();
		_tok._type = INT;
		break;
	case XBIN_TOKEN_GUID:
		// ignore the UUID value
		_buffer.readUint32LE();
		_buffer.readUint16LE();
		_buffer.readUint16LE();

		for (int i = 0; i < 8; ++i) {
			_buffer.readByte();
		}

		_tok._type = UUID;
		break;
	case XBIN_TOKEN_INTEGER_LIST:
		_integersToRead = _buffer.readUint32LE();
		_tok._type = INT;
		_expectsTerminator = false;
		break;
	case XBIN_TOKEN_FLOAT_LIST:
		_floatsToRead = _buffer.readUint32LE();
		_tok._type = FLOAT;
		_expectsTerminator = false;
		break;
	case XBIN_TOKEN_OBRACE:
		_tok._type = OPEN_BRACES;
		break;
	case XBIN_TOKEN_CBRACE:
		_tok._type = CLOSE_BRACES;
		break;
	case XBIN_TOKEN_OPAREN:
		_tok._type = OPEN_PAREN;
		break;
	case XBIN_TOKEN_CPAREN:
		_tok._type = CLOSE_PAREN;
		break;
	case XBIN_TOKEN_OBRACKET:
		_tok._type = OPEN_BRACKET;
		break;
	case XBIN_TOKEN_CBRACKET:
		_tok._type = CLOSE_BRACKET;
		break;
	case XBIN_TOKEN_OANGLE:
		_tok._type = OPEN_ANGLE;
		break;
	case XBIN_TOKEN_CANGLE:
		_tok._type = CLOSE_ANGLE;
		break;
	case XBIN_TOKEN_DOT:
		_tok._type = DOT;
		break;
	case XBIN_TOKEN_COMMA:
		_tok._type = COMMA;
		break;
	case XBIN_TOKEN_SEMICOLON:
		_tok._type = SEMICOLON;
		break;
	case XBIN_TOKEN_TEMPLATE:
		_tok._textVal = "template";
		_tok._type = IDENTIFIER;
		break;
	case XBIN_TOKEN_WORD:
		break;
	case XBIN_TOKEN_DWORD:
		break;
	case XBIN_TOKEN_FLOAT:
		break;
	case XBIN_TOKEN_DOUBLE:
		break;
	case XBIN_TOKEN_CHAR:
		break;
	case XBIN_TOKEN_UCHAR:
		break;
	case XBIN_TOKEN_SWORD:
		break;
	case XBIN_TOKEN_SDWORD:
		break;
	case XBIN_TOKEN_VOID:
		break;
	case XBIN_TOKEN_LPSTR:
		break;
	case XBIN_TOKEN_UNICODE:
		break;
	case XBIN_TOKEN_CSTRING:
		break;
	case XBIN_TOKEN_ARRAY:
		break;
	case 0:
		_tok._type = NULL_CHAR;
		break;
	default:
		_tok._type = UNKNOWN_TOKEN;
		warning("XFileLexer::nextBinaryToken: Unknown token encountered");
	}
}

XFileLexer::XFileLexer(byte *buffer, uint32 fileSize, bool isText)
	: _buffer(buffer, fileSize), _lineCount(1), _isText(isText), _integersToRead(0), _floatsToRead(0), _expectsTerminator(true) {
}

void XFileLexer::advanceToNextToken() {
	_tok._textVal.clear();

	if (_isText) {
		nextTokenText(_buffer, _lineCount, _tok);
	} else {
		nextTokenBinary();
	}
}

void XFileLexer::skipTerminator() {
	if (_expectsTerminator) {
		advanceToNextToken();
	}

	_expectsTerminator = (_floatsToRead == 0) && (_integersToRead == 0);
}

bool XFileLexer::eof() {
	return _buffer.pos() == _buffer.size();
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

uint32 XFileLexer::tokenToUint32() {
	// All integer values in a .X file are unsigned and at most 32 bit
	// so parsing to unsigned long and then converting down should be fine
	return strtoul(_tok._textVal.c_str(), nullptr, 10);
}

void XFileLexer::skipObject() {
	advanceToNextToken(); // optional name
	advanceToNextToken();
	advanceOnOpenBraces();

	// we have one open braces right now, once the counter reaches zero, we're done
	int closedBracesCount = 1;

	while (closedBracesCount > 0) {
		while (_integersToRead > 0 || _floatsToRead > 0) {
			if (_integersToRead > 0) {
				readInt();
			}

			if (_floatsToRead > 0) {
				readFloat();
			}
		}

		if (_tok._type == OPEN_BRACES) {
			++closedBracesCount;
		}

		if (_tok._type == CLOSE_BRACES) {
			--closedBracesCount;
		}

		advanceToNextToken();
	}
}

void Token::pushChar(char c) {
	_textVal.insertChar(c, _textVal.size());
}

float XFileLexer::readFloat() {
	if (_floatsToRead > 0) {
		--_floatsToRead;
		float tmp = _buffer.readFloatLE();

		if (_floatsToRead == 0) {
			advanceToNextToken();
		}

		return tmp;
	}

	float tmp = tokenToFloat();
	advanceToNextToken();
	advanceToNextToken(); // skip comma or semicolon
	return tmp;
}

int XFileLexer::readInt() {
	if (_integersToRead > 0) {
		--_integersToRead;
		int tmp = _buffer.readUint32LE();

		if (_integersToRead == 0) {
			advanceToNextToken();
		}

		return tmp;
	}

	int tmp = tokenToInt();
	advanceToNextToken();
	advanceToNextToken(); // skip comma or semicolon
	return tmp;
}

Common::String XFileLexer::readString() {
	Common::String tmp = tokenToString();
	advanceToNextToken();
	advanceToNextToken(); // skip comma or semicolon
	return tmp;
}

uint32 XFileLexer::readUint32() {
	if (_integersToRead > 0) {
		--_integersToRead;
		uint32 tmp = _buffer.readUint32LE();

		if (_integersToRead == 0) {
			advanceToNextToken();
		}

		return tmp;
	}

	uint32 tmp = tokenToUint32();
	advanceToNextToken();
	advanceToNextToken(); // skip comma or semicolon
	return tmp;
}

} // namespace Wintermute
