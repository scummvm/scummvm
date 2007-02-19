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

