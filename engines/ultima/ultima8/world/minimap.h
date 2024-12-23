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

#include "ultima/ultima8/misc/common_types.h"
#include "graphics/surface.h"

#ifndef ULTIMA8_WORLD_MINIMAP_H
#define ULTIMA8_WORLD_MINIMAP_H

namespace Common {
class Path;
}

namespace Ultima {
namespace Ultima8 {

class CurrentMap;
class Item;

#define MINMAPGUMP_SCALE 8

class MiniMap {
private:
	uint32 _mapNum;
	Graphics::Surface _surface;

	uint32 sampleAtPoint(const CurrentMap &map, int x, int y);
	uint32 sampleAtPoint(const Item &item, int x, int y);

	const Common::Rect getCropBounds() const;
public:
	MiniMap(uint32 mapNum);
	~MiniMap();

	void update(const CurrentMap &map);
	Common::Point getItemLocation(const Item &item, unsigned int chunkSize);

	uint32 getMapNum() const { return _mapNum; }
	Graphics::Surface *getSurface() { return &_surface; }

	bool load(Common::ReadStream *rs, uint32 version);
	void save(Common::WriteStream *ws) const;
	bool dump(const Common::Path &filename) const;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
