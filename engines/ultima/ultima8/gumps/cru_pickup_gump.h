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

#ifndef ULTIMA8_GUMPS_CRUPICKUPGUMP_H
#define ULTIMA8_GUMPS_CRUPICKUPGUMP_H

#include "ultima/shared/std/string.h"
#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

class TextWidget;

/**
 * Pickup box, the box that appears in the top left when a new item is picked up
 */
class CruPickupGump : public Gump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	CruPickupGump();
	CruPickupGump(const Item *item, int y, bool showCount);

	~CruPickupGump() override {};

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;

	// Paint this Gump
	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	uint32 getShapeNo() const {
		return _itemShapeNo;
	}

	uint16 getQ() {
		return _q;
	}

	//! Update for a second item pickup - updates existing count text.
	void updateForNewItem(const Item *item);
	void addCountText();

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

private:
	uint32 _startFrame;
	uint32 _itemShapeNo;
	uint16 _gumpShapeNo;
	uint16 _gumpFrameNo;
	Std::string _itemName;
	uint16 _q;
	bool _showCount;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
