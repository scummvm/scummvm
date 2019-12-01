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

#ifndef ULTIMA8_GUMPS_GAMEMAPGUMP_H
#define ULTIMA8_GUMPS_GAMEMAPGUMP_H

#include "ultima8/gumps/gump.h"
#include "ultima8/std/containers.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class ItemSorter;
class CameraProcess;

class GameMapGump : public Gump {
protected:
	ItemSorter      *display_list;

public:
	ENABLE_RUNTIME_CLASSTYPE()

	GameMapGump();
	GameMapGump(int x, int y, int w, int h);
	virtual ~GameMapGump();

	virtual void        PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled);

	void                GetCameraLocation(int32 &x, int32 &y, int32 &z,
	                                      int lerp_factor = 256);

	// Trace a click, and return ObjId (parent coord space)
	virtual uint16      TraceObjId(int mx, int my);

	// Trace a click, return ObjId, and the coordinates of the mouse click (gump coord space)
	virtual uint16      TraceCoordinates(int mx, int my, int32 coords[3],
	                                     int offsetx = 0, int offsety = 0,
	                                     Item *item = 0);

	// Get the location of an item in the gump (coords relative to this).
	// Returns false on failure
	virtual bool        GetLocationOfItem(uint16 itemid, int &gx, int &gy,
	                                      int32 lerp_factor = 256);

	virtual bool        StartDraggingItem(Item *item, int mx, int my);
	virtual bool        DraggingItem(Item *item, int mx, int my);
	virtual void        DraggingItemLeftGump(Item *item);
	virtual void        StopDraggingItem(Item *item, bool moved);
	virtual void        DropItem(Item *item, int mx, int my);

	virtual Gump       *OnMouseDown(int button, int mx, int my);
	virtual void        OnMouseUp(int button, int mx, int my);
	virtual void        OnMouseClick(int button, int mx, int my);
	virtual void        OnMouseDouble(int button, int mx, int my);

	void IncSortOrder(int count);

	bool loadData(IDataSource *ids, uint32 version);

	static void         SetHighlightItems(bool highlight) {
		highlightItems = highlight;
	}
	static bool         isHighlightItems() {
		return highlightItems;
	}

	static void ConCmd_toggleHighlightItems(const Console::ArgvType &argv);
	static void ConCmd_dumpMap(const Console::ArgvType &argv);

	static void ConCmd_incrementSortOrder(const Console::ArgvType &argv);
	static void ConCmd_decrementSortOrder(const Console::ArgvType &argv);

	virtual void        RenderSurfaceChanged();

protected:
	virtual void saveData(ODataSource *ods);

	bool display_dragging;
	uint32 dragging_shape;
	uint32 dragging_frame;
	uint32 dragging_flags;
	int32 dragging_pos[3];

	static bool highlightItems;

};

} // End of namespace Ultima8

#endif
