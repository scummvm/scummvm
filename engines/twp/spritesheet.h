
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

#ifndef TWP_SPRITESHEET_H
#define TWP_SPRITESHEET_H

#include "common/hashmap.h"
#include "common/rect.h"
#include "common/stream.h"
#include "math/vector2d.h"

namespace Twp {

struct SpriteSheetFrame {
  Common::String name;
  Common::Rect frame;
  Common::Rect spriteSourceSize;
  Math::Vector2d sourceSize;
};

struct SpriteSheet {
	Common::HashMap<Common::String, SpriteSheetFrame> frameTable;

	void parseSpriteSheet(const Common::String& contents);
};

}

#endif // TWP_SPRITESHEET_H
