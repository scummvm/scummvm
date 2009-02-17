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

#ifndef OLD_OBJECTS_H
#define OLD_OBJECTS_H

#include "common/scummsys.h"
#include "engine/sci/include/sciresource.h"
#include "engine/sci/include/util.h"

typedef FLEXARRAY(script_opcode, int number;) script_method;

typedef struct object_ {
	/*These are based on cached selector values, and set to the values
	 *the selectors had at load time. If the selectors are changed in
	 *instances, inconsistency will follow*/
	struct object_* parent;
	const char* name;

	FLEXARRAY_NOEXTRA(struct object_*) children;

	/*No flexarray, size the size is known from the start*/
	script_method** methods;
	int method_count;

	int selector_count;
	int* selector_numbers;
} object;

typedef struct {
	int id;
	object* classID;
	byte* heap;
	int offset;
} instance;

extern object **object_map, *object_root;
extern int max_object;

#define SCRIPT_PRINT_METHODS	1
#define SCRIPT_PRINT_CHILDREN	2
#define SCRIPT_PRINT_SELECTORS  3
void printObject(object* obj, int flags);

int loadObjects(resource_mgr_t *resmgr);
void freeObject(object*);

extern const char* globals[];

#endif
