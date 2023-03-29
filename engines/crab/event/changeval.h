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

#ifndef CRAB_CHANGEVAL_H
#define CRAB_CHANGEVAL_H

#include "crab/common_header.h"
#include "crab/people/opinion.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
struct ChangeVal {
	// The person whose opinion is changed
	std::string id;

	// How much does opinion change?
	int val[pyrodactyl::people::OPI_TOTAL];

	ChangeVal() {
		using namespace pyrodactyl::people;
		val[OPI_LIKE] = 0;
		val[OPI_RESPECT] = 0;
		val[OPI_FEAR] = 0;
	}

	ChangeVal(rapidxml::xml_node<char> *node) : ChangeVal() { Load(node); }

	void Load(rapidxml::xml_node<char> *node) {
		using namespace pyrodactyl::people;
		LoadStr(id, "id", node);
		LoadNum(val[OPI_LIKE], "like", node);
		LoadNum(val[OPI_RESPECT], "respect", node);
		LoadNum(val[OPI_FEAR], "fear", node);
	}
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_CHANGEVAL_H
