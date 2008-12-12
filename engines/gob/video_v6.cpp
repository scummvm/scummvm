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
#include "gob/indeo3.h"

namespace Gob {

Video_v6::Video_v6(GobEngine *vm) : Video_v2(vm) {
}

char Video_v6::spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
	    int16 x, int16 y, int16 transp, SurfaceDesc *destDesc) {
	if (!destDesc)
		return 1;

	_vm->validateVideoMode(destDesc->_vidMode);

	if ((sprBuf[0] == 1) && (sprBuf[1] == 3)) {
		drawPacked(sprBuf, x, y, destDesc);
		return 1;
	}

	warning("Urban Stub: spriteUncompressor(), sprBuf[0,1] = %d,%d",
			sprBuf[0], sprBuf[1]);
	return 1;
}

void Video_v6::drawPacked(const byte *sprBuf, int16 x, int16 y, SurfaceDesc *surfDesc) {
	const byte *data = sprBuf + 2;

	int16 width = READ_LE_UINT16(data);
	int16 height = READ_LE_UINT16(data + 2);
	data += 4;

	warning("drawPacked: %d, %d, %d, %d", x, y, width, height);

	const byte *srcData = data;
	byte *uncBuf = 0;

	if (*srcData++ != 0) {
		uint32 size = READ_LE_UINT32(data);

		uncBuf = new byte[size];

		//sub_4F020(data, buf);
		warning("Urban Stub: drawPacked: sub_4F020(data, uncBuf)");

		srcData = uncBuf;
	}

	drawYUVData(srcData, surfDesc, width, height, x, y);

	delete[] uncBuf;
}

void Video_v6::drawYUVData(const byte *srcData, SurfaceDesc *destDesc,
		int16 width, int16 height, int16 x, int16 y) {

	int16 dataWidth = width;
	int16 dataHeight = height;

	if (dataWidth & 0xF)
		dataWidth = (dataWidth & 0xFFF0) + 16;
	if (dataHeight & 0xF)
		dataHeight = (dataHeight & 0xFFF0) + 16;

	warning("drawYUVData: %d, %d, %d, %d, %d, %d", x, y, width, height, dataWidth, dataHeight);

	const byte *dataY = srcData;
	const byte *dataU = dataY +  (dataWidth * dataHeight);
	const byte *dataV = dataU + ((dataWidth * dataHeight) >> 4);

/*
	if (destDesc->field_14 == 1) {
		SurfaceDesc *tmpSurf = _vid_initSurfDesc(2, width, height, 0);

		sub_46126(tmpSurf, 0, 0, dataWidth, dataHeight, width, height, dataY, dataU, dataV);

		_vid_drawSprite(tmpSurf, destDesc, 0, 0, width - 1, height - 1, x, y, 0);

		_vid_freeSurfDesc(tmpSurf);

		return;
	}
*/

	drawYUV(destDesc, x, y, dataWidth, dataHeight, width, height, dataY, dataU, dataV);

}

void Video_v6::drawYUV(SurfaceDesc *destDesc, int16 x, int16 y,
		int16 dataWidth, int16 dataHeight, int16 width, int16 height,
		const byte *dataY, const byte *dataU, const byte *dataV) {

	warning("drawYUV: %dx%d->%d+%d (%dx%d) (%dx%d)", width, height, x, y, dataWidth, dataHeight,
			destDesc->getWidth(), destDesc->getHeight());

	byte *vidMem = destDesc->getVidMem() + y * destDesc->getWidth() + x;

	if ((x + width - 1) >= destDesc->getWidth())
		width = destDesc->getWidth() - x;
	if ((y + height - 1) >= destDesc->getHeight())
		height = destDesc->getHeight() - y;

	SierraLight *dither = new SierraLight(width, height, _palLUT);

	for (int i = 0; i < height; i++) {
		byte *dest = vidMem;
		const byte *srcY = dataY +  i       *  dataWidth;
		const byte *srcU = dataU + (i >> 2) * (dataWidth >> 2);
		const byte *srcV = dataV + (i >> 2) * (dataWidth >> 2);

		for (int j = 0; j < (width >> 2); j++, srcU++, srcV++) {
			for (int n = 0; n < 4; n++, dest++, srcY++) {
				byte dY = *srcY << 1, dU = *srcU << 1, dV = *srcV << 1;

				*dest = dither->dither(dY, dU, dV, j * 4 + n);
			}
		}

		dither->nextLine();
		vidMem += destDesc->getWidth();
	}
}	

} // End of namespace Gob
