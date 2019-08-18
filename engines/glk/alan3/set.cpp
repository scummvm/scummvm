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

#include "glk/alan3/set.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/instance.h"

namespace Glk {
namespace Alan3 {

#define EXTENT 5

/*======================================================================*/
Set *newSet(int allocation) {
	Set *theSet = NEW(Set);

	if (allocation) {
		theSet->members = (Aword *)allocate(allocation * sizeof(theSet->members[0]));
		theSet->size = 0;
		theSet->allocated = allocation;
	}
	return theSet;
}


/*======================================================================*/
void initSets(SetInitEntry *initTable) {
	SetInitEntry *init;
	int i;

	for (init = initTable; !isEndOfArray(init); init++) {
		Set *set = newSet(init->size);
		Aword *member = (Aword *)pointerTo(init->setAddress);
		for (i = 0; i < init->size; i++, member++)
			addToSet(set, *member);
		setInstanceAttribute(init->instanceCode, init->attributeCode, toAptr(set));
	}
}


/*======================================================================*/
int setSize(Set *theSet) {
	return theSet->size;
}


/*======================================================================*/
void clearSet(Set *theSet) {
	theSet->size = 0;
}


/*======================================================================*/
Set *copySet(Set *theSet) {
	Set *nset = newSet(theSet->size);
	int i;

	for (i = 1; i <= theSet->size; i++)
		addToSet(nset, getSetMember(theSet, i));
	return nset;
}


/*======================================================================*/
Aword getSetMember(Set *theSet, Aint theMember) {
	if (theMember > theSet->size || theMember < 1)
		apperr("Accessing nonexisting member in a set");
	return theSet->members[theMember - 1];
}


/*======================================================================*/
bool inSet(Set *theSet, Aword member) {
	int i;

	for (i = 1; i <= theSet->size; i++)
		if (getSetMember(theSet, i) == member)
			return TRUE;
	return FALSE;
}


/*=======================================================================*/
Set *setUnion(Set *set1, Set *set2) {
	Set *theUnion = newSet(set1->size + set2->size);
	int i;

	for (i = 0; i < set1->size; i++)
		addToSet(theUnion, set1->members[i]);
	for (i = 0; i < set2->size; i++)
		addToSet(theUnion, set2->members[i]);
	return theUnion;
}


/*=======================================================================*/
void addToSet(Set *theSet, Aword newMember) {
	if (inSet(theSet, newMember)) return;
	if (theSet->size == theSet->allocated) {
		theSet->allocated += EXTENT;
		theSet->members = (Aword *)realloc(theSet->members, theSet->allocated * sizeof(theSet->members[0]));
	}
	theSet->members[theSet->size] = newMember;
	theSet->size++;
}


/*=======================================================================*/
void removeFromSet(Set *theSet, Aword member) {
	int i, j;

	if (!inSet(theSet, member)) return;

	for (i = 0; i < theSet->size; i++) {
		if ((Aword)theSet->members[i] == member) {
			for (j = i; j < theSet->size - 1; j++)
				theSet->members[j] = theSet->members[j + 1];
			theSet->size--;
			break;
		}
	}
}


/*=======================================================================*/
bool equalSets(Set *set1, Set *set2) {
	int i;

	if (set1->size != set2->size) return FALSE;

	for (i = 0; i < set1->size; i++) {
		if (!inSet(set2, set1->members[i]))
			return FALSE;
	}
	return TRUE;
}


/*======================================================================*/
void freeSet(Set *theSet) {
	if (theSet != NULL) {
		if (theSet->members != NULL)
			deallocate(theSet->members);
		deallocate(theSet);
	}
}

} // End of namespace Alan3
} // End of namespace Glk
