/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA4_MAP_CITY_H
#define ULTIMA4_MAP_CITY_H

#include "ultima/ultima4/map/map.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class Person;
class Dialogue;

struct PersonRole {
	int _role;
	int _id;
};

typedef Std::vector<Person *> PersonList;
typedef Common::List<PersonRole *> PersonRoleList;

class City : public Map {
public:
	City();
	~City() override;

	// Members
	/**
	 * Returns the name of the city
	 */
	Common::String getName() override;

	/**
	 * Adds a person object to the map
	 */
	Person *addPerson(Person *p);

	/**
	 * Add people to the map
	 */
	void addPeople();

	/**
	 * Removes all people from the current map
	 */
	void removeAllPeople();

	/**
	 * Returns the person object at the given (x,y,z) coords, if one exists.
	 * Otherwise, returns nullptr.
	 */
	Person *personAt(const Coords &coords);

	// Properties
	Common::String _name;
	Common::String _type;
	PersonList _persons;
	Common::String _tlkFname;
	PersonRoleList _personRoles;
	Std::vector<Dialogue *> _extraDialogues;
};

/**
 * Returns true if the Map pointed to by 'punknown'
 * is a City map
 */
bool isCity(Map *punknown);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
