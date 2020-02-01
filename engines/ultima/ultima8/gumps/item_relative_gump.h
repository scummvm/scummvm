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

class ItemRelativeGump : public Gump {
protected:
	int32 ix, iy;

public:
	ENABLE_RUNTIME_CLASSTYPE()

	ItemRelativeGump();
	ItemRelativeGump(int32 x, int32 y, int32 width, int32 height, uint16 owner, uint32 _Flags = 0, int32 layer = LAYER_NORMAL);
	virtual ~ItemRelativeGump(void);

	virtual void        InitGump(Gump *newparent, bool take_focus = true);

	// Paint the Gump (RenderSurface is relative to parent).
	// Calls PaintThis and PaintChildren.
	virtual void        Paint(RenderSurface *, int32 lerp_factor, bool scaled);

	virtual void        ParentToGump(int &px, int &py,
	                                 PointRoundDir r = ROUND_TOPLEFT);
	virtual void        GumpToParent(int &gx, int &gy,
	                                 PointRoundDir r = ROUND_TOPLEFT);

	virtual void        Move(int x, int y);

	bool                loadData(IDataSource *ids, uint32 version);
protected:
	virtual void        saveData(ODataSource *ods);

	virtual void        GetItemLocation(int32 lerp_factor);

	//! Move Gump so that it totally overlaps parent.
	void                MoveOnScreen();
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
