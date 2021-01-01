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

#ifndef GRAPHICS_PIXELBUFFER_H
#define GRAPHICS_PIXELBUFFER_H

#include "common/types.h"
#include "common/endian.h"
#include "common/textconsole.h"

#include "graphics/colormasks.h"
#include "graphics/pixelformat.h"

namespace Graphics {

/**
 * @defgroup graphics_pixelbuffer Pixel buffers
 * @ingroup graphics
 *
 * @brief Class for managing pixel buffers.
 *
 * @{
 */

class PixelBuffer {
public:
	/**
	 * Create a PixelBuffer.
	 * Convenience syntax for PixelBuffer(createPixelFormat<format>(), buffersize, dispose).
	 */
	template<int format>
	inline static PixelBuffer createBuffer(int buffersize, DisposeAfterUse::Flag dispose) {
		return PixelBuffer(createPixelFormat<format>(), buffersize, dispose);
	}

	/**
	 * Create a PixelBuffer.
	 * Convenience syntax for PixelBuffer(createPixelFormat<format>(), buffer).
	 */
	template<int format>
	inline static PixelBuffer createBuffer(byte *buffer) {
		return PixelBuffer(createPixelFormat<format>(), buffer);
	}

	/**
	 * Construct an empty PixelBuffer.
	 */
	PixelBuffer();
	/**
	 * Construct a PixelBuffer, allocating the internal buffer.
	 *
	 * @param format The format of the pixels in this buffer.
	 * @param buffersize The number of pixels the buffer will store.
	 * @param dispose If YES the internal buffer will be deleted when this object is destroyed,
	 */
	PixelBuffer(const Graphics::PixelFormat &format, int buffersize, DisposeAfterUse::Flag dispose);
	/**
	 * Construct a PixelBuffer, using an already allocated buffer.
	 *
	 * @param format The format of the pixels in this buffer.
	 * @param buffer The raw buffer containing the pixels.
	 */
	PixelBuffer(const Graphics::PixelFormat &format, byte *buffer);
	/**
	 * Copy constructor.
	 * The internal buffer will NOT be duplicated, it will be shared between the instances.
	 */
	PixelBuffer(const PixelBuffer &buf);
	/**
	 * Destroy the object.
	 */
	~PixelBuffer();

	/**
	 * Initialize the buffer.
	 *
	 * @param format The format of the pixels.
	 * @param buffersize The number of pixels the buffer will store.
	 * @param dispose If YES the internal buffer will be deleted when this object is destroyed,
	 */
	void create(const Graphics::PixelFormat &format, int buffersize, DisposeAfterUse::Flag dispose);
	/**
	 * Initialize the buffer, using the already set pixel format.
	 * @note If the pixel format was not set before the results are undefined.
	 *
	 * @param buffersize The number of pixels the buffer will store.
	 * @param dispose If YES the internal buffer will be deleted when this object is destroyed,
	 */
	void create(int buffersize, DisposeAfterUse::Flag dispose);

	/**
	 * Initialize the buffer.
	 *
	 * @param format The format of the pixels in this buffer.
	 * @param buffer The raw buffer containing the pixels.
	 */
	void set(const Graphics::PixelFormat &format, byte *buffer);

	/**
	 * Delete the internal pixel buffer.
	 */
	void free();

	/**
	 * Reset the value of the pixels.
	 *
	 * @param length The length of the buffer, in pixels.
	 */
	void clear(int length);

	/**
	 * Set the value of the pixel at index 'pixel' to 'value',
	 */
	inline void setPixelAt(int pixel, uint32 value) {
		switch (_format.bytesPerPixel) {
		case 2:
			((uint16 *) _buffer)[pixel] = value;
			return;
		case 3:
			pixel *= 3;
#if defined(SCUMM_BIG_ENDIAN)
			_buffer[pixel + 0] = (value >> 16) & 0xFF;
			_buffer[pixel + 1] = (value >> 8) & 0xFF;
			_buffer[pixel + 2] = value & 0xFF;
#elif defined(SCUMM_LITTLE_ENDIAN)
			_buffer[pixel + 0] = value & 0xFF;
			_buffer[pixel + 1] = (value >> 8) & 0xFF;
			_buffer[pixel + 2] = (value >> 16) & 0xFF;
#endif
			return;
		case 4:
			((uint32 *) _buffer)[pixel] = value;
			return;
		}
		error("setPixelAt: Unhandled bytesPerPixel %i", _format.bytesPerPixel);
	}
	/**
	 * Set the value of a pixel. The pixel will be converted from a pixel in another PixelBuffer,
	 * at the same index.
	 *
	 * @param pixel The index of the pixel to set.
	 * @param buf The buffer storing the source pixel.
	 */
	inline void setPixelAt(int pixel, const PixelBuffer &buf) { setPixelAt(pixel, buf, pixel); }
	/**
	 * Set the value of a pixel. The pixel will be converted from a pixel in another PixelBuffer.
	 *
	 * @param thisPix The index of the pixel to set.
	 * @param buf The buffer storing the source pixel.
	 * @param otherPix The index of the source pixel in 'buf'.
	 */
	inline void setPixelAt(int thisPix, const PixelBuffer &buf, int otherPix) {
		if (_format == buf._format) {
			memcpy(getRawBuffer(thisPix), buf.getRawBuffer(otherPix), _format.bytesPerPixel);
			return;
		}
		uint8 a, r, g, b;
		buf.getARGBAt(otherPix, a, r, g, b);
		setPixelAt(thisPix, a, r, g, b);
	}
	/**
	 * Set a pixel, from RGB values.
	 */
	inline void setPixelAt(int pixel, uint8 r, uint8 g, uint8 b) { setPixelAt(pixel, _format.RGBToColor(r, g, b)); }
	/**
	 * Set a pixel, from ARGB values.
	 */
	inline void setPixelAt(int pixel, uint8 a, uint8 r, uint8 g, uint8 b) { setPixelAt(pixel, _format.ARGBToColor(a, r, g, b)); }

	/**
	 * Copy some pixels from a buffer. The pixels will be converted, storing the same ARGB value.
	 *
	 * @param from The starting index.
	 * @param length The number of pixels to copy.
	 * @param buf The source buffer.
	 */
	inline void copyBuffer(int from, int length, const PixelBuffer &buf) { copyBuffer(from, from, length, buf); }
	/**
	 * Copy some pixels from a buffer. The pixels will be converted, storing the same ARGB value.
	 *
	 * @param thisFrom The starting index.
	 * @param otherFrom The starting index in the source buffer.
	 * @param length The number of pixels to copy.
	 * @param buf The source buffer.
	 */
	void copyBuffer(int thisFrom, int otherFrom, int length, const PixelBuffer &buf);

	/**
	 * Shift the internal buffer of some pixels, losing some pixels at the start of the buffer.
	 * The pixels lost will NOT be deleted.
	 */
	inline void shiftBy(int amount) { _buffer += amount * _format.bytesPerPixel; }

	/**
	 * Return the encoded value of the pixel at the given index.
	 */
	inline uint32 getValueAt(int i) const {
		switch (_format.bytesPerPixel) {
		case 2:
			return ((uint16 *) _buffer)[i];
		case 3:
			i *= 3;
#if defined(SCUMM_BIG_ENDIAN)
			return (_buffer[i + 0] << 16) | (_buffer[i + 1] << 8) | _buffer[i + 2];
#elif defined(SCUMM_LITTLE_ENDIAN)
			return _buffer[i + 0] | (_buffer[i + 1] << 8) | (_buffer[i + 2] << 16);
#endif
		case 4:
			return ((uint32 *) _buffer)[i];
		}
		error("getValueAt: Unhandled bytesPerPixel %i", _format.bytesPerPixel);
	}
	/**
	 * Return the RGB value of the pixel at the given index.
	 */
	inline void getRGBAt(int i, uint8 &r, uint8 &g, uint8 &b) const { _format.colorToRGB(getValueAt(i), r, g, b); }
	/**
	 * Return the ARGB value of the pixel at the given index.
	 */
	inline void getARGBAt(int i, uint8 &a, uint8 &r, uint8 &g, uint8 &b) const { _format.colorToARGB(getValueAt(i), a, r, g, b); }

	/**
	 * Return the internal buffer.
	 */
	inline byte *getRawBuffer() const { return _buffer; }
	/**
	 * Return the internal buffer, pointing at the wanted pixel.
	 */
	inline byte *getRawBuffer(int pixel) const { return _buffer + _format.bytesPerPixel * pixel; }

	/**
	 * Return the pixel format used.
	 */
	inline const PixelFormat &getFormat() const { return _format; }

	/**
	 * Copy a PixelBuffer object.
	 * The internal buffer will NOT be duplicated, it will be shared between the instances.
	 */
	PixelBuffer &operator=(const PixelBuffer &buf);
	/**
	 * Set the internal buffer to an already allocated array.
	 *
	 * @param buffer The pointer to the array.
	 */
	PixelBuffer &operator=(byte *buffer);

	/**
	 * Check if the interal buffer is allocated.
	 *
	 * @returns true if allocated.
	 */
	inline operator bool() const { return (_buffer); }

private:
	byte *_buffer;
	Graphics::PixelFormat _format;
	DisposeAfterUse::Flag _dispose;
};

}
 /** @} */
#endif
