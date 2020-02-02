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
	virtual ~ContainerGump(void);

	void setItemArea(Pentagram::Rect *itemarea_) {
		itemarea = *itemarea_;
	}

	// Close the gump
	virtual void Close(bool no_del = false);

	// Init the gump, call after construction
	virtual void InitGump(Gump *newparent, bool take_focus = true);

	// Paint the Gump
	virtual void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled);

	// Trace a click, and return ObjId
	virtual uint16 TraceObjId(int32 mx, int32 my);

	// Get the location of an item in the gump (coords relative to this).
	// Returns false on failure.
	virtual bool GetLocationOfItem(uint16 itemid, int32 &gx, int32 &gy,
	                               int32 lerp_factor = 256);


	virtual bool StartDraggingItem(Item *item, int mx, int my);
	virtual bool DraggingItem(Item *item, int mx, int my);
	virtual void DraggingItemLeftGump(Item *item);
	virtual void StopDraggingItem(Item *item, bool moved);
	virtual void DropItem(Item *item, int mx, int my);

	virtual Gump *OnMouseDown(int button, int32 mx, int32 my);
	virtual void OnMouseClick(int button, int32 mx, int32 my);
	virtual void OnMouseDouble(int button, int32 mx, int32 my);

	bool loadData(IDataSource *ids, uint32 version);
protected:
	virtual void saveData(ODataSource *ods);

	virtual void GetItemLocation(int32 lerp_factor);

	virtual Container *getTargetContainer(Item *item, int mx, int my);

	void getItemCoords(Item *item, int32 &itemx, int32 &itemy);

	Pentagram::Rect itemarea;

	bool display_dragging;
	uint32 dragging_shape;
	uint32 dragging_frame;
	uint32 dragging_flags;
	int32 dragging_x, dragging_y;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
