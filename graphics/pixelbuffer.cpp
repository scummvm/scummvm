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

#include "graphics/pixelbuffer.h"

namespace Graphics {

PixelBuffer::PixelBuffer()
	: _buffer(NULL),
	  _dispose(DisposeAfterUse::NO) {

}

PixelBuffer::PixelBuffer(const PixelFormat &format, int buffersize, DisposeAfterUse::Flag dispose)
	: _buffer(NULL) {
	create(format, buffersize, dispose);
}

PixelBuffer::PixelBuffer(const PixelFormat &format, byte *buffer)
	: _buffer(buffer),
	  _format(format),
	  _dispose(DisposeAfterUse::NO) {

}

PixelBuffer::PixelBuffer(const PixelBuffer &buf) {
	*this = buf;
}

PixelBuffer::~PixelBuffer() {
	if (_dispose == DisposeAfterUse::YES)
		free();
}

void PixelBuffer::create(const Graphics::PixelFormat &format, int buffersize, DisposeAfterUse::Flag dispose) {
	if (_dispose == DisposeAfterUse::YES)
		free();

	_format = format;
	_dispose = dispose;
	_buffer = new byte[buffersize * format.bytesPerPixel];
}

void PixelBuffer::create(int buffersize, DisposeAfterUse::Flag dispose) {
	if (_dispose == DisposeAfterUse::YES)
		free();

	_dispose = dispose;
	_buffer = new byte[buffersize * _format.bytesPerPixel];
}

void PixelBuffer::set(const Graphics::PixelFormat &format, byte *buffer) {
	if (_dispose == DisposeAfterUse::YES)
		free();

	_format = format;
	_buffer = buffer;
}

void PixelBuffer::free() {
	delete[] _buffer;
	_buffer = NULL;
}

void PixelBuffer::clear(int length) {
	memset(_buffer, 0, length);
}

void PixelBuffer::setPixelAt(int pixel, uint32 value) {
#if defined(SCUMM_BIG_ENDIAN)
	byte *buffer = _buffer + pixel * _format.bytesPerPixel;
	for (int i = 0; i < _format.bytesPerPixel; ++i) {
		buffer[i] = value >> ((_format.bytesPerPixel - i - 1) * 8) & 0xFF;
	}
#elif defined(SCUMM_LITTLE_ENDIAN)
	byte *buffer = _buffer + pixel * _format.bytesPerPixel;
	for (int i = 0; i < _format.bytesPerPixel; ++i) {
		buffer[i] = value >> (i * 8) & 0xFF;
	}
#endif
}

void PixelBuffer::copyBuffer(int thisFrom, int otherFrom, int length, const PixelBuffer &buf) {
	if (buf._format == _format) {
		memcpy(_buffer + thisFrom * _format.bytesPerPixel, buf._buffer + otherFrom * _format.bytesPerPixel, length * _format.bytesPerPixel);
	} else {
		uint8 r, g, b;
		for (int i = 0; i < length; ++i) {
			buf.getRGBAt(i + otherFrom, r, g, b);
			setPixelAt(i + thisFrom, r, g, b);
		}
	}
}

uint32 PixelBuffer::getValueAt(int i) const {
#if defined(SCUMM_BIG_ENDIAN)
	byte *buffer = _buffer + i * _format.bytesPerPixel;
	uint32 p = buffer[0] << ((_format.bytesPerPixel - 1) * 8);
	for (int l = 1; l < _format.bytesPerPixel; ++l) {
		p = p | (buffer[l] << (8 * (_format.bytesPerPixel - l - 1)));
	}
	return p;
#elif defined(SCUMM_LITTLE_ENDIAN)
	byte *buffer = _buffer + i * _format.bytesPerPixel;
	uint32 p = buffer[0];
	for (int l = 1; l < _format.bytesPerPixel; ++l) {
		p = p | (buffer[l] << (8 * l));
	}
	return p;
#endif
}

PixelBuffer &PixelBuffer::operator=(const PixelBuffer &buf) {
	_buffer = buf._buffer;
	_format = buf._format;
	_dispose = DisposeAfterUse::NO;

	return *this;
}

PixelBuffer &PixelBuffer::operator=(byte *buffer) {
	_buffer = buffer;
	_dispose = DisposeAfterUse::NO;

	return *this;
}

}