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


#include "crab/people/opinion.h"

namespace Crab {

namespace pyrodactyl {
namespace people {
int OPINION_MIN = 0, OPINION_MAX = 100;
}
} // End of namespace pyrodactyl

using namespace pyrodactyl::people;

Opinion::Opinion() {
	val[OPI_LIKE] = 0;
	val[OPI_FEAR] = 0;
	val[OPI_RESPECT] = 0;
}

void Opinion::Load(rapidxml::xml_node<char> *node) {
	LoadNum(val[OPI_LIKE], "like", node);
	LoadNum(val[OPI_FEAR], "fear", node);
	LoadNum(val[OPI_RESPECT], "respect", node);
}

void Opinion::Change(const OpinionType &type, const int &change) {
	val[type] += change;
	Validate(type);
}

void Opinion::Set(const OpinionType &type, const int &num) {
	val[type] = num;
	Validate(type);
}

void Opinion::Validate(const OpinionType &type) {
	if (val[type] < OPINION_MIN)
		val[type] = OPINION_MIN;
	else if (val[type] > OPINION_MAX)
		val[type] = OPINION_MAX;
}

void Opinion::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "opinion");
	child->append_attribute(doc.allocate_attribute("like", gStrPool->Get(val[OPI_LIKE])));
	child->append_attribute(doc.allocate_attribute("fear", gStrPool->Get(val[OPI_FEAR])));
	child->append_attribute(doc.allocate_attribute("respect", gStrPool->Get(val[OPI_RESPECT])));
	root->append_node(child);
}

} // End of namespace Crab
