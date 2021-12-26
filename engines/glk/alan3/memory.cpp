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

#include "glk/alan3/memory.h"
#include "glk/alan3/types.h"
#include "glk/alan3/syserr.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */

Aword *memory = nullptr;
static ACodeHeader dummyHeader; /* Dummy to use until memory allocated */
ACodeHeader *header = &dummyHeader;
int memTop = 0;         /* Top of load memory */


/*======================================================================*/
void *allocate(unsigned long lengthInBytes) {
	void *p = (void *)calloc((size_t)lengthInBytes, 1);

	if (p == nullptr)
		syserr("Out of memory.");

	return p;
}


/*======================================================================*/
void deallocate(void *ptr) {
	free(ptr);
}


/*======================================================================*/
void *duplicate(void *original, unsigned long len) {
	void *p = allocate(len + 1);

	memcpy(p, original, len);
	return p;
}


typedef struct {
	Aptr aptr;
	void *voidp;
} PointerMapEntry;

static PointerMapEntry *pointerMap = nullptr;
static int pointerMapSize = 0;
static int nextAptr = 1;

/*======================================================================*/
void resetPointerMap(void) {
	if (pointerMap != nullptr) free(pointerMap);
	pointerMap = nullptr;
	pointerMapSize = 0;
}

/*======================================================================*/
void *fromAptr(Aptr aptr) {
	int index;

	for (index = 0; index < pointerMapSize && pointerMap[index].aptr != aptr; index++)
		;

	if (index == pointerMapSize)
		syserr("No pointerMap entry for Aptr");

	return pointerMap[index].voidp;
}


/*======================================================================*/
Aptr toAptr(void *ptr) {
	int index;

	if (pointerMap == nullptr) {
		pointerMap = (PointerMapEntry *)allocate(sizeof(PointerMapEntry));
		pointerMapSize = 1;
	}

	for (index = 0; index < pointerMapSize && pointerMap[index].voidp != nullptr; index++)
		;
	if (index == pointerMapSize) {
		pointerMap = (PointerMapEntry *)realloc(pointerMap, (index + 1) * sizeof(PointerMapEntry));
		pointerMapSize++;
	}

	pointerMap[index].voidp = ptr;
	pointerMap[index].aptr = nextAptr++;
	return pointerMap[index].aptr;
}

} // End of namespace Alan3
} // End of namespace Glk
