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

#include "sci/console.h"
#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/lists32.h"
#include "sci/graphics/plane32.h"
#include "sci/graphics/remap32.h"
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
uint16 Plane::_nextObjectId; // Will be initialized in Plane::init()
uint32 Plane::_nextCreationId; // ditto

Plane::Plane(const Common::Rect &gameRect, PlanePictureCodes pictureId) :
_creationId(_nextCreationId++),
_pictureId(pictureId),
_mirrored(false),
_pictureChanged(false),
_type(kPlaneTypeColored),
_back(0),
_priorityChanged(false),
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
_creationId(_nextCreationId++),
_type(kPlaneTypeColored),
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

	if (g_sci->_features->usesAlternateSelectors()) {
		_gameRect.left = readSelectorValue(segMan, object, SELECTOR(left));
		_gameRect.top = readSelectorValue(segMan, object, SELECTOR(top));
		_gameRect.right = readSelectorValue(segMan, object, SELECTOR(right)) + 1;
		_gameRect.bottom = readSelectorValue(segMan, object, SELECTOR(bottom)) + 1;
	} else {
		_gameRect.left = readSelectorValue(segMan, object, SELECTOR(inLeft));
		_gameRect.top = readSelectorValue(segMan, object, SELECTOR(inTop));
		_gameRect.right = readSelectorValue(segMan, object, SELECTOR(inRight)) + 1;
		_gameRect.bottom = readSelectorValue(segMan, object, SELECTOR(inBottom)) + 1;
	}
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
_creationId(other._creationId),
_pictureId(other._pictureId),
_mirrored(other._mirrored),
_type(other._type),
_back(other._back),
_object(other._object),
_priority(other._priority),
_planeRect(other._planeRect),
_gameRect(other._gameRect),
_screenRect(other._screenRect),
_screenItemList(other._screenItemList) {}

void Plane::operator=(const Plane &other) {
	_creationId = other._creationId;
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
	_nextObjectId = g_sci->_features->detectPlaneIdBase();
	_nextCreationId = 0;
}

void Plane::convertGameRectToPlaneRect() {
	const int16 screenWidth = g_sci->_gfxFrameout->getScreenWidth();
	const int16 screenHeight = g_sci->_gfxFrameout->getScreenHeight();
	const int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

	const Ratio ratioX = Ratio(screenWidth, scriptWidth);
	const Ratio ratioY = Ratio(screenHeight, scriptHeight);

	_planeRect = _gameRect;
	mulru(_planeRect, ratioX, ratioY, 1);
}

void Plane::printDebugInfo(Console *con) const {
	const char *name;
	if (_object.isNumber()) {
		name = "-scummvm-";
	} else {
		name = g_sci->getEngineState()->_segMan->getObjectName(_object);
	}

	con->debugPrintf("%04x:%04x (%s): type %d, prio %d, ins %u, pic %d, mirror %d, back %d\n",
		PRINT_REG(_object),
		name,
		_type,
		_priority,
		_creationId,
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
	bool transparent = true;
	for (uint16 celNo = 0; celNo < celCount; ++celNo) {
		CelObjPic *celObj = new CelObjPic(pictureId, celNo);
		if (celCount == 1000) {
			celCount = celObj->_celCount;
		}
		if (!celObj->_transparent) {
			transparent = false;
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
		screenItem->_celObj.reset(celObj);

		_screenItemList.add(screenItem);
	}
	_type = (g_sci->_features->hasTransparentPicturePlanes() && transparent) ? kPlaneTypeTransparentPicture : kPlaneTypePicture;
}

GuiResourceId Plane::addPic(const GuiResourceId pictureId, const Common::Point &position, const bool mirrorX, const bool deleteDuplicate) {
	if (deleteDuplicate) {
		deletePic(pictureId);
	}
	addPicInternal(pictureId, &position, mirrorX);
	return _pictureId;
}

void Plane::changePic() {
	_pictureChanged = false;

	if (_type != kPlaneTypePicture && _type != kPlaneTypeTransparentPicture) {
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
	const int nextPlaneIndex = planeList.findIndexByObject(_object) + 1;
	const PlaneList::size_type planeCount = planeList.size();

	for (DrawList::size_type i = 0; i < drawList.size(); ++i) {
		for (PlaneList::size_type j = nextPlaneIndex; j < planeCount; ++j) {
			if (
				planeList[j]->_type != kPlaneTypeTransparent &&
				planeList[j]->_type != kPlaneTypeTransparentPicture
			) {
				Common::Rect outRects[4];
				int splitCount = splitRects(drawList[i]->rect, planeList[j]->_screenRect, outRects);
				if (splitCount != -1) {
					while (splitCount--) {
						drawList.add(drawList[i]->screenItem, outRects[splitCount]);
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
	const int nextPlaneIndex = planeList.findIndexByObject(_object) + 1;
	const PlaneList::size_type planeCount = planeList.size();

	for (RectList::size_type i = 0; i < eraseList.size(); ++i) {
		for (PlaneList::size_type j = nextPlaneIndex; j < planeCount; ++j) {
			if (
				planeList[j]->_type != kPlaneTypeTransparent &&
				planeList[j]->_type != kPlaneTypeTransparentPicture
			) {
				Common::Rect outRects[4];
				int splitCount = splitRects(*eraseList[i], planeList[j]->_screenRect, outRects);
				if (splitCount != -1) {
					while (splitCount--) {
						eraseList.add(outRects[splitCount]);
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
	const ScreenItemList::size_type screenItemCount = _screenItemList.size();
	const ScreenItemList::size_type visiblePlaneItemCount = visiblePlane._screenItemList.size();

	for (ScreenItemList::size_type i = 0; i < screenItemCount; ++i) {
		// Items can be added to ScreenItemList and we don't want to process
		// those new items, but the list also can grow smaller, so we need to
		// check that we are still within the upper bound of the list and quit
		// if we aren't any more
		if (i >= _screenItemList.size()) {
			break;
		}

		ScreenItem *item = _screenItemList[i];
		if (item == nullptr) {
			continue;
		}

		// SSCI used an array without bounds checking so could just get the
		// visible screen item directly; we need to verify that the index is
		// actually within the valid range for the visible plane before
		// accessing the item to avoid a range error.
		const ScreenItem *visibleItem = nullptr;
		if (i < visiblePlaneItemCount) {
			visibleItem = visiblePlane._screenItemList[i];
		}

		// Keep erase rects for this screen item from drawing outside of its
		// owner plane
		Common::Rect visibleItemScreenRect;
		if (visibleItem != nullptr) {
			visibleItemScreenRect = visibleItem->_screenRect;
			visibleItemScreenRect.clip(_screenRect);
		}

		if (item->_deleted) {
			// Add item's rect to erase list
			if (
				visibleItem != nullptr &&
				!visibleItemScreenRect.isEmpty()
			) {
				if (g_sci->_gfxRemap32->getRemapCount()) {
					mergeToRectList(visibleItemScreenRect, eraseList);
				} else {
					eraseList.add(visibleItemScreenRect);
				}
			}
		}

		if (!item->_created && !item->_updated) {
			continue;
		}

		item->calcRects(*this);
		const Common::Rect itemScreenRect(item->_screenRect);

		if (item->_created) {
			// Add item to draw list
			if(!itemScreenRect.isEmpty()) {
				if (g_sci->_gfxRemap32->getRemapCount()) {
					drawList.add(item, itemScreenRect);
					mergeToRectList(itemScreenRect, eraseList);
				} else {
					drawList.add(item, itemScreenRect);
				}
			}
		} else {
			// Add old rect to erase list, new item to draw list

			if (g_sci->_gfxRemap32->getRemapCount()) {
				// If item and visibleItem don't overlap...
				if (itemScreenRect.isEmpty() ||
					visibleItem == nullptr ||
					visibleItemScreenRect.isEmpty() ||
					!visibleItemScreenRect.intersects(itemScreenRect)
				) {
					// ...add item to draw list, and old rect to erase list...
					if (!itemScreenRect.isEmpty()) {
						drawList.add(item, itemScreenRect);
						mergeToRectList(itemScreenRect, eraseList);
					}
					if (visibleItem != nullptr && !visibleItemScreenRect.isEmpty()) {
						mergeToRectList(visibleItemScreenRect, eraseList);
					}
				} else {
					// ...otherwise, add bounding box of old+new to erase list,
					// and item to draw list
					Common::Rect extendedScreenRect = visibleItemScreenRect;
					extendedScreenRect.extend(itemScreenRect);

					drawList.add(item, itemScreenRect);
					mergeToRectList(extendedScreenRect, eraseList);
				}
			} else {
				// If no active remaps, just add item to draw list and old rect
				// to erase list

				// TODO: SCI3 update rects for VMD?
				if (!itemScreenRect.isEmpty()) {
					drawList.add(item, itemScreenRect);
				}
				if (visibleItem != nullptr && !visibleItemScreenRect.isEmpty()) {
					eraseList.add(visibleItemScreenRect);
				}
			}
		}
	}

	// Remove parts of eraselist/drawlist that are covered by other planes
	breakEraseListByPlanes(eraseList, planeList);
	breakDrawListByPlanes(drawList, planeList);

	// The current size of the draw list is stored here, as we need to loop over
	// only the already-inserted entries later.
	DrawList::size_type drawListSizePrimary = drawList.size();
	const RectList::size_type eraseListCount = eraseList.size();

	if (getSciVersion() == SCI_VERSION_3) {
		_screenItemList.sort();
		bool pictureDrawn = false;
		bool screenItemDrawn = false;

		for (RectList::size_type i = 0; i < eraseListCount; ++i) {
			const Common::Rect &rect = *eraseList[i];

			for (ScreenItemList::size_type j = 0; j < screenItemCount; ++j) {
				ScreenItem *item = _screenItemList[j];

				if (item == nullptr) {
					continue;
				}

				if (rect.intersects(item->_screenRect)) {
					const Common::Rect intersection = rect.findIntersectingRect(item->_screenRect);
					if (!item->_deleted) {
						if (pictureDrawn) {
							if (item->_celInfo.type == kCelTypePic) {
								if (screenItemDrawn || item->_celInfo.celNo == 0) {
									mergeToDrawList(j, intersection, drawList);
								}
							} else {
								if (!item->_updated && !item->_created) {
									mergeToDrawList(j, intersection, drawList);
								}
								screenItemDrawn = true;
							}
						} else {
							if (!item->_updated && !item->_created) {
								mergeToDrawList(j, intersection, drawList);
							}
							if (item->_celInfo.type == kCelTypePic) {
								pictureDrawn = true;
							}
						}
					}
				}
			}
		}

		_screenItemList.unsort();
	} else {
		// Add all items overlapping the erase list to the draw list
		for (RectList::size_type i = 0; i < eraseListCount; ++i) {
			const Common::Rect &rect = *eraseList[i];
			for (ScreenItemList::size_type j = 0; j < screenItemCount; ++j) {
				ScreenItem *item = _screenItemList[j];
				if (
					item != nullptr &&
					!item->_created && !item->_updated && !item->_deleted &&
					rect.intersects(item->_screenRect)
				) {
					drawList.add(item, rect.findIntersectingRect(item->_screenRect));
				}
			}
		}
	}

	if (g_sci->_gfxRemap32->getRemapCount() == 0) {
		// Add all items that overlap with items in the drawlist and have higher
		// priority.

		// We only loop over "primary" items in the draw list, skipping
		// those that were added because of the erase list in the previous loop,
		// or those to be added in this loop.
		for (DrawList::size_type i = 0; i < drawListSizePrimary; ++i) {
			const DrawItem *drawListEntry = nullptr;
			if (i < drawList.size()) {
				drawListEntry = drawList[i];
			}

			for (ScreenItemList::size_type j = 0; j < screenItemCount; ++j) {
				ScreenItem *newItem = nullptr;
				if (j < _screenItemList.size()) {
					newItem = _screenItemList[j];
				}

				if (
					drawListEntry != nullptr && newItem != nullptr &&
					!newItem->_created && !newItem->_updated && !newItem->_deleted
				) {
					const ScreenItem *drawnItem = drawListEntry->screenItem;

					if (newItem->hasPriorityAbove(*drawnItem) &&
						drawListEntry->rect.intersects(newItem->_screenRect)
					) {
						mergeToDrawList(j, drawListEntry->rect.findIntersectingRect(newItem->_screenRect), drawList);
					}
				}
			}
		}
	}

	decrementScreenItemArrayCounts(&visiblePlane, false);
}

void Plane::decrementScreenItemArrayCounts(Plane *visiblePlane, const bool forceUpdate) {
	const ScreenItemList::size_type screenItemCount = _screenItemList.size();
	for (ScreenItemList::size_type i = 0; i < screenItemCount; ++i) {
		ScreenItem *item = _screenItemList[i];

		if (item != nullptr) {
			// update item in visiblePlane if item is updated
			if (visiblePlane != nullptr && (
				item->_updated || (forceUpdate && visiblePlane->_screenItemList.findByObject(item->_object) != nullptr))) {
				*visiblePlane->_screenItemList[i] = *item;
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
					if (visiblePlane != nullptr && visiblePlane->_screenItemList.findByObject(item->_object) != nullptr) {
						visiblePlane->_screenItemList.erase_at(i);
					}
					_screenItemList.erase_at(i);
				}
			}
		}
	}

	_screenItemList.pack();
	if (visiblePlane != nullptr) {
		visiblePlane->_screenItemList.pack();
	}
}

void Plane::filterDownEraseRects(DrawList &drawList, RectList &eraseList, RectList &higherEraseList) const {
	const RectList::size_type higherEraseCount = higherEraseList.size();

	if (_type == kPlaneTypeTransparent || _type == kPlaneTypeTransparentPicture) {
		for (RectList::size_type i = 0; i < higherEraseCount; ++i) {
			const Common::Rect &r = *higherEraseList[i];
			const ScreenItemList::size_type screenItemCount = _screenItemList.size();
			for (ScreenItemList::size_type j = 0; j < screenItemCount; ++j) {
				const ScreenItem *item = _screenItemList[j];
				if (item != nullptr && r.intersects(item->_screenRect)) {
					mergeToDrawList(j, r, drawList);
				}
			}
		}
	} else {
		for (RectList::size_type i = 0; i < higherEraseCount; ++i) {
			Common::Rect r = *higherEraseList[i];
			if (r.intersects(_screenRect)) {
				r.clip(_screenRect);
				mergeToRectList(r, eraseList);

				const ScreenItemList::size_type screenItemCount = _screenItemList.size();
				for (ScreenItemList::size_type j = 0; j < screenItemCount; ++j) {
					const ScreenItem *item = _screenItemList[j];
					if (item != nullptr && r.intersects(item->_screenRect)) {
						mergeToDrawList(j, r, drawList);
					}
				}

				Common::Rect outRects[4];
				const Common::Rect &r2 = *higherEraseList[i];
				int splitCount = splitRects(r2, r, outRects);
				if (splitCount > 0) {
					while (splitCount--) {
						higherEraseList.add(outRects[splitCount]);
					}
				}
				higherEraseList.erase_at(i);
			}
		}

		higherEraseList.pack();
	}
}

void Plane::filterUpDrawRects(DrawList &drawList, const DrawList &lowerDrawList) const {
	const DrawList::size_type lowerDrawCount = lowerDrawList.size();
	for (DrawList::size_type i = 0; i < lowerDrawCount; ++i) {
		const Common::Rect &r = lowerDrawList[i]->rect;
		const ScreenItemList::size_type screenItemCount = _screenItemList.size();
		for (ScreenItemList::size_type j = 0; j < screenItemCount; ++j) {
			const ScreenItem *item = _screenItemList[j];
			if (item != nullptr && r.intersects(item->_screenRect)) {
				mergeToDrawList(j, r, drawList);
			}
		}
	}
}

void Plane::filterUpEraseRects(DrawList &drawList, const RectList &lowerEraseList) const {
	const RectList::size_type lowerEraseCount = lowerEraseList.size();
	for (RectList::size_type i = 0; i < lowerEraseCount; ++i) {
		const Common::Rect &r = *lowerEraseList[i];
		const ScreenItemList::size_type screenItemCount = _screenItemList.size();
		for (ScreenItemList::size_type j = 0; j < screenItemCount; ++j) {
			const ScreenItem *item = _screenItemList[j];
			if (item != nullptr && r.intersects(item->_screenRect)) {
				mergeToDrawList(j, r, drawList);
			}
		}
	}
}

void Plane::mergeToDrawList(const ScreenItemList::size_type index, const Common::Rect &rect, DrawList &drawList) const {
	RectList mergeList;
	ScreenItem &item = *_screenItemList[index];
	Common::Rect r = item._screenRect;
	r.clip(rect);
	mergeList.add(r);

	for (RectList::size_type i = 0; i < mergeList.size(); ++i) {
		r = *mergeList[i];

		const DrawList::size_type drawCount = drawList.size();
		for (DrawList::size_type j = 0; j < drawCount; ++j) {
			const DrawItem &drawItem = *drawList[j];
			if (item._object == drawItem.screenItem->_object) {
				if (drawItem.rect.contains(r)) {
					mergeList.erase_at(i);
					break;
				}

				Common::Rect outRects[4];
				int splitCount = splitRects(r, drawItem.rect, outRects);
				if (splitCount != -1) {
					while (splitCount--) {
						mergeList.add(outRects[splitCount]);
					}

					mergeList.erase_at(i);

					// proceed to the next rect
					r = *mergeList[++i];
				}
			}
		}
	}

	mergeList.pack();

	for (RectList::size_type i = 0; i < mergeList.size(); ++i) {
		drawList.add(&item, *mergeList[i]);
	}
}

void Plane::mergeToRectList(const Common::Rect &rect, RectList &eraseList) const {
	RectList mergeList;
	Common::Rect r;
	mergeList.add(rect);

	for (RectList::size_type i = 0; i < mergeList.size(); ++i) {
		r = *mergeList[i];

		const RectList::size_type eraseCount = eraseList.size();
		for (RectList::size_type j = 0; j < eraseCount; ++j) {
			const Common::Rect &eraseRect = *eraseList[j];
			if (eraseRect.contains(r)) {
				mergeList.erase_at(i);
				break;
			}

			Common::Rect outRects[4];
			int splitCount = splitRects(r, eraseRect, outRects);
			if (splitCount != -1) {
				while (splitCount--) {
					mergeList.add(outRects[splitCount]);
				}

				mergeList.erase_at(i);

				// proceed to the next rect
				r = *mergeList[++i];
			}
		}
	}

	mergeList.pack();

	for (RectList::size_type i = 0; i < mergeList.size(); ++i) {
		eraseList.add(*mergeList[i]);
	}
}

void Plane::redrawAll(Plane *visiblePlane, const PlaneList &planeList, DrawList &drawList, RectList &eraseList) {
	const ScreenItemList::size_type screenItemCount = _screenItemList.size();
	for (ScreenItemList::size_type i = 0; i < screenItemCount; ++i) {
		ScreenItem *screenItem = _screenItemList[i];
		if (screenItem != nullptr && !screenItem->_deleted) {
			screenItem->calcRects(*this);
			if (!screenItem->_screenRect.isEmpty()) {
				mergeToDrawList(i, screenItem->_screenRect, drawList);
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
}

void Plane::setType() {
	switch (_pictureId) {
	case kPlanePicColored:
		_type = kPlaneTypeColored;
		break;
	case kPlanePicTransparent:
		_type = kPlaneTypeTransparent;
		break;
	case kPlanePicOpaque:
		_type = kPlaneTypeOpaque;
		break;
	case kPlanePicTransparentPicture:
		if (g_sci->_features->hasTransparentPicturePlanes()) {
			_type = kPlaneTypeTransparentPicture;
			break;
		}
		// The game doesn't have transparent picture planes
		// fall through
	default:
		if (!g_sci->_features->hasTransparentPicturePlanes() || _type != kPlaneTypeTransparentPicture) {
			_type = kPlaneTypePicture;
		}
		break;
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
			// the plane moved or got larger
			_redrawAllCount = g_sci->_gfxFrameout->getScreenCount();
			_moved = g_sci->_gfxFrameout->getScreenCount();
		} else if (_planeRect != other->_planeRect) {
			// the plane got smaller
			_moved = g_sci->_gfxFrameout->getScreenCount();
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
		_updated = g_sci->_gfxFrameout->getScreenCount();
	}

	convertGameRectToPlaneRect();
	_screenRect = _planeRect;
	// screenRect was retrieved through globals in SSCI instead of being passed
	// into the function. We don't do that just to avoid the extra indirection
	clipScreenRect(screenRect);
}

void Plane::update(const reg_t object) {
	SegManager *segMan = g_sci->getEngineState()->_segMan;
	_vanishingPoint.x = readSelectorValue(segMan, object, SELECTOR(vanishingX));
	_vanishingPoint.y = readSelectorValue(segMan, object, SELECTOR(vanishingY));

	if (g_sci->_features->usesAlternateSelectors()) {
		_gameRect.left = readSelectorValue(segMan, object, SELECTOR(left));
		_gameRect.top = readSelectorValue(segMan, object, SELECTOR(top));
		_gameRect.right = readSelectorValue(segMan, object, SELECTOR(right)) + 1;
		_gameRect.bottom = readSelectorValue(segMan, object, SELECTOR(bottom)) + 1;
	} else {
		_gameRect.left = readSelectorValue(segMan, object, SELECTOR(inLeft));
		_gameRect.top = readSelectorValue(segMan, object, SELECTOR(inTop));
		_gameRect.right = readSelectorValue(segMan, object, SELECTOR(inRight)) + 1;
		_gameRect.bottom = readSelectorValue(segMan, object, SELECTOR(inBottom)) + 1;
	}
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
	ScreenItemList::size_type screenItemCount = _screenItemList.size();
	for (ScreenItemList::size_type i = 0; i < screenItemCount; ++i) {
		ScreenItem *screenItem = _screenItemList[i];
		if (
			screenItem != nullptr &&
			!screenItem->_deleted && !screenItem->_created &&
			screenItem->getCelObj()._remap
		) {
			screenItem->_updated = g_sci->_gfxFrameout->getScreenCount();
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
