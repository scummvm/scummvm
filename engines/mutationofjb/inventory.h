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

#ifndef MUTATIONOFJB_INVENTORY_H
#define MUTATIONOFJB_INVENTORY_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str.h"

namespace MutationOfJB {

class Inventory {
public:
	typedef Common::Array<Common::String> Items;

	const Items &getItems() const;
	void addItem(const Common::String &item);
	void removeItem(const Common::String &item);
	void removeAllItems();

	void rotateItemsRight(uint n);
	void rotateItemsLeft(uint n);

private:
	void reverseItems(uint from, uint to);

	Items _items;
};

}

#endif
