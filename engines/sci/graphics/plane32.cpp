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

#include "sci/console.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/lists32.h"
#include "sci/graphics/plane32.h"
#include "sci/graphics/remap.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/screen_item32.h"

namespace Sci {
#pragma mark DrawList
void DrawList::add(ScreenItem *screenItem, const Common::Rect &rect) {
	DrawItem *drawItem = new DrawItem;
	drawItem->screenItem = screenItem;
	drawItem->rect = rect;
	DrawListBase::add(drawItem);
}

#pragma mark -
#pragma mark Plane
uint16 Plane::_nextObjectId = 20000;

Plane::Plane(const Common::Rect &gameRect, PlanePictureCodes pictureId) :
_pictureId(pictureId),
_mirrored(false),
_back(0),
_priorityChanged(0),
_object(make_reg(0, _nextObjectId++)),
_redrawAllCount(g_sci->_gfxFrameout->getScreenCount()),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_moved(0),
_gameRect(gameRect) {
	convertGameRectToPlaneRect();
	_priority = MAX(10000, g_sci->_gfxFrameout->getPlanes().getTopPlanePriority() + 1);
	setType();
	_screenRect = _planeRect;
}

Plane::Plane(reg_t object) :
_priorityChanged(false),
_object(object),
_redrawAllCount(g_sci->_gfxFrameout->getScreenCount()),
_created(g_sci->_gfxFrameout->getScreenCount()),
_updated(0),
_deleted(0),
_moved(0) {
	SegManager *segMan = g_sci->getEngineState()->_segMan;
	_vanishingPoint.x = readSelectorValue(segMan, object, SELECTOR(vanishingX));
	_vanishingPoint.y = readSelectorValue(segMan, object, SELECTOR(vanishingY));

	_gameRect.left = readSelectorValue(segMan, object, SELECTOR(inLeft));
	_gameRect.top = readSelectorValue(segMan, object, SELECTOR(inTop));
	_gameRect.right = readSelectorValue(segMan, object, SELECTOR(inRight)) + 1;
	_gameRect.bottom = readSelectorValue(segMan, object, SELECTOR(inBottom)) + 1;
	convertGameRectToPlaneRect();

	_back = readSelectorValue(segMan, object, SELECTOR(back));
	_priority = readSelectorValue(segMan, object, SELECTOR(priority));
	_pictureId = readSelectorValue(segMan, object, SELECTOR(picture));
	setType();

	_mirrored = readSelectorValue(segMan, object, SELECTOR(mirrored));
	_screenRect = _planeRect;
	changePic();
}

Plane::Plane(const Plane &other) :
_pictureId(other._pictureId),
_mirrored(other._mirrored),
_back(other._back),
_object(other._object),
_priority(other._priority),
_planeRect(other._planeRect),
_gameRect(other._gameRect),
_screenRect(other._screenRect),
_screenItemList(other._screenItemList) {}

void Plane::operator=(const Plane &other) {
	_gameRect = other._gameRect;
	_planeRect = other._planeRect;
	_vanishingPoint = other._vanishingPoint;
	_pictureId = other._pictureId;
	_type = other._type;
	_mirrored = other._mirrored;
	_priority = other._priority;
	_back = other._back;
	_screenRect = other._screenRect;
	_priorityChanged = other._priorityChanged;
}

void Plane::init() {
	_nextObjectId = 20000;
}

void Plane::convertGameRectToPlaneRect() {
	const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
	const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
	const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

	const Ratio ratioX = Ratio(screenWidth, scriptWidth);
	const Ratio ratioY = Ratio(screenHeight, scriptHeight);

	_planeRect = _gameRect;
	mulru(_planeRect, ratioX, ratioY, 1);
}

void Plane::printDebugInfo(Console *con) const {
	Common::String name;

	if (_object.isNumber()) {
		name = "-scummvm-";
	} else {
		name = g_sci->getEngineState()->_segMan->getObjectName(_object);
	}

	con->debugPrintf("%04x:%04x (%s): type %d, prio %d, pic %d, mirror %d, back %d\n",
		PRINT_REG(_object),
		name.c_str(),
		_type,
		_priority,
		_pictureId,
		_mirrored,
		_back
	);
	con->debugPrintf("  game rect: (%d, %d, %d, %d), plane rect: (%d, %d, %d, %d)\n  screen rect: (%d, %d, %d, %d)\n",
		PRINT_RECT(_gameRect),
		PRINT_RECT(_planeRect),
		PRINT_RECT(_screenRect)
	);
	con->debugPrintf("  # screen items: %d\n", _screenItemList.size());
}

#pragma mark -
#pragma mark Plane - Pic

void Plane::addPicInternal(const GuiResourceId pictureId, const Common::Point *position, const bool mirrorX) {

	uint16 celCount = 1000;
	for (uint16 celNo = 0; celNo < celCount; ++celNo) {
		CelObjPic *celObj = new CelObjPic(pictureId, celNo);
		if (celCount == 1000) {
			celCount = celObj->_celCount;
		}

		ScreenItem *screenItem = new ScreenItem(_object, celObj->_info);
		screenItem->_pictureId = pictureId;
		screenItem->_mirrorX = mirrorX;
		screenItem->_priority = celObj->_priority;
		screenItem->_fixedPriority = true;
		if (position != nullptr) {
			screenItem->_position = *position + celObj->_relativePosition;
		} else {
			screenItem->_position = celObj->_relativePosition;
		}
		_screenItemList.add(screenItem);

		delete screenItem->_celObj;
		screenItem->_celObj = celObj;
	}
}

void Plane::addPic(const GuiResourceId pictureId, const Common::Point &position, const bool mirrorX) {
	deletePic(pictureId);
	addPicInternal(pictureId, &position, mirrorX);
	// NOTE: In SCI engine this method returned the pictureId of the
	// plane, but this return value was never used
}

void Plane::changePic() {
	_pictureChanged = false;

	if (_type != kPlaneTypePicture) {
		return;
	}

	addPicInternal(_pictureId, nullptr, _mirrored);
}

void Plane::deletePic(const GuiResourceId pictureId) {
	for (ScreenItemList::iterator it = _screenItemList.begin(); it != _screenItemList.end(); ++it) {
		ScreenItem *screenItem = *it;
		if (screenItem->_pictureId == pictureId) {
			screenItem->_created = 0;
			screenItem->_updated = 0;
			screenItem->_deleted = g_sci->_gfxFrameout->getScreenCount();
		}
	}
}

void Plane::deletePic(const GuiResourceId oldPictureId, const GuiResourceId newPictureId) {
	deletePic(oldPictureId);
	_pictureId = newPictureId;
}

void Plane::deleteAllPics() {
	for (ScreenItemList::iterator it = _screenItemList.begin(); it != _screenItemList.end(); ++it) {
		ScreenItem *screenItem = *it;
		if (screenItem != nullptr && screenItem->_celInfo.type == kCelTypePic) {
			if (screenItem->_created == 0) {
				screenItem->_created = 0;
				screenItem->_updated = 0;
				screenItem->_deleted = g_sci->_gfxFrameout->getScreenCount();
			} else {
				_screenItemList.erase(it);
			}
		}
	}

	_screenItemList.pack();
}

#pragma mark -
#pragma mark Plane - Rendering

void Plane::breakDrawListByPlanes(DrawList &drawList, const PlaneList &planeList) const {
	int index = planeList.findIndexByObject(_object);

	for (DrawList::size_type i = 0; i < drawList.size(); ++i) {
		for (PlaneList::size_type j = index + 1; j < planeList.size(); ++j) {
			if (planeList[j]->_type != kPlaneTypeTransparent) {
				Common::Rect ptr[4];
				int count = splitRects(drawList[i]->rect, planeList[j]->_screenRect, ptr);
				if (count != -1) {
					for (int k = count - 1; k >= 0; --k) {
						drawList.add(drawList[i]->screenItem, ptr[k]);
					}

					drawList.erase_at(i);
					break;
				}
			}
		}
	}
	drawList.pack();
}

void Plane::breakEraseListByPlanes(RectList &eraseList, const PlaneList &planeList) const {
	int index = planeList.findIndexByObject(_object);

	for (RectList::size_type i = 0; i < eraseList.size(); ++i) {
		for (PlaneList::size_type j = index + 1; j < planeList.size(); ++j) {
			if (planeList[j]->_type != kPlaneTypeTransparent) {
				Common::Rect ptr[4];

				int count = splitRects(*eraseList[i], planeList[j]->_screenRect, ptr);
				if (count != -1) {
					for (int k = count - 1; k >= 0; --k) {
						eraseList.add(ptr[k]);
					}

					eraseList.erase_at(i);
					break;
				}
			}
		}
	}
	eraseList.pack();
}

void Plane::calcLists(Plane &visiblePlane, const PlaneList &planeList, DrawList &drawList, RectList &eraseList) {
	ScreenItemList::size_type planeItemCount = _screenItemList.size();
	ScreenItemList::size_type visiblePlaneItemCount = visiblePlane._screenItemList.size();

	for (ScreenItemList::size_type i = 0; i < planeItemCount; ++i) {
		ScreenItem *vitem = nullptr;
		// NOTE: The original engine used an array without bounds checking
		// so could just get the visible screen item directly; we need to
		// verify that the index is actually within the valid range for
		// the visible plane before accessing the item to avoid a range
		// error.
		if (i < visiblePlaneItemCount) {
			vitem = visiblePlane._screenItemList[i];
		}
		ScreenItem *item = _screenItemList[i];

		if (i < _screenItemList.size() && item != nullptr) {
			if (item->_deleted) {
				// add item's rect to erase list
				if (
					i < visiblePlane._screenItemList.size() &&
					vitem != nullptr &&
					!vitem->_screenRect.isEmpty()
				) {
					if (g_sci->_gfxRemap32->getRemapCount()) {
						mergeToRectList(vitem->_screenRect, eraseList);
					} else {
						eraseList.add(vitem->_screenRect);
					}
				}
			} else if (item->_created) {
				// add item to draw list
				item->calcRects(*this);

				if(!item->_screenRect.isEmpty()) {
					if (g_sci->_gfxRemap32->getRemapCount()) {
						drawList.add(item, item->_screenRect);
						mergeToRectList(item->_screenRect, eraseList);
					} else {
						drawList.add(item, item->_screenRect);
					}
				}
			} else if (item->_updated) {
				// add old rect to erase list, new item to draw list
				item->calcRects(*this);
				if (g_sci->_gfxRemap32->getRemapCount()) {
					// if item and vitem don't overlap, ...
					if (item->_screenRect.isEmpty() ||
						i >= visiblePlaneItemCount ||
						vitem == nullptr ||
						vitem->_screenRect.isEmpty() ||
						!vitem->_screenRect.intersects(item->_screenRect)
					) {
						// add item to draw list, and old rect to erase list
						if (!item->_screenRect.isEmpty()) {
							drawList.add(item, item->_screenRect);
							mergeToRectList(item->_screenRect, eraseList);
						}
						if (
							i < visiblePlaneItemCount &&
							vitem != nullptr &&
							!vitem->_screenRect.isEmpty()
						) {
							mergeToRectList(vitem->_screenRect, eraseList);
						}
					} else {
						// otherwise, add bounding box of old+new to erase list,
						// and item to draw list

						// TODO: This was changed from disasm, verify please!
						Common::Rect extendedScreenRect = vitem->_screenRect;
						extendedScreenRect.extend(item->_screenRect);

						drawList.add(item, item->_screenRect);
						mergeToRectList(extendedScreenRect, eraseList);
					}
				} else {
					// if no active remaps, just add item to draw list and old rect
					// to erase list
					if (!item->_screenRect.isEmpty()) {
						drawList.add(item, item->_screenRect);
					}
					if (
						i < visiblePlaneItemCount &&
						vitem != nullptr &&
						!vitem->_screenRect.isEmpty()
					) {
						eraseList.add(vitem->_screenRect);
					}
				}
			}
		}
	}

	// Remove parts of eraselist/drawlist that are covered by other planes
	breakEraseListByPlanes(eraseList, planeList);
	breakDrawListByPlanes(drawList, planeList);

	// We store the current size of the drawlist, as we want to loop
	// over the currently inserted entries later.
	DrawList::size_type drawListSizePrimary = drawList.size();

	if (/* TODO: dword_C6288 */ false) {  // "high resolution pictures"????
		_screenItemList.sort();
		bool encounteredPic = false;
		bool v81 = false;

		for (RectList::size_type i = 0; i < eraseList.size(); ++i) {
			const Common::Rect *rect = eraseList[i];

			for (ScreenItemList::size_type j = 0; j < _screenItemList.size(); ++j) {
				ScreenItem *item = _screenItemList[j];

				if (j < _screenItemList.size() && item != nullptr) {
					if (rect->intersects(item->_screenRect)) {
						const Common::Rect intersection = rect->findIntersectingRect(item->_screenRect);
						if (!item->_deleted) {
							if (encounteredPic) {
								if (item->_celInfo.type == kCelTypePic) {
									if (v81 || item->_celInfo.celNo == 0) {
										drawList.add(item, intersection);
									}
								} else {
									if (!item->_updated && !item->_created) {
										drawList.add(item, intersection);
									}
									v81 = true;
								}
							} else {
								if (!item->_updated && !item->_created) {
									drawList.add(item, intersection);
								}
								if (item->_celInfo.type == kCelTypePic) {
									encounteredPic = true;
								}
							}
						}
					}
				}
			}
		}

		_screenItemList.unsort();
	} else {
		// add all items overlapping the erase list to the draw list
		for (RectList::size_type i = 0; i < eraseList.size(); ++i) {
			for (ScreenItemList::size_type j = 0; j < _screenItemList.size(); ++j) {
				ScreenItem *item = _screenItemList[j];
				if (
					item != nullptr &&
					!item->_created && !item->_updated && !item->_deleted &&
					eraseList[i]->intersects(item->_screenRect)
				) {
					drawList.add(item, eraseList[i]->findIntersectingRect(item->_screenRect));
				}
			}
		}
	}

	if (g_sci->_gfxRemap32->getRemapCount() == 0) { // no remaps active?
		// Add all items that overlap with items in the drawlist and have higher
		// priority.

		// We only loop over "primary" items in the draw list, skipping
		// those that were added because of the erase list in the previous loop,
		// or those to be added in this loop.
		for (DrawList::size_type i = 0; i < drawListSizePrimary; ++i) {
			DrawItem *dli = drawList[i];

			for (ScreenItemList::size_type j = 0; j < planeItemCount; ++j) {
				ScreenItem *sli = _screenItemList[j];

				if (
					i < drawList.size() && dli != nullptr &&
					j < _screenItemList.size() && sli != nullptr &&
					!sli->_created && !sli->_updated && !sli->_deleted
				) {
					ScreenItem *item = dli->screenItem;

					if (
						(sli->_priority > item->_priority || (sli->_priority == item->_priority && sli->_object > item->_object)) &&
						dli->rect.intersects(sli->_screenRect)
					) {
						drawList.add(sli, dli->rect.findIntersectingRect(sli->_screenRect));
					}
				}
			}
		}
	}

	decrementScreenItemArrayCounts(&visiblePlane, false);
	_screenItemList.pack();
	visiblePlane._screenItemList.pack();
}

void Plane::decrementScreenItemArrayCounts(Plane *visiblePlane, const bool forceUpdate) {
	// The size of the screenItemList may change, so it is
	// critical to re-check the size on each iteration
	for (ScreenItemList::size_type i = 0; i < _screenItemList.size(); ++i) {
		ScreenItem *item = _screenItemList[i];

		if (item != nullptr) {
			// update item in visiblePlane if item is updated
			if (
				item->_updated ||
				(
					forceUpdate &&
					visiblePlane != nullptr &&
					visiblePlane->_screenItemList.findByObject(item->_object) != nullptr
				)
			) {
				*visiblePlane->_screenItemList[i] = *_screenItemList[i];
			}

			if (item->_updated) {
				item->_updated--;
			}

			// create new item in visiblePlane if item was added
			if (item->_created) {
				item->_created--;
				if (visiblePlane != nullptr) {
					visiblePlane->_screenItemList.add(new ScreenItem(*item));
				}
			}

			// delete item from both planes if it was deleted
			if (item->_deleted) {
				item->_deleted--;
				if (!item->_deleted) {
					visiblePlane->_screenItemList.erase_at(i);
					_screenItemList.erase_at(i);
				}
			}
		}
	}
}

void Plane::filterDownEraseRects(DrawList &drawList, RectList &eraseList, RectList &transparentEraseList) const {
	if (_type == kPlaneTypeTransparent) {
		for (RectList::size_type i = 0; i < transparentEraseList.size(); ++i) {
			const Common::Rect *r = transparentEraseList[i];
			for (ScreenItemList::size_type j = 0; j < _screenItemList.size(); ++j) {
				ScreenItem *item = _screenItemList[j];
				if (item != nullptr) {
					if (r->intersects(item->_screenRect)) {
						mergeToDrawList(j, *r, drawList);
					}
				}
			}
		}
	} else {
		for (RectList::size_type i = 0; i < transparentEraseList.size(); ++i) {
			Common::Rect *r = transparentEraseList[i];
			if (r->intersects(_screenRect)) {
				r->clip(_screenRect);
				mergeToRectList(*r, eraseList);

				for (ScreenItemList::size_type j = 0; j < _screenItemList.size(); ++j) {
					ScreenItem *item = _screenItemList[j];

					if (item != nullptr) {
						if (r->intersects(item->_screenRect)) {
							mergeToDrawList(j, *r, drawList);
						}
					}
				}

				Common::Rect ptr[4];
				const Common::Rect *r2 = transparentEraseList[i];
				int count = splitRects(*r2, *r, ptr);
				for (int k = count - 1; k >= 0; --k) {
					transparentEraseList.add(ptr[k]);
				}
				transparentEraseList.erase_at(i);
			}
		}

		transparentEraseList.pack();
	}
}

void Plane::filterUpDrawRects(DrawList &transparentDrawList, const DrawList &drawList) const {
	for (DrawList::size_type i = 0; i < drawList.size(); ++i) {
		const Common::Rect &r = drawList[i]->rect;

		for (ScreenItemList::size_type j = 0; j < _screenItemList.size(); ++j) {
			ScreenItem *item = _screenItemList[j];
			if (item != nullptr) {
				if (r.intersects(item->_screenRect)) {
					mergeToDrawList(j, r, transparentDrawList);
				}
			}
		}
	}
}

void Plane::filterUpEraseRects(DrawList &drawList, RectList &eraseList) const {
	for (RectList::size_type i = 0; i < eraseList.size(); ++i) {
		const Common::Rect &r = *eraseList[i];
		for (ScreenItemList::size_type j = 0; j < _screenItemList.size(); ++j) {
			ScreenItem *item = _screenItemList[j];

			if (item != nullptr) {
				if (r.intersects(item->_screenRect)) {
					mergeToDrawList(j, r, drawList);
				}
			}
		}
	}
}

void Plane::mergeToDrawList(const ScreenItemList::size_type index, const Common::Rect &rect, DrawList &drawList) const {
	RectList rects;

	ScreenItem *item = _screenItemList[index];
	Common::Rect r = item->_screenRect;
	r.clip(rect);
	rects.add(r);

	for (RectList::size_type i = 0; i < rects.size(); ++i) {
		r = *rects[i];

		for (DrawList::size_type j = 0; j < drawList.size(); ++j) {
			const DrawItem *drawitem = drawList[j];
			if (item->_object == drawitem->screenItem->_object) {
				if (drawitem->rect.contains(r)) {
					rects.erase_at(i);
					break;
				}

				Common::Rect outRects[4];
				const int count = splitRects(r, drawitem->rect, outRects);
				if (count != -1) {
					for (int k = count - 1; k >= 0; --k) {
						rects.add(outRects[k]);
					}

					rects.erase_at(i);

					// proceed to the next rect
					r = *rects[++i];
				}
			}
		}
	}

	rects.pack();

	for (RectList::size_type i = 0; i < rects.size(); ++i) {
		drawList.add(item, *rects[i]);
	}
}

void Plane::mergeToRectList(const Common::Rect &rect, RectList &rectList) const {
	RectList temp;
	temp.add(rect);

	for (RectList::size_type i = 0; i < temp.size(); ++i) {
		Common::Rect r = *temp[i];

		for (RectList::size_type j = 0; j < rectList.size(); ++j) {
			const Common::Rect *innerRect = rectList[j];
			if (innerRect->contains(r)) {
				temp.erase_at(i);
				break;
			}

			Common::Rect out[4];
			const int count = splitRects(r, *innerRect, out);
			if (count != -1) {
				for (int k = count - 1; k >= 0; --k) {
					temp.add(out[k]);
				}

				temp.erase_at(i);

				// proceed to the next rect
				r = *temp[++i];
			}
		}
	}

	temp.pack();

	for (RectList::size_type i = 0; i < temp.size(); ++i) {
		rectList.add(*temp[i]);
	}
}

void Plane::redrawAll(Plane *visiblePlane, const PlaneList &planeList, DrawList &drawList, RectList &eraseList) {
	for (ScreenItemList::const_iterator screenItemPtr = _screenItemList.begin(); screenItemPtr != _screenItemList.end(); ++screenItemPtr) {
		if (*screenItemPtr != nullptr) {
			ScreenItem &screenItem = **screenItemPtr;
			if (!screenItem._deleted) {
				screenItem.calcRects(*this);
				if (!screenItem._screenRect.isEmpty()) {
					drawList.add(&screenItem, screenItem._screenRect);
				}
			}
		}
	}

	eraseList.clear();

	if (!_screenRect.isEmpty() && _type != kPlaneTypePicture && _type != kPlaneTypeOpaque) {
		eraseList.add(_screenRect);
	}
	breakEraseListByPlanes(eraseList, planeList);
	breakDrawListByPlanes(drawList, planeList);
	--_redrawAllCount;
	decrementScreenItemArrayCounts(visiblePlane, true);
	_screenItemList.pack();
	if (visiblePlane != nullptr) {
		visiblePlane->_screenItemList.pack();
	}
}

void Plane::setType() {
	if (_pictureId == kPlanePicOpaque) {
		_type = kPlaneTypeOpaque;
	} else if (_pictureId == kPlanePicTransparent) {
		_type = kPlaneTypeTransparent;
	} else if (_pictureId == kPlanePicColored) {
		_type = kPlaneTypeColored;
	} else {
		_type = kPlaneTypePicture;
	}
}

void Plane::sync(const Plane *other, const Common::Rect &screenRect) {
	if (other == nullptr) {
		if (_pictureChanged) {
			deleteAllPics();
			setType();
			changePic();
			_redrawAllCount = g_sci->_gfxFrameout->getScreenCount();
		} else {
			setType();
		}
	} else {
		if (
			_planeRect.top != other->_planeRect.top ||
			_planeRect.left != other->_planeRect.left ||
			_planeRect.right > other->_planeRect.right ||
			_planeRect.bottom > other->_planeRect.bottom
		) {
			_redrawAllCount = g_sci->_gfxFrameout->getScreenCount();
			_updated = g_sci->_gfxFrameout->getScreenCount();
		} else if (_planeRect != other->_planeRect) {
			_updated = g_sci->_gfxFrameout->getScreenCount();
		}

		if (_priority != other->_priority) {
			_priorityChanged = g_sci->_gfxFrameout->getScreenCount();
		}

		if (_pictureId != other->_pictureId || _mirrored != other->_mirrored || _pictureChanged) {
			deleteAllPics();
			setType();
			changePic();
			_redrawAllCount = g_sci->_gfxFrameout->getScreenCount();
		}

		if (_back != other->_back) {
			_redrawAllCount = g_sci->_gfxFrameout->getScreenCount();
		}
	}

	_deleted = 0;
	if (_created == 0) {
		_moved = g_sci->_gfxFrameout->getScreenCount();
	}

	convertGameRectToPlaneRect();
	_screenRect = _planeRect;
	// NOTE: screenRect originally was retrieved through globals
	// instead of being passed into the function
	clipScreenRect(screenRect);
}

void Plane::update(const reg_t object) {
	SegManager *segMan = g_sci->getEngineState()->_segMan;
	_vanishingPoint.x = readSelectorValue(segMan, object, SELECTOR(vanishingX));
	_vanishingPoint.y = readSelectorValue(segMan, object, SELECTOR(vanishingY));
	_gameRect.left = readSelectorValue(segMan, object, SELECTOR(inLeft));
	_gameRect.top = readSelectorValue(segMan, object, SELECTOR(inTop));
	_gameRect.right = readSelectorValue(segMan, object, SELECTOR(inRight)) + 1;
	_gameRect.bottom = readSelectorValue(segMan, object, SELECTOR(inBottom)) + 1;
	convertGameRectToPlaneRect();

	_priority = readSelectorValue(segMan, object, SELECTOR(priority));
	GuiResourceId pictureId = readSelectorValue(segMan, object, SELECTOR(picture));
	if (_pictureId != pictureId) {
		_pictureId = pictureId;
		_pictureChanged = true;
	}

	_mirrored = readSelectorValue(segMan, object, SELECTOR(mirrored));
	_back = readSelectorValue(segMan, object, SELECTOR(back));
}

void Plane::scrollScreenItems(const int16 deltaX, const int16 deltaY, const bool scrollPics) {
	_redrawAllCount = g_sci->_gfxFrameout->getScreenCount();

	for (ScreenItemList::iterator it = _screenItemList.begin(); it != _screenItemList.end(); ++it) {
		if (*it != nullptr) {
			ScreenItem &screenItem = **it;
			if (!screenItem._deleted && (screenItem._celInfo.type != kCelTypePic || scrollPics)) {
				screenItem._position.x += deltaX;
				screenItem._position.y += deltaY;
			}
		}
	}
}

void Plane::remapMarkRedraw() {
	for (ScreenItemList::const_iterator screenItemPtr = _screenItemList.begin(); screenItemPtr != _screenItemList.end(); ++screenItemPtr) {
		if (*screenItemPtr != nullptr) {
			ScreenItem &screenItem = **screenItemPtr;
			if (screenItem.getCelObj()._remap && !screenItem._deleted && !screenItem._created) {
				screenItem._updated = g_sci->_gfxFrameout->getScreenCount();
			}
		}
	}
}

#pragma mark -
#pragma mark PlaneList
void PlaneList::add(Plane *plane) {
	for (iterator it = begin(); it != end(); ++it) {
		if ((*it)->_priority > plane->_priority) {
			insert(it, plane);
			return;
		}
	}

	push_back(plane);
}

void PlaneList::clear() {
	for (iterator it = begin(); it != end(); ++it) {
		delete *it;
	}

	PlaneListBase::clear();
}

void PlaneList::erase(Plane *plane) {
	for (iterator it = begin(); it != end(); ++it) {
		if (*it == plane) {
			erase(it);
			break;
		}
	}
}

PlaneList::iterator PlaneList::erase(iterator it) {
	delete *it;
	return PlaneListBase::erase(it);
}

int PlaneList::findIndexByObject(const reg_t object) const {
	for (size_type i = 0; i < size(); ++i) {
		if ((*this)[i] != nullptr && (*this)[i]->_object == object) {
			return i;
		}
	}

	return -1;
}

Plane *PlaneList::findByObject(const reg_t object) const {
	const_iterator planeIt = Common::find_if(begin(), end(), FindByObject<Plane *>(object));

	if (planeIt == end()) {
		return nullptr;
	}

	return *planeIt;
}

int16 PlaneList::getTopPlanePriority() const {
	if (size() > 0) {
		return (*this)[size() - 1]->_priority;
	}

	return 0;
}

int16 PlaneList::getTopSciPlanePriority() const {
	int16 priority = 0;

	for (const_iterator it = begin(); it != end(); ++it) {
		if ((*it)->_priority >= 10000) {
			break;
		}

		priority = (*it)->_priority;
	}

	return priority;
}

void PlaneList::remove_at(size_type index) {
	delete PlaneListBase::remove_at(index);
}

} // End of namespace Sci
