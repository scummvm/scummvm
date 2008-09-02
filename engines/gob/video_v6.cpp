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
 */

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/video.h"

namespace Gob {

Video_v6::Video_v6(GobEngine *vm) : Video_v2(vm) {
}

char Video_v6::spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
	    int16 x, int16 y, int16 transp, SurfaceDesc *destDesc) {
	if (!destDesc)
		return 1;

	_vm->validateVideoMode(destDesc->_vidMode);

	if (sprBuf[0] != 1)
		return 0;

	if (sprBuf[1] != 3)
		return 0;

	sprBuf += 2;

	srcWidth = READ_LE_UINT16(sprBuf);
	sprBuf += 2;
	srcHeight = READ_LE_UINT16(sprBuf);
	sprBuf += 2;

	if (sprBuf[0] == 0) {
		SurfaceDesc sourceDesc(0x13, srcWidth, srcHeight, sprBuf + 3);
		Video::drawSprite(&sourceDesc, destDesc, 0, 0, srcWidth - 1,
		    srcHeight - 1, x, y, transp);
		return 1;
	} else {
		warning("Urban Stub: spriteUncompressor()");
		return 0;
	}

	return 1;
}

} // End of namespace Gob
