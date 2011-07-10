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

#ifndef ASYLUM_CURSOR_H
#define ASYLUM_CURSOR_H

#include "asylum/shared.h"

#include "common/events.h"
#include "common/rect.h"

namespace Asylum {

class AsylumEngine;
class GraphicResource;

enum CursorState {
	kCursorStateLeft = 1,
	kCursorStateRight = 2,
	kCursorMiddle = 3
};

enum CursorAnimation {
	kCursorAnimationNone   = 0,
	kCursorAnimationLinear = 1,
	kCursorAnimationMirror = 2
};

/**
 * Asylum cursors are GraphicResources, and are stored in
 * ResourcePacks, as are all game assets.
 */
class Cursor {
public:
	Cursor(AsylumEngine *engine);
	virtual ~Cursor();

	/**
	 * Show the current cursor
	 */
	void show() const;

	/**
	 * Hide the current cursor
	 */
	void hide() const;

	/**
	 * Query if the cursor is hidden.
	 *
	 * @return true if hidden, false if not.
	 */
	bool isHidden() const;

	/**
	 * Set the current cursor instance to the graphic resource provide. The frames parameter defaults to -1, which in this case means that the frame count
	 * should be derived from the graphic resource as opposed to being explicitly set.
	 *
	 * @param resourceId  Identifier for the resource.
	 * @param cnt         The counter.
	 * @param anim        The animation type
	 * @param frames      The frames.
	 */
	void set(ResourceId resourceId, int32 cnt = 0, CursorAnimation anim = kCursorAnimationMirror, int32 frames = -1);

	/**
	 * Get the next logical frame from the currently loaded
	 * cursorResource and draw it
	 */
	void animate();

	void setState(const Common::Event &evt);
	byte getState() { return _state; }

	/**
	 * Return the cursor's position on the screen
	 */
	const Common::Point position() const;

	// NOTE: The original engine contains a function that assigns global variables to a
	// struct associated with cursor graphics info. Since this functionality only
	// ever seems to be used to reference cursor info, the struct members
	// may as well be class members in order to simplify the logic a bit

	ResourceId graphicResourceId;
	uint32 currentFrame;
	uint32 lastFrameIndex;
	int32 counter;
	CursorAnimation animation;

private:
	AsylumEngine *_vm;

	byte _state;

	// Cursor resource
	GraphicResource *_cursorRes;

	/** the point on the screen the cursor is at */
	Common::Point _pos;

	/** the point of the cursor that triggers click hits */
	Common::Point _hotspot;

	// The number of milliseconds between cursor gfx updates
	uint32 _nextTick;

	int32 _frameStep;

	/**
	 * Updates the cursor
	 */
	void update();

	/**
	 * Updates the cursor current frame.
	 */
	void updateFrame();

	/**
	 * Gets the hotspot for a specific frame.
	 *
	 * @param frameIndex Zero-based index of the frame.
	 *
	 * @return The hotspot.
	 */
	Common::Point getHotspot(uint32 frameIndex);
};

} // end of namespace Asylum

#endif
