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
#include "sci/engine/vm_types.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/lists32.h"
#include "sci/graphics/screen_item32.h"

namespace Sci {
enum PlaneType {
	kPlaneTypeColored     = 0,
	kPlaneTypePicture     = 1,
	kPlaneTypeTransparent = 2,
	kPlaneTypeOpaque      = 3
};

enum PlanePictureCodes {
	// NOTE: Any value at or below 65532 means the plane
	// is a kPlaneTypePicture.
	kPlanePic            = 65532,
	kPlanePicOpaque      = 65533,
	kPlanePicTransparent = 65534,
	kPlanePicColored     = 65535
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
	ScreenItem *screenItem;
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
	 * A serial used for planes that are generated inside
	 * the graphics engine, rather than the interpreter.
	 */
	static uint16 _nextObjectId;

	/**
	 * The dimensions of the plane, in game script
	 * coordinates.
	 * TODO: These are never used and are always
	 * scriptWidth x scriptHeight in SCI engine? The actual
	 * dimensions of the plane are always in
	 * gameRect/planeRect.
	 */
	int16 _width, _height;

	/**
	 * For planes that are used to render picture data, the
	 * resource ID of the picture to be displayed. This
	 * value may also be one of the special
	 * PlanePictureCodes, in which case the plane becomes a
	 * non-picture plane.
	 */
	GuiResourceId _pictureId;

	/**
	 * Whether or not the contents of picture planes should
	 * be drawn horizontally mirrored. Only applies to
	 * planes of type kPlaneTypePicture.
	 */
	bool _mirrored;

	/**
	 * Whether the picture ID for this plane has changed.
	 * This flag is set when the plane is created or updated
	 * from a VM object, and is cleared when the plane is
	 * synchronised to another plane (which calls
	 * changePic).
	 */
	bool _pictureChanged;

	// TODO: Are these ever actually used?
	int _field_34, _field_38; // probably a point or ratio
	int _field_3C, _field_40; // probably a point or ratio

	/**
	 * Converts the dimensions of the game rect used by
	 * scripts to the dimensions of the plane rect used to
	 * render content to the screen. Coordinates with
	 * remainders are rounded up to the next whole pixel.
	 */
	void convertGameRectToPlaneRect();

	/**
	 * Sets the type of the plane according to its assigned
	 * picture resource ID.
	 */
	void setType();

public:
	/**
	 * The color to use when erasing the plane. Only
	 * applies to planes of type kPlaneTypeColored.
	 */
	byte _back;

	/**
	 * Whether the priority of this plane has changed.
	 * This flag is set when the plane is updated from
	 * another plane and cleared when draw list calculation
	 * occurs.
	 */
	int _priorityChanged; // ?

	/**
	 * A handle to the VM object corresponding to this
	 * plane. Some planes are generated purely within the
	 * graphics engine and have a numeric object value.
	 */
	reg_t _object;

	/**
	 * The rendering priority of the plane. Higher
	 * priorities are drawn above lower priorities.
	 */
	int16 _priority;

	/**
	 * TODO: Document
	 */
	int _redrawAllCount;

	PlaneType _type;

	/**
	 * Flags indicating the state of the plane.
	 * - `created` is set when the plane is first created,
	 *   either from a VM object or from within the engine
	 *   itself
	 * - `updated` is set when the plane is updated from
	 *   another plane and the two planes' `planeRect`s do
	 *   not match
	 * - `deleted` is set when the plane is deleted by a
	 *   kernel call
	 * - `moved` is set when the plane is synchronised from
	 *   another plane and is not already in the "created"
	 *   state
	 */
	int _created, _updated, _deleted, _moved;

	/**
	 * The vanishing point for the plane. Used when
	 * calculating the correct scaling of the plane's screen
	 * items according to their position.
	 */
	Common::Point _vanishingPoint;

	/**
	 * The position & dimensions of the plane in screen
	 * coordinates. This rect is not clipped to the screen,
	 * so may include coordinates that are offscreen.
	 */
	Common::Rect _planeRect;

	/**
	 * The position & dimensions of the plane in game script
	 * coordinates.
	 */
	Common::Rect _gameRect;

	/**
	 * The position & dimensions of the plane in screen
	 * coordinates. This rect is clipped to the screen.
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

	// NOTE: This constructor signature originally did not accept a
	// picture ID, but some calls to construct planes with this signature
	// immediately set the picture ID and then called setType again, so
	// it made more sense to just make the picture ID a parameter instead.
	Plane(const Common::Rect &gameRect, PlanePictureCodes pictureId = kPlanePicColored);

	Plane(const reg_t object);

	Plane(const Plane &other);

	void operator=(const Plane &other);

	inline bool operator<(const Plane &other) const {
		if (_priority < other._priority) {
			return true;
		}

		if (_priority == other._priority) {
			return _object < other._object;
		}

		return false;
	}

	/**
	 * Clips the screen rect of this plane to fit within the
	 * given screen rect.
	 */
	inline void clipScreenRect(const Common::Rect &screenRect) {
		if (_screenRect.intersects(screenRect)) {
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
	 * Compares the properties of the current plane against
	 * the properties of the `other` plane (which is the
	 * corresponding plane from the visible plane list) to
	 * discover which properties have been changed on this
	 * plane by a call to `update(reg_t)`.
	 *
	 * @note This method was originally called UpdatePlane
	 * in SCI engine.
	 */
	void sync(const Plane *other, const Common::Rect &screenRect);

	/**
	 * Updates the plane to match the state of the plane
	 * object from the virtual machine.
	 *
	 * @note This method was originally called UpdatePlane
	 * in SCI engine.
	 */
	void update(const reg_t object);

#pragma mark -
#pragma mark Plane - Pic
private:
	/**
	 * Adds all cels from the specified picture resource to
	 * the plane as screen items. If a position is provided,
	 * the screen items will be given that position;
	 * otherwise, the default relative positions for each
	 * cel will be taken from the picture resource data.
	 */
	inline void addPicInternal(const GuiResourceId pictureId, const Common::Point *position, const bool mirrorX);

	/**
	 * Marks all screen items to be deleted that are within
	 * this plane and match the given picture ID.
	 */
	void deletePic(const GuiResourceId pictureId);

	/**
	 * Marks all screen items to be deleted that are within
	 * this plane and match the given picture ID, then sets
	 * the picture ID of the plane to the new picture ID
	 * without adding any screen items.
	 */
	void deletePic(const GuiResourceId oldPictureId, const GuiResourceId newPictureId);

	/**
	 * Marks all screen items to be deleted that are within
	 * this plane and are picture cels.
	 */
	void deleteAllPics();

public:
	/**
	 * Marks all existing screen items matching the current
	 * picture to be deleted, then adds all cels from the
	 * new picture resource to the plane at the given
	 * position.
	 */
	void addPic(const GuiResourceId pictureId, const Common::Point &position, const bool mirrorX);

	/**
	 * If the plane is a picture plane, re-adds all cels
	 * from its picture resource to the plane. Otherwise,
	 * just clears the _pictureChanged flag.
	 */
	void changePic();

#pragma mark -
#pragma mark Plane - Rendering
private:
	/**
	 * Splits all rects in the given draw list at the edges
	 * of all non-transparent planes above the current
	 * plane.
	 */
	void breakDrawListByPlanes(DrawList &drawList, const PlaneList &planeList) const;

	/**
	 * Splits all rects in the given erase list rects at the
	 * edges of all non-transparent planes above the current
	 * plane.
	 */
	void breakEraseListByPlanes(RectList &eraseList, const PlaneList &planeList) const;

	/**
	 * Synchronises changes to screen items from the current
	 * plane to the visible plane and deletes screen items
	 * from the current plane that have been marked as
	 * deleted. If `forceUpdate` is true, all screen items
	 * on the visible plane will be updated, even if they
	 * are not marked as having changed.
	 */
	void decrementScreenItemArrayCounts(Plane *visiblePlane, const bool forceUpdate);

	/**
	 * Merges the screen item from this plane at the given
	 * index into the given draw list, clipped to the given
	 * rect. TODO: Finish documenting
	 */
	void mergeToDrawList(const DrawList::size_type index, const Common::Rect &rect, DrawList &drawList) const;

	/**
	 * Adds the given rect into the given rect list,
	 * merging it with other rects already inside the list,
	 * if possible, to avoid overdraw. TODO: Finish
	 * documenting
	 */
	void mergeToRectList(const Common::Rect &rect, RectList &rectList) const;

public:
	/**
	 * Calculates the location and dimensions of dirty rects
	 * of the screen items in this plane and adds them to
	 * the given draw and erase lists, and synchronises this
	 * plane's list of screen items to the given visible
	 * plane.
	 */
	void calcLists(Plane &visiblePlane, const PlaneList &planeList, DrawList &drawList, RectList &eraseList);

	/**
	 * TODO: Documentation
	 */
	void filterDownEraseRects(DrawList &drawList, RectList &eraseList, RectList &transparentEraseList) const;

	/**
	 * TODO: Documentation
	 */
	void filterUpEraseRects(DrawList &drawList, RectList &eraseList) const;

	/**
	 * TODO: Documentation
	 */
	void filterUpDrawRects(DrawList &transparentDrawList, const DrawList &drawList) const;

	/**
	 * Updates all of the plane's non-deleted screen items
	 * and adds them to the given draw and erase lists.
	 */
	void redrawAll(Plane *visiblePlane, const PlaneList &planeList, DrawList &drawList, RectList &eraseList);
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
	// A method for finding the index of a plane inside a
	// PlaneList is used because entries in the main plane
	// list and visible plane list of GfxFrameout are
	// synchronised by index
	int findIndexByObject(const reg_t object) const;
	Plane *findByObject(const reg_t object) const;

	/**
	 * Gets the priority of the top plane in the plane list.
	 */
	int16 getTopPlanePriority() const;

	/**
	 * Gets the priority of the top plane in the plane list
	 * created by a game script.
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
