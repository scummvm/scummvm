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

#ifndef KYRA_ITEM_H
#define KYRA_ITEM_H

#include "common/scummsys.h"

namespace Kyra {

typedef int16 Item;

enum ItemIndex : Item {
	/**
	 * Constant for invalid item.
	 */
	kItemNone = -1
};

enum ItemTypes : int8 {
	kItemTypeBow = 0,
	kItemTypeLongSword = 1,
	kItemTypeShortSword = 2,
	kItemTypeSling = 7,
	kItemTypeArrow = 16,
	kItemTypeRock = 18,
	kItemTypeRations = 31
};

enum RingTypes : int8 {
	kRingOfWizardry  = 1,
	kRingOfSustenance  = 2,
	kRingOfFeatherFalling = 3
};

} // End of namespace Kyra

#endif
