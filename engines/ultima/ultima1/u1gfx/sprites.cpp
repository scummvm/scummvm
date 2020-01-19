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

#include "ultima/ultima1/u1gfx/sprites.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/game.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

BEGIN_MESSAGE_MAP(Sprites, Shared::TreeItem)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

#define ANIMATE_FRAME_DELAY 200

void Sprites::load(bool isOverworld) {
	_isOverworld = isOverworld;

	if (isOverworld)
		Shared::Gfx::Sprites::load("t1ktiles.bin", 4);
	else
		Shared::Gfx::Sprites::load("t1ktown.bin", 4, 8, 8);
}

bool Sprites::FrameMsg(CFrameMsg &msg) {
	if (!empty() && _isOverworld) {
		animateWater();
	}

	++_frameCtr;

	return false;
}

void Sprites::animateWater() {
	byte lineBuffer[16];
	Shared::Gfx::Sprite &sprite = (*this)[0];

	Common::copy(sprite.getBasePtr(0, 15), sprite.getBasePtr(0, 16), lineBuffer);
	Common::copy_backward(sprite.getBasePtr(0, 0), sprite.getBasePtr(0, 15), sprite.getBasePtr(0, 16));
	Common::copy(lineBuffer, lineBuffer + 16, sprite.getBasePtr(0, 0));
}

Shared::Gfx::Sprite &Sprites::operator[](uint idx) {
	int offset = 2;
	if ((_frameCtr % 6) == 0)
		offset = 0;
	else if ((_frameCtr % 3) == 0)
		offset = 1;

	if (!_isOverworld) {
		// Don't do overworld tile animations within the cities and castles
		return Shared::Gfx::Sprites::operator[](idx);
	} else if (idx == 4 && offset != 2) {
		// Castle flag waving
		return Shared::Gfx::Sprites::operator[](4 + offset);
	} else if (idx == 6) {
		// City flag waving
		return Shared::Gfx::Sprites::operator[](7 + ((_frameCtr & 3) ? 1 : 0));
	} else {
		if (idx >= 7 && idx < 50)
			idx += 2;

		if (idx == 14 || idx == 25) {
			// Transports
			return Shared::Gfx::Sprites::operator[](idx + (_frameCtr & 1));
		} else if (idx >= 19 && idx <= 47) {
			// Random monster animation
			return Shared::Gfx::Sprites::operator[](idx + (g_vm->getRandomNumber(1, 100) & 1));
		} else {
			return Shared::Gfx::Sprites::operator[](idx);
		}
	}
}

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima
