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

#include "crab/event/triggerset.h"

namespace Crab {

using namespace pyrodactyl::event;

void TriggerSet::load(rapidxml::xml_node<char> *node) {
	for (rapidxml::xml_node<char> *n = node->first_node("trigger"); n != nullptr; n = n->next_sibling("trigger")) {
		Trigger t(n);
		_statement.push_back(t);
	}
}

void TriggerSet::add(const Trigger &t) {
	_statement.push_back(t);
}

bool TriggerSet::evaluate(pyrodactyl::event::Info &info) {
	_result = true;

	if (_statement.empty() == false) {
		RelOp nextOp = OP_AND;
		bool curResult = false;

		for (auto &i : _statement) {
			if (i._negate)
				curResult = !i.evaluate(info);
			else
				curResult = i.evaluate(info);

			if (nextOp == OP_AND)
				_result = curResult && _result;
			else
				_result = curResult || _result;

			nextOp = i._rel;
		}
	}

	return _result;
}

} // End of namespace Crab
