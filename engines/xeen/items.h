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

#ifndef XEEN_ITEMS_H
#define XEEN_ITEMS_H

#include "common/scummsys.h"
#include "common/serializer.h"

namespace Xeen {

#define TOTAL_ITEMS 10

class XeenItem {
public:
	int _material;
	int _name;
	int _bonusFlags;
	bool _equipped;
public:
	XeenItem();

	void synchronize(Common::Serializer &s);

	int getElementalCategory() const;

	int getAttributeCategory() const;
};

class Treasure {
public:
	XeenItem _misc[TOTAL_ITEMS];
	XeenItem _accessories[TOTAL_ITEMS];
	XeenItem _armor[TOTAL_ITEMS];
	XeenItem _weapons[TOTAL_ITEMS];
	bool _hasItems;
public:
	Treasure();
};

} // End of namespace Xeen

#endif	/* XEEN_ITEMS_H */
