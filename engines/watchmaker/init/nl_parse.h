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

#ifndef WATCHMAKER_NL_PARSE_H
#define WATCHMAKER_NL_PARSE_H

#define J_MAXSTRLEN 512L
#include "common/stream.h"

namespace Watchmaker {

class NLParser {
	unsigned int jStringLimit = J_MAXSTRLEN, jTillEOL = 0, jUsingComments = 0;
	unsigned long nlLineCounter;
	int  ReadArgument_(char *str, int teol);
	Common::SharedPtr<Common::SeekableReadStream> _stream = nullptr;
	void (*ErrorFunc)() = nullptr;
public:
	// TODO: Need a static version that allows for failure, as we have fallbacks in the code to use.
	static Common::SharedPtr<NLParser> open(const Common::String &name);
	NLParser(Common::SharedPtr<Common::SeekableReadStream> stream);
	int ParseError(const char *ln, ...);
	void IfParseErrorDo(void (*func)());
	int ReadArgumentEOL(char *str) {
		return ReadArgument_(str, 1);
	}
	int ReadArgument(char *str) {
		return ReadArgument_(str, 0);
	}
	int ReadNumber();              //max 80 cifre
	int SearchArgument(char *t, ...);
	int getCurLine() {
		return nlLineCounter;
	}

	int JParse_ReadByte(void) {
		int a = _stream->readByte();
		if (a == '\n')
			nlLineCounter++;
		return a;
	}

	int JParse_PrevByte(void) {

		if (_stream->seek(-1, SEEK_SET))
			return 0;
		return 1;
	}

	int MatchWord(const char *token);

#define EOF_PARSED    -2
};


} // End of namespace Watchmaker

#endif // WATCHMAKER_NL_PARSE_H
