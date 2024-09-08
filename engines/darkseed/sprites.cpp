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

#include "sprites.h"
#include "darkseed.h"
#define DARKSEED_MAX_SPRITES_ON_SCREEN 30

Darkseed::Sprites::Sprites() {
	spriteDrawList.reserve(DARKSEED_MAX_SPRITES_ON_SCREEN);
}

void Darkseed::Sprites::addSpriteToDrawList(uint16 destX, uint16 destY, const Darkseed::Sprite *sprite, uint8 order, uint8 destW, uint8 destH, bool flip) {
	if (spriteDrawList.size() == DARKSEED_MAX_SPRITES_ON_SCREEN || destX >= 570) {
		return;
	}

	SpriteDrawInstruction drawInstruction;
	drawInstruction.destX = destX;
	drawInstruction.destY = destY;
	drawInstruction.sprite = sprite;
	drawInstruction.order = order;
	drawInstruction.destW = destW;
	drawInstruction.destH = destH;
	drawInstruction.flip = flip;

	if (!spriteDrawList.empty()) {
		int insertLocation = 0;
		for (; insertLocation < spriteDrawList.size(); insertLocation++) {
			if (order < spriteDrawList[insertLocation].order) {
				break;
			}
		}
		spriteDrawList.insert_at(insertLocation, drawInstruction);
	} else {
		spriteDrawList.push_back(drawInstruction);
	}
}

void Darkseed::Sprites::clearSpriteDrawList() {
	// not using clear() here to avoid freeing array storage memory.
	while(!spriteDrawList.empty()) {
		spriteDrawList.pop_back();
	}
}

void Darkseed::Sprites::drawSprites() {
	for (int i = spriteDrawList.size() - 1; i >= 0; i--) {
		SpriteDrawInstruction &drawInstruction = spriteDrawList[i];
		if (drawInstruction.sprite->width == drawInstruction.destW && drawInstruction.sprite->height == drawInstruction.destH && !drawInstruction.flip) {
			drawInstruction.sprite->draw(drawInstruction.destX, drawInstruction.destY, g_engine->_frameBottom); // TODO add support for flipping sprite.
		} else {
			drawInstruction.sprite->drawScaled(drawInstruction.destX, drawInstruction.destY, drawInstruction.destW, drawInstruction.destH, drawInstruction.flip);
		}
	}
}
