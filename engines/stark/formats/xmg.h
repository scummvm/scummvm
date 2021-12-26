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

#ifndef STARK_XMG_H
#define STARK_XMG_H

#include "common/stream.h"

namespace Graphics {
	struct Surface;
}

namespace Stark {
namespace Formats {

/**
 * XMG (still image) decoder
 */
class XMGDecoder {
public:
	static Graphics::Surface *decode(Common::ReadStream *stream);
	static void readSize(Common::ReadStream *stream, uint &width, uint &height);

private:
	explicit XMGDecoder(Common::ReadStream *stream);

	struct Block {
		uint32 a1, a2;
		uint32 b1, b2;
	};

	void readHeader();
	Graphics::Surface *decodeImage();
	Block decodeBlock(byte op);
	void drawBlock(const Block &block, Graphics::Surface *surface);

	Block processYCrCb();
	Block processTrans();
	Block processRGB();

	uint32 _width;
	uint32 _height;

	uint32 _currX;
	uint32 _currY;

	Common::ReadStream *_stream;

	/**
	 * The transparency color in the RGB and transparency blocks.
	 * In the output surface, the transparent color is black with zero
	 * alpha. So the images are effectively pre-multiplied alpha.
	 */
	uint32 _transColor;
};

} // End of namespace Formats
} // End of namespace Stark

#endif // STARK_XMG_H
