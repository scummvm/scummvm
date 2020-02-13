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

#ifndef TITANIC_PLACE_HOLDER_ITEM_H
#define TITANIC_PLACE_HOLDER_ITEM_H

#include "titanic/core/named_item.h"
#include "titanic/game/variable_list.h"
#include "titanic/support/movie_clip.h"

namespace Titanic {

class CPlaceHolderItem : public CNamedItem {
	DECLARE_MESSAGE_MAP;
public:
	CString _string1;
	Point _pos1;
	Point _pos2;
	CString _string2;
	int _field4C;
	CVariableList _list;
	int _field60;
	int _field64;
	int _field68;
	CMovieClipList _clips;
	int _field7C;
public:
	CLASSDEF;
	CPlaceHolderItem();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

} // End of namespace Titanic

#endif /* TITANIC_PLACE_HOLDER_ITEM_H */
