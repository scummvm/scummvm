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

#ifndef XEEN_SPRITES_H
#define XEEN_SPRITES_H

#include "mm/shared/xeen/sprites.h"
#include "mm/shared/xeen/xsurface.h"

namespace MM {
namespace Xeen {

using namespace Shared::Xeen;

class SpriteResource : public MM::Shared::Xeen::SpriteResource {
private:
	/**
	 * Draw the sprite onto a given window
	 */
	void draw(int windowNum, int frame, const Common::Point &destPos,
		const Common::Rect &bounds, uint flags = 0, int scale = 0);

public:
	SpriteResource() : Shared::Xeen::SpriteResource() {}
	SpriteResource(const Common::String &filename) :
		Shared::Xeen::SpriteResource(filename) {}
	SpriteResource(const Common::String &filename, int ccMode);

	/**
	 * Load a sprite resource from a given file
	 */
	void load(const Common::String &filename) {
		Shared::Xeen::SpriteResource::load(filename);
	}

	/**
	 * Load a sprite resource from a given file and archive
	 */
	void load(const Common::String &filename, int ccMode);

	/**
	 * Draw a sprite onto a surface
	 * @param dest		Destination surface
	 * @param frame		Frame number
	 * @param destPos	Destination position
	 * @param flags		Flags
	 * @param scale		Scale: 0=No scale, SCALE_ENLARGE=Enlarge it
	 *					1..15   -> reduces the sprite: the higher, the smaller it'll be
	 */
	void draw(Shared::Xeen::XSurface &dest, int frame, const Common::Point &destPos,
		uint flags = 0, int scale = 0) {
		Shared::Xeen::SpriteResource::draw(dest, frame, destPos, flags, 0);
	}

	/**
	 * Draw the sprite onto the given surface
	 * @param dest		Destination surface
	 * @param frame		Frame number
	 */
	void draw(Shared::Xeen::XSurface &dest, int frame) {
		Shared::Xeen::SpriteResource::draw(dest, frame);
	}

	/**
	 * Draw the sprite onto a given surface
	 */
	void draw(Graphics::ManagedSurface *dest, int frame, const Common::Point &destPos) {
		Shared::Xeen::SpriteResource::draw(dest, frame, destPos);
	}

	/**
	 * Draw a sprite onto a specific window
	 * @param dest		Destination window
	 * @param frame		Frame number
	 * @param destPos	Destination position
	 * @param flags		Flags
	 * @param scale		Scale: 0=No scale, SCALE_ENLARGE=Enlarge it
	 *					1..15   -> reduces the sprite: the higher, the smaller it'll be
	 */
	void draw(Window &dest, int frame, const Common::Point &destPos,
		uint flags = 0, int scale = 0);

	/**
	 * Draw the sprite onto the given window
	 * @param windowIndex	Destination window number
	 * @param frame			Frame number
	 */
	void draw(int windowIndex, int frame);

	/**
	 * Draw a sprite onto a given window
	 * @param windowIndex	Destination window number
	 * @param frame		Frame number
	 * @param destPos	Destination position
	 * @param flags		Flags
	 * @param scale		Scale: 0=No scale, SCALE_ENLARGE=Enlarge it
	 *					1..15   -> reduces the sprite: the higher, the smaller it'll be
	 */
	void draw(int windowIndex, int frame, const Common::Point &destPos,
		uint flags = 0, int scale = 0);
};

using namespace MM::Shared;

} // End of namespace Xeen
} // End of namespace MM

#endif
