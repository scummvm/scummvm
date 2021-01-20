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

#ifndef ULTIMA8_GUMPS_WEASELDAT_H
#define ULTIMA8_GUMPS_WEASELDAT_H

#include "common/stream.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Data for the Weasel (shop) gump on a single level. Contains a list of things you can buy.
 */
class WeaselDat {
public:

	enum WeaselType {
		kUnknown,
		kWeapon,
		kItem
	};

	/** A single item in the shop */
	struct WeaselEntry {
		char _id[4];		// eg, "W01", "I02", etc
		uint16 _shapeNo;
		uint32 _cost;
		uint16 _entryNo;
		uint16 _unk;
		enum WeaselType _type;
	};

	WeaselDat(Common::ReadStream *rs);

	uint16 getNumItems() const {
		return _items.size();
	}

	uint16 getNumOfType(WeaselType type) const;

	const Std::vector<WeaselEntry> &getItems() const {
		return _items;
	}

private:
	Std::vector<WeaselEntry> _items;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
