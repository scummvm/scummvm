/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRAPHICS_TEXELBUFFER_H
#define GRAPHICS_TEXELBUFFER_H

#include "graphics/pixelbuffer.h"

namespace Graphics {

class TexelBuffer {
public:
	TexelBuffer(unsigned int width, unsigned int height, unsigned int textureSize);
	virtual ~TexelBuffer() {};

	void getARGBAt(
		unsigned int wrap_s, unsigned int wrap_t,
		int s, int t,
		uint8 &a, uint8 &r, uint8 &g, uint8 &b
	) const;

protected:
	virtual void getARGBAt(
		unsigned int pixel,
		unsigned int ds, unsigned int dt,
		uint8 &a, uint8 &r, uint8 &g, uint8 &b
	) const = 0;
	unsigned int _width, _height, _fracTextureUnit, _fracTextureMask;
	float _widthRatio, _heightRatio;
};

class NearestTexelBuffer : public TexelBuffer {
public:
	NearestTexelBuffer(const PixelBuffer &buf, unsigned int width, unsigned int height, unsigned int textureSize);
	~NearestTexelBuffer();

protected:
	void getARGBAt(
		unsigned int pixel,
		unsigned int, unsigned int,
		uint8 &a, uint8 &r, uint8 &g, uint8 &b
	) const override;

private:
	PixelBuffer _buf;
};

class BilinearTexelBuffer : public TexelBuffer {
public:
	BilinearTexelBuffer(const PixelBuffer &buf, unsigned int width, unsigned int height, unsigned int textureSize);
	~BilinearTexelBuffer();

protected:
	void getARGBAt(
		unsigned int pixel,
		unsigned int ds, unsigned int dt,
		uint8 &a, uint8 &r, uint8 &g, uint8 &b
	) const override;

private:
	uint32 *_texels;
};

}

#endif
