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
 * Palette Allocator for IBM PC.
 */

#include "tinsel/dw.h"		// TBLUE1 definition
#include "tinsel/graphics.h"
#include "tinsel/handle.h"	// LockMem definition
#include "tinsel/palette.h"	// palette allocator structures etc.
#include "tinsel/tinsel.h"

#include "common/system.h"

namespace Tinsel {

/** background colour */
static COLORREF bgndColour = BLACK;

/** palette allocator data */
static PALQ palAllocData[NUM_PALETTES];


/** video DAC transfer Q length */
#define VDACQLENGTH (NUM_PALETTES+2)

/** video DAC transfer Q */
static VIDEO_DAC_Q vidDACdata[VDACQLENGTH];

/** video DAC transfer Q head pointer */
static VIDEO_DAC_Q *pDAChead;

/** colour index of the 4 colours used for the translucent palette */
#define COL_HILIGHT	TBLUE1

/** the translucent palette lookup table */
uint8 transPalette[MAX_COLOURS];	// used in graphics.cpp

#ifdef DEBUG
// diagnostic palette counters
static int numPals = 0;
static int maxPals = 0;
static int maxDACQ = 0;
#endif

/**
 * Transfer palettes in the palette Q to Video DAC.
 */
void PalettesToVideoDAC(void) {
	PPALQ pPalQ;				// palette Q iterator
	PVIDEO_DAC_Q pDACtail = vidDACdata;	// set tail pointer
	bool needUpdate = false;

	// while Q is not empty
	while (pDAChead != pDACtail) {
		PALETTE *pPalette;	// pointer to hardware palette
		COLORREF *pColours;	// pointer to list of RGB triples

#ifdef	DEBUG
		// make sure palette does not overlap
		assert(pDACtail->destDACindex + pDACtail->numColours <= MAX_COLOURS);
#else
		// make sure palette does not overlap
		if (pDACtail->destDACindex + pDACtail->numColours > MAX_COLOURS)
			pDACtail->numColours = MAX_COLOURS - pDACtail->destDACindex;
#endif

		if (pDACtail->bHandle) {
			// we are using a palette handle

			// get hardware palette pointer
			pPalette = (PALETTE *)LockMem(pDACtail->pal.hRGBarray);

			// get RGB pointer
			pColours = pPalette->palRGB;
		} else {
			// we are using a palette pointer
			pColours = pDACtail->pal.pRGBarray;
		}

		if (pDACtail->numColours > 0)
			needUpdate = true;

		// update the system palette
		g_system->setPalette((byte *)pColours, pDACtail->destDACindex, pDACtail->numColours);

		// update tail pointer
		pDACtail++;

	}

	// reset video DAC transfer Q head pointer
	pDAChead = vidDACdata;

	// clear all palette moved bits
	for (pPalQ = palAllocData; pPalQ < palAllocData + NUM_PALETTES; pPalQ++)
		pPalQ->posInDAC &= ~PALETTE_MOVED;

	if (needUpdate)
		_vm->screen().update();
}

/**
 * Commpletely reset the palette allocator.
 */
void ResetPalAllocator(void) {
#ifdef DEBUG
	// clear number of palettes in use
	numPals = 0;
#endif

	// wipe out the palette allocator data
	memset(palAllocData, 0, sizeof(palAllocData));

	// reset video DAC transfer Q head pointer
	pDAChead = vidDACdata;
}

#ifdef	DEBUG
/**
 * Shows the maximum number of palettes used at once.
 */
void PaletteStats(void) {
	printf("%i palettes of %i used.\n", maxPals, NUM_PALETTES);
	printf("%i DAC queue entries of %i used.\n", maxDACQ, VDACQLENGTH);
}
#endif

/**
 * Places a palette in the video DAC queue.
 * @param posInDAC			Position in video DAC
 * @param numColours		Number of colours in palette
 * @param hPalette			Handle to palette
 */
void UpdateDACqueueHandle(int posInDAC, int numColours, SCNHANDLE hPalette) {
	// check Q overflow
	assert(pDAChead < vidDACdata + VDACQLENGTH);

	pDAChead->destDACindex = posInDAC & ~PALETTE_MOVED;	// set index in video DAC
	pDAChead->numColours = numColours;	// set number of colours
	pDAChead->pal.hRGBarray = hPalette;	// set handle of palette
	pDAChead->bHandle = true;		// we are using a palette handle

	// update head pointer
	++pDAChead;

#ifdef DEBUG
	if ((pDAChead-vidDACdata) > maxDACQ)
		maxDACQ = pDAChead-vidDACdata;
#endif
}

/**
 * Places a palette in the video DAC queue.
 * @param posInDAC			Position in video DAC
 * @param numColours,		Number of colours in palette
 * @param pColours			List of RGB triples
 */
void UpdateDACqueue(int posInDAC, int numColours, COLORREF *pColours) {
	// check Q overflow
	assert(pDAChead < vidDACdata + NUM_PALETTES);

	pDAChead->destDACindex = posInDAC & ~PALETTE_MOVED;	// set index in video DAC
	pDAChead->numColours = numColours;	// set number of colours
	pDAChead->pal.pRGBarray = pColours;	// set addr of palette
	pDAChead->bHandle = false;		// we are not using a palette handle

	// update head pointer
	++pDAChead;

#ifdef DEBUG
	if ((pDAChead-vidDACdata) > maxDACQ)
		maxDACQ = pDAChead-vidDACdata;
#endif
}

/**
 * Allocate a palette.
 * @param hNewPal			Palette to allocate
 */
PPALQ AllocPalette(SCNHANDLE hNewPal) {
	PPALQ pPrev, p;		// walks palAllocData
	int iDAC;		// colour index in video DAC
	PPALQ pNxtPal;		// next PALQ struct in palette allocator
	PALETTE *pNewPal;

	// get pointer to new palette
	pNewPal = (PALETTE *)LockMem(hNewPal);

	// search all structs in palette allocator - see if palette already allocated
	for (p = palAllocData; p < palAllocData + NUM_PALETTES; p++) {
		if (p->hPal == hNewPal) {
			// found the desired palette in palette allocator
			p->objCount++;	// update number of objects using palette
			return p;	// return palette queue position
		}
	}

	// search all structs in palette allocator - find a free slot
	iDAC = FGND_DAC_INDEX;	// init DAC index to first available foreground colour

	for (p = palAllocData; p < palAllocData + NUM_PALETTES; p++) {
		if (p->hPal == 0) {
			// found a free slot in palette allocator
			p->objCount = 1;	// init number of objects using palette
			p->posInDAC = iDAC;	// set palettes start pos in video DAC
			p->hPal = hNewPal;	// set hardware palette data
			p->numColours = FROM_LE_32(pNewPal->numColours);	// set number of colours in palette

#ifdef DEBUG
			// one more palette in use
			if (++numPals > maxPals)
				maxPals = numPals;
#endif

			// Q the change to the video DAC
			UpdateDACqueueHandle(p->posInDAC, p->numColours, p->hPal);

			// move all palettes after this one down (if necessary)
			for (pPrev = p, pNxtPal = pPrev + 1; pNxtPal < palAllocData + NUM_PALETTES; pNxtPal++) {
				if (pNxtPal->hPal != 0) {
					// palette slot is in use
					if (pNxtPal->posInDAC >= pPrev->posInDAC + pPrev->numColours)
						// no need to move palettes down
						break;

					// move palette down - indicate change
					pNxtPal->posInDAC = pPrev->posInDAC
						+ pPrev->numColours | PALETTE_MOVED;

					// Q the palette change in position to the video DAC
					UpdateDACqueueHandle(pNxtPal->posInDAC,
						pNxtPal->numColours,
						pNxtPal->hPal);

					// update previous palette to current palette
					pPrev = pNxtPal;
				}
			}

			// return palette pointer
			return p;
		}

		// set new DAC index
		iDAC = p->posInDAC + p->numColours;
	}

	// no free palettes
	error("AllocPalette(): formally 'assert(0)!'");
}

/**
 * Free a palette allocated with "AllocPalette".
 * @param pFreePal			Palette queue entry to free
 */
void FreePalette(PPALQ pFreePal) {
	// validate palette Q pointer
	assert(pFreePal >= palAllocData && pFreePal <= palAllocData + NUM_PALETTES - 1);

	// reduce the palettes object reference count
	pFreePal->objCount--;

	// make sure palette has not been deallocated too many times
	assert(pFreePal->objCount >= 0);

	if (pFreePal->objCount == 0) {
		pFreePal->hPal = 0;	// palette is no longer in use

#ifdef DEBUG
		// one less palette in use
		--numPals;
		assert(numPals >= 0);
#endif
	}
}

/**
 * Find the specified palette.
 * @param hSrchPal			Hardware palette to search for
 */
PPALQ FindPalette(SCNHANDLE hSrchPal) {
	PPALQ pPal;		// palette allocator iterator

	// search all structs in palette allocator
	for (pPal = palAllocData; pPal < palAllocData + NUM_PALETTES; pPal++) {
		if (pPal->hPal == hSrchPal)
			// found palette in palette allocator
			return pPal;
	}

	// palette not found
	return NULL;
}

/**
 * Swaps the palettes at the specified palette queue position.
 * @param pPalQ			Palette queue position
 * @param hNewPal		New palette
 */
void SwapPalette(PPALQ pPalQ, SCNHANDLE hNewPal) {
	// convert handle to palette pointer
	PALETTE *pNewPal = (PALETTE *)LockMem(hNewPal);

	// validate palette Q pointer
	assert(pPalQ >= palAllocData && pPalQ <= palAllocData + NUM_PALETTES - 1);

	if (pPalQ->numColours >= (int)FROM_LE_32(pNewPal->numColours)) {
		// new palette will fit the slot

		// install new palette
		pPalQ->hPal = hNewPal;

		// Q the change to the video DAC
		UpdateDACqueueHandle(pPalQ->posInDAC, FROM_LE_32(pNewPal->numColours), hNewPal);
	} else {
		// # colours are different - will have to update all following palette entries

		PPALQ pNxtPalQ;		// next palette queue position

		for (pNxtPalQ = pPalQ + 1; pNxtPalQ < palAllocData + NUM_PALETTES; pNxtPalQ++) {
			if (pNxtPalQ->posInDAC >= pPalQ->posInDAC + pPalQ->numColours)
				// no need to move palettes down
				break;

			// move palette down
			pNxtPalQ->posInDAC = pPalQ->posInDAC
				+ pPalQ->numColours | PALETTE_MOVED;

			// Q the palette change in position to the video DAC
			UpdateDACqueueHandle(pNxtPalQ->posInDAC,
				pNxtPalQ->numColours,
				pNxtPalQ->hPal);

			// update previous palette to current palette
			pPalQ = pNxtPalQ;
		}
	}
}

/**
 * Statless palette iterator. Returns the next palette in the list
 * @param pStrtPal			Palette to start from - when NULL will start from beginning of list
 */
PPALQ GetNextPalette(PPALQ pStrtPal) {
	if (pStrtPal == NULL) {
		// start of palette iteration - return 1st palette
		return (palAllocData[0].objCount) ? palAllocData : NULL;
	}

	// validate palette Q pointer
	assert(pStrtPal >= palAllocData && pStrtPal <= palAllocData + NUM_PALETTES - 1);

	// return next active palette in list
	while (++pStrtPal < palAllocData + NUM_PALETTES) {
		if (pStrtPal->objCount)
			// active palette found
			return pStrtPal;
	}

	// non found
	return NULL;
}

/**
 * Sets the current background colour.
 * @param colour			Colour to set the background to
 */
void SetBgndColour(COLORREF colour) {
	// update background colour struct
	bgndColour = colour;

	// Q the change to the video DAC
	UpdateDACqueue(BGND_DAC_INDEX, 1, &bgndColour);
}

/**
 * Builds the translucent palette from the current backgrounds palette.
 * @param hPalette			Handle to current background palette
 */
void CreateTranslucentPalette(SCNHANDLE hPalette) {
	// get a pointer to the palette
	PALETTE *pPal = (PALETTE *)LockMem(hPalette);

	// leave background colour alone
	transPalette[0] = 0;

	for (uint i = 0; i < FROM_LE_32(pPal->numColours); i++) {
		// get the RGB colour model values
		uint8 red   = GetRValue(pPal->palRGB[i]);
		uint8 green = GetGValue(pPal->palRGB[i]);
		uint8 blue  = GetBValue(pPal->palRGB[i]);

		// calculate the Value field of the HSV colour model
		unsigned val = (red > green) ? red : green;
		val = (val > blue) ? val : blue;

		// map the Value field to one of the 4 colours reserved for the translucent palette
		val /= 63;
		transPalette[i + 1] = (uint8)((val == 0) ? 0 : val + COL_HILIGHT - 1);
	}
}

} // end of namespace Tinsel
