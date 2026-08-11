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
#include "common/memory.h"

#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

FrameBuffer::FrameBuffer(int width, int height, const Graphics::PixelFormat &format, bool enableStencilBuffer) {
	_pbufWidth = width;
	_pbufHeight = height;
	_pbufFormat = format;
	_pbufBpp = _pbufFormat.bytesPerPixel;
	_pbufPitch = (_pbufWidth * _pbufBpp + 3) & ~3;

	_pbuf = (byte *)gl_zalloc(_pbufHeight * _pbufPitch * sizeof(byte));
	_zbuf = (uint *)gl_zalloc(_pbufWidth * _pbufHeight * sizeof(uint));
	if (enableStencilBuffer)
		_sbuf = (byte *)gl_zalloc(_pbufWidth * _pbufHeight * sizeof(byte));
	else
		_sbuf = nullptr;

	_offscreenBuffer.pbuf = _pbuf;
	_offscreenBuffer.zbuf = _zbuf;

	_currentTexture = nullptr;

	_clippingEnabled = false;
}

FrameBuffer::~FrameBuffer() {
	gl_free(_pbuf);
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
	if (_clippingEnabled) {
		clearRegion(_clipRectangle.left, _clipRectangle.top,
				_clipRectangle.width(), _clipRectangle.height(),
				clearZ, z, clearColor, r, g, b, clearStencil, stencilValue);
		return;
	}
	if (clearZ) {
		const uint8 *zc = (const uint8 *)&z;
		uint i;
		for (i = 1; i < sizeof(z) && zc[0] == zc[i]; i++) { ; }
		if (i == sizeof(z)) {
			// All "z" bytes are identical, use memset (fast)
			memset(_zbuf, zc[0], sizeof(uint) * _pbufWidth * _pbufHeight);
		} else {
			// Cannot use memset, use a variant working on integers (possibly slower)
			Common::memset32((uint32 *)_zbuf, z, _pbufWidth * _pbufHeight);
		}
	}
	if (clearColor) {
		byte *pp = _pbuf;
		uint32 color = _pbufFormat.RGBToColor(r, g, b);
		const uint8 *colorc = (uint8 *)&color;
		uint i;
		for (i = 1; i < sizeof(color) && colorc[0] == colorc[i]; i++) { ; }
		if (i == sizeof(color)) {
			// All "color" bytes are identical, use memset (fast)
			memset(pp, colorc[0], _pbufPitch * _pbufHeight);
		} else {
			// Cannot use memset, use a variant working on shorts/ints (possibly slower)
			switch(_pbufBpp) {
			case 2:
				Common::memset16((uint16 *)pp, color, _pbufWidth * _pbufHeight);
				break;
			case 4:
				Common::memset32((uint32 *)pp, color, _pbufWidth * _pbufHeight);
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
		uint *zbuf = _zbuf + (y * _pbufWidth) + x;
		const uint8 *zc = (const uint8 *)&z;
		uint i;
		for (i = 1; i < sizeof(z) && zc[0] == zc[i]; i++) { ; }
		if (i == sizeof(z)) {
			// All "z" bytes are identical, use memset (fast)
			while (height--) {
				memset(zbuf, zc[0], sizeof(*zbuf) * w);
				zbuf += _pbufWidth;
			}
		} else {
			// Cannot use memset, use a variant working on integers (possibly slower)
			while (height--) {
				Common::memset32((uint32 *)zbuf, z, w);
				zbuf += _pbufWidth;
			}
		}
	}
	if (clearColor) {
		int height = h;
		byte *pp = _pbuf + y * _pbufPitch + x * _pbufBpp;
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
			// Cannot use memset, use a variant working on shorts/ints (possibly slower)
			while (height--) {
				switch(_pbufBpp) {
				case 2:
					Common::memset16((uint16 *)pp, color, w);
					break;
				case 4:
					Common::memset32((uint32 *)pp, color, w);
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
		byte *to = _pbuf;
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

static byte satAdd(byte a, byte b) {
	// from: https://web.archive.org/web/20190213215419/https://locklessinc.com/articles/sat_arithmetic/
	byte r = a + b;
	return (byte)(r | -(r < a));
}

static byte sat16_to_8(uint32 x) {
	x = (x + 128) >> 8; // rounding 16 to 8 
	return (byte)(x | -!!(x >> 8)); // branchfree saturation
}

static byte fpMul(byte a, byte b) {
	// from: https://community.khronos.org/t/precision-curiosity-1-255-or-1-256/40539/11
	// correct would be (a*b)/255 but that is slow, instead we use (a*b) * 257/256 / 256
	// this also implicitly saturates
	uint32 r = a * b;
	return (byte)((r + (r >> 8) + 127) >> 8);
}

void FrameBuffer::applyTextureEnvironment(
	int internalformat,
	uint previousA, uint previousR, uint previousG, uint previousB,
	byte &texA, byte &texR, byte &texG, byte &texB)
{
	// summary notation is used from https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glTexEnv.xml
	// previousARGB is still in 16bit fixed-point format
	// texARGB is both input and output
	// GL_RGB/GL_RGBA might be identical as TexelBuffer returns As=1

	struct Arg {
		byte a, r, g, b;
	};
	const auto getCombineArg = [&](const GLTextureEnvArgument &mode) -> Arg {
		Arg op = {}, opColor = {};

		// Source values
		switch (mode.sourceRGB) {
		case TGL_TEXTURE:
			opColor.a = texA;
			opColor.r = texR;
			opColor.g = texG;
			opColor.b = texB;
			break;
		case TGL_PRIMARY_COLOR:
			opColor.a = sat16_to_8(previousA);
			opColor.r = sat16_to_8(previousR);
			opColor.g = sat16_to_8(previousG);
			opColor.b = sat16_to_8(previousB);
			break;
		case TGL_CONSTANT:
			opColor.a = _textureEnv->constA;
			opColor.r = _textureEnv->constR;
			opColor.g = _textureEnv->constG;
			opColor.b = _textureEnv->constB;
			break;
		default:
			assert(false && "Invalid texture environment arg color source");
			break;
		}
		switch (mode.sourceAlpha) {
		case TGL_TEXTURE:
			op.a = texA;
			break;
		case TGL_PRIMARY_COLOR:
			op.a = sat16_to_8(previousA);
			break;
		case TGL_CONSTANT:
			op.a = _textureEnv->constA;
			break;
		default:
			assert(false && "Invalid texture environment arg alpha source");
			break;
		}

		// Operands
		switch (mode.operandRGB) {
		case TGL_SRC_COLOR:
			op.r = opColor.r; // intermediate values were necessary for operandRGB == TGL_SRC_ALPHA 
			op.g = opColor.g;
			op.b = opColor.b;
			break;
		case TGL_ONE_MINUS_SRC_COLOR:
			op.r = 255 - opColor.r;
			op.g = 255 - opColor.g;
			op.b = 255 - opColor.b;
			break;
		case TGL_SRC_ALPHA:
			op.r = op.g = op.b = opColor.a;
			break;
		default:
			assert(false && "Invalid texture environment arg color operand");
			break;
		}
		switch (mode.operandAlpha) {
		case TGL_SRC_ALPHA:
			break;
		case TGL_ONE_MINUS_SRC_ALPHA:
			op.a = 255 - op.a;
			break;
		default:
			assert(false && "Invalid texture environment arg alpha operand");
			break;
		}

		return op;
	};

	switch (_textureEnv->envMode) {
	case TGL_REPLACE:
		// GL_RGB:  Cs | Ap
		// GL_RGBA: Cs | As
		texA = internalformat == TGL_RGBA ? texA : sat16_to_8(previousA);
		break;
	case TGL_MODULATE:
	{
		// GL_RGB:  CpCs | Ap 
		// GL_RGBA: CpCs | ApAs
		applyModulation(previousA, previousR, previousG, previousB, texA, texR, texG, texB);
		break;
	}
	case TGL_DECAL:
	{
		// GL_RGB:  Cs              | Ap
		// GL_RGBA: Cp(1-As) + CsAs | Ap
		texR = satAdd(fpMul(sat16_to_8(previousR), 255 - texA), fpMul(texR, texA));
		texG = satAdd(fpMul(sat16_to_8(previousG), 255 - texA), fpMul(texG, texA));
		texB = satAdd(fpMul(sat16_to_8(previousB), 255 - texA), fpMul(texB, texA));
		texA = sat16_to_8(previousA);
		break;
	}
	case TGL_ADD:
	{
		// GL_RGB:  Cp + Cs | Ap
		// GL_RGBA: Cp + Cs | ApAs
		texA = fpMul(sat16_to_8(previousA), texA);
		texR = satAdd(sat16_to_8(previousR), texR);
		texG = satAdd(sat16_to_8(previousG), texG);
		texB = satAdd(sat16_to_8(previousB), texB);
		break;
	}
	case TGL_BLEND:
	{
		// GL_RGB:  Cp(1-Cs) + CcCs | Ap
		// GL_RGBA: Cp(1-Cs) + CcCs | ApAs
		texA = fpMul(sat16_to_8(previousA), texA);
		texR = satAdd(fpMul(sat16_to_8(previousR), 255 - texR), fpMul(_textureEnv->constR, texR));
		texG = satAdd(fpMul(sat16_to_8(previousG), 255 - texG), fpMul(_textureEnv->constG, texG));
		texB = satAdd(fpMul(sat16_to_8(previousB), 255 - texB), fpMul(_textureEnv->constB, texB));
		break;
	}
	case TGL_COMBINE:
	{
		Arg arg0 = getCombineArg(_textureEnv->arg0);
		Arg arg1 = getCombineArg(_textureEnv->arg1);
		switch (_textureEnv->combineRGB) {
		case TGL_REPLACE:
			texR = arg0.r;
			texG = arg0.g;
			texB = arg0.b;
			break;
		case TGL_MODULATE:
			texR = fpMul(arg0.r, arg1.r);
			texG = fpMul(arg0.g, arg1.g);
			texB = fpMul(arg0.b, arg1.b);
			break;
		case TGL_ADD:
			texR = satAdd(arg0.r, arg1.r);
			texG = satAdd(arg0.g, arg1.g);
			texB = satAdd(arg0.b, arg1.b);
			break;
		default:
			assert(false && "Invalid texture environment color combine");
			break;
		}
		
		switch (_textureEnv->combineAlpha) {
		case TGL_REPLACE:
			texA = arg0.a;
			break;
		case TGL_MODULATE:
			texA = fpMul(arg0.a, arg1.a);
			break;
		case TGL_ADD:
			texA = satAdd(arg0.a, arg1.a);
			break;
		default:
			assert(false && "Invalid texture environment alpha combine");
		}
		break;
	}
	default:
		assert(false && "Invalid texture environment mode");
		break;
	}
}

void FrameBuffer::applyModulation(
	uint previousA, uint previousR, uint previousG, uint previousB,
	byte &texA, byte &texR, byte &texG, byte &texB) {
	texA = fpMul(sat16_to_8(previousA), texA);
	texR = fpMul(sat16_to_8(previousR), texR);
	texG = fpMul(sat16_to_8(previousG), texG);
	texB = fpMul(sat16_to_8(previousB), texB);
}

} // end of namespace TinyGL
