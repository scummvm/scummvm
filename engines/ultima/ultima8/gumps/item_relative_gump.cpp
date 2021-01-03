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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/item_relative_gump.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/container.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ItemRelativeGump)

ItemRelativeGump::ItemRelativeGump() : Gump(), _ix(0), _iy(0) {
}

ItemRelativeGump::ItemRelativeGump(int32 x, int32 y, int32 width, int32 height,
                                   uint16 owner, uint32 flags, int32 layer)
	: Gump(x, y, width, height, owner, flags, layer), _ix(0), _iy(0) {
}

ItemRelativeGump::~ItemRelativeGump(void) {
}

void ItemRelativeGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	GetItemLocation(0);

	if (!newparent && _parent)
		MoveOnScreen();
}

void ItemRelativeGump::MoveOnScreen() {
	assert(_parent);
	Rect sd, gd;
	_parent->GetDims(sd);

	// first move back to our desired location
	_x = 0;
	_y = 0;

	// get rectangle that gump occupies in desktops's coordinate space
	int32 left, right, top, bottom;
	left = -_dims.left;
	right = left + _dims.width();
	top = -_dims.top;
	bottom = top + _dims.height();
	GumpToParent(left, top);
	GumpToParent(right, bottom);

	int32 movex = 0, movey = 0;

	if (left < -sd.left)
		movex = -sd.left - left;
	else if (right > -sd.left + sd.width())
		movex = -sd.left + sd.width() - right;

	if (top < -sd.top)
		movey = -sd.top - top;
	else if (bottom > -sd.top + sd.height())
		movey = -sd.top + sd.height() - bottom;

	Move(left + movex, top + movey);
}

// Paint the Gump (RenderSurface is relative to parent).
// Calls PaintThis and PaintChildren
void ItemRelativeGump::Paint(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	GetItemLocation(lerp_factor);
	Gump::Paint(surf, lerp_factor, scaled);
}


// Convert a parent relative point to a gump point
void ItemRelativeGump::ParentToGump(int32 &px, int32 &py, PointRoundDir r) {
	px -= _ix;
	py -= _iy;
	Gump::ParentToGump(px, py, r);
}

// Convert a gump point to parent relative point
void ItemRelativeGump::GumpToParent(int32 &gx, int32 &gy, PointRoundDir r) {
	Gump::GumpToParent(gx, gy, r);
	gx += _ix;
	gy += _iy;
}

void ItemRelativeGump::GetItemLocation(int32 lerp_factor) {
	Gump *gump = nullptr;

	Item *it = getItem(_owner);

	if (!it) {
		// This shouldn't ever happen, the GumpNotifyProcess should
		// close us before we get here
		Close();
		return;
	}

	Item *next;
	Item *prev = nullptr;
	while ((next = it->getParentAsContainer()) != nullptr) {
		prev = it;
		it = next;
		gump = getGump(it->getGump());
		if (gump) break;
	}

	int32 gx, gy;

	if (!gump) {
		gump = GetRootGump()->FindGump<GameMapGump>();

		if (!gump) {
			perr << "ItemRelativeGump::GetItemLocation(): "
			     << "Unable to find GameMapGump!?!?" << Std::endl;
			return;
		}

		gump->GetLocationOfItem(_owner, gx, gy, lerp_factor);
	} else {
		assert(prev);
		gump->GetLocationOfItem(prev->getObjId(), gx, gy, lerp_factor);
	}

	// Convert the GumpSpaceCoord relative to the world/item gump
	// into screenspace coords
	gy = gy - it->getShapeInfo()->_z * 8 - 16;
	gump->GumpToScreenSpace(gx, gy);

	// Convert the screenspace coords into the coords of us
	if (_parent) _parent->ScreenSpaceToGump(gx, gy);

	// Set x and y, and center us over it
	_ix = gx - _dims.width() / 2;
//	_iy = gy-_dims.h-it->getShapeInfo()->z*8-16;
	_iy = gy - _dims.height();


	if (_flags & FLAG_KEEP_VISIBLE)
		MoveOnScreen();
}

void ItemRelativeGump::Move(int32 x, int32 y) {
	ParentToGump(x, y);
	_x += x;
	_y += y;
}

void ItemRelativeGump::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);
}

bool ItemRelativeGump::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Gump::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
