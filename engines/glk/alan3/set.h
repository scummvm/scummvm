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

#ifndef GLK_ALAN3_SET
#define GLK_ALAN3_SET

/* Abstract datatype Set for Alan interpreter

    A set is implemented as a struct holding a size and a
    dynamically allocated array of members. Members can be
    integers or instance numbers. Attributes of Set type is
    allocated and the pointer to it is used as the attribute
    value. As members are only references, clearing a set can
    simply be done by setting the size to zero.
*/

#include "glk/alan3/acode.h"
#include "glk/alan3/types.h"

namespace Glk {
namespace Alan3 {

struct Set {
	int size;
	int allocated;
	Aword *members;
};

extern Set *newSet(int size);
extern void initSets(SetInitEntry *initTable);
extern int setSize(Set *theSet);
extern void clearSet(Set *theSet);
extern Set *copySet(Set *theSet);
extern Aword getSetMember(Set *theSet, Aint member);
extern bool inSet(Set *theSet, Aword member);
extern void addToSet(Set *theSet, Aword newMember);
extern void removeFromSet(Set *theSet, Aword member);
extern Set *setUnion(Set *theSet, Set *other);
extern bool equalSets(Set *theSet, Set *other);
extern void freeSet(Set *theSet);

} // End of namespace Alan3
} // End of namespace Glk

#endif
