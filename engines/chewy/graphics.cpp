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

#include "chewy/cursor.h"
#include "chewy/graphics.h"
#include "chewy/resource.h"
#include "chewy/text.h"
#include "chewy/video/cfo_decoder.h"

namespace Chewy {

#define DESC_WIDTH 80
#define DESC_HEIGHT 8

Graphics::Graphics(ChewyEngine *vm) : _vm(vm) {
	_font = nullptr;
	_descSurface.create(DESC_WIDTH, DESC_HEIGHT, ::Graphics::PixelFormat::createFormatCLUT8());
}

Graphics::~Graphics() {
	delete _font;
	_descSurface.free();
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

void Graphics::drawRect(Common::Rect r, byte color) {
	::Graphics::Surface *screen = g_system->lockScreen();
	screen->drawLine(r.left, r.top, r.right, r.top, color);
	screen->drawLine(r.right, r.top, r.right, r.bottom, color);
	screen->drawLine(r.left, r.bottom, r.right, r.bottom, color);
	screen->drawLine(r.left, r.top, r.left, r.bottom, color);
	g_system->unlockScreen();
}

void Graphics::loadFont(Common::String filename) {
	_font = new Font(filename);
}

void Graphics::drawTransparent(uint16 x, uint16 y, byte *data, uint16 width, uint16 height, byte transparentColor) {
	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < width; curX++) {
		for (uint curY = 0; curY < height; curY++) {
			if (curX + x < 320 && curY + y < 200) {
				byte *src = data + (curY * width) + curX;
				byte *dst = (byte *)screen->getBasePtr(curX + x, curY + y);
				if (*src != transparentColor)
					*dst = *src;
			}
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
	CfoDecoder *cfoDecoder = new CfoDecoder(_vm->_sound);
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
	byte curPalette[256 * 3];

	g_system->getPaletteManager()->grabPalette(curPalette, 0, 256);
	_vm->_cursor->hideCursor();

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

	g_system->getPaletteManager()->setPalette(curPalette, 0, 256);
	_vm->_cursor->showCursor();

	delete videoResource;
	delete cfoDecoder;
}

void Graphics::setDescSurface(Common::Point pos) {
	_descPos = pos;

	if (pos.x < 0)
		return;

	::Graphics::Surface *s = g_system->lockScreen();
	Common::Rect r = Common::Rect(pos.x, pos.y, pos.x + _descSurface.w, pos.y + _descSurface.h);
	r.clip(Common::Rect(0, 0, 320, 200));
	_descSurface.copyRectToSurface(*s, 0, 0, r);
	g_system->unlockScreen();
}

void Graphics::restoreDescSurface() {
	if (_descPos.x < 0)
		return;

	Common::Rect r = Common::Rect(_descPos.x, _descPos.y, _descPos.x + _descSurface.w, _descPos.y + _descSurface.h);
	r.clip(Common::Rect(0, 0, 320, 200));
	g_system->copyRectToScreen(_descSurface.getPixels(), _descSurface.pitch, _descPos.x, _descPos.y, r.width(), r.height());
	_descPos = Common::Point(-1, -1);
}

} // End of namespace Chewy
