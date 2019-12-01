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

#ifndef ULTIMA8_WORLD_MONSTEREGG_H
#define ULTIMA8_WORLD_MONSTEREGG_H

#include "ultima8/world/item.h"
#include "ultima8/usecode/intrinsics.h"

namespace Ultima8 {

class MonsterEgg : public Item {
	friend class ItemFactory;
public:
	MonsterEgg();
	virtual ~MonsterEgg();

	ENABLE_RUNTIME_CLASSTYPE()

	int getProb() const {
		return (quality >> 11) & 0x1F;
	}
	int getMonsterShape() const {
		return quality & 0x7FF;
	}
	int getActivity() const {
		return mapnum & 0x07;
	}

	uint16 hatch();

	bool loadData(IDataSource *ids, uint32 version);

	INTRINSIC(I_monsterEggHatch);
	INTRINSIC(I_getMonId);

protected:
	virtual void saveData(ODataSource *ods);
};

} // End of namespace Ultima8

#endif
