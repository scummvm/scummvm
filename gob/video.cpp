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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/global.h"
#include "gob/video.h"
#include "gob/dataio.h"

#include "gob/driver_vga.h"

namespace Gob {

VideoDriver *_videoDriver;

	
/* NOT IMPLEMENTED */
	
//XXX: Use this function to update the screen for now.
//     This should be moved to a better location later on.
void vid_waitRetrace(int16) {
	if (pPrimarySurfDesc) {
		g_system->copyRectToScreen(pPrimarySurfDesc->vidPtr, 320, 0, 0, 320, 200);
		g_system->updateScreen();
	}
}
	
char vid_initDriver(int16 vidMode) {
	warning("STUB: vid_initDriver");

	// FIXME: Finish all this stuff :)
	_videoDriver = new VGAVideoDriver();
	
	return 1;
}

void vid_freeDriver() {
	delete _videoDriver;
	warning("STUB: vid_freeDriver");
}

int32 vid_getRectSize(int16 width, int16 height, int16 flag, int16 mode) {
	int32 size;

	if ((mode & 0x7f) != 0x13)
		warning
		    ("vid_getRectSize: Video mode %d is not fully supported!",
		    mode & 0x7f);
	switch (mode & 0x7f) {
	case 5:
	case 7:
		size = ((int32)((width + 3) / 4)) * height * (flag + 1);
		break;
	case 0x13:
		size = (int32)width *height;
		break;
	case 0x14:
	case 0x15:
	case 0x16:
		size = ((int32)((width + 3) / 4)) * height * 4;
		break;
	default:
		size = ((int32)((width + 7) / 8)) * height * (flag + 4);
		break;
	}
	return size;
}

SurfaceDesc *vid_initSurfDesc(int16 vidMode, int16 width, int16 height, int16 flags) {
	char flagsAnd2;
	byte *vidMem;
	int32 sprSize;
	int16 someFlags = 1;
	SurfaceDesc *descPtr;

	if (flags != PRIMARY_SURFACE)
		sprAllocated++;

	if (flags & RETURN_PRIMARY)
		return pPrimarySurfDesc;

	if (vidMode != 0x13)
		error("vid_initSurfDesc: Only VGA 0x13 mode is supported!");

	if ((flags & PRIMARY_SURFACE) == 0)
		vidMode += 0x80;

	if (flags & 2)
		flagsAnd2 = 1;
	else
		flagsAnd2 = 0;

	if (flags & PRIMARY_SURFACE) {
		vidMem = 0;
		primaryWidth = width;
		mouseMaxCol = width;
		primaryHeight = height;
		mouseMaxRow = height;
		sprSize = 0;

	} else {
		vidMem = 0;
		sprSize = vid_getRectSize(width, height, flagsAnd2, vidMode);
		if (flagsAnd2)
			someFlags += 0x80;
	}
	if (flags & PRIMARY_SURFACE) {
		descPtr = pPrimarySurfDesc;
		vidMem = (byte *)malloc(320 * 200);
	} else {
		if (flags & DISABLE_SPR_ALLOC)
			descPtr = (SurfaceDesc *)malloc(sizeof(SurfaceDesc));
		else
			descPtr = (SurfaceDesc *)malloc(sizeof(SurfaceDesc) + sprSize);
	}
	if (descPtr == 0)
		return 0;

	descPtr->width = width;
	descPtr->height = height;
	descPtr->flag = someFlags;
	descPtr->vidMode = vidMode;
	if (vidMem == 0)
		vidMem = ((byte *)descPtr) + sizeof(SurfaceDesc);
	descPtr->vidPtr = vidMem;

	descPtr->reserved1 = 0;
	descPtr->reserved2 = 0;
	return descPtr;
}

void vid_freeSurfDesc(SurfaceDesc * surfDesc) {
	sprAllocated--;
	if (surfDesc != pPrimarySurfDesc)
		free(surfDesc);
	else 
		free(surfDesc->vidPtr);
}

int16 vid_clampValue(int16 val, int16 max) {
	if (val >= max)
		val = max - 1;

	if (val < 0)
		val = 0;

	return val;
}

void vid_drawSprite(SurfaceDesc *source, SurfaceDesc *dest,
	    int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) {
	int16 temp;
	int16 destRight;
	int16 destBottom;

	if (doRangeClamp) {
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
		if (right < 0)
			return;
		if (bottom < 0)
			return;
		if (left >= source->width)
			return;
		if (top >= source->height)
			return;

		if (left < 0) {
			x -= left;
			left = 0;
		}
		if (top < 0) {
			y -= top;
			top = 0;
		}
		right = vid_clampValue(right, source->width);
		bottom = vid_clampValue(bottom, source->height);
		if (right - left >= source->width)
			right = left + source->width - 1;
		if (bottom - top >= source->height)
			bottom = top + source->height - 1;

		if (x < 0) {
			left -= x;
			x = 0;
		}
		if (y < 0) {
			top -= y;
			y = 0;
		}
		if (left > right)
			return;
		if (top > bottom)
			return;

		if (x >= dest->width)
			return;

		if (y >= dest->height)
			return;

		destRight = x + right - left;
		destBottom = y + bottom - top;
		if (destRight >= dest->width)
			right -= destRight - dest->width + 1;

		if (destBottom >= dest->height)
			bottom -= destBottom - dest->height + 1;
	}

//	pDrawSprite(source, dest, left, top, right, bottom, x, y, transp);
	_videoDriver->drawSprite(source, dest, left, top, right, bottom, x, y, transp);
}

void vid_fillRect(SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom,
	    int16 color) {
	int16 temp;

	if (doRangeClamp) {
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
		if (right < 0)
			return;
		if (bottom < 0)
			return;
		if (left >= dest->width)
			return;
		if (top >= dest->height)
			return;

		left = vid_clampValue(left, dest->width);
		top = vid_clampValue(top, dest->height);
		right = vid_clampValue(right, dest->width);
		bottom = vid_clampValue(bottom, dest->height);
	}

//	pFillRect(dest, left, top, right, bottom, color);
	_videoDriver->fillRect(dest, left, top, right, bottom, color);
}

void vid_drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1, int16 color) {
	if (x0 == x1 || y0 == y1) {
		vid_fillRect(dest, x0, y0, x1, y1, color);
		return;
	}

//	pDrawLine(dest, x0, y0, x1, y1, color);
	_videoDriver->drawLine(dest, x0, y0, x1, y1, color);
}

void vid_putPixel(int16 x, int16 y, int16 color, SurfaceDesc *dest) {
	if (x < 0 || y < 0 || x >= dest->width || y >= dest->height)
		return;

//	pPutPixel(x, y, color, dest);
	_videoDriver->putPixel(x, y, color, dest);
}

void vid_drawLetter(unsigned char item, int16 x, int16 y, FontDesc *fontDesc, int16 color1,
	    int16 color2, int16 transp, SurfaceDesc * dest) {

//	pDrawLetter(item, x, y, fontDesc, color1, color2, transp, dest);
	_videoDriver->drawLetter(item, x, y, fontDesc, color1, color2, transp, dest);
}

void vid_clearSurf(SurfaceDesc *dest) {
	vid_fillRect(dest, 0, 0, dest->width - 1, dest->height - 1, 0);
}

void vid_drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y,
	    int16 transp, SurfaceDesc *dest) {

	if (vid_spriteUncompressor(sprBuf, width, height, x, y, transp, dest))
		return;

	if ((dest->vidMode & 0x7f) != 0x13)
		error("vid_drawPackedSprite: Vide mode 0x%x is not fully supported!",
		    dest->vidMode & 0x7f);

//	pDrawPackedSprite(sprBuf, width, height, x, y, transp, dest);
	_videoDriver->drawPackedSprite(sprBuf, width, height, x, y, transp, dest);
}

void vid_setPalElem(int16 index, char red, char green, char blue, int16 unused,
	    int16 vidMode) {
	byte pal[4];

	redPalette[index] = red;
	greenPalette[index] = green;
	bluePalette[index] = blue;

	if (vidMode != 0x13)
		error("vid_setPalElem: Video mode 0x%x is not supported!",
		    vidMode);

	pal[0] = (red << 2) | (red >> 4);
	pal[1] = (green << 2) | (green >> 4);
	pal[2] = (blue << 2) | (blue >> 4);
	pal[3] = 0;
	g_system->setPalette(pal, index, 1);
}

void vid_setPalette(PalDesc *palDesc) {
	int16 i;
	byte pal[1024];
	int16 numcolors;

	if (videoMode != 0x13)
		error("vid_setPalette: Video mode 0x%x is not supported!",
		    videoMode);

	if (setAllPalette)
		numcolors = 256;
	else
		numcolors = 16;

	for (i = 0; i < numcolors; i++) {
		pal[i * 4 + 0] = (palDesc->vgaPal[i].red << 2) | (palDesc->vgaPal[i].red >> 4);
		pal[i * 4 + 1] = (palDesc->vgaPal[i].green << 2) | (palDesc->vgaPal[i].green >> 4);
		pal[i * 4 + 2] = (palDesc->vgaPal[i].blue << 2) | (palDesc->vgaPal[i].blue >> 4);
		pal[i * 4 + 3] = 0;
	}
	
	g_system->setPalette(pal, 0, numcolors);
}

void vid_setFullPalette(PalDesc *palDesc) {
	Color *colors;
	int16 i;
	byte pal[1024];

	if (setAllPalette) {
		colors = palDesc->vgaPal;
		for (i = 0; i < 256; i++) {
			redPalette[i] = colors[i].red;
			greenPalette[i] = colors[i].green;
			bluePalette[i] = colors[i].blue;
		}

		for (i = 0; i < 256; i++) {
			pal[i * 4 + 0] = (colors[i].red << 2) | (colors[i].red >> 4);
			pal[i * 4 + 1] = (colors[i].green << 2) | (colors[i].green >> 4);
			pal[i * 4 + 2] = (colors[i].blue << 2) | (colors[i].blue >> 4);
			pal[i * 4 + 3] = 0;
		}
		g_system->setPalette(pal, 0, 256);
	} else {
		vid_setPalette(palDesc);
	}
}

void vid_initPrimary(int16 mode) {
	int16 old;
	if (curPrimaryDesc) {
		vid_freeSurfDesc(curPrimaryDesc);
		vid_freeSurfDesc(allocatedPrimary);

		curPrimaryDesc = 0;
		allocatedPrimary = 0;
	}
	if (mode != 0x13 && mode != 3 && mode != -1)
		error("vid_initPrimary: Video mode 0x%x is not supported!",
		    mode);

	if (videoMode != 0x13)
		error("vid_initPrimary: Video mode 0x%x is not supported!",
		    mode);

	old = oldMode;
	if (mode == -1)
		mode = 3;

	oldMode = mode;
	if (mode != 3)
		vid_initDriver(mode);

	needDriverInit = 1;

	if (mode != 3) {
		vid_initSurfDesc(mode, 320, 200, PRIMARY_SURFACE);

		if (dontSetPalette)
			return;

		vid_setFullPalette(pPaletteDesc);
	}
}

char vid_spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
	    int16 x, int16 y, int16 transp, SurfaceDesc *destDesc) {
	SurfaceDesc sourceDesc;
	byte *memBuffer;
	byte *srcPtr;
	byte *destPtr;
	byte *linePtr;
	byte temp;
	uint16 sourceLeft;
	int16 curWidth;
	int16 curHeight;
	int16 offset;
	int16 counter2;
	uint16 cmdVar;
	int16 bufPos;
	int16 strLen;

	if (!destDesc)
		return 1;

	if ((destDesc->vidMode & 0x7f) != 0x13)
		error("vid_spriteUncompressor: Video mode 0x%x is not supported!",
		    destDesc->vidMode & 0x7f);

	if (sprBuf[0] != 1)
		return 0;

	if (sprBuf[1] != 2)
		return 0;

	if (sprBuf[2] == 2) {
		sourceDesc.width = srcWidth;
		sourceDesc.height = srcHeight;
		sourceDesc.vidMode = 0x93;
		sourceDesc.vidPtr = sprBuf + 3;
		vid_drawSprite(&sourceDesc, destDesc, 0, 0, srcWidth - 1,
		    srcHeight - 1, x, y, transp);
		return 1;
	} else {
		memBuffer = (byte *)malloc(4114);
		if (memBuffer == 0)
			return 0;

		srcPtr = sprBuf + 3;
		sourceLeft = READ_LE_UINT16(srcPtr);

		destPtr = destDesc->vidPtr + destDesc->width * y + x;

		curWidth = 0;
		curHeight = 0;

		linePtr = destPtr;
		srcPtr += 4;

		for (offset = 0; offset < 4078; offset++)
			memBuffer[offset] = 0x20;

		cmdVar = 0;
		bufPos = 4078;
		while (1) {
			cmdVar >>= 1;
			if ((cmdVar & 0x100) == 0) {
				cmdVar = *srcPtr | 0xff00;
				srcPtr++;
			}
			if ((cmdVar & 1) != 0) {
				temp = *srcPtr++;
				if (temp != 0 || transp == 0)
					*destPtr = temp;
				destPtr++;
				curWidth++;
				if (curWidth >= srcWidth) {
					curWidth = 0;
					linePtr += destDesc->width;
					destPtr = linePtr;
					curHeight++;
					if (curHeight >= srcHeight)
						break;
				}
				sourceLeft--;
				if (sourceLeft == 0)
					break;

				memBuffer[bufPos] = temp;
				bufPos++;
				bufPos %= 4096;
			} else {
				offset = *srcPtr;
				srcPtr++;
				offset |= (*srcPtr & 0xf0) << 4;
				strLen = (*srcPtr & 0x0f) + 3;
				srcPtr++;

				for (counter2 = 0; counter2 < strLen;
				    counter2++) {
					temp =
					    memBuffer[(offset +
						counter2) % 4096];
					if (temp != 0 || transp == 0)
						*destPtr = temp;
					destPtr++;

					curWidth++;
					if (curWidth >= srcWidth) {
						curWidth = 0;
						linePtr += destDesc->width;
						destPtr = linePtr;
						curHeight++;
						if (curHeight >= srcHeight) {
							free(memBuffer);
							return 1;
						}
					}
					sourceLeft--;
					if (sourceLeft == 0) {
						free(memBuffer);
						return 1;
					}
					memBuffer[bufPos] = temp;
					bufPos++;
					bufPos %= 4096;
				}
			}
		}
	}
	free(memBuffer);
	return 1;
}

void vid_setHandlers() {
	//pDrawPacked = &vid_spriteUncompressor;
	pFileHandler = 0;
	setAllPalette = 1;
}

}				// End of namespace Gob
