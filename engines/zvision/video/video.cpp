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
 *
 */

#include "common/scummsys.h"

#include "zvision/zvision.h"

#include "zvision/utility/clock.h"
#include "zvision/graphics/render_manager.h"

#include "common/system.h"

#include "video/video_decoder.h"

#include "engines/util.h"

#include "graphics/surface.h"


namespace ZVision {

// Taken/modified from SCI
void scaleBuffer(const byte *src, byte *dst, uint32 srcWidth, uint32 srcHeight, byte bytesPerPixel, uint scaleAmount) {
	assert(bytesPerPixel == 1 || bytesPerPixel == 2);

	const uint32 newWidth = srcWidth * scaleAmount;
	const uint32 pitch = newWidth * bytesPerPixel;
	const byte *srcPtr = src;

	if (bytesPerPixel == 1) {
		for (uint32 y = 0; y < srcHeight; ++y) {
			for (uint32 x = 0; x < srcWidth; ++x) {
				const byte color = *srcPtr++;

				for (uint i = 0; i < scaleAmount; ++i) {
					dst[i] = color;
					dst[pitch + i] = color;
				}
				dst += scaleAmount;
			}
			dst += pitch;
		}
	} else if (bytesPerPixel == 2) {
		for (uint32 y = 0; y < srcHeight; ++y) {
			for (uint32 x = 0; x < srcWidth; ++x) {
				const byte color = *srcPtr++;
				const byte color2 = *srcPtr++;

				for (uint i = 0; i < scaleAmount; ++i) {
					uint index = i *2;

					dst[index] = color;
					dst[index + 1] = color2;
					dst[pitch + index] = color;
					dst[pitch + index + 1] = color2;
				}
				dst += 2 * scaleAmount;
			}
			dst += pitch;
		}
	}
}

void ZVision::playVideo(Video::VideoDecoder &videoDecoder, const Common::Rect &destRect, bool skippable) {
	byte bytesPerPixel = videoDecoder.getPixelFormat().bytesPerPixel;

	uint16 origWidth = videoDecoder.getWidth();
	uint16 origHeight = videoDecoder.getHeight();

	uint scale = 1;
	// If destRect is empty, no specific scaling was requested. However, we may choose to do scaling anyway
	if (destRect.isEmpty()) {
		// Most videos are very small. Therefore we do a simple 2x scale
		if (origWidth * 2 <= 640 && origHeight * 2 <= 480) {
			scale = 2;
		}
	} else {
		// Assume bilinear scaling. AKA calculate the scale from just the width.
		// Also assume that the scaling is in integral intervals. AKA no 1.5x scaling
		// TODO: Test ^these^ assumptions
		scale = destRect.width() / origWidth;

		// TODO: Test if we need to support downscale.
	}

	uint16 pitch = origWidth * bytesPerPixel;

	uint16 finalWidth = origWidth * scale;
	uint16 finalHeight = origHeight * scale;

	byte *scaledVideoFrameBuffer = 0;
	if (scale != 1) {
		scaledVideoFrameBuffer = new byte[finalWidth * finalHeight * bytesPerPixel];
	}

	uint16 x = ((WINDOW_WIDTH - finalWidth) / 2) + destRect.left;
	uint16 y = ((WINDOW_HEIGHT - finalHeight) / 2) + destRect.top;

	_clock.stop();
	videoDecoder.start();

	// Only continue while the video is still playing
	while (!shouldQuit() && !videoDecoder.endOfVideo() && videoDecoder.isPlaying()) {
		// Check for engine quit and video stop key presses
		while (!videoDecoder.endOfVideo() && videoDecoder.isPlaying() && _eventMan->pollEvent(_event)) {
			switch (_event.type) {
			case Common::EVENT_KEYDOWN:
				switch (_event.kbd.keycode) {
				case Common::KEYCODE_q:
					if (_event.kbd.hasFlags(Common::KBD_CTRL))
						quitGame();
					break;
				case Common::KEYCODE_SPACE:
					if (skippable) {
						videoDecoder.stop();
					}
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		if (videoDecoder.needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder.decodeNextFrame();

			if (frame) {
				if (scale != 1) {
					scaleBuffer((const byte *)frame->getPixels(), scaledVideoFrameBuffer, origWidth, origHeight, bytesPerPixel, scale);
					_system->copyRectToScreen(scaledVideoFrameBuffer, pitch * 2, x, y, finalWidth, finalHeight);
				} else {
					_system->copyRectToScreen((const byte *)frame->getPixels(), pitch, x, y, finalWidth, finalHeight);
				}
			}
		}

		// Always update the screen so the mouse continues to render
		_system->updateScreen();

		_system->delayMillis(videoDecoder.getTimeToNextFrame());
	}

	_clock.start();

	if (scale != 1) {
		delete[] scaledVideoFrameBuffer;
	}
}

} // End of namespace ZVision
