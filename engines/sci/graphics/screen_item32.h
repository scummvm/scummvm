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

#ifndef SCI_GRAPHICS_SCREEN_ITEM32_H
#define SCI_GRAPHICS_SCREEN_ITEM32_H

#include "common/rect.h"
#include "sci/graphics/celobj32.h"
#include "sci/graphics/lists32.h"

namespace Sci {

enum ScaleSignals32 {
	kScaleSignalNone                    = 0,
	kScaleSignalDoScaling32				= 1, // enables scaling when drawing that cel (involves scaleX and scaleY)
	kScaleSignalUseVanishingPoint       = 2,
	// TODO: Is this actually a thing? I have not seen it and
	// the original engine masks &3 where it uses scale signals.
	kScaleSignalDisableGlobalScaling32	= 4
};

struct ScaleInfo {
	int x, y, max;
	ScaleSignals32 signal;
	ScaleInfo() : x(128), y(128), max(100), signal(kScaleSignalNone) {}
};

class CelObj;
class Plane;
class SegManager;

#pragma mark -
#pragma mark ScreenItem

/**
 * A ScreenItem is the engine-side representation of a
 * game script View.
 */
class ScreenItem {
private:
	/**
	 * A serial used for screen items that are generated
	 * inside the graphics engine, rather than the
	 * interpreter.
	 */
	static uint16 _nextObjectId;

	/**
	 * The parent plane of this screen item.
	 */
	reg_t _plane;

public:
	/**
	 * Scaling data used to calculate the final screen
	 * dimensions of the screen item as well as the scaling
	 * ratios used when drawing the item to screen.
	 */
	ScaleInfo _scale;

private:
	/**
	 * The position & dimensions of the screen item in
	 * screen coordinates. This rect includes the offset
	 * of the parent plane, but is not clipped to the
	 * screen, so may include coordinates that are
	 * offscreen.
	 */
	Common::Rect _screenItemRect;

	/**
	 * TODO: Document
	 */
	bool _useInsetRect;

	/**
	 * TODO: Documentation
	 * The insetRect is also used to describe the fill
	 * rectangle of a screen item that is drawn using
	 * CelObjColor.
	 */
	Common::Rect _insetRect;

	/**
	 * The z-index of the screen item in pseudo-3D space.
	 * Higher values are drawn on top of lower values.
	 */
	int _z;

	/**
	 * Sets the common properties of a screen item that must
	 * be set both during creation and update of a screen
	 * item.
	 */
	void setFromObject(SegManager *segMan, const reg_t object, const bool updateCel, const bool updateBitmap);

public:
	/**
	 * A descriptor for the cel object represented by the
	 * screen item.
	 */
	CelInfo32 _celInfo;

	/**
	 * The cel object used to actually render the screen
	 * item. This member is populated by calling
	 * `getCelObj`.
	 */
	mutable CelObj *_celObj;

	/**
	 * If set, the priority for this screen item is fixed
	 * in place. Otherwise, the priority of the screen item
	 * is calculated from its y-position + z-index.
	 */
	bool _fixedPriority;

	/**
	 * The rendering priority of the screen item, relative
	 * only to the other screen items within the same plane.
	 * Higher priorities are drawn above lower priorities.
	 */
	int16 _priority;

	/**
	 * The top-left corner of the screen item, in game
	 * script coordinates, relative to the parent plane.
	 */
	Common::Point _position;

	/**
	 * The associated View script object that was
	 * used to create the ScreenItem, or a numeric
	 * value in the case of a ScreenItem that was
	 * generated outside of the VM.
	 */
	reg_t _object;

	/**
	 * For screen items representing picture resources,
	 * the resource ID of the picture.
	 */
	GuiResourceId _pictureId;

	/**
	 * Flags indicating the state of the screen item.
	 * - `created` is set when the screen item is first
	 *   created, either from a VM object or from within the
	 *   engine itself
	 * - `updated` is set when `created` is not already set
	 *   and the screen item is updated from a VM object
	 * - `deleted` is set by the parent plane, if the parent
	 *   plane is a pic type and its picture resource ID has
	 *   changed
	 */
	int _created, _updated, _deleted; // ?

	/**
	 * For screen items that represent picture cels, this
	 * value is set to match the `_mirrorX` property of the
	 * parent plane and indicates that the cel should be
	 * drawn horizontally mirrored. For final drawing, it is
	 * XORed with the `_mirrorX` property of the cel object.
	 * The cel object's `_mirrorX` property comes from the
	 * resource data itself.
	 */
	bool _mirrorX;

	/**
	 * The scaling ratios to use when drawing this screen
	 * item. These values are calculated according to the
	 * scale info whenever the screen item is updated.
	 */
	Ratio _ratioX, _ratioY;

	/**
	 * The top-left corner of the screen item, in screen
	 * coordinates.
	 */
	Common::Point _scaledPosition;

	/**
	 * The position & dimensions of the screen item in
	 * screen coordinates. This rect includes the offset of
	 * the parent plane and is clipped to the screen.
	 */
	Common::Rect _screenRect;

	/**
	 * Initialises static Plane members.
	 */
	static void init();

	ScreenItem(const reg_t screenItem);
	ScreenItem(const reg_t plane, const CelInfo32 &celInfo);
	ScreenItem(const reg_t plane, const CelInfo32 &celInfo, const Common::Rect &rect);
	ScreenItem(const reg_t plane, const CelInfo32 &celInfo, const Common::Point &position, const ScaleInfo &scaleInfo);
	ScreenItem(const ScreenItem &other);
	~ScreenItem();
	void operator=(const ScreenItem &);

	inline bool operator<(const ScreenItem &other) const {
		if (_priority < other._priority) {
			return true;
		}

		if (_priority == other._priority) {
			if (_position.y + _z < other._position.y + other._z) {
				return true;
			}

			if (_position.y + _z == other._position.y + other._z) {
				return _object < other._object;
			}
		}

		return false;
	}

	/**
	 * Calculates the dimensions and scaling parameters for
	 * the screen item, using the given plane as the parent
	 * plane for screen rect positioning.
	 *
	 * @note This method was called Update in SCI engine.
	 */
	void calcRects(const Plane &plane);

	/**
	 * Retrieves the corresponding cel object for this
	 * screen item. If a cel object does not already exist,
	 * one will be created and assigned.
	 */
	CelObj &getCelObj() const;

	void printDebugInfo(Console *con) const;

	/**
	 * Updates the properties of the screen item from a
	 * VM object.
	 */
	void update(const reg_t object);

	/**
	 * Updates the properties of the screen item for one not belonging
	 * to a VM object. Originally GraphicsMgr::UpdateScreenItem.
	 */
	void update();

	/**
	 * Gets the "now seen" rect for the screen item, which
	 * represents the current size and position of the
	 * screen item on the screen in script coordinates.
	 */
	Common::Rect getNowSeenRect(const Plane &plane) const;
};

#pragma mark -
#pragma mark ScreenItemList

typedef StablePointerArray<ScreenItem, 250> ScreenItemListBase;
class ScreenItemList : public ScreenItemListBase {
	inline static bool sortHelper(const ScreenItem *a, const ScreenItem *b) {
		return *a < *b;
	}
public:
	ScreenItem *_unsorted[250];

	ScreenItem *findByObject(const reg_t object) const;
	void sort();
	void unsort();
};
} // End of namespace Sci

#endif
