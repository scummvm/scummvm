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
#include "video/video_decoder.h"
#include "engines/util.h"
#include "graphics/surface.h"

#include "zvision/render_manager.h"


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

void RenderManager::startVideo(Video::VideoDecoder *videoDecoder) {
	if (!videoDecoder)
		return;

	_currentVideo = videoDecoder;

	Common::List<Graphics::PixelFormat> formats;
	formats.push_back(videoDecoder->getPixelFormat());
	initGraphics(_width, _height, true, formats);
	
	_scaledVideoFrameBuffer = new byte[_currentVideo->getWidth() * _currentVideo->getHeight() * _currentVideo->getPixelFormat().bytesPerPixel * 4];

	_currentVideo->start();

	// Load the first frame
	continueVideo();
}

void RenderManager::continueVideo() {
	byte bytesPerPixel = _currentVideo->getPixelFormat().bytesPerPixel;
	uint16 width = _currentVideo->getWidth();
	uint16 height = _currentVideo->getHeight();
	uint16 pitch = width * bytesPerPixel;

	uint16 x = (_system->getWidth() - (width * 2)) / 2;
	uint16 y = (_system->getHeight() - (height * 2)) / 2;

	if (!_currentVideo->endOfVideo()) {
		if (_currentVideo->needsUpdate()) {
			const Graphics::Surface *frame = _currentVideo->decodeNextFrame();

			if (frame) {		
				scale2x(static_cast<byte *>(frame->pixels), _scaledVideoFrameBuffer, width, height, bytesPerPixel);
				_system->copyRectToScreen(_scaledVideoFrameBuffer, pitch * 2, x, y, width * 2, height * 2);

				_needsScreenUpdate = true;
			}
		}
	} else {
		cancelVideo();
	}
}

void RenderManager::cancelVideo() {
	initGraphics(_width, _height, true, &_pixelFormat);
	delete _currentVideo;
	_currentVideo = 0;
	delete[] _scaledVideoFrameBuffer;
	_scaledVideoFrameBuffer = 0;
}

} // End of namespace ZVision
