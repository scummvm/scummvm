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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_CURSOR_H_
#define ASYLUM_CURSOR_H_

#include "asylum/graphics.h"

namespace Asylum {

/**
 * Asylum cursors are GraphicResources, and are stored in
 * ResourcePacks, as are all game assets.
 */
class Cursor {
public:
	Cursor();
	Cursor(ResourcePack *pack);

	virtual ~Cursor();

	/**
	 * Generate a new cursor instance from the resource id
	 * within the resource pack provided.
	 */
	static void create(Cursor *&cursor, ResourcePack *pack, int32 id);

	/**
	 * Show the current cursor
	 */
	void show();

	/**
	 * Hide the current cursor
	 */
	void hide();

	/** .text:00435400
	 * Set the current cursor instance to the graphic resource provide.
	 * The frames parameter defaults to -1, which in this case means that the
	 * frame count should be derived from the graphic resource as opposed to being
	 * explicitely set.
	 */
	void set(uint32 resId, int32 cntr, int32 flgs, int32 frames = -1);
	/**
	 * Set the x/y coordinates of the cursor
	 */
	void move(int16 x, int16 y);

	/**
	 * Scene-based update to the current cursor. This
	 * checks whether the cursor should be updated depending
	 * on the MainActor's current action.
	 *
	 * TODO this probably doesn't belong here, but on the
	 * scene, where it originally was
	 */
	//void update(WorldStats *ws, int32 currentAction);
	/**
	 * Get the next logical frame from the currently loaded
	 * cursorResource and draw it
	 */
	//void animate();

	void update();

	/**
	 * Return the cursor's position on the screen
	 */
	Common::Point position() const { return _pos; }

	// NOTE
	// .text:00435060 contains a function that assigns global variables to a
	// struct associated with cursor graphics info. Since this functionality only
	// ever seems to be used to reference cursor info, the struct members
	// may as well be class members in order to simplify the logic a bit

	// typedef struct CursorInfo {
	int32 grResId;
	int32 currentFrame; // assuming field_4c is the current frame pointer
	                    // since it's generally initialized to zero
	int32 frameCount;
	int32 counter; // cursor counter
	byte  flags;
	byte  field_11;
	// } CursorInfo;

private:
	ResourcePack    *_pack;
	GraphicResource *_cursorRes;

	/** the point on the screen the cursor is at */
	Common::Point _pos;

	/** the point of the cursor that triggers click hits */
	Common::Point _hotspot;

	// The number of millis between
	// cursor gfx updates
	uint32 _cursorTicks;

	byte _cursor_byte_45756C;
}; // end of class Cursor

} // end of namespace Asylum

#endif
