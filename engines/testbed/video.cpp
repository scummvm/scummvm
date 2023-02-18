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
 *
 */

#include "common/events.h"
#include "engines/util.h"
#include "video/qt_decoder.h"
#include "video/qt_data.h"

#include "testbed/testbed.h"
#include "graphics/palette.h"

namespace Testbed {

void TestbedEngine::videoTest() {
	Graphics::PixelFormat pixelformat = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(640, 480, &pixelformat);

	Common::String path = ConfMan.get("start_movie");

	Video::VideoDecoder *video = new Video::QuickTimeDecoder();

	if (!video->loadFile(path)) {
		warning("Cannot open video %s", path.c_str());
		return;
	}

	const byte *palette = video->getPalette();

	if (!palette) {
		palette = Video::quickTimeDefaultPalette256;
	}
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	video->start();

	while (!video->endOfVideo()) {
		if (video->needsUpdate()) {
			uint32 pos = video->getTime();
			warning("video time: %d", pos);

			const Graphics::Surface *frame = video->decodeNextFrame();
			if (frame) {
				Graphics::Surface *conv = frame->convertTo(pixelformat, 0, 0, palette, 256);

				int x = 0, y = 0;

				if (conv->w < g_system->getWidth() && conv->h < g_system->getHeight()) {
					x = (g_system->getWidth() - conv->w) >> 1;
					y = (g_system->getHeight() - conv->h) >> 1;
				}
				g_system->copyRectToScreen(conv->getPixels(), conv->pitch, x, y, MIN<uint16>(conv->w, 640), MIN<uint16>(conv->h, 480));

				conv->free();
				delete conv;
			}

			Common::Event event;

			while (g_system->getEventManager()->pollEvent(event)) {
				if (Engine::shouldQuit()) {
					video->close();
					delete video;
					return;
				}
			}
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}
	video->close();
	delete video;
}

}
