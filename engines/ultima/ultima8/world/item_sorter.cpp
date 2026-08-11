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
#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/world/item_sorter.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/texture.h"
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

static const uint32 TRANSPARENT_COLOR = TEX32_PACK_RGBA(0x7F, 0x00, 0x00, 0x7F);
static const uint32 HIGHLIGHT_COLOR = TEX32_PACK_RGBA(0xFF, 0xFF, 0x00, 0x1F);

ItemSorter::ItemSorter(int capacity) :
	_shapes(nullptr), _clipWindow(0, 0, 0, 0), _items(nullptr), _itemsTail(nullptr),
	_itemsUnused(nullptr), _painted(nullptr), _camSx(0), _camSy(0),
	_sortLimit(0), _sortLimitChanged(false) {
	int i = capacity;
	while (i--) {
		SortItem *next = _itemsUnused;
		_itemsUnused = new SortItem();
		_itemsUnused->_next = next;
	}
}

ItemSorter::~ItemSorter() {
	if (_itemsTail) {
		_itemsTail->_next = _itemsUnused;
		_itemsUnused = _items;
	}
	_items = nullptr;
	_itemsTail = nullptr;

	while (_itemsUnused) {
		SortItem *next = _itemsUnused->_next;
		delete _itemsUnused;
		_itemsUnused = next;
	}
}

void ItemSorter::BeginDisplayList(const Common::Rect32 &clipWindow, const Point3 &cam) {
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
	int32 camSx = (cam.x - cam.y) / 4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	int32 camSy = (cam.x + cam.y) / 8 - cam.z;

	if (camSx != _camSx || camSy != _camSy) {
		_camSx = camSx;
		_camSy = camSy;

		// Reset sort limit debugging on camera move
		_sortLimit = 0;
	}
}

void ItemSorter::AddItem(const Point3 &pt, uint32 shapeNum, uint32 frame_num, uint32 flags, uint32 ext_flags, uint16 itemNum) {

	// First thing, get a SortItem to use (first of unused)
	if (!_itemsUnused)
		_itemsUnused = new SortItem();
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
	Box box(pt.x, pt.y, pt.z, xd, yd, zd);
	si->setBoxBounds(box, _camSx, _camSy);

	// Real Screenspace from shape frame
	if (si->_flags & Item::FLG_FLIPPED) {
		si->_sr.left = si->_sxBot + frame->_xoff - frame->_width;
		si->_sr.top = si->_syBot - frame->_yoff;
		si->_sr.right = si->_sr.left + frame->_width;
		si->_sr.bottom = si->_sr.top + frame->_height;
	} else {
		si->_sr.left = si->_sxBot - frame->_xoff;
		si->_sr.top = si->_syBot - frame->_yoff;
		si->_sr.right = si->_sr.left + frame->_width;
		si->_sr.bottom = si->_sr.top + frame->_height;
	}

	// Do Clipping here
	if (!_clipWindow.intersects(si->_sr)) {
		// Clipped away entirely - don't add to the list.
		return;
	}

#ifdef SORTITEM_OCCLUSION_EXPERIMENTAL
	si->_xAdjoin = nullptr;
	si->_yAdjoin = nullptr;
	si->_groupNum = 0;
#endif // SORTITEM_OCCLUSION_EXPERIMENTAL

	si->_draw = info->is_draw();
	si->_solid = info->is_solid();
	si->_occl = info->is_occl() && !info->is_translucent() &&
				!(si->_flags & Item::FLG_INVISIBLE) &&
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
		if (!addpoint && si->listLessThan(*si2))
			addpoint = si2;

		if (si2->_occluded)
			continue;

#ifdef SORTITEM_OCCLUSION_EXPERIMENTAL
		// Find adjoining rects for better occlusion
		if (si->_occl && si2->_occl && si->_z == si2->_z) {
			// Does this share an edge?
			if (si->_y == si2->_y && si->_yFar == si2->_yFar) {
				if (si->_xLeft == si2->_x) {
					si->_xAdjoin = si2;
				} else if (si->_x == si2->_xLeft) {
					si2->_xAdjoin = si;
				}
			}
			else if (si->_x == si2->_x && si->_xLeft == si2->_xLeft) {
				if (si->_yFar == si2->_y) {
					si->_yAdjoin = si2;
				} else if (si->_y == si2->_yFar) {
					si2->_yAdjoin = si;
				}
			}
		}
#endif // SORTITEM_OCCLUSION_EXPERIMENTAL

		// Attempt to find paint dependency order
		if (si->overlap(*si2)) {
			if (si->below(*si2)) {
				if (si2->_occl && si2->occludes(*si)) {
					// No need to do any more checks, this isn't visible
					si->_occluded = true;
					break;
				} else {
					// si1 is behind si2, so add it to si2's dependency list
					si2->_depends.insert_sorted(si);
				}
			} else {
				if (si->_occl && si->occludes(*si2)) {
					// Occluded, but we can't remove it from the list
					si2->_occluded = true;
				} else {
					// si2 is behind si1, so add it to si1's dependency list
					si->_depends.insert_sorted(si2);
				}
			}
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
	AddItem(add->getLerped(), add->getShape(), add->getFrame(),
			add->getFlags(), add->getExtFlags(), add->getObjId());
}

void ItemSorter::PaintDisplayList(RenderSurface *surf, bool item_highlight, bool showFootpads, int gridlines) {
	if (_sortLimit) {
		// Clear the surface when debugging the sorter
		uint32 color = TEX32_PACK_RGB(0, 0, 0);
		surf->fill32(color, _clipWindow);
	}

#ifdef SORTITEM_OCCLUSION_EXPERIMENTAL
	int32 minZ = _items ? _items->_z : 0;

	// Reverse iterate to check higher z items first.
	// This increases odds of occluding items below before checking them.
	// Ignore items already occluded or at lowest Z as they are less likely occlude additional items.
	for (SortItem *si1 = _itemsTail; si1 != nullptr; si1 = si1->_prev) {
		// Check if item is part of a 2x2 rects square
		if (si1->_occl && !si1->_occluded && si1->_z > minZ &&
			si1->_xAdjoin && si1->_yAdjoin &&
			si1->_xAdjoin->_yAdjoin && si1->_yAdjoin->_xAdjoin &&
			si1->_xAdjoin->_yAdjoin == si1->_yAdjoin->_xAdjoin) {
			SortItem *si2 = si1;
			SortItem *siX = si1;
			SortItem *siY = si1;

			uint16 group = si1->_itemNum;
			si1->_groupNum = group;

			int32 zTop = si1->_zTop;

			// Expand NxN rects square - up to 4x4 appears sufficient
			for (int n = 2; n <= 4; n++) {
				// Expand out 1 from X and Y edge points
				SortItem *p1 = siX->_xAdjoin;
				SortItem *p2 = siY->_yAdjoin;

				if (!p1 || !p2)
					break;

				// Converge to meet
				while (p1 != p2 && p1->_yAdjoin && p2->_xAdjoin) {
					p1->_groupNum = group;
					p2->_groupNum = group;

					zTop = MIN(zTop, p1->_zTop);
					zTop = MIN(zTop, p2->_zTop);

					p1 = p1->_yAdjoin;
					p2 = p2->_xAdjoin;
				}

				if (p1 != p2)
					break;

				// Set the new end point
				si2 = p1;
				si2->_groupNum = group;

				zTop = MIN(zTop, p2->_zTop);

				// Set the new edge points
				siX = siX->_xAdjoin;
				siY = siY->_yAdjoin;
			}

			if (si1 != si2) {
				SortItem oc;
				oc._occl = true;
				oc._flat = si1->_flat;
				oc._solid = si1->_solid;
				oc._roof = si1->_roof;
				oc._fixed = si1->_fixed;
				oc._land = si1->_land;

				Box box = si1->getBoxBounds();
				box.extend(si2->getBoxBounds());

				// Use min z top to avoid wrong occlusions caused by different heights
				box._zd = zTop - box._z;

				oc.setBoxBounds(box, _camSx, _camSy);

				for (si2 = _items; si2 != nullptr; si2 = si2->_next) {
					if (si2->_groupNum != group && !si2->_occluded &&
						si2->overlap(oc) && si2->below(oc) && oc.occludes(*si2)) {
						si2->_occluded = true;
					}
				}
			}
		}
	}
#endif

	SortItem *it = _items;
	SortItem *end = nullptr;
	_painted = nullptr;  // Reset the paint tracking
	while (it != end) {
		if (it->_order == -1)
			if (PaintSortItem(surf, it, showFootpads, gridlines))
				return;
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
				                          (it->_flags & Item::FLG_FLIPPED) != 0,
										  HIGHLIGHT_COLOR);
			}

			it = it->_next;
		}

	}
}

/**
 * Recursively paint this item and all its dependencies.
 * Returns true if recursion should stop.
 */
bool ItemSorter::PaintSortItem(RenderSurface *surf, SortItem *si, bool showFootpad, int gridlines) {
	// Don't paint this, or dependencies (yet) if occluded
	if (si->_occluded)
		return false;

	// Resursion detection
	si->_order = -2;

	// Iterate through our dependancies, and paint them, if possible
	for (auto *d : si->_depends) {
		if (d->_order == -2) {
			if (!_sortLimit) {
				debugC(kDebugObject, "Cycle in paint dependency graph %d -> %d -> ... -> %d",
					   si->_shapeNum, d->_shapeNum, si->_shapeNum);
			}
			break;
		}
		else if (d->_order == -1) {
			if (PaintSortItem(surf, d, showFootpad, gridlines))
				return true;
		}
	}

	// Now paint us!
	if (surf) {
		if (si->_extFlags & Item::EXT_HIGHLIGHT && si->_extFlags & Item::EXT_TRANSPARENT)
			surf->PaintHighlightInvis(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans, (si->_flags & Item::FLG_FLIPPED) != 0, TRANSPARENT_COLOR);
		if (si->_extFlags & Item::EXT_HIGHLIGHT)
			surf->PaintHighlight(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans, (si->_flags & Item::FLG_FLIPPED) != 0, TRANSPARENT_COLOR);
		else if (si->_extFlags & Item::EXT_TRANSPARENT)
			surf->PaintInvisible(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans, (si->_flags & Item::FLG_FLIPPED) != 0);
		else if (si->_trans)
			surf->PaintTranslucent(si->_shape, si->_frame, si->_sxBot, si->_syBot, (si->_flags & Item::FLG_FLIPPED) != 0);
		else
			surf->Paint(si->_shape, si->_frame, si->_sxBot, si->_syBot, (si->_flags & Item::FLG_FLIPPED) != 0);

		// Draw wire frame footpads
		if (showFootpad) {
			uint32 color = TEX32_PACK_RGB(0xFF, 0xFF, 0xFF);

			// NOTE: Precision loss from integer division is intention
			int32 syLeftTop = si->_xLeft / 8 + si->_y / 8 - si->_zTop - _camSy;
			int32 syRightTop = si->_x / 8 + si->_yFar / 8 - si->_zTop - _camSy;
			int32 syNearTop = si->_x / 8 + si->_y / 8 - si->_zTop - _camSy;

			surf->drawLine32(color, si->_sxTop, si->_syTop, si->_sxLeft, syLeftTop);
			surf->drawLine32(color, si->_sxTop, si->_syTop, si->_sxRight, syRightTop);
			surf->drawLine32(color, si->_sxBot, syNearTop, si->_sxLeft, syLeftTop);
			surf->drawLine32(color, si->_sxBot, syNearTop, si->_sxRight, syRightTop);

			if (si->_z < si->_zTop) {
				int32 syLeftBot = si->_xLeft / 8 + si->_y / 8 - si->_z - _camSy;
				int32 syRightBot = si->_x / 8 + si->_yFar / 8 - si->_z - _camSy;
				surf->drawLine32(color, si->_sxLeft, syLeftTop, si->_sxLeft, syLeftBot);
				surf->drawLine32(color, si->_sxRight, syRightTop, si->_sxRight, syRightBot);
				surf->drawLine32(color, si->_sxBot, syNearTop, si->_sxBot, si->_syBot);
				surf->drawLine32(color, si->_sxLeft, syLeftBot, si->_sxBot, si->_syBot);
				surf->drawLine32(color, si->_sxRight, syRightBot, si->_sxBot, si->_syBot);
			}
		}

		// Draw gridlines
		if (gridlines > 0 && (si->_land || si->_roof || si->_flat)) {
			uint32 color = TEX32_PACK_RGB(0x00, 0xFF, 0xFF);

			int32 gridx = (si->_xLeft / gridlines + 1) * gridlines;
			while (gridx <= si->_x) {
				int32 sx1 = gridx / 4 - si->_y / 4 - _camSx;
				int32 sy1 = gridx / 8 + si->_y / 8 - si->_zTop - _camSy;
				int32 sx2 = gridx / 4 - si->_yFar / 4 - _camSx;
				int32 sy2 = gridx / 8 + si->_yFar / 8 - si->_zTop - _camSy;
				surf->drawLine32(color, sx1, sy1, sx2, sy2);
				if (si->_z < si->_zTop) {
					int32 sx3 = gridx / 4 - si->_y / 4 - _camSx;
					int32 sy3 = gridx / 8 + si->_y / 8 - si->_z - _camSy;
					surf->drawLine32(color, sx1, sy1, sx3, sy3);
				}

				gridx += gridlines;
			}

			int32 gridy = (si->_yFar / gridlines + 1) * gridlines;
			while (gridy <= si->_y) {
				int32 sx1 = si->_xLeft / 4 - gridy / 4 - _camSx;
				int32 sy1 = si->_xLeft / 8 + gridy / 8 - si->_zTop - _camSy;
				int32 sx2 = si->_x / 4 - gridy / 4 - _camSx;
				int32 sy2 = si->_x / 8 + gridy / 8 - si->_zTop - _camSy;
				surf->drawLine32(color, sx1, sy1, sx2, sy2);
				if (si->_z < si->_zTop) {
				int32 sx3 = si->_x / 4 - gridy / 4 - _camSx;
				int32 sy3 = si->_x / 8 + gridy / 8 - si->_z - _camSy;
					surf->drawLine32(color, sx2, sy2, sx3, sy3);
				}

				gridy += gridlines;
			}
		}

		// weapon overlay
		// FIXME: use highlight/invisibility, also add to Trace() ?
		if (si->_shapeNum == 1 && si->_itemNum == kMainActorId) {
			MainActor *av = getMainActor();
			const WeaponOverlayFrame *wo_frame = nullptr;
			uint32 wo_shapenum;
			av->getWeaponOverlay(wo_frame, wo_shapenum);
			if (wo_frame) {
				const Shape *wo_shape = GameData::get_instance()->getMainShapes()->getShape(wo_shapenum);
				surf->Paint(wo_shape, wo_frame->_frame,
							si->_sxBot + wo_frame->_xOff,
							si->_syBot + wo_frame->_yOff, false);
			}
		}
	}

	// Set our painting _order based on previously painted item
	si->_order = _painted ? _painted->_order + 1 : 0;

	if (_sortLimit && si->_order == _sortLimit) {
		if (_sortLimitChanged) {
			_sortLimitChanged = false;

			debugC(kDebugObject, "SortItem: %s", si->dumpInfo().c_str());
			if (_painted && si->overlap(*_painted)) {
				debugC(kDebugObject, "Overlaps: %s", _painted->dumpInfo().c_str());
				if (si->below(*_painted)) {
					debugC(kDebugObject, "Paint order incorrect!");
				}
			}
		}

		_painted = si;
		return true;
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
			if (it->_order == -1)
				if (PaintSortItem(nullptr, it, false, 0))
					break;

			it = it->_next;
		}
	}

	// Firstly, we check for highlighted _items
	selected = nullptr;

	if (item_highlight) {
		selected = nullptr;

		for (it = _itemsTail; it != nullptr; it = it->_prev) {
			if (!(it->_flags & (Item::FLG_DISPOSABLE | Item::FLG_FAST_ONLY)) && !it->_fixed) {
				if (!it->_itemNum || !it->contains(x, y))
					continue;

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
			if (!it->_itemNum || !it->contains(x, y))
				continue;

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
	_sortLimitChanged = true;
	if (_sortLimit < 0)
		_sortLimit = 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
