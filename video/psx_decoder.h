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
 */

#ifndef VIDEO_PSX_DECODER_H
#define VIDEO_PSX_DECODER_H

#include "common/endian.h"
#include "common/rational.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class BitStream;
class Huffman;
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
}

namespace Video {

/**
 * Decoder for PSX stream videos.
 *
 * Video decoder used in engines:
 *  - sword1 (psx)
 *  - sword2 (psx)
 */
class PSXStreamDecoder : public FixedRateVideoDecoder {
public:
	// CD speed in sectors/second
	// Calling code should use these enum values instead of the constants
	enum CDSpeed {
		kCDUnk = 0,
		kCD1x = 75,
		kCD2x = 150
	};

	PSXStreamDecoder(Common::Rational frameRate);
	PSXStreamDecoder(CDSpeed speed, uint32 frameCount);
	virtual ~PSXStreamDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

	bool isVideoLoaded() const { return _stream != 0; }
	uint16 getWidth() const { return _surface->w; }
	uint16 getHeight() const { return _surface->h; }
	uint32 getFrameCount() const { return _frameCount; }
	uint32 getElapsedTime() const;
	const Graphics::Surface *decodeNextFrame();
	Graphics::PixelFormat getPixelFormat() const { return _surface->format; }
	bool endOfVideo() const { return _stream->pos() >= _stream->size(); }

protected:
	// Hardcoded frame rate
	Common::Rational getFrameRate() const { return _frameRate; }

private:
	void initCommon();
	Common::SeekableReadStream *_stream;
	Graphics::Surface *_surface;

	CDSpeed _speed;
	uint32 _frameCount;
	Common::Rational _frameRate;

	Audio::SoundHandle _audHandle;
	Audio::QueuingAudioStream *_audStream;
	void queueAudioFromSector(Common::SeekableReadStream *sector);

	Common::Huffman *_huffman;
	uint16 _macroBlocksW, _macroBlocksH;
	byte *_yBuffer, *_cbBuffer, *_crBuffer;
	void decodeFrame(Common::SeekableReadStream *frame);
	void decodeMacroBlock(Common::BitStream *bits, int mbX, int mbY, uint16 scale, uint16 version);
	void decodeBlock(Common::BitStream *bits, byte *block, int pitch, uint16 scale, uint16 version);

	void dequantizeBlock(int *coefficients, float *block, uint16 scale);
	void readAC(Common::BitStream *bits, int *block);
	void idct(float *dequantData, float *result);
	int readSignedCoefficient(Common::BitStream *bits);

	struct ADPCMStatus {
		int16 sample[2];
	} _adpcmStatus[2];

	Common::SeekableReadStream *readSector();
};

} // End of namespace Video

#endif
