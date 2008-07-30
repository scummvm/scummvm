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
 * Low level graphics interface.
 */

#include "tinsel/graphics.h"
#include "tinsel/handle.h"	// LockMem()
#include "tinsel/object.h"
#include "tinsel/palette.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

// Defines used in graphic drawing
#define CHARPTR_OFFSET 16
#define CHAR_WIDTH 4
#define CHAR_HEIGHT 4

extern uint8 transPalette[MAX_COLOURS];

//----------------- SUPPORT FUNCTIONS ---------------------

/**
 * Straight rendering with transparency support
 */

static void WrtNonZero(DRAWOBJECT *pObj, uint8 *srcP, uint8 *destP, bool applyClipping) {
	// Set up the offset between destination blocks
	int rightClip = applyClipping ? pObj->rightClip : 0;
	Common::Rect boxBounds;

	if (applyClipping) {
		// Adjust the height down to skip any bottom clipping
		pObj->height -= pObj->botClip;

		// Make adjustment for the top clipping row
		srcP += sizeof(uint16) * ((pObj->width + 3) >> 2) * (pObj->topClip >> 2);
		pObj->height -= pObj->topClip;
		pObj->topClip %= 4;
	}

	// Vertical loop
	while (pObj->height > 0) {
		// Get the start of the next line output
		uint8 *tempDest = destP;

		// Get the line width, and figure out which row range within the 4 row high blocks
		// will be displayed if clipping is to be taken into account
		int width = pObj->width;

		if (!applyClipping) {
			// No clipping, so so set box bounding area for drawing full 4x4 pixel blocks
			boxBounds.top = 0;
			boxBounds.bottom = 3;
			boxBounds.left = 0;
		} else {
			// Handle any possible clipping at the top of the char block.
			// We already handled topClip partially at the beginning of this function.
			// Hence the only non-zero values it can assume at this point are 1,2,3,
			// and that only during the very first iteration (i.e. when the top char
			// block is drawn only partially). In particular, we set topClip to zero,
			// as all following blocks are not to be top clipped.
			boxBounds.top = pObj->topClip;
			pObj->topClip = 0;

			boxBounds.bottom = MIN(boxBounds.top + pObj->height - 1, 3);

			// Handle any possible clipping at the start of the line
			boxBounds.left = pObj->leftClip;
			if (boxBounds.left >= 4) {
				srcP += sizeof(uint16) * (boxBounds.left >> 2);
				width -= boxBounds.left & 0xfffc;
				boxBounds.left %= 4;
			}

			width -= boxBounds.left;
		}

		// Horizontal loop
		while (width > rightClip) {
			boxBounds.right = MIN(boxBounds.left + width - rightClip - 1, 3);
			assert(boxBounds.bottom >= boxBounds.top);
			assert(boxBounds.right >= boxBounds.left);

			int16 indexVal = READ_LE_UINT16(srcP);
			srcP += sizeof(uint16);

			if (indexVal >= 0) {
				// Draw a 4x4 block based on the opcode as in index into the block list
				const uint8 *p = (uint8 *)pObj->charBase + (indexVal << 4);
				p += boxBounds.top * sizeof(uint32);
				for (int yp = boxBounds.top; yp <= boxBounds.bottom; ++yp, p += sizeof(uint32)) {
					Common::copy(p + boxBounds.left, p + boxBounds.right + 1, tempDest + (SCREEN_WIDTH * (yp - boxBounds.top)));
				}

			} else {
				// Draw a 4x4 block with transparency support
				indexVal &= 0x7fff;

				// If index is zero, then skip drawing the block completely
				if (indexVal > 0) {
					// Use the index along with the object's translation offset
					const uint8 *p = (uint8 *)pObj->charBase + ((pObj->transOffset + indexVal) << 4);

					// Loop through each pixel - only draw a pixel if it's non-zero
					p += boxBounds.top * sizeof(uint32);
					for (int yp = boxBounds.top; yp <= boxBounds.bottom; ++yp) {
						p += boxBounds.left;
						for (int xp = boxBounds.left; xp <= boxBounds.right; ++xp, ++p) {
							if (*p)
								*(tempDest + SCREEN_WIDTH * (yp - boxBounds.top) + (xp - boxBounds.left)) = *p;
						}
						p += 3 - boxBounds.right;
					}
				}
			}

			tempDest += boxBounds.right - boxBounds.left + 1;
			width -= 3 - boxBounds.left + 1;
			
			// None of the remaining horizontal blocks should be left clipped
			boxBounds.left = 0;
		}

		// If there is any width remaining, there must be a right edge clipping
		if (width >= 0)
			srcP += sizeof(uint16) * ((width + 3) >> 2);

		// Move to next line line
		pObj->height -= boxBounds.bottom - boxBounds.top + 1;
		destP += (boxBounds.bottom - boxBounds.top + 1) * SCREEN_WIDTH;
	}
}

/**
 * Fill the destination area with a constant colour
 */

static void WrtConst(DRAWOBJECT *pObj, uint8 *destP, bool applyClipping) {
	if (applyClipping) {
		pObj->height -= pObj->topClip + pObj->botClip;
		pObj->width -= pObj->leftClip + pObj->rightClip;

		if (pObj->width <= 0)
			return;
	}

	// Loop through any remaining lines
	while (pObj->height > 0) {
		Common::set_to(destP, destP + pObj->width, pObj->constant);

		--pObj->height;
		destP += SCREEN_WIDTH;
	}
}

/**
 * Translates the destination surface within the object's bounds using the transparency
 * lookup table from transpal.cpp (the contents of which have been moved into palette.cpp)
 */

static void WrtTrans(DRAWOBJECT *pObj, uint8 *destP, bool applyClipping) {
	if (applyClipping) {
		pObj->height -= pObj->topClip + pObj->botClip;
		pObj->width -= pObj->leftClip + pObj->rightClip;

		if (pObj->width <= 0)
			return;
	}

	// Set up the offset between destination lines 
	int lineOffset = SCREEN_WIDTH - pObj->width;

	// Loop through any remaining lines
	while (pObj->height > 0) {
		for (int i = 0; i < pObj->width; ++i, ++destP)
			*destP = transPalette[*destP];

		--pObj->height;
		destP += lineOffset;
	}		
}


#if 0
// This commented out code is the untested original WrtNonZero/ClpWrtNonZero combo method
// from the v1 source. It may be needed to be included later on to support v1 gfx files

/**
 * Straight rendering with transparency support
 * Possibly only used in the Discworld Demo
 */

static void DemoWrtNonZero(DRAWOBJECT *pObj, uint8 *srcP, uint8 *destP, bool applyClipping) {
	// FIXME: If this method is used for the demo, it still needs to be made Endian safe

	// Set up the offset between destination lines
	pObj->lineoffset = SCREEN_WIDTH - pObj->width - (applyClipping ? pObj->leftClip - pObj->rightClip : 0);

	// Top clipped line handling
	while (applyClipping && (pObj->topClip > 0)) {
		// Loop through discarding the data for the line
		int width = pObj->width;
		while (width > 0) {
			int32 opcodeOrLen = (int32)READ_LE_UINT32(srcP);
			srcP += sizeof(uint32);

			if (opcodeOrLen >= 0) {
				// Dump the data
				srcP += ((opcodeOrLen + 3) / 4) * 4;
				width -= opcodeOrLen;
			} else {
				// Dump the run-length opcode
				width -= -opcodeOrLen;
			}
		}

		--pObj->height;
		--pObj->topClip;
	}

	// Loop for the required number of rows
	while (pObj->height > 0) {

		int width = pObj->width;
		
		// Handling for left edge clipping - this basically involves dumping data until we reach
		// the part of the line to be displayed
		int clipLeft = pObj->leftClip;
		while (applyClipping && (clipLeft > 0)) {
			int32 opcodeOrLen = (int32)READ_LE_UINT32(srcP);
			srcP += sizeof(uint32);

			if (opcodeOrLen >= 0) {
				// Copy a specified number of bytes
				// Make adjustments for past the clipping width
				int remainder = 4 - (opcodeOrLen % 4);
				srcP += MIN(clipLeft, opcodeOrLen);
				opcodeOrLen -= MIN(clipLeft, opcodeOrLen);
				clipLeft -= MIN(clipLeft, opcodeOrLen);
				width -= opcodeOrLen;
				

				// Handle any right edge clipping (if run length covers entire width)
				if (width < pObj->rightClip) {
					remainder += (pObj->rightClip - width);
					opcodeOrLen -= (pObj->rightClip - width);
				}
				
				if (opcodeOrLen > 0) 
					Common::copy(srcP, srcP + opcodeOrLen, destP);

			} else {
				// Output a run length number of bytes
				// Get data for byte value and run length
				opcodeOrLen = -opcodeOrLen;
				int runLength = opcodeOrLen & 0xff;
				uint8 colourVal = (opcodeOrLen >> 8) & 0xff;

				// Make adjustments for past the clipping width
				runLength -= MIN(clipLeft, runLength);
				clipLeft -= MIN(clipLeft, runLength);
				width -= runLength;

				// Handle any right edge clipping (if run length covers entire width)
				if (width < pObj->rightClip) 
					runLength -= (pObj->rightClip - width);

				if (runLength > 0) {
					// Displayable part starts partway through the slice
					if (colourVal != 0)
						Common::set_to(destP, destP + runLength, colourVal);
					destP += runLength;
				}
			}

			if (width < pObj->rightClip)
				width = 0;
		}

		// Handling for the visible part of the line
		int endWidth = applyClipping ? pObj->rightClip : 0;
		while (width > endWidth) {
			int32 opcodeOrLen = (int32)READ_LE_UINT32(srcP);
			srcP += sizeof(uint32);

			if (opcodeOrLen >= 0) {
				// Copy the specified number of bytes
				int remainder = 4 - (opcodeOrLen % 4);

				if (width < endWidth) {
					// Shorten run length by right clipping
					remainder += (pObj->rightClip - width);
					opcodeOrLen -= (pObj->rightClip - width);
				}

				Common::copy(srcP, srcP + opcodeOrLen, destP);
				srcP += opcodeOrLen + remainder;
				destP += opcodeOrLen;
				width -= opcodeOrLen;

			} else {
				// Handle a given run length
				opcodeOrLen = -opcodeOrLen;
				int runLength = opcodeOrLen & 0xff;
				uint8 colourVal = (opcodeOrLen >> 8) & 0xff;

				if (width < endWidth) 
					// Shorten run length by right clipping
					runLength -= (pObj->rightClip - width);

				// Only set pixels if colourVal non-zero (0 signifies transparency)
				if (colourVal != 0) 
					// Fill out a run length of a specified colour
					Common::set_to(destP, destP + runLength, colourVal);

				destP += runLength;
				width -= runLength;
			}
		}

		// If right edge clipping is being applied, then width may still be non-zero - in
		// that case all remaining line data until the end of the line must be ignored
		while (width > 0) {
			int32 opcodeOrLen = (int32)READ_LE_UINT32(srcP);
			srcP += sizeof(uint32);

			if (opcodeOrLen >= 0) {
				// Dump the data
				srcP += ((opcodeOrLen + 3) / 4) * 4;
				width -= opcodeOrLen;
			} else {
				// Dump the run-length opcode
				width -= -opcodeOrLen;
			}
		}

		--pObj->height;
		destP += pObj->lineoffset;
	}
}
#endif

//----------------- MAIN FUNCTIONS ---------------------

/**
 * Clears both the screen surface buffer and screen to the specified value
 */
void ClearScreen() {
	void *pDest = _vm->screen().getBasePtr(0, 0);
	memset(pDest, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
	g_system->clearScreen();
	g_system->updateScreen(); 
}

/**
 * Updates the screen surface within the following rectangle
 */
void UpdateScreenRect(const Common::Rect &pClip) {
	byte *pDest = (byte *)_vm->screen().getBasePtr(pClip.left, pClip.top);
	g_system->copyRectToScreen(pDest, _vm->screen().pitch, pClip.left, pClip.top, pClip.width(), pClip.height());
	g_system->updateScreen(); 
}

/**
 * Draws the specified object onto the screen surface buffer
 */
void DrawObject(DRAWOBJECT *pObj) {
	uint8 *srcPtr = NULL;
	uint8 *destPtr;

	if ((pObj->width <= 0) || (pObj->height <= 0)) 
		// Empty image, so return immediately
		return;

	// If writing constant data, don't bother locking the data pointer and reading src details
	if ((pObj->flags & DMA_CONST) == 0) {
		byte *p = (byte *)LockMem(pObj->hBits & 0xFF800000);
		
		srcPtr = p + (pObj->hBits & 0x7FFFFF);
		pObj->charBase = (char *)p + READ_LE_UINT32(p + 0x10);
		pObj->transOffset = READ_LE_UINT32(p + 0x14);
	}

	// Get destination starting point
	destPtr = (byte *)_vm->screen().getBasePtr(pObj->xPos, pObj->yPos);
	
	// Handle various draw types
	uint8 typeId = pObj->flags & 0xff;
	switch (typeId) {
	case 0x01:
	case 0x08:
	case 0x41:
	case 0x48:
		WrtNonZero(pObj, srcPtr, destPtr, typeId >= 0x40);
		break;

	case 0x04:
	case 0x44:
		// ClpWrtConst with/without clipping
		WrtConst(pObj,destPtr, typeId == 0x44);
		break;

	case 0x84:
	case 0xC4:
		// WrtTrans with/without clipping
		WrtTrans(pObj, destPtr, typeId == 0xC4);
		break;

	default:
		// NoOp
		error("Unknown drawing type %d", typeId);
		break;
	}
}

} // End of namespace Tinsel
