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
#ifndef DRAGONS_CREDITS_H
#define DRAGONS_CREDITS_H

#include "graphics/surface.h"
#include "graphics/pixelformat.h"

namespace Dragons {

class BigfileArchive;
class DragonsEngine;
class FontManager;

class Credits {
private:
	DragonsEngine *_vm;
	FontManager *_fontManager;
	BigfileArchive *_bigfileArchive;
	Graphics::Surface *_surface;
	bool _running;
	int16 _yOffset;
	int16 _linesRemaining;
	int16 _updateCounter;

	byte *_creditsData;
	char *_curPtr;
	uint32 _dataLength;
	uint32 _curPosition;

public:
	Credits(DragonsEngine *vm, FontManager *fontManager, BigfileArchive *bigfileArchive);
	void start();
	bool isRunning();
	void draw();
	void update();
private:
	void cleanup();
	void convertToWideChar(uint16 *destBuf, const byte *text, uint16 maxLength);
};

} // End of namespace Dragons

#endif //DRAGONS_CREDITS_H
