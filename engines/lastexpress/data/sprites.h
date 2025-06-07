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

#ifndef LASTEXPRESS_SPRITES_H
#define LASTEXPRESS_SPRITES_H

#include "lastexpress/lastexpress.h"

namespace LastExpress {

class LastExpressEngine;

typedef struct Extent {
	int left;
	int top;
	int right;
	int bottom;
	int width;
	int height;

	Extent() {
		left = 0;
		top = 0;
		right = 0;
		bottom = 0;
		width = 0;
		height = 0;
	}

	Extent(int newLeft, int newTop, int newRight, int newBottom, int newWidth, int newHeight) {
		left = newLeft;
		top = newTop;
		right = newRight;
		bottom = newBottom;
		width = newWidth;
		height = newHeight;
	}
} Extent;

struct Sprite;

typedef uint16 PixMap;

class SpriteManager {

public:
	SpriteManager(LastExpressEngine *engine);
	~SpriteManager() {}

	void drawCycle();
	void drawCycleSimple(PixMap *pixels);
	void queueErase(Sprite *sprite);
	void resetEraseQueue();
	void killSpriteQueue();
	void touchSpriteQueue();
	void drawSprite(Sprite *sprite);
	void removeSprite(Sprite *sprite);
	void destroySprite(Sprite **sprites, bool redrawFlag);

private:
	LastExpressEngine *_engine = nullptr;

	bool _drawSequencesFlag = false;
	Sprite *_frameQueue = nullptr;
	bool _coordinatesAreSet = false;
	Extent _eraseRect = Extent(640, 480, 0, 0, 0, 0);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SPRITES_H
