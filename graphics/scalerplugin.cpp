/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
 */

#include "graphics/scalerplugin.h"

SourceScaler::SourceScaler() : _oldSrc(NULL), _enable(false) {
}

SourceScaler::~SourceScaler() {
	if (_oldSrc != NULL)
		delete[] _oldSrc;
}

void SourceScaler::setSource(const byte *src, uint pitch, int width, int height, int padding) {
	if (_oldSrc != NULL)
		delete[] _oldSrc;

	_padding = padding;
	// Give _oldSrc same pitch
	int size = (height + padding * 2) * pitch;
	_oldSrc = new byte[size];
	memset(_oldSrc, 0, size);
}

void SourceScaler::scale(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr,
                         uint32 dstPitch, int width, int height, int x, int y) {
	if (!_enable) {
		// Do not pass _oldSrc, do not update _oldSrc
		internScale(srcPtr, srcPitch,
					dstPtr, dstPitch,
					NULL, 0,
					width, height);
		return;
	}
	int offset = (_padding + x) * _format.bytesPerPixel + (_padding + y) * srcPitch;
	// Call user defined scale function
	internScale(srcPtr, srcPitch,
				dstPtr, dstPitch,
				_oldSrc + offset, srcPitch,
				width, height);
	// Update old src
	byte *oldSrc = _oldSrc + offset;
	while (height--) {
		memcpy(oldSrc, srcPtr, width * _format.bytesPerPixel);
		oldSrc += srcPitch;
		srcPtr += srcPitch;
	}
}

