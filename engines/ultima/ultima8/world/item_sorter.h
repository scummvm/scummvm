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

#ifndef ULTIMA8_WORLD_ITEMSORTER_H
#define ULTIMA8_WORLD_ITEMSORTER_H

#include "common/rect.h"

namespace Ultima {
namespace Ultima8 {

class MainShapeArchive;
class Item;
class RenderSurface;
struct SortItem;
struct Point3;

class ItemSorter {
	MainShapeArchive    *_shapes;
	Common::Rect32      _clipWindow;

	SortItem    *_items;
	SortItem    *_itemsTail;
	SortItem    *_itemsUnused;
	SortItem    *_painted;

	int32       _camSx, _camSy;
	int32       _sortLimit;
	bool        _sortLimitChanged;

public:
	ItemSorter(int capacity);
	~ItemSorter();

	enum HitFace {
		X_FACE, Y_FACE, Z_FACE
	};

	// Begin creating the display list
	void BeginDisplayList(const Common::Rect32 &clipWindow, const Point3 &cam);

	void AddItem(const Point3 &pt, uint32 shape_num, uint32 frame_num, uint32 item_flags, uint32 ext_flags, uint16 item_num = 0);
	void AddItem(const Item *);                   // Add an Item. SetupLerp() MUST have been called

	// Finishes the display list and Paints
	void PaintDisplayList(RenderSurface *surf, bool item_highlight = false, bool showFootpads = false, int gridlines = 0);

	// Trace and find an object. Returns objid.
	// If face is non-NULL, also return the face of the 3d bbox (x,y) is on
	uint16 Trace(int32 x, int32 y, HitFace *face = 0, bool item_highlight = false);

	void IncSortLimit(int count);

private:
	bool PaintSortItem(RenderSurface *surf, SortItem *si, bool showFootpad, int gridlines);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
