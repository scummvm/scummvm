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

#ifndef CRAB_OPINION_H
#define CRAB_OPINION_H
#include "crab/common_header.h"

#include "crab/GameParam.h"

namespace Crab {

namespace pyrodactyl {
namespace people {
// What each opinion type is
// like - how much a person likes you
// intimidate - how much a person fears you
// respect - how much a person respects you
enum OpinionType { OPI_LIKE,
				   OPI_RESPECT,
				   OPI_FEAR,
				   OPI_TOTAL };

// The limits on opinion values
extern int OPINION_MIN, OPINION_MAX;

struct Opinion {
	// The opinion of the character about the player
	// Range 0 to 100 , 100 = absolutely adore you, and 0 = really hate you
	int val[OPI_TOTAL];

	Opinion();
	~Opinion() {}

	void Change(const OpinionType &type, const int &change);
	void Set(const OpinionType &type, const int &val);
	void Validate(const OpinionType &type);

	void load(rapidxml::xml_node<char> *node);
	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
};
} // End of namespace people
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_OPINION_H
