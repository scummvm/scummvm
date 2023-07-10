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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_MAPDATA_H
#define CRAB_MAPDATA_H

#include "crab/Rectangle.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
struct MapData {
	// The paths of set of map images - background and the fully revealed map overlay
	Common::String path_bg, path_overlay;

	// The places the player has revealed in this world map
	Common::Array<Rect> reveal;

	struct MarkerData {
		// The name of the marker, same name as the quest
		Common::String name;

		// Position of the marker
		Vector2i pos;
	};

	// The set of destinations currently active
	Common::Array<MarkerData> dest;

	MapData() {}
	MapData(rapidxml::xml_node<char> *node) { load(node); }
	~MapData() {}

	void load(rapidxml::xml_node<char> *node);
	void DestAdd(const Common::String &name, const int &x, const int &y);

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MAPDATA_H
