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


#include "crab/triggerset.h"

namespace Crab {

using namespace pyrodactyl::event;

void TriggerSet::Load(rapidxml::xml_node<char> *node) {
	for (rapidxml::xml_node<char> *n = node->first_node("trigger"); n != NULL; n = n->next_sibling("trigger")) {
		Trigger t(n);
		statement.push_back(t);
	}
}

void TriggerSet::Add(const Trigger &t) {
	statement.push_back(t);
}

bool TriggerSet::Evaluate(pyrodactyl::event::Info &info) {
	result = true;

	if (statement.empty() == false) {
		RelOp next_op = OP_AND;
		result = true;
		bool cur_result = false;

		for (auto i = statement.begin(); i != statement.end(); ++i) {
			if (i->negate)
				cur_result = !i->Evaluate(info);
			else
				cur_result = i->Evaluate(info);

			if (next_op == OP_AND)
				result = cur_result && result;
			else
				result = cur_result || result;

			next_op = i->rel;
		}
	} else
		result = true;

	return result;
}

} // End of namespace Crab
