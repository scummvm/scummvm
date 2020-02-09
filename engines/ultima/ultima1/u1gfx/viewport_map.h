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

#ifndef ULTIMA_ULTIMA1_GFX_VIEWPORT_MAP_H
#define ULTIMA_ULTIMA1_GFX_VIEWPORT_MAP_H

#include "ultima/shared/gfx/viewport_map.h"
#include "ultima/ultima1/maps/map.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

using Shared::CFrameMsg;

class ViewportMap : public Shared::ViewportMap {
	DECLARE_MESSAGE_MAP;
	bool FrameMsg(CFrameMsg &msg);
private:
	Maps::MapType _mapType;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	ViewportMap(Shared::TreeItem *parent);
	
	/**
	 * Destructor
	 */
	~ViewportMap() override;

	/**
	 * Draws the map
	 */
	void draw() override;
};

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
