/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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

int16 draw_fontIndex = 0;
int16 draw_spriteLeft = 0;
int16 draw_spriteTop = 0;
int16 draw_spriteRight = 0;
int16 draw_spriteBottom = 0;
int16 draw_destSpriteX = 0;
int16 draw_destSpriteY = 0;
int16 draw_backColor = 0;
int16 draw_frontColor = 0;
char draw_letterToPrint = 0;
Draw_FontToSprite draw_fontToSprite[4];
int16 draw_destSurface = 0;
int16 draw_sourceSurface = 0;
int16 draw_renderFlags = 0;
int16 draw_backDeltaX = 0;
int16 draw_backDeltaY = 0;
FontDesc *draw_fonts[4];
char *draw_textToPrint = 0;
int16 draw_transparency = 0;
SurfaceDesc *draw_spritesArray[50];

int16 draw_invalidatedCount;
int16 draw_invalidatedTops[30];
int16 draw_invalidatedLefts[30];
int16 draw_invalidatedRights[30];
int16 draw_invalidatedBottoms[30];

int8 draw_noInvalidated = 0;
int8 draw_applyPal = 0;
int8 draw_paletteCleared = 0;

SurfaceDesc *draw_backSurface = 0;
SurfaceDesc *draw_frontSurface = 0;

int16 draw_unusedPalette1[18];
int16 draw_unusedPalette2[16];
Color draw_vgaPalette[256];
Color draw_vgaSmallPalette[16];

int16 draw_cursorX = 0;
int16 draw_cursorY = 0;
int16 draw_cursorWidth = 0;
int16 draw_cursorHeight = 0;

int16 draw_cursorXDeltaVar = -1;
int16 draw_cursorYDeltaVar = -1;

int16 draw_cursorIndex = 0;
int16 draw_transparentCursor = 0;
SurfaceDesc *draw_cursorSprites = 0;
SurfaceDesc *draw_cursorBack = 0;
int16 draw_cursorAnim = 0;
int8 draw_cursorAnimLow[40];
int8 draw_cursorAnimHigh[40];
int8 draw_cursorAnimDelays[40];
static uint32 draw_cursorTimeKey = 0;

int16 draw_palLoadData1[] = { 0, 17, 34, 51 };
int16 draw_palLoadData2[] = { 0, 68, 136, 204 };

void draw_invalidateRect(int16 left, int16 top, int16 right, int16 bottom) {
	int16 temp;
	int16 rect;
	int16 i;

	if (draw_renderFlags & RENDERFLAG_NOINVALIDATE)
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

	draw_noInvalidated = 0;

	if (draw_invalidatedCount >= 30) {
		draw_invalidatedLefts[0] = 0;
		draw_invalidatedTops[0] = 0;
		draw_invalidatedRights[0] = 319;
		draw_invalidatedBottoms[0] = 199;
		draw_invalidatedCount = 1;
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

	for (rect = 0; rect < draw_invalidatedCount; rect++) {

		if (draw_invalidatedTops[rect] > top) {
			if (draw_invalidatedTops[rect] > bottom) {
				for (i = draw_invalidatedCount; i > rect; i--) {
					draw_invalidatedLefts[i] =
					    draw_invalidatedLefts[i - 1];
					draw_invalidatedTops[i] =
					    draw_invalidatedTops[i - 1];
					draw_invalidatedRights[i] =
					    draw_invalidatedRights[i - 1];
					draw_invalidatedBottoms[i] =
					    draw_invalidatedBottoms[i - 1];
				}
				draw_invalidatedLefts[rect] = left;
				draw_invalidatedTops[rect] = top;
				draw_invalidatedRights[rect] = right;
				draw_invalidatedBottoms[rect] = bottom;
				draw_invalidatedCount++;
				return;
			}
			if (draw_invalidatedBottoms[rect] < bottom)
				draw_invalidatedBottoms[rect] = bottom;

			if (draw_invalidatedLefts[rect] > left)
				draw_invalidatedLefts[rect] = left;

			if (draw_invalidatedRights[rect] < right)
				draw_invalidatedRights[rect] = right;

			draw_invalidatedTops[rect] = top;
			return;
		}

		if (draw_invalidatedBottoms[rect] < top)
			continue;

		if (draw_invalidatedBottoms[rect] < bottom)
			draw_invalidatedBottoms[rect] = bottom;

		if (draw_invalidatedLefts[rect] > left)
			draw_invalidatedLefts[rect] = left;

		if (draw_invalidatedRights[rect] < right)
			draw_invalidatedRights[rect] = right;

		return;
	}

	draw_invalidatedLefts[draw_invalidatedCount] = left;
	draw_invalidatedTops[draw_invalidatedCount] = top;
	draw_invalidatedRights[draw_invalidatedCount] = right;
	draw_invalidatedBottoms[draw_invalidatedCount] = bottom;
	draw_invalidatedCount++;
	return;
}

void draw_blitInvalidated(void) {
	int16 i;

	if (draw_cursorIndex == 4)
		draw_blitCursor();

	if (inter_terminate)
		return;

	if (draw_noInvalidated && draw_applyPal == 0)
		return;

	if (draw_noInvalidated) {
		draw_setPalette();
		draw_applyPal = 0;
		return;
	}

	if (draw_applyPal) {
		draw_clearPalette();

		vid_drawSprite(draw_backSurface, draw_frontSurface, 0, 0, 319,
		    199, 0, 0, 0);
		draw_setPalette();
		draw_invalidatedCount = 0;
		draw_noInvalidated = 1;
		draw_applyPal = 0;
		return;
	}

	doRangeClamp = 0;
	for (i = 0; i < draw_invalidatedCount; i++) {
		vid_drawSprite(draw_backSurface, draw_frontSurface,
		    draw_invalidatedLefts[i], draw_invalidatedTops[i],
		    draw_invalidatedRights[i], draw_invalidatedBottoms[i],
		    draw_invalidatedLefts[i], draw_invalidatedTops[i], 0);
	}
	doRangeClamp = 1;

	draw_invalidatedCount = 0;
	draw_noInvalidated = 1;
	draw_applyPal = 0;
}

void draw_setPalette(void) {
	if (videoMode != 0x13)
		error("draw_setPalette: Video mode 0x%x is not supported!\n",
		    videoMode);

	pPaletteDesc->unused1 = draw_unusedPalette1;
	pPaletteDesc->unused2 = draw_unusedPalette2;
	pPaletteDesc->vgaPal = draw_vgaPalette;
	vid_setFullPalette(pPaletteDesc);
	draw_paletteCleared = 0;
}

void draw_clearPalette(void) {
	if (draw_paletteCleared == 0) {
		draw_paletteCleared = 1;
		util_clearPalette();
	}
}

void draw_blitCursor(void) {
	if (draw_cursorIndex == -1)
		return;

	draw_cursorIndex = -1;
	if (draw_cursorX + draw_cursorWidth > 320)
		draw_cursorWidth = 320 - draw_cursorX;

	if (draw_cursorY + draw_cursorHeight > 200)
		draw_cursorHeight = 200 - draw_cursorY;

	if (draw_noInvalidated) {
		vid_drawSprite(draw_backSurface, draw_frontSurface,
		    draw_cursorX, draw_cursorY,
		    draw_cursorX + draw_cursorWidth - 1,
		    draw_cursorY + draw_cursorHeight - 1, draw_cursorX,
		    draw_cursorY, 0);
	} else {
		draw_invalidateRect(draw_cursorX, draw_cursorY,
		    draw_cursorX + draw_cursorWidth - 1,
		    draw_cursorY + draw_cursorHeight - 1);
	}
}

void draw_spriteOperation(int16 operation) {
	uint16 id;
	char *dataBuf;
	Game_TotResItem *itemPtr;
	int32 offset;
	int16 len;
	int16 i;
	int16 x;
	int16 y;
	int16 perLine;

	if (draw_sourceSurface >= 100)
		draw_sourceSurface -= 80;

	if (draw_destSurface >= 100)
		draw_destSurface -= 80;

	if (draw_renderFlags & RENDERFLAG_USEDELTAS) {
		if (draw_sourceSurface == 21) {
			draw_spriteLeft += draw_backDeltaX;
			draw_spriteTop += draw_backDeltaY;
		}

		if (draw_destSurface == 21) {
			draw_destSpriteX += draw_backDeltaX;
			draw_destSpriteY += draw_backDeltaY;
			if (operation == DRAW_DRAWLINE ||
			    (operation >= DRAW_DRAWBAR
				&& operation <= DRAW_FILLRECTABS)) {
				draw_spriteRight += draw_backDeltaX;
				draw_spriteBottom += draw_backDeltaY;
			}
		}
	}

	switch (operation) {
	case DRAW_BLITSURF:
		vid_drawSprite(draw_spritesArray[draw_sourceSurface],
		    draw_spritesArray[draw_destSurface],
		    draw_spriteLeft, draw_spriteTop,
		    draw_spriteLeft + draw_spriteRight - 1,
		    draw_spriteTop + draw_spriteBottom - 1,
		    draw_destSpriteX, draw_destSpriteY, draw_transparency);

		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_destSpriteX + draw_spriteRight - 1,
			    draw_destSpriteY + draw_spriteBottom - 1);
		}
		break;

	case DRAW_PUTPIXEL:
		vid_putPixel(draw_destSpriteX, draw_destSpriteY,
		    draw_frontColor, draw_spritesArray[draw_destSurface]);
		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_destSpriteX, draw_destSpriteY);
		}
		break;

	case DRAW_FILLRECT:
		vid_fillRect(draw_spritesArray[draw_destSurface],
		    draw_destSpriteX, draw_destSpriteY,
		    draw_destSpriteX + draw_spriteRight - 1,
		    draw_destSpriteY + draw_spriteBottom - 1, draw_backColor);

		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_destSpriteX + draw_spriteRight - 1,
			    draw_destSpriteY + draw_spriteBottom - 1);
		}
		break;

	case DRAW_DRAWLINE:
		vid_drawLine(draw_spritesArray[draw_destSurface],
		    draw_destSpriteX, draw_destSpriteY,
		    draw_spriteRight, draw_spriteBottom, draw_frontColor);

		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_spriteRight, draw_spriteBottom);
		}
		break;

	case DRAW_INVALIDATE:
		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX - draw_spriteRight, draw_destSpriteY - draw_spriteBottom,	// !!
			    draw_destSpriteX + draw_spriteRight,
			    draw_destSpriteY + draw_spriteBottom);
		}
		break;

	case DRAW_LOADSPRITE:
		id = draw_spriteLeft;
		if (id >= 30000) {
			dataBuf =
			    game_loadExtData(id, &draw_spriteRight,
			    &draw_spriteBottom);
			vid_drawPackedSprite((byte *)dataBuf, draw_spriteRight,
			    draw_spriteBottom, draw_destSpriteX,
			    draw_destSpriteY, draw_transparency,
			    draw_spritesArray[draw_destSurface]);
			if (draw_destSurface == 21) {
				draw_invalidateRect(draw_destSpriteX,
				    draw_destSpriteY,
				    draw_destSpriteX + draw_spriteRight - 1,
				    draw_destSpriteY + draw_spriteBottom - 1);
			}
			free(dataBuf);
			break;
		}
		// Load from .TOT resources
		itemPtr = &game_totResourceTable->items[id];
		offset = itemPtr->offset;
		if (offset >= 0) {
			dataBuf =
			    ((char *)game_totResourceTable) +
			    szGame_TotResTable + szGame_TotResItem *
			    game_totResourceTable->itemsCount + offset;
		} else {
			dataBuf =
			    game_imFileData +
			    (int32)READ_LE_UINT32(&((int32 *)game_imFileData)[-offset - 1]);
		}

		draw_spriteRight = itemPtr->width;
		draw_spriteBottom = itemPtr->height;
		vid_drawPackedSprite((byte *)dataBuf,
		    draw_spriteRight, draw_spriteBottom,
		    draw_destSpriteX, draw_destSpriteY,
		    draw_transparency, draw_spritesArray[draw_destSurface]);

		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_destSpriteX + draw_spriteRight - 1,
			    draw_destSpriteY + draw_spriteBottom - 1);
		}
		break;

	case DRAW_PRINTTEXT:
		len = strlen(draw_textToPrint);
		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_destSpriteX +
			    len * draw_fonts[draw_fontIndex]->itemWidth - 1,
			    draw_destSpriteY +
			    draw_fonts[draw_fontIndex]->itemHeight - 1);
		}

		for (i = 0; i < len; i++) {
			vid_drawLetter(draw_textToPrint[i],
			    draw_destSpriteX, draw_destSpriteY,
			    draw_fonts[draw_fontIndex],
			    draw_transparency,
			    draw_frontColor, draw_backColor,
			    draw_spritesArray[draw_destSurface]);

			draw_destSpriteX += draw_fonts[draw_fontIndex]->itemWidth;
		}
		break;

	case DRAW_DRAWBAR:
		vid_drawLine(draw_spritesArray[draw_destSurface],
		    draw_destSpriteX, draw_spriteBottom,
		    draw_spriteRight, draw_spriteBottom, draw_frontColor);

		vid_drawLine(draw_spritesArray[draw_destSurface],
		    draw_destSpriteX, draw_destSpriteY,
		    draw_destSpriteX, draw_spriteBottom, draw_frontColor);

		vid_drawLine(draw_spritesArray[draw_destSurface],
		    draw_spriteRight, draw_destSpriteY,
		    draw_spriteRight, draw_spriteBottom, draw_frontColor);

		vid_drawLine(draw_spritesArray[draw_destSurface],
		    draw_destSpriteX, draw_destSpriteY,
		    draw_spriteRight, draw_destSpriteY, draw_frontColor);

		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_spriteRight, draw_spriteBottom);
		}
		break;

	case DRAW_CLEARRECT:
		if (draw_backColor < 16) {
			vid_fillRect(draw_spritesArray[draw_destSurface],
			    draw_destSpriteX, draw_destSpriteY,
			    draw_spriteRight, draw_spriteBottom,
			    draw_backColor);
		}
		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_spriteRight, draw_spriteBottom);
		}
		break;

	case DRAW_FILLRECTABS:
		vid_fillRect(draw_spritesArray[draw_destSurface],
		    draw_destSpriteX, draw_destSpriteY,
		    draw_spriteRight, draw_spriteBottom, draw_backColor);

		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_spriteRight, draw_spriteBottom);
		}
		break;

	case DRAW_DRAWLETTER:
		if (draw_fontToSprite[draw_fontIndex].sprite == -1) {
			if (draw_destSurface == 21) {
				draw_invalidateRect(draw_destSpriteX,
				    draw_destSpriteY,
				    draw_destSpriteX +
				    draw_fonts[draw_fontIndex]->itemWidth - 1,
				    draw_destSpriteY +
				    draw_fonts[draw_fontIndex]->itemHeight -
				    1);
			}
			vid_drawLetter(draw_letterToPrint,
			    draw_destSpriteX, draw_destSpriteY,
			    draw_fonts[draw_fontIndex],
			    draw_transparency,
			    draw_frontColor, draw_backColor,
			    draw_spritesArray[draw_destSurface]);
			break;
		}

		perLine =
		    draw_spritesArray[(int16)draw_fontToSprite[draw_fontIndex].
		    sprite]->width / draw_fontToSprite[draw_fontIndex].width;

		y = (draw_letterToPrint -
		    draw_fontToSprite[draw_fontIndex].base) / perLine *
		    draw_fontToSprite[draw_fontIndex].height;

		x = (draw_letterToPrint -
		    draw_fontToSprite[draw_fontIndex].base) % perLine *
		    draw_fontToSprite[draw_fontIndex].width;

		if (draw_destSurface == 21) {
			draw_invalidateRect(draw_destSpriteX, draw_destSpriteY,
			    draw_destSpriteX +
			    draw_fontToSprite[draw_fontIndex].width,
			    draw_destSpriteY +
			    draw_fontToSprite[draw_fontIndex].height);
		}

		vid_drawSprite(draw_spritesArray[(int16)draw_fontToSprite
			[draw_fontIndex].sprite],
		    draw_spritesArray[draw_destSurface], x, y,
		    x + draw_fontToSprite[draw_fontIndex].width,
		    y + draw_fontToSprite[draw_fontIndex].height,
		    draw_destSpriteX, draw_destSpriteY, draw_transparency);

		break;
	}

	if (draw_renderFlags & RENDERFLAG_USEDELTAS) {
		if (draw_sourceSurface == 21) {
			draw_spriteLeft -= draw_backDeltaX;
			draw_spriteTop -= draw_backDeltaY;
		}

		if (draw_destSurface == 21) {
			draw_destSpriteX -= draw_backDeltaX;
			draw_destSpriteY -= draw_backDeltaY;
		}
	}
}

void draw_animateCursor(int16 cursor) {
	int16 newX = 0;
	int16 newY = 0;
	Game_Collision *ptr;
	int16 minX;
	int16 minY;
	int16 maxX;
	int16 maxY;
	int16 cursorIndex;

	cursorIndex = cursor;

	if (cursorIndex == -1) {
		cursorIndex = 0;
		for (ptr = game_collisionAreas; ptr->left != -1; ptr++) {
			if (ptr->flags & 0xfff0)
				continue;

			if (ptr->left > inter_mouseX)
				continue;

			if (ptr->right < inter_mouseX)
				continue;

			if (ptr->top > inter_mouseY)
				continue;

			if (ptr->bottom < inter_mouseY)
				continue;

			if ((ptr->flags & 0xf) < 3)
				cursorIndex = 1;
			else
				cursorIndex = 3;
			break;
		}
		if (draw_cursorAnimLow[cursorIndex] == -1)
			cursorIndex = 1;
	}

	if (draw_cursorAnimLow[cursorIndex] != -1) {
		if (cursorIndex == draw_cursorIndex) {
			if (draw_cursorAnimDelays[draw_cursorIndex] != 0 &&
			    draw_cursorAnimDelays[draw_cursorIndex] * 10 +
			    draw_cursorTimeKey <= util_getTimeKey()) {
				draw_cursorAnim++;
				draw_cursorTimeKey = util_getTimeKey();
			} else {
/*				if (draw_noInvalidated &&
					inter_mouseX == draw_cursorX &&	inter_mouseY == draw_cursorY)
						return;*/
			}
		} else {
			draw_cursorIndex = cursorIndex;
			if (draw_cursorAnimDelays[draw_cursorIndex] != 0) {
				draw_cursorAnim =
				    draw_cursorAnimLow[draw_cursorIndex];
				draw_cursorTimeKey = util_getTimeKey();
			} else {
				draw_cursorAnim = draw_cursorIndex;
			}
		}

		if (draw_cursorAnimDelays[draw_cursorIndex] != 0 &&
		    (draw_cursorAnimHigh[draw_cursorIndex] < draw_cursorAnim ||
			draw_cursorAnimLow[draw_cursorIndex] >
			draw_cursorAnim)) {
			draw_cursorAnim = draw_cursorAnimLow[draw_cursorIndex];
		}

		newX = inter_mouseX;
		newY = inter_mouseY;
		if (draw_cursorXDeltaVar != -1) {
			newX -= (uint16)VAR_OFFSET(draw_cursorIndex * 4 + (draw_cursorXDeltaVar / 4) * 4);
			newY -= (uint16)VAR_OFFSET(draw_cursorIndex * 4 + (draw_cursorYDeltaVar / 4) * 4);
		}

		minX = MIN(newX, draw_cursorX);
		minY = MIN(newY, draw_cursorY);
		maxX = MAX(draw_cursorX, newX) + draw_cursorWidth - 1;
		maxY = MAX(draw_cursorY, newY) + draw_cursorHeight - 1;
		vid_drawSprite(draw_backSurface, draw_cursorBack,
		    newX, newY, newX + draw_cursorWidth - 1,
		    newY + draw_cursorHeight - 1, 0, 0, 0);

		vid_drawSprite(draw_cursorSprites, draw_backSurface,
		    draw_cursorWidth * draw_cursorAnim, 0,
		    draw_cursorWidth * (draw_cursorAnim + 1) - 1,
		    draw_cursorHeight - 1, newX, newY, draw_transparentCursor);

		if (draw_noInvalidated == 0) {
			cursorIndex = draw_cursorIndex;
			draw_cursorIndex = -1;
			draw_blitInvalidated();
			draw_cursorIndex = cursorIndex;
		} else {
			vid_waitRetrace(videoMode);
		}

		vid_drawSprite(draw_backSurface, draw_frontSurface,
		    minX, minY, maxX, maxY, minX, minY, 0);

		vid_drawSprite(draw_cursorBack, draw_backSurface,
		    0, 0, draw_cursorWidth - 1, draw_cursorHeight - 1,
		    newX, newY, 0);
	} else {
		draw_blitCursor();
	}

	draw_cursorX = newX;
	draw_cursorY = newY;
}

void draw_interPalLoad(void) {
	int16 i;
	int16 ind1;
	int16 ind2;
	byte cmd;
	char *palPtr;

	cmd = *inter_execPtr++;
	draw_applyPal = 0;
	if (cmd & 0x80)
		cmd &= 0x7f;
	else
		draw_applyPal = 1;

	if (cmd == 49) {
		warning("inter_palLoad: cmd == 49 is not supported");
		//var_B = 1;
		for (i = 0; i < 18; i++, inter_execPtr++) {
			if (i < 2) {
				if (draw_applyPal == 0)
					continue;

				draw_unusedPalette1[i] = *inter_execPtr;
				continue;
			}
			//if (*inter_execPtr != 0)
			//      var_B = 0;

			ind1 = *inter_execPtr >> 4;
			ind2 = (*inter_execPtr & 0xf);

			draw_unusedPalette1[i] =
			    ((draw_palLoadData1[ind1] + draw_palLoadData2[ind2]) << 8) +
			    (draw_palLoadData2[ind1] + draw_palLoadData1[ind2]);
		}

		pPaletteDesc->unused1 = draw_unusedPalette1;
	}

	switch (cmd) {
	case 52:
		for (i = 0; i < 16; i++, inter_execPtr += 3) {
			draw_vgaSmallPalette[i].red = inter_execPtr[0];
			draw_vgaSmallPalette[i].green = inter_execPtr[1];
			draw_vgaSmallPalette[i].blue = inter_execPtr[2];
		}
		break;

	case 50:
		for (i = 0; i < 16; i++, inter_execPtr++)
			draw_unusedPalette2[i] = *inter_execPtr;
		break;

	case 53:
		palPtr = game_loadTotResource(inter_load16());
		memcpy((char *)draw_vgaPalette, palPtr, 768);
		break;

	case 54:
		memset((char *)draw_vgaPalette, 0, 768);
		break;
	}
	if (!draw_applyPal) {
		pPaletteDesc->unused2 = draw_unusedPalette2;
		pPaletteDesc->unused1 = draw_unusedPalette1;

		if (videoMode != 0x13)
			pPaletteDesc->vgaPal = (Color *)draw_vgaSmallPalette;
		else
			pPaletteDesc->vgaPal = (Color *)draw_vgaPalette;

		pal_fade((PalDesc *) pPaletteDesc, 0, 0);
	}
}

void draw_printText(void) {
	int16 savedFlags;
	int16 destSpriteX;
	char *dataPtr;
	char *ptr;
	char *ptr2;
	int16 index;
	int16 destX;
	int16 destY;
	char cmd;
	int16 val;
	char buf[20];

	index = inter_load16();

	cd_playMultMusic();

	dataPtr = (char *)game_totTextData + game_totTextData->items[index].offset;
	ptr = dataPtr;

	if (draw_renderFlags & RENDERFLAG_CAPTUREPUSH) {
		draw_destSpriteX = READ_LE_UINT16(ptr);
		draw_destSpriteY = READ_LE_UINT16(ptr + 2);
		draw_spriteRight = READ_LE_UINT16(ptr + 4) - draw_destSpriteX + 1;
		draw_spriteBottom = READ_LE_UINT16(ptr + 6) - draw_destSpriteY + 1;
		game_capturePush(draw_destSpriteX, draw_destSpriteY,
						 draw_spriteRight, draw_spriteBottom);
		(*scen_pCaptureCounter)++;
	}
	draw_destSpriteX = READ_LE_UINT16(ptr);
	destX = draw_destSpriteX;

	draw_destSpriteY = READ_LE_UINT16(ptr + 2);
	destY = draw_destSpriteY;

	draw_spriteRight = READ_LE_UINT16(ptr + 4);
	draw_spriteBottom = READ_LE_UINT16(ptr + 6);
	draw_destSurface = 21;

	ptr += 8;

	draw_backColor = *ptr++;
	draw_transparency = 1;
	draw_spriteOperation(DRAW_CLEARRECT);

	draw_backColor = 0;
	savedFlags = draw_renderFlags;

	draw_renderFlags &= ~RENDERFLAG_NOINVALIDATE;
	for (; (draw_destSpriteX = READ_LE_UINT16(ptr)) != -1; ptr++) {
		draw_destSpriteX += destX;
		draw_destSpriteY = READ_LE_UINT16(ptr + 2) + destY;
		draw_spriteRight = READ_LE_UINT16(ptr + 4) + destX;
		draw_spriteBottom = READ_LE_UINT16(ptr + 6) + destY;
		ptr += 8;

		cmd = (*ptr & 0xf0) >> 4;
		if (cmd == 0) {
			draw_frontColor = *ptr & 0xf;
			draw_spriteOperation(DRAW_DRAWLINE);
		} else if (cmd == 1) {
			draw_frontColor = *ptr & 0xf;
			draw_spriteOperation(DRAW_DRAWBAR);
		} else if (cmd == 2) {
			draw_backColor = *ptr & 0xf;
			draw_spriteOperation(DRAW_FILLRECTABS);
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
			draw_fontIndex = (*ptr & 0xf0) >> 4;
			draw_frontColor = *ptr & 0xf;
			ptr++;
			continue;
		} else if (cmd == 2) {
			ptr++;
			draw_destSpriteX = destX + READ_LE_UINT16(ptr);
			draw_destSpriteY = destY + READ_LE_UINT16(ptr + 2);
			ptr += 4;
			continue;
		}

		if ((byte)*ptr != 0xba) {
			draw_letterToPrint = *ptr;
			draw_spriteOperation(DRAW_DRAWLETTER);
			draw_destSpriteX +=
			    draw_fonts[draw_fontIndex]->itemWidth;
			ptr++;
		} else {
			cmd = ptr2[17] & 0x7f;
			if (cmd == 0) {
				val = READ_LE_UINT16(ptr2 + 18) * 4;
				sprintf(buf, "%d",  VAR_OFFSET(val));
			} else if (cmd == 1) {
				val = READ_LE_UINT16(ptr2 + 18) * 4;

				strcpy(buf, inter_variables + val);
			} else {
				val = READ_LE_UINT16(ptr2 + 18) * 4;

				sprintf(buf, "%d",  VAR_OFFSET(val));
				if (buf[0] == '-') {
					while (strlen(buf) - 1 < (uint32)ptr2[17]) {
						util_insertStr("0", buf, 1);
					}
				} else {
					while (strlen(buf) - 1 < (uint32)ptr2[17]) {
						util_insertStr("0", buf, 0);
					}
				}

				util_insertStr(",", buf, strlen(buf) + 1 - ptr2[17]);
			}

			draw_textToPrint = buf;
			destSpriteX = draw_destSpriteX;
			draw_spriteOperation(DRAW_PRINTTEXT);
			if (ptr2[17] & 0x80) {
				if (ptr[1] == ' ') {
					draw_destSpriteX += draw_fonts[draw_fontIndex]->itemWidth;
					while (ptr[1] == ' ')
						ptr++;
					if (ptr[1] == 2) {
						if (READ_LE_UINT16(ptr + 4) == draw_destSpriteY)
							ptr += 5;
					}
				} else if (ptr[1] == 2 && READ_LE_UINT16(ptr + 4) == draw_destSpriteY) {
					ptr += 5;
					draw_destSpriteX += draw_fonts[draw_fontIndex]->itemWidth;
				}
			} else {
				draw_destSpriteX = destSpriteX + draw_fonts[draw_fontIndex]->itemWidth;
			}
			ptr2 += 23;
			ptr++;
		}
	}

	draw_renderFlags = savedFlags;
	if (draw_renderFlags & 4) {
		warning("draw_printText: Input not supported!");
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

	if ((draw_renderFlags & RENDERFLAG_CAPTUREPOP) && *scen_pCaptureCounter != 0) {
		(*scen_pCaptureCounter)--;
		game_capturePop(1);
	}
}

}				// End of namespace Gob
