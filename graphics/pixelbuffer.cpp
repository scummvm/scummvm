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
	: _buffer(NULL),
	_dispose(DisposeAfterUse::NO) {
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
	memset(_buffer, 0, length * _format.bytesPerPixel);
}

void PixelBuffer::copyBuffer(int thisFrom, int otherFrom, int length, const PixelBuffer &buf) {
	if (buf._format.bytesPerPixel == _format.bytesPerPixel &&
	    buf._format.rShift == _format.rShift &&
	    buf._format.gShift == _format.gShift &&
	    buf._format.bShift == _format.bShift &&
	    buf._format.rLoss == _format.rLoss &&
	    buf._format.gLoss == _format.gLoss &&
	    buf._format.bLoss == _format.bLoss && (
		_format.aLoss == 8 ||
		buf._format.aLoss == _format.aLoss
	    )
	) {
		memcpy(_buffer + thisFrom * _format.bytesPerPixel, buf._buffer + otherFrom * _format.bytesPerPixel, length * _format.bytesPerPixel);
	} else {
		uint8 r, g, b, a;
		for (int i = 0; i < length; ++i) {
			buf.getARGBAt(i + otherFrom, a, r, g, b);
			setPixelAt(i + thisFrom, a, r, g, b);
		}
	}
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
