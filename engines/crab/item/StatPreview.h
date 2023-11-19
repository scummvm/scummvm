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

#ifndef CRAB_STATPREVIEW_H
#define CRAB_STATPREVIEW_H

#include "crab/item/Item.h"
#include "crab/ui/SectionHeader.h"
#include "crab/stat/StatDrawHelper.h"

namespace Crab {

namespace pyrodactyl {
namespace item {
struct StatPreview {
	pyrodactyl::ui::SectionHeader stat, unit;

	// We increment stat draw position by this much for every new item bonus
	Vector2i _incS, _incU;

	bool _enabled;

	StatPreview() {
		_enabled = false;
	}

	void load(rapidxml::xml_node<char> *node);
	void draw(Item &item, pyrodactyl::stat::StatDrawHelper &helper);
};
} // End of namespace item
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_STATPREVIEW_H
