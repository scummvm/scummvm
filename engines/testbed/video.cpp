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

#include "common/events.h"
#include "engines/util.h"
#include "video/qt_decoder.h"

#include "testbed/testbed.h"

namespace Testbed {

void TestbedEngine::videoTest() {
	Graphics::PixelFormat pixelformat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);

	initGraphics(640, 480, &pixelformat);

	Common::String path = ConfMan.get("start_movie");

	Video::VideoDecoder *video = new Video::QuickTimeDecoder();

	if (!video->loadFile(path)) {
		warning("Cannot open video %s", path.c_str());
		return;
	}

	video->start();

	while (!video->endOfVideo()) {
		if (video->needsUpdate()) {
			uint32 pos = video->getTime();
			warning("video time: %d", pos);

			const Graphics::Surface *frame = video->decodeNextFrame();
			if (frame) {
				Graphics::Surface *conv = frame->convertTo(pixelformat);

				int x = 0, y = 0;

				if (conv->w < g_system->getWidth() && conv->h < g_system->getHeight()) {
					x = (g_system->getWidth() - conv->w) >> 1;
					y = (g_system->getHeight() - conv->h) >> 1;
				}
				g_system->copyRectToScreen(conv->getPixels(), conv->pitch, x, y, MIN<uint16>(conv->w, 640), MIN<uint16>(conv->h, 480));

				delete conv;
			}

			Common::Event event;

			while (g_system->getEventManager()->pollEvent(event)) {
				if (Engine::shouldQuit()) {
					delete video;
					return;
				}
			}

			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}

	delete video;
}

}
