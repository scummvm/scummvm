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

#include "darkseed/sprites.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

#define DARKSEED_MAX_SPRITES_ON_SCREEN 30

Sprites::Sprites() {
	_spriteDrawList.reserve(DARKSEED_MAX_SPRITES_ON_SCREEN);
}

void Sprites::addSpriteToDrawList(uint16 destX, uint16 destY, const Sprite *sprite, uint8 order, uint16 destW, uint16 destH, bool flip) {
	if (_spriteDrawList.size() == DARKSEED_MAX_SPRITES_ON_SCREEN || destX >= 570) {
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

	if (!_spriteDrawList.empty()) {
		uint insertLocation = 0;
		for (; insertLocation < _spriteDrawList.size(); insertLocation++) {
			if (order < _spriteDrawList[insertLocation].order) {
				break;
			}
		}
		_spriteDrawList.insert_at(insertLocation, drawInstruction);
	} else {
		_spriteDrawList.push_back(drawInstruction);
	}
}

void Sprites::clearSpriteDrawList() {
	// not using clear() here to avoid freeing array storage memory.
	while (!_spriteDrawList.empty()) {
		_spriteDrawList.pop_back();
	}
}

void Sprites::drawSprites() {
	for (int i = _spriteDrawList.size() - 1; i >= 0; i--) {
		SpriteDrawInstruction &drawInstruction = _spriteDrawList[i];
		if (drawInstruction.sprite->_width == drawInstruction.destW && drawInstruction.sprite->_height == drawInstruction.destH && !drawInstruction.flip) {
			drawInstruction.sprite->draw(drawInstruction.destX, drawInstruction.destY, g_engine->_frameBottom); // TODO add support for flipping sprite.
		} else {
			drawInstruction.sprite->drawScaled(drawInstruction.destX, drawInstruction.destY, drawInstruction.destW, drawInstruction.destH, drawInstruction.flip);
		}
	}
}

} // End of namespace Darkseed
