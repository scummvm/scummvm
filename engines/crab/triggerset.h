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

#ifndef CRAB_TRIGGERSET_H
#define CRAB_TRIGGERSET_H

#include "crab/common_header.h"
#include "crab/trigger.h"

namespace pyrodactyl {
namespace event {
class TriggerSet {
	// The conditions needed to unlock whatever we want
	// Places where this is used - events, levels, fighting moves
	std::vector<Trigger> statement;

	// Is the thing unlocked or not?
	bool result;

public:
	TriggerSet() { result = true; }

	void Load(rapidxml::xml_node<char> *node);
	bool Evaluate(pyrodactyl::event::Info &info);
	void Add(const Trigger &t);

	bool Result() { return result; }
	void Result(bool val) { result = val; }

	void Clear(const bool &val = true) {
		statement.clear();
		result = val;
	}
	bool Empty() { return statement.empty(); }
};
} // End of namespace event
} // End of namespace pyrodactyl

#endif // CRAB_TRIGGERSET_H
