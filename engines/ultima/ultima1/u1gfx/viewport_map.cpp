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

#include "ultima/ultima1/u1gfx/viewport_map.h"
#include "ultima/ultima1/u1gfx/sprites.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

BEGIN_MESSAGE_MAP(ViewportMap, Shared::ViewportMap)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

ViewportMap::ViewportMap(TreeItem *parent) : Shared::ViewportMap(parent), _mapType(Maps::MAP_OVERWORLD) {
	_sprites = new Sprites(this);	
}

ViewportMap::~ViewportMap() {
}

void ViewportMap::draw() {
	Maps::Ultima1Map *map = static_cast<Maps::Ultima1Map *>(getGame()->getMap());

	// If necessary, load the sprites for rendering the map
	if (_sprites->empty() || _mapType != map->_mapType) {
		_mapType = map->_mapType;
		Sprites *sprites = static_cast<Sprites *>(_sprites);
		sprites->load(_mapType == Maps::MAP_OVERWORLD);
	}

	// Draw the map
	Shared::ViewportMap::draw();
}

bool ViewportMap::FrameMsg(CFrameMsg &msg) {
	// To allow map to animate, on each frame mark the map as dirty again
	setDirty(true);
	return true;
}

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima
