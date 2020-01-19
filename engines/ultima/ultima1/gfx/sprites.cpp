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

#include "ultima/ultima1/gfx/sprites.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

BEGIN_MESSAGE_MAP(Sprites, TreeItem)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

bool Sprites::FrameMsg(CFrameMsg &msg) {
	if (!empty()) {
		animateWater();
	}

	return false;
}

void Sprites::animateWater() {
	byte lineBuffer[16];
	Shared::Gfx::Sprite &sprite = (*this)[0];

	Common::copy(sprite.getBasePtr(0, 15), sprite.getBasePtr(0, 16), lineBuffer);
	Common::copy_backward(sprite.getBasePtr(0, 0), sprite.getBasePtr(0, 15), sprite.getBasePtr(0, 16));
	Common::copy(lineBuffer, lineBuffer + 16, sprite.getBasePtr(0, 0));
}

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima
