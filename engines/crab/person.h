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

#include "crab/ImageManager.h"
#include "crab/Stat.h"
#include "crab/StatTemplate.h"
#include "crab/common_header.h"
#include "crab/opinion.h"
#include "personbase.h"
#include "crab/trait.h"

namespace pyrodactyl {
namespace people {
struct Person {
	// The id of the object
	std::string id;

	// Opinion of the person towards the player
	Opinion opinion;

	// The state of the object, defines what behavior it is doing right now
	PersonState state;

	// The stats of the character
	pyrodactyl::stat::StatGroup stat;

	// Name of object
	std::string name;

	// Type of object
	PersonType type;

	// Sometimes a person's journal entry isn't the same as their name
	std::string journal_name;

	// If this is true, use the alternate journal name instead
	bool alt_journal_name;

	// The picture of the object - DISABLED DUE TO LOW BUDGET
	// ImageKey pic;

	// The trigger areas the person is in right now
	std::vector<int> trig;

	// The traits of a person
	std::vector<Trait> trait;

	Person();
	void Load(rapidxml::xml_node<char> *node, const pyrodactyl::stat::StatTemplates &stem);

	void Reset();
	void Validate();

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);
};

typedef std::unordered_map<std::string, Person> PersonMap;
} // End of namespace people
} // End of namespace pyrodactyl

#endif // CRAB_PERSON_H
