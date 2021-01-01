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

#ifndef ULTIMA8_GUMPS_CONTAINERGUMP_H
#define ULTIMA8_GUMPS_CONTAINERGUMP_H

#include "ultima/ultima8/gumps/item_relative_gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class Shape;
class Container;

/**
 * Base gump class for all containers (backpack, barrel, etc)
 */
class ContainerGump : public ItemRelativeGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	ContainerGump();
	ContainerGump(const Shape *shape, uint32 frameNum, uint16 owner,
	              uint32 flags = FLAG_DRAGGABLE, int32 layer = LAYER_NORMAL);
	~ContainerGump() override;

	void setItemArea(Rect *itemArea) {
		_itemArea = *itemArea;
	}

	// Close the gump
	void Close(bool no_del = false) override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;

	// Paint the Gump
	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	// Trace a click, and return ObjId
	uint16 TraceObjId(int32 mx, int32 my) override;

	// Get the location of an item in the gump (coords relative to this).
	// Returns false on failure.
	bool GetLocationOfItem(uint16 itemid, int32 &gx, int32 &gy,
	                               int32 lerp_factor = 256) override;


	bool StartDraggingItem(Item *item, int mx, int my) override;
	bool DraggingItem(Item *item, int mx, int my) override;
	void DraggingItemLeftGump(Item *item) override;
	void StopDraggingItem(Item *item, bool moved) override;
	void DropItem(Item *item, int mx, int my) override;

	Gump *onMouseDown(int button, int32 mx, int32 my) override;
	void onMouseClick(int button, int32 mx, int32 my) override;
	void onMouseDouble(int button, int32 mx, int32 my) override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	void GetItemLocation(int32 lerp_factor) override;

	virtual Container *getTargetContainer(Item *item, int mx, int my);

	void getItemCoords(Item *item, int32 &itemx, int32 &itemy);

	Rect _itemArea;

	bool _displayDragging;
	uint32 _draggingShape;
	uint32 _draggingFrame;
	uint32 _draggingFlags;
	int32 _draggingX, _draggingY;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
