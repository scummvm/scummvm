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

#ifndef SCI_GRAPHICS_FRAMEOUT_H
#define SCI_GRAPHICS_FRAMEOUT_H

#include "engines/util.h"                // for initGraphics
#include "sci/graphics/plane32.h"
#include "sci/graphics/screen_item32.h"

namespace Sci {
typedef Common::Array<DrawList> ScreenItemListList;
typedef Common::Array<RectList> EraseListList;

class GfxCursor32;
class GfxTransitions32;
struct PlaneShowStyle;

/**
 * Frameout class, kFrameout and relevant functions for SCI32 games.
 * Roughly equivalent to GraphicsMgr in the actual SCI engine.
 */
class GfxFrameout {
	friend class GfxTransitions32;
private:
	GfxCursor32 *_cursor;
	GfxPalette32 *_palette;
	SegManager *_segMan;

	/**
	 * Determines whether the current game should be rendered in
	 * high resolution.
	 */
	bool gameIsHiRes() const;

public:
	GfxFrameout(SegManager *segMan, GfxPalette32 *palette, GfxTransitions32 *transitions, GfxCursor32 *cursor);
	~GfxFrameout();

	bool _isHiRes;

	void clear();
	void run();

#pragma mark -
#pragma mark Screen items
private:
	void remapMarkRedraw();

public:
	/**
	 * Adds a screen item.
	 */
	void addScreenItem(ScreenItem &screenItem) const;

	/**
	 * Updates a screen item.
	 */
	void updateScreenItem(ScreenItem &screenItem) const;

	/**
	 * Deletes a screen item.
	 */
	void deleteScreenItem(ScreenItem &screenItem);

	/**
	 * Deletes a screen item from the given plane.
	 */
	void deleteScreenItem(ScreenItem &screenItem, Plane &plane);

	/**
	 * Deletes a screen item from the given plane.
	 */
	void deleteScreenItem(ScreenItem &screenItem, const reg_t plane);

	void kernelAddScreenItem(const reg_t object);
	void kernelUpdateScreenItem(const reg_t object);
	void kernelDeleteScreenItem(const reg_t object);
	bool kernelSetNowSeen(const reg_t screenItemObject) const;

#pragma mark -
#pragma mark Planes
private:
	/**
	 * The list of planes (i.e. layers) that have been added
	 * to the screen.
	 *
	 * @note This field is on `GraphicsMgr.screen` in SCI
	 * engine.
	 */
	PlaneList _planes;

	/**
	 * Updates an existing plane with properties from the
	 * given VM object.
	 */
	void updatePlane(Plane &plane);

public:
	/**
	 * Creates and adds a new plane to the plane list, or
	 * cancels deletion and updates an already-existing
	 * plane if a plane matching the given plane VM object
	 * already exists within the current plane list.
	 *
	 * @note This method is on Screen in SCI engine, but it
	 * is only ever called on `GraphicsMgr.screen`.
	 */
	void addPlane(Plane &plane);

	/**
	 * Deletes a plane within the current plane list.
	 *
	 * @note This method is on Screen in SCI engine, but it
	 * is only ever called on `GraphicsMgr.screen`.
	 */
	void deletePlane(Plane &plane);

	const PlaneList &getPlanes() const {
		return _planes;
	}
	const PlaneList &getVisiblePlanes() const {
		return _visiblePlanes;
	}
	void kernelAddPlane(const reg_t object);
	void kernelUpdatePlane(const reg_t object);
	void kernelDeletePlane(const reg_t object);
	void kernelMovePlaneItems(const reg_t object, const int16 deltaX, const int16 deltaY, const bool scrollPics);
	int16 kernelGetHighPlanePri();

#pragma mark -
#pragma mark Pics
public:
	void kernelAddPicAt(const reg_t planeObject, const GuiResourceId pictureId, const int16 pictureX, const int16 pictureY, const bool mirrorX, const bool deleteDuplicate);

#pragma mark -
#pragma mark Rendering
private:
	GfxTransitions32 *_transitions;

	/**
	 * State tracker to provide more accurate 60fps
	 * video throttling.
	 */
	uint8 _throttleState;

	/**
	 * The internal display pixel buffer. During frameOut,
	 * this buffer is drawn into according to the draw and
	 * erase rects calculated by `calcLists`, then drawn out
	 * to the hardware surface according to the `_showList`
	 * rects (which are also calculated by `calcLists`).
	 */
	Buffer _currentBuffer;

	/**
	 * When true, a change to the remap zone in the palette
	 * has occurred and screen items with remap data need to
	 * be redrawn.
	 */
	bool _remapOccurred;

	/**
	 * TODO: Document
	 * TODO: Depending upon if the engine ever modifies this
	 * rect, it may be stupid to store it separately instead
	 * of just getting width/height from GfxScreen.
	 *
	 * @note This field is on `GraphicsMgr.screen` in SCI
	 * engine.
	 */
	Common::Rect _screenRect;

	/**
	 * A list of rectangles, in display coordinates, that
	 * represent portions of the internal screen buffer that
	 * should be drawn to the hardware display surface.
	 *
	 * @note This field is on `GraphicsMgr.screen` in SCI
	 * engine.
	 */
	RectList _showList;

	/**
	 * The amount of extra overdraw that is acceptable when
	 * merging two show list rectangles together into a
	 * single larger rectangle.
	 *
	 * @note This field is on `GraphicsMgr.screen` in SCI
	 * engine.
	 */
	int _overdrawThreshold;

	/**
	 * A list of planes that are currently drawn to the
	 * hardware display surface. Used to calculate
	 * differences in plane properties between the last
	 * frame and current frame.
	 *
	 * @note This field is on `GraphicsMgr.visibleScreen` in
	 * SCI engine.
	 */
	PlaneList _visiblePlanes;

	/**
	 * Calculates the location and dimensions of dirty rects
	 * over the entire screen for rendering the next frame.
	 * The draw and erase lists in `drawLists` and
	 * `eraseLists` each represent one plane on the screen.
	 * The optional `eraseRect` argument allows a specific
	 * area of the screen to be erased.
	 */
	void calcLists(ScreenItemListList &drawLists, EraseListList &eraseLists, const Common::Rect &eraseRect = Common::Rect());

	/**
	 * Erases the areas in the given erase list from the
	 * visible screen buffer by filling them with the color
	 * from the corresponding plane. This is an optimisation
	 * for colored-type planes only; other plane types have
	 * to be redrawn from pixel data.
	 */
	void drawEraseList(const RectList &eraseList, const Plane &plane);

	/**
	 * Draws all screen items from the given draw list to
	 * the visible screen buffer.
	 */
	void drawScreenItemList(const DrawList &screenItemList);

	/**
	 * Adds a new rectangle to the list of regions to write
	 * out to the hardware. The provided rect may be merged
	 * into an existing rectangle to reduce the number of
	 * blit operations.
	 */
	void mergeToShowList(const Common::Rect &drawRect, RectList &showList, const int overdrawThreshold);

	/**
	 * Writes the internal frame buffer out to hardware and
	 * clears the show list.
	 */
	void showBits();

	/**
	 * Validates whether the given palette index in the
	 * style range should copy a color from the next
	 * palette to the source palette during a palette
	 * morph operation.
	 */
	inline bool validZeroStyle(const uint8 style, const int i) const {
		if (style != 0) {
			return false;
		}

		// TODO: Cannot check Shivers or MGDX until those executables can be
		// unwrapped
		switch (g_sci->getGameId()) {
		case GID_KQ7:
		case GID_PHANTASMAGORIA:
		case GID_SQ6:
			return (i > 71 && i < 104);
			break;
		default:
			return true;
		}
	}

public:
	void setPixelFormat(const Graphics::PixelFormat &format) const {
		initGraphics(_currentBuffer.screenWidth, _currentBuffer.screenHeight, _isHiRes, &format);
	}

	/**
	 * Whether palMorphFrameOut should be used instead of
	 * frameOut for rendering. Used by kMorphOn to
	 * explicitly enable palMorphFrameOut for one frame.
	 */
	bool _palMorphIsOn;

	inline const Buffer &getCurrentBuffer() const {
		return _currentBuffer;
	}

	void kernelFrameOut(const bool showBits);

	/**
	 * Throttles the engine as necessary to maintain
	 * 60fps output.
	 */
	void throttle();

	/**
	 * Updates the internal screen buffer for the next
	 * frame. If `shouldShowBits` is true, also sends the
	 * buffer to hardware. If `eraseRect` is non-empty,
	 * it is added to the erase list for this frame.
	 */
	void frameOut(const bool shouldShowBits, const Common::Rect &eraseRect = Common::Rect());

	/**
	 * TODO: Documentation
	 */
	void palMorphFrameOut(const int8 *styleRanges, PlaneShowStyle *showStyle);

	/**
	 * Modifies the raw pixel data for the next frame with
	 * new palette indexes based on matched style ranges.
	 */
	void alterVmap(const Palette &palette1, const Palette &palette2, const int8 style, const int8 *const styleRanges);

	// NOTE: This function is used within ScreenItem subsystem and assigned
	// to various booleanish fields that seem to represent the state of the
	// screen item (created, updated, deleted). In GK1/DOS, Phant1/m68k,
	// SQ6/DOS, SQ6/Win, and Phant2/Win, this function simply returns 1. If
	// you know of any game/environment where this function returns some
	// value other than 1, or if you used to work at Sierra and can explain
	// why this is a thing (and if anyone needs to care about it), please
	// open a ticket!!
	inline int getScreenCount() const {
		return 1;
	};

	/**
	 * Draws a portion of the current screen buffer to
	 * hardware. Used to display show styles in SCI2.1mid+.
	 */
	void showRect(const Common::Rect &rect);

	/**
	 * Shakes the screen.
	 */
	void shakeScreen(const int16 numShakes, const ShakeDirection direction);

#pragma mark -
#pragma mark Mouse cursor
private:
	/**
	 * Determines whether or not the point given by
	 * `position` is inside of the given screen item.
	 */
	bool isOnMe(const ScreenItem &screenItem, const Plane &plane, const Common::Point &position, const bool checkPixel) const;

public:
	reg_t kernelIsOnMe(const reg_t object, const Common::Point &position, const bool checkPixel) const;

#pragma mark -
#pragma mark Debugging
public:
	void printPlaneList(Console *con) const;
	void printVisiblePlaneList(Console *con) const;
	void printPlaneListInternal(Console *con, const PlaneList &planeList) const;
	void printPlaneItemList(Console *con, const reg_t planeObject) const;
	void printVisiblePlaneItemList(Console *con, const reg_t planeObject) const;
	void printPlaneItemListInternal(Console *con, const ScreenItemList &screenItemList) const;
};

} // End of namespace Sci

#endif
