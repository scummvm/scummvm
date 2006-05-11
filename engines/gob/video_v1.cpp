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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/video.h"

namespace Gob {

Video_v1::Video_v1(GobEngine *vm) : Video(vm) {
}

//XXX: Use this function to update the screen for now.
//     This should be moved to a better location later on.
void Video_v1::waitRetrace(int16) {
	if (_vm->_global->_pPrimarySurfDesc) {
		g_system->copyRectToScreen(_vm->_global->_pPrimarySurfDesc->vidPtr, 320, 0, 0, 320, 200);
		g_system->updateScreen();
	}
}

void Video_v1::drawLetter(int16 item, int16 x, int16 y, FontDesc *fontDesc, int16 color1,
	    int16 color2, int16 transp, SurfaceDesc *dest) {

	_videoDriver->drawLetter((unsigned char) item, x, y, fontDesc, color1, color2, transp, dest);
}

Video::SurfaceDesc *Video_v1::initSurfDesc(int16 vidMode, int16 width, int16 height, int16 flags) {
	int8 flagsAnd2;
	byte *vidMem = 0;
	int32 sprSize;
	int16 someFlags = 1;
	SurfaceDesc *descPtr;

	if (flags != PRIMARY_SURFACE)
		_vm->_global->_sprAllocated++;

	if (flags & RETURN_PRIMARY)
		return _vm->_global->_pPrimarySurfDesc;

	if (vidMode != 0x13)
		error("Video::initSurfDesc: Only VGA 0x13 mode is supported!");

	if ((flags & PRIMARY_SURFACE) == 0)
		vidMode += 0x80;

	if (flags & 2)
		flagsAnd2 = 1;
	else
		flagsAnd2 = 0;

	if (flags & PRIMARY_SURFACE) {
		_vm->_global->_primaryWidth = width;
		_vm->_global->_mouseMaxCol = width;
		_vm->_global->_primaryHeight = height;
		_vm->_global->_mouseMaxRow = height;
		sprSize = 0;
	} else {
		sprSize = Video::getRectSize(width, height, flagsAnd2, vidMode);
		if (flagsAnd2)
			someFlags += 0x80;
	}
	if (flags & PRIMARY_SURFACE) {
		descPtr = _vm->_global->_pPrimarySurfDesc;
		delete[] descPtr->vidPtr;
		assert(descPtr);
		vidMem = new byte[320 * 200];
	} else {
		if (flags & DISABLE_SPR_ALLOC) {
			descPtr = new SurfaceDesc;
			// this case causes vidPtr to be set to invalid memory
			assert(false);
		} else {
			descPtr = new SurfaceDesc;
			descPtr->vidPtr = new byte[sprSize];
			vidMem = descPtr->vidPtr;
		}
	}
	if (descPtr == 0)
		return 0;

	descPtr->width = width;
	descPtr->height = height;
	descPtr->flag = someFlags;
	descPtr->vidMode = vidMode;
	descPtr->vidPtr = vidMem;

	descPtr->reserved1 = 0;
	descPtr->reserved2 = 0;
	return descPtr;
}

char Video_v1::spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
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
		error("Video::spriteUncompressor: Video mode 0x%x is not supported!",
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
		Video::drawSprite(&sourceDesc, destDesc, 0, 0, srcWidth - 1,
		    srcHeight - 1, x, y, transp);
		return 1;
	} else {
		memBuffer = new byte[4114];
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
							delete[] memBuffer;
							return 1;
						}
					}
					sourceLeft--;
					if (sourceLeft == 0) {
						delete[] memBuffer;
						return 1;
					}
					memBuffer[bufPos] = temp;
					bufPos++;
					bufPos %= 4096;
				}
			}
		}
	}
	delete[] memBuffer;
	return 1;
}

} // End of namespace Gob
