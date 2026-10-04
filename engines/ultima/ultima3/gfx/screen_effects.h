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

#ifndef ULTIMA3_GFX_SCREEN_EFFECTS_H
#define ULTIMA3_GFX_SCREEN_EFFECTS_H

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

constexpr int FLASH_FRAMES = 3;

/**
 * Brief colour inversions of part of the game screen, used to show a party
 * member being hurt or the whole view being struck
 */
struct ScreenEffects {
	int _slot = -1;
	bool _viewport = false;
	int _frames = 0;

	void flashSlot(int slot) {
		_slot = slot;
		_frames = FLASH_FRAMES;
	}

	void flashViewport() {
		_viewport = true;
		_frames = FLASH_FRAMES;
	}

	void tick() {
		if (_frames > 0 && --_frames == 0) {
			_slot = -1;
			_viewport = false;
		}
	}
};

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
