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
#include "graphics/surface.h"

#include "chewy/graphics.h"
#include "chewy/resource.h"
#include "chewy/video/cfo_decoder.h"

namespace Chewy {

const byte _cursorFrames[] = {
	4, 1, 1, 1,		// walk
	4, 1, 1, 1,		// pick up / use
	1, 1, 1, 1, 1,
	4, 1, 1, 1,		// look
	4, 1, 1, 1,		// talk
	4, 1, 1, 1,		// open
	1,
	1, 1, 1, 1,		// left, right, up, down
	1,				// save
	1,
	5, 1, 1, 1, 1,
	1,
	1,				// use (inventory)
	1,				// look (inventory)
	1				// gun
};

Graphics::Graphics(ChewyEngine *vm) : _vm(vm) {
	_curCursor = 0;
	_curCursorFrame = 0;
	_cursorSprites = new SpriteResource("cursor.taf");
	_font = nullptr;
}

Graphics::~Graphics() {
	delete _font;
	delete _cursorSprites;
}

void Graphics::drawSprite(Common::String filename, int spriteNum, uint x, uint y) {
	SpriteResource *res = new SpriteResource(filename);
	TAFChunk *sprite = res->getSprite(spriteNum);

	drawTransparent(x, y, sprite->data, sprite->width, sprite->height, 0);
	g_system->updateScreen();

	delete[] sprite->data;
	delete sprite;
	delete res;
}

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

void Graphics::loadFont(Common::String filename) {
	_font = new Font(filename);
}

void Graphics::drawTransparent(uint16 x, uint16 y, byte *data, uint16 width, uint16 height, byte transparentColor) {
	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < width; curX++) {
		for (uint curY = 0; curY < height; curY++) {
			byte *src = data + (curY * width) + curX;
			byte *dst = (byte *)screen->getBasePtr(curX + x, curY + y);
			if (*src != transparentColor)
				*dst = *src;
		}
	}
	g_system->unlockScreen();
}

void Graphics::drawText(Common::String text, uint x, uint y) {
	::Graphics::Surface *textSurface = _font->getLine(text);

	drawTransparent(x, y, (byte *)textSurface->getPixels(), textSurface->pitch, textSurface->h, 0xFF);

	textSurface->free();
	delete textSurface;
}

void Graphics::playVideo(uint num) {
	CfoDecoder *cfoDecoder = new CfoDecoder(_vm->_mixer);
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

	hideCursor();

	cfoDecoder->start();

	while (!_vm->shouldQuit() && !cfoDecoder->endOfVideo() && !skipVideo) {
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

	showCursor();
}

void Graphics::setCursor(uint num, bool newCursor) {
	TAFChunk *cursor = _cursorSprites->getSprite(num);
	if (newCursor)
		_curCursor = num;

	CursorMan.replaceCursor(cursor->data, cursor->width, cursor->height, 0, 0, 0);

	delete[] cursor->data;
	delete cursor;
}

void Graphics::showCursor() {
	CursorMan.showMouse(true);
}

void Graphics::hideCursor() {
	CursorMan.showMouse(false);
}

void Graphics::animateCursor() {
	if (_cursorFrames[_curCursor] > 1) {
		_curCursorFrame++;

		if (_curCursorFrame >= _cursorFrames[_curCursor])
			_curCursorFrame = 0;

		setCursor(_curCursor + _curCursorFrame, false);
	}
}

void Graphics::nextCursor() {
	uint maxCursors = ARRAYSIZE(_cursorFrames);

	if (_cursorFrames[_curCursor] > 0)
		_curCursor += _cursorFrames[_curCursor];
	else
		_curCursor++;

	if (_curCursor >= maxCursors)
		_curCursor = 0;

	_curCursorFrame = 0;
	setCursor(_curCursor);
}

} // End of namespace Chewy
