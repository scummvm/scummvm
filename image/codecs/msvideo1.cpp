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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

 // Based off ffmpeg's msvideo.cpp

#include "image/codecs/msvideo1.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Image {

#define CHECK_STREAM_PTR(n) \
  if ((stream.pos() + n) > stream.size() ) { \
	warning ("MS Video-1: Stream out of bounds (%d >= %d) d%d", stream.pos() + n, stream.size(), n); \
    return; \
  }

MSVideo1Decoder::MSVideo1Decoder(uint16 width, uint16 height, byte bitsPerPixel) : Codec() {
	_surface = new Graphics::Surface();
	_surface->create(width, height, (bitsPerPixel == 8) ? Graphics::PixelFormat::createFormatCLUT8() :
                                                          Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

	_bitsPerPixel = bitsPerPixel;
}

MSVideo1Decoder::~MSVideo1Decoder() {
	_surface->free();
	delete _surface;
}

void MSVideo1Decoder::decode8(Common::SeekableReadStream &stream) {
    byte colors[8];
    byte *pixels = (byte *)_surface->getPixels();
    uint16 stride = _surface->w;

    int skipBlocks = 0;
    uint16 blocks_wide = _surface->w / 4;
    uint16 blocks_high = _surface->h / 4;
    uint32 totalBlocks = blocks_wide * blocks_high;
    uint32 blockInc = 4;
    uint16 rowDec = stride + 4;

    for (uint16 block_y = blocks_high; block_y > 0; block_y--) {
        uint32 blockPtr = (block_y * 4 - 1) * stride;
        for (uint16 block_x = blocks_wide; block_x > 0; block_x--) {
            // check if this block should be skipped
            if (skipBlocks > 0) {
                blockPtr += blockInc;
                skipBlocks--;
                totalBlocks--;
                continue;
            }

            uint32 pixelPtr = blockPtr;

            /* get the next two bytes in the encoded data stream */
            CHECK_STREAM_PTR(2);
            byte byte_a = stream.readByte();
            byte byte_b = stream.readByte();

            /* check if the decode is finished */
            if (byte_a == 0 && byte_b == 0 && totalBlocks == 0) {
                return;
            } else if ((byte_b & 0xFC) == 0x84) {
                // skip code, but don't count the current block
                skipBlocks = ((byte_b - 0x84) << 8) + byte_a - 1;
            } else if (byte_b < 0x80) {
                // 2-color encoding
                uint16 flags = (byte_b << 8) | byte_a;

                CHECK_STREAM_PTR(2);
                colors[0] = stream.readByte();
                colors[1] = stream.readByte();

                for (byte pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (byte pixel_x = 0; pixel_x < 4; pixel_x++, flags >>= 1)
                        pixels[pixelPtr++] = colors[(flags & 0x1) ^ 1];
                    pixelPtr -= rowDec;
                }
            } else if (byte_b >= 0x90) {
                // 8-color encoding
                uint16 flags = (byte_b << 8) | byte_a;

                CHECK_STREAM_PTR(8);
				for (byte i = 0; i < 8; i++)
					colors[i] = stream.readByte();

                for (byte pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (byte pixel_x = 0; pixel_x < 4; pixel_x++, flags >>= 1)
                        pixels[pixelPtr++] = colors[((pixel_y & 0x2) << 1) + (pixel_x & 0x2) + ((flags & 0x1) ^ 1)];
                    pixelPtr -= rowDec;
                }
            } else {
                // 1-color encoding
                colors[0] = byte_a;

                for (byte pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (byte pixel_x = 0; pixel_x < 4; pixel_x++)
                        pixels[pixelPtr++] = colors[0];
                    pixelPtr -= rowDec;
                }
            }

            blockPtr += blockInc;
            totalBlocks--;
        }
    }
}

void MSVideo1Decoder::decode16(Common::SeekableReadStream &stream) {
    /* decoding parameters */
    uint16 colors[8];
    uint16 *pixels = (uint16 *)_surface->getPixels();
    int32 stride = _surface->w;

    int32 skip_blocks = 0;
    int32 blocks_wide = _surface->w / 4;
    int32 blocks_high = _surface->h / 4;
    int32 total_blocks = blocks_wide * blocks_high;
    int32 block_inc = 4;
    int32 row_dec = stride + 4;

    for (int32 block_y = blocks_high; block_y > 0; block_y--) {
        int32 block_ptr = ((block_y * 4) - 1) * stride;
        for (int32 block_x = blocks_wide; block_x > 0; block_x--) {
            /* check if this block should be skipped */
            if (skip_blocks) {
                block_ptr += block_inc;
                skip_blocks--;
                total_blocks--;
                continue;
            }

            int32 pixel_ptr = block_ptr;

            /* get the next two bytes in the encoded data stream */
            CHECK_STREAM_PTR(2);
            byte byte_a = stream.readByte();
            byte byte_b = stream.readByte();

            /* check if the decode is finished */
            if ((byte_a == 0) && (byte_b == 0) && (total_blocks == 0)) {
                return;
            } else if ((byte_b & 0xFC) == 0x84) {
                /* skip code, but don't count the current block */
                skip_blocks = ((byte_b - 0x84) << 8) + byte_a - 1;
            } else if (byte_b < 0x80) {
                /* 2- or 8-color encoding modes */
                uint16 flags = (byte_b << 8) | byte_a;

                CHECK_STREAM_PTR(4);
                colors[0] = stream.readUint16LE();
                colors[1] = stream.readUint16LE();

                if (colors[0] & 0x8000) {
                    /* 8-color encoding */
                    CHECK_STREAM_PTR(12);
                    colors[2] = stream.readUint16LE();
                    colors[3] = stream.readUint16LE();
                    colors[4] = stream.readUint16LE();
                    colors[5] = stream.readUint16LE();
                    colors[6] = stream.readUint16LE();
                    colors[7] = stream.readUint16LE();

                    for (int pixel_y = 0; pixel_y < 4; pixel_y++) {
                        for (int pixel_x = 0; pixel_x < 4; pixel_x++, flags >>= 1)
                            pixels[pixel_ptr++] =
                                colors[((pixel_y & 0x2) << 1) +
                                    (pixel_x & 0x2) + ((flags & 0x1) ^ 1)];
                        pixel_ptr -= row_dec;
                    }
                } else {
                    /* 2-color encoding */
                    for (int pixel_y = 0; pixel_y < 4; pixel_y++) {
                        for (int pixel_x = 0; pixel_x < 4; pixel_x++, flags >>= 1)
                            pixels[pixel_ptr++] = colors[(flags & 0x1) ^ 1];
                        pixel_ptr -= row_dec;
                    }
                }
            } else {
                /* otherwise, it's a 1-color block */
                colors[0] = (byte_b << 8) | byte_a;

                for (int pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (int pixel_x = 0; pixel_x < 4; pixel_x++)
                        pixels[pixel_ptr++] = colors[0];
                    pixel_ptr -= row_dec;
                }
            }

            block_ptr += block_inc;
            total_blocks--;
        }
    }
}

const Graphics::Surface *MSVideo1Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	if (_bitsPerPixel == 8)
		decode8(stream);
	else
        decode16(stream);

    return _surface;
}

} // End of namespace Image
