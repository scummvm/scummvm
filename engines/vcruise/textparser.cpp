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

#include "common/path.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "vcruise/textparser.h"

namespace VCruise {

TextParserState::TextParserState() : _lineNum(1), _col(1), _prevWasCR(false), _isParsingComment(false) {
}


TextParser::TextParser(Common::ReadStream *stream) : _stream(stream), _returnedBufferPos(kReturnedBufferSize) {
	memset(_returnedBuffer, 0, kReturnedBufferSize);
}

bool TextParser::readOneChar(char &outC, TextParserState &outState) {
	if (_returnedBufferPos == kReturnedBufferSize) {
		if (_stream->eos())
			return false;
	}

	char c = 0;

	if (_returnedBufferPos != kReturnedBufferSize) {
		c = _returnedBuffer[_returnedBufferPos++];
	} else {
		if (!_stream->read(&c, 1))
			return false;
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

bool TextParser::isCompoundDelimiter(char c1, char c2) {
	if (c2 == '=' && (c1 == '=' || c1 == '<' || c1 == '>' || c1 == '!'))
		return true;

	return false;
}

bool TextParser::isWhitespace(char c) {
	return (c == ' ') || ((c & 0xe0) == 0);
}

void TextParser::requeue(const char *chars, uint numChars, const TextParserState &state) {
	_state = state;
	assert(_returnedBufferPos >= numChars);
	_returnedBufferPos -= numChars;
	memcpy(_returnedBuffer + _returnedBufferPos, chars, numChars);
}

void TextParser::requeue(const Common::String &str, const TextParserState &state) {
	requeue(str.c_str(), str.size(), state);
}

void TextParser::expectToken(Common::String &outToken, const Common::Path &blamePath) {
	TextParserState state;
	expectTokenInternal(outToken, blamePath, state);
}

void TextParser::expectShort(int16 &outInt, const Common::Path &blamePath) {
	int i;
	expectInt(i, blamePath);
	outInt = static_cast<int16>(i);
}

void TextParser::expectInt(int &outInt, const Common::Path &blamePath) {
	Common::String token;
	TextParserState state;
	expectTokenInternal(token, blamePath, state);

	if (!sscanf(token.c_str(), "%i", &outInt))
		error("Parsing error in '%s' at line %i col %i: Integer was malformed", blamePath.toString(Common::Path::kNativeSeparator).c_str(), static_cast<int>(state._lineNum), static_cast<int>(state._col));
}

void TextParser::expectUInt(uint &outUInt, const Common::Path &blamePath) {
	Common::String token;
	TextParserState state;
	expectTokenInternal(token, blamePath, state);

	if (!sscanf(token.c_str(), "%u", &outUInt))
		error("Parsing error in '%s' at line %i col %i: Unsigned integer was malformed", blamePath.toString(Common::Path::kNativeSeparator).c_str(), static_cast<int>(state._lineNum), static_cast<int>(state._col));
}

void TextParser::expectLine(Common::String &outToken, const Common::Path &blamePath, bool continueToNextLine) {
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

void TextParser::expect(const char *str, const Common::Path &blamePath) {
	Common::String token;
	TextParserState state;
	expectTokenInternal(token, blamePath, state);

	if (token != str)
		error("Parsing error in '%s' at line %i col %i: Expected token '%s' but found '%s'", blamePath.toString(Common::Path::kNativeSeparator).c_str(), static_cast<int>(state._lineNum), static_cast<int>(state._col), str, token.c_str());
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

void TextParser::expectTokenInternal(Common::String &outToken, const Common::Path &blamePath, TextParserState &outState) {
	if (!parseToken(outToken, outState))
		error("Parsing error in '%s' unexpected end of file", blamePath.toString(Common::Path::kNativeSeparator).c_str());
}

bool TextParser::parseToken(Common::String &outString, TextParserState &outState) {
	outString.clear();

	char c = 0;
	TextParserState state;

	if (!skipWhitespaceAndComments(c, state))
		return false;

	outState = state;

	outString += c;

	if (c == '\"') {
		while (readOneChar(c, state)) {
			if (c == '\n' || c == '\r') {
				requeue(&c, 1, state);
				return true;
			}

			outString += c;
			if (c == '\"')
				return true;
		}
		return true;
	}

	if (isDelimiter(c)) {
		char firstC = c;
		if (readOneChar(c, state)) {
			if (isCompoundDelimiter(firstC, c))
				outString += c;
			else
				requeue(&c, 1, state);
		}

		return true;
	}

	while (readOneChar(c, state)) {
		if (isWhitespace(c) || _state._isParsingComment) {
			requeue(&c, 1, state);
			break;
		}

		if (outString.size() == 1 && isCompoundDelimiter(outString[0], c)) {
			outString += c;
			break;
		}

		if (isDelimiter(c)) {
			requeue(&c, 1, state);
			break;
		}

		outString += c;
	}

	return true;
}

} // End of namespace VCruise
