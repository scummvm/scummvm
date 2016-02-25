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

#ifndef TITANIC_LINK_ITEM_H
#define TITANIC_LINK_ITEM_H

#include "titanic/objects/named_item.h"

namespace Titanic {

class CLinkItemSub {
public:
	int _field0;
	int _field4;
	int _field8;
	int _fieldC;
public:
	CLinkItemSub();
};

class CLinkItem : public CNamedItem {
protected:
	int _field24;
	int _field28;
	int _field2C;
	int _field30;
	int _field34;
	CLinkItemSub _sub;
public:
	CLinkItem();

	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CLinkItem"; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};

} // End of namespace Titanic

#endif /* TITANIC_LINK_ITEM_H */
