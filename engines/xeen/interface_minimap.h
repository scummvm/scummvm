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

#ifndef XEEN_INTERFACE_MINIMAP_H
#define XEEN_INTERFACE_MINIMAP_H

namespace Xeen {

class InterfaceMinimap {
private:
	int _animFrame;
private:
	/**
	 * Draws the outdoors minimap
	 */
	void drawOutdoorsMinimap();

	/**
	 * Draws the indoors minimap
	 */
	void drawIndoorsMinimap();
public:
	InterfaceMinimap() : _animFrame(0) {}

	/**
	 * Draw the mini-map
	 */
	void drawMinimap();
};

} // End of namespace Xeen

#endif /* XEEN_INTERFACE_MINIMAP_H */
