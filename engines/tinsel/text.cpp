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
 * Text utilities.
 */

#include "tinsel/dw.h"
#include "tinsel/graphics.h"	// object plotting
#include "tinsel/handle.h"
#include "tinsel/sched.h"	// process scheduler defines
#include "tinsel/strres.h"	// g_bMultiByte
#include "tinsel/text.h"	// text defines

namespace Tinsel {


//----------------- LOCAL GLOBAL DATA --------------------
/** TinselV3, base color for the text color replacement */
static uint32 g_t3fontBaseColor;

/**
 * Returns the length of one line of a string in pixels.
 * @param szStr			String
 * @param pFont			Which font to use for dimensions
 */
int StringLengthPix(char *szStr, const FONT *pFont) {
	int strLen;	// accumulated length of string
	byte	c;
	SCNHANDLE	hImg;

	// while not end of string or end of line
	for (strLen = 0; (c = *szStr) != EOS_CHAR && c != LF_CHAR; szStr++) {
		if (g_bMultiByte) {
			if (c & 0x80)
				c = ((c & ~0x80) << 8) + *++szStr;
		}
		hImg = pFont->fontDef[c];

		if (hImg) {
			// there is a IMAGE for this character
			const IMAGE *pChar = _vm->_handle->GetImage(hImg);

			// add width of font bitmap
			strLen += pChar->imgWidth;

			delete pChar;
		} else
			// use width of space character
			strLen += pFont->spaceSize;

		// finally add the inter-character spacing
		strLen += pFont->xSpacing;
	}

	// return length of line in pixels - minus inter-char spacing for last character
	strLen -= pFont->xSpacing;
	return (strLen > 0) ? strLen : 0;
}

/**
 * Returns the justified x start position of a line of text.
 * @param szStr			String to output
 * @param xPos			X position of string
 * @param pFont			Which font to use
 * @param mode			Mode flags for the string
 */
int JustifyText(char *szStr, int xPos, const FONT *pFont, int mode) {
	if (mode & TXT_CENTER) {
		// center justify the text

		// adjust x positioning by half the length of line in pixels
		xPos -= StringLengthPix(szStr, pFont) / 2;
	} else if (mode & TXT_RIGHT) {
		// right justify the text

		// adjust x positioning by length of line in pixels
		xPos -= StringLengthPix(szStr, pFont);
	}

	// return text line x start position
	return xPos;
}

/**
 * Main text outputting routine. If a object list is specified a
 * multi-object is created for the whole text and a pointer to the head
 * of the list is returned.
 * @param pList			Object list to add text to
 * @param szStr			String to output
 * @param color		Color for monochrome text
 * @param xPos			X position of string
 * @param yPos			Y position of string
 * @param hFont			Which font to use
 * @param mode			Mode flags for the string
 * @param sleepTime		Sleep time between each character (if non-zero)
 */
OBJECT *ObjectTextOut(OBJECT **pList, char *szStr, int color,
					  int xPos, int yPos, SCNHANDLE hFont, int mode, int sleepTime) {
	int xJustify;	// x position of text after justification
	int yOffset;	// offset to next line of text
	OBJECT *pFirst;	// head of multi-object text list
	OBJECT *pChar = 0;	// object ptr for the character
	byte c;
	SCNHANDLE hImg;

	// make sure there is a linked list to add text to
	assert(pList);

	// get font pointer
	FONT *pFont = _vm->_handle->GetFont(hFont);
	const OBJ_INIT *pFontInit = &pFont->fontInit;

	// init head of text list
	pFirst = nullptr;

	// get image for capital W
	SCNHANDLE imgHandle = pFont->fontDef[(int)'W'];
	assert(imgHandle);

	// get height of capital W for offset to next line
	const IMAGE *pImgCapitalW = _vm->_handle->GetImage(imgHandle);
	yOffset = pImgCapitalW->imgHeight & ~C16_FLAG_MASK;
	delete pImgCapitalW;

	while (*szStr) {
		// x justify the text according to the mode flags
		xJustify = JustifyText(szStr, xPos, pFont, mode);

		// repeat until end of string or end of line
		while ((c = *szStr) != EOS_CHAR && c != LF_CHAR) {
			if (g_bMultiByte) {
				if (c & 0x80)
					c = ((c & ~0x80) << 8) + *++szStr;
			}
			hImg = pFont->fontDef[c];

			if (hImg == 0) {
				// no image for this character

				// add font spacing for a space character
				xJustify += pFont->spaceSize;
			} else {	// printable character

				int aniX, aniY;		// char image animation offsets

				// allocate and init a character object
				if (pFirst == NULL)
					// first time - init head of list
					pFirst = pChar = InitObject(pFontInit);
				else
					// chain to multi-char list
					pChar = pChar->pSlave = InitObject(pFontInit);

				// convert image handle to pointer
				const IMAGE *pImg = _vm->_handle->GetImage(hImg);

				// fill in character object
				pChar->hImg   = hImg;			// image def
				pChar->width  = pImg->imgWidth;		// width of chars bitmap
				pChar->height = pImg->imgHeight & ~C16_FLAG_MASK;	// height of chars bitmap
				pChar->hBits  = pImg->hImgBits;		// bitmap

				// check for absolute positioning
				if (mode & TXT_ABSOLUTE)
					pChar->flags |= DMA_ABS;

				// set characters color - only effective for mono fonts
				pChar->constant = color;

				// set the base font color to be replaced with supplied color, only for Tinsel V3
				g_t3fontBaseColor = TinselV3 ? pFont->baseColor : 0;

				// get Y animation offset
				GetAniOffset(hImg, pChar->flags, &aniX, &aniY);

				// set x position - ignore animation point
				pChar->xPos = intToFrac(xJustify);

				// set y position - adjust for animation point
				pChar->yPos = intToFrac(yPos - aniY);

				if (mode & TXT_SHADOW) {
					// we want to shadow the character
					OBJECT *pShad;

					// allocate a object for the shadow and chain to multi-char list
					pShad = pChar->pSlave = AllocObject();

					// copy the character for a shadow
					CopyObject(pShad, pChar);

					// add shadow offsets to characters position
					pShad->xPos += intToFrac(pFont->xShadow);
					pShad->yPos += intToFrac(pFont->yShadow);

					// shadow is behind the character
					pShad->zPos--;

					// shadow is always mono
					pShad->flags = DMA_CNZ | DMA_CHANGED;

					// check for absolute positioning
					if (mode & TXT_ABSOLUTE)
						pShad->flags |= DMA_ABS;

					// shadow always uses first palette entry
					// should really alloc a palette here also ????
					pShad->constant = 1;

					// add shadow to object list
					InsertObject(pList, pShad);
				}

				// add character to object list
				InsertObject(pList, pChar);

				// move to end of list
				if (pChar->pSlave)
					pChar = pChar->pSlave;

				// add character spacing
				xJustify += pImg->imgWidth;

				delete pImg;
			}

			// finally add the inter-character spacing
			xJustify += pFont->xSpacing;

			// next character in string
			++szStr;
		}

		// adjust the text y position and add the inter-line spacing
		yPos += yOffset + pFont->ySpacing;

		// check for newline
		if (c == LF_CHAR)
			// next character in string
			++szStr;
	}

	delete pFont;

	// return head of list
	return pFirst;
}

/**
 * Is there an image for this character in this font?
 * @param hFont	which font to use
 * @param c		character to test
 */
bool IsCharImage(SCNHANDLE hFont, char c) {
	byte c2 = (byte)c;

	// Inventory save game name editor needs to be more clever for
	// multi-byte characters. This bodge will stop it erring.
	if (g_bMultiByte && (c2 & 0x80))
		return false;

	// get font pointer
	FONT *pFont = _vm->_handle->GetFont(hFont);
	bool result = pFont->fontDef[c2] != 0;
	delete pFont;

	return result;
}

uint32 t3GetBaseColor()
{
	return g_t3fontBaseColor;
}


} // End of namespace Tinsel
