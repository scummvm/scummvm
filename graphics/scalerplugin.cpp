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
 */

#include "graphics/scalerplugin.h"

namespace {
/**
 * Trivial 'scaler' - in fact it doesn't do any scaling but just copies the
 * source to the destination.
 */
template<typename Pixel>
void Normal1x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	// Spot the case when it can all be done in 1 hit
	int BytesPerPixel = sizeof(Pixel);
	if ((srcPitch == BytesPerPixel * (uint)width) && (dstPitch == BytesPerPixel * (uint)width)) {
		memcpy(dstPtr, srcPtr, BytesPerPixel * width * height);
		return;
	}
	while (height--) {
		memcpy(dstPtr, srcPtr, BytesPerPixel * width);
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}
} // End of anonymous namespace

void Scaler::scale(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr,
	                           uint32 dstPitch, int width, int height, int x, int y) {
	if (_factor == 1) {
		if (_format.bytesPerPixel == 2) {
			Normal1x<uint16>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		} else {
			Normal1x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		}
	} else {
		scaleIntern(srcPtr, srcPitch, dstPtr, dstPitch, width, height, x, y);
	}
}

SourceScaler::SourceScaler(const Graphics::PixelFormat &format) : Scaler(format), _width(0), _height(0), _oldSrc(NULL), _enable(false) {
}

SourceScaler::~SourceScaler() {
	if (_oldSrc != NULL)
		delete[] _oldSrc;

	_bufferedOutput.free();
}

void SourceScaler::setSource(const byte *src, uint pitch, int width, int height, int padding) {
	if (_oldSrc != NULL)
		delete[] _oldSrc;

	_width = width;
	_height = height;
	_padding = padding;

	// Give _oldSrc same pitch
	int size = (height + padding * 2) * pitch;
	_oldSrc = new byte[size]();

	_bufferedOutput.create(_width * _factor, _height * _factor, _format);
}

uint SourceScaler::setFactor(uint factor) {
	uint oldFactor = _factor;
	_factor = factor;

	if (factor != oldFactor && _width != 0 && _height != 0) {
		_bufferedOutput.create(_width * _factor, _height * _factor, _format);
	}

	return oldFactor;
}

void SourceScaler::scaleIntern(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr,
						 uint32 dstPitch, int width, int height, int x, int y) {
	if (!_enable) {
		// Do not pass _oldSrc, do not update _oldSrc
		internScale(srcPtr, srcPitch,
		            dstPtr, dstPitch,
		            NULL, 0,
		            width, height,
		            NULL, 0);
		return;
	}
	int offset = (_padding + x) * _format.bytesPerPixel + (_padding + y) * srcPitch;
	// Call user defined scale function
	internScale(srcPtr, srcPitch,
	            dstPtr, dstPitch,
	            _oldSrc + offset, srcPitch,
	            width, height,
	            (uint8 *)_bufferedOutput.getBasePtr(x * _factor, y * _factor), _bufferedOutput.pitch);

	// Update the destination buffer
	byte *buffer = (byte *)_bufferedOutput.getBasePtr(x * _factor, y * _factor);
	for (uint i = 0; i < height * _factor; ++i) {
		memcpy(buffer, dstPtr, width * _factor * _format.bytesPerPixel);
		buffer += _bufferedOutput.pitch;
		dstPtr += dstPitch;
	}

	// Update old src
	byte *oldSrc = _oldSrc + offset;
	while (height--) {
		memcpy(oldSrc, srcPtr, width * _format.bytesPerPixel);
		oldSrc += srcPitch;
		srcPtr += srcPitch;
	}
}

