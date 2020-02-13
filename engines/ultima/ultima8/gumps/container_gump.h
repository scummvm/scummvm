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

class ContainerGump : public ItemRelativeGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	ContainerGump();
	ContainerGump(Shape *shape, uint32 framenum, uint16 owner,
	              uint32 _Flags = FLAG_DRAGGABLE, int32 layer = LAYER_NORMAL);
	~ContainerGump(void) override;

	void setItemArea(Rect *itemarea_) {
		itemarea = *itemarea_;
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

	Gump *OnMouseDown(int button, int32 mx, int32 my) override;
	void OnMouseClick(int button, int32 mx, int32 my) override;
	void OnMouseDouble(int button, int32 mx, int32 my) override;

	bool loadData(IDataSource *ids, uint32 version);
protected:
	void saveData(ODataSource *ods) override;

	void GetItemLocation(int32 lerp_factor) override;

	virtual Container *getTargetContainer(Item *item, int mx, int my);

	void getItemCoords(Item *item, int32 &itemx, int32 &itemy);

	Rect itemarea;

	bool display_dragging;
	uint32 dragging_shape;
	uint32 dragging_frame;
	uint32 dragging_flags;
	int32 dragging_x, dragging_y;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
