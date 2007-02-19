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
#include "parallaction/parallaction.h"
#include "parallaction/disk.h"

namespace Parallaction {

char			_tokens[20][40];
/*
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
*/
LocScript::LocScript(const char* s) : _src(s) {

}

char *LocScript::readLine(char *buf, size_t bufSize) {

	uint16 _si;
	char v2 = 0;
	for ( _si = 0; _si<bufSize; _si++) {

		v2 = *_src++;
		if (v2 == 0xA || v2 == -1) break;
		if (v2 != -1 && _si < bufSize) buf[_si] = v2;
	}

	if (_si == 0 && v2 == -1)
		return 0;

	buf[_si] = 0xA;
	buf[_si+1] = '\0';

	return buf;

}

uint32 LocScript::read(void *dataPtr, uint32 dataSize) {
	error("binary read not supported on LocScript streams");
}

bool LocScript::eos() const {
	error("EoS not supported on LocScript streams");
}

uint32 LocScript::pos() const {
	error("position not supported on LocScript streams");
}

uint32 LocScript::size() const {
	error("can't get size of LocScript streams");
}

void LocScript::seek(int32 offset, int whence) {
	error("seek not supported on LocScript streams");
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

//
//	a comment can appear both at location and Zone levels
//	comments are displayed into rectangles on the screen
//
char *Parallaction::parseComment(ArchivedFile *file) {

	char			_tmp_comment[1000] = "\0";
	char *v194;

	do {
		char v190[400];
		v194 = _locationScript->readLine(v190, 400);

		v194[strlen(v194)-1] = '\0';
		if (!scumm_stricmp(v194, "endtext"))
			break;

		strcat(_tmp_comment, v194);
		strcat(_tmp_comment, " ");
	} while (true);

	v194 = (char*)memAlloc(strlen(_tmp_comment)+1);
	strcpy(v194, _tmp_comment);
	_tmp_comment[0] = '\0';

	return v194;
}

void clearTokens() {

	for (uint16 i = 0; i < 20; i++)
		_tokens[i][0] = '\0';

	return;

}

uint16 fillTokens(char* line) {

	uint16 i = 0;
	while (strlen(line) > 0 && i < 20) {
		line = parseNextToken(line, _tokens[i], 40, " \t\n");
		if (_tokens[i][0] == '"' && _tokens[i][strlen(_tokens[i]) - 1] != '"') {

			line = parseNextToken(line, _tokens[i+1], 40, "\"");
			strcat(_tokens[i], _tokens[i+1]);
			_tokens[i][0] = ' ';
			line++;

		}

		line = Common::ltrim(line);
		i++;
	}

	return i;
}

uint16 fillBuffers(Common::SeekableReadStream &stream, bool errorOnEOF) {

	clearTokens();

	char buf[200];
	char *line = NULL;
	do {
		line = stream.readLine(buf, 200);
		if (line == NULL) {
			if (errorOnEOF)
				error("unexpected end of file while parsing");
			else
				return 0;
		}
		line = Common::ltrim(line);
	} while (strlen(line) == 0 || line[0] == '#');

	return fillTokens(line);
}
/*
//
//	FIXME
//	this function does the same Job as fillBuffers, except that
//	it gets input from a SeekableStream instead of a memory buffer
//
uint16 tableFillBuffers(Common::SeekableReadStream &stream) {

	clearTokens();

	char buf[200];
	char *line = NULL;
	do {
		line = stream.readLine(buf, 200);
		if (line == NULL) {
			return 0;
		}
		line = Common::ltrim(line);
	} while (strlen(line) == 0 || line[0] == '#');

	return fillTokens(line);
}
*/

//	FIXME
//	this function does the same Job as fillBuffers, except that
//	it gets input from an ArchivedFile instead of a memory buffer
//
uint16 scriptFillBuffers(ArchivedFile *file) {

	clearTokens();

	char buf[200];
	char *line = NULL;
	do {
		line = readArchivedFileText(buf, 200, file);
		if (line == NULL) {
			return 0;
		}
		line = Common::ltrim(line);
	} while (strlen(line) == 0 || line[0] == '#');

	return fillTokens(line);
}

} // namespace Parallaction
