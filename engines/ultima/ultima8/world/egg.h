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
#ifndef ULTIMA8_WORLD_EGG_H
#define ULTIMA8_WORLD_EGG_H

#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/usecode/intrinsics.h"

namespace Ultima8 {

class Egg : public Item {
	friend class ItemFactory;
public:
	Egg();
	virtual ~Egg();

	ENABLE_RUNTIME_CLASSTYPE()

	int getXRange() const {
		return (npcnum >> 4) & 0xF;
	}
	int getYRange() const {
		return npcnum & 0xF;
	}

	void setXRange(int r) {
		npcnum &= 0x0F;
		npcnum |= (r & 0xF) << 4;
	}
	void setYRange(int r) {
		npcnum &= 0xF0;
		npcnum |= (r & 0xF);
	}

	//! hatch the egg
	virtual uint16 hatch();

	//! The item has left the fast area
	virtual void leaveFastArea();

	//! clear the 'hatched' flag
	void reset() {
		hatched = false;
	}

	virtual void dumpInfo();

	bool loadData(IDataSource *ids, uint32 version);

	INTRINSIC(I_getEggXRange);
	INTRINSIC(I_getEggYRange);
	INTRINSIC(I_setEggXRange);
	INTRINSIC(I_setEggYRange);
	INTRINSIC(I_getEggId);
	INTRINSIC(I_setEggId);

protected:
	virtual void saveData(ODataSource *ods);

	bool hatched;
};

} // End of namespace Ultima8

#endif
