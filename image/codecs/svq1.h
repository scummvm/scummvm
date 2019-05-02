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

#ifndef IMAGE_CODECS_SVQ1_H
#define IMAGE_CODECS_SVQ1_H

#include "common/bitstream.h"
#include "image/codecs/codec.h"

namespace Common {
template <class BITSTREAM>
class Huffman;
struct Point;
}

namespace Image {

/**
 * Sorenson Vector Quantizer 1 decoder.
 *
 * Used by PICT/QuickTime.
 */
class SVQ1Decoder : public Codec {
public:
	SVQ1Decoder(uint16 width, uint16 height);
	~SVQ1Decoder();

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream);
	Graphics::PixelFormat getPixelFormat() const { return _surface->format; }

private:
	Graphics::Surface *_surface;
	uint16 _width, _height;
	uint16 _frameWidth, _frameHeight;

	byte *_last[3];

	typedef Common::Huffman<Common::BitStream32BEMSB> HuffmanDecoder;

	HuffmanDecoder *_blockType;
	HuffmanDecoder *_intraMultistage[6];
	HuffmanDecoder *_interMultistage[6];
	HuffmanDecoder *_intraMean;
	HuffmanDecoder *_interMean;
	HuffmanDecoder *_motionComponent;

	bool svq1DecodeBlockIntra(Common::BitStream32BEMSB *s, byte *pixels, int pitch);
	bool svq1DecodeBlockNonIntra(Common::BitStream32BEMSB *s, byte *pixels, int pitch);
	bool svq1DecodeMotionVector(Common::BitStream32BEMSB *s, Common::Point *mv, Common::Point **pmv);
	void svq1SkipBlock(byte *current, byte *previous, int pitch, int x, int y);
	bool svq1MotionInterBlock(Common::BitStream32BEMSB *ss, byte *current, byte *previous, int pitch,
			Common::Point *motion, int x, int y);
	bool svq1MotionInter4vBlock(Common::BitStream32BEMSB *ss, byte *current, byte *previous, int pitch,
			Common::Point *motion, int x, int y);
	bool svq1DecodeDeltaBlock(Common::BitStream32BEMSB *ss, byte *current, byte *previous, int pitch,
			Common::Point *motion, int x, int y);

	void putPixels8C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels8L2(byte *dst, const byte *src1, const byte *src2, int dstStride, int srcStride1, int srcStride2, int h);
	void putPixels8X2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels8Y2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels8XY2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels16C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels16X2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels16Y2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels16XY2C(byte *block, const byte *pixels, int lineSize, int h);
};

} // End of namespace Image

#endif
