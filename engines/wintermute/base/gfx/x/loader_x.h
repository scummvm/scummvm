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

#ifndef WINTERMUTE_X_FILE_LEXER_H
#define WINTERMUTE_X_FILE_LEXER_H

#include "common/memstream.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Wintermute {

enum TokenType {
	IDENTIFIER = 0,
	STRING,
	UUID,
	INT,
	FLOAT,
	OPEN_BRACES,
	CLOSE_BRACES,
	OPEN_PAREN,
	CLOSE_PAREN,
	OPEN_ANGLE,
	CLOSE_ANGLE,
	OPEN_BRACKET,
	CLOSE_BRACKET,
	SEMICOLON,
	COMMA,
	DOT,
	NULL_CHAR,
	UNKNOWN_TOKEN
};

struct Token {
	TokenType _type;
	Common::String _textVal;
	int _integerVal;
	float _floatVal;

	void pushChar(char c);
};

class XFileLexer {
public:
	XFileLexer(byte *buffer, uint32 fileSize, bool isText);

	void advanceToNextToken();
	void skipTerminator();
	bool eof();
	bool tokenIsIdentifier();
	bool tokenIsIdentifier(const char *val);
	void advanceOnOpenBraces();
	bool reachedClosedBraces();
	TokenType getTypeOfToken();
	bool tokenIsOfType(TokenType type);
	int tokenToInt();
	double tokenToFloat();
	Common::String tokenToString();
	uint32 tokenToUint32();

	void skipObject();

	float readFloat();
	int readInt();
	Common::String readString();
	uint32 readUint32();

private:
	void nextTokenBinary();

	Token _tok;
	Common::MemoryReadStream _buffer;
	int _lineCount;
	bool _isText;
	int _integersToRead;
	int _floatsToRead;
	bool _expectsTerminator;
};

} // namespace Wintermute

#endif
