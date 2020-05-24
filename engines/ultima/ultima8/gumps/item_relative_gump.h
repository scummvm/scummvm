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

#ifndef ULTIMA8_GUMPS_ITEMRELATIVEGUMP_H
#define ULTIMA8_GUMPS_ITEMRELATIVEGUMP_H

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Base class for gumps whose location is relative to an owning item (eg, inventory, bark, etc)
 */
class ItemRelativeGump : public Gump {
protected:
	int32 _ix, _iy;

public:
	ENABLE_RUNTIME_CLASSTYPE()

	ItemRelativeGump();
	ItemRelativeGump(int32 x, int32 y, int32 width, int32 height, uint16 owner, uint32 flags = 0, int32 layer = LAYER_NORMAL);
	~ItemRelativeGump() override;

	void        InitGump(Gump *newparent, bool take_focus = true) override;

	// Paint the Gump (RenderSurface is relative to parent).
	// Calls PaintThis and PaintChildren.
	void        Paint(RenderSurface *, int32 lerp_factor, bool scaled) override;

	void        ParentToGump(int32 &px, int32 &py,
	                                 PointRoundDir r = ROUND_TOPLEFT) override;
	void        GumpToParent(int32 &gx, int32 &gy,
	                                 PointRoundDir r = ROUND_TOPLEFT) override;

	void        Move(int32 x, int32 y) override;

	bool                loadData(Common::ReadStream *rs, uint32 version);
protected:
	void        saveData(Common::WriteStream *ws) override;

	virtual void        GetItemLocation(int32 lerp_factor);

	//! Move Gump so that it totally overlaps parent.
	void                MoveOnScreen();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
