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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

// Z buffer: 16,32 bits Z / 16 bits color

#include "common/scummsys.h"
#include "common/endian.h"

#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

// adr must be aligned on an 'int'
static void memset_s(void *adr, int val, int count) {
	int n, v;
	uint *p;
	unsigned short *q;

	p = (uint *)adr;
	v = val | (val << 16);

	n = count >> 3;
	for (int i = 0; i < n; i++) {
		p[0] = v;
		p[1] = v;
		p[2] = v;
		p[3] = v;
		p += 4;
	}

	q = (unsigned short *) p;
	n = count & 7;
	for (int i = 0; i < n; i++)
		*q++ = val;
}

static void memset_l(void *adr, int val, int count) {
	int n, v;
	uint *p;

	p = (uint *)adr;
	v = val;
	n = count >> 2;
	for (int i = 0; i < n; i++) {
		p[0] = v;
		p[1] = v;
		p[2] = v;
		p[3] = v;
		p += 4;
	}

	n = count & 3;
	for (int i = 0; i < n; i++)
		*p++ = val;
}

FrameBuffer::FrameBuffer(int width, int height, const Graphics::PixelFormat &format, bool enableStencilBuffer) {
	_pbufWidth = width;
	_pbufHeight = height;
	_pbufFormat = format;
	_pbufBpp = _pbufFormat.bytesPerPixel;
	_pbufPitch = (_pbufWidth * _pbufBpp + 3) & ~3;

	_pbuf.set(_pbufFormat, new byte[_pbufHeight * _pbufPitch]);
	_zbuf = (uint *)gl_zalloc(_pbufWidth * _pbufHeight * sizeof(uint));
	if (enableStencilBuffer)
		_sbuf = (byte *)gl_zalloc(_pbufWidth * _pbufHeight * sizeof(byte));
	else
		_sbuf = nullptr;

	_offscreenBuffer.pbuf = _pbuf.getRawBuffer();
	_offscreenBuffer.zbuf = _zbuf;

	_currentTexture = nullptr;

	_enableScissor = false;
}

FrameBuffer::~FrameBuffer() {
	_pbuf.free();
	gl_free(_zbuf);
	if (_sbuf)
		gl_free(_sbuf);
}

Buffer *FrameBuffer::genOffscreenBuffer() {
	Buffer *buf = (Buffer *)gl_malloc(sizeof(Buffer));
	buf->pbuf = (byte *)gl_zalloc(_pbufHeight * _pbufPitch);
	buf->zbuf = (uint *)gl_zalloc(_pbufWidth * _pbufHeight * sizeof(uint));
	return buf;
}

void FrameBuffer::delOffscreenBuffer(Buffer *buf) {
	gl_free(buf->pbuf);
	gl_free(buf->zbuf);
	gl_free(buf);
}

void FrameBuffer::clear(int clearZ, int z, int clearColor, int r, int g, int b,
                        bool clearStencil, int stencilValue) {
	if (clearZ) {
		const uint8 *zc = (const uint8 *)&z;
		uint i;
		for (i = 1; i < sizeof(z) && zc[0] == zc[i]; i++) { ; }
		if (i == sizeof(z)) {
			// All "z" bytes are identical, use memset (fast)
			memset(_zbuf, zc[0], sizeof(uint) * _pbufWidth * _pbufHeight);
		} else {
			// Cannot use memset, use a variant working on integers (slow)
			memset_l(_zbuf, z, _pbufWidth * _pbufHeight);
		}
	}
	if (clearColor) {
		byte *pp = _pbuf.getRawBuffer();
		uint32 color = _pbufFormat.RGBToColor(r, g, b);
		const uint8 *colorc = (uint8 *)&color;
		uint i;
		for (i = 1; i < sizeof(color) && colorc[0] == colorc[i]; i++) { ; }
		if (i == sizeof(color)) {
			// All "color" bytes are identical, use memset (fast)
			memset(pp, colorc[0], _pbufPitch * _pbufHeight);
		} else {
			// Cannot use memset, use a variant working on shorts/ints (slow)
			switch(_pbufBpp) {
			case 2:
				memset_s(pp, color, _pbufWidth * _pbufHeight);
				break;
			case 4:
				memset_l(pp, color, _pbufWidth * _pbufHeight);
				break;
			default:
				error("Unsupported pixel size %i", _pbufBpp);
			}
		}
	}
	if (_sbuf && clearStencil) {
		memset(_sbuf, stencilValue, _pbufWidth * _pbufHeight);
	}
}

void FrameBuffer::clearRegion(int x, int y, int w, int h, bool clearZ, int z,
                              bool clearColor, int r, int g, int b, bool clearStencil, int stencilValue) {
	if (clearZ) {
		int height = h;
		uint *zbuf = _zbuf + (y * _pbufWidth);
		const uint8 *zc = (const uint8 *)&z;
		uint i;
		for (i = 1; i < sizeof(z) && zc[0] == zc[i]; i++) { ; }
		if (i == sizeof(z)) {
			// All "z" bytes are identical, use memset (fast)
			while (height--) {
				memset(zbuf + x, zc[0], sizeof(*zbuf) * w);
				zbuf += _pbufWidth;
			}
		} else {
			// Cannot use memset, use a variant working on integers (slow)
			while (height--) {
				memset_l(zbuf + x, z, w);
				zbuf += _pbufWidth;
			}
		}
	}
	if (clearColor) {
		int height = h;
		byte *pp = _pbuf.getRawBuffer() + y * _pbufPitch + x * _pbufBpp;
		uint32 color = _pbufFormat.RGBToColor(r, g, b);
		const uint8 *colorc = (uint8 *)&color;
		uint i;
		for (i = 1; i < sizeof(color) && colorc[0] == colorc[i]; i++) { ; }
		if (i == sizeof(color)) {
			// All "color" bytes are identical, use memset (fast)
			while (height--) {
				memset(pp, colorc[0], _pbufBpp * w);
				pp += _pbufPitch;
			}
		} else {
			// Cannot use memset, use a variant working on shorts/ints (slow)
			while (height--) {
				switch(_pbufBpp) {
				case 2:
					memset_s(pp, color, w);
					break;
				case 4:
					memset_l(pp, color, w);
					break;
				default:
					error("Unsupported pixel size %i", _pbufBpp);
				}
				pp += _pbufPitch;
			}
		}
	}
	if (_sbuf && clearStencil) {
		byte *pp = _sbuf + y * _pbufWidth + x;
		for (int i = y; i < y + h; i++) {
			memset(pp, stencilValue, w);
			pp += _pbufWidth;
		}
	}
}

inline static void blitPixel(uint8 offset, uint *from_z, uint *to_z, uint z_length, byte *from_color, byte *to_color, uint color_length) {
	const uint d = from_z[offset];
	if (d > to_z[offset]) {
		memcpy(to_color + offset, from_color + offset, color_length);
		memcpy(to_z + offset, &d, z_length);
	}
}

void FrameBuffer::blitOffscreenBuffer(Buffer *buf) {
	// TODO: could be faster, probably.
#define UNROLL_COUNT 16
	if (buf->used) {
		const int pixel_bytes = _pbufBpp;
		const int unrolled_pixel_bytes = pixel_bytes * UNROLL_COUNT;
		byte *to = _pbuf.getRawBuffer();
		byte *from = buf->pbuf;
		uint *to_z = _zbuf;
		uint *from_z = buf->zbuf;
		int count = _pbufWidth * _pbufHeight;
		while (count >= UNROLL_COUNT) {
			blitPixel(0x0, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0x1, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0x2, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0x3, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0x4, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0x5, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0x6, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0x7, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0x8, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0x9, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0xA, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0xB, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0xC, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0xD, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0xE, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			blitPixel(0xF, from_z, to_z, sizeof(int), from, to, pixel_bytes);
			count -= UNROLL_COUNT;
			to += unrolled_pixel_bytes;
			from += unrolled_pixel_bytes;
			to_z += UNROLL_COUNT;
		}
		switch (count) {
		case 0xF: blitPixel(0xE, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0xE: blitPixel(0xD, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0xD: blitPixel(0xC, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0xC: blitPixel(0xB, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0xB: blitPixel(0xA, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0xA: blitPixel(0x9, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x9: blitPixel(0x8, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x8: blitPixel(0x7, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x7: blitPixel(0x6, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x6: blitPixel(0x5, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x5: blitPixel(0x4, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x4: blitPixel(0x3, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x3: blitPixel(0x2, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x2: blitPixel(0x1, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x1: blitPixel(0x0, from_z, to_z, sizeof(int), from, to, pixel_bytes); // fall through
		case 0x0: break;
		}
	}
#undef UNROLL_COUNT
}

void FrameBuffer::selectOffscreenBuffer(Buffer *buf) {
	if (buf) {
		_pbuf = buf->pbuf;
		_zbuf = buf->zbuf;
		buf->used = true;
	} else {
		_pbuf = _offscreenBuffer.pbuf;
		_zbuf = _offscreenBuffer.zbuf;
	}
}

void FrameBuffer::clearOffscreenBuffer(Buffer *buf) {
	memset(buf->pbuf, 0, _pbufHeight * _pbufPitch);
	memset(buf->zbuf, 0, _pbufHeight * _pbufWidth * sizeof(uint));
	buf->used = false;
}

void getSurfaceRef(Graphics::Surface &surface) {
	GLContext *c = gl_get_context();
	assert(c->fb);
	c->fb->getSurfaceRef(surface);
}

Graphics::Surface *copyFromFrameBuffer(const Graphics::PixelFormat &dstFormat) {
	GLContext *c = gl_get_context();
	assert(c->fb);
	return c->fb->copyFromFrameBuffer(dstFormat);
}

} // end of namespace TinyGL
