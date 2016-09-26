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

#include "common/system.h"
#include "common/events.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"

#include "chewy/graphics.h"
#include "chewy/resource.h"
#include "chewy/video/cfo_decoder.h"

namespace Chewy {

void Graphics::drawImage(Common::String filename, int imageNum) {
	BackgroundResource *res = new BackgroundResource(filename);
	TBFChunk *image = res->getImage(imageNum);

	g_system->getPaletteManager()->setPalette(image->palette, 0, 256);
	g_system->copyRectToScreen(image->data, image->width, 0, 0, image->width, image->height);
	g_system->updateScreen();

	delete[] image->data;
	delete image;
	delete res;
}

void Graphics::playVideo(uint num) {
	CfoDecoder *cfoDecoder = new CfoDecoder();
	VideoResource *videoResource = new VideoResource("cut.tap");
	Common::SeekableReadStream *videoStream = videoResource->getVideoStream(num);

	if (!cfoDecoder->loadStream(videoStream)) {
		delete videoResource;
		delete cfoDecoder;
		return;
	}

	uint16 x = (g_system->getWidth() - cfoDecoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - cfoDecoder->getHeight()) / 2;
	bool skipVideo = false;

	cfoDecoder->start();

	while (!g_engine->shouldQuit() && !cfoDecoder->endOfVideo() && !skipVideo) {
		if (cfoDecoder->needsUpdate()) {
			const ::Graphics::Surface *frame = cfoDecoder->decodeNextFrame();
			if (frame) {
				g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);

				if (cfoDecoder->hasDirtyPalette())
					g_system->getPaletteManager()->setPalette(cfoDecoder->getPalette(), 0, 256);

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

	cfoDecoder->close();
}

void Graphics::setCursor(uint num) {
	SpriteResource *res = new SpriteResource("cursor.taf");
	TAFChunk *cursor = res->getSprite(num);

	CursorMan.replaceCursor(cursor->data, cursor->width, cursor->height, 0, 0, 0);

	delete[] cursor->data;
	delete cursor;
	delete res;
}

void Graphics::showCursor() {
	CursorMan.showMouse(true);
}

void Graphics::hideCursor() {
	CursorMan.showMouse(false);
}

} // End of namespace Chewy
