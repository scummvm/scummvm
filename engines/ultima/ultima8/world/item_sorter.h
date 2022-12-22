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

#include "ultima/ultima8/misc/rect.h"

namespace Ultima {
namespace Ultima8 {

class MainShapeArchive;
class Item;
class RenderSurface;
struct SortItem;

class ItemSorter {
	MainShapeArchive    *_shapes;
	Rect        _clipWindow;

	SortItem    *_items;
	SortItem    *_itemsTail;
	SortItem    *_itemsUnused;
	SortItem    *_painted;

	int32       _sortLimit;
	int32       _camSx, _camSy;

public:
	ItemSorter(int capacity);
	~ItemSorter();

	enum HitFace {
		X_FACE, Y_FACE, Z_FACE
	};

	// Begin creating the display list
	void BeginDisplayList(const Rect &clipWindow, int32 camx, int32 camy, int32 camz);

	void AddItem(int32 x, int32 y, int32 z, uint32 shape_num, uint32 frame_num, uint32 item_flags, uint32 ext_flags, uint16 item_num = 0);
	void AddItem(const Item *);                   // Add an Item. SetupLerp() MUST have been called

	// Finishes the display list and Paints
	void PaintDisplayList(RenderSurface *surf, bool item_highlight = false);

	// Trace and find an object. Returns objid.
	// If face is non-NULL, also return the face of the 3d bbox (x,y) is on
	uint16 Trace(int32 x, int32 y, HitFace *face = 0, bool item_highlight = false);

	void IncSortLimit(int count);

private:
	bool PaintSortItem(RenderSurface *surf, SortItem *si);

	//! Check Clipped. -1 if off screen, 0 if not clipped, 1 if clipped
	int16 CheckClipped(const Rect &) const;

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
