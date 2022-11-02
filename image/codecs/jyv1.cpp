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

#include "image/codecs/jyv1.h"

#include "common/stream.h"
#include "common/bitstream.h"
#include "common/memstream.h"
#include "common/util.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "common/debug.h"
#include "graphics/surface.h"

#define ID_JYV1 MKTAG('J','Y','V','1')
#define ID_RRV1 MKTAG('R','R','V','1')
#define ID_RRV2 MKTAG('R','R','V','2')

namespace Image {

/*static*/
bool JYV1Decoder::isJYV1StreamTag(uint32 streamTag) {
	return (streamTag == ID_JYV1 || streamTag == ID_RRV1 || streamTag == ID_RRV2);
}

JYV1Decoder::JYV1Decoder(int width, int height, uint32 streamTag) : Codec(),
		_width(width), _height(height), _streamType(streamTag) {
	assert(isJYV1StreamTag(streamTag));
	_surface.create(_width, _height, getPixelFormat());
}

JYV1Decoder::~JYV1Decoder() {
	_surface.free();
}

static const uint32 BASE_LEN[] = {0, 1 << 7, 1 << 3, 0, 1 << 1, 0, 1 << 5, 0,
								  1, 1 << 8, 1 << 4, 0, 1 << 2, 0, 1 << 6, 0};
static const uint32 FINE_LEN_BITS[] = {0, 7, 3, 0, 1, 16, 5, 0,
									   1, 8, 4, 0, 2, 24, 6, 0};

/**
 * Details of this decoding algorithm are here:
 * https://wiki.multimedia.cx/index.php/Origin_Flic_Codec
 */
const Graphics::Surface *JYV1Decoder::decodeFrame(Common::SeekableReadStream &stream) {

	byte *dst = (byte *)_surface.getPixels();

	uint32 offsets[16]; // RRV2 has 15 block offsets, others have 5
	const int numOffsets = (_streamType == ID_RRV2 ? 15 : 5);
	const int blockHeight = _height / numOffsets;
	const int startOffset = stream.pos();

	// Read in the block offsets and convert to stream offsets
	for (int i = 0; i < numOffsets; i++) {
		offsets[i] = stream.readUint32LE() + startOffset;
	}

	bool upscale = false;

	//
	// Slight HACK: test if we need to scale up this frame without
	// changing the output data yet.  This has a bit of duplicated code
	// with the loop below just to measure the frame size from the
	// first block.
	//
	if (_streamType == ID_RRV1 || _streamType == ID_RRV2) {
		stream.seek(offsets[0], SEEK_SET);
		const int cmdLen = stream.readUint32LE();
		uint8 *cmdData = new uint8[cmdLen];
		stream.read(cmdData, cmdLen);
		Common::BitStreamMemoryStream cmdMemStream(cmdData, cmdLen);
		Common::BitStreamMemory8MSB cmdBitStream(cmdMemStream);
		int total = 0;
		while (!cmdBitStream.eos()) {
			uint32 idx = cmdBitStream.getBits<4>();
			total += BASE_LEN[idx];
			if (idx != 0 && idx != 8) {
			   total += cmdBitStream.getBits(FINE_LEN_BITS[idx]);
			}
		}
		delete [] cmdData;

		if (total == _width * blockHeight / 2)
			upscale = true;
	}

	int y = 0;
	int x = 0;

	for (int i = 0; i < numOffsets && y < _height; i++) {
		stream.seek(offsets[i], SEEK_SET);
		const int cmdLen = stream.readUint32LE();

		// TODO: can probably avoid this copy to make it faster
		uint8 *cmdData = new uint8[cmdLen];
		stream.read(cmdData, cmdLen);
		Common::BitStreamMemoryStream cmdMemStream(cmdData, cmdLen);
		Common::BitStreamMemory8MSB cmdBitStream(cmdMemStream);
		bool skipping = true;
		while (cmdBitStream.size() - cmdBitStream.pos() >= 4 && y < _height) {
			uint32 idx = cmdBitStream.getBits<4>();
			uint32 blocksize = BASE_LEN[idx];
			if (idx != 0 && idx != 8) {
			   blocksize += cmdBitStream.getBits(FINE_LEN_BITS[idx]);
			}
			if (skipping) {
				// leave blocksize pixels unchanged
				if (upscale)
					blocksize *= 2;

				while (blocksize) {
					blocksize--;
					x++;
					if (x == _width) {
						x = 0;
						y++;
					}
				}
			} else {
				// draw blocksize pixels from data block
				while (blocksize && y < _height) {
					// TODO: would be nicer to read these in whole scanlines.
					// Also this upscale code is kinda ugly.
					const uint8 p = stream.readByte();
					dst[y * _width + x] = p;
					x++;
					if (x == _width) {
						x = 0;
						y++;
					}
					if (upscale) {
						dst[y * _width + x] = p;
						x++;
						if (x == _width) {
							x = 0;
							y++;
						}
					}
					blocksize--;
				}
			}
			skipping = !skipping;
		}
		delete [] cmdData;
	}
	return &_surface;
}

Graphics::PixelFormat JYV1Decoder::getPixelFormat() const {
	return Graphics::PixelFormat::createFormatCLUT8();
}

} // End of namespace Image
