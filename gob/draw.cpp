/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/draw.h"
#include "gob/global.h"
#include "gob/video.h"
#include "gob/game.h"
#include "gob/util.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/video.h"
#include "gob/palanim.h"
#include "gob/cdrom.h"

namespace Gob {

Draw::Draw(GobEngine *vm) : _vm(vm) {
	fontIndex = 0;
	spriteLeft = 0;
	spriteTop = 0;
	spriteRight = 0;
	spriteBottom = 0;
	destSpriteX = 0;
	destSpriteY = 0;
	backColor = 0;
	frontColor = 0;
	letterToPrint = 0;

	destSurface = 0;
	sourceSurface = 0;
	renderFlags = 0;
	backDeltaX = 0;
	backDeltaY = 0;
	
	for (int i = 0; i < 4; i++)
		fonts[i] = 0;
	
	textToPrint = 0;
	transparency = 0;

	for (int i = 0; i < 50; i++)
		spritesArray[i] = 0;

	invalidatedCount = 0;
	for (int i = 0; i < 30; i++) {
		invalidatedTops[i] = 0;
		invalidatedLefts[i] = 0;
		invalidatedRights[i] = 0;
		invalidatedBottoms[i] = 0;
	}

	noInvalidated = 0;
	applyPal = 0;
	paletteCleared = 0;

	backSurface = 0;
	frontSurface = 0;

	for (int i = 0; i < 18; i++)
		unusedPalette1[i] = 0;
	for (int i = 0; i < 16; i++)
		unusedPalette2[i] = 0;
	for (int i = 0; i < 256; i++) {
		vgaPalette[i].red = 0;
		vgaPalette[i].blue = 0;
		vgaPalette[i].green = 0;
	}
	for (int i = 0; i < 16; i++) {
		vgaSmallPalette[i].red = 0;
		vgaSmallPalette[i].blue = 0;
		vgaSmallPalette[i].green = 0;
	}

	cursorX = 0;
	cursorY = 0;
	cursorWidth = 0;
	cursorHeight = 0;

	cursorXDeltaVar = -1;
	cursorYDeltaVar = -1;

	for (int i = 0; i < 40; i++) {
		cursorAnimLow[i] = 0;
		cursorAnimHigh[i] = 0;
		cursorAnimDelays[i] = 0;
	}

	gcursorIndex = 0;
	transparentCursor = 0;
	cursorSprites = 0;
	cursorBack = 0;
	cursorAnim = 0;

	palLoadData1[0] = 0;
	palLoadData1[1] = 17;
	palLoadData1[2] = 34;
	palLoadData1[3] = 51;
	palLoadData2[0] = 0;
	palLoadData2[1] = 68;
	palLoadData2[2] = 136;
	palLoadData2[3] = 204;

	cursorTimeKey = 0;
}

void Draw::invalidateRect(int16 left, int16 top, int16 right, int16 bottom) {
	int16 temp;
	int16 rect;
	int16 i;

	if (renderFlags & RENDERFLAG_NOINVALIDATE)
		return;

	if (left > right) {
		temp = left;
		left = right;
		right = temp;
	}
	if (top > bottom) {
		temp = top;
		top = bottom;
		bottom = temp;
	}

	if (left > 319 || right < 0 || top > 199 || bottom < 0)
		return;

	noInvalidated = 0;

	if (invalidatedCount >= 30) {
		invalidatedLefts[0] = 0;
		invalidatedTops[0] = 0;
		invalidatedRights[0] = 319;
		invalidatedBottoms[0] = 199;
		invalidatedCount = 1;
		return;
	}

	if (left < 0)
		left = 0;

	if (right > 319)
		right = 319;

	if (top < 0)
		top = 0;

	if (bottom > 199)
		bottom = 199;

	left &= 0xfff0;
	right |= 0x000f;

	for (rect = 0; rect < invalidatedCount; rect++) {

		if (invalidatedTops[rect] > top) {
			if (invalidatedTops[rect] > bottom) {
				for (i = invalidatedCount; i > rect; i--) {
					invalidatedLefts[i] =
					    invalidatedLefts[i - 1];
					invalidatedTops[i] =
					    invalidatedTops[i - 1];
					invalidatedRights[i] =
					    invalidatedRights[i - 1];
					invalidatedBottoms[i] =
					    invalidatedBottoms[i - 1];
				}
				invalidatedLefts[rect] = left;
				invalidatedTops[rect] = top;
				invalidatedRights[rect] = right;
				invalidatedBottoms[rect] = bottom;
				invalidatedCount++;
				return;
			}
			if (invalidatedBottoms[rect] < bottom)
				invalidatedBottoms[rect] = bottom;

			if (invalidatedLefts[rect] > left)
				invalidatedLefts[rect] = left;

			if (invalidatedRights[rect] < right)
				invalidatedRights[rect] = right;

			invalidatedTops[rect] = top;
			return;
		}

		if (invalidatedBottoms[rect] < top)
			continue;

		if (invalidatedBottoms[rect] < bottom)
			invalidatedBottoms[rect] = bottom;

		if (invalidatedLefts[rect] > left)
			invalidatedLefts[rect] = left;

		if (invalidatedRights[rect] < right)
			invalidatedRights[rect] = right;

		return;
	}

	invalidatedLefts[invalidatedCount] = left;
	invalidatedTops[invalidatedCount] = top;
	invalidatedRights[invalidatedCount] = right;
	invalidatedBottoms[invalidatedCount] = bottom;
	invalidatedCount++;
	return;
}

void Draw::blitInvalidated(void) {
	int16 i;

	if (gcursorIndex == 4)
		blitCursor();

	if (_vm->_inter->_terminate)
		return;

	if (noInvalidated && applyPal == 0)
		return;

	if (noInvalidated) {
		setPalette();
		applyPal = 0;
		return;
	}

	if (applyPal) {
		clearPalette();

		_vm->_video->drawSprite(backSurface, frontSurface, 0, 0, 319,
		    199, 0, 0, 0);
		setPalette();
		invalidatedCount = 0;
		noInvalidated = 1;
		applyPal = 0;
		return;
	}

	_vm->_global->_doRangeClamp = 0;
	for (i = 0; i < invalidatedCount; i++) {
		_vm->_video->drawSprite(backSurface, frontSurface,
		    invalidatedLefts[i], invalidatedTops[i],
		    invalidatedRights[i], invalidatedBottoms[i],
		    invalidatedLefts[i], invalidatedTops[i], 0);
	}
	_vm->_global->_doRangeClamp = 1;

	invalidatedCount = 0;
	noInvalidated = 1;
	applyPal = 0;
}

void Draw::setPalette(void) {
	if (_vm->_global->_videoMode != 0x13)
		error("setPalette: Video mode 0x%x is not supported!\n",
		    _vm->_global->_videoMode);

	_vm->_global->_pPaletteDesc->unused1 = unusedPalette1;
	_vm->_global->_pPaletteDesc->unused2 = unusedPalette2;
	_vm->_global->_pPaletteDesc->vgaPal = vgaPalette;
	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	paletteCleared = 0;
}

void Draw::clearPalette(void) {
	if (paletteCleared == 0) {
		paletteCleared = 1;
		_vm->_util->clearPalette();
	}
}

void Draw::blitCursor(void) {
	if (gcursorIndex == -1)
		return;

	gcursorIndex = -1;
	if (cursorX + cursorWidth > 320)
		cursorWidth = 320 - cursorX;

	if (cursorY + cursorHeight > 200)
		cursorHeight = 200 - cursorY;

	if (noInvalidated) {
		_vm->_video->drawSprite(backSurface, frontSurface,
		    cursorX, cursorY,
		    cursorX + cursorWidth - 1,
		    cursorY + cursorHeight - 1, cursorX,
		    cursorY, 0);
	} else {
		invalidateRect(cursorX, cursorY,
		    cursorX + cursorWidth - 1,
		    cursorY + cursorHeight - 1);
	}
}

void Draw::spriteOperation(int16 operation) {
	uint16 id;
	char *dataBuf;
	Game::TotResItem *itemPtr;
	int32 offset;
	int16 len;
	int16 i;
	int16 x;
	int16 y;
	int16 perLine;

	if (sourceSurface >= 100)
		sourceSurface -= 80;

	if (destSurface >= 100)
		destSurface -= 80;

	if (renderFlags & RENDERFLAG_USEDELTAS) {
		if (sourceSurface == 21) {
			spriteLeft += backDeltaX;
			spriteTop += backDeltaY;
		}

		if (destSurface == 21) {
			destSpriteX += backDeltaX;
			destSpriteY += backDeltaY;
			if (operation == DRAW_DRAWLINE ||
			    (operation >= DRAW_DRAWBAR
				&& operation <= DRAW_FILLRECTABS)) {
				spriteRight += backDeltaX;
				spriteBottom += backDeltaY;
			}
		}
	}

	switch (operation) {
	case DRAW_BLITSURF:
		_vm->_video->drawSprite(spritesArray[sourceSurface],
		    spritesArray[destSurface],
		    spriteLeft, spriteTop,
		    spriteLeft + spriteRight - 1,
		    spriteTop + spriteBottom - 1,
		    destSpriteX, destSpriteY, transparency);

		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    destSpriteX + spriteRight - 1,
			    destSpriteY + spriteBottom - 1);
		}
		break;

	case DRAW_PUTPIXEL:
		_vm->_video->putPixel(destSpriteX, destSpriteY,
		    frontColor, spritesArray[destSurface]);
		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    destSpriteX, destSpriteY);
		}
		break;

	case DRAW_FILLRECT:
		_vm->_video->fillRect(spritesArray[destSurface],
		    destSpriteX, destSpriteY,
		    destSpriteX + spriteRight - 1,
		    destSpriteY + spriteBottom - 1, backColor);

		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    destSpriteX + spriteRight - 1,
			    destSpriteY + spriteBottom - 1);
		}
		break;

	case DRAW_DRAWLINE:
		_vm->_video->drawLine(spritesArray[destSurface],
		    destSpriteX, destSpriteY,
		    spriteRight, spriteBottom, frontColor);

		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    spriteRight, spriteBottom);
		}
		break;

	case DRAW_INVALIDATE:
		if (destSurface == 21) {
			invalidateRect(destSpriteX - spriteRight, destSpriteY - spriteBottom,	// !!
			    destSpriteX + spriteRight,
			    destSpriteY + spriteBottom);
		}
		break;

	case DRAW_LOADSPRITE:
		id = spriteLeft;
		if (id >= 30000) {
			dataBuf =
			    _vm->_game->loadExtData(id, &spriteRight,
			    &spriteBottom);
			_vm->_video->drawPackedSprite((byte *)dataBuf, spriteRight,
			    spriteBottom, destSpriteX,
			    destSpriteY, transparency,
			    spritesArray[destSurface]);
			if (destSurface == 21) {
				invalidateRect(destSpriteX,
				    destSpriteY,
				    destSpriteX + spriteRight - 1,
				    destSpriteY + spriteBottom - 1);
			}
			free(dataBuf);
			break;
		}
		// Load from .TOT resources
		itemPtr = &_vm->_game->totResourceTable->items[id];
		offset = itemPtr->offset;
		if (offset >= 0) {
			dataBuf =
			    ((char *)_vm->_game->totResourceTable) +
			    szGame_TotResTable + szGame_TotResItem *
			    _vm->_game->totResourceTable->itemsCount + offset;
		} else {
			dataBuf =
			    _vm->_game->imFileData +
			    (int32)READ_LE_UINT32(&((int32 *)_vm->_game->imFileData)[-offset - 1]);
		}

		spriteRight = itemPtr->width;
		spriteBottom = itemPtr->height;
		_vm->_video->drawPackedSprite((byte *)dataBuf,
		    spriteRight, spriteBottom,
		    destSpriteX, destSpriteY,
		    transparency, spritesArray[destSurface]);

		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    destSpriteX + spriteRight - 1,
			    destSpriteY + spriteBottom - 1);
		}
		break;

	case DRAW_PRINTTEXT:
		len = strlen(textToPrint);
		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    destSpriteX +
			    len * fonts[fontIndex]->itemWidth - 1,
			    destSpriteY +
			    fonts[fontIndex]->itemHeight - 1);
		}

		for (i = 0; i < len; i++) {
			_vm->_video->drawLetter(textToPrint[i],
			    destSpriteX, destSpriteY,
			    fonts[fontIndex],
			    transparency,
			    frontColor, backColor,
			    spritesArray[destSurface]);

			destSpriteX += fonts[fontIndex]->itemWidth;
		}
		break;

	case DRAW_DRAWBAR:
		_vm->_video->drawLine(spritesArray[destSurface],
		    destSpriteX, spriteBottom,
		    spriteRight, spriteBottom, frontColor);

		_vm->_video->drawLine(spritesArray[destSurface],
		    destSpriteX, destSpriteY,
		    destSpriteX, spriteBottom, frontColor);

		_vm->_video->drawLine(spritesArray[destSurface],
		    spriteRight, destSpriteY,
		    spriteRight, spriteBottom, frontColor);

		_vm->_video->drawLine(spritesArray[destSurface],
		    destSpriteX, destSpriteY,
		    spriteRight, destSpriteY, frontColor);

		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    spriteRight, spriteBottom);
		}
		break;

	case DRAW_CLEARRECT:
		if (backColor < 16) {
			_vm->_video->fillRect(spritesArray[destSurface],
			    destSpriteX, destSpriteY,
			    spriteRight, spriteBottom,
			    backColor);
		}
		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    spriteRight, spriteBottom);
		}
		break;

	case DRAW_FILLRECTABS:
		_vm->_video->fillRect(spritesArray[destSurface],
		    destSpriteX, destSpriteY,
		    spriteRight, spriteBottom, backColor);

		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    spriteRight, spriteBottom);
		}
		break;

	case DRAW_DRAWLETTER:
		if (fontToSprite[fontIndex].sprite == -1) {
			if (destSurface == 21) {
				invalidateRect(destSpriteX,
				    destSpriteY,
				    destSpriteX +
				    fonts[fontIndex]->itemWidth - 1,
				    destSpriteY +
				    fonts[fontIndex]->itemHeight -
				    1);
			}
			_vm->_video->drawLetter(letterToPrint,
			    destSpriteX, destSpriteY,
			    fonts[fontIndex],
			    transparency,
			    frontColor, backColor,
			    spritesArray[destSurface]);
			break;
		}

		perLine =
		    spritesArray[(int16)fontToSprite[fontIndex].
		    sprite]->width / fontToSprite[fontIndex].width;

		y = (letterToPrint -
		    fontToSprite[fontIndex].base) / perLine *
		    fontToSprite[fontIndex].height;

		x = (letterToPrint -
		    fontToSprite[fontIndex].base) % perLine *
		    fontToSprite[fontIndex].width;

		if (destSurface == 21) {
			invalidateRect(destSpriteX, destSpriteY,
			    destSpriteX +
			    fontToSprite[fontIndex].width,
			    destSpriteY +
			    fontToSprite[fontIndex].height);
		}

		_vm->_video->drawSprite(spritesArray[(int16)fontToSprite
			[fontIndex].sprite],
		    spritesArray[destSurface], x, y,
		    x + fontToSprite[fontIndex].width,
		    y + fontToSprite[fontIndex].height,
		    destSpriteX, destSpriteY, transparency);

		break;
	}

	if (renderFlags & RENDERFLAG_USEDELTAS) {
		if (sourceSurface == 21) {
			spriteLeft -= backDeltaX;
			spriteTop -= backDeltaY;
		}

		if (destSurface == 21) {
			destSpriteX -= backDeltaX;
			destSpriteY -= backDeltaY;
		}
	}
}

void Draw::animateCursor(int16 cursor) {
	int16 newX = 0;
	int16 newY = 0;
	Game::Collision *ptr;
	int16 minX;
	int16 minY;
	int16 maxX;
	int16 maxY;
	int16 cursorIndex;

	cursorIndex = cursor;

	if (cursorIndex == -1) {
		cursorIndex = 0;
		for (ptr = _vm->_game->collisionAreas; ptr->left != -1; ptr++) {
			if (ptr->flags & 0xfff0)
				continue;

			if (ptr->left > _vm->_global->_inter_mouseX)
				continue;

			if (ptr->right < _vm->_global->_inter_mouseX)
				continue;

			if (ptr->top > _vm->_global->_inter_mouseY)
				continue;

			if (ptr->bottom < _vm->_global->_inter_mouseY)
				continue;

			if ((ptr->flags & 0xf) < 3)
				cursorIndex = 1;
			else
				cursorIndex = 3;
			break;
		}
		if (cursorAnimLow[cursorIndex] == -1)
			cursorIndex = 1;
	}

	if (cursorAnimLow[cursorIndex] != -1) {
		if (cursorIndex == gcursorIndex) {
			if (cursorAnimDelays[gcursorIndex] != 0 &&
			    cursorAnimDelays[gcursorIndex] * 10 +
			    cursorTimeKey <= _vm->_util->getTimeKey()) {
				cursorAnim++;
				cursorTimeKey = _vm->_util->getTimeKey();
			} else {
/*				if (noInvalidated &&
					inter_mouseX == cursorX &&	inter_mouseY == cursorY)
						return;*/
			}
		} else {
			gcursorIndex = cursorIndex;
			if (cursorAnimDelays[gcursorIndex] != 0) {
				cursorAnim =
				    cursorAnimLow[gcursorIndex];
				cursorTimeKey = _vm->_util->getTimeKey();
			} else {
				cursorAnim = gcursorIndex;
			}
		}

		if (cursorAnimDelays[gcursorIndex] != 0 &&
		    (cursorAnimHigh[gcursorIndex] < cursorAnim ||
			cursorAnimLow[gcursorIndex] >
			cursorAnim)) {
			cursorAnim = cursorAnimLow[gcursorIndex];
		}

		newX = _vm->_global->_inter_mouseX;
		newY = _vm->_global->_inter_mouseY;
		if (cursorXDeltaVar != -1) {
			newX -= (uint16)VAR_OFFSET(gcursorIndex * 4 + (cursorXDeltaVar / 4) * 4);
			newY -= (uint16)VAR_OFFSET(gcursorIndex * 4 + (cursorYDeltaVar / 4) * 4);
		}

		minX = MIN(newX, cursorX);
		minY = MIN(newY, cursorY);
		maxX = MAX(cursorX, newX) + cursorWidth - 1;
		maxY = MAX(cursorY, newY) + cursorHeight - 1;
		_vm->_video->drawSprite(backSurface, cursorBack,
		    newX, newY, newX + cursorWidth - 1,
		    newY + cursorHeight - 1, 0, 0, 0);

		_vm->_video->drawSprite(cursorSprites, backSurface,
		    cursorWidth * cursorAnim, 0,
		    cursorWidth * (cursorAnim + 1) - 1,
		    cursorHeight - 1, newX, newY, transparentCursor);

		if (noInvalidated == 0) {
			cursorIndex = gcursorIndex;
			gcursorIndex = -1;
			blitInvalidated();
			gcursorIndex = cursorIndex;
		} else {
			_vm->_video->waitRetrace(_vm->_global->_videoMode);
		}

		_vm->_video->drawSprite(backSurface, frontSurface,
		    minX, minY, maxX, maxY, minX, minY, 0);

		_vm->_video->drawSprite(cursorBack, backSurface,
		    0, 0, cursorWidth - 1, cursorHeight - 1,
		    newX, newY, 0);
	} else {
		blitCursor();
	}

	cursorX = newX;
	cursorY = newY;
}

void Draw::interPalLoad(void) {
	int16 i;
	int16 ind1;
	int16 ind2;
	byte cmd;
	char *palPtr;

	cmd = *_vm->_global->_inter_execPtr++;
	applyPal = 0;
	if (cmd & 0x80)
		cmd &= 0x7f;
	else
		applyPal = 1;

	if (cmd == 49) {
		warning("inter_palLoad: cmd == 49 is not supported");
		//var_B = 1;
		for (i = 0; i < 18; i++, _vm->_global->_inter_execPtr++) {
			if (i < 2) {
				if (applyPal == 0)
					continue;

				unusedPalette1[i] = *_vm->_global->_inter_execPtr;
				continue;
			}
			//if (*inter_execPtr != 0)
			//      var_B = 0;

			ind1 = *_vm->_global->_inter_execPtr >> 4;
			ind2 = (*_vm->_global->_inter_execPtr & 0xf);

			unusedPalette1[i] =
			    ((palLoadData1[ind1] + palLoadData2[ind2]) << 8) +
			    (palLoadData2[ind1] + palLoadData1[ind2]);
		}

		_vm->_global->_pPaletteDesc->unused1 = unusedPalette1;
	}

	switch (cmd) {
	case 52:
		for (i = 0; i < 16; i++, _vm->_global->_inter_execPtr += 3) {
			vgaSmallPalette[i].red = _vm->_global->_inter_execPtr[0];
			vgaSmallPalette[i].green = _vm->_global->_inter_execPtr[1];
			vgaSmallPalette[i].blue = _vm->_global->_inter_execPtr[2];
		}
		break;

	case 50:
		for (i = 0; i < 16; i++, _vm->_global->_inter_execPtr++)
			unusedPalette2[i] = *_vm->_global->_inter_execPtr;
		break;

	case 53:
		palPtr = _vm->_game->loadTotResource(_vm->_inter->load16());
		memcpy((char *)vgaPalette, palPtr, 768);
		break;

	case 54:
		memset((char *)vgaPalette, 0, 768);
		break;
	}
	if (!applyPal) {
		_vm->_global->_pPaletteDesc->unused2 = unusedPalette2;
		_vm->_global->_pPaletteDesc->unused1 = unusedPalette1;

		if (_vm->_global->_videoMode != 0x13)
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *)vgaSmallPalette;
		else
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *)vgaPalette;

		_vm->_palanim->fade((Video::PalDesc *) _vm->_global->_pPaletteDesc, 0, 0);
	}
}

void Draw::printText(void) {
	int16 savedFlags;
	int16 ldestSpriteX;
	char *dataPtr;
	char *ptr;
	char *ptr2;
	int16 index;
	int16 destX;
	int16 destY;
	char cmd;
	int16 val;
	char buf[20];

	index = _vm->_inter->load16();

	_vm->_cdrom->playMultMusic();

	dataPtr = (char *)_vm->_game->totTextData + _vm->_game->totTextData->items[index].offset;
	ptr = dataPtr;

	if (renderFlags & RENDERFLAG_CAPTUREPUSH) {
		destSpriteX = READ_LE_UINT16(ptr);
		destSpriteY = READ_LE_UINT16(ptr + 2);
		spriteRight = READ_LE_UINT16(ptr + 4) - destSpriteX + 1;
		spriteBottom = READ_LE_UINT16(ptr + 6) - destSpriteY + 1;
		_vm->_game->capturePush(destSpriteX, destSpriteY,
						 spriteRight, spriteBottom);
		(*_vm->_scenery->pCaptureCounter)++;
	}
	destSpriteX = READ_LE_UINT16(ptr);
	destX = destSpriteX;

	destSpriteY = READ_LE_UINT16(ptr + 2);
	destY = destSpriteY;

	spriteRight = READ_LE_UINT16(ptr + 4);
	spriteBottom = READ_LE_UINT16(ptr + 6);
	destSurface = 21;

	ptr += 8;

	backColor = *ptr++;
	transparency = 1;
	spriteOperation(DRAW_CLEARRECT);

	backColor = 0;
	savedFlags = renderFlags;

	renderFlags &= ~RENDERFLAG_NOINVALIDATE;
	for (; (destSpriteX = READ_LE_UINT16(ptr)) != -1; ptr++) {
		destSpriteX += destX;
		destSpriteY = READ_LE_UINT16(ptr + 2) + destY;
		spriteRight = READ_LE_UINT16(ptr + 4) + destX;
		spriteBottom = READ_LE_UINT16(ptr + 6) + destY;
		ptr += 8;

		cmd = (*ptr & 0xf0) >> 4;
		if (cmd == 0) {
			frontColor = *ptr & 0xf;
			spriteOperation(DRAW_DRAWLINE);
		} else if (cmd == 1) {
			frontColor = *ptr & 0xf;
			spriteOperation(DRAW_DRAWBAR);
		} else if (cmd == 2) {
			backColor = *ptr & 0xf;
			spriteOperation(DRAW_FILLRECTABS);
		}
	}
	ptr += 2;

	for (ptr2 = ptr; *ptr2 != 1; ptr2++) {
		if (*ptr2 == 3)
			ptr2++;

		if (*ptr2 == 2)
			ptr2 += 4;
	}

	ptr2++;

	while (*ptr != 1) {
		cmd = *ptr;
		if (cmd == 3) {
			ptr++;
			fontIndex = (*ptr & 0xf0) >> 4;
			frontColor = *ptr & 0xf;
			ptr++;
			continue;
		} else if (cmd == 2) {
			ptr++;
			destSpriteX = destX + READ_LE_UINT16(ptr);
			destSpriteY = destY + READ_LE_UINT16(ptr + 2);
			ptr += 4;
			continue;
		}

		if ((byte)*ptr != 0xba) {
			letterToPrint = *ptr;
			spriteOperation(DRAW_DRAWLETTER);
			destSpriteX +=
			    fonts[fontIndex]->itemWidth;
			ptr++;
		} else {
			cmd = ptr2[17] & 0x7f;
			if (cmd == 0) {
				val = READ_LE_UINT16(ptr2 + 18) * 4;
				sprintf(buf, "%d",  VAR_OFFSET(val));
			} else if (cmd == 1) {
				val = READ_LE_UINT16(ptr2 + 18) * 4;

				strcpy(buf, _vm->_global->_inter_variables + val);
			} else {
				val = READ_LE_UINT16(ptr2 + 18) * 4;

				sprintf(buf, "%d",  VAR_OFFSET(val));
				if (buf[0] == '-') {
					while (strlen(buf) - 1 < (uint32)ptr2[17]) {
						_vm->_util->insertStr("0", buf, 1);
					}
				} else {
					while (strlen(buf) - 1 < (uint32)ptr2[17]) {
						_vm->_util->insertStr("0", buf, 0);
					}
				}

				_vm->_util->insertStr(",", buf, strlen(buf) + 1 - ptr2[17]);
			}

			textToPrint = buf;
			ldestSpriteX = destSpriteX;
			spriteOperation(DRAW_PRINTTEXT);
			if (ptr2[17] & 0x80) {
				if (ptr[1] == ' ') {
					destSpriteX += fonts[fontIndex]->itemWidth;
					while (ptr[1] == ' ')
						ptr++;
					if (ptr[1] == 2) {
						if (READ_LE_UINT16(ptr + 4) == destSpriteY)
							ptr += 5;
					}
				} else if (ptr[1] == 2 && READ_LE_UINT16(ptr + 4) == destSpriteY) {
					ptr += 5;
					destSpriteX += fonts[fontIndex]->itemWidth;
				}
			} else {
				destSpriteX = ldestSpriteX + fonts[fontIndex]->itemWidth;
			}
			ptr2 += 23;
			ptr++;
		}
	}

	renderFlags = savedFlags;
	if (renderFlags & 4) {
		warning("printText: Input not supported!");
//              xor     ax, ax
//              loc_436_1391:
//              xor     dx, dx
//              push    ax
//              push    dx
//              push    ax
//              push    dx
//              push    ax
//              mov     al, 0
//              push    ax
//              call    sub_9FF_1E71
//              add     sp, 0Ch
	}

	if ((renderFlags & RENDERFLAG_CAPTUREPOP) && *_vm->_scenery->pCaptureCounter != 0) {
		(*_vm->_scenery->pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
}

}				// End of namespace Gob
