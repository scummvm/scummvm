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

#ifndef CRAB_STAT_H
#define CRAB_STAT_H
#include "crab/common_header.h"

#include "crab/image/ImageManager.h"
//#include "crab/text/TextManager.h"

namespace Crab {

namespace pyrodactyl {
namespace stat {
struct Stat {
	// The current, default, minimum and maximum value of the stat
	int cur, def, min, max;

	Stat() {
		min = 0;
		max = 1;
		cur = 1;
		def = 1;
	}

	void Reset() { cur = def; }
	void Validate();

	void load(rapidxml::xml_node<char> *node);
	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name);
};

struct StatGroup {
	// We group all stats a single person can have
	Stat val[STAT_TOTAL];

	StatGroup() {}
	StatGroup(rapidxml::xml_node<char> *node) { load(node); }

	void Change(const pyrodactyl::stat::StatType &type, const int &change);
	void Set(const pyrodactyl::stat::StatType &type, const int &val);

	void load(rapidxml::xml_node<char> *node);
};

StatType StringToStatType(const Common::String &val);
const char *StatTypeToString(const StatType &val);
} // End of namespace stat
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_STAT_H
