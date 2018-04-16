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

#ifndef SCI_GRAPHICS_TRANSITIONS32_H
#define SCI_GRAPHICS_TRANSITIONS32_H

#include "common/list.h"
#include "common/scummsys.h"
#include "sci/engine/vm_types.h"

namespace Sci {
enum ShowStyleType {
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
	kShowStyleDissolveNoMorph = 11,
	kShowStyleDissolve = 12,
	kShowStyleFadeOut = 13,
	kShowStyleFadeIn = 14,
	kShowStyleMorph = 15
};

/**
 * A show style represents a transition applied to a Plane. One show style per
 * plane can be active at a time.
 */
struct PlaneShowStyle {
	/**
	 * The ID of the plane this transition applies to.
	 */
	reg_t plane;

	/**
	 * The type of the transition.
	 */
	ShowStyleType type;

	/**
	 * When true, the show style is an entry transition to a new room. When
	 * false, it is an exit transition away from an old room.
	 */
	bool fadeUp;

	/**
	 * The number of steps for the show style.
	 */
	int16 divisions;

	/**
	 * The color used by transitions that draw CelObjColor screen items. -1 for
	 * transitions that do not draw screen items.
	 */
	int16 color;

	/**
	 * The amount of time, in ticks, between each cycle of the animation.
	 */
	int delay;

	/**
	 * If true, GfxTransitions32 will yield back to the main game loop after
	 * calculating the next frame. Otherwise, GfxTransitions32 takes exclusive
	 * control over the game loop until the transition has completed.
	 */
	bool animate;

	/**
	 * The time at which the next step of the animation should execute.
	 */
	uint32 nextTick;

	/**
	 * During playback of the show style, the current step (out of `divisions`).
	 */
	int currentStep;

	/**
	 * Whether or not this style has finished running and is ready for disposal.
	 */
	bool processed;

	//
	// Engine specific properties for SCI2.1early
	//

	/**
	 * A list of screen items, each representing one block of a wipe transition.
	 * These screen items are owned by GfxFrameout.
	 */
	Common::Array<ScreenItem *> screenItems;

	/**
	 * For wipe transitions, the number of edges with a moving wipe (1, 2, or
	 * 4).
	 */
	uint8 numEdges;

	/**
	 * The dimensions of the plane, in game script coordinates.
	 */
	int16 width, height;

	/**
	 * For pixel dissolve transitions, the screen item used to render the
	 * transition. This screen item is owned by GfxFrameout.
	 */
	ScreenItem *bitmapScreenItem;

	/**
	 * For pixel dissolve transitions, the bitmap used to render the transition.
	 */
	reg_t bitmap;

	/**
	 * The bit mask used by pixel dissolve transitions.
	 */
	uint32 dissolveMask;

	/**
	 * The first pixel that was dissolved in a pixel dissolve transition.
	 */
	uint32 firstPixel;

	/**
	 * The last pixel that was dissolved. Once all pixels have been dissolved,
	 * `pixel` will once again equal `firstPixel`.
	 */
	uint32 pixel;

	//
	// Engine specific properties for SCI2.1mid through SCI3
	//

	/**
	 * An array of palette indexes, in the order [ fromColor, toColor, ... ].
	 * Only colors within this range are transitioned.
	 */
	Common::Array<uint16> fadeColorRanges;
};

/**
 * PlaneScroll describes a transition between two different pictures within a
 * single plane.
 */
struct PlaneScroll {
	/**
	 * The ID of the plane to be scrolled.
	 */
	reg_t plane;

	/**
	 * The current position of the scroll.
	 */
	int16 x, y;

	/**
	 * The distance that should be scrolled. Only one of `deltaX` or `deltaY`
	 * may be set.
	 */
	int16 deltaX, deltaY;

	/**
	 * The pic that should be created and scrolled into view inside the plane.
	 */
	GuiResourceId newPictureId;

	/**
	 * The picture that should be scrolled out of view and deleted from the
	 * plane.
	 */
	GuiResourceId oldPictureId;

	/**
	 * If true, GfxTransitions32 will yield back to the main game loop after
	 * calculating the next frame. Otherwise, GfxTransitions32 takes exclusive
	 * control over the game loop until the transition has completed.
	 */
	bool animate;

	/**
	 * The tick after which the animation will start.
	 */
	uint32 startTick;
};

typedef Common::List<PlaneShowStyle> ShowStyleList;
typedef Common::List<PlaneScroll> ScrollList;

class GfxTransitions32 {
public:
	GfxTransitions32(SegManager *_segMan);
	~GfxTransitions32();

private:
	SegManager *_segMan;

	/**
	 * Throttles transition playback to prevent transitions from being
	 * instantaneous on modern computers.
	 *
	 * kSetShowStyle transitions are throttled at 10ms intervals, under the
	 * assumption that the default fade transition of 101 divisions was designed
	 * to finish in one second. Empirically, this seems to roughly match the
	 * speed of DOSBox, and feels reasonable.
	 *
	 * Transitions using kSetScroll (used in the LSL6hires intro) need to be
	 * slower, so they get throttled at 33ms instead of 10ms. This value was
	 * chosen by gut feel, as these scrolling transitions are instantaneous in
	 * DOSBox.
	 */
	void throttle(const uint32 ms = 10);

	void clearShowRects();
	void addShowRect(const Common::Rect &rect);
	void sendShowRects();

#pragma mark -
#pragma mark Show styles
public:
	inline bool hasShowStyles() const { return !_showStyles.empty(); }

	/**
	 * Processes all active show styles in a loop until they are finished.
	 */
	void processShowStyles();

	/**
	 * Processes show styles that are applied through
	 * `GfxFrameout::palMorphFrameOut`.
	 */
	void processEffects(PlaneShowStyle &showStyle);

	void kernelSetShowStyle(const uint16 argc, const reg_t planeObj, const ShowStyleType type, const int16 seconds, const int16 direction, const int16 priority, const int16 animate, const int16 frameOutNow, reg_t pFadeArray, int16 divisions, const int16 blackScreen);

	/**
	 * Sets the range that will be used by `GfxFrameout::palMorphFrameOut` to
	 * alter palette entries.
	 */
	void kernelSetPalStyleRange(const uint8 fromColor, const uint8 toColor);

	/**
	 * A map of palette entries that can be morphed by the Morph show style.
	 */
	int8 _styleRanges[256];

private:
	/**
	 * Default sequence values for pixel dissolve transition bit masks.
	 */
	int *_dissolveSequenceSeeds;

	/**
	 * Default values for `PlaneShowStyle::divisions` for the current SCI
	 * version.
	 */
	int16 *_defaultDivisions;

	/**
	 * The list of PlaneShowStyles that are currently active.
	 */
	ShowStyleList _showStyles;

	/**
	 * Finds a show style that applies to the given plane.
	 */
	PlaneShowStyle *findShowStyleForPlane(const reg_t planeObj);

	/**
	 * Finds the iterator for a show style that applies to the given plane.
	 */
	ShowStyleList::iterator findIteratorForPlane(const reg_t planeObj);

	/**
	 * Deletes the given PlaneShowStyle and returns the next PlaneShowStyle from
	 * the list of styles.
	 */
	ShowStyleList::iterator deleteShowStyle(const ShowStyleList::iterator &showStyle);

	/**
	 * Initializes the given PlaneShowStyle for a horizontal wipe effect for
	 * SCI2 to 2.1early.
	 */
	void configure21EarlyHorizontalWipe(PlaneShowStyle &showStyle, const int16 priority);

	/**
	 * Initializes the given PlaneShowStyle for a horizontal shutter effect for
	 * SCI2 to 2.1early.
	 */
	void configure21EarlyHorizontalShutter(PlaneShowStyle &showStyle, const int16 priority);

	/**
	 * Initializes the given PlaneShowStyle for an iris effect for SCI2 to
	 * 2.1early.
	 */
	void configure21EarlyIris(PlaneShowStyle &showStyle, const int16 priority);

	/**
	 * Initializes the given PlaneShowStyle for a pixel dissolve effect for SCI2
	 * to 2.1early.
	 */
	void configure21EarlyDissolve(PlaneShowStyle &showStyle, const int16 priority, const Common::Rect &gameRect);

	/**
	 * Processes one tick of the given PlaneShowStyle.
	 */
	bool processShowStyle(PlaneShowStyle &showStyle, uint32 now);

	/**
	 * Performs an instant transition between two rooms.
	 */
	bool processNone(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders into a room with a horizontal shutter
	 * effect.
	 */
	bool processHShutterOut(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders to black with a horizontal shutter
	 * effect.
	 */
	void processHShutterIn(const PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders into a room with a vertical shutter
	 * effect.
	 */
	void processVShutterOut(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders to black with a vertical shutter
	 * effect.
	 */
	void processVShutterIn(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders into a room with a wipe to the left.
	 */
	void processWipeLeft(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders to black with a wipe to the right.
	 */
	void processWipeRight(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders into a room with a wipe upwards.
	 */
	void processWipeUp(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders to black with a wipe downwards.
	 */
	void processWipeDown(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders into a room with an iris effect.
	 */
	bool processIrisOut(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders to black with an iris effect.
	 */
	bool processIrisIn(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders between rooms using a block dissolve
	 * effect.
	 */
	void processDissolveNoMorph(PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that renders between rooms with a pixel dissolve
	 * effect.
	 */
	bool processPixelDissolve(PlaneShowStyle &showStyle);

	/**
	 * SCI2 to 2.1early implementation of pixel dissolve.
	 */
	bool processPixelDissolve21Early(PlaneShowStyle &showStyle);

	/**
	 * SCI2.1mid and later implementation of pixel dissolve.
	 */
	bool processPixelDissolve21Mid(const PlaneShowStyle &showStyle);

	/**
	 * Performs a transition that fades to black between rooms.
	 */
	bool processFade(const int8 direction, PlaneShowStyle &showStyle);

	/**
	 * Morph transition calls back into the transition system's `processEffects`
	 * method, which then applies transitions other than None, Fade, or Morph.
	 */
	bool processMorph(PlaneShowStyle &showStyle);

	/**
	 * Performs a generic transition for any of the wipe/shutter/iris effects.
	 */
	bool processWipe(const int8 direction, PlaneShowStyle &showStyle);

#pragma mark -
#pragma mark Scrolls
public:
	inline bool hasScrolls() const { return !_scrolls.empty(); }

	/**
	 * Processes all active plane scrolls in a loop until they are finished.
	 */
	void processScrolls();

	void kernelSetScroll(const reg_t plane, const int16 deltaX, const int16 deltaY, const GuiResourceId pictureId, const bool animate, const bool mirrorX);

private:
	/**
	 * A list of active plane scrolls.
	 */
	ScrollList _scrolls;

	/**
	 * Performs a scroll of the content of a plane.
	 */
	bool processScroll(PlaneScroll &scroll);
};

} // End of namespace Sci
#endif
