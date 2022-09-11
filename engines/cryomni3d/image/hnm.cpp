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

#include "cryomni3d/image/hnm.h"

#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "image/codecs/codec.h"

#include "image/codecs/hnm.h"

namespace Image {

HNMFileDecoder::HNMFileDecoder(const Graphics::PixelFormat &format) :
	_format(format), _surface(nullptr), _codec(nullptr) {
}

HNMFileDecoder::~HNMFileDecoder() {
	destroy();
}

void HNMFileDecoder::destroy() {
	delete _codec;
	_codec = nullptr;
	_surface = nullptr;
}

bool HNMFileDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	uint32 tag = stream.readUint32BE();

	/* Only HNM6 for HNM images */
	if (tag != MKTAG('H', 'N', 'M', '6')) {
		return false;
	}

	//uint16 ukn = stream.readUint16BE();
	//byte audioflags = stream.readByte();
	//byte bpp = stream.readByte();
	stream.skip(4);
	uint16 width = stream.readUint16LE();
	uint16 height = stream.readUint16LE();
	//uint32 filesize = stream.readUint32LE();
	//uint32 numframes = stream.readUint32LE();
	//uint32 ukn2 = stream.readUint32LE();
	//uint16 speed = stream.readUint16LE();
	//uint16 maxbuffer = stream.readUint16LE();
	//uint32 buffer_size = stream.readUint32LE();
	//byte unknownStr[16];
	//byte copyright[16];
	//stream.read(unknownStr, sizeof(unknownStr));
	//stream.read(copyright, sizeof(copyright));

	stream.skip(52);

	if (width == 0 || height == 0) {
		return false;
	}

	// Read frame header
	uint32 frameSize = stream.readUint32LE();
	if (frameSize < 12) {
		return false;
	}
	frameSize -= 4;

	// Read chunk header
	uint32 chunkSize = stream.readUint32LE();
	uint16 chunkTag = stream.readUint16BE();
	//uint16 chunkUkn = stream.readUint16LE();
	stream.skip(2);

	if (frameSize < chunkSize ||
	        chunkSize < 8 + 24) {
		return false;
	}

	bool warp;
	if (chunkTag == MKTAG16('I', 'W')) {
		warp = true;
	} else if (chunkTag == MKTAG16('I', 'X')) {
		warp = false;
	} else {
		return false;
	}

	// buffer_size is not reliable on IW and we already have the real size of the image source
	_codec = createHNM6Decoder(width, height, _format, chunkSize, false);
	_codec->setWarpMode(warp);
	_surface = _codec->decodeFrame(stream);
	return true;
}

} // End of namespace Image
