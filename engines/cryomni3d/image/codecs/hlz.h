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

#ifndef CRYOMNI3D_IMAGE_CODECS_HLZ_H
#define CRYOMNI3D_IMAGE_CODECS_HLZ_H

#include "image/codecs/codec.h"

namespace Image {

/**
 * HLZ image decoder.
 *
 * Used by HLZ image format and HNM video format.
 */
class HLZDecoder : public Codec {
public:
	HLZDecoder(int width, int height);
	~HLZDecoder() override;

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	Graphics::PixelFormat getPixelFormat() const override;

	static void decodeFrameInPlace(Common::SeekableReadStream &stream, uint32 size, byte *dst);

private:
	Graphics::Surface *_surface;
	int _width, _height;
};

} // End of namespace Image

#endif
