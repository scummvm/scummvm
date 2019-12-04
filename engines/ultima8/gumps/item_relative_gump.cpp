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

#include "ultima8/misc/pent_include.h"
#include "ultima8/gumps/item_relative_gump.h"
#include "ultima8/gumps/game_map_gump.h"
#include "ultima8/world/item.h"
#include "ultima8/world/container.h"
#include "ultima8/graphics/shape_info.h"
#include "ultima8/world/get_object.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ItemRelativeGump, Gump)

ItemRelativeGump::ItemRelativeGump()
	: Gump(), ix(0), iy(0) {
}

ItemRelativeGump::ItemRelativeGump(int x_, int y_, int width, int height,
                                   uint16 owner_, uint32 _Flags, int32 _Layer)
	: Gump(x_, y_, width, height, owner_, _Flags, _Layer), ix(0), iy(0) {
}

ItemRelativeGump::~ItemRelativeGump(void) {
}

void ItemRelativeGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	GetItemLocation(0);

	if (!newparent && parent)
		MoveOnScreen();
}

void ItemRelativeGump::MoveOnScreen() {
	assert(parent);
	Pentagram::Rect sd, gd;
	parent->GetDims(sd);

	// first move back to our desired location
	x = 0;
	y = 0;

	// get rectangle that gump occupies in scalerGump's coordinate space
	int32 left, right, top, bottom;
	left = -dims.x;
	right = left + dims.w;
	top = -dims.y;
	bottom = top + dims.h;
	GumpToParent(left, top);
	GumpToParent(right, bottom);

	int32 movex = 0, movey = 0;

	if (left < -sd.x)
		movex = -sd.x - left;
	else if (right > -sd.x + sd.w)
		movex = -sd.x + sd.w - right;

	if (top < -sd.y)
		movey = -sd.y - top;
	else if (bottom > -sd.y + sd.h)
		movey = -sd.y + sd.h - bottom;

	Move(left + movex, top + movey);
}

// Paint the Gump (RenderSurface is relative to parent).
// Calls PaintThis and PaintChildren
void ItemRelativeGump::Paint(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	GetItemLocation(lerp_factor);
	Gump::Paint(surf, lerp_factor, scaled);
}


// Convert a parent relative point to a gump point
void ItemRelativeGump::ParentToGump(int &px, int &py, PointRoundDir r) {
	px -= ix;
	py -= iy;
	Gump::ParentToGump(px, py, r);
}

// Convert a gump point to parent relative point
void ItemRelativeGump::GumpToParent(int &gx, int &gy, PointRoundDir r) {
	Gump::GumpToParent(gx, gy, r);
	gx += ix;
	gy += iy;
}

void ItemRelativeGump::GetItemLocation(int32 lerp_factor) {
	Item *it = 0;
	Item *next = 0;
	Item *prev = 0;
	Gump *gump = 0;

	it = getItem(owner);

	if (!it) {
		// This shouldn't ever happen, the GumpNotifyProcess should
		// close us before we get here
		Close();
		return;
	}

	while ((next = it->getParentAsContainer()) != 0) {
		prev = it;
		it = next;
		gump = getGump(it->getGump());
		if (gump) break;
	}

	int gx, gy;

	if (!gump) {
		gump = GetRootGump()->FindGump(GameMapGump::ClassType);

		if (!gump) {
			perr << "ItemRelativeGump::GetItemLocation(): "
			     << "Unable to find GameMapGump!?!?" << std::endl;
			return;
		}

		gump->GetLocationOfItem(owner, gx, gy, lerp_factor);
	} else {
		gump->GetLocationOfItem(prev->getObjId(), gx, gy, lerp_factor);
	}

	// Convert the GumpSpaceCoord relative to the world/item gump
	// into screenspace coords
	gy = gy - it->getShapeInfo()->z * 8 - 16;
	gump->GumpToScreenSpace(gx, gy);

	// Convert the screenspace coords into the coords of us
	if (parent) parent->ScreenSpaceToGump(gx, gy);

	// Set x and y, and center us over it
	ix = gx - dims.w / 2;
//	iy = gy-dims.h-it->getShapeInfo()->z*8-16;
	iy = gy - dims.h;


	if (flags & FLAG_KEEP_VISIBLE)
		MoveOnScreen();
}

void ItemRelativeGump::Move(int x_, int y_) {
	ParentToGump(x_, y_);
	x += x_;
	y += y_;
}

void ItemRelativeGump::saveData(ODataSource *ods) {
	Gump::saveData(ods);
}

bool ItemRelativeGump::loadData(IDataSource *ids, uint32 version) {
	if (!Gump::loadData(ids, version)) return false;

	return true;
}

} // End of namespace Ultima8
