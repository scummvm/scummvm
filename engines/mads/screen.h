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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_SCREEN_H
#define MADS_SCREEN_H

#include "common/scummsys.h"
#include "common/array.h"
#include "mads/msurface.h"

namespace MADS {

#define MADS_SCREEN_WIDTH 320
#define MADS_SCREEN_HEIGHT 200

enum ScreenTransition {
	kTransitionNone = 0,
	kTransitionFadeIn, kTransitionFadeOutIn,
	kTransitionBoxInBottomLeft, kTransitionBoxInBottomRight,
	kTransitionBoxInTopLeft, kTransitionBoxInTopRight,
	kTransitionPanLeftToRight, kTransitionPanRightToLeft,
	kTransitionCircleIn1, kTransitionCircleIn2,
	kTransitionCircleIn3, kTransitionCircleIn4,
	kVertTransition1, kVertTransition2, kVertTransition3,
	kVertTransition4, kVertTransition5, kVertTransition6,
	kVertTransition7, kCenterVertTransition
};

class ScreenSurface : public MSurface {
private:
	/**
	 * Handles screen fade out
	 */
	void fadeOut();

	/**
	 * Handles screen fade in
	 */
	void fadeIn();
public:
	Common::Point _offset;
	byte *_dataP;
public:
	/**
	 * Constructor
	 */
	ScreenSurface();

	/**
	 * Initialise the surface
	 */
	void init();

	void setPointer(MSurface *s);

	/**
	 * Copys an area of the screen surface to a given destination position on
	 * the ScummVM physical screen buffer
	 * @param destPos	Destination position
	 * @param bounds	Area of screen surface to copy
	 */
	void copyRectToScreen(const Common::Point &destPos, const Common::Rect &bounds);

	/**
	 * Copys an area of the screen surface to the ScmmVM physical screen buffer
	 * @param bounds	Area of screen surface to copy
	 */
	void copyRectToScreen(const Common::Rect &bounds);

	/**
	 * Updates the screen with the contents of the surface
	 */
	void updateScreen();

	void transition(ScreenTransition transitionType, bool surfaceFlag);
};

} // End of namespace MADS

#endif /* MADS_SCREEN_H */
