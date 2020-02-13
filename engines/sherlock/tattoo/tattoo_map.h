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

#ifndef SHERLOCK_TATTOO_MAP_H
#define SHERLOCK_TATTOO_MAP_H

#include "common/scummsys.h"
#include "sherlock/map.h"
#include "sherlock/resources.h"
#include "sherlock/surface.h"
#include "sherlock/tattoo/widget_tooltip.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

struct MapEntry : Common::Point {
	int _iconNum;
	Common::String _description;

	MapEntry() : Common::Point(), _iconNum(-1) {}
	MapEntry(int posX, int posY, int iconNum) : Common::Point(posX, posY), _iconNum(iconNum) {}
	void clear();
};

class TattooMap : public Map {
private:
	Common::Array<MapEntry> _data;
	ImageFile *_iconImages;
	int _bgFound, _oldBgFound;
	WidgetMapTooltip _mapTooltip;
	Common::Point _targetScroll;

	/**
	 * Load data  needed for the map
	 */
	void loadData();

	/**
	 * Draws all available location icons onto the back buffer
	 */
	void drawMapIcons();

	/**
	 * Draws the location names of whatever the mouse moves over on the map
	 */
	void checkMapNames(bool slamIt);

	/**
	 * Restores an area of the map background
	 */
	void restoreArea(const Common::Rect &bounds);

	/**
	 * This will load a specified close up and zoom it up to the middle of the screen
	 */
	void showCloseUp(int closeUpNum);
public:
	TattooMap(SherlockEngine *vm);
	~TattooMap() override {}

	/**
	 * Show the map
	 */
	int show() override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
