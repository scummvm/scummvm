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

#include "glk/alan3/syntax.h"
#include "glk/alan3/word.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/compatibility.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
SyntaxEntry *stxs;      /* Syntax table pointer */


/* PRIVATE TYPES & DATA */


/*+++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*======================================================================*/
ElementEntry *elementTreeOf(SyntaxEntry *stx) {
	return (ElementEntry *) pointerTo(stx->elms);
}


/*----------------------------------------------------------------------*/
static SyntaxEntry *findSyntaxEntryForPreBeta2(int verbCode, SyntaxEntry *foundStx) {
	SyntaxEntryPreBeta2 *stx;
	for (stx = (SyntaxEntryPreBeta2 *)stxs; !isEndOfArray(stx); stx++)
		if (stx->code == verbCode) {
			foundStx = (SyntaxEntry *)stx;
			break;
		}
	return (foundStx);
}


/*----------------------------------------------------------------------*/
static SyntaxEntry *findSyntaxEntry(int verbCode) {
	SyntaxEntry *stx;
	for (stx = stxs; !isEndOfArray(stx); stx++)
		if (stx->code == verbCode) {
			return stx;
			break;
		}
	return NULL;
}


/*======================================================================*/
SyntaxEntry *findSyntaxTreeForVerb(CONTEXT, int verbCode) {
	SyntaxEntry *foundStx = NULL;
	if (isPreBeta2(header->version)) {
		foundStx = findSyntaxEntryForPreBeta2(verbCode, foundStx);
	} else {
		foundStx = findSyntaxEntry(verbCode);
	}
	if (foundStx == NULL)
		// No matching syntax
		R0CALL1(error, M_WHAT)
	return foundStx;
}


/*======================================================================*/
char *parameterNameInSyntax(int syntaxNumber, int parameterNumber) {
	Aaddr adr = addressAfterTable(header->parameterMapAddress, sizeof(ParameterMapEntry));
	Aaddr *syntaxParameterNameTable = (Aaddr *)pointerTo(memory[adr]);
	Aaddr *parameterNameTable = (Aaddr *)pointerTo(syntaxParameterNameTable[syntaxNumber - 1]);
	return stringAt(parameterNameTable[parameterNumber - 1]);
}

} // End of namespace Alan3
} // End of namespace Glk
