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

#include "common/file.h"
#include "parallaction/parallaction.h"
#include "parallaction/parser.h"
#include "parallaction/disk.h"


namespace Parallaction {

uint16 tableFillBuffers(Common::SeekableReadStream &stream);

//
//	FIXME
//	this function does the same Job as parseFillBuffers, except that
//	it gets input from a SeekableStream instead of a memory buffer
//
uint16 tableFillBuffers(Common::SeekableReadStream &stream) {

	for (uint16 i = 0; i < 20; i++)
		_tokens[i][0] = '\0';

	char buf[200];
	char *line = NULL;
	do {
		line = stream.readLine(buf, 200);
		if (line == NULL) return 0;

		line = skip_whitespace(line);
	} while (strlen(line) == 0 || line[0] == '#');

	uint16 count = 0;
	while (strlen(line) > 0 && count < 20) {
		line = parseNextToken(line, _tokens[count], 40, " \t\n");
		if (_tokens[count][0] == '"' && _tokens[count][strlen(_tokens[count]) - 1] != '"') {

			line = parseNextToken(line, _tokens[count+1], 40, "\"");
			strcat(_tokens[count], _tokens[count+1] );
			_tokens[count][0] = ' ';
			line++;

		}

		line = skip_whitespace(line);
		count++;
	}

	return count;

}

void Parallaction::initTable(const char *path, char** table) {
//	printf("initTable(%s)\n", path);

	Common::File	stream;

	if (!stream.open(path))
		errorFileNotFound(path);

	uint16 count = 0;

	tableFillBuffers(stream);

	while (scumm_stricmp(_tokens[0], "ENDTABLE")) {

		table[count] = (char*)memAlloc(strlen(_tokens[0])+1);
		strcpy(table[count], _tokens[0]);

		count++;
		tableFillBuffers(stream);
	}

	table[count] = NULL;

	stream.close();

	return;

}

void Parallaction::freeTable(char** table) {

	uint16 count = 0;

	while (table[count]) {

		memFree(table[count]);
		table[count] = NULL;

		count++;
	}

	return;

}

int16 Parallaction::searchTable(const char *s, const char **table) {

	int16 count = 0;

	if (!s) return 0;

	while (table[count]) {
		if (!scumm_stricmp(table[count], s)) return count + 1;
		count++;
	}

	return -1;
}



} // namespace Parallaction

