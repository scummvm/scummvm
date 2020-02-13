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

#ifndef TITANIC_VARIABLE_LIST_H
#define TITANIC_VARIABLE_LIST_H

#include "titanic/core/list.h"

namespace Titanic {

class CVariableListItem : public ListItem {
public:
	CString _string1;
	int _field18;
	CString _string2;
	CString _string3;
	CString _string4;
	int _field40;
	int _field44;
public:
	CLASSDEF;
	CVariableListItem() : ListItem(), _field18(0), _field40(0), _field44(0) {}

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};


/**
 * Movie clip list
 */
class CVariableList: public List<CVariableListItem> {
};

} // End of namespace Titanic

#endif /* TITANIC_VARIABLE_LIST_H */
