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

#ifndef GRAPHICS_TEXELBUFFER_H
#define GRAPHICS_TEXELBUFFER_H

#include "graphics/pixelformat.h"

namespace TinyGL {

class TexelBuffer {
public:
	TexelBuffer(uint width, uint height, uint textureSize);
	virtual ~TexelBuffer() {};

	void getARGBAt(
		uint wrap_s, uint wrap_t,
		int s, int t,
		uint8 &a, uint8 &r, uint8 &g, uint8 &b
	) const;

protected:
	virtual void getARGBAt(
		uint pixel,
		uint ds, uint dt,
		uint8 &a, uint8 &r, uint8 &g, uint8 &b
	) const = 0;
	uint _width, _height, _fracTextureUnit, _fracTextureMask;
	float _widthRatio, _heightRatio;
};

TexelBuffer *createNearestTexelBuffer(const byte *buf, const Graphics::PixelFormat &pf, uint format, uint type, uint width, uint height, uint textureSize);
TexelBuffer *createBilinearTexelBuffer(byte *buf, const Graphics::PixelFormat &pf, uint format, uint type, uint width, uint height, uint textureSize);

} // end of namespace TinyGL

#endif
