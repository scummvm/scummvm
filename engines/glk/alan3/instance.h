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

#ifndef GLK_ALAN3_INSTANCE
#define GLK_ALAN3_INSTANCE

#include "common/serializer.h"
#include "glk/alan3/acode.h"
#include "glk/jumps.h"
#include "glk/alan3/types.h"
#include "glk/alan3/set.h"

namespace Glk {
namespace Alan3 {

/* Types: */
struct AdminEntry { /* Administrative data about instances */
	Aint location;
	AttributeEntry *attributes;
	Abool alreadyDescribed;
	Aint visitsCount;
	Aint script;
	Aint step;
	Aint waitCount;

	/**
	 * Save/Restore data
	 */
	void synchronize(Common::Serializer &s);
};


/* Data: */
extern InstanceEntry *instances; /* Instance table pointer */

extern AdminEntry *admin;   /* Administrative data about instances */
extern AttributeEntry *attributes; /* Dynamic attribute values */


/* Functions: */
extern bool isA(int instance, int ancestor);
extern bool isAObject(int instance);
extern bool isAContainer(int instance);
extern bool isAActor(int instance);
extern bool isALocation(int instance);
extern bool isLiteral(int instance);
extern bool isANumeric(int instance);
extern bool isAString(int instance);

extern Aptr getInstanceAttribute(int instance, int attribute);
extern char *getInstanceStringAttribute(int instane, int attribute);
extern Set *getInstanceSetAttribute(int instance, int attribute);

extern void setInstanceAttribute(int instance, int atr, Aptr value);
extern void setInstanceStringAttribute(int instance, int attribute, char *string);
extern void setInstanceSetAttribute(int instance, int atr, Aptr set);

extern void say(CONTEXT, int instance);
extern void sayForm(CONTEXT, int instance, SayForm form);
extern void sayInstance(CONTEXT, int instance);

extern bool hasDescription(int instance);
extern bool isDescribable(int instance);
extern void describeAnything(CONTEXT, int instance);
extern void describeInstances(CONTEXT);
extern bool describe(CONTEXT, int instance);

extern int where(int instance, ATrans trans);
extern int positionOf(int instance);
extern int locationOf(int instance);

extern bool isAt(int instance, int other, ATrans trans);
extern bool isIn(int instance, int theContainer, ATrans trans);
extern bool isHere(int instance, ATrans trans);
extern bool isNearby(int instance, ATrans trans);
extern bool isNear(int instance, int other, ATrans trans);

extern bool isOpaque(int container);

extern void locate(CONTEXT, int instance, int whr);

} // End of namespace Alan3
} // End of namespace Glk

#endif
