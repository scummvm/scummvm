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

#ifndef ULTIMA8_WORLD_ITEMSORTER_H
#define ULTIMA8_WORLD_ITEMSORTER_H

namespace Ultima {
namespace Ultima8 {

class MainShapeArchive;
class Item;
class RenderSurface;
struct SortItem;

class ItemSorter {
	MainShapeArchive    *shapes;
	RenderSurface   *surf;

	SortItem    *items;
	SortItem    *items_tail;
	SortItem    *items_unused;
	int32       sort_limit;

	int32       order_counter;

	int32       cam_sx, cam_sy;

public:
	ItemSorter();
	~ItemSorter();

	enum HitFace {
		X_FACE, Y_FACE, Z_FACE
	};

	// Begin creating the display list
	void BeginDisplayList(RenderSurface *,
	                      int32 camx, int32 camy, int32 camz);

	void AddItem(int32 x, int32 y, int32 z, uint32 shape_num, uint32 frame_num, uint32 item_flags, uint32 ext_flags, uint16 item_num = 0);
	void AddItem(Item *);                   // Add an Item. SetupLerp() MUST have been called

	void PaintDisplayList(bool item_highlight = false);             // Finishes the display list and Paints

	// Trace and find an object. Returns objid.
	// If face is non-NULL, also return the face of the 3d bbox (x,y) is on
	uint16 Trace(int32 x, int32 y, HitFace *face = 0, bool item_highlight = false);

	void IncSortLimit() {
		sort_limit++;
	}
	void DecSortLimit() {
		if (sort_limit > 0) sort_limit--;
	}

private:
	bool PaintSortItem(SortItem *);
	bool NullPaintSortItem(SortItem *);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
