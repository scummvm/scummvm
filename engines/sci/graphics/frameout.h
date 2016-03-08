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

#include "sci/graphics/plane32.h"
#include "sci/graphics/screen_item32.h"

namespace Sci {
// TODO: Don't do this this way
int splitRects(Common::Rect r, const Common::Rect &other, Common::Rect(&outRects)[4]);

// TODO: Verify display styles and adjust names appropriately for
// types 1 through 12 & 15 (others are correct)
// Names should be:
// * VShutterIn, VShutterOut
// * HShutterIn, HShutterOut
// * WipeLeft, WipeRight, WipeDown, WipeUp
// * PixelDissolve
// * ShutDown and Kill? (and Plain and Fade?)
enum ShowStyleType /* : uint8 */ {
	kShowStyleNone = 0,
	kShowStyleHShutterOut = 1,
	kShowStyleHShutterIn = 2,
	kShowStyleVShutterOut = 3,
	kShowStyleVShutterIn = 4,
	kShowStyleWipeLeft = 5,
	kShowStyleWipeRight = 6,
	kShowStyleWipeUp = 7,
	kShowStyleWipeDown = 8,
	kShowStyleIrisOut = 9,
	kShowStyleIrisIn = 10,
	kShowStyle11 = 11,
	kShowStyle12 = 12,
	kShowStyleFadeOut = 13,
	kShowStyleFadeIn = 14,
	// TODO: Only in SCI3
	kShowStyleUnknown = 15
};

/**
 * Show styles represent transitions applied to draw planes.
 * One show style per plane can be active at a time.
 */
struct ShowStyleEntry {
	/**
	 * The ID of the plane this show style belongs to.
	 * In SCI2.1mid (at least SQ6), per-plane transitions
	 * were removed and a single plane ID is used.
	 */
	reg_t plane;

	/**
	 * The type of the transition.
	 */
	ShowStyleType type;

	// TODO: This name is probably incorrect
	bool fadeUp;

	/**
	 * The number of steps for the show style.
	 */
	int16 divisions;

	// NOTE: This property exists from SCI2 through at least
	// SCI2.1mid but is never used in the actual processing
	// of the styles?
	int unknownC;

	/**
	 * The color used by transitions that draw CelObjColor
	 * screen items. -1 for transitions that do not draw
	 * screen items.
	 */
	int16 color;

	// TODO: Probably uint32
	// TODO: This field probably should be used in order to
	// provide time-accurate processing of show styles. In the
	// actual SCI engine (at least 2–2.1mid) it appears that
	// style transitions are drawn “as fast as possible”, one
	// step per loop, even though this delay field exists
	int delay;

	// TODO: Probably bool, but never seems to be true?
	int animate;

	/**
	 * The wall time at which the next step of the animation
	 * should execute.
	 */
	uint32 nextTick;

	/**
	 * During playback of the show style, the current step
	 * (out of divisions).
	 */
	int currentStep;

	/**
	 * The next show style.
	 */
	ShowStyleEntry *next;

	/**
	 * Whether or not this style has finished running and
	 * is ready for disposal.
	 */
	bool processed;

	//
	// Engine-specific properties for SCI2 through 2.1early
	//

	// TODO: Could union this stuff to save literally
	// several bytes of memory.

	/**
	 * The width of the plane. Used to determine the correct
	 * size of screen items for wipes.
	 */
	int width;

	/**
	 * The height of the plane. Used to determine the correct
	 * size of screen items for wipes.
	 */
	int height;

	/**
	 * The number of edges that a transition operates on.
	 * Slide wipe: 1 edge
	 * Reveal wipe: 2 edges
	 * Iris wipe: 4 edges
	 */
	// TODO: I have no idea why SCI engine stores this instead
	// of a screenItems count
	int edgeCount;

	/**
	 * Used by transition types 1 through 10.
	 * One screen item per division per edge.
	 */
	ScreenItemList screenItems;

	/**
	 * Used by transition types 11 and 12. A copy of the
	 * visible frame buffer.
	 */
	// TODO: This is a reg_t in SCI engine; not sure if
	// we can avoid allocation through SegMan or not.
	reg_t bitmapMemId;

	/**
	 * Used by transition types 11 and 12. A screen item
	 * used to display the associated bitmap data.
	 */
	ScreenItem *bitmapScreenItem;

	/**
	 * A number used to pick pixels to dissolve by types
	 * 11 and 12.
	 */
	int dissolveSeed;
	int unknown3A;
	// max?
	int dissolveInitial;

	//
	// Engine specific properties for SCI2.1mid through SCI3
	//

	/**
	 * The number of entries in the fadeColorRanges array.
	 */
	uint8 fadeColorRangesCount;

	/**
	 * A pointer to an dynamically sized array of palette
	 * indexes, in the order [ fromColor, toColor, ... ].
	 * Only colors within this range are transitioned.
	 */
	uint16 *fadeColorRanges;
};

typedef Common::Array<DrawList> ScreenItemListList;
typedef Common::Array<RectList> EraseListList;

class GfxCache;
class GfxCoordAdjuster32;
class GfxPaint32;
class GfxPalette;
class GfxScreen;

/**
 * Frameout class, kFrameout and relevant functions for SCI32 games.
 * Roughly equivalent to GraphicsMgr in the actual SCI engine.
 */
class GfxFrameout {
private:
	bool _isHiRes;
	GfxCache *_cache;
	GfxCoordAdjuster32 *_coordAdjuster;
	GfxPalette32 *_palette;
	ResourceManager *_resMan;
	GfxScreen *_screen;
	SegManager *_segMan;
	GfxPaint32 *_paint32;

public:
	GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxCache *cache, GfxScreen *screen, GfxPalette32 *palette, GfxPaint32 *paint32);
	~GfxFrameout();

	void clear();
	void syncWithScripts(bool addElements); // this is what Game::restore does, only needed when our ScummVM dialogs are patched in
	void run();

#pragma mark -
#pragma mark Screen items
private:
	void deleteScreenItem(ScreenItem *screenItem, const reg_t plane);

public:
	void kernelAddScreenItem(const reg_t object);
	void kernelUpdateScreenItem(const reg_t object);
	void kernelDeleteScreenItem(const reg_t object);

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
	int16 kernelGetHighPlanePri();

#pragma mark -
#pragma mark Pics
public:
	void kernelAddPicAt(const reg_t planeObject, const GuiResourceId pictureId, const int16 pictureX, const int16 pictureY, const bool mirrorX);

#pragma mark -

	// TODO: Remap-related?
	void kernelSetPalStyleRange(const uint8 fromColor, const uint8 toColor);

#pragma mark -
#pragma mark Transitions
private:
	int *_dissolveSequenceSeeds;
	int16 *_defaultDivisions;
	int16 *_defaultUnknownC;

	/**
	 * TODO: Documentation
	 */
	ShowStyleEntry *_showStyles;

	inline ShowStyleEntry *findShowStyleForPlane(const reg_t planeObj) const;
	inline ShowStyleEntry *deleteShowStyleInternal(ShowStyleEntry *const showStyle);
	void processShowStyles();
	bool processShowStyleNone(ShowStyleEntry *showStyle);
	bool processShowStyleMorph(ShowStyleEntry *showStyle);
	bool processShowStyleFade(const int direction, ShowStyleEntry *showStyle);
#if 0
	bool processShowStyleWipe(const int direction, ShowStyleEntry *const showStyle);
#endif

public:
	// NOTE: This signature is taken from SCI3 Phantasmagoria 2
	// and is valid for all implementations of SCI32
	void kernelSetShowStyle(const uint16 argc, const reg_t &planeObj, const ShowStyleType type, const int16 seconds, const int16 direction, const int16 priority, const int16 animate, const int16 frameOutNow, const reg_t &pFadeArray, const int16 divisions, const int16 blackScreen);

#pragma mark -
#pragma mark Rendering
private:
	/**
	 * TODO: Documentation
	 */
	int8 _styleRanges[256];

	/**
	 * The internal display pixel buffer. During frameOut,
	 * this buffer is drawn into according to the draw and
	 * erase rects calculated by `calcLists`, then drawn out
	 * to the hardware surface according to the `_showList`
	 * rects (which are also calculated by `calcLists`).
	 */
	Buffer _currentBuffer;

	// TODO: In SCI2.1/SQ6, priority map pixels are not allocated
	// by default. In SCI2/GK1, pixels are allocated, but not used
	// anywhere except within CelObj::Draw in seemingly the same
	// way they are used in SCI2.1/SQ6: that is, never read, only
	// written.
	Buffer _priorityMap;

	/**
	 * TODO: Documentation
	 */
	bool _remapOccurred;

	/**
	 * Whether or not the data in the current buffer is what
	 * is visible to the user. During rendering updates,
	 * this flag is set to false.
	 */
	bool _frameNowVisible;

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
	 */
	void calcLists(ScreenItemListList &drawLists, EraseListList &eraseLists, const Common::Rect &calcRect);

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
	 * TODO: Documentation
	 */
	void palMorphFrameOut(const int8 *styleRanges, const ShowStyleEntry *showStyle);

	/**
	 * Writes the internal frame buffer out to hardware and
	 * clears the show list.
	 */
	void showBits();

public:
	/**
	 * TODO: Document
	 * This is used by CelObj::Draw.
	 */
	bool _hasRemappedScreenItem;

	/**
	 * Whether palMorphFrameOut should be used instead of
	 * frameOut for rendering. Used by kMorphOn to
	 * explicitly enable palMorphFrameOut for one frame.
	 */
	bool _palMorphIsOn;

	inline Buffer &getCurrentBuffer() {
		return _currentBuffer;
	}

	void kernelFrameOut(const bool showBits);

	/**
	 * Updates the internal screen buffer for the next
	 * frame. If `shouldShowBits` is true, also sends the
	 * buffer to hardware.
	 */
	void frameOut(const bool shouldShowBits, const Common::Rect &rect = Common::Rect());

	/**
	 * Modifies the raw pixel data for the next frame with
	 * new palette indexes based on matched style ranges.
	 */
	void alterVmap(const Palette &palette1, const Palette &palette2, const int8 style, const int8 *const styleRanges);

	// TODO: SCI2 engine never uses priority map?
	inline Buffer &getPriorityMap() {
		return _priorityMap;
	}

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
