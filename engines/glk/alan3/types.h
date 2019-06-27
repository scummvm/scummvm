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

#ifndef GLK_ALAN3_TYPES
#define GLK_ALAN3_TYPES

/* Header file for the Alan interpreter module. */

#include "glk/alan3/sysdep.h"
#include "glk/alan3/acode.h"
#include "glk/alan3/memory.h"

namespace Glk {
namespace Alan3 {

/* PREPROCESSOR */
#define FORWARD
#define NEW(type) ((type *)allocate(sizeof(type)))


/* CONSTANTS */

#define HERO (header->theHero)
#define ENTITY (header->entityClassId)
#define OBJECT (header->objectClassId)
#define LOCATION (header->locationClassId)
#define THING (header->thingClassId)
#define ACTOR (header->actorClassId)

#define MAXPARAMS (header->maxParameters)
#define MAXINSTANCE (header->instanceMax)

#define pointerTo(x) ((void *)&memory[x])
#define addressOf(x) ((((long)x)-((long)memory))/sizeof(Aword))
#define stringAt(x) ((char *)pointerTo(x))

#define ASIZE(x) (sizeof(x)/sizeof(Aword))

/* The various tables */
struct VerbEntry {  /* VERB TABLE */
	Aint code;            /* Code for the verb */
	Aaddr alts;           /* Address to alternatives */
};

struct LimitEntry { /* LIMIT Type */
	Aword atr;            /* Attribute that limits */
	Aword val;            /* And the limiting value */
	Aaddr stms;           /* Statements if fail */
};

/* Functions: */
extern Aaddr addressAfterTable(Aaddr adr, int size);

} // End of namespace Alan3
} // End of namespace Glk

#endif
