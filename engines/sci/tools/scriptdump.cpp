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

#include <console.h>
#include <script.h>
#include <vocabulary.h>
#include <old_objects.h>
#include <stdio.h>
#include <stdlib.h>
#include <engine.h>
#include "sciunpack.h"

int script_dump() {
	con_passthrough = 1;

	if (loadObjects(resmgr)) {
		fprintf(stderr, "Unable to load object hierarchy\n");
		return 1;
	}

	printObject(object_root, SCRIPT_PRINT_METHODS | SCRIPT_PRINT_CHILDREN);
	return 0;
}
