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

#ifndef VIDEO_CODECS_SVQ1_H
#define VIDEO_CODECS_SVQ1_H

#include "video/codecs/codec.h"

namespace Common {
class BitStream;
class Huffman;
class Point;
}

namespace Video {

class SVQ1Decoder : public Codec {
public:
	SVQ1Decoder(uint16 width, uint16 height);
	~SVQ1Decoder();

	const Graphics::Surface *decodeImage(Common::SeekableReadStream *stream);
	Graphics::PixelFormat getPixelFormat() const { return _surface->format; }

private:
	Graphics::Surface *_surface;

	byte *_current[3];
	byte *_last[3];

	Common::Huffman *_blockType;
	Common::Huffman *_intraMultistage[6];
	Common::Huffman *_interMultistage[6];
	Common::Huffman *_intraMean;
	Common::Huffman *_interMean;
	Common::Huffman *_motionComponent;

	int svq1DecodeBlockIntra(Common::BitStream *s, uint8 *pixels, int pitch);
	int svq1DecodeBlockNonIntra(Common::BitStream *s, uint8 *pixels, int pitch);
	int svq1DecodeMotionVector(Common::BitStream *s, Common::Point *mv, Common::Point **pmv);
	void svq1SkipBlock(uint8 *current, uint8 *previous, int pitch, int x, int y);
	int svq1MotionInterBlock(Common::BitStream *ss, uint8 *current, uint8 *previous, int pitch,
			Common::Point *motion, int x, int y);
	int svq1MotionInter4vBlock(Common::BitStream *ss, uint8 *current, uint8 *previous, int pitch,
			Common::Point *motion, int x, int y);
	int svq1DecodeDeltaBlock(Common::BitStream *ss, uint8 *current, uint8 *previous, int pitch,
			Common::Point *motion, int x, int y);
};

} // End of namespace Video

#endif
