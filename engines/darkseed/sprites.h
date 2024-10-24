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

#ifndef DARKSEED_SPRITES_H
#define DARKSEED_SPRITES_H

#include "common/scummsys.h"
#include "darkseed/nsp.h"

namespace Darkseed {

struct SpriteDrawInstruction {
	uint16 destX = 0;
	uint16 destY = 0;
	uint16 srcW = 0;
	uint16 srcH = 0;
	const Sprite *sprite = nullptr;
	uint8 order = 0;
	uint16 destW = 0;
	uint16 destH = 0;
	bool flip = false;
};

class Sprites {
	Common::Array<SpriteDrawInstruction> _spriteDrawList;

public:
	Sprites();
	void addSpriteToDrawList(uint16 destX, uint16 destY, const Sprite *sprite, uint8 order, uint16 destW, uint16 destH, bool flip);
	void clearSpriteDrawList();

	void drawSprites();
};

} // namespace Darkseed
#endif // DARKSEED_SPRITES_H
