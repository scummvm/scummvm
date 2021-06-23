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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "common/debug.h"
#include "graphics/palette.h"

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/vpal.h"
#include "saga2/palette.h"
#include "saga2/display.h"
#include "saga2/hresmgr.h"
#include "saga2/savefile.h"

namespace Saga2 {

const uint32            paletteID   = MKTAG('P', 'A', 'L',  0);

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern hResContext      *tileRes;           // tile resource handle
extern volatile int32   gameTime;

/* ===================================================================== *
   Local struct
 * ===================================================================== */

struct PaletteStateArchive {
	gPalette            currentPalette,
	                    oldPalette,
	                    destPalette;
	int32               startTime,
	                    totalTime;
};

/* ===================================================================== *
   Exports
 * ===================================================================== */

void lightsOut(void);

gPalettePtr             midnightPalette,
                        noonPalette = nullptr,
                        darkPalette;

/* ===================================================================== *
   Globals
 * ===================================================================== */

static gPalette         currentPalette;     //  Currently loaded palette

//  Fade up/down data
static gPalette         oldPalette,         //  Palette at start of fade
       destPalette,        //  Destination palette of fade
       quickPalette;
static int32            startTime,          //  Time index of start of fade
       totalTime;          //  Total fade duration


static PaletteStateArchive  archive;        //  Used for loading and saving
//  palette state

/* ===================================================================== *
   Functions
 * ===================================================================== */

void assertCurrentPalette(void) {
	if (paletteChangesEnabled()) {
		byte palette[256 * 3];
		for (int i = 0; i < 256; i++) {
			palette[i * 3 + 0] = ((byte *)&currentPalette)[i * 3 + 0] << 2;
			palette[i * 3 + 1] = ((byte *)&currentPalette)[i * 3 + 1] << 2;
			palette[i * 3 + 2] = ((byte *)&currentPalette)[i * 3 + 2] << 2;
		}
		g_system->getPaletteManager()->setPalette(palette, 0, 256);
	}
}


//----------------------------------------------------------------------
//	Initialize global palette resources

static inline int16 bscale(int16 s) {
	if (s < 32) return s * 3 / 2;
	else return (63 + s) / 2;
}

void loadPalettes(void) {
	int     i;

	//  Create a black palette for fades
	darkPalette = new gPalette;
	memset(darkPalette, 0, sizeof(gPalette));


	//  Load standard palette
	noonPalette = (gPalettePtr)LoadResource(tileRes, paletteID, "noon palette");

	//  Create a midnight palette for night time effect
	midnightPalette = new gPalette;

	gPalette    *dayPal = noonPalette;
	gPalette    *nightPal = midnightPalette;

	// these colors darkened
	for (i = 10; i < 240; i++) {
//		nightPal->entry[i].r = dayPal->entry[i].r / 2;
//		nightPal->entry[i].g = dayPal->entry[i].g * 2 / 3;
//		nightPal->entry[i].b = dayPal->entry[i].b;

		nightPal->entry[i].r = dayPal->entry[i].r / 3;
		nightPal->entry[i].g = dayPal->entry[i].g / 2;
		nightPal->entry[i].b = bscale(dayPal->entry[i].b);
	}

	// these colors are not
	for (i = 0; i < 10; i++) {
		nightPal->entry[i].r = dayPal->entry[i].r;
		nightPal->entry[i].g = dayPal->entry[i].g;
		nightPal->entry[i].b = dayPal->entry[i].b;
	}

	// and these colors are not
	for (i = 240; i < 256; i++) {
		nightPal->entry[i].r = dayPal->entry[i].r;
		nightPal->entry[i].g = dayPal->entry[i].g;
		nightPal->entry[i].b = dayPal->entry[i].b;
	}

	// single color additions to palette shift
	nightPal->entry[244].r = dayPal->entry[244].r / 3;
	nightPal->entry[244].g = dayPal->entry[244].g / 2;
	nightPal->entry[244].b = bscale(dayPal->entry[244].b);
}

//----------------------------------------------------------------------
//	Dump global palette resources

void cleanupPalettes(void) {
	if (noonPalette) {
		free(noonPalette);
		noonPalette = nullptr;
	}

	if (darkPalette) {
		delete darkPalette;
		darkPalette = nullptr;
	}

	if (midnightPalette) {
		delete midnightPalette;
		midnightPalette = nullptr;
	}
}

//----------------------------------------------------------------------
//	Begin fade up/down

void beginFade(gPalettePtr newPalette, int32 fadeDuration) {
	startTime = gameTime;
	totalTime = fadeDuration;

	//  Save the current palette for interpolation
	memcpy(&oldPalette, &currentPalette, sizeof(gPalette));

	//  Copy the destination palette
	memcpy(&destPalette, newPalette, sizeof(gPalette));
}

//----------------------------------------------------------------------
//	Update state of palette fade up/down

bool updatePalette() {
	int32           elapsedTime;

	elapsedTime = gameTime - startTime;
	if (totalTime == 0)
		return false;

	if (elapsedTime >= totalTime) {
		//  Fade is completed
		totalTime = 0;
		memcpy(&currentPalette, &destPalette, sizeof(gPalette));
		assertCurrentPalette();
		return false;
	} else {
		gPalette        tempPalette;

		debugC(2, kDebugPalettes, "Fade: %d/%d", elapsedTime, totalTime);

		createPalette(
		    &tempPalette,
		    &oldPalette,
		    &destPalette,
		    elapsedTime,
		    totalTime);

		if (memcmp(&tempPalette, &currentPalette, sizeof(gPalette)) != 0) {
			debugC(2, kDebugPalettes, "Fade:*%d/%d", elapsedTime, totalTime);

			memcpy(&currentPalette, &tempPalette, sizeof(gPalette));
			assertCurrentPalette();

		}
		return true;
	}
}

//----------------------------------------------------------------------
//	Linearly interpolate between two specified palettes

void createPalette(
    gPalettePtr newP,
    gPalettePtr srcP,
    gPalettePtr dstP,
    int32       elapsedTime,
    int32       totalTime_) {
	assert(totalTime_ != 0);

	int             i;
	uint32          fadeProgress = (elapsedTime << 8) / totalTime_;

	for (i = 0; i < (long)ARRAYSIZE(newP->entry); i++) {
		gPaletteEntry   *srcPal = &srcP->entry[i];
		gPaletteEntry   *dstPal = &dstP->entry[i];
		gPaletteEntry   *curPal = &newP->entry[i];

		//  Linearly interpolate between the source and end palette.
		curPal->r = srcPal->r + (((dstPal->r - srcPal->r) * fadeProgress) >> 8);
		curPal->g = srcPal->g + (((dstPal->g - srcPal->g) * fadeProgress) >> 8);
		curPal->b = srcPal->b + (((dstPal->b - srcPal->b) * fadeProgress) >> 8);
	}
}

//----------------------------------------------------------------------
//	Set the current palette

void setCurrentPalette(gPalettePtr newPal) {
	memcpy(&currentPalette, newPal, sizeof(gPalette));
	assertCurrentPalette();
}

//----------------------------------------------------------------------
//	Return the current palette

void getCurrentPalette(gPalettePtr pal) {
	memcpy(pal, &currentPalette, sizeof(gPalette));
}

//----------------------------------------------------------------------

void setPaletteToBlack(void) {
	setCurrentPalette(darkPalette);
}


//----------------------------------------------------------------------
//	Initialize the state of the current palette and fade up/down.

void initPaletteState(void) {
	//setCurrentPalette( *noonPalette );
	totalTime = startTime = 0;
}

//----------------------------------------------------------------------
// routines to suspend & restore a palette state (during videos)

void lightsOut(void) {
	memset(&currentPalette, 0, sizeof(currentPalette));
	assertCurrentPalette();
}

void lightsOn(void) {
	setCurrentPalette(noonPalette);
	totalTime = startTime = 0;
	assertCurrentPalette();
}

void usePalette(void *p, size_t s) {
	memcpy(&currentPalette, p, s);
	assertCurrentPalette();
}


//----------------------------------------------------------------------
// routines to suspend & restore a palette state (during videos)

void quickSavePalette(void) {
	memcpy(&quickPalette, &currentPalette, sizeof(gPalette));
}

void quickRestorePalette(void) {
	memcpy(&currentPalette, &quickPalette, sizeof(gPalette));
	assertCurrentPalette();
}


//----------------------------------------------------------------------
//	Save the current state of the current palette and fade up/down in
//	a save file.

void savePaletteState(SaveFileConstructor &saveGame) {
	memcpy(&archive.currentPalette, &currentPalette, sizeof(gPalette));
	memcpy(&archive.oldPalette, &oldPalette, sizeof(gPalette));
	memcpy(&archive.destPalette, &destPalette, sizeof(gPalette));
	archive.startTime = startTime;
	archive.totalTime = totalTime;

	saveGame.writeChunk(
	    MakeID('P', 'A', 'L', 'E'),
	    &archive,
	    sizeof(archive));
}

//----------------------------------------------------------------------
//	Load and set the current state of the current palette and fade
//	up/down from a save file.

void loadPaletteState(SaveFileReader &saveGame) {
	gPalette                tempPalette;

	saveGame.read(&archive, sizeof(archive));

	memcpy(&tempPalette, &archive.currentPalette, sizeof(gPalette));
	memcpy(&oldPalette, &archive.oldPalette, sizeof(gPalette));
	memcpy(&destPalette, &archive.destPalette, sizeof(gPalette));
	startTime = archive.startTime;
	totalTime = archive.totalTime;

	setCurrentPalette(&tempPalette);
}

} // end of namespace Saga2
