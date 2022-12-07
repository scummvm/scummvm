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

#ifndef ULTIMA8_GUMPS_MINIMAPGUMP_H
#define ULTIMA8_GUMPS_MINIMAPGUMP_H

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/misc/classtype.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima8 {

class MiniMapGump : public Gump {
private:
	Common::Array<Graphics::Surface> _minimaps;
	Graphics::PixelFormat _format;
	int32 _ax, _ay;

	void update(CurrentMap *map);
	uint32 sampleAtPoint(CurrentMap *map, int x, int y);
	uint32 sampleAtPoint(const Item *item, int x, int y);
public:
	ENABLE_RUNTIME_CLASSTYPE()

	MiniMapGump();
	MiniMapGump(int x, int y);
	~MiniMapGump() override;

	void run() override;

	void generate();
	void clear();

	void        PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) override;
	uint16      TraceObjId(int32 mx, int32 my) override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
