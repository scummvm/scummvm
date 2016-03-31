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

#ifndef TITANIC_PET_ROOMS_SECTION_H
#define TITANIC_PET_ROOMS_SECTION_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_control_sub11.h"
#include "titanic/pet_control/pet_control_sub12.h"
#include "titanic/pet_control/pet_control_list_item2.h"

namespace Titanic {

class CPetRoomsSection : public CPetSection {
private:
	CPetControlSub11 _sub11;
	CPetControlListItem2 _listItem;
	int _field100;
	int _field104;
	int _field108;
	int _field10C;
	int _field110;
	int _field114;
	int _field118;
	int _field11C;
	CPetGfxElement _val1;
	CPetControlSub12 _sub12;
	int _field1C0;
	int _field1C4;
	int _field1C8;
	int _field1CC;
	int _field1D0;
	int _field1D4;
public:
	CPetRoomsSection();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file, int param);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_ROOMS_SECTION_H */
