/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

// Z buffer: 16,32 bits Z / 16 bits color

#include "common/scummsys.h"
#include "common/endian.h"

#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

// adr must be aligned on an 'int'
void memset_s(void *adr, int val, int count) {
	int n, v;
	unsigned int *p;
	unsigned short *q;

	p = (unsigned int *)adr;
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

void memset_l(void *adr, int val, int count) {
	int n, v;
	unsigned int *p;

	p = (unsigned int *)adr;
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

FrameBuffer::FrameBuffer(int width, int height, const Graphics::PixelBuffer &frame_buffer) : _depthWrite(true), _enableScissor(false) {
	this->xsize = width;
	this->ysize = height;
	this->cmode = frame_buffer.getFormat();
	this->pixelbytes = this->cmode.bytesPerPixel;
	this->linesize = (xsize * this->pixelbytes + 3) & ~3;

	int size = this->xsize * this->ysize * sizeof(unsigned int);

	this->_zbuf = (unsigned int *)gl_malloc(size);
	memset(this->_zbuf, 0, size);

	this->frame_buffer_allocated = 0;
	this->pbuf = frame_buffer;

	this->current_texture = NULL;
	this->shadow_mask_buf = NULL;

	this->buffer.pbuf = this->pbuf.getRawBuffer();
	this->buffer.zbuf = this->_zbuf;
	_blendingEnabled = false;
	_alphaTestEnabled = false;
	_depthTestEnabled = false;
	_depthFunc = TGL_LESS;
}

FrameBuffer::FrameBuffer(int width, int height, const Graphics::PixelFormat &format) : _depthWrite(true), _enableScissor(false) {
	this->xsize = width;
	this->ysize = height;
	this->cmode = format;
	this->pixelbytes = this->cmode.bytesPerPixel;
	this->linesize = (xsize * this->pixelbytes + 3) & ~3;

	int size = this->xsize * this->ysize * sizeof(unsigned int);

	this->_zbuf = (unsigned int *)gl_malloc(size);
	memset(this->_zbuf, 0, size);

	byte *pixelBuffer = (byte *)gl_malloc(this->ysize * this->linesize);
	this->pbuf.set(this->cmode, pixelBuffer);
	this->frame_buffer_allocated = 1;

	this->current_texture = NULL;
	this->shadow_mask_buf = NULL;

	this->buffer.pbuf = this->pbuf.getRawBuffer();
	this->buffer.zbuf = this->_zbuf;
	_blendingEnabled = false;
	_alphaTestEnabled = false;
	_depthTestEnabled = false;
	_depthFunc = TGL_LESS;
}

FrameBuffer::~FrameBuffer() {
	if (frame_buffer_allocated)
		pbuf.free();
	gl_free(_zbuf);
}

Buffer *FrameBuffer::genOffscreenBuffer() {
	Buffer *buf = (Buffer *)gl_malloc(sizeof(Buffer));
	buf->pbuf = (byte *)gl_malloc(this->ysize * this->linesize);
	int size = this->xsize * this->ysize * sizeof(unsigned int);
	buf->zbuf = (unsigned int *)gl_malloc(size);

	return buf;
}

void FrameBuffer::delOffscreenBuffer(Buffer *buf) {
	gl_free(buf->pbuf);
	gl_free(buf->zbuf);
	gl_free(buf);
}

void FrameBuffer::clear(int clearZ, int z, int clearColor, int r, int g, int b) {
	if (clearZ) {
		const uint8 *zc = (const uint8 *)&z;
		unsigned int i;
		for (i = 1; i < sizeof(z) && zc[0] == zc[i]; i++) { ; }
		if (i == sizeof(z)) {
			// All "z" bytes are identical, use memset (fast)
			memset(this->_zbuf, zc[0], sizeof(*this->_zbuf) * this->xsize * this->ysize);
		} else {
			// Cannot use memset, use a variant working on integers (slow)
			memset_l(this->_zbuf, z, this->xsize * this->ysize);
		}
	}
	if (clearColor) {
		byte *pp = this->pbuf.getRawBuffer();
		uint32 color = this->cmode.RGBToColor(r, g, b);
		const uint8 *colorc = (uint8 *)&color;
		unsigned int i;
		for (i = 1; i < sizeof(color) && colorc[0] == colorc[i]; i++) { ; }
		if (i == sizeof(color)) {
			// All "color" bytes are identical, use memset (fast)
			memset(pp, colorc[0], this->linesize * this->ysize);
		} else {
			// Cannot use memset, use a variant working on shorts/ints (slow)
			switch(this->pixelbytes) {
			case 2:
				memset_s(pp, color, this->xsize * this->ysize);
				break;
			case 4:
				memset_l(pp, color, this->xsize * this->ysize);
				break;
			default:
				error("Unsupported pixel size %i", this->pixelbytes);
			}
		}
	}
}

void FrameBuffer::clearRegion(int x, int y, int w, int h, int clearZ, int z, int clearColor, int r, int g, int b) {
	if (clearZ) {
		int height = h;
		unsigned int *zbuf = this->_zbuf + (y * this->xsize);
		const uint8 *zc = (const uint8 *)&z;
		unsigned int i;
		for (i = 1; i < sizeof(z) && zc[0] == zc[i]; i++) { ; }
		if (i == sizeof(z)) {
			// All "z" bytes are identical, use memset (fast)
			while (height--) {
				memset(zbuf + x, zc[0], sizeof(*zbuf) * w);
				zbuf += this->xsize;
			}
		} else {
			// Cannot use memset, use a variant working on integers (slow)
			while (height--) {
				memset_l(zbuf + x, z, w);
				zbuf += this->xsize;
			}
		}
	}
	if (clearColor) {
		int height = h;
		byte *pp = this->pbuf.getRawBuffer() + y * this->linesize + x * this->pixelbytes;
		uint32 color = this->cmode.RGBToColor(r, g, b);
		const uint8 *colorc = (uint8 *)&color;
		unsigned int i;
		for (i = 1; i < sizeof(color) && colorc[0] == colorc[i]; i++) { ; }
		if (i == sizeof(color)) {
			// All "color" bytes are identical, use memset (fast)
			while (height--) {
				memset(pp, colorc[0], this->pixelbytes * w);
				pp += this->linesize;
			}
		} else {
			// Cannot use memset, use a variant working on shorts/ints (slow)
			while (height--) {
				switch(this->pixelbytes) {
				case 2:
					memset_s(pp, color, w);
					break;
				case 4:
					memset_l(pp, color, w);
					break;
				default:
					error("Unsupported pixel size %i", this->pixelbytes);
				}
				pp += this->linesize;
			}
		}
	}
}

inline static void blitPixel(uint8 offset, unsigned int *from_z, unsigned int *to_z, unsigned int z_length, byte *from_color, byte *to_color, unsigned int color_length) {
	const unsigned int d = from_z[offset];
	if (d > to_z[offset]) {
		memcpy(to_color + offset, from_color + offset, color_length);
		memcpy(to_z + offset, &d, z_length);
	}
}

void FrameBuffer::blitOffscreenBuffer(Buffer *buf) {
	// TODO: could be faster, probably.
#define UNROLL_COUNT 16
	if (buf->used) {
		const int pixel_bytes = this->pixelbytes;
		const int unrolled_pixel_bytes = pixel_bytes * UNROLL_COUNT;
		byte *to = this->pbuf.getRawBuffer();
		byte *from = buf->pbuf;
		unsigned int *to_z = this->_zbuf;
		unsigned int *from_z = buf->zbuf;
		int count = this->xsize * this->ysize;
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
		this->pbuf = buf->pbuf;
		this->_zbuf = buf->zbuf;
		buf->used = true;
	} else {
		this->pbuf = this->buffer.pbuf;
		this->_zbuf = this->buffer.zbuf;
	}
}

void FrameBuffer::clearOffscreenBuffer(Buffer *buf) {
	memset(buf->pbuf, 0, this->ysize * this->linesize);
	memset(buf->zbuf, 0, this->ysize * this->xsize * sizeof(unsigned int));
	buf->used = false;
}

void FrameBuffer::setTexture(const Graphics::TexelBuffer *texture, unsigned int wraps, unsigned int wrapt) {
	current_texture = texture;
	wrapS = wraps;
	wrapT = wrapt;
}

} // end of namespace TinyGL
