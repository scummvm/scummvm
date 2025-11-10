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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "common/endian.h"
#include "common/savefile.h"

#include "graphics/blit.h"
#include "graphics/conversion.h"

#include "gob/dataio.h"
#include "gob/gob.h"
#include "gob/video.h"

namespace Gob {

Video_v6::Video_v6(GobEngine *vm) : Video_v2(vm), _highColorPackedSpriteFormat(2, 5, 5, 5, 0, 10, 5, 0, 0)
{
}

char Video_v6::spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
	    int16 x, int16 y, int16 transp, Surface &destDesc) {

	if ((sprBuf[0] == 1) && (sprBuf[1] == 3)) {
		drawPacked(sprBuf, x, y, destDesc);
		return 1;
	}

	if (srcWidth & 0xC000) {
		warning("Playtoons Stub: srcWidth & 0xC000 == %04X", srcWidth & 0xC000);
		srcWidth &= 0x3FFF;
	}

	if ((sprBuf[0] == 1) && (sprBuf[1] == 2)) {
		if (Video_v2::spriteUncompressor(sprBuf, srcWidth, srcHeight, x, y, transp, destDesc))
			return 1;

		Video::drawPacked(sprBuf, srcWidth, srcHeight, x, y, transp, destDesc);
		return 1;
	}

	warning("Urban Stub: spriteUncompressor(), sprBuf[0,1,2] = %d,%d,%d",
			sprBuf[0], sprBuf[1], sprBuf[2]);
	return 1;
}

#ifndef INT32_MAX
#define INT32_MAX              (2147483647)
#endif

void Video_v6::drawPacked(const byte *sprBuf, int16 x, int16 y, Surface &surfDesc) {
	const byte *data = sprBuf + 2;

	int16 width = READ_LE_UINT16(data);
	int16 height = READ_LE_UINT16(data + 2);
	data += 4;

	const byte *srcData = data;
	byte dataType = *srcData++;

	if (dataType == 0) {
		// Uncompressed YUV data
		drawYUVData(srcData, surfDesc, width, height, x, y);
	} else if (dataType == 1) {
		// Compressed YUV data
		warning("drawPacked: untested case, compressed YUV data");
		int32 uncompresedSize = 0;
		byte *uncompressedData = DataIO::unpack(srcData, INT32_MAX, uncompresedSize, 1);
		drawYUVData(uncompressedData, surfDesc, width, height, x, y);
		delete[] uncompressedData;
	} else if (dataType == 3) {
		// Compressed high-color RGB data
		int32 uncompresesSize = 0;
		byte *uncompressedData = DataIO::unpack(srcData, INT32_MAX, uncompresesSize, 1);
		Graphics::PixelFormat &format = _highColorPackedSpriteFormat;

		if (_vm->getPixelFormat().aBits() > 0) {
			// We need to force the transparent color 0 to be mapped to 0 in the target format
			// First we fill the destination area with 0, then we will cross-blit skipping pixels with value 0 in the source
			surfDesc.fillRectRaw(x, y, x + width - 1, y + height - 1, 0);
		}

		bool conversionOk = false;
		if (_vm->getPixelFormat().aBits() > 0) {
			conversionOk = Graphics::crossKeyBlit(surfDesc.getData(x, y), uncompressedData,
												  surfDesc.getWidth() * surfDesc.getBPP(), width * format.bytesPerPixel,
												  width, height,
												  _vm->getPixelFormat(), format, 0);
		} else {
			conversionOk = Graphics::crossBlit(surfDesc.getData(x, y), uncompressedData,
											   surfDesc.getWidth() * surfDesc.getBPP(), width * format.bytesPerPixel,
											   width, height,
											   _vm->getPixelFormat(), format);
		}

		if (!conversionOk)
			warning("drawPacked: error when cross-blitting from compressed RGB high-color data");

		delete[] uncompressedData;
	} else {
		warning("drawPacked: unknown compression type %d", dataType);
	}
}

void Video_v6::drawYUVData(const byte *srcData, Surface &destDesc,
		int16 width, int16 height, int16 x, int16 y) {

	int16 dataWidth = width;
	int16 dataHeight = height;

	if (dataWidth & 0xF)
		dataWidth = (dataWidth & 0xFFF0) + 16;
	if (dataHeight & 0xF)
		dataHeight = (dataHeight & 0xFFF0) + 16;

	const byte *dataY = srcData;
	const byte *dataU = dataY +  (dataWidth * dataHeight);
	const byte *dataV = dataU + ((dataWidth * dataHeight) >> 4);

	drawYUV(destDesc, x, y, dataWidth, dataHeight, width, height, dataY, dataU, dataV);

}

void Video_v6::drawYUV(Surface &destDesc, int16 x, int16 y,
		int16 dataWidth, int16 dataHeight, int16 width, int16 height,
		const byte *dataY, const byte *dataU, const byte *dataV) {

	const Graphics::PixelFormat &pixelFormat = _vm->getPixelFormat();

	if ((x + width - 1) >= destDesc.getWidth())
		width = destDesc.getWidth() - x;
	if ((y + height - 1) >= destDesc.getHeight())
		height = destDesc.getHeight() - y;

	Pixel dst = destDesc.get(x, y);

	for (int i = 0; i < height; i++) {
		Pixel dstRow = dst;

		int nextChromaLine = (i < ((height - 1) & ~3) ) ? dataWidth : 0;

		for (int j = 0; j < width; j++, ++dstRow) {

			int nextChromaColumn = (j < ((width - 1) & ~3)) ? 1 : 0;

			// Get (7bit) Y data. It is at full res, does not need to be interpolated
			byte dY = dataY[j] << 1;

			// do linear interpolation on chroma values (7bits)
			// to avoid blockiness
			byte dU0 = dataU[j >> 2];
			byte dV0 = dataV[j >> 2];

			byte dU1 = dataU[(j >> 2) + nextChromaColumn];
			byte dV1 = dataV[(j >> 2) + nextChromaColumn];

			byte dU2 = dataU[(j + nextChromaLine) >> 2];
			byte dV2 = dataV[(j + nextChromaLine) >> 2];

			byte dU3 = dataU[((j + nextChromaLine) >> 2) + nextChromaColumn];
			byte dV3 = dataV[((j + nextChromaLine) >> 2) + nextChromaColumn];

			byte tX = j & 3;
			byte tY = i & 3;
			byte invtX = 4 - tX;
			byte invtY = 4 - tY;

			int16 dUX1 = dU0 * invtX + dU1 * tX;
			int16 dUX2 = dU2 * invtX + dU3 * tX;
			byte dU = (dUX1 * invtY + dUX2 * tY) >> 3;

			int16 dVY1 = dV0 * invtX + dV1 * tX;
			int16 dVY2 = dV2 * invtX + dV3 * tX;
			byte dV = (dVY1 * invtY + dVY2 * tY) >> 3;

			byte r, g, b;
			Graphics::YUV2RGB(dY, dU, dV, r, g, b);

			if (dY != 0) {
				// Solid pixel
				uint32 c = pixelFormat.RGBToColor(r, g, b);

				// If the solid pixel's value is 0, we'll fudge it to 1
				dstRow.set((c == 0) ? 1 : c);
			} else
				// Transparent pixel, we'll use pixel value 0
				dstRow.set(0);

		}

		dst   += destDesc.getWidth();
		dataY += dataWidth;

		if ((i & 3) == 3) {
			// Next line of chroma data
			dataU += dataWidth >> 2;
			dataV += dataWidth >> 2;
		}
	}

}

} // End of namespace Gob
