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

#include "zvision/clock.h"
#include "zvision/render_manager.h"
#include "zvision/zvision.h"


namespace ZVision {

// Taken from SCI
void scale2x(const byte *src, byte *dst, uint32 srcWidth, uint32 srcHeight, byte bytesPerPixel) {
	assert(bytesPerPixel == 1 || bytesPerPixel == 2);
	const uint32 newWidth = srcWidth * 2;
	const uint32 pitch = newWidth * bytesPerPixel;
	const byte *srcPtr = src;

	if (bytesPerPixel == 1) {
		for (uint32 y = 0; y < srcHeight; y++) {
			for (uint32 x = 0; x < srcWidth; x++) {
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
		for (uint32 y = 0; y < srcHeight; y++) {
			for (uint32 x = 0; x < srcWidth; x++) {
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

void ZVision::playVideo(Video::VideoDecoder &videoDecoder) {
	// Videos use a different pixel format
	Common::List<Graphics::PixelFormat> formats;
	formats.push_back(videoDecoder.getPixelFormat());
	initGraphics(_width, _height, true, formats);

	byte bytesPerPixel = videoDecoder.getPixelFormat().bytesPerPixel;
	uint16 origWidth = videoDecoder.getWidth();
	uint16 origHeight = videoDecoder.getHeight();
	uint16 pitch = origWidth * bytesPerPixel;

	// Most videos are very small. Therefore we do a simple 2x scale
	bool shouldBeScaled = (origWidth * 2 <= 640 && origHeight * 2 <= 480);
	uint16 finalWidth = shouldBeScaled ? origWidth * 2 : origWidth;
	uint16 finalHeight = shouldBeScaled ? origHeight * 2 : origHeight;

	byte *scaledVideoFrameBuffer = new byte[origHeight * pitch * 4];

	uint16 x = (_width - finalWidth) / 2;
	uint16 y = (_height - finalHeight) / 2;

	_clock.stop();
	videoDecoder.start();

	// Only continue while the video is still playing
	while (videoDecoder.isPlaying()) {
		_clock.update();
		uint32 currentTime = _clock.getLastMeasuredTime();

		// Check for engine quit and video stop key presses
		while (_eventMan->pollEvent(_event)) {
			switch (_event.type) {
			case Common::EVENT_KEYDOWN:
				switch (_event.kbd.keycode) {
				case Common::KEYCODE_q:
					if (_event.kbd.hasFlags(Common::KBD_CTRL))
						quitGame();
					break;
				case Common::KEYCODE_SPACE:
					videoDecoder.stop();
					break;
				}
			}
		}

		if (videoDecoder.needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder.decodeNextFrame();

			if (frame) {
				if (shouldBeScaled) {
					scale2x((byte *)frame->pixels, scaledVideoFrameBuffer, origWidth, origHeight, bytesPerPixel);
					_system->copyRectToScreen(scaledVideoFrameBuffer, pitch * 2, x, y, finalWidth, finalHeight);
				} else {
					_system->copyRectToScreen((byte *)frame->pixels, pitch, x, y, finalWidth, finalHeight);
				}

				_system->updateScreen();
			}
		}

		// Calculate the frame delay based off a desired frame time
		int delay = _desiredFrameTime - (currentTime - _system->getMillis());
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;
		_system->delayMillis(delay);
	}

	_clock.stop();

	// Reset the pixel format to the original state
	initGraphics(_width, _height, true, &_pixelFormat);
}

} // End of namespace ZVision
