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
#include "gob/draw.h"

namespace Gob {

Video_v2::Video_v2(GobEngine *vm) : Video_v1(vm) {
}

//XXX: Use this function to update the screen for now.
//     This should be moved to a better location later on.
void Video_v2::waitRetrace(int16) {
	if (_vm->_draw->_frontSurface) {
		g_system->copyRectToScreen(_vm->_draw->_frontSurface->vidPtr, 320, 0, 0, 320, 200);
		g_system->updateScreen();
	}
}

void Video_v2::drawLetter(int16 item, int16 x, int16 y, FontDesc *fontDesc, int16 color1,
	    int16 color2, int16 transp, SurfaceDesc *dest) {
	int16 videoMode;

	videoMode = dest->vidMode;

	// Is that needed at all? And what does it do anyway?
	char *dataPtr;
	int16 itemSize;
	int16 si; 
	int16 di;
	int16 dx;
	char *var_A;
	int16 var_10;
	if (fontDesc->endItem == 0) {
		itemSize = fontDesc->itemSize + 3;
		dataPtr = fontDesc->dataPtr;
		var_10 = dataPtr[-2] - 1;
		si = 0;
		do {
			di = ((si + var_10) / 2) * itemSize;
			var_A = fontDesc->dataPtr + di;
			dx = (READ_LE_UINT16(var_A) & 0x7FFF);
			if (item > dx)
				var_10 = di - 1;
			else
				si = di + 1;
		} while ((dx != item) && (si <= var_10));
		if (dx != item)
			return;
		fontDesc->dataPtr = var_A + 3;
		item = 0;
	}

	dest->vidMode &= 0x7F;
	_videoDriver->drawLetter((unsigned char) item, x, y, fontDesc, color1, color2, transp, dest);
	dest->vidMode = videoMode;
}

Video::SurfaceDesc *Video_v2::initSurfDesc(int16 vidMode, int16 width, int16 height, int16 flags) {
	int8 flagsAnd2;
	byte *vidMem = 0;
	int32 sprSize;
	int16 someFlags = 1;
	SurfaceDesc *descPtr;

	if (flags != PRIMARY_SURFACE)
		_vm->_global->_sprAllocated++;

	if (flags & RETURN_PRIMARY)
		return _vm->_draw->_frontSurface;

	if (vidMode != 0x13)
		error("Video::initSurfDesc: Only VGA 0x13 mode is supported!");

	if ((flags & PRIMARY_SURFACE) == 0)
		vidMode += 0x80;

	if (flags & 2)
		flagsAnd2 = 1;
	else
		flagsAnd2 = 0;

	width = (width + 7) & 0xFFF8;

	if (flags & PRIMARY_SURFACE) {
		_vm->_global->_primaryWidth = width;
		_vm->_global->_mouseMaxCol = width;
		_vm->_global->_primaryHeight = height;
		_vm->_global->_mouseMaxRow = height;
		sprSize = 0;
	} else {
		vidMem = 0;
		sprSize = Video::getRectSize(width, height, flagsAnd2, vidMode);
		someFlags = 4;
		if (flagsAnd2)
			someFlags += 0x80;
	}
	if (flags & PRIMARY_SURFACE) {
		descPtr = _vm->_draw->_frontSurface;
		assert(descPtr);
		if (descPtr->vidPtr != 0)
			delete[] descPtr->vidPtr;
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

} // End of namespace Gob
