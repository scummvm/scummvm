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
#include "saga2/errbase.h"

namespace Saga2 {

/* ===================================================================== *
   Global constants
 * ===================================================================== */

char  ErrorList::noErrorString[] = "";
char  ErrorList::unkErrorString[] = "Unknown error";
const ErrText noErrorText = (const char *) ErrorList::noErrorString;
const ErrText unkErrorText = (const char *) ErrorList::unkErrorString;

/* ===================================================================== *
   ErrorList implementation
 * ===================================================================== */

// ------------------------------------------------------------------
//

ErrorList::ErrorList() {
	sourceID = nullErrorList;
	errCount = 0;
	errList = NULL;
}

// ------------------------------------------------------------------
//

ErrorList::~ErrorList() {
	errCount = 0;
	if (errList != NULL) delete [] errList;
	errList = NULL;
}

// ------------------------------------------------------------------
//

ErrorList::ErrorList(ErrType et, uint32 ec) {
	setErrorType(et);
	setListSize(ec);
}

// ------------------------------------------------------------------
//

size_t ErrorList::setListSize(size_t s) {
	if (s)
		errList = new ErrorRec[s];
	else
		errList = NULL;
	if (errList)
		errCount = s;
	else
		errCount = 0;
	return errCount;
}


// ------------------------------------------------------------------
//

void ErrorList::setEntry(uint32 i, ErrorID id, ErrText text) {
	assert(i >= 0 && i < errCount);
	errList[i].ID = id;
	errList[i].text = text;
}

// ------------------------------------------------------------------
//

ErrText ErrorList::errMessage(ErrorID id) {
	uint32 index = lookupMessage(id);
	if (index == unkError)
		return unkErrorText;
	return errList[index].text;
}

}
