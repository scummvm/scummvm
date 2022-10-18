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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/util.h"
#include "glk/scott/layout_text.h"

namespace Glk {
namespace Scott {

int findBreak(const char *buf, int pos, int columns) {
	int diff = 0;

	while (diff < columns && !Common::isSpace((unsigned char)buf[pos])) {
		pos--;
		diff++;
	}

	if (diff >= columns || diff < 1) /* Found no space */ {
		return -1;
	}

	return diff;
}

/* Breaks a null-terminated string up by inserting newlines, moving words
 down to the next line when reaching the end of the line */
char *lineBreakText(char *source, int columns, int *rows, int *length) {
	columns -= 1;

	char *result = nullptr;
	char buf[768];
	int col = 0;
	int row = 0;
	int sourcepos = 0;
	int destpos = 0;
	int diff = 0;
	*rows = 0;
	while (source[sourcepos] != '\0') {
		while (col < columns && source[sourcepos] != '\0') {
			if (source[sourcepos] == 10 || source[sourcepos] == 13) {
				/* Found a line break. */
				/* Any spaces before a line break may cause trouble, */
				/* so we delete them */
				while (destpos && buf[destpos - 1] == ' ') {
					destpos--;
				}
				col = 0;
				row++;
			} else {
				col++;
			}

			buf[destpos++] = source[sourcepos++];

			if (source[sourcepos] == 10 || source[sourcepos] == 13)
				col--;
		}

		/* We have reached the end of a line */
		row++;
		col = 0;

		if (source[sourcepos] == '\0') {
			break;
		}

		diff = findBreak(source, sourcepos, columns);
		if (diff > -1) { /* We found a suitable break */
			sourcepos = sourcepos - diff;
			destpos = destpos - diff;
			buf[destpos++] = '\n';

			if (Common::isSpace((unsigned char)source[sourcepos])) {
				sourcepos++;
			}
		}
	}
	*rows = row;
	*length = 0;
	result = new char[destpos + 1];
	if (result == nullptr)
		return nullptr;
	memcpy(result, buf, destpos);
	result[destpos] = '\0';
	*length = destpos;
	return result;
}

} // End of namespace Scott
} // End of namespace Glk
