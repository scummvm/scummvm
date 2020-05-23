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

#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/usecode/intrinsics.h"

namespace Ultima {
namespace Ultima8 {

class MonsterEgg : public Item {
	friend class ItemFactory;
public:
	MonsterEgg();
	~MonsterEgg() override;

	ENABLE_RUNTIME_CLASSTYPE()

	int getProb() const {
		return (_quality >> 11) & 0x1F;
	}
	int getMonsterShape() const {
		return _quality & 0x7FF;
	}
	int getActivity() const {
		return _mapNum & 0x07;
	}

	uint16 hatch();

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	INTRINSIC(I_monsterEggHatch);
	INTRINSIC(I_getMonId);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
