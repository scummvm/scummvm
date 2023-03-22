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

#ifndef CRAB_STATDRAWHELPER_H
#define CRAB_STATDRAWHELPER_H
#include "common_header.h"

#include "TextData.h"
#include "person.h"

namespace pyrodactyl {
namespace stat {
struct StatInfo {
	// Used to draw stat value and description
	pyrodactyl::ui::TextData desc;
	std::string text;

	Rect dim;
	ImageKey full, empty;
	bool active;

	StatInfo() {
		active = false;
		full = 0;
		empty = 0;
	}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(const int &val, const int &max);
};

class StatDrawHelper {
	StatInfo info[STAT_TOTAL];

public:
	StatDrawHelper() {}
	~StatDrawHelper() {}

	void Load(rapidxml::xml_node<char> *node);
	void DrawInfo(const pyrodactyl::people::Person &obj);

	const std::string &Name(const StatType &type) { return info[type].text; }
};
} // End of namespace stat
} // End of namespace pyrodactyl

#endif // CRAB_STATDRAWHELPER_H
