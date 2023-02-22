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

#ifndef IMAGE_CODECS_JYV1_H
#define IMAGE_CODECS_JYV1_H

#include "image/codecs/codec.h"

namespace Image {

/**
 * JYV1/RRV1/RRV2 image decoder.
 *
 * Used by Crusader: No Remorse AVI files
 */
class JYV1Decoder : public Codec {
public:
	JYV1Decoder (int width, int height, uint32 streamTag);
	~JYV1Decoder() override;

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	Graphics::PixelFormat getPixelFormat() const override;

	static bool isJYV1StreamTag(uint32 streamTag);

private:
	Graphics::Surface _surface;
	int _width, _height;
	uint32 _streamType;
};

} // End of namespace Image

#endif
