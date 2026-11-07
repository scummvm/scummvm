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

#ifndef SCOOBY_TILE_LAYER_H
#define SCOOBY_TILE_LAYER_H

// Identifies one logical layer in a decoded tile scene.

namespace Scooby {

enum class TileLayer {
	// The layer drawn behind the foreground layer at each priority level.
	Background,

	// The layer drawn in front of the background layer at each priority level.
	Foreground,

	// The lower-interface map selected below the room's scanline-168 raster split.
	Interface
};
} // namespace Scooby

#endif // SCOOBY_TILE_LAYER_H
