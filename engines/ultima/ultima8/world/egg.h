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

namespace Ultima {
namespace Ultima8 {

class Egg : public Item {
	friend class ItemFactory;
public:
	Egg();
	~Egg() override;

	ENABLE_RUNTIME_CLASSTYPE()

	int getXRange() const {
		return (_npcNum >> 4) & 0xF;
	}
	int getYRange() const {
		return _npcNum & 0xF;
	}

	void setXRange(int r) {
		_npcNum &= 0x0F;
		_npcNum |= (r & 0xF) << 4;
	}
	void setYRange(int r) {
		_npcNum &= 0xF0;
		_npcNum |= (r & 0xF);
	}

	//! hatch the egg
	virtual uint16 hatch();

	//! unhatch the egg (for Crusader only)
	virtual uint16 unhatch();

	//! The item has left the fast area
	void leaveFastArea() override;

	//! clear the '_hatched' flag
	void reset() {
		_hatched = false;
	}

	void dumpInfo() const override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	INTRINSIC(I_getEggXRange);
	INTRINSIC(I_getEggYRange);
	INTRINSIC(I_setEggXRange);
	INTRINSIC(I_setEggYRange);
	INTRINSIC(I_getEggId);
	INTRINSIC(I_setEggId);

protected:
	bool _hatched;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
