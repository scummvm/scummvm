/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "gob/gob.h"
#include "gob/goblin.h"
#include "gob/mult.h"

namespace Gob {

Goblin_v3::Goblin_v3(GobEngine *vm) : Goblin_v2(vm) {
}

bool Goblin_v3::isMovement(int8 state) {
	if ((state >= 0) && (state < 8))
		return true;
	if ((state >= 40) && (state < 44))
		return true;
	if ((state == 26) || (state == 27))
		return true;
	if ((state == 38) || (state == 39))
		return true;

	return false;
}

void Goblin_v3::advMovement(Mult::Mult_Object *obj, int8 state) {
	switch (state) {
	case 0:
		obj->goblinX--;
		break;

	case 1:
		obj->goblinX--;
		obj->goblinY--;
		break;

	case 2:
	case 26:
	case 38:
		obj->goblinY--;
		break;

	case 3:
		obj->goblinX++;
		obj->goblinY--;
		break;

	case 4:
		obj->goblinX++;
		break;

	case 5:
		obj->goblinX++;
		obj->goblinY++;
		break;

	case 6:
	case 27:
	case 39:
		obj->goblinY++;
		break;

	case 7:
		obj->goblinX--;
		obj->goblinY++;
		break;

	case 40:
		obj->goblinX--;
		obj->goblinY -= 2;
		break;

	case 41:
		obj->goblinX--;
		obj->goblinY += 2;
		break;

	case 42:
		obj->goblinX++;
		obj->goblinY -= 2;
		break;

	case 43:
		obj->goblinX++;
		obj->goblinY += 2;
		break;
	}
}

} // End of namespace Gob
