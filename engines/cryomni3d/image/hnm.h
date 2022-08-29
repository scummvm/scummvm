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

#ifndef CRYOMNI3D_IMAGE_HNM_H
#define CRYOMNI3D_IMAGE_HNM_H

#include "common/scummsys.h"
#include "common/str.h"
#include "graphics/pixelformat.h"
#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {
class HNM6Decoder;

class HNMFileDecoder : public ImageDecoder {
public:
	HNMFileDecoder(const Graphics::PixelFormat &format);
	~HNMFileDecoder() override;

	// ImageDecoder API
	void destroy() override;
	bool loadStream(Common::SeekableReadStream &stream) override;
	const Graphics::Surface *getSurface() const override { return _surface; }

private:
	Graphics::PixelFormat _format;
	HNM6Decoder *_codec;
	const Graphics::Surface *_surface;
};

} // End of namespace Image

#endif
