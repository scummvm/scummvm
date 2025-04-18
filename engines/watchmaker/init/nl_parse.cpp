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

#include "watchmaker/init/nl_parse.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "watchmaker/ll/ll_system.h"

namespace Watchmaker {

#define JParse_EOF()    (_stream->eos())

int NLParser::MatchWord(const char *token) {
	const char *str = token;
	while (!JParse_EOF() && (*str) != 0) {
		if (JParse_ReadByte() != *str)
			str = token;
		else
			str++;
	}
	return (*str);
}

// PELS: supporto per il parsing dei .nl **********************************

int NLParser::ParseError(const char *ln, ...) {
	char err[J_MAXSTRLEN];
	va_list ap;
	if (ErrorFunc)
		ErrorFunc();

	va_start(ap, ln);
	vsnprintf(err, J_MAXSTRLEN, ln, ap);
	va_end(ap);
	warning("%s", err);
	return 0;
}

void NLParser::IfParseErrorDo(void (*func)()) {
	ErrorFunc = func;
}

Common::SharedPtr<NLParser> NLParser::open(const Common::String &name) {
	auto stream = openFile(name);
	if (!stream) {
		return nullptr;
	}
	return Common::SharedPtr<NLParser>(new NLParser(stream));
}

NLParser::NLParser(Common::SharedPtr<Common::SeekableReadStream> stream) : _stream(stream), nlLineCounter(0), jUsingComments(1) {

}

static int lastreadeol = 0;
int NLParser::ReadArgument_(char *str, int teol) {
	int a, notstring = 1, t;
	unsigned long c = 0;
	if (teol & lastreadeol)
		return -1;
	while (!JParse_EOF()) {
skipped_comms:
		a = JParse_ReadByte();
		if (notstring) {
			if (jUsingComments)
				if (a == '/') {
					t = JParse_ReadByte() & 0xff;
					switch (t) {
					case '/':
						if (MatchWord("\n"))
							return -1;
						if (teol) {
							lastreadeol = 1;
							return -1;
						}
						goto skipped_comms;
					case '*':
						if (MatchWord("*/"))
							return -1;
						goto skipped_comms;
					default:
						if (JParse_PrevByte())
							return -1;
					}
				}
			switch (a) {
			case '\"':
				notstring = 0;
				break;
			case '\r':
			case '\f':
			case '\n':
				if (teol && c == 0) {
					lastreadeol = 1;
					return -1;
				}

			case '\t':
			case ' ':
			case '(':
				if (c) {
					lastreadeol = 0;
					if (a == '\n')lastreadeol = 1;
					*str = 0;
					return (c + 1);
				}
				break;
			case ')':
			case ',':
			case ';':
			case '|':

				lastreadeol = 0;
				*str = 0;
				return (c + 1);
			default:
				if (c++ >= jStringLimit)
					return -1;
				*str = a;
				str++;
				break;
			}                                    //end switch
		} else
			switch (a) {
			case '\"':
				/*while(!JParse_EOF())
				switch(JParse_ReadByte()) {
				case '(':
				case ')':
				case ',':
				case ';': */

				*str = 0;
				return (c + 1);
			// }
			// break;
			case '\\':
				t = JParse_ReadByte();
				switch (t) {
				case '\\':
					a = '\\';
					break;
				case 't':
					a = '\t';
					break;
				case 'n':
					a = '\n';
					break;
				case '\"':
					a = '\"';
					break;
				default:
					a = ' ';
				}                                    // senza break....ok
			default:
				if (c++ >= jStringLimit)
					return -1;
				*str = a;
				str++;
				break;
			}
	}

	return EOF_PARSED;

}



int NLParser::ReadNumber(void) {
	char stri[J_MAXSTRLEN];
	int res;
	if (ReadArgument(stri) < 0)
		return 0;
	if (stri[1] == 'x' || stri[1] == 'X')
		sscanf(stri, "%x", &res);
	else
		sscanf(stri, "%d", &res);
	return (int)res;
}

int NLParser::SearchArgument(char *t, ...) {
	va_list args;
	char *curarg;
	int c = 0, res;
	va_start(args, t);
	if (ReadArgument(t) < 0)
		return -1;
	res = -2;
	curarg = va_arg(args, char *);
	while (curarg != NULL) {
		if (!scumm_stricmp(curarg, t))
			res = c;
		c++;
		curarg = va_arg(args, char *);
	}
	va_end(args);
	return res;
}

} // End of namespace Watchmaker
