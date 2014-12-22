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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/stddefines.h"
#include "lab/labfun.h"

namespace Lab {

const uint32 LargeSetSIZE = sizeof(LargeSetRecord) - 2;



/*****************************************************************************/
/* Creates a large set.                                                      */
/*****************************************************************************/
bool createSet(LargeSet *set, uint16 last) {
	last = (((last + 15) >> 4) << 4);

	if ((*set = (LargeSet)calloc((last >> 3) + LargeSetSIZE, 1))) {
		(*set)->lastElement  = last;
		return true;
	} else /* Not Enough Memory! */
		return false;

}




/*****************************************************************************/
/* Deletes a large set.                                                      */
/*****************************************************************************/
void deleteSet(LargeSet set) {
	if (set)
		free(set);
}


#define INCL(BITSET,BIT) ((BITSET) |= (BIT))

#define EXCL(BITSET,BIT) ((BITSET) &= (~(BIT)))



/*****************************************************************************/
/* Tests if an element is in the set.                                        */
/*****************************************************************************/
bool In(LargeSet set, uint16 element) {
	return ((1 << ((element - 1) % 16)) & (set->array[(element - 1) >> 4])) > 0;
}



/*****************************************************************************/
/* Sets an element in the Large set.                                         */
/*****************************************************************************/
void inclElement(LargeSet set, uint16 element) {
	INCL((set->array[(element - 1) >> 4]), (1 << ((element - 1) % 16)));
}



/*****************************************************************************/
/* Removes an element in the Large set.                                      */
/*****************************************************************************/
void exclElement(LargeSet set, uint16 element) {
	EXCL((set->array[(element - 1) >> 4]), (1 << ((element - 1) % 16)));
}

} // End of namespace Lab
