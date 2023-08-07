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

#include "crab/people/person.h"
#include "crab/ui/TextData.h"

namespace Crab {

namespace pyrodactyl {
namespace stat {
struct StatInfo {
	// Used to draw stat value and description
	pyrodactyl::ui::TextData _desc;
	Common::String _text;

	Rect _dim;
	ImageKey _full, _empty;
	bool _active;

	StatInfo() {
		_active = false;
		_full = 0;
		_empty = 0;
	}

	void load(rapidxml::xml_node<char> *node);
	void draw(const int &val, const int &max);
};

class StatDrawHelper {
	StatInfo _info[STAT_TOTAL];

public:
	StatDrawHelper() {}
	~StatDrawHelper() {}

	void load(rapidxml::xml_node<char> *node);
	void drawInfo(const pyrodactyl::people::Person &obj);

	const Common::String &name(const StatType &type) {
		return _info[type]._text;
	}
};
} // End of namespace stat
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_STATDRAWHELPER_H
