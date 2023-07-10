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

#ifndef CRAB_PERSON_H
#define CRAB_PERSON_H

#include "crab/image/ImageManager.h"
#include "crab/stat/Stat.h"
#include "crab/stat/StatTemplate.h"
#include "crab/common_header.h"
#include "crab/people/opinion.h"
#include "crab/people/personbase.h"
#include "crab/people/trait.h"

namespace Crab {

namespace pyrodactyl {
namespace people {
struct Person {
	// The id of the object
	Common::String id;

	// Opinion of the person towards the player
	Opinion opinion;

	// The state of the object, defines what behavior it is doing right now
	PersonState state;

	// The stats of the character
	pyrodactyl::stat::StatGroup stat;

	// Name of object
	Common::String name;

	// Type of object
	PersonType type;

	// Sometimes a person's journal entry isn't the same as their name
	Common::String journal_name;

	// If this is true, use the alternate journal name instead
	bool alt_journal_name;

	// The picture of the object - DISABLED DUE TO LOW BUDGET
	// ImageKey pic;

	// The trigger areas the person is in right now
	Common::Array<int> trig;

	// The traits of a person
	Common::Array<Trait> trait;

	Person();
	void load(rapidxml::xml_node<char> *node, const pyrodactyl::stat::StatTemplates &stem);

	void reset();
	void Validate();

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);
};

typedef Common::HashMap<Common::String, Person> PersonMap;
} // End of namespace people
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_PERSON_H
