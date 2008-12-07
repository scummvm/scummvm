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
#include "tinsel/scene.h"
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
 * Straight rendering of uncompressed data
 */
static void t0WrtNonZero(DRAWOBJECT *pObj, uint8 *srcP, uint8 *destP, bool applyClipping) {
	int yClip = 0;

	if (applyClipping) {
		// Adjust the height down to skip any bottom clipping
		pObj->height -= pObj->botClip;
		yClip = pObj->topClip;
	}

	// Vertical loop
	for (int y = 0; y < pObj->height; ++y) {
		// Get the start of the next line output
		uint8 *tempDest = destP;

		int leftClip = applyClipping ? pObj->leftClip : 0;
		int rightClip = applyClipping ? pObj->rightClip : 0;
		
		// Horizontal loop
		for (int x = 0; x < pObj->width; ) {
			uint32 numBytes = READ_UINT32(srcP);
			srcP += sizeof(uint32);
			bool repeatFlag = (numBytes & 0x80000000L) != 0;
			numBytes &= 0x7fffffff;

			uint clipAmount = MIN((int)numBytes & 0xff, leftClip);
			leftClip -= clipAmount;
			x += clipAmount;

			if (repeatFlag) {
				// Repeat of a given colour
				uint8 colour = (numBytes >> 8) & 0xff;
				int runLength = (numBytes & 0xff) - clipAmount;

				int rptLength = MAX(MIN(runLength, pObj->width - rightClip - x), 0);
				if (yClip == 0) {
					if (colour != 0)
						memset(tempDest, colour, rptLength);
					tempDest += rptLength;
				}

				x += runLength;
			} else {
				// Copy a specified sequence length of pixels
				srcP += clipAmount;
				
				int runLength = numBytes - clipAmount;
				int rptLength = MAX(MIN(runLength, pObj->width - rightClip - x), 0);
				if (yClip == 0) {
					memmove(tempDest, srcP, rptLength);
					tempDest += rptLength;
				}

				int overflow = (numBytes % 4) == 0 ? 0 : 4 - (numBytes % 4);
				x += runLength;
				srcP += runLength + overflow;
			}
		}

		// Move to next line
		if (yClip > 0)
			--yClip;
		else
			destP += SCREEN_WIDTH;
	}
}

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
 * Tinsel 2 Straight rendering with transparency support
 */
static void t2WrtNonZero(DRAWOBJECT *pObj, uint8 *srcP, uint8 *destP, bool applyClipping, bool horizFlipped) {
	// Setup for correct clipping of object edges
	int yClip = applyClipping ? pObj->topClip : 0;
	if (applyClipping)
		pObj->height -= pObj->botClip;
	int numBytes;
	int clipAmount;

	for (int y = 0; y < pObj->height; ++y) {
		// Get the position to start writing out from
		uint8 *tempP = !horizFlipped ? destP :
			destP + (pObj->width - pObj->leftClip - pObj->rightClip) - 1;
		int leftClip = applyClipping ? pObj->leftClip : 0;
		int rightClip = applyClipping ? pObj->rightClip : 0;
		if (horizFlipped)
			SWAP(leftClip, rightClip);

		int x = 0;
		while (x < pObj->width) {
			// Get the next opcode
			numBytes = *srcP++;
			if (numBytes & 0x80) {
				// Run length following
				numBytes &= 0x7f;
				clipAmount = MIN(numBytes, leftClip);
				leftClip -= clipAmount;
				x+= clipAmount;

				int runLength = numBytes - clipAmount;
				uint8 colour = *srcP++;

				if ((yClip == 0) && (runLength > 0) && (colour != 0)) {
					runLength = MIN(runLength, pObj->width - rightClip - x);

					if (runLength > 0) {
						// Non-transparent run length
						colour += pObj->constant;
						if (horizFlipped)
							Common::set_to(tempP - runLength + 1, tempP + 1, colour);
						else
							Common::set_to(tempP, tempP + runLength, colour);
					}
				}

				if (horizFlipped)
					tempP -= runLength;
				else
					tempP += runLength;

				x += numBytes - clipAmount;

			} else {
				// Dump a length of pixels
				clipAmount = MIN(numBytes, leftClip);
				leftClip -= clipAmount;
				srcP += clipAmount;
				int runLength = numBytes - clipAmount;
				x += numBytes - runLength;

				for (int xp = 0; xp < runLength; ++xp) {
					if ((yClip > 0) || (x >= (pObj->width - rightClip)))
						++srcP;
					else if (horizFlipped)
						*tempP-- = pObj->constant + *srcP++;
					else
						*tempP++ = pObj->constant + *srcP++;
					++x;
				}
			}
		}
		assert(x == pObj->width);

		if (yClip > 0)
			--yClip;
		else
			destP += SCREEN_WIDTH;
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

/**
 * Copies an uncompressed block of data straight to the screen
 */
static void WrtAll(DRAWOBJECT *pObj, uint8 *srcP, uint8 *destP, bool applyClipping) {
	int objWidth = pObj->width;

	if (applyClipping) {
		srcP += (pObj->topClip * pObj->width) + pObj->leftClip;

		pObj->height -= pObj->topClip + pObj->botClip;
		pObj->width -= pObj->leftClip + pObj->rightClip;

		if (pObj->width <= 0)
			return;
	}

	for (int y = 0; y < pObj->height; ++y) {
		Common::copy(srcP, srcP + pObj->width, destP);
		srcP += objWidth;
		destP += SCREEN_WIDTH;
	}
}

/**
 * Renders a packed data stream with a variable sized palette
 */
static void PackedWrtNonZero(DRAWOBJECT *pObj, uint8 *srcP, uint8 *destP,
							 bool applyClipping, bool horizFlipped, int packingType) {
	uint8 numColours = 0;
	uint8 *colourTable = NULL;
	int topClip = 0;
	int xOffset = 0;
	int numBytes, colour;
	int v;

	if (applyClipping) {
		pObj->height -= pObj->botClip;
		topClip = pObj->topClip;
	}

	if (packingType == 3) {
		// Variable colours
		numColours = *srcP++;
		colourTable = srcP;
		srcP += numColours;
	}

	for (int y = 0; y < pObj->height; ++y) {
		// Get the position to start writing out from
		uint8 *tempP = !horizFlipped ? destP :
			destP + (pObj->width - pObj->leftClip - pObj->rightClip) - 1;
		int leftClip = applyClipping ? pObj->leftClip : 0;
		int rightClip = applyClipping ? pObj->rightClip : 0;
		if (horizFlipped)
			SWAP(leftClip, rightClip);
		bool eolFlag = false;

		// Get offset for first pixels in next line
		xOffset = *srcP++;

		int x = 0;
		while (x < pObj->width) {
			// Get next run size and colour to use
			for (;;) {
				if (xOffset > 0) {
					x += xOffset;

					// Reduce offset amount by any remaining left clipping
					v = MIN(xOffset, leftClip);
					xOffset -= v;
					leftClip -= v;

					if (horizFlipped) tempP -= xOffset; else tempP += xOffset;
					xOffset = 0;
				}

				v = *srcP++;
				numBytes = v & 0xf;	// No. bytes 1-15
				if (packingType == 3)
					colour = colourTable[v >> 4];
				else
					colour = pObj->baseCol + (v >> 4);

				if (numBytes != 0)
					break;

				numBytes = *srcP++;
				if (numBytes >= 16)
					break;

				xOffset = numBytes + v;
				if (xOffset == 0) {
					// End of line encountered
					eolFlag = true;
					break;
				}
			}

			if (eolFlag)
				break;

			// Apply clipping on byte sequence
			v = MIN(numBytes, leftClip);
			leftClip -= v;
			numBytes -= v;
			x += v;

			while (numBytes-- > 0) {
				if ((topClip == 0) && (x < (pObj->width - rightClip))) {
					*tempP = colour;
					if (horizFlipped) --tempP; else ++tempP;
				}
				++x;
			}
		}
		assert(x <= pObj->width);

		if (!eolFlag) {
			// Assert that the next bytes signal a line end
			assert((*srcP++ & 0xf) == 0);
			assert(*srcP++ == 0);
		}

		if (topClip > 0)
			--topClip;
		else
			destP += SCREEN_WIDTH;
	}
}

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
	int yOffset = (g_system->getHeight() - SCREEN_HEIGHT) / 2;
	byte *pSrc = (byte *)_vm->screen().getBasePtr(pClip.left, pClip.top);
	g_system->copyRectToScreen(pSrc, _vm->screen().pitch, pClip.left, pClip.top + yOffset,
		pClip.width(), pClip.height());
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
		if (TinselV2) {
			srcPtr  = (byte *)LockMem(pObj->hBits);
			pObj->charBase = NULL;
			pObj->transOffset = 0;
		} else {
			byte *p = (byte *)LockMem(pObj->hBits & HANDLEMASK);
			
			srcPtr = p + (pObj->hBits & OFFSETMASK);
			pObj->charBase = (char *)p + READ_LE_UINT32(p + 0x10);
			pObj->transOffset = READ_LE_UINT32(p + 0x14);
		}
	}

	// Get destination starting point
	destPtr = (byte *)_vm->screen().getBasePtr(pObj->xPos, pObj->yPos);
	
	// Handle various draw types
	uint8 typeId = pObj->flags & 0xff;

	if (TinselV2) {
		// Tinsel v2 decoders
		// Initial switch statement for the different bit packing types
		int packType = pObj->flags >> 14;

		if (packType == 0) {
			// No colour packing
			switch (typeId) {
			case 0x01:
			case 0x11:
			case 0x41:
			case 0x51:
			case 0x81:
			case 0xC1:
				t2WrtNonZero(pObj, srcPtr, destPtr, typeId >= 0x40, (typeId & 0x10) != 0);
				break;
			case 0x02:
			case 0x42:
				// This renderer called 'RlWrtAll', but is the same as t2WrtNonZero
				t2WrtNonZero(pObj, srcPtr, destPtr, typeId >= 0x40, false);
				break;
			case 0x04:
			case 0x44:
				// WrtConst with/without clipping
				WrtConst(pObj, destPtr, typeId == 0x44);
				break;
			case 0x08:
			case 0x48:
				WrtAll(pObj, srcPtr, destPtr, typeId >= 0x40);
				break;
			case 0x84:
			case 0xC4:
				// WrtTrans with/without clipping
				WrtTrans(pObj, destPtr, typeId == 0xC4);
				break;
			default:
				error("Unknown drawing type %d", typeId);
			}
		} else {
			// 1 = 16 from 240
			// 2 = 16 from 224
			// 3 = variable colour
			if (packType == 1) pObj->baseCol = 0xF0;
			else if (packType == 2) pObj->baseCol = 0xE0;

			PackedWrtNonZero(pObj, srcPtr, destPtr, (pObj->flags & DMA_CLIP) != 0,
				(pObj->flags & DMA_FLIPH), packType);
		}

	} else if (TinselV1) {
		// Tinsel v1 decoders
		switch (typeId) {
		case 0x01:
		case 0x08:
		case 0x41:
		case 0x48:
			WrtNonZero(pObj, srcPtr, destPtr, typeId >= 0x40);
			break;

		case 0x04:
		case 0x44:
			// WrtConst with/without clipping
			WrtConst(pObj,destPtr, typeId == 0x44);
			break;

		case 0x84:
		case 0xC4:
			// WrtTrans with/without clipping
			WrtTrans(pObj, destPtr, typeId == 0xC4);
			break;

		default:
			error("Unknown drawing type %d", typeId);
		}
	} else {
		// Tinsel v0 decoders
		switch (typeId) {
		case 0x01:
		case 0x41:
			t0WrtNonZero(pObj, srcPtr, destPtr, typeId >= 0x40);
			break;
		case 0x08:
		case 0x48:
			WrtAll(pObj, srcPtr, destPtr, typeId >= 0x40);
			break;
		case 0x84:
		case 0xC4:
			WrtTrans(pObj, destPtr, (typeId & 0x40) != 0);
			break;
		default:
			error("Unknown drawing type %d", typeId);
		}
	}
}

} // End of namespace Tinsel
