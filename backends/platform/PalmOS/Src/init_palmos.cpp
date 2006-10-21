/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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

#include <PalmOS.h>

#include "globals.h"
#include "init_palmos.h"

static UInt16 autoOffDelay;

void PalmInit(UInt8 init) {
	// set screen depth
	UInt32 depth = 8;
	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);

	if (init & INIT_AUTOOFF) {
		autoOffDelay = SysSetAutoOffTime(0);
		EvtResetAutoOffTimer();
	}

}

void PalmRelease(UInt8 init) {
	if (init & INIT_AUTOOFF) {
		SysSetAutoOffTime(autoOffDelay);
		EvtResetAutoOffTimer();
	}
}

Err PalmHRInit(UInt32 depth) {
	Err e;
	UInt32 width = 320;
	UInt32 height = 320;
	Boolean color = true;

	e = WinScreenMode (winScreenModeSet, &width, &height, &depth, &color);

	if (!e) {
		UInt32 attr;
		WinScreenGetAttribute(winScreenDensity, &attr);
		e = (attr != kDensityDouble);
	}

	return e;
}

void PalmHRRelease() {
	// should i do something here ?
}

UInt8 PalmScreenSize(Coord *stdw, Coord *stdh, Coord *fullw, Coord *fullh) {
	UInt32 ftr;
	UInt8 mode = 0;

	Coord sw = 160;
	Coord sh = 160;	

	// Hi-Density present ?
	if (!FtrGet(sysFtrCreator, sysFtrNumWinVersion, &ftr)) {
		if (ftr >= 4) {
			sw = 320;
			sh = 320;
		}
	}

	Coord fw = sw;
	Coord fh = sh;

	// if feature set, not set on Garmin iQue3600 ???
	if (!(FtrGet(sysFtrCreator, sysFtrNumInputAreaFlags, &ftr))) {
		if (ftr & grfFtrInputAreaFlagCollapsible) {
			UInt16 curOrientation = SysGetOrientation();

			if (curOrientation == sysOrientationLandscape ||
				curOrientation == sysOrientationReverseLandscape
					)
				mode = PALM_LANDSCAPE;
			else
				mode = PALM_PORTRAIT;

			PINSetInputTriggerState(pinInputTriggerEnabled);
			PINSetInputAreaState(pinInputAreaClosed);
			StatHide();

			WinGetDisplayExtent(&fw, &fh);
			fw *= 2;
			fh *= 2;

			StatShow();
			PINSetInputAreaState(pinInputAreaOpen);
			PINSetInputTriggerState(pinInputTriggerDisabled);
		}
	}

	if (stdw) *stdw = sw;
	if (stdh) *stdh = sh;
	if (fullw) *fullw = fw;
	if (fullh) *fullh = fh;

	return mode;
}

void PalmGetMemory(UInt32* storageMemoryP, UInt32* dynamicMemoryP, UInt32 *storageFreeP, UInt32 *dynamicFreeP) {
	UInt32		free, max;

	Int16		i;
	Int16		nCards;
	UInt16		cardNo;
	UInt16		heapID;

	UInt32		storageMemory = 0;
	UInt32		dynamicMemory = 0;
	UInt32		storageFree = 0;
	UInt32		dynamicFree = 0;

	// Iterate through each card to support devices with multiple cards.
	nCards = MemNumCards();		

	for (cardNo = 0; cardNo < nCards; cardNo++) {
		// Iterate through the RAM heaps on a card (excludes ROM).
		for (i=0; i< MemNumRAMHeaps(cardNo); i++) {
			// Obtain the ID of the heap.
			heapID = MemHeapID(cardNo, i);
			// Calculate the total memory and free memory of the heap.
			MemHeapFreeBytes(heapID, &free, &max);

			// If the heap is dynamic, increment the dynamic memory total.
			if (MemHeapDynamic(heapID)) {
				dynamicMemory += MemHeapSize(heapID);
				dynamicFree += free;

			// The heap is nondynamic (storage ?).
			} else {
				storageMemory += MemHeapSize(heapID);
				storageFree += free;
			}
		}
	}
	// Reduce the stats to KB.  Round the results.
	dynamicMemory = dynamicMemory / 1024L;
	storageMemory = storageMemory / 1024L;

	dynamicFree = dynamicFree / 1024L;
	storageFree = storageFree / 1024L;

	if (dynamicMemoryP) *dynamicMemoryP = dynamicMemory;
	if (storageMemoryP) *storageMemoryP = storageMemory;
	if (dynamicFreeP) *dynamicFreeP = dynamicFree;
	if (storageFreeP) *storageFreeP = storageFree;
}
