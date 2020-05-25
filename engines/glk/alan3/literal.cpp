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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/alan3/literal.h"
#include "glk/alan3/types.h"
#include "glk/alan3/memory.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
int litCount = 0;
static LiteralEntry literalTable[100];
LiteralEntry *literals = literalTable;


/* PRIVATE TYPES & DATA */


/*+++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*======================================================================*/
int instanceFromLiteral(int literalIndex) {
	return literalIndex + header->instanceMax;
}

/*----------------------------------------------------------------------*/
void createIntegerLiteral(int integerValue) {
	litCount++;
	literals[litCount]._class = header->integerClassId;
	literals[litCount].type = NUMERIC_LITERAL;
	literals[litCount].value = integerValue;
}

/*----------------------------------------------------------------------*/
void createStringLiteral(char *unquotedString) {
	litCount++;
	literals[litCount]._class = header->stringClassId;
	literals[litCount].type = STRING_LITERAL;
	literals[litCount].value = toAptr(scumm_strdup(unquotedString));
}

/*----------------------------------------------------------------------*/
void freeLiterals() {
	int i;

	for (i = 0; i <= litCount; i++)
		if (literals[i].type == STRING_LITERAL && literals[i].value != 0) {
			deallocate((void *)fromAptr(literals[i].value));
		}
	litCount = 0;
}



/*======================================================================*/
int literalFromInstance(int instance) {
	return instance - header->instanceMax;
}

} // End of namespace Alan3
} // End of namespace Glk
