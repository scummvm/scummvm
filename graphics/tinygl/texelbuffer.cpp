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

#include "graphics/tinygl/gl.h"
#include "graphics/tinygl/zgl.h"
#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/texelbuffer.h"

namespace Graphics {

#define ZB_POINT_ST_UNIT (1 << ZB_POINT_ST_FRAC_BITS)
#define ZB_POINT_ST_FRAC_MASK (ZB_POINT_ST_UNIT - 1)

TexelBuffer::TexelBuffer(unsigned int width, unsigned int height, unsigned int textureSize) {
	assert(width);
	assert(height);
	assert(textureSize);

	_width = width;
	_height = height;
	_fracTextureUnit = textureSize << ZB_POINT_ST_FRAC_BITS;
	_fracTextureMask = _fracTextureUnit - 1;
	_widthRatio = (float) width / textureSize;
	_heightRatio = (float) height / textureSize;
}

static inline unsigned int wrap(unsigned int wrap_mode, int coord, unsigned int _fracTextureUnit, unsigned int _fracTextureMask) {
	switch (wrap_mode) {
	case TGL_MIRRORED_REPEAT:
		if (coord & _fracTextureUnit)
			return _fracTextureMask - (coord & _fracTextureMask);
		return coord & _fracTextureMask;
	case TGL_CLAMP_TO_EDGE:
		if (coord < 0)
			return 0;
		if ((unsigned int) coord > _fracTextureMask)
			return _fracTextureMask;
		return coord;
	default:
		// Fall through
	case TGL_REPEAT:
		return coord & _fracTextureMask;
	}
}

void TexelBuffer::getARGBAt(
	unsigned int wrap_s, unsigned int wrap_t,
	int s, int t,
	uint8 &a, uint8 &r, uint8 &g, uint8 &b
) const {
	unsigned int x, y;
	x = wrap(wrap_s, s, _fracTextureUnit, _fracTextureMask) * _widthRatio;
	y = wrap(wrap_t, t, _fracTextureUnit, _fracTextureMask) * _heightRatio;
	getARGBAt(
		(x >> ZB_POINT_ST_FRAC_BITS) + (y >> ZB_POINT_ST_FRAC_BITS) * _width,
		x & ZB_POINT_ST_FRAC_MASK, y & ZB_POINT_ST_FRAC_MASK,
		a, r, g, b
	);
}

// Nearest: store texture in original size.
NearestTexelBuffer::NearestTexelBuffer(const PixelBuffer &buf, unsigned int width, unsigned int height, unsigned int textureSize) : TexelBuffer(width, height, textureSize) {
	unsigned int pixel_count = _width * _height;
	_buf = PixelBuffer(buf.getFormat(), pixel_count, DisposeAfterUse::NO);
	_buf.copyBuffer(0, pixel_count, buf);
}

NearestTexelBuffer::~NearestTexelBuffer() {
	_buf.free();
}

void NearestTexelBuffer::getARGBAt(
	unsigned int pixel,
	unsigned int, unsigned int,
	uint8 &a, uint8 &r, uint8 &g, uint8 &b
) const {
	_buf.getARGBAt(pixel, a, r, g, b);
}

// Bilinear: each texture coordinates corresponds to the 4 original image
// pixels linear interpolation has to work on, so that they are near each
// other in CPU data cache, and a single actual memory fetch happens. This
// allows applying linear filtering at render time at a very low performance
// cost. As we expect to work on small-ish textures (512*512 ?) the 4x memory
// usage increase should be negligible.
#define A_OFFSET (0 * 4)
#define R_OFFSET (1 * 4)
#define G_OFFSET (2 * 4)
#define B_OFFSET (3 * 4)
#define P00_OFFSET 0
#define P01_OFFSET 1
#define P10_OFFSET 2
#define P11_OFFSET 3
#define PIXEL_PER_TEXEL_SHIFT 2

BilinearTexelBuffer::BilinearTexelBuffer(const PixelBuffer &buf, unsigned int width, unsigned int height, unsigned int textureSize) : TexelBuffer(width, height, textureSize) {
	unsigned int pixel00_offset = 0, pixel11_offset, pixel01_offset, pixel10_offset;
	uint8 *texel8;
	uint32 *texel32;

	texel32 = _texels = new uint32[_width * _height << PIXEL_PER_TEXEL_SHIFT];
	for (unsigned int y = 0; y < _height; y++) {
		for (unsigned int x = 0; x < _width; x++) {
			texel8 = (uint8 *)texel32;
			pixel11_offset = pixel00_offset + _width + 1;
			buf.getARGBAt(
				pixel00_offset,
				*(texel8 + P00_OFFSET + A_OFFSET),
				*(texel8 + P00_OFFSET + R_OFFSET),
				*(texel8 + P00_OFFSET + G_OFFSET),
				*(texel8 + P00_OFFSET + B_OFFSET)
			);
			if ((x + 1) == _width) {
				pixel11_offset -= 1;
				pixel01_offset = pixel00_offset;
			} else
				pixel01_offset = pixel00_offset + 1;
			buf.getARGBAt(
				pixel01_offset,
				*(texel8 + P01_OFFSET + A_OFFSET),
				*(texel8 + P01_OFFSET + R_OFFSET),
				*(texel8 + P01_OFFSET + G_OFFSET),
				*(texel8 + P01_OFFSET + B_OFFSET)
			);
			if ((y + 1) == _height) {
				pixel11_offset -= _width;
				pixel10_offset = pixel00_offset;
			} else
				pixel10_offset = pixel00_offset + _width;
			buf.getARGBAt(
				pixel10_offset,
				*(texel8 + P10_OFFSET + A_OFFSET),
				*(texel8 + P10_OFFSET + R_OFFSET),
				*(texel8 + P10_OFFSET + G_OFFSET),
				*(texel8 + P10_OFFSET + B_OFFSET)
			);
			buf.getARGBAt(
				pixel11_offset,
				*(texel8 + P11_OFFSET + A_OFFSET),
				*(texel8 + P11_OFFSET + R_OFFSET),
				*(texel8 + P11_OFFSET + G_OFFSET),
				*(texel8 + P11_OFFSET + B_OFFSET)
			);
			texel32 += 1 << PIXEL_PER_TEXEL_SHIFT;
			pixel00_offset++;
		}
	}
}

BilinearTexelBuffer::~BilinearTexelBuffer() {
	delete[] _texels;
}

static inline int interpolate(int v00, int v01, int v10, int xf, int yf) {
	return v00 + (((v01 - v00) * xf + (v10 - v00) * yf) >> ZB_POINT_ST_FRAC_BITS);
}

void BilinearTexelBuffer::getARGBAt(
	unsigned int pixel,
	unsigned int ds, unsigned int dt,
	uint8 &a, uint8 &r, uint8 &g, uint8 &b
) const {
	unsigned int p00_offset, p01_offset, p10_offset;
	uint8 *texel = (uint8 *)(_texels + (pixel << PIXEL_PER_TEXEL_SHIFT));
	if ((ds + dt) > ZB_POINT_ST_UNIT) {
		p00_offset = P11_OFFSET;
		p10_offset = P01_OFFSET;
		p01_offset = P10_OFFSET;
		ds = ZB_POINT_ST_UNIT - ds;
		dt = ZB_POINT_ST_UNIT - dt;
	} else {
		p00_offset = P00_OFFSET;
		p10_offset = P10_OFFSET;
		p01_offset = P01_OFFSET;
	}
	a = interpolate(
		*(texel + p00_offset + A_OFFSET),
		*(texel + p01_offset + A_OFFSET),
		*(texel + p10_offset + A_OFFSET),
		ds,
		dt
	);
	r = interpolate(
		*(texel + p00_offset + R_OFFSET),
		*(texel + p01_offset + R_OFFSET),
		*(texel + p10_offset + R_OFFSET),
		ds,
		dt
	);
	g = interpolate(
		*(texel + p00_offset + G_OFFSET),
		*(texel + p01_offset + G_OFFSET),
		*(texel + p10_offset + G_OFFSET),
		ds,
		dt
	);
	b = interpolate(
		*(texel + p00_offset + B_OFFSET),
		*(texel + p01_offset + B_OFFSET),
		*(texel + p10_offset + B_OFFSET),
		ds,
		dt
	);
}

}
