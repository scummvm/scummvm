/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_TOWER_H
#define SAGA2_TOWER_H

namespace Saga2 {

/* ===================================================================== *
   definitions
 * ===================================================================== */

// Program initialization code
#define INITIALIZER(name)  bool name( void )
// Program cleanup code
#define TERMINATOR(name)   void name( void )

/* ===================================================================== *
   Types
 * ===================================================================== */

// ------------------------------------------------------------------------
// function types for init & cleanup
typedef bool PROGRAM_INITIALIZER();
typedef void PROGRAM_TERMINATOR();

typedef PROGRAM_INITIALIZER *pPROGRAM_INITIALIZER;
typedef PROGRAM_TERMINATOR *pPROGRAM_TERMINATOR;

// ------------------------------------------------------------------------
// init, cleanup record

struct TowerLayer {
	int                     ord;
	pPROGRAM_INITIALIZER    init;
	pPROGRAM_TERMINATOR     term;
};

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

// ------------------------------------------------------------------------
// state transition hooks for debugging

int initState();
void setInitState(int i);

// ------------------------------------------------------------------------
// System Init & Cleanup

INITIALIZER(programInit);
TERMINATOR(programTerm);

// ------------------------------------------------------------------------
// System error handling

INITIALIZER(initErrorManagers);
TERMINATOR(termErrorManagers);
INITIALIZER(initDelayedErrors);
TERMINATOR(termDelayedErrors);
INITIALIZER(initActiveErrors) ;
TERMINATOR(termActiveErrors) ;

// ------------------------------------------------------------------------
// Null initializer/ terminator

INITIALIZER(initTowerBase);
TERMINATOR(termTowerBase);

} // end of namespace Saga2

#endif
