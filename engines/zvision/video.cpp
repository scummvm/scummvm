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

#include "engines/engine.h"
#include "graphics/decoders/tga.h"

#include "zvision/zork_avi_decoder.h"
#include "zvision/zork_raw.h"

#include "common/EventRecorder.h"
#include "common/file.h"

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

void playVideo(Video::VideoDecoder *videoDecoder /*, VideoState videoState*/) {
	if (!videoDecoder)
		return;

	videoDecoder->start();

	byte *scaleBuffer = 0;
	byte bytesPerPixel = videoDecoder->getPixelFormat().bytesPerPixel;
	uint16 width = videoDecoder->getWidth();
	uint16 height = videoDecoder->getHeight();
	uint16 pitch = videoDecoder->getWidth() * bytesPerPixel;
	uint16 screenWidth = 640;//g_sci->_gfxScreen->getDisplayWidth();
	uint16 screenHeight = 480;//g_sci->_gfxScreen->getDisplayHeight();

	bool zoom2x = true;

	if (zoom2x) {
		width *= 2;
		height *= 2;
		pitch *= 2;
		scaleBuffer = new byte[width * height * bytesPerPixel];
	}

	uint16 x, y;

	x = (screenWidth - width) / 2;
	y = (screenHeight - height) / 2;

	bool skipVideo = false;

	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();

			if (frame) {
				if (scaleBuffer) {
					scale2x((byte *)frame->pixels, scaleBuffer, videoDecoder->getWidth(), videoDecoder->getHeight(), bytesPerPixel);
					g_system->copyRectToScreen(scaleBuffer, pitch, x, y, width, height);
				} else {
					g_system->copyRectToScreen(frame->pixels, frame->pitch, x, y, width, height);
				}

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}

	delete[] scaleBuffer;
	delete videoDecoder;
}

void tests() {
#if 0
	// Video test
	Video::VideoDecoder *videoDecoder = new ZorkAVIDecoder();
	if (videoDecoder && videoDecoder->loadFile("zassets/temple/t000a11c.avi")) {
		Common::List<Graphics::PixelFormat> formats;
		formats.push_back(videoDecoder->getPixelFormat());
		//initGraphics(640, 480, true, formats);

		playVideo(videoDecoder);
	}
#endif

	Common::File f;

#if 0
	// Image test

	//initGraphics(640, 480, true, &format);

	if (f.open("zassets/castle/cb8eb11c.tga")) {
		Graphics::TGADecoder tga;
		if (!tga.loadStream(f))
			error("Error while reading TGA image");
		f.close();

		const Graphics::Surface *tgaSurface = tga.getSurface();

		Graphics::Surface *screen = g_system->lockScreen();
		for (uint16 y = 0; y < tgaSurface->h; y++)
			memcpy(screen->getBasePtr(0, y), tgaSurface->getBasePtr(0, y), tgaSurface->pitch);
		g_system->unlockScreen();

		tga.destroy();
	}


#endif

#if 0
	// Sound test
	if (f.open("zassets/castle/c000h9tc.raw")) {
		Audio::SeekableAudioStream *audioStream = makeRawZorkStream(&f, 22050, DisposeAfterUse::YES);
		Audio::SoundHandle handle;
		g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &handle, audioStream);
	}

#endif
}

} // End of namespace ZVision
