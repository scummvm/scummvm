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

#include "asylum/screen.h"
#include "asylum/shared.h"

namespace Asylum {

Screen::Screen(OSystem *sys) : _sys(sys) {
	_backBuffer.create(640, 480, 1);
}

Screen::~Screen() {
	_backBuffer.free();
}

void Screen::copyBackBufferToScreen() {
	_sys->copyRectToScreen((byte *)_backBuffer.pixels, _backBuffer.w, 0, 0, _backBuffer.w, _backBuffer.h);
}

void Screen::copyToBackBuffer(byte *buffer, int pitch, int x, int y, int width, int height) {
	int h = height;
	int w = width;
	byte *dest = (byte *)_backBuffer.pixels;

	while (h--) {
		memcpy(dest + y * _backBuffer.pitch + x, buffer, w);
		dest += 640;
		buffer += pitch;
	}
}

void Screen::copyToBackBufferWithTransparency(byte *buffer, int pitch, int x, int y, int width, int height) {
	// int h = height;
	// int w = width;
	byte *dest = (byte *)_backBuffer.pixels;

	for (int curY = 0; curY < height; curY++) {
		for (int curX = 0; curX < width; curX++) {
			if (buffer[curX + curY * pitch] != 0) {
				dest[x + curX + (y + curY) * 640] = buffer[curX + curY * pitch];
			}
		}
	}
}

void Screen::copyRectToScreen(byte *buffer, int pitch, int x, int y, int width, int height) {
	_sys->copyRectToScreen(buffer, pitch, x, y, width, height);
}

void Screen::copyRectToScreenWithTransparency(byte *buffer, int pitch, int x, int y, int width, int height) {
	byte *screenBuffer = (byte *)_sys->lockScreen()->pixels;

	for (int curY = 0; curY < height; curY++) {
		for (int curX = 0; curX < width; curX++) {
			if (buffer[curX + curY * pitch] != 0 && (x + curX + (y + curY) * 640 <= 640*480)) {
                screenBuffer[x + curX + (y + curY) * 640] = buffer[curX + curY * pitch];
			}
		}
	}

	_sys->unlockScreen();
}

void Screen::setPalette(byte *rgbPalette) {
	byte palette[256 * 4];
	byte *p = rgbPalette;

    // skip first color and set it to black always.
    memset(palette, 0, 4); p+=3;

	for (int i = 1; i < 256; i++) {
		palette[i * 4 + 0] = *p++ << 2;
		palette[i * 4 + 1] = *p++ << 2;
		palette[i * 4 + 2] = *p++ << 2;
		palette[i * 4 + 3] = 0;
	}

	_sys->setPalette(palette, 0, 256);
}

void Screen::drawWideScreen(int16 barSize) {
    if(barSize > 0) {
        
        _sys->lockScreen()->fillRect(Common::Rect(0, 0, 640, barSize), 0);
        _sys->unlockScreen();
        _sys->lockScreen()->fillRect(Common::Rect(0, 480 - barSize, 640, 480), 0);
        _sys->unlockScreen();
    }
}

void Screen::clearScreen() {
    _sys->fillScreen(0);
}

void Screen::addGraphicToQueue(uint32 redId, uint32 frameIdx, uint32 x, uint32 y, uint32 flags, uint32 transTableNum, uint32 priority) {
    GraphicQueueItem item;
    item.resId = redId;
    item.x = x;
    item.y = y;
    item.frameIdx = frameIdx;
    item.flags = flags;
    item.priority = priority;

    _queueItems.push_back(item);
}

void Screen::addCrossFadeGraphicToQueue(uint32 redId, uint32 frameIdx, uint32 x, uint32 y, uint32 redId2, uint32 x2, uint32 y2, uint32 flags, uint32 priority) {
    
}

void Screen::addGraphicToQueue(GraphicQueueItem item) {
    _queueItems.push_back(item);
}

void Screen::drawGraphicsInQueue() {
    // sort by priority first
    graphicsSelectionSort();
    for(uint i = 0; i < _queueItems.size(); i++) {
        GraphicResource *grRes = Shared.getScene()->getGraphicResource(_queueItems[i].resId);
        GraphicFrame *fra = grRes->getFrame(_queueItems[i].frameIdx);
        copyRectToScreenWithTransparency((byte *)fra->surface.pixels, fra->surface.w, _queueItems[i].x, _queueItems[i].y, fra->surface.w, fra->surface.h);
        delete grRes;
    }
}

void Screen::clearGraphicsInQueue() {
    _queueItems.clear();
}

void Screen::graphicsSelectionSort() {
    uint minIdx;

    for (uint i = 0; i < _queueItems.size(); i++) {
        minIdx = i;

        for (uint j = i + 1; j < _queueItems.size(); j++)
            if (_queueItems[j].priority > _queueItems[i].priority)
                minIdx = j;

        if(i != minIdx)
            swapGraphicItem(i, minIdx);
   }
}

void Screen::swapGraphicItem(int item1, int item2) {
   GraphicQueueItem temp;
   temp = _queueItems[item1];
   _queueItems[item1] = _queueItems[item2];
   _queueItems[item2] = temp;
}

void Screen::deleteGraphicFromQueue(uint32 resId) {
    for(uint i = 0; i < _queueItems.size(); i++) {
        if(_queueItems[i].resId == resId) {
            _queueItems.remove_at(i);
            break;
        }
    }
}

} // end of namespace Asylum
