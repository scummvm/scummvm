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

#ifndef IMAGE_CODECS_HNM6_H
#define IMAGE_CODECS_HNM6_H

#include "image/codecs/codec.h"

namespace Image {

/**
 * HNM6 image decoder interface.
 *
 * Used by HNM6 image and video formats.
 */
class HNM6Decoder : public Codec {
public:
	uint16 getWidth() const { return _width; }
	uint16 getHeight() const { return _height; }
	Graphics::PixelFormat getPixelFormat() const override { return _format; }

	void setWarpMode(bool warpMode) { assert(!warpMode || !_videoMode); _warpMode = warpMode; }
protected:
	HNM6Decoder(uint16 width, uint16 height, const Graphics::PixelFormat &format,
	            bool videoMode = false) : Codec(),
		_width(width), _height(height), _format(format), _videoMode(videoMode), _warpMode(false) { }

	uint16 _width, _height;
	Graphics::PixelFormat _format;
	bool _warpMode, _videoMode;
};

HNM6Decoder *createHNM6Decoder(uint16 width, uint16 height, const Graphics::PixelFormat &format,
                               uint32 bufferSize, bool videoMode = false);

} // End of namespace Image

#endif
