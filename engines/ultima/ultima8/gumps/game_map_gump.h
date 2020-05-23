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

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class ItemSorter;
class CameraProcess;

/**
 * The  gump which holds all the game map elements (floor, avatar, objects, etc)
 */
class GameMapGump : public Gump {
protected:
	ItemSorter *_displayList;

public:
	ENABLE_RUNTIME_CLASSTYPE()

	GameMapGump();
	GameMapGump(int x, int y, int w, int h);
	~GameMapGump() override;

	void        PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) override;

	void                GetCameraLocation(int32 &x, int32 &y, int32 &z,
	                                      int lerp_factor = 256);

	// Trace a click, and return ObjId (_parent coord space)
	uint16      TraceObjId(int32 mx, int32 my) override;

	// Trace a click, return ObjId, and the coordinates of the mouse click (gump coord space)
	virtual uint16      TraceCoordinates(int mx, int my, int32 coords[3],
	                                     int offsetx = 0, int offsety = 0,
	                                     Item *item = 0);

	// Get the location of an item in the gump (coords relative to this).
	// Returns false on failure
	bool        GetLocationOfItem(uint16 itemid, int32 &gx, int32 &gy,
	                                      int32 lerp_factor = 256) override;

	bool        StartDraggingItem(Item *item, int mx, int my) override;
	bool        DraggingItem(Item *item, int mx, int my) override;
	void        DraggingItemLeftGump(Item *item) override;
	void        StopDraggingItem(Item *item, bool moved) override;
	void        DropItem(Item *item, int mx, int my) override;

	Gump       *onMouseDown(int button, int32 mx, int32 my) override;
	void        onMouseUp(int button, int32 mx, int32 my) override;
	void        onMouseClick(int button, int32 mx, int32 my) override;
	void        onMouseDouble(int button, int32 mx, int32 my) override;

	void IncSortOrder(int count);

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	static void Set_highlightItems(bool highlight) {
		_highlightItems = highlight;
	}
	static bool is_highlightItems() {
		return _highlightItems;
	}

	void        RenderSurfaceChanged() override;

protected:
	bool _displayDragging;
	uint32 _draggingShape;
	uint32 _draggingFrame;
	uint32 _draggingFlags;
	int32 _draggingPos[3];

	static bool _highlightItems;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
