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

#ifndef MM1_VIEWS_ENH_GAME_FRAME_H
#define MM1_VIEWS_ENH_GAME_FRAME_H

#include "common/rect.h"
#include "graphics/managed_surface.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

/**
 * Area of the game frame's lower-right jewel, used as the Bestiary button
 */
inline Common::Rect bestiaryButtonBounds() {
	// The original jewel, excluding the sidebar: 16 x 16 pixels
	return Common::Rect(216, 131, 232, 147);
}

/**
 * Adjusts the game background for MM1. The background comes from Xeen
 * resources, so some parts need replacing, and a skull is drawn on the
 * jewel to mark the Bestiary button
 */
inline void decorateGameFrame(Graphics::ManagedSurface &s) {
	if (s.w != 320 || s.h != 200 || s.format.bytesPerPixel != 1)
		return;

	// Xeen filled these background slots with clairvoyance/levitation
	// sprites. MM1 has no such overlays, so mirror the intact stone
	// strip from the right instead
	for (int y = 32; y < 120; ++y) {
		if (y >= 55 && y < 82)
			continue;

		for (int x = 0; x < 8; ++x)
			*(byte *)s.getBasePtr(x, y) = *(const byte *)s.getBasePtr(231 - x, y);
	}

	// Replace only the jewel's inset and keep the existing gold frame,
	// sampling a light stone shade from the background's own palette
	const byte stone = *(const byte *)s.getBasePtr(224, 131);
	for (int y = 134; y < 145; ++y) {
		for (int x = 219; x < 230; ++x)
			*(byte *)s.getBasePtr(x, y) = stone;
	}

	static const char *const SKULL[9] = {
		"..#####..",
		".#######.",
		"#########",
		"#########",
		"#..###..#",
		"#..###..#",
		".###.###.",
		"..#####..",
		"..#.#.#.."
	};
	for (int y = 0; y < 9; ++y) {
		for (int x = 0; x < 9; ++x) {
			if (SKULL[y][x] == '#')
				*(byte *)s.getBasePtr(220 + x, 135 + y) = 0;
		}
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
