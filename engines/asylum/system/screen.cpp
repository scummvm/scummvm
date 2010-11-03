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
 * $URL$
 * $Id$
 *
 */

#include "asylum/system/screen.h"

#include "asylum/resources/worldstats.h"

namespace Asylum {

Screen::Screen(AsylumEngine *vm) : _vm(vm) {
	_backBuffer.create(640, 480, 1);
}

Screen::~Screen() {
	_backBuffer.free();
}

void Screen::copyBackBufferToScreen() {
	_vm->_system->copyRectToScreen((byte *)_backBuffer.pixels, _backBuffer.w, 0, 0, _backBuffer.w, _backBuffer.h);
}

void Screen::copyToBackBuffer(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height) {
	int32 h = height;
	int32 w = width;
	byte *dest = (byte *)_backBuffer.pixels;

	while (h--) {
		memcpy(dest + y * _backBuffer.pitch + x, buffer, w);
		dest += 640;
		buffer += pitch;
	}
}

void Screen::copyToBackBufferWithTransparency(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height) {
	byte *dest = (byte *)_backBuffer.pixels;

	int32 left = (x < 0) ? -x : 0;
	int32 top = (y < 0) ? -y : 0;
	int32 right = (x + width > 640) ? 640 - abs(x) : width;
	int32 bottom = (y + height > 480) ? 480 - abs(y) : height;

	for (int32 curY = top; curY < bottom; curY++) {
		for (int32 curX = left; curX < right; curX++) {
			if (buffer[curX + curY * pitch] != 0 ) {
				dest[x + curX + (y + curY) * 640] = buffer[curX + curY * pitch];
			}
		}
	}
}

void Screen::copyRectToScreen(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height) {
	_vm->_system->copyRectToScreen(buffer, pitch, x, y, width, height);
}

void Screen::copyRectToScreenWithTransparency(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height) {
	byte *screenBuffer = (byte *)_vm->_system->lockScreen()->pixels;

	int32 left = (x < 0) ? -x : 0;
	int32 top = (y < 0) ? -y : 0;
	int32 right = (x + width > 640) ? 640 - abs(x) : width;
	int32 bottom = (y + height > 480) ? 480 - abs(y) : height;

	for (int32 curY = top; curY < bottom; curY++) {
		for (int32 curX = left; curX < right; curX++) {
			if (buffer[curX + curY * pitch] != 0) {
				screenBuffer[x + curX + (y + curY) * 640] = buffer[curX + curY * pitch];
			}
		}
	}

	_vm->_system->unlockScreen();
}

void Screen::setPalette(byte *rgbPalette) {
	byte palette[256 * 4];
	byte *p = rgbPalette;

	// skip first color and set it to black always.
	memset(palette, 0, 4);
	p += 3;

	for (int32 i = 1; i < 256; i++) {
		palette[i * 4 + 0] = *p++ << 2;
		palette[i * 4 + 1] = *p++ << 2;
		palette[i * 4 + 2] = *p++ << 2;
		palette[i * 4 + 3] = 0;
	}

	_vm->_system->setPalette(palette, 0, 256);
}

void Screen::setGammaLevel(ResourcePack *resPack, ResourceId id, int32 val) {
	error("[Screen::setGammaLevel] not implemented");
}

void Screen::drawWideScreen(int16 barSize) {
	if (barSize > 0) {

		_vm->_system->lockScreen()->fillRect(Common::Rect(0, 0, 640, barSize), 0);
		_vm->_system->unlockScreen();
		_vm->_system->lockScreen()->fillRect(Common::Rect(0, 480 - barSize, 640, 480), 0);
		_vm->_system->unlockScreen();
	}
}

void Screen::clearScreen() {
	_vm->_system->fillScreen(0);
}

void Screen::paletteFade(uint32 red, int32 milliseconds, int32 param) {
	error("[Screen::palFade] not implemented");
}

void Screen::startPaletteFade(ResourceId resourceId, int32 milliseconds, int32 param) {
	error("[Screen::startPaletteFade] not implemented");
}

void Screen::addGraphicToQueue(ResourceId resourceId, int32 frameIdx, int32 x, int32 y, int32 flags, int32 transTableNum, int32 priority) {
	GraphicQueueItem item;
	item.resourceId = resourceId;
	item.x = x;
	item.y = y;
	item.frameIdx = frameIdx;
	item.flags = flags;
	item.transTableNum = transTableNum;
	item.priority = priority;

	_queueItems.push_back(item);
}

void Screen::addCrossFadeGraphicToQueue(ResourceId resourceId, int32 frameIdx, int32 x, int32 y, int32 redId2, int32 x2, int32 y2, int32 flags, int32 priority) {
	error("[Screen::addCrossFadeGraphicToQueue] not implemented");
}

void Screen::addGraphicToQueue(GraphicQueueItem item) {
	_queueItems.push_back(item);
}

void Screen::drawGraphicsInQueue() {
	WorldStats *ws = _vm->scene()->worldstats();

	// sort by priority first
	graphicsSelectionSort();

	for (uint32 i = 0; i < _queueItems.size(); i++) {
		GraphicResource *grRes = _vm->scene()->getGraphicResource(_queueItems[i].resourceId);
		GraphicFrame    *fra   = grRes->getFrame(_queueItems[i].frameIdx);

		copyToBackBufferWithTransparency((byte *)fra->surface.pixels,
				fra->surface.w,
				_queueItems[i].x - ws->xLeft, _queueItems[i].y - ws->yTop,
				fra->surface.w,
				fra->surface.h);

		delete grRes;
	}
}

void Screen::clearGraphicsInQueue() {
	_queueItems.clear();
}

void Screen::graphicsSelectionSort() {
	uint32 minIdx;

	for (uint32 i = 0; i < _queueItems.size(); i++) {
		minIdx = i;

		for (uint32 j = i + 1; j < _queueItems.size(); j++)
			if (_queueItems[j].priority > _queueItems[i].priority)
				minIdx = j;

		if (i != minIdx)
			swapGraphicItem(i, minIdx);
	}
}

void Screen::swapGraphicItem(int32 item1, int32 item2) {
	GraphicQueueItem temp;
	temp = _queueItems[item1];
	_queueItems[item1] = _queueItems[item2];
	_queueItems[item2] = temp;
}

void Screen::deleteGraphicFromQueue(ResourceId resourceId) {
	for (uint32 i = 0; i < _queueItems.size(); i++) {
		if (_queueItems[i].resourceId == resourceId) {
			_queueItems.remove_at(i);
			break;
		}
	}
}

} // end of namespace Asylum
