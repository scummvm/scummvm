/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

// WALKER.CPP by James (14nov96)

// Script functions for moving megas about the place & also for keeping tabs
// on them

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/memory.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/driver/d_draw.h"

namespace Sword2 {

/**
 * Work out direction from start to dest.
 */

// Used in whatTarget(); not valid for all megas
#define	diagonalx 36
#define	diagonaly 8

int Logic::whatTarget(int startX, int startY, int destX, int destY) {
	int deltaX = destX - startX;
	int deltaY = destY - startY;

	// 7 0 1
	// 6   2
	// 5 4 3

	// Flat route

	if (ABS(deltaY) * diagonalx < ABS(deltaX) * diagonaly / 2)
		return (deltaX > 0) ? 2 : 6;

	// Vertical route

	if (ABS(deltaY) * diagonalx / 2 > ABS(deltaX) * diagonaly)
		return (deltaY > 0) ? 4 : 0;

	// Diagonal route

	if (deltaX > 0)
		return (deltaY > 0) ? 3 : 1;

	return (deltaY > 0) ? 5 : 7;
}

} // End of namespace Sword2
