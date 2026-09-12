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
 */

#include "made/graphics.h"

#include "common/array.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/util.h"

#include "graphics/surface.h"

namespace Made {

byte ValueReader::readPixel() {
	byte value;
	if (_nibbleMode) {
		if (_nibbleSwitch) {
			value = (_buffer[0] >> 4) & 0x0F;
			_buffer++;
		} else {
			value = _buffer[0] & 0x0F;
		}
		_nibbleSwitch = !_nibbleSwitch;
	} else {
		value = _buffer[0];
		_buffer++;
	}
	return value;
}

uint16 ValueReader::readUint16() {
	uint16 value = READ_LE_UINT16(_buffer);
	_buffer += 2;
	return value;
}

uint32 ValueReader::readUint32() {
	uint32 value = READ_LE_UINT32(_buffer);
	_buffer += 4;
	return value;
}

void ValueReader::resetNibbleSwitch() {
	_nibbleSwitch = false;
}

static uint rleDecompress(byte *source, const byte *end, Common::Array<byte> &dest) {
	while (source < end) {
		byte count = *source;
		source++;
		if (count < 0x80) {
			// copy
			for (count++; count > 0; count--) {
				dest.push_back(*source);
				source++;
			}
		} else {
			// repeat
			for (count = 257 - count; count > 0; count --)
				dest.push_back(*source);
			source++;
		}
	}

	if (source > end)
		warning("graphics::rleDecompress(): input stream over-read by %d bytes!", source - end);

	return dest.size();
}

void decompressImage(byte *source, Graphics::Surface &surface, uint16 cmdOffs, uint16 pixelOffs, uint16 maskOffs, uint16 lineSize, byte cmdFlags, byte pixelFlags, byte maskFlags, bool deltaFrame) {

	const int offsets[] = {
		0, 1, 2, 3,
		320, 321, 322, 323,
		640, 641, 642, 643,
		960, 961, 962, 963
	};

	uint16 pitch = surface.pitch;
	uint16 width = surface.w;
	uint16 height = surface.h;

	byte *cmdBuffer = source + cmdOffs;
	ValueReader maskReader(source + maskOffs, (maskFlags & 2) != 0);
	ValueReader pixelReader(source + pixelOffs, (pixelFlags & 2) != 0);

	if ((maskFlags != 0) && (maskFlags != 2) && (pixelFlags != 0) && (pixelFlags != 2) && (cmdFlags != 0))
		error("decompressImage() Unsupported flags: cmdFlags = %02X; maskFlags = %02X, pixelFlags = %02X", cmdFlags, maskFlags, pixelFlags);

	byte *destPtr = (byte *)surface.getPixels();

	byte lineBuf[640 * 4];
	byte bitBuf[40];

	int bitBufLastOfs = (((lineSize + 1) >> 1) << 1) - 2;
	int bitBufLastCount = ((width + 3) >> 2) & 7;
	if (bitBufLastCount == 0)
		bitBufLastCount = 8;

	while (height > 0) {

		int drawDestOfs = 0;

		memset(lineBuf, 0, sizeof(lineBuf));

		memcpy(bitBuf, cmdBuffer, lineSize);
		cmdBuffer += lineSize;

		for (uint16 bitBufOfs = 0; bitBufOfs < lineSize; bitBufOfs += 2) {

			uint16 bits = READ_LE_UINT16(&bitBuf[bitBufOfs]);

			int bitCount;
			if (bitBufOfs == bitBufLastOfs)
				bitCount = bitBufLastCount;
			else
				bitCount = 8;

			for (int curCmd = 0; curCmd < bitCount; curCmd++) {
				int cmd = bits & 3;
				bits >>= 2;

				byte pixels[4];
				uint32 mask;

				switch (cmd) {

				case 0:
					pixels[0] = pixelReader.readPixel();
					for (int i = 0; i < 16; i++)
						lineBuf[drawDestOfs + offsets[i]] = pixels[0];
					break;

				case 1:
					pixels[0] = pixelReader.readPixel();
					pixels[1] = pixelReader.readPixel();
					mask = maskReader.readUint16();
					for (int i = 0; i < 16; i++) {
						lineBuf[drawDestOfs + offsets[i]] = pixels[mask & 1];
						mask >>= 1;
					}
					break;

				case 2:
					pixels[0] = pixelReader.readPixel();
					pixels[1] = pixelReader.readPixel();
					pixels[2] = pixelReader.readPixel();
					pixels[3] = pixelReader.readPixel();
					mask = maskReader.readUint32();
					for (int i = 0; i < 16; i++) {
						lineBuf[drawDestOfs + offsets[i]] = pixels[mask & 3];
						mask >>= 2;
					}
					break;

				case 3:
					if (!deltaFrame) {
						// For EGA pictures: Pixels are read starting from a new byte
						maskReader.resetNibbleSwitch();
						// Yes, it reads from maskReader here
						for (int i = 0; i < 16; i++)
							lineBuf[drawDestOfs + offsets[i]] = maskReader.readPixel();
					}
					break;

				default:
					break;
				}

				drawDestOfs += 4;

			}

		}

		if (deltaFrame) {
			for (int y = 0; y < 4 && height > 0; y++, height--) {
				for (int x = 0; x < width; x++) {
					if (lineBuf[x + y * 320] != 0)
						*destPtr = lineBuf[x + y * 320];
					destPtr++;
				}
				destPtr += pitch - width;
			}
		} else {
			for (int y = 0; y < 4 && height > 0; y++, height--) {
				memcpy(destPtr, &lineBuf[y * 320], width);
				destPtr += pitch;
			}
		}

	}

}

void decompressMovieImage(byte *source, Graphics::Surface &surface, uint16 width, uint16 height,
						  uint16 cmdOffs, uint16 pixelOffs, uint16 maskOffs,
						  uint16 cmdSize, uint16 pixelSize, uint16 maskSize, uint16 lineSize,
						  byte cmdFlags, byte pixelFlags, byte maskFlags) {

	if ((maskFlags & ~1) || (pixelFlags & ~1) || (cmdFlags & ~1))
		error("decompressMovieImage() Unsupported flags: cmdFlags = %02X; maskFlags = %02X, pixelFlags = %02X", cmdFlags, maskFlags, pixelFlags);

	// RLE decompression buffers
	//  Reserved sizes for pixelArray and maskArray are guesses based on input params
	Common::Array<byte> cmdArray;
	Common::Array<byte> pixelArray;
	Common::Array<byte> maskArray;

	uint16 bx = 0, by = 0, bw = ((width + 3) / 4) * 4;

	// RLE decompress the buffers as needed
#define GET_BUFFER(name, reserveSize)                                                      \
	byte *name##Buffer;                                                                    \
	if (name##Flags & 1) {                                                                 \
		name##Array.reserve(reserveSize);                                                  \
		rleDecompress(source + name##Offs, source + name##Offs + name##Size, name##Array); \
		name##Buffer = name##Array.data();                                                 \
	} else                                                                                 \
		name##Buffer = source + name##Offs;

	GET_BUFFER(cmd, ((height + 3) / 4) * lineSize)
	GET_BUFFER(pixel, pixelSize)
	GET_BUFFER(mask, maskSize)

	//
	byte *destPtr = (byte *)surface.getPixels();

	byte bitBuf[40];

	int bitBufLastOfs = (((lineSize + 1) >> 1) << 1) - 2;
	int bitBufLastCount = ((width + 3) >> 2) & 7;
	if (bitBufLastCount == 0)
		bitBufLastCount = 8;

	byte *pCmdBuf = cmdBuffer;
	byte *pPixelBuf = pixelBuffer;
	byte *pMaskBuf = maskBuffer;
	for (int row = 0; row < height; row += 4) {

		memcpy(bitBuf, pCmdBuf, lineSize);
		pCmdBuf += lineSize;

		for (uint16 bitBufOfs = 0; bitBufOfs < lineSize; bitBufOfs += 2) {

			uint16 bits = READ_LE_UINT16(&bitBuf[bitBufOfs]);

			int bitCount;
			if (bitBufOfs == bitBufLastOfs)
				bitCount = bitBufLastCount;
			else
				bitCount = 8;

			for (int curCmd = 0; curCmd < bitCount; curCmd++) {
				uint cmd = bits & 3;
				bits >>= 2;

				byte pixels[4], block[16];
				uint32 mask;

				switch (cmd) {

				case 0:
					// solid color fill of block
					pixels[0] = *pPixelBuf++;
					for (int i = 0; i < 16; i++)
						block[i] = pixels[0];
					break;

				case 1:
					// 2-color block
					pixels[0] = *pPixelBuf++;
					pixels[1] = *pPixelBuf++;
					mask = READ_LE_UINT16(pMaskBuf);
					pMaskBuf += 2;
					for (int i = 0; i < 16; i++) {
						block[i] = pixels[mask & 1];
						mask >>= 1;
					}
					break;

				case 2:
					// 4 colors
					pixels[0] = *pPixelBuf++;
					pixels[1] = *pPixelBuf++;
					pixels[2] = *pPixelBuf++;
					pixels[3] = *pPixelBuf++;
					mask = READ_LE_UINT32(pMaskBuf);
					pMaskBuf += 4;
					for (int i = 0; i < 16; i++) {
						block[i] = pixels[mask & 3];
						mask >>= 2;
					}
					break;

				// case 3:
				default:
					// "Skip" (delta frame optimization)
					break;
				}

				if (cmd != 3) {
					uint16 blockPos = 0;
					uint32 maxW = MIN(4, width - bx);
					uint32 maxH = (MIN(4, height - by) + by) * surface.w;
					for (uint32 yc = by * surface.w; yc < maxH; yc += surface.w) {
						for (uint32 xc = 0; xc < maxW; xc++) {
							destPtr[(bx + xc) + yc] = block[xc + blockPos];
						}
						blockPos += 4;
					}
				}

				bx += 4;
				if (bx >= bw) {
					bx = 0;
					by += 4;
				}

			}

		}
	}
}

} // End of namespace Made
