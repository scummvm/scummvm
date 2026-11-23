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

#ifndef IMAGE_CODECS_TM2A_H
#define IMAGE_CODECS_TM2A_H

#include "common/array.h"
#include "image/codecs/codec.h"

namespace Image {

// Version 4 intra frames, as used by the Runaway museum demo.
class TM2ADecoder : public Codec {
public:
	TM2ADecoder(int width, int height);
	~TM2ADecoder() override;

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	Graphics::PixelFormat getPixelFormat() const override { return _pixelFormat; }
	bool setOutputPixelFormat(const Graphics::PixelFormat &format) override;

private:
	bool readChunks(Common::Array<byte> &packet);
	bool readHeader(const byte *data, uint32 size);
	bool readCodebook(const byte *data, uint32 size);
	bool decodePixels();
	byte readToken();
	int16 readDelta(uint table);

	int _width, _height;
	Graphics::PixelFormat _pixelFormat;
	Graphics::Surface _surface;
	Common::Array<byte> _planes;
	Common::Array<uint32> _row;
	Common::Array<byte> _blocks;

	byte _codebook[256][8];
	byte _codeLengths[256];
	int16 _deltas[2][128];
	byte _deltaCounts[2];
	byte _modes[25];
	byte _modeCount;

	const byte *_data, *_dataEnd;
	const byte *_tokens, *_tokensEnd;
	bool _failed;
};

} // End of namespace Image

#endif
