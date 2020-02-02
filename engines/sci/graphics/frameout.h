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
#include "sci/event.h"
#include "sci/graphics/plane32.h"
#include "sci/graphics/screen_item32.h"

namespace Sci {
typedef Common::Array<DrawList> ScreenItemListList;
typedef Common::Array<RectList> EraseListList;

class GfxCursor32;
class GfxTransitions32;
struct PlaneShowStyle;

/**
 * Frameout class, kFrameOut and relevant functions for SCI32 games.
 * Roughly equivalent to GraphicsMgr in SSCI.
 */
class GfxFrameout {
	friend class GfxTransitions32;

public:
	GfxFrameout(SegManager *segMan, GfxPalette32 *palette, GfxTransitions32 *transitions, GfxCursor32 *cursor);
	~GfxFrameout();

	void clear();
	void run();

	/**
	 * Returns true if the game should render at a resolution greater than
	 * 320x240.
	 */
	inline bool isHiRes() const { return _isHiRes; }

	/**
	 * Gets the x-resolution used by game scripts.
	 */
	inline int16 getScriptWidth() const { return _scriptWidth; }

	/**
	 * Gets the y-resolution used by game scripts.
	 */
	inline int16 getScriptHeight() const { return _scriptHeight; }

	/**
	 * Gets the x-resolution of the output buffer.
	 */
	inline int16 getScreenWidth() const { return _currentBuffer.w; }

	/**
	 * Gets the y-resolution of the output buffer.
	 */
	inline int16 getScreenHeight() const { return _currentBuffer.h; }

private:
	GfxCursor32 *_cursor;
	GfxPalette32 *_palette;
	GfxTransitions32 *_transitions;
	SegManager *_segMan;

	/**
	 * Whether or not the game should render at a resolution above 320x240.
	 */
	bool _isHiRes;

	/**
	 * The resolution used by game scripts.
	 * @see celobj32.h comments on kLowResX/kLowResY.
	 */
	int16 _scriptWidth, _scriptHeight;

	/**
	 * Determines whether the current game should be rendered in high
	 * resolution.
	 */
	bool detectHiRes() const;

#pragma mark -
#pragma mark Screen items
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
	int16 kernelObjectIntersect(const reg_t object1, const reg_t object2) const;

private:
	void remapMarkRedraw();
	bool getNowSeenRect(const reg_t screenItemObject, Common::Rect &result) const;

#pragma mark -
#pragma mark Planes
public:
	/**
	 * Creates and adds a new plane to the plane list. Ownership of the passed
	 * object is transferred to GfxFrameout.
	 *
	 * @note This method is on Screen in SSCI, but it is only ever called on
	 * `GraphicsMgr.screen`.
	 */
	void addPlane(Plane *plane);

	/**
	 * Deletes a plane within the current plane list.
	 *
	 * @note This method is on Screen in SSCI, but it is only ever called on
	 * `GraphicsMgr.screen`.
	 */
	void deletePlane(Plane &plane);

	/**
	 * Deletes planes when restoring a Mac game.
	 */
	void deletePlanesForMacRestore();

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

private:
	/**
	 * The list of planes (i.e. layers) that will be rendered to the screen on
	 * the next call to `frameOut`.
	 *
	 * @note This field is on `GraphicsMgr.screen` in SSCI.
	 */
	PlaneList _planes;

	/**
	 * Updates an existing plane with properties from the given VM object.
	 */
	void updatePlane(Plane &plane);

#pragma mark -
#pragma mark Pics
public:
	void kernelAddPicAt(const reg_t planeObject, const GuiResourceId pictureId, const int16 pictureX, const int16 pictureY, const bool mirrorX, const bool deleteDuplicate);

#pragma mark -
#pragma mark Rendering
public:
	/**
	 * Updates the hardware screen, no more than once per tick.
	 *
	 * @param delta An additional number of ticks that should elapse
	 * since the last time the screen was updated before it gets updated now.
	 * This is used for updating the screen within run_vm, where we normally
	 * expect that a call to kFrameOut will occur later during the current
	 * frame, but if it does not, then update the screen on the second frame
	 * anyway since the game is doing something bad.
	 */
	void updateScreen(const int delta = 0);

	/**
	 * Resets the pixel format of the hardware surface to the given format.
	 */
	void setPixelFormat(const Graphics::PixelFormat &format) const {
		initGraphics(_currentBuffer.w, _currentBuffer.h, &format);
	}

	/**
	 * Whether or not to throttle kFrameOut calls.
	 */
	bool _throttleKernelFrameOut;

	/**
	 * Whether `palMorphFrameOut` should be used instead of `frameOut` for
	 * rendering. Used by `kMorphOn` to explicitly enable `palMorphFrameOut` for
	 * one frame.
	 */
	bool _palMorphIsOn;

	inline const Buffer &getCurrentBuffer() const {
		return _currentBuffer;
	}

	void kernelFrameOut(const bool showBits);

	/**
	 * Throttles the engine as necessary to maintain 60fps output.
	 */
	void throttle();

	/**
	 * Updates the internal screen buffer for the next frame. If
	 * `shouldShowBits` is true, also sends the buffer to hardware. If
	 * `eraseRect` is non-empty, it is added to the erase list for this frame.
	 */
	void frameOut(const bool shouldShowBits, const Common::Rect &eraseRect = Common::Rect());

	/**
	 * TODO: Documentation
	 */
	void palMorphFrameOut(const int8 *styleRanges, PlaneShowStyle *showStyle);

	/**
	 * Draws the given rect from the internal screen buffer to hardware without
	 * processing any other graphics updates except for cursor changes.
	 */
	void directFrameOut(const Common::Rect &showRect);

	/**
	 * Redraws the game screen from the internal frame buffer to the system.
	 * Used after pixel format changes.
	 *
	 * @param skipRect An area of the screen that does not need to be redrawn.
	 */
	void redrawGameScreen(const Common::Rect &skipRect) const;

#ifdef USE_RGB_COLOR
	/**
	 * Sends the entire internal screen buffer and palette to hardware.
	 */
	void resetHardware();
#endif

	/**
	 * Modifies the raw pixel data for the next frame with new palette indexes
	 * based on matched style ranges.
	 */
	void alterVmap(const Palette &palette1, const Palette &palette2, const int8 style, const int8 *const styleRanges);

	// This function is used within ScreenItem subsystem and assigned to various
	// booleanish fields that seem to represent the state of the screen item
	// (created, updated, deleted). In GK1/DOS, Phant1/m68k, SQ6/DOS, SQ6/Win,
	// and Phant2/Win, this function simply returns 1. If you know of any
	// game/environment where this function returns some value other than 1, or
	// if you used to work at Sierra and can explain why this is a thing (and if
	// anyone needs to care about it), please open a ticket!!
	inline int getScreenCount() const {
		return 1;
	};

	/**
	 * Shakes the screen.
	 */
	void shakeScreen(const int16 numShakes, const ShakeDirection direction);

	Plane *getTopVisiblePlane();

private:
	/**
	 * The last time the hardware screen was updated.
	 */
	uint32 _lastScreenUpdateTick;

	/**
	 * State tracker to provide more accurate 60fps video throttling.
	 */
	uint8 _throttleState;

	/**
	 * The internal display pixel buffer. During `frameOut`, this buffer is
	 * drawn into according to the draw and erase rects calculated by
	 * `calcLists`, then drawn out to the hardware surface according to the
	 * `_showList` rects (which are also calculated by `calcLists`).
	 */
	Buffer _currentBuffer;

	/**
	 * When true, a change to the remap zone in the palette has occurred and
	 * screen items with remap data need to be redrawn.
	 */
	bool _remapOccurred;

	/**
	 * A list of rectangles, in screen coordinates, that represent portions of
	 * the internal screen buffer that are dirty and should be drawn to the
	 * hardware display surface.
	 *
	 * @note This field is on `GraphicsMgr.screen` in SSCI.
	 */
	RectList _showList;

	/**
	 * The amount of extra overdraw that is acceptable when merging two show
	 * list rectangles together into a single larger rectangle.
	 *
	 * @note This field is on `GraphicsMgr.screen` in SSCI.
	 */
	int _overdrawThreshold;

	/**
	 * The list of planes that are currently drawn to the hardware display
	 * surface. Used to calculate differences in plane properties between the
	 * last frame and current frame.
	 *
	 * @note This field is on `GraphicsMgr.visibleScreen` in SSCI.
	 */
	PlaneList _visiblePlanes;

	/**
	 * Calculates the location and dimensions of dirty rects over the entire
	 * screen for rendering the next frame. The draw and erase lists in
	 * `drawLists` and `eraseLists` each represent one plane on the screen.
	 * The optional `eraseRect` argument allows a specific area of the screen to
	 * be explicitly erased.
	 */
	void calcLists(ScreenItemListList &drawLists, EraseListList &eraseLists, const Common::Rect &eraseRect = Common::Rect());

	/**
	 * Erases the areas in the given erase list from the visible screen buffer
	 * by filling them with the color from the corresponding plane. This is an
	 * optimisation for colored-type planes only; other plane types have to be
	 * redrawn from pixel data.
	 */
	void drawEraseList(const RectList &eraseList, const Plane &plane);

	/**
	 * Draws all screen items from the given draw list to the visible screen
	 * buffer.
	 */
	void drawScreenItemList(const DrawList &screenItemList);

	/**
	 * Adds a new rectangle to the list of regions to write out to the hardware.
	 * The provided rect may be merged into an existing rectangle to reduce the
	 * number of blit operations.
	 */
	void mergeToShowList(const Common::Rect &drawRect, RectList &showList, const int overdrawThreshold);

	/**
	 * Sends all dirty rects from the internal frame buffer to the backend, then
	 * updates the hardware screen.
	 */
	void showBits();

	/**
	 * Validates whether the given palette index in the style range should copy
	 * a color from the next palette to the source palette during a palette
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

#pragma mark -
#pragma mark Mouse cursor
public:
	reg_t kernelIsOnMe(const reg_t object, const Common::Point &position, const bool checkPixel) const;

private:
	void updateMousePositionForRendering() const {
		// In SSCI, mouse events were received via hardware interrupt, so the
		// mouse cursor would always get updated immediately when the user moved
		// the mouse. ScummVM must poll for mouse events from the backend
		// instead, so we poll just before rendering so that the latest mouse
		// position is rendered instead of whatever position it was at the last
		// time kGetEvent was called. Without this, the mouse appears stuck
		// during loops that do not make calls to kGetEvent, like transitions.
		g_sci->getEventManager()->getSciEvent(kSciEventPeek);
	}

	/**
	 * Determines whether or not the point given by `position` is inside of the
	 * given screen item.
	 */
	bool isOnMe(const ScreenItem &screenItem, const Plane &plane, const Common::Point &position, const bool checkPixel) const;

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
