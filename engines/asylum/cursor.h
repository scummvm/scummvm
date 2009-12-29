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

#include "asylum/respack.h"
#include "asylum/worldstats.h"

namespace Asylum {

class WorldStats;

/**
 * Asylum cursors are GraphicResources, and are stored in
 * ResourcePacks, as are all game assets.
 */
class Cursor {
public:
	Cursor(ResourcePack *res);
	virtual ~Cursor();
	/**
	 * Show the current cursor
	 */
	void show();
	/**
	 * Hide the current cursor
	 */
	void hide();
	/**
	 * Load a GraphicResource at the position specified by
	 * index from the buffered ResourcePack
	 */
	void load(int32 index);
	/**
	 * Set the current cursor to a specific frame
	 * within the loaded cursorResource
	 */
	void set(int32 frame);
	/**
	 * Set the x/y coordinates of the cursor
	 */
	void setCoords(int32 mouseX, int32 mouseY);
	/**
	 * Scene-based update to the current cursor. This
	 * checks whether the cursor should be updated depending
	 * on the MainActor's current action.
	 *
	 * TODO this probably doesn't belong here, but on the
	 * scene, where it originally was
	 */
	void update(WorldStats *ws, int32 currentAction);
	/**
	 * Get the next logical frame from the currently loaded
	 * cursorResource and draw it
	 */
	void animate();

	/**
	 * Get the X position of the cursor
	 */
	int32 x() {
		return _mouseX;
	}
	/**
	 * get the Y position of the cursor
	 */
	int32 y() {
		return _mouseY;
	}
	/**
	 * Get the current frame number of the
	 * loaded cursorResource
	 */
	uint32 currentFrame() {
		return _curFrame;
	}


	// NOTE
	// .text:00435060 contains a function that assigns global variables to a
	// struct associated with cursor graphics info. Since this functionality only
	// ever seems to be used to reference cursor info, the struct members
	// may as well be class members in order to simplify the logic a bit

	// typedef struct CursorInfo {
	int32 grResId;
	int32 field_4;
	int32 frameCount;
	int32 field_C;
	byte  flags;
	byte  field_11;
	// } CursorInfo;
private:
	void set(byte *data, byte width, byte height);

	ResourcePack	*_resPack;
	GraphicResource *_cursorResource;
	bool   cursorLoaded;
	uint32 _curFrame;
	int32  _cursorStep;
	int32 _mouseX;
	int32 _mouseY;

	// The number of millis between
	// cursor gfx updates
	uint32 _cursorTicks;

}; // end of class Cursor

} // end of namespace Asylum

#endif
