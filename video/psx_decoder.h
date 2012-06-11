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
 * This currently implements the most basic PSX stream format that is
 * used by most games on the system. Special variants are not supported
 * at this time.
 *
 * Video decoder used in engines:
 *  - sword1 (psx)
 *  - sword2 (psx)
 */
class PSXStreamDecoder : public VideoDecoder {
public:
	// CD speed in sectors/second
	// Calling code should use these enum values instead of the constants
	enum CDSpeed {
		kCD1x = 75,
		kCD2x = 150
	};

	PSXStreamDecoder(CDSpeed speed, uint32 frameCount = 0);
	virtual ~PSXStreamDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

	bool isVideoLoaded() const { return _stream != 0; }
	uint16 getWidth() const { return _surface->w; }
	uint16 getHeight() const { return _surface->h; }
	uint32 getFrameCount() const { return _frameCount; }
	uint32 getTime() const;
	uint32 getTimeToNextFrame() const;
	const Graphics::Surface *decodeNextFrame();
	Graphics::PixelFormat getPixelFormat() const { return _surface->format; }
	bool endOfVideo() const { return _stream->pos() >= _stream->size(); }

protected:
	// VideoDecoder API
	void updateVolume();
	void updateBalance();

private:
	void initCommon();
	Common::SeekableReadStream *_stream;
	Graphics::Surface *_surface;

	uint32 _frameCount;
	Audio::Timestamp _nextFrameStartTime;

	Audio::SoundHandle _audHandle;
	Audio::QueuingAudioStream *_audStream;
	void queueAudioFromSector(Common::SeekableReadStream *sector);

	enum PlaneType {
		kPlaneY = 0,
		kPlaneU = 1,
		kPlaneV = 2
	};

	uint16 _macroBlocksW, _macroBlocksH;
	byte *_yBuffer, *_cbBuffer, *_crBuffer;
	void decodeFrame(Common::SeekableReadStream *frame);
	void decodeMacroBlock(Common::BitStream *bits, int mbX, int mbY, uint16 scale, uint16 version);
	void decodeBlock(Common::BitStream *bits, byte *block, int pitch, uint16 scale, uint16 version, PlaneType plane);

	void readAC(Common::BitStream *bits, int *block);
	Common::Huffman *_acHuffman;

	int readDC(Common::BitStream *bits, uint16 version, PlaneType plane);
	Common::Huffman *_dcHuffmanLuma, *_dcHuffmanChroma;
	int _lastDC[3];

	void dequantizeBlock(int *coefficients, float *block, uint16 scale);
	void idct(float *dequantData, float *result);
	int readSignedCoefficient(Common::BitStream *bits);

	struct ADPCMStatus {
		int16 sample[2];
	} _adpcmStatus[2];

	Common::SeekableReadStream *readSector();
};

} // End of namespace Video

#endif
