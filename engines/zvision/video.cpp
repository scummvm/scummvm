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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "common/system.h"
#include "engines/util.h"

#include "graphics/surface.h"

#include "zvision/zvision.h"


namespace ZVision {

// Taken from SCI
void scale2x(const byte *src, byte *dst, int16 srcWidth, int16 srcHeight, byte bytesPerPixel) {
	assert(bytesPerPixel == 1 || bytesPerPixel == 2);
	const int newWidth = srcWidth * 2;
	const int pitch = newWidth * bytesPerPixel;
	const byte *srcPtr = src;

	if (bytesPerPixel == 1) {
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				const byte color = *srcPtr++;
				dst[0] = color;
				dst[1] = color;
				dst[newWidth] = color;
				dst[newWidth + 1] = color;
				dst += 2;
			}
			dst += newWidth;
		}
	} else if (bytesPerPixel == 2) {
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				const byte color = *srcPtr++;
				const byte color2 = *srcPtr++;
				dst[0] = color;
				dst[1] = color2;
				dst[2] = color;
				dst[3] = color2;
				dst[pitch] = color;
				dst[pitch + 1] = color2;
				dst[pitch + 2] = color;
				dst[pitch + 3] = color2;
				dst += 4;
			}
			dst += pitch;
		}
	}
}

void ZVision::startVideo(Video::VideoDecoder *videoDecoder) {
	if (!videoDecoder)
		return;

	_currentVideo = videoDecoder;

	Common::List<Graphics::PixelFormat> formats;
	formats.push_back(videoDecoder->getPixelFormat());
	initGraphics(640, 480, true, formats);
	_currentVideo->start();

	// Load the first frame
	continueVideo();
}

void ZVision::continueVideo() {
	if (_currentVideo == 0) {
		warning("No video loaded. Nothing to continue");
		return;
	}

	byte bytesPerPixel = _currentVideo->getPixelFormat().bytesPerPixel;
	uint16 width = _currentVideo->getWidth();
	uint16 height = _currentVideo->getHeight();
	uint16 pitch = _currentVideo->getWidth() * bytesPerPixel;

	uint16 x = (_system->getWidth() - width) / 2;
	uint16 y = (_system->getWidth() - height) / 2;

	if (!_currentVideo->endOfVideo()) {
		if (_currentVideo->needsUpdate()) {
			const Graphics::Surface *frame = _currentVideo->decodeNextFrame();

			if (frame) {
				_system->copyRectToScreen(frame->pixels, pitch, x, y, width, height);

				_needsScreenUpdate = true;
			}
		}
	} else {
		initGraphics(_width, _height, true, &_pixelFormat);
		delete _currentVideo;
		_currentVideo = 0;
	}

}

} // End of namespace ZVision
