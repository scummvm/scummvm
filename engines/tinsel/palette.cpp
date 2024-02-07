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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Palette Allocator for IBM PC.
 */

#include "tinsel/dw.h"		// TBLUE1 definition
#include "tinsel/graphics.h"
#include "tinsel/handle.h"	// LockMem definition
#include "tinsel/palette.h"	// palette allocator structures etc.
#include "tinsel/sysvar.h"
#include "tinsel/tinsel.h"

#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/paletteman.h"

namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

/** video DAC transfer Q structure */
struct VIDEO_DAC_Q {
	union {
		SCNHANDLE hRGBarray;	///< handle of palette or
		COLORREF *pRGBarray;	///< list of palette colors
		COLORREF  singleRGB;
	} pal;
	bool bHandle;		///< when set - use handle of palette
	int destDACindex;	///< start index of palette in video DAC
	int numColors;		///< number of colors in "hRGBarray"
};


/** video DAC transfer Q length */
#define VDACQLENGTH (NUM_PALETTES + 2)

/** color index of the 4 colors used for the translucent palette */
#define COL_HILIGHT TBLUE1

//----------------- LOCAL GLOBAL DATA --------------------

// These vars are reset upon engine destruction

/** palette allocator data */
static PALQ g_palAllocData[NUM_PALETTES];

/** video DAC transfer Q */
static VIDEO_DAC_Q g_vidDACdata[VDACQLENGTH];

/** video DAC transfer Q head pointer */
static VIDEO_DAC_Q *g_pDAChead;

/** the translucent palette lookup table */
uint8 g_transPalette[MAX_COLORS];	// used in graphics.cpp

static int g_translucentIndex	= 228;
static int g_talkIndex		= 233;

static COLORREF g_talkColRef = 0;
static COLORREF g_tagColRef = 0;


#ifdef DEBUG
// diagnostic palette counters
static int numPals = 0;
static int maxPals = 0;
static int maxDACQ = 0;
#endif

void ResetVarsPalette() {
	memset(g_palAllocData, 0, sizeof(g_palAllocData));

	g_pDAChead = g_vidDACdata;

	memset(g_transPalette, 0, sizeof(g_transPalette));

	g_translucentIndex = 228;
	g_talkIndex = 233;

	g_talkColRef = 0;
	g_tagColRef = 0;
}

/**
 * Map PSX palettes to original palette from resource file
 */
void psxPaletteMapper(PALQ *originalPal, uint8 *psxClut, byte *mapperTable) {
	PALETTE *pal = _vm->_handle->GetPalette(originalPal->hPal);
	bool colorFound = false;
	uint16 clutEntry = 0;

	// Empty the table with color correspondences
	memset(mapperTable, 0, 16);

	for (int j = 1; j < 16; j++) {
		clutEntry = READ_16(psxClut + (sizeof(uint16) * j));
		if (clutEntry) {
			if (clutEntry == 0x7EC0) { // This is an already known value, used by the in-game text
				mapperTable[j] = 232;
				continue;
			}

			// Check for correspondent color
			for (int32 i = 0; (i < pal->numColors) && !colorFound; i++) {
				// get R G B values in the same way as psx format converters
				uint16 psxEquivalent = TINSEL_PSX_RGB(
					pal->palette[i * 3] >> 3,
					pal->palette[i * 3 + 1] >> 3,
					pal->palette[i * 3 + 2] >> 3
				);

				if (psxEquivalent == clutEntry) {
					mapperTable[j] = i + 1; // Add entry in the table for the found color
					colorFound = true;
				}
			}
			colorFound = false;
		} else { // The rest of the entries are zeroes
			delete pal;
			return;
		}
	}

	delete pal;
}

/**
 * Transfer palettes in the palette Q to Video DAC.
 */
void PalettesToVideoDAC() {
	PALQ *pPalQ;				// palette Q iterator
	VIDEO_DAC_Q *pDACtail = g_vidDACdata;	// set tail pointer
	byte pal[768];

	memset(pal, 0, sizeof(pal));

	// while Q is not empty
	while (g_pDAChead != pDACtail) {
#ifdef	DEBUG
		// make sure palette does not overlap
		assert(pDACtail->destDACindex + pDACtail->numColors <= MAX_COLORS);
#else
		// make sure palette does not overlap
		if (pDACtail->destDACindex + pDACtail->numColors > MAX_COLORS)
			pDACtail->numColors = MAX_COLORS - pDACtail->destDACindex;
#endif

		if (pDACtail->bHandle) {
			// we are using a palette handle
			PALETTE *newPal = _vm->_handle->GetPalette(pDACtail->pal.hRGBarray);
			memcpy(pal, newPal->palette, pDACtail->numColors * 3);
			delete newPal;
		} else if (pDACtail->numColors == 1) {
			// we are using a single color palette
			pal[0] = (byte)(pDACtail->pal.singleRGB & 0xFF);
			pal[1] = (byte)((pDACtail->pal.singleRGB >> 8) & 0xFF);
			pal[2] = (byte)((pDACtail->pal.singleRGB >> 16) & 0xFF);
		} else {
			// we are using a palette pointer
			for (int i = 0; i < pDACtail->numColors; ++i) {
				pal[i * 3 + 0] = (byte)(pDACtail->pal.pRGBarray[i] & 0xFF);
				pal[i * 3 + 1] = (byte)((pDACtail->pal.pRGBarray[i] >> 8) & 0xFF);
				pal[i * 3 + 2] = (byte)((pDACtail->pal.pRGBarray[i] >> 16) & 0xFF);
			}
		}

		// Swap black/white colors in the Mac version.
		// We need to swap the current black/white values so that screen fade
		// in/out is done correctly.
		if (TinselV1Mac) {
			byte macWhite = pal[  0 * 3 + 0];
			byte macBlack = pal[254 * 3 + 0];
			pal[254 * 3 + 0] = pal[254 * 3 + 1] = pal[254 * 3 + 2] = macWhite;
			pal[  0 * 3 + 0] = pal[  0 * 3 + 1] = pal[  0 * 3 + 2] = macBlack;
		}

		// update the system palette
		g_system->getPaletteManager()->setPalette(pal, pDACtail->destDACindex, pDACtail->numColors);

		// update tail pointer
		pDACtail++;

	}

	// reset video DAC transfer Q head pointer
	g_pDAChead = g_vidDACdata;

	// clear all palette moved bits
	for (pPalQ = g_palAllocData; pPalQ < g_palAllocData + NUM_PALETTES; pPalQ++)
		pPalQ->posInDAC &= ~PALETTE_MOVED;
}

/**
 * Commpletely reset the palette allocator.
 */
void ResetPalAllocator() {
#ifdef DEBUG
	// clear number of palettes in use
	numPals = 0;
#endif

	// wipe out the palette allocator data
	memset(g_palAllocData, 0, sizeof(g_palAllocData));

	// reset video DAC transfer Q head pointer
	g_pDAChead = g_vidDACdata;
}

#ifdef	DEBUG
/**
 * Shows the maximum number of palettes used at once.
 */
void PaletteStats() {
	debug("%i palettes of %i used", maxPals, NUM_PALETTES);
	debug("%i DAC queue entries of %i used", maxDACQ, VDACQLENGTH);
}
#endif

/**
 * Places a palette in the video DAC queue.
 * @param posInDAC			Position in video DAC
 * @param numColors		Number of colors in palette
 * @param hPalette			Handle to palette
 */
void UpdateDACqueueHandle(int posInDAC, int numColors, SCNHANDLE hPalette) {
	// check Q overflow
	assert(g_pDAChead < g_vidDACdata + VDACQLENGTH);

	g_pDAChead->destDACindex = posInDAC & ~PALETTE_MOVED;	// set index in video DAC
	g_pDAChead->numColors = numColors;	// set number of colors
	g_pDAChead->pal.hRGBarray = hPalette;	// set handle of palette
	g_pDAChead->bHandle = true;		// we are using a palette handle

	// update head pointer
	++g_pDAChead;

#ifdef DEBUG
	if ((g_pDAChead-g_vidDACdata) > maxDACQ)
		maxDACQ = g_pDAChead-g_vidDACdata;
#endif
}

/**
 * Places a palette in the video DAC queue.
 * @param posInDAC			Position in video DAC
 * @param numColors		Number of colors in palette
 * @param pColors			List of RGB triples
 */
void UpdateDACqueue(int posInDAC, int numColors, COLORREF *pColors) {
	// check Q overflow
	assert(g_pDAChead < g_vidDACdata + NUM_PALETTES);

	g_pDAChead->destDACindex = posInDAC & ~PALETTE_MOVED;	// set index in video DAC
	g_pDAChead->numColors = numColors;	// set number of colors
	if (numColors == 1)
		g_pDAChead->pal.singleRGB = *pColors;	// set single color of which the "palette" consists
	else
		g_pDAChead->pal.pRGBarray = pColors;	// set addr of palette
	g_pDAChead->bHandle = false;		// we are not using a palette handle

	// update head pointer
	++g_pDAChead;

#ifdef DEBUG
	if ((g_pDAChead-g_vidDACdata) > maxDACQ)
		maxDACQ = g_pDAChead-g_vidDACdata;
#endif
}


/**
 * Places a "palette" consisting of a single color in the video DAC queue.
 * @param posInDAC			Position in video DAC
 * @param color				Single RGB triple
 */
void UpdateDACqueue(int posInDAC, COLORREF color) {
	// check Q overflow
	assert(g_pDAChead < g_vidDACdata + NUM_PALETTES);

	g_pDAChead->destDACindex = posInDAC & ~PALETTE_MOVED;	// set index in video DAC
	g_pDAChead->numColors = 1;	// set number of colors
	g_pDAChead->pal.singleRGB = color;	// set single color of which the "palette" consists
	g_pDAChead->bHandle = false;		// we are not using a palette handle

	// update head pointer
	++g_pDAChead;

#ifdef DEBUG
	if ((g_pDAChead-g_vidDACdata) > maxDACQ)
		maxDACQ = g_pDAChead-g_vidDACdata;
#endif
}

/**
 * Allocate a palette.
 * @param hNewPal			Palette to allocate
 */
PALQ *AllocPalette(SCNHANDLE hNewPal) {
	PALQ *pPrev, *p;		// walks palAllocData
	int iDAC;		// color index in video DAC
	PALQ *pNxtPal;		// next PALQ struct in palette allocator

	// search all structs in palette allocator - see if palette already allocated
	for (p = g_palAllocData; p < g_palAllocData + NUM_PALETTES; p++) {
		if (p->hPal == hNewPal) {
			// found the desired palette in palette allocator
			p->objCount++;	// update number of objects using palette
			return p;	// return palette queue position
		}
	}

	PALETTE *pal = _vm->_handle->GetPalette(hNewPal);

	// search all structs in palette allocator - find a free slot
	iDAC = FGND_DAC_INDEX;	// init DAC index to first available foreground color

	for (p = g_palAllocData; p < g_palAllocData + NUM_PALETTES; p++) {
		if (p->hPal == 0) {
			// found a free slot in palette allocator
			p->objCount = 1;	// init number of objects using palette
			p->posInDAC = iDAC;	// set palettes start pos in video DAC
			p->hPal = hNewPal;	// set hardware palette data
			p->numColors = pal->numColors;	// set number of colors in palette

			if (TinselVersion >= 2)
				// Copy all the colors
				memcpy(p->palRGB, pal->palRGB, p->numColors * sizeof(COLORREF));

#ifdef DEBUG
			// one more palette in use
			if (++numPals > maxPals)
				maxPals = numPals;
#endif

			// Q the change to the video DAC
			if (TinselVersion >= 2)
				UpdateDACqueue(p->posInDAC, p->numColors, p->palRGB);
			else
				UpdateDACqueueHandle(p->posInDAC, p->numColors, p->hPal);

			// move all palettes after this one down (if necessary)
			for (pPrev = p, pNxtPal = pPrev + 1; pNxtPal < g_palAllocData + NUM_PALETTES; pNxtPal++) {
				if (pNxtPal->hPal != 0) {
					// palette slot is in use
					if (pNxtPal->posInDAC >= pPrev->posInDAC + pPrev->numColors)
						// no need to move palettes down
						break;

					// move palette down - indicate change
					pNxtPal->posInDAC = (pPrev->posInDAC + pPrev->numColors) | PALETTE_MOVED;

					// Q the palette change in position to the video DAC
					if (TinselVersion <= 1)
						UpdateDACqueueHandle(pNxtPal->posInDAC, pNxtPal->numColors, pNxtPal->hPal);
					else if (!pNxtPal->bFading)
						UpdateDACqueue(pNxtPal->posInDAC, pNxtPal->numColors, pNxtPal->palRGB);

					// update previous palette to current palette
					pPrev = pNxtPal;
				}
			}

			delete pal;

			// return palette pointer
			return p;
		}

		// set new DAC index
		iDAC = p->posInDAC + p->numColors;
	}

	// no free palettes
	error("AllocPalette(): formally 'assert(0)!'");
}

/**
 * Free a palette allocated with "AllocPalette".
 * @param pFreePal			Palette queue entry to free
 */
void FreePalette(PALQ *pFreePal) {
	// validate palette Q pointer
	assert(pFreePal >= g_palAllocData && pFreePal <= g_palAllocData + NUM_PALETTES - 1);

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
PALQ *FindPalette(SCNHANDLE hSrchPal) {
	PALQ *pPal;		// palette allocator iterator

	// search all structs in palette allocator
	for (pPal = g_palAllocData; pPal < g_palAllocData + NUM_PALETTES; pPal++) {
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
void SwapPalette(PALQ *pPalQ, SCNHANDLE hNewPal) {
	PALETTE *pal = _vm->_handle->GetPalette(hNewPal);

	// validate palette Q pointer
	assert(pPalQ >= g_palAllocData && pPalQ <= g_palAllocData + NUM_PALETTES - 1);

	if (pPalQ->numColors >= pal->numColors) {
		// new palette will fit the slot

		// install new palette
		pPalQ->hPal = hNewPal;

		if (TinselVersion >= 2) {
			pPalQ->numColors = pal->numColors;

			// Copy all the colors
			memcpy(pPalQ->palRGB, pal->palRGB, pal->numColors * sizeof(COLORREF));

			if (!pPalQ->bFading)
				// Q the change to the video DAC
				UpdateDACqueue(pPalQ->posInDAC, pal->numColors, pPalQ->palRGB);
		} else {
			// Q the change to the video DAC
			UpdateDACqueueHandle(pPalQ->posInDAC, pal->numColors, hNewPal);
		}
	} else {
		// # colors are different - will have to update all following palette entries
		assert(TinselVersion <= 1); // Fatal error for Tinsel 2

		PALQ *pNxtPalQ;		// next palette queue position

		for (pNxtPalQ = pPalQ + 1; pNxtPalQ < g_palAllocData + NUM_PALETTES; pNxtPalQ++) {
			if (pNxtPalQ->posInDAC >= pPalQ->posInDAC + pPalQ->numColors)
				// no need to move palettes down
				break;

			// move palette down
			pNxtPalQ->posInDAC = (pPalQ->posInDAC + pPalQ->numColors) | PALETTE_MOVED;

			// Q the palette change in position to the video DAC
			UpdateDACqueueHandle(pNxtPalQ->posInDAC,
				pNxtPalQ->numColors,
				pNxtPalQ->hPal);

			// update previous palette to current palette
			pPalQ = pNxtPalQ;
		}
	}

	delete pal;
}

/**
 * Statless palette iterator. Returns the next palette in the list
 * @param pStrtPal			Palette to start from - when NULL will start from beginning of list
 */
PALQ *GetNextPalette(PALQ *pStrtPal) {
	if (pStrtPal == NULL) {
		// start of palette iteration - return 1st palette
		return (g_palAllocData[0].objCount) ? g_palAllocData : NULL;
	}

	// validate palette Q pointer
	assert(pStrtPal >= g_palAllocData && pStrtPal <= g_palAllocData + NUM_PALETTES - 1);

	// return next active palette in list
	while (++pStrtPal < g_palAllocData + NUM_PALETTES) {
		if (pStrtPal->objCount)
			// active palette found
			return pStrtPal;
	}

	// non found
	return NULL;
}

/**
 * Sets the current background color.
 * @param color			Color to set the background to
 */
void SetBgndColor(COLORREF color) {
	// update background color struct by queuing the change to the video DAC
	UpdateDACqueue(BGND_DAC_INDEX, color);
}

/**
 * Note whether a palette is being faded.
 * @param pPalQ			Palette queue position
 * @param bFading		Whether it is fading
 */
void FadingPalette(PALQ *pPalQ, bool bFading) {
	// validate palette Q pointer
	assert(pPalQ >= g_palAllocData && pPalQ <= g_palAllocData + NUM_PALETTES - 1);

	// validate that this is a change
	assert(pPalQ->bFading != bFading);

	pPalQ->bFading = bFading;
}

/**
 * All fading processes have just been killed, so none of the
 * palettes are fading.
 */
void NoFadingPalettes() {
	PALQ *pPalQ;

	for (pPalQ = g_palAllocData; pPalQ <= g_palAllocData + NUM_PALETTES - 1; pPalQ++) {
		pPalQ->bFading = false;
	}
}

/**
 * Builds the translucent palette from the current backgrounds palette.
 * @param hPalette			Handle to current background palette
 */
void CreateTranslucentPalette(SCNHANDLE hPalette) {
	PALETTE *pal = _vm->_handle->GetPalette(hPalette);

	// leave background color alone
	g_transPalette[0] = 0;

	for (int32 i = 0; i < pal->numColors; i++) {
		byte red = pal->palette[i * 3];
		byte green = pal->palette[i * 3 + 1];
		byte blue = pal->palette[i * 3 + 2];

		// calculate the Value field of the HSV color model
		unsigned val = (red > green) ? red : green;
		val = (val > blue) ? val : blue;

		// map the Value field to one of the 4 colors reserved for the translucent palette
		val /= 63;
		byte blackColorIndex = (!TinselV1Mac) ? 0 : 255;
		g_transPalette[i + 1] = (uint8)((val == 0) ? blackColorIndex : val +
			((TinselVersion >= 2) ? TranslucentColor() : COL_HILIGHT) - 1);
	}

	delete pal;
}

/**
 * Returns an adjusted color RGB
 * @param color		Color to scale
 */
static COLORREF DimColor(byte r, byte g, byte b, int factor) {
	if (factor == 10) {
		// No change
		return TINSEL_RGB(r, g, b);
	} else if (factor == 0) {
		// No brightness
		return 0;
	} else {
		// apply multiplier to RGB components
		uint32 red   = r * factor / 10;
		uint32 green = g * factor / 10;
		uint32 blue  = b * factor / 10;

		// return new color
		return TINSEL_RGB(red, green, blue);
	}
}

/**
 * DimPartPalette
 */
void DimPartPalette(SCNHANDLE hDimPal, int startColor, int length, int brightness) {
	PALQ *pPalQ = FindPalette(hDimPal);
	assert(pPalQ);

	// Adjust for the fact that palettes don't contain color 0
	startColor -= 1;

	// Check some other things
	if (startColor + length > pPalQ->numColors)
		error("DimPartPalette(): color overrun");

	// Check if the palette actually contains entries
	if (length == 0)
		return;

	PALETTE *pal = _vm->_handle->GetPalette(hDimPal);

	for (int iColor = startColor; iColor < startColor + length; iColor++) {
		byte r = pal->palette[iColor * 3];
		byte g = pal->palette[iColor * 3 + 1];
		byte b = pal->palette[iColor * 3 + 2];
		pPalQ->palRGB[iColor] = DimColor(r, g, b, brightness);
	}

	delete pal;

	if (!pPalQ->bFading) {
		// Q the change to the video DAC
		UpdateDACqueue(pPalQ->posInDAC + startColor, length, &pPalQ->palRGB[startColor]);
	}
}

int DarkGreen() {
	return _vm->screen().format.RGBToColor(0x00, 0x40, 0x00);
}

int TranslucentColor() {
	return g_translucentIndex;
}

int HighlightColor() {
	if (TinselVersion == 3) {
		return _vm->screen().format.RGBToColor(0x00, 0x80, 0x00);
	}
	UpdateDACqueue(g_talkIndex, (COLORREF)SysVar(SYS_HighlightRGB));

	return g_talkIndex;
}

int TalkColor() {
	return (TinselVersion >= 2) ? g_talkIndex : TALKFONT_COL;
}

void SetTalkColorRef(COLORREF colRef) {
	g_talkColRef = colRef;
}

COLORREF GetTalkColorRef() {
	return g_talkColRef;
}

void SetTagColorRef(COLORREF colRef) {
	g_tagColRef = colRef;
}

COLORREF GetTagColorRef() {
	return g_tagColRef;
}

void SetTranslucencyOffset(int offset) {
	g_translucentIndex = offset;
}

void SetTalkTextOffset(int offset) {
	g_talkIndex = offset;
}

} // End of namespace Tinsel
