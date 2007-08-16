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

		if (v2 == 0xA || v2 == 0xD || _input->eos()) break;
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

	v194 = strdup(_tmp_comment);
	_tmp_comment[0] = '\0';

	return v194;
}

void clearTokens() {

	for (uint16 i = 0; i < 20; i++)
		_tokens[i][0] = '\0';

	return;

}

//
//	Scans 's' until one of the stop-chars in 'brk' is found, building a token.
//	If the routine encounters quotes, it will extract the contained text and
//  make a proper token. When scanning inside quotes, 'brk' is ignored and
//  only newlines are considered stop-chars.
//
//	The routine returns the unparsed portion of the input string 's'.
//
char *parseNextToken(char *s, char *tok, uint16 count, const char *brk, bool ignoreQuotes) {

	enum STATES { NORMAL, QUOTED };

	STATES state = NORMAL;

	char *t = s;

	while (count > 0) {

		switch (state) {
		case NORMAL:
			if (*s == '\0') {
				*tok = '\0';
				return s;
			}

			if (strchr(brk, *s)) {
				*tok = '\0';
				return ++s;
			}

			if (*s == '"') {
				if (ignoreQuotes) {
					*tok++ = *s++;
					count--;
				} else {
					state = QUOTED;
					s++;
				}
			} else {
				*tok++ = *s++;
				count--;
			}
			break;

		case QUOTED:
			if (*s == '\0') {
				*tok = '\0';
				return s;
			}
			if (*s == '"' || *s == '\n' || *s == '\t') {
				*tok = '\0';
				return ++s;
			}

			*tok++ = *s++;
			count--;
			break;
		}

	}

	*tok = '\0';
	warning("token was truncated from line '%s'", t);

	return tok;

}

uint16 fillTokens(char* line) {

	uint16 i = 0;
	while (strlen(line) > 0 && i < 20) {
		line = parseNextToken(line, _tokens[i], 40, " \t\n");
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
