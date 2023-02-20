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

#ifndef VCRUISE_TEXTPARSER_H
#define VCRUISE_TEXTPARSER_H

#include "common/str.h"
#include "common/memstream.h"

namespace Common {

class ReadStream;

} // End of namespace Common

namespace VCruise {

struct TextParserState {
	TextParserState();

	uint _lineNum;
	uint _col;
	bool _prevWasCR;
	bool _isParsingComment;
};

class TextParser {
public:
	explicit TextParser(Common::ReadStream *stream);

	bool parseToken(Common::String &outString, TextParserState &outState);

	bool readOneChar(char &outC, TextParserState &outState);
	bool skipWhitespaceAndComments(char &outC, TextParserState &outState);

	void requeue(const char *chars, uint numChars, const TextParserState &state);
	void requeue(const Common::String &str, const TextParserState &state);

	void expectToken(Common::String &outToken, const Common::String &blamePath);
	void expectShort(int16 &outInt, const Common::String &blamePath);
	void expectInt(int &outInt, const Common::String &blamePath);
	void expectUInt(uint &outUInt, const Common::String &blamePath);
	void expectLine(Common::String &outToken, const Common::String &blamePath, bool continueToNextLine);

	void expect(const char *str, const Common::String &blamePath);

	void skipToEOL();
	bool checkEOL();

private:
	void expectTokenInternal(Common::String &outToken, const Common::String &blamePath, TextParserState &outState);

	static bool isDelimiter(char c);
	static bool isWhitespace(char c);

	TextParserState _state;

	Common::ReadStream *_stream;

	static const uint kReturnedBufferSize = 8;

	char _returnedBuffer[kReturnedBufferSize];
	uint _returnedBufferPos;
};

} // End of namespace VCruise

#endif
