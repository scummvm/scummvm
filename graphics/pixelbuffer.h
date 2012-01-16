/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRAPHICS_PIXELBUFFER_H
#define GRAPHICS_PIXELBUFFER_H

#include "common/types.h"

#include "graphics/pixelformat.h"

namespace Graphics {

class PixelBuffer {
public:
	PixelBuffer();
	PixelBuffer(const Graphics::PixelFormat &format, int buffersize, DisposeAfterUse::Flag dispose);
	PixelBuffer(const Graphics::PixelFormat &format, byte *buffer);
	PixelBuffer(const PixelBuffer &buf);
	~PixelBuffer();

	void create(const Graphics::PixelFormat &format, int buffersize, DisposeAfterUse::Flag dispose);
	void free();

	void setPixelAt(int pixel, uint32 value);
	inline void setPixelAt(int pixel, const PixelBuffer &buf) { setPixelAt(pixel, buf, pixel); }
	inline void setPixelAt(int thisPix, const PixelBuffer &buf, int otherPix) {
		uint8 a, r, g, b;
		buf.getARGBAt(otherPix, a, r, g, b);
		setPixelAt(thisPix, a, r, g, b);
	}
	inline void setPixelAt(int pixel, uint8 r, uint8 g, uint8 b) { setPixelAt(pixel, _format.RGBToColor(r, g, b)); }
	inline void setPixelAt(int pixel, uint8 a, uint8 r, uint8 g, uint8 b) { setPixelAt(pixel, _format.ARGBToColor(a, r, g, b)); }

	void copyBuffer(int from, int length, const PixelBuffer &buf);
	void copyBuffer(int thisFrom, int otherFrom, int length, const PixelBuffer &buf);

	inline void shiftBy(int amount) { _buffer += amount * _format.bytesPerPixel; }

	uint32 getValueAt(int i) const;
	inline void getRGBAt(int i, uint8 &r, uint8 &g, uint8 &b) const { _format.colorToRGB(getValueAt(i), r, g, b); }
	inline void getARGBAt(int i, uint8 &a, uint8 &r, uint8 &g, uint8 &b) const { _format.colorToARGB(getValueAt(i), a, r, g, b); }

	byte *getRawBuffer() const;
	PixelFormat getFormat() const;

	PixelBuffer &operator=(const PixelBuffer &buf);
	PixelBuffer &operator=(byte *buffer);
	inline operator bool() const { return (_buffer); }

private:
	byte *_buffer;
	Graphics::PixelFormat _format;
	DisposeAfterUse::Flag _dispose;
};

}

#endif
