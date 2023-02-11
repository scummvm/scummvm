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

#include "common/stream.h"
#include "common/textconsole.h"

#include "vcruise/textparser.h"

namespace VCruise {

TextParserState::TextParserState() : _lineNum(1), _col(1), _prevWasCR(false), _isParsingComment(false) {
}

TextParser::TextParser(Common::ReadStream *stream) : _stream(stream), _readBufferPos(0), _readBufferEnd(0), _returnBufferPos(kReturnBufferSize) {
	memset(_readBuffer, 0, kReadBufferSize);
	memset(_returnedBuffer, 0, kReturnBufferSize);
}

bool TextParser::readOneChar(char &outC, TextParserState &outState) {
	if (_returnBufferPos == kReturnBufferSize) {
		if (_readBufferPos == _readBufferEnd) {
			if (_stream->eos())
				return false;

			_readBufferPos = 0;
			_readBufferEnd = _stream->read(_readBuffer, kReadBufferSize);
			if (_readBufferEnd == 0)
				return false;
		}
	}

	char c = 0;

	if (_returnBufferPos != kReturnBufferSize) {
		c = _returnedBuffer[_returnBufferPos++];
	} else {
		c = _readBuffer[_readBufferPos++];
	}

	TextParserState prevState = _state;

	if (c == '\r') {
		_state._lineNum++;
		_state._col = 1;
		_state._isParsingComment = false;
		_state._prevWasCR = true;
	} else if (c == '\n') {
		if (!_state._prevWasCR) {
			_state._lineNum++;
			_state._col = 1;
		}
		_state._prevWasCR = false;
	} else {
		_state._col++;
		_state._prevWasCR = false;

		if (c == ';')
			_state._isParsingComment = true;
	}


	outC = c;
	outState = prevState;

	return true;
}

bool TextParser::skipWhitespaceAndComments(char &outC, TextParserState &outState) {
	char c = 0;
	TextParserState firstCharState;

	while (readOneChar(c, firstCharState)) {
		if (isWhitespace(c))
			continue;

		if (_state._isParsingComment)
			continue;

		outC = c;
		outState = firstCharState;
		return true;
	}

	return false;
}

bool TextParser::isDelimiter(char c) {
	if (c == ',' || c == '=' || c == '[' || c == ']')
		return true;

	return false;
}

bool TextParser::isWhitespace(char c) {
	return (c == ' ') || ((c & 0xe0) == 0);
}

void TextParser::requeue(const char *chars, uint numChars, const TextParserState &state) {
	_state = state;
	assert(_returnBufferPos >= numChars);
	_returnBufferPos -= numChars;
	memcpy(_returnedBuffer + _returnBufferPos, chars, numChars);
}

void TextParser::requeue(const Common::String &str, const TextParserState &state) {
	requeue(str.c_str(), str.size(), state);
}

void TextParser::expectToken(Common::String &outToken, const Common::String &blamePath) {
	TextParserState state;
	expectTokenInternal(outToken, blamePath, state);
}

void TextParser::expectShort(int16 &outInt, const Common::String &blamePath) {
	int i;
	expectInt(i, blamePath);
	outInt = static_cast<int16>(i);
}

void TextParser::expectInt(int &outInt, const Common::String &blamePath) {
	Common::String token;
	TextParserState state;
	expectTokenInternal(token, blamePath, state);

	int result = 0;
	bool isNegative = false;
	uint startIndex = 0;
	if (token[0] == '-') {
		if (token.size() == 1)
			error("Parsing error in '%s' at line %i col %i: Signed integer was malformed", blamePath.c_str(), static_cast<int>(state._lineNum), static_cast<int>(state._col));

		isNegative = true;
		startIndex = 1;
	}

	int base = 10;
	bool isHex = (token.size() >= (startIndex) + 3 && token[startIndex] == '0' && token[startIndex + 1] == 'x');
	if (isHex) {
		startIndex += 2;
		base = 16;
	}

	for (uint i = startIndex; i < token.size(); i++) {
		char c = token[i];
		int digit = 0;
		if (c >= '0' && c <= '9')
			digit = (c - '0');
		else if (isHex && (c >= 'a' && c <= 'f'))
			digit = (c - 'a') + 0xa;
		else if (isHex && (c >= 'A' && c <= 'F'))
			digit = (c - 'A') + 0xa;
		else
			error("Parsing error in '%s' at line %i col %i: Integer contained non-digits", blamePath.c_str(), static_cast<int>(state._lineNum), static_cast<int>(state._col));

		if (isNegative)
			digit = -digit;

		result = result * base + digit;
	}

	outInt = result;
}

void TextParser::expectUInt(uint &outUInt, const Common::String &blamePath) {
	Common::String token;
	TextParserState state;
	expectTokenInternal(token, blamePath, state);

	uint result = 0;

	for (uint i = 0; i < token.size(); i++) {
		char c = token[i];
		if (c < '0' || c > '9')
			error("Parsing error in '%s' at line %i col %i: Integer contained non-digits", blamePath.c_str(), static_cast<int>(state._lineNum), static_cast<int>(state._col));

		uint additional = c - '0';

		result = result * 10 + additional;
	}

	outUInt = result;
}

void TextParser::expectLine(Common::String &outToken, const Common::String &blamePath, bool continueToNextLine) {
	outToken.clear();

	char c = 0;
	TextParserState state;

	bool isSkippingWhitespace = true;
	uint nonWhitespaceLength = 0;

	while (readOneChar(c, state)) {
		if (c == '\r' || c == '\n' || _state._isParsingComment) {
			requeue(&c, 1, state);
			if (continueToNextLine)
				skipToEOL();
			break;
		}

		bool cIsWhitespace = isWhitespace(c);
		if (isSkippingWhitespace) {
			if (cIsWhitespace)
				continue;
			isSkippingWhitespace = false;
		}

		outToken += c;
		if (!cIsWhitespace)
			nonWhitespaceLength = outToken.size();
	}

	if (nonWhitespaceLength != outToken.size())
		outToken = outToken.substr(0, nonWhitespaceLength);
}

void TextParser::expect(const char *str, const Common::String &blamePath) {
	Common::String token;
	TextParserState state;
	expectTokenInternal(token, blamePath, state);

	if (token != str)
		error("Parsing error in '%s' at line %i col %i: Expected token '%s' but found '%s'", blamePath.c_str(), static_cast<int>(state._lineNum), static_cast<int>(state._col), str, token.c_str());
}

void TextParser::skipToEOL() {
	char c = 0;
	TextParserState state;

	while (readOneChar(c, state)) {
		if (c == '\n')
			return;

		if (c == '\r') {
			if (readOneChar(c, state)) {
				if (c != '\n')
					requeue(&c, 1, state);
				return;
			}
		}
	}
}

bool TextParser::checkEOL() {
	char c = 0;
	TextParserState state;

	for (;;) {
		if (!readOneChar(c, state))
			return true;

		if (_state._isParsingComment || c == '\n' || c == '\r') {
			// EOL or comment
			requeue(&c, 1, state);
			return true;
		}

		if (!isWhitespace(c)) {
			// Non-whitespace
			requeue(&c, 1, state);
			return false;
		}
	}
}

void TextParser::expectTokenInternal(Common::String &outToken, const Common::String &blamePath, TextParserState &outState) {
	if (!parseToken(outToken, outState))
		error("Parsing error in '%s' unexpected end of file", blamePath.c_str());
}

bool TextParser::parseToken(Common::String &outString, TextParserState &outState) {
	outString.clear();

	char c = 0;
	TextParserState state;

	if (!skipWhitespaceAndComments(c, state))
		return false;

	outState = state;

	outString += c;

	if (isDelimiter(c))
		return true;

	while (readOneChar(c, state)) {
		if (isWhitespace(c) || _state._isParsingComment) {
			requeue(&c, 1, state);
			return true;
		}

		if (isDelimiter(c)) {
			requeue(&c, 1, state);
			return true;
		}

		outString += c;
	}

	return true;
}

} // End of namespace VCruise
