/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "common/stdafx.h"

#include "parallaction/parallaction.h"


namespace Parallaction {

char			_tokens[20][40];

Script::Script(Common::SeekableReadStream *input, bool disposeSource) : _input(input), _disposeSource(disposeSource) {
}

Script::~Script() {
	if (_disposeSource)
		delete _input;
}

char *Script::readLine(char *buf, size_t bufSize) {

	uint16 _si;
	char v2 = 0;
	for ( _si = 0; _si<bufSize; _si++) {

		v2 = _input->readSByte();

		if (v2 == 0xA || _input->eos()) break;
		if (!_input->eos() && _si < bufSize) buf[_si] = v2;
	}

	if (_si == 0 && _input->eos())
		return 0;

	buf[_si] = 0xA;
	buf[_si+1] = '\0';

	return buf;

}

uint32 Script::read(void *dataPtr, uint32 dataSize) {
	error("binary read not supported on Script streams");
}

bool Script::eos() const {
	error("EoS not supported on Script streams");
}

uint32 Script::pos() const {
	error("position not supported on Script streams");
}

uint32 Script::size() const {
	error("can't get size of Script streams");
}

void Script::seek(int32 offset, int whence) {
	error("seek not supported on Script streams");
}

//
//	a comment can appear both at location and Zone levels
//	comments are displayed into rectangles on the screen
//
char *Parallaction::parseComment(Script &script) {

	char			_tmp_comment[1000] = "\0";
	char *v194;

	do {
		char v190[400];
		v194 = script.readLine(v190, 400);

		v194[strlen(v194)-1] = '\0';
		if (!scumm_stricmp(v194, "endtext"))
			break;

		strcat(_tmp_comment, v194);
		strcat(_tmp_comment, " ");
	} while (true);

	v194 = (char*)malloc(strlen(_tmp_comment)+1);
	strcpy(v194, _tmp_comment);
	_tmp_comment[0] = '\0';

	return v194;
}

void clearTokens() {

	for (uint16 i = 0; i < 20; i++)
		_tokens[i][0] = '\0';

	return;

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

} // namespace Parallaction
