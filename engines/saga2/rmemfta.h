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

#ifndef SAGA2_RMEMFTA_H
#define SAGA2_RMEMFTA_H

namespace Saga2 {

/* ===================================================================== *
   Memory Classes
 * ===================================================================== */

enum memUsageGroup {
	memUnknown      = 0,  // Unknown Allocation
	// system classes
	memMain,              // Init & Cleanup
	memMessagers,         // Error handling, debugging
	memSysEnviron,        // System Environment Interface
	memSysResource,       // System Resource (subtyped)
	memSysTaskMan,        // Task & thread management
	// program classes
	memPrgResMan,         // General resource management
	memPrgInput,          // Global input queues
	memPrgOutput,         // Global output services
	// app classes
	memPropList,          // Property lists
	memTempAlloc,         // Temp bitmaps for interface
	memBitArray,          // Bit Array class
	memDispNode,          // Display Node Memory
	memInterface,         // User Interface
	memSpells,            // Spell memory
	memEffects,           // Effect memory
	memTimer,             // Game Timers
	memPathFind,          // PathFinders
	memScripts,           // SAGA memory
	memObjects,           // Iterators mostly

	NumMemGroups,         // max value
};

enum memSysResSubtype {
	msrsMemory      = 'M',
	msrsFile        = 'F',
	msrsSocket      = 'N',
	msrsGraphics    = 'G',
	msrsVideo       = 'V',
	msrsAudio       = 'A',
	msrsTimer       = 'T',
};


// FIXME: These have to go

#define MALLOC(s)       malloc(s)
#define TALLOC(s,t)     malloc(s)
#define SALLOC(s,t,c)   malloc(s)

#define NEW         new
#define NEW_MAIN    new
#define NEW_MSGR    new
#define NEW_SYSE    new
#define NEW_SYST    new
#define NEW_PRES    new
#define NEW_PINP    new
#define NEW_POUT    new
#define NEW_UI      new
#define NEW_SPEL    new
#define NEW_TMR     new
#define NEW_PATH    new
#define NEW_DISP    new
#define NEW_ITER    new
#define NEW_SAGA    new
#define NEW_EFCT    new

#define NEW_SRMEM   new
#define NEW_SRFIL   new
#define NEW_SRSOK   new
#define NEW_SRGRF   new
#define NEW_SRVID   new
#define NEW_SRAUD   new
#define NEW_SRTIM   new

}

#endif  //SAGA2_RMEMFTA_H
