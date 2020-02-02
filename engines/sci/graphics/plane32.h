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

#ifndef SCI_GRAPHICS_PLANE32_H
#define SCI_GRAPHICS_PLANE32_H

#include "common/array.h"
#include "common/rect.h"
#include "sci/engine/features.h"
#include "sci/engine/vm_types.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/lists32.h"
#include "sci/graphics/screen_item32.h"

namespace Sci {
enum PlaneType {
	kPlaneTypeColored            = 0,
	kPlaneTypePicture            = 1,
	kPlaneTypeTransparent        = 2,
	kPlaneTypeOpaque             = 3,
	kPlaneTypeTransparentPicture = 4
};

enum PlanePictureCodes {
	// Any value at or below 65531 means the plane is a kPlaneTypePicture
	kPlanePic                   = 65531,
	kPlanePicTransparentPicture = 65532,
	kPlanePicOpaque             = 65533,
	kPlanePicTransparent        = 65534,
	kPlanePicColored            = 65535
};

#pragma mark -
#pragma mark RectList

typedef StablePointerArray<Common::Rect, 200> RectListBase;
class RectList : public RectListBase {
public:
	void add(const Common::Rect &rect) {
		RectListBase::add(new Common::Rect(rect));
	}
};

#pragma mark -
#pragma mark DrawList

struct DrawItem {
	/**
	 * The screen item to draw.
	 */
	ScreenItem *screenItem;

	/**
	 * The target rectangle of the draw operation.
	 */
	Common::Rect rect;

	inline bool operator<(const DrawItem &other) const {
		return *screenItem < *other.screenItem;
	}
};

typedef StablePointerArray<DrawItem, 250> DrawListBase;
class DrawList : public DrawListBase {
private:
	inline static bool sortHelper(const DrawItem *a, const DrawItem *b) {
		return *a < *b;
	}
public:
	void add(ScreenItem *screenItem, const Common::Rect &rect);
	inline void sort() {
		pack();
		Common::sort(begin(), end(), sortHelper);
	}
};

class PlaneList;

#pragma mark -
#pragma mark Plane

/**
 * A plane is a grouped layer of screen items.
 */
class Plane {
private:
	/**
	 * A serial used for planes that are generated inside the graphics engine,
	 * rather than the interpreter.
	 */
	static uint16 _nextObjectId;

	/**
	 * A serial used to identify the creation order of planes, to ensure a
	 * stable sort order for planes with identical priorities.
	 */
	static uint32 _nextCreationId;

	/**
	 * The creation order number, which ensures a stable sort when planes with
	 * identical priorities are added to the plane list.
	 */
	uint32 _creationId;

	/**
	 * For planes that are used to render picture data, the resource ID of the
	 * picture to be displayed. This value may also be one of the special
	 * PlanePictureCodes, in which case the plane becomes a non-picture plane.
	 */
	GuiResourceId _pictureId;

	/**
	 * Whether or not the contents of picture planes should be drawn
	 * horizontally mirrored. Only applies to planes of type kPlaneTypePicture.
	 */
	bool _mirrored;

	/**
	 * Whether the picture ID for this plane has changed. This flag is set when
	 * the plane is created or updated from a VM object, and is cleared when the
	 * plane is synchronised to another plane (which calls changePic).
	 */
	bool _pictureChanged;

	/**
	 * Converts the dimensions of the game rect used by scripts to the
	 * dimensions of the plane rect used to render content to the screen.
	 * Coordinates with remainders are rounded up to the next whole pixel.
	 */
	void convertGameRectToPlaneRect();

	/**
	 * Sets the type of the plane according to its assigned picture resource ID.
	 */
	void setType();

public:
	/**
	 * The type of the plane.
	 */
	PlaneType _type;

	/**
	 * The color to use when erasing the plane. Only applies to planes of type
	 * kPlaneTypeColored.
	 */
	byte _back;

	/**
	 * Whether the priority of this plane has changed. This flag is set when the
	 * plane is updated from another plane and cleared when draw list
	 * calculation occurs.
	 */
	int _priorityChanged;

	/**
	 * A handle to the VM object corresponding to this plane. Some planes are
	 * generated purely within the graphics engine and have a numeric object
	 * value.
	 */
	reg_t _object;

	/**
	 * The rendering priority of the plane. Higher priorities are drawn above
	 * lower priorities.
	 */
	int16 _priority;

	/**
	 * Whether or not all screen items in this plane should be redrawn on the
	 * next frameout, instead of just the screen items marked as updated. This
	 * is set when visual changes to the plane itself are made that affect the
	 * rendering of the entire plane, and cleared once those changes are
	 * rendered by `redrawAll`.
	 */
	int _redrawAllCount;

	/**
	 * Flags indicating the state of the plane.
	 * - `created` is set when the plane is first created, either from a VM
	 *   object or from within the engine itself
	 * - `updated` is set when the plane is updated from another plane and the
	 *   two planes' `planeRect`s do not match
	 * - `deleted` is set when the plane is deleted by a kernel call
	 * - `moved` is set when the plane has been moved or resized
	 */
	int _created, _updated, _deleted, _moved;

	/**
	 * The vanishing point for the plane. Used when automatically calculating
	 * the correct scaling of the plane's screen items according to their
	 * position.
	 */
	Common::Point _vanishingPoint;

	/**
	 * The position & dimensions of the plane in screen coordinates. This rect
	 * is not clipped to the screen, so may include coordinates that are
	 * offscreen.
	 */
	Common::Rect _planeRect;

	/**
	 * The position & dimensions of the plane in game script coordinates.
	 */
	Common::Rect _gameRect;

	/**
	 * The position & dimensions of the plane in screen coordinates. This rect
	 * is clipped to the screen.
	 */
	Common::Rect _screenRect;

	/**
	 * The list of screen items grouped within this plane.
	 */
	ScreenItemList _screenItemList;

public:
	/**
	 * Initialises static Plane members.
	 */
	static void init();

	// In SSCI this constructor signature did not accept a picture ID, but some
	// calls to construct planes with this signature immediately set the picture
	// ID and then called setType again, so it made more sense to just make the
	// picture ID a parameter instead.
	Plane(const Common::Rect &gameRect, PlanePictureCodes pictureId = kPlanePicColored);

	Plane(const reg_t object);

	Plane(const Plane &other);

	void operator=(const Plane &other);

	inline bool operator<(const Plane &other) const {
		if (_priority < other._priority) {
			return true;
		}

		if (_priority == other._priority) {
			// This is different than SSCI; see ScreenItem::operator< for an
			// explanation
			return _creationId < other._creationId;
		}

		return false;
	}

	/**
	 * Clips the screen rect of this plane to fit within the given screen rect.
	 */
	inline void clipScreenRect(const Common::Rect &screenRect) {
		// LSL6 hires creates planes with invalid rects; SSCI does not care
		// about this, but `Common::Rect::clip` does, so we need to check
		// whether or not the rect is actually valid before clipping and only
		// clip valid rects
		if (_screenRect.isValidRect() && _screenRect.intersects(screenRect)) {
			_screenRect.clip(screenRect);
		} else {
			_screenRect.left = 0;
			_screenRect.top = 0;
			_screenRect.right = 0;
			_screenRect.bottom = 0;
		}
	}

	void printDebugInfo(Console *con) const;

	/**
	 * Compares the properties of the current plane against the properties of
	 * the `other` plane (which is the corresponding plane from the visible
	 * plane list) to discover which properties have been changed on this plane
	 * by a call to `update(reg_t)`.
	 *
	 * @note This method was called UpdatePlane in SSCI.
	 */
	void sync(const Plane *other, const Common::Rect &screenRect);

	/**
	 * Updates the plane to match the state of the plane object from the VM.
	 *
	 * @note This method was called UpdatePlane in SSCI.
	 */
	void update(const reg_t object);

	/**
	 * Modifies the position of all non-pic screen items by the given delta. If
	 * `scrollPics` is true, pic items are also repositioned.
	 */
	void scrollScreenItems(const int16 deltaX, const int16 deltaY, const bool scrollPics);

	/**
	 * The default plane is the first plane generated inside the graphics engine.
	 */
	inline bool isDefaultPlane() {
		return _object.isNumber() && _object.getOffset() == (uint32)g_sci->_features->detectPlaneIdBase();
	}

#pragma mark -
#pragma mark Plane - Pic
private:
	/**
	 * Adds all cels from the specified picture resource to the plane as screen
	 * items. If a position is provided, the screen items will be given that
	 * position; otherwise, the default relative positions for each cel will be
	 * taken from the picture resource data.
	 */
	inline void addPicInternal(const GuiResourceId pictureId, const Common::Point *position, const bool mirrorX);

	/**
	 * Marks all screen items to be deleted that are within this plane and match
	 * the given picture ID.
	 */
	void deletePic(const GuiResourceId pictureId);

	/**
	 * Marks all screen items to be deleted that are within this plane and are
	 * picture cels.
	 */
	void deleteAllPics();

public:
	/**
	 * Marks all existing screen items matching the current picture to be
	 * deleted, then adds all cels from the new picture resource to the plane at
	 * the given position.
	 */
	GuiResourceId addPic(const GuiResourceId pictureId, const Common::Point &position, const bool mirrorX, const bool deleteDuplicate = true);

	/**
	 * If the plane is a picture plane, re-adds all cels from its picture
	 * resource to the plane. Otherwise, just clears the _pictureChanged flag.
	 */
	void changePic();

	/**
	 * Marks all screen items to be deleted that are within this plane and match
	 * the given picture ID, then sets the picture ID of the plane to the new
	 * picture ID without adding any screen items.
	 */
	void deletePic(const GuiResourceId oldPictureId, const GuiResourceId newPictureId);

#pragma mark -
#pragma mark Plane - Rendering
private:
	/**
	 * Splits all rects in the given draw list at the edges of all
	 * higher-priority, non-transparent, intersecting planes.
	 */
	void breakDrawListByPlanes(DrawList &drawList, const PlaneList &planeList) const;

	/**
	 * Splits all rects in the given erase list at the edges of higher-priority,
	 * non-transparent, intersecting planes.
	 */
	void breakEraseListByPlanes(RectList &eraseList, const PlaneList &planeList) const;

	/**
	 * Adds the screen item at `index` into `drawList`, ensuring it is only
	 * drawn within the bounds of `rect`. If an existing draw list entry exists
	 * for this screen item, it will be modified. Otherwise, a new entry will be
	 * added.
	 */
	void mergeToDrawList(const DrawList::size_type index, const Common::Rect &rect, DrawList &drawList) const;

	/**
	 * Merges `rect` with an existing rect in `eraseList`, if possible.
	 * Otherwise, adds the rect as a new entry to `eraseList`.
	 */
	void mergeToRectList(const Common::Rect &rect, RectList &eraseList) const;

public:
	/**
	 * Calculates the location and dimensions of dirty rects of the screen items
	 * in this plane and adds them to the given draw and erase lists, and
	 * synchronises this plane's list of screen items to the given visible
	 * plane.
	 */
	void calcLists(Plane &visiblePlane, const PlaneList &planeList, DrawList &drawList, RectList &eraseList);

	/**
	 * Synchronises changes to screen items from the current plane to the
	 * visible plane and deletes screen items from the current plane that have
	 * been marked as deleted. If `forceUpdate` is true, all screen items on the
	 * visible plane will be updated, even if they are not marked as having
	 * changed.
	 */
	void decrementScreenItemArrayCounts(Plane *visiblePlane, const bool forceUpdate);

	/**
	 * This method is called from the highest priority plane to the lowest
	 * priority plane.
	 *
	 * Adds screen items from this plane to the draw list that must be redrawn
	 * because they intersect entries in the `higherEraseList`.
	 *
	 * If this plane is opaque, all intersecting erase rects in `lowerEraseList`
	 * are removed, as they would be completely overwritten by the contents of
	 * this plane.
	 *
	 * If this plane is transparent, erase rects from the `lowerEraseList` are
	 * added to the erase list for this plane, so that lower planes.
	 *
	 * @param drawList The draw list for this plane.
	 * @param eraseList The erase list for this plane.
	 * @param higherEraseList The erase list for a plane above this plane.
	 */
	void filterDownEraseRects(DrawList &drawList, RectList &eraseList, RectList &higherEraseList) const;

	/**
	 * This method is called from the lowest priority plane to the highest
	 * priority plane.
	 *
	 * Adds screen items from this plane to the draw list that must be drawn
	 * because the lower plane is being redrawn and potentially transparent
	 * screen items from this plane would draw over the lower priority plane's
	 * screen items.
	 *
	 * This method applies only to transparent planes.
	 *
	 * @param drawList The draw list for this plane.
	 * @param eraseList The erase list for a plane below this plane.
	 */
	void filterUpEraseRects(DrawList &drawList, const RectList &lowerEraseList) const;

	/**
	 * This method is called from the lowest priority plane to the highest
	 * priority plane.
	 *
	 * Adds screen items from this plane to the draw list that must be drawn
	 * because the lower plane is being redrawn and potentially transparent
	 * screen items from this plane would draw over the lower priority plane's
	 * screen items.
	 *
	 * This method applies only to transparent planes.
	 *
	 * @param drawList The draw list for this plane.
	 * @param lowerDrawList The draw list for a plane below this plane.
	 */
	void filterUpDrawRects(DrawList &drawList, const DrawList &lowerDrawList) const;

	/**
	 * Updates all of the plane's non-deleted screen items and adds them to the
	 * given draw and erase lists.
	 */
	void redrawAll(Plane *visiblePlane, const PlaneList &planeList, DrawList &drawList, RectList &eraseList);

	/**
	 * Marks all non-deleted remapped screen items within the plane as needing
	 * to be updated during the next frameout.
	 */
	void remapMarkRedraw();
};

#pragma mark -
#pragma mark PlaneList

typedef Common::Array<Plane *> PlaneListBase;
class PlaneList : public PlaneListBase {
private:
	inline static bool sortHelper(const Plane *a, const Plane *b) {
		return *a < *b;
	}

	using PlaneListBase::push_back;

public:
	// A method for finding the index of a plane inside a PlaneList is used
	// because entries in the main plane list and visible plane list of
	// GfxFrameout are synchronised by index
	int findIndexByObject(const reg_t object) const;
	Plane *findByObject(const reg_t object) const;

	/**
	 * Gets the priority of the top plane in the plane list.
	 */
	int16 getTopPlanePriority() const;

	/**
	 * Gets the priority of the top plane in the plane list created by a game
	 * script.
	 */
	int16 getTopSciPlanePriority() const;

	void add(Plane *plane);
	void clear();
	iterator erase(iterator it);
	void erase(Plane *plane);
	inline void sort() {
		Common::sort(begin(), end(), sortHelper);
	}
	void remove_at(size_type index);
};

} // End of namespace Sci

#endif
