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
#include "graphics/tinygl/colormasks.h"

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

class BaseNearestTexelBuffer : public TexelBuffer {
public:
	BaseNearestTexelBuffer(const byte *buf, const Graphics::PixelFormat &format, uint width, uint height, uint textureSize);
	~BaseNearestTexelBuffer();

protected:
	byte *_buf;
	Graphics::PixelFormat _format;
};

template<uint Format, uint Type>
class NearestTexelBuffer final : public BaseNearestTexelBuffer {
public:
	NearestTexelBuffer(const byte *buf, const Graphics::PixelFormat &format, uint width, uint height, uint textureSize)
	  : BaseNearestTexelBuffer(buf, format, width, height, textureSize) {}

protected:
	void getARGBAt(
		uint pixel,
		uint, uint,
		uint8 &a, uint8 &r, uint8 &g, uint8 &b
	) const override {
		Pixel col = *(((const Pixel *)_buf) + pixel);
		_format.colorToARGBT<ColorMask>(col, a, r, g, b);
	}

	typedef ColorMasks<Format, Type> ColorMask;
	typedef typename ColorMask::PixelType Pixel;
};

template<>
class NearestTexelBuffer<TGL_RGB, TGL_UNSIGNED_BYTE> final : public BaseNearestTexelBuffer {
public:
	NearestTexelBuffer(const byte *buf, const Graphics::PixelFormat &format, uint width, uint height, uint textureSize)
	  : BaseNearestTexelBuffer(buf, format, width, height, textureSize) {}

protected:
	void getARGBAt(
		uint pixel,
		uint, uint,
		uint8 &a, uint8 &r, uint8 &g, uint8 &b
	) const override {
		byte *col = _buf + (pixel * 3);
		a = 0xff;
		r = col[0];
		g = col[1];
		b = col[2];
	}
};

class BilinearTexelBuffer : public TexelBuffer {
public:
	BilinearTexelBuffer(byte *buf, const Graphics::PixelFormat &format, uint width, uint height, uint textureSize);
	~BilinearTexelBuffer();

protected:
	void getARGBAt(
		uint pixel,
		uint ds, uint dt,
		uint8 &a, uint8 &r, uint8 &g, uint8 &b
	) const override;

private:
	uint32 *_texels;
};

} // end of namespace TinyGL

#endif
