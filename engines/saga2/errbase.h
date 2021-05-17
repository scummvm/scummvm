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

#ifndef SAGA2_ERRBASE_H
#define SAGA2_ERRBASE_H

namespace Saga2 {

/* ===================================================================== *
   Base types and constants
 * ===================================================================== */

// ------------------------------------------------------------------
//

typedef uint32  ErrorID;
const ErrorID noError = 0;
const ErrorID unkError = 0xFFFFFFFF;

// ------------------------------------------------------------------
//

typedef const char *ErrText;

// ------------------------------------------------------------------
//

typedef uint32  ErrType;
const ErrType nullErrorList = 0;


/* ===================================================================== *
   Structures and classes
 * ===================================================================== */

// ------------------------------------------------------------------
//

struct ErrorRec {
	ErrorID ID;
	ErrText text;
};


// ------------------------------------------------------------------
//

class ErrorList {
public:
	static char  noErrorString[];
	static char  unkErrorString[];
private:
	ErrType   sourceID;
	size_t    errCount;
protected:
	ErrorRec *errList;

private:
	ErrorList();

protected:
	ErrorList(ErrType et, uint32 ec);
	~ErrorList();
	void setErrorType(ErrType et) {
		sourceID = et;
	}
	size_t setListSize(size_t s);
	void setEntry(uint32 i, ErrorID id, ErrText text);
	size_t eLimit(void) {
		return errCount;
	}


	virtual uint32 lookupMessage(ErrorID id) = 0;

public:
	ErrText errMessage(ErrorID id);
};

extern const ErrText noErrorText;
extern const ErrText unkErrorText;

#endif

}
