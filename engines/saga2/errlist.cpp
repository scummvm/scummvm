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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

/* ===================================================================== *
   Includes
 * ===================================================================== */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/errlist.h"

namespace Saga2 {

/* ===================================================================== *
   SequentialErrorList implementation
 * ===================================================================== */

// ------------------------------------------------------------------
//

SequentialErrorList::SequentialErrorList(ErrType et, uint32 ec, char *es[]) :
	ErrorList(et, ec) {
	for (int i = 0; i < eLimit(); i++)
		setEntry(i, i, es[i]);
}

// ------------------------------------------------------------------
//

SequentialErrorList::SequentialErrorList(ErrType et, char *es[]) :
	ErrorList(et, stringCount(es)) {
	for (int i = 0; i < eLimit(); i++)
		setEntry(i, i, es[i]);
}

// ------------------------------------------------------------------
//

uint32 SequentialErrorList::stringCount(char *es[]) {
	uint32 rv = 0;
	if (es == NULL)
		return 0;
	while (strlen(es[rv])) rv++;
	return rv;
}

// ------------------------------------------------------------------
//

uint32 SequentialErrorList::lookupMessage(ErrorID id) {
	if (id >= 0 && id < eLimit()) {
		return id;
	}
	return unkError;
}



/* ===================================================================== *
   SparseErrorList implementation
 * ===================================================================== */

// ------------------------------------------------------------------
//

SparseErrorList::SparseErrorList(ErrType et,
                                 uint32 ec,
                                 ErrorID ei[],
                                 const char *es[]):
	ErrorList(et, ec) {
	for (int i = 0; i < eLimit(); i++)
		setEntry(i, ei[i], es[i]);
}

// ------------------------------------------------------------------
//

SparseErrorList::SparseErrorList(ErrType et,
                                 uint32 ec,
                                 ErrorRec er[]):
	ErrorList(et, ec) {
	for (int i = 0; i < eLimit(); i++)
		setEntry(i, er[i].ID, er[i].text);
}

// ------------------------------------------------------------------
//

uint32 SparseErrorList::lookupMessage(ErrorID id) {
	for (int i = 0; i < eLimit(); i++) {
		if (errList[i].ID == id)
			return i;
	}
	return unkError;
}

}
