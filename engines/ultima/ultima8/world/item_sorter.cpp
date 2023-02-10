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

#include "ultima/ultima.h"
#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/world/item_sorter.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/ultima8.h"

// temp
#include "ultima/ultima8/world/actors/weapon_overlay.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/get_object.h"
// --

#include "ultima/ultima8/world/sort_item.h"

namespace Ultima {
namespace Ultima8 {

ItemSorter::ItemSorter(int capacity) :
	_shapes(nullptr), _clipWindow(0, 0, 0, 0), _items(nullptr), _itemsTail(nullptr),
	_itemsUnused(nullptr), _painted(nullptr), _sortLimit(0), _camSx(0), _camSy(0) {
	int i = capacity;
	while (i--) _itemsUnused = new SortItem(_itemsUnused);
}

ItemSorter::~ItemSorter() {
	//
	if (_itemsTail) {
		_itemsTail->_next = _itemsUnused;
		_itemsUnused = _items;
	}
	_items = nullptr;
	_itemsTail = nullptr;

	while (_itemsUnused) {
		SortItem *_next = _itemsUnused->_next;
		delete _itemsUnused;
		_itemsUnused = _next;
	}

	delete [] _items;
}

void ItemSorter::BeginDisplayList(const Rect &clipWindow, int32 camx, int32 camy, int32 camz) {
	// Get the _shapes, if required
	if (!_shapes) _shapes = GameData::get_instance()->getMainShapes();

	// Set the clip window, and reset the item list
	_clipWindow = clipWindow;

	if (_itemsTail) {
		_itemsTail->_next = _itemsUnused;
		_itemsUnused = _items;
	}

	_items = nullptr;
	_itemsTail = nullptr;
	_painted = nullptr;

	// Screenspace bounding box bottom x coord (RNB x coord)
	_camSx = (camx - camy) / 4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	_camSy = (camx + camy) / 8 - camz;
}

void ItemSorter::AddItem(int32 x, int32 y, int32 z, uint32 shapeNum, uint32 frame_num, uint32 flags, uint32 ext_flags, uint16 itemNum) {

	// First thing, get a SortItem to use (first of unused)
	if (!_itemsUnused)
		_itemsUnused = new SortItem(nullptr);
	SortItem *si = _itemsUnused;

	si->_itemNum = itemNum;
	si->_shape = _shapes->getShape(shapeNum);
	si->_shapeNum = shapeNum;
	si->_frame = frame_num;
	const ShapeFrame *frame = si->_shape ? si->_shape->getFrame(si->_frame) : nullptr;
	if (!frame) {
		// Keep the last shape we skipped so we don't spam the warnings too much
		static uint32 last_invalid_shape = 0;
		static uint32 last_invalid_frame = 0;
		if (si->_shapeNum != last_invalid_shape || si->_frame != last_invalid_frame) {
			warning("Skipping invalid shape in sorter: %d,%d", si->_shapeNum, si->_frame);
			last_invalid_frame = si->_frame;
			last_invalid_shape = si->_shapeNum;
		}
		return;
	}

	si->_flags = flags;
	si->_extFlags = ext_flags;

	const ShapeInfo *info = _shapes->getShapeInfo(shapeNum);
	// Dimensions
	int32 xd, yd, zd;
	info->getFootpadWorld(xd, yd, zd, flags & Item::FLG_FLIPPED);

	// Worldspace bounding box
	si->_x = x;
	si->_y = y;
	si->_z = z;
	si->_xLeft = si->_x - xd;
	si->_yFar = si->_y - yd;
	si->_zTop = si->_z + zd;

	si->calculateBoxBounds(_camSx, _camSy);

	// Real Screenspace coords
	si->_sx = si->_sxBot - frame->_xoff;   // Left
	si->_sy = si->_syBot - frame->_yoff;   // Top
	si->_sx2 = si->_sx + frame->_width;    // Right
	si->_sy2 = si->_sy + frame->_height;   // Bottom

	// Do Clipping here
	int16 clipped = CheckClipped(Rect(si->_sx, si->_sy, si->_sx2, si->_sy2));
	if (clipped < 0)
		// Clipped away entirely - don't add to the list.
		return;

	si->_clipped = (clipped != 0);

	// These help out with sorting. We calc them now, so it will be faster
	si->_fbigsq = (xd == 128 && yd == 128) || (xd == 256 && yd == 256) || (xd == 512 && yd == 512);
	si->_flat = zd == 0;

	si->_draw = info->is_draw();
	si->_solid = info->is_solid();
	si->_occl = info->is_occl() && !(si->_flags & Item::FLG_INVISIBLE) &&
			   !(si->_extFlags & Item::EXT_TRANSPARENT);
	si->_roof = info->is_roof();
	si->_noisy = info->is_noisy();
	si->_anim = info->_animType != 0;
	si->_trans = info->is_translucent();
	si->_fixed = info->is_fixed();
	si->_land = info->is_land();
	if (GAME_IS_CRUSADER) {
		si->_sprite = si->_extFlags & Item::EXT_SPRITE;
		si->_invitem = info->is_invitem();
	}

	si->_occluded = false;
	si->_order = -1;

	// We will clear all the vector memory
	// Stictly speaking the vector will sort of leak memory, since they
	// are never deleted
	si->_depends.clear();

	// Iterate the list and compare _shapes

	// Ok,
	SortItem *addpoint = nullptr;
	for (SortItem *si2 = _items; si2 != nullptr; si2 = si2->_next) {
		// Get the insert point... which is before the first item that has higher z than us
		if (!addpoint && si->ListLessThan(si2))
			addpoint = si2;

		// Doesn't overlap
		if (si2->_occluded || !si->overlap(*si2))
			continue;

		// Attempt to find which is infront
		if (si->below(*si2)) {
			// si2 occludes si (us)
			if (si2->_occl && si2->occludes(*si)) {
				// No need to do any more checks, this isn't visible
				si->_occluded = true;
				break;
			}

			// si1 is behind si2, so add it to si2's dependency list
			si2->_depends.insert_sorted(si);
		} else {
			// ss occludes si2. Sadly, we can't remove it from the list.
			if (si->_occl && si->occludes(*si2))
				si2->_occluded = true;
			// si2 is behind si1, so add it to si1's dependency list
			else
				si->_depends.push_back(si2);
		}
	}

	// Add it to the list
	_itemsUnused = _itemsUnused->_next;

	// have a position
	//addpoint = 0;
	if (addpoint) {
		si->_next = addpoint;
		si->_prev = addpoint->_prev;
		addpoint->_prev = si;
		if (si->_prev)
			si->_prev->_next = si;
		else
			_items = si;
	}
	// Add it to the end of the list
	else {
		if (_itemsTail)
			_itemsTail->_next = si;
		if (!_items)
			_items = si;
		si->_next = nullptr;
		si->_prev = _itemsTail;
		_itemsTail = si;
	}
}

void ItemSorter::AddItem(const Item *add) {
	int32 x, y, z;
	add->getLerped(x, y, z);
	AddItem(x, y, z, add->getShape(), add->getFrame(),
			add->getFlags(), add->getExtFlags(), add->getObjId());
}

void ItemSorter::PaintDisplayList(RenderSurface *surf, bool item_highlight) {
	if (_sortLimit) {
		// Clear the surface when debugging the sorter
		surf->Fill32(0, _clipWindow);
	}

	SortItem *it = _items;
	SortItem *end = nullptr;
	_painted = nullptr;  // Reset the paint tracking
	while (it != end) {
		if (it->_order == -1) if (PaintSortItem(surf, it)) return;
		it = it->_next;
	}

	// Item highlighting. We redraw each 'item' transparent
	if (item_highlight) {
		it = _items;
		while (it != end) {
			if (!(it->_flags & (Item::FLG_DISPOSABLE | Item::FLG_FAST_ONLY)) && !it->_fixed) {
				surf->PaintHighlightInvis(it->_shape,
				                          it->_frame,
				                          it->_sxBot,
				                          it->_syBot,
				                          it->_trans,
				                          (it->_flags & Item::FLG_FLIPPED) != 0, 0x1f00ffff);
			}

			it = it->_next;
		}

	}
}

/**
 * Recursively paint this item and all its dependencies.
 * Returns true if recursion should stop.
 */
bool ItemSorter::PaintSortItem(RenderSurface *surf, SortItem *si) {
	// Don't paint this, or dependencies (yet) if occluded
	if (si->_occluded)
		return false;

	// Resursion detection
	si->_order = -2;

	// Iterate through our dependancies, and paint them, if possible
	SortItem::DependsList::iterator it = si->_depends.begin();
	SortItem::DependsList::iterator end = si->_depends.end();
	while (it != end) {
		if ((*it)->_order == -2) {
			//warning("cycle in paint dependency graph %d -> %d -> ... -> %d",
			//		si->_shapeNum, (*it)->_shapeNum, si->_shapeNum);
			break;
		}
		else if ((*it)->_order == -1) {
			if (PaintSortItem(surf, *it))
				return true;
		}
		++it;
	}

	// Set our painting _order based on previously painted item
	si->_order = _painted ? _painted->_order + 1 : 0;

	// Now paint us!
	if (surf) {
		//	if (wire) si->info->draw_box_back(s, dispx, dispy, 255);

		if (si->_extFlags & Item::EXT_HIGHLIGHT && si->_extFlags & Item::EXT_TRANSPARENT)
			surf->PaintHighlightInvis(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans, (si->_flags & Item::FLG_FLIPPED) != 0, 0x7F00007F);
		if (si->_extFlags & Item::EXT_HIGHLIGHT)
			surf->PaintHighlight(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans, (si->_flags & Item::FLG_FLIPPED) != 0, 0x7F00007F);
		else if (si->_extFlags & Item::EXT_TRANSPARENT)
			surf->PaintInvisible(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans, (si->_flags & Item::FLG_FLIPPED) != 0);
		else if (si->_flags & Item::FLG_FLIPPED)
			surf->PaintMirrored(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans);
		else if (si->_trans)
			surf->PaintTranslucent(si->_shape, si->_frame, si->_sxBot, si->_syBot);
		else if (!si->_clipped)
			surf->PaintNoClip(si->_shape, si->_frame, si->_sxBot, si->_syBot);
		else
			surf->Paint(si->_shape, si->_frame, si->_sxBot, si->_syBot);

	//	if (wire) si->info->draw_box_front(s, dispx, dispy, 255);

		// weapon overlay
		// FIXME: use highlight/invisibility, also add to Trace() ?
		if (si->_shapeNum == 1 && si->_itemNum == 1) {
			MainActor *av = getMainActor();
			const WeaponOverlayFrame *wo_frame = nullptr;
			uint32 wo_shapenum;
			av->getWeaponOverlay(wo_frame, wo_shapenum);
			if (wo_frame) {
				const Shape *wo_shape = GameData::get_instance()->getMainShapes()->getShape(wo_shapenum);
				surf->Paint(wo_shape, wo_frame->_frame,
							si->_sxBot + wo_frame->_xOff,
							si->_syBot + wo_frame->_yOff);
			}
		}
	}

	if (_sortLimit) {
		if (si->_order == _sortLimit) {
			if (!_painted || _painted->_itemNum != si->_itemNum) {
				debugC(kDebugObject, "SortItem: %s", si->dumpInfo().c_str());
				if (_painted && si->overlap(_painted)) {
					debugC(kDebugObject, "Overlaps: %s", _painted->dumpInfo().c_str());
				}
			}

			_painted = si;
			return true;
		}
	}

	_painted = si;
	return false;
}

uint16 ItemSorter::Trace(int32 x, int32 y, HitFace *face, bool item_highlight) {
	SortItem *it;
	SortItem *selected;

	if (!_painted) { // If no painted item found, we need to sort the items
		it = _items;
		_painted = nullptr;
		while (it != nullptr) {
			if (it->_order == -1) if (PaintSortItem(nullptr ,it)) break;

			it = it->_next;
		}
	}

	// Firstly, we check for highlighted _items
	selected = nullptr;

	if (item_highlight) {
		selected = nullptr;

		for (it = _itemsTail; it != nullptr; it = it->_prev) {
			if (!(it->_flags & (Item::FLG_DISPOSABLE | Item::FLG_FAST_ONLY)) && !it->_fixed) {

				if (!it->_itemNum) continue;

				// Doesn't Overlap
				if (x < it->_sx || x >= it->_sx2 || y < it->_sy || y >= it->_sy2) continue;

				// Skip transparent non-solids
				if (!it->_solid && it->_trans)
					continue;

				// Now check the _frame itself
				const ShapeFrame *_frame = it->_shape->getFrame(it->_frame);
				assert(_frame); // invalid frames shouldn't have been added to the list

				// Nope, doesn't have a point
				if (it->_flags & Item::FLG_FLIPPED) {
					if (!_frame->hasPoint(it->_sxBot - x, y - it->_syBot)) continue;
				} else {
					if (!_frame->hasPoint(x - it->_sxBot, y - it->_syBot)) continue;
				}

				// Ok now check against selected
				selected = it;
			}
		}

	}

	// Ok, this is all pretty simple. We iterate all the _items.
	// We then check to see if the item has a point where the trace goes.
	// Finally we then set the selected SortItem if it's '_order' is highest

	if (!selected) {
		for (it = _items; it != nullptr; it = it->_next) {
			if (!it->_itemNum) continue;

			// Doesn't Overlap
			if (x < it->_sx || x >= it->_sx2 || y < it->_sy || y >= it->_sy2) continue;

			// Skip transparent non-solids
			if (!it->_solid && it->_trans)
				continue;

			// Now check the _frame itself
			const ShapeFrame *_frame = it->_shape->getFrame(it->_frame);
			assert(_frame); // invalid frames shouldn't have been added to the list

			// Nope, doesn't have a point
			if (it->_flags & Item::FLG_FLIPPED) {
				if (!_frame->hasPoint(it->_sxBot - x, y - it->_syBot)) continue;
			} else {
				if (!_frame->hasPoint(x - it->_sxBot, y - it->_syBot)) continue;
			}

			// Ok now check against selected
			if (!selected || (it->_order > selected->_order)) selected = it;
		}
	}

	if (selected) {

		if (face) {
			// shortcut for zero-height _items
			if (selected->_zTop == selected->_z) {
				*face = Z_FACE;
			} else {
				// determine face that was hit

				// RNT coordinates
				int32 RNTx = selected->_sxBot;
				int32 RNTy = selected->_syBot - selected->_zTop + selected->_z;

				/*
				            Bounding Box layout (top part)

				       1
				     /   \
				   /       \     1 = Left  Far  Top LFT --+
				 2  Z-face   3   2 = Left  Near Top LNT -++
				 | \       / |   3 = Right Far  Top RFT +-+
				 |   \   /   |   4 = Right Near Top RNT +++
				 | Y   4  X  |
				 |face |face |

				*/

				if (2 * (y - RNTy) <= (x - RNTx) && // if above/on line 4-3
				        2 * (y - RNTy) < (RNTx - x)) // and above/on line 4-2
					*face = Z_FACE;
				else if (x > RNTx)
					*face = X_FACE;
				else
					*face = Y_FACE;
			}
		}

		return selected->_itemNum;
	}

	return 0;
}

void ItemSorter::IncSortLimit(int count) {
	_sortLimit += count;
	if (_sortLimit < 0)
		_sortLimit = 0;
}

//
// int16 ItemSorter::CheckClipped(Rect &r)
//
// Desc: Check for a clipped rectangle
// Returns: -1 if off screen,
//           0 if not clipped,
//           1 if clipped
//
int16 ItemSorter::CheckClipped(const Rect &c) const {
	Rect r = c;
	r.clip(_clipWindow);

	// Clipped away to the void
	if (r.isEmpty())
		return -1;
	else if (r == c) return 0;
	else return 1;
}

} // End of namespace Ultima8
} // End of namespace Ultima
