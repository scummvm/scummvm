/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "parallaction/defs.h"
#include "parallaction/parser.h"


namespace Parallaction {

char			_tokens[20][40];
static char    *_src = NULL;



void parseInit(char *s) {
	_src = s;
}

char *parseNextLine(char *s, uint16 count) {

	uint16 _si;
	char v2 = 0;
	for ( _si = 0; _si<count; _si++) {

		v2 = *_src++;
		if (v2 == 0xA || v2 == -1) break;
		if (v2 != -1 && _si < count) s[_si] = v2;
	}

	if (_si == 0 && v2 == -1)
		return 0;

	s[_si] = 0xA;
	s[_si+1] = '\0';

	return s;
}

//
//	a comment can appear both at location and Zone levels
//	comments are displayed into rectangles on the screen
//
char *parseComment(ArchivedFile *file) {

	char			_tmp_comment[1000] = "\0";
	char *v194;

	do {
		char v190[400];
		v194 = parseNextLine(v190, 400);

		v194[strlen(v194)-1] = '\0';
		if (!scumm_stricmp(v194, "endtext")) break;

		strcat(_tmp_comment, v194);
		strcat(_tmp_comment, " ");
	} while (true);

	v194 = (char*)memAlloc(strlen(_tmp_comment)+1);
	strcpy(v194, _tmp_comment);
	_tmp_comment[0] = '\0';

	return v194;
}

uint16 parseFillBuffers() {

	char tmp[200];

	uint16 _si;
	for (_si = 0; _si < 20; _si++) _tokens[_si][0] = '\0';

	char *v4;
	do {
		v4 = parseNextLine(tmp, 200);
		if (v4 == NULL) {
			puts("non ho letto ");
			exit(0);
		}
		v4 = Common::ltrim(v4);
	} while (strlen(v4) == 0 || v4[0] == '#');

	_si = 0;
	while (_si < 20 && strlen(v4) > 0) {
		v4 = parseNextToken(v4, _tokens[_si], 40, " \t\n");
		if (_tokens[_si][0] == '"' && _tokens[_si][strlen(_tokens[_si]) - 1] != '"') {

			v4 = parseNextToken(v4, _tokens[_si+1], 40, "\"\"\"");
			strcat(_tokens[_si], _tokens[_si+1]);
			_tokens[_si][0] = ' ';
			v4++;

		}

		v4 = Common::ltrim(v4);
		_si++;
	}

	return _si;
}

//	looks for next token in a string
//
//	scans 's' until one of the stop-chars in 'brk' is found
//	builds a token and return the part of the string which hasn't been parsed

char *parseNextToken(char *s, char *tok, uint16 count, const char *brk) {

	while (*s != '\0') {

		if (brk[0] == *s) break;
		if (brk[1] == *s) break;
		if (brk[2] == *s) break;

		*tok++ = *s++;
	}

	*tok = '\0';
	return s;
}


} // namespace Parallaction
