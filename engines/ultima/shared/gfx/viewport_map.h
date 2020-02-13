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

#ifndef ULTIMA_SHARED_GFX_VIEWPORT_MAP_H
#define ULTIMA_SHARED_GFX_VIEWPORT_MAP_H

#include "ultima/shared/gfx/visual_item.h"
#include "ultima/shared/gfx/sprites.h"

namespace Ultima {
namespace Shared {

class ViewportMap : public Gfx::VisualItem {
	DECLARE_MESSAGE_MAP;
protected:
	Gfx::Sprites *_sprites;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	ViewportMap(TreeItem *parent) : Gfx::VisualItem("ViewportMap", Rect(8, 8, 312, 152), parent), _sprites(nullptr) {}
	
	/**
	 * Destructor
	 */
	~ViewportMap() override {}

	/**
	 * Draws the map
	 */
	void draw() override;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
