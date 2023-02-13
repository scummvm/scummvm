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

#include "mm/xeen/sprites.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

SpriteResource::SpriteResource(const Common::String &filename, int ccMode) {
	_data = nullptr;
	load(filename, ccMode);
}

void SpriteResource::load(const Common::String &filename, int ccMode) {
	_filename = filename;
	File f(filename, ccMode);
	Shared::Xeen::SpriteResource::load(f);
}

void SpriteResource::draw(Window &dest, int frame, const Common::Point &destPos,
	uint flags, int scale) {
	Shared::Xeen::SpriteResource::draw(dest, frame, destPos, dest.getBounds(), flags, scale);
}

void SpriteResource::draw(int windowIndex, int frame, const Common::Point &destPos,
	uint flags, int scale) {
	Window &win = (*g_vm->_windows)[windowIndex];
	draw(win, frame, destPos, flags, scale);
}

void SpriteResource::draw(int windowIndex, int frame) {
	draw((*g_vm->_windows)[windowIndex], frame, Common::Point());
}

} // End of namespace Xeen
} // End of namespace MM
